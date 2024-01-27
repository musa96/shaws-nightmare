#include "debug.h"
#include "sn.h"
#include "weapon.h"
#include "player.h"
#include "sfx.h"
#include "str.h"
#include "movers.h"
#include "timer.h"
#include "triggers.h"

SwitchList_T switchlist[] =
{
	{Switch1A, Switch1B},
	{Switch2A, Switch2B},
	{Switch3A, Switch3B},
	{Switch4A, Switch4B},
	{Switch5A, Switch5B},
	{Switch6A, Switch6B},
	{Switch7A, Switch7B},
	{Switch8A, Switch8B},
	{Switch9A, Switch9B}
};

short gatesprite[16], gatetrob[16], gatecnt;
short revolvesector[4], revolveang[4], revolvecnt;
long revolvex[4][16], revolvey[4][16];
long revolvepivotx[4], revolvepivoty[4];
short dragsector[16], dragxdir[16], dragydir[16], dragsectorcnt;
long dragx1[16], dragy1[16], dragx2[16], dragy2[16], dragfloorz[16];
int pansectorcnt;
short pansprite[16], pansector[16];
long panx[16], pany[16];
int crushcnt;
short crushsector[32];
long crusholdz[32];
short wallfind[MAXSECTORS][2];
int activelifts[MAXLIFTS];
long liftoldz[MAXLIFTS];
short liftsector[MAXLIFTS], lifttrob[MAXLIFTS];
short swingcnt, swingwall[32][5], swingsector[32];
short swingangopen[32], swingangclosed[32], swingangopendir[32];
short swingang[32], swinganginc[32];
long swingx[32][8], swingy[32][8];
long numinterpolations = 0, startofdynamicinterpolations = 0;
long oldipos[MAXINTERPOLATIONS];
long bakipos[MAXINTERPOLATIONS];
long *curipos[MAXINTERPOLATIONS];
int tag502cnt, tag502active, tag502time;

void setinterpolation(long *posptr)
{
	long i;

	if (numinterpolations >= MAXINTERPOLATIONS) return;
	for(i=numinterpolations-1;i>=0;i--)
		if (curipos[i] == posptr) return;
	curipos[numinterpolations] = posptr;
	oldipos[numinterpolations] = *posptr;
	numinterpolations++;
}

void stopinterpolation(long *posptr)
{
	long i;

	for(i=numinterpolations-1;i>=startofdynamicinterpolations;i--)
		if (curipos[i] == posptr)
		{
			numinterpolations--;
			oldipos[i] = oldipos[numinterpolations];
			bakipos[i] = bakipos[numinterpolations];
			curipos[i] = curipos[numinterpolations];
		}
}

void updateinterpolations(void)  //Stick at beginning of domovethings
{
	long i;

	for(i=numinterpolations-1;i>=0;i--) oldipos[i] = *curipos[i];
}

void dointerpolations(void)       //Stick at beginning of drawscreen
{
	long i, j, odelta, ndelta;

	ndelta = 0; j = 0;
	for(i=numinterpolations-1;i>=0;i--)
	{
		bakipos[i] = *curipos[i];
		odelta = ndelta; ndelta = (*curipos[i])-oldipos[i];
		//if (odelta != ndelta) j = mulscale16(ndelta,smoothratio);
		*curipos[i] = oldipos[i]+j;
	}
}

void restoreinterpolations(void)  //Stick at end of drawscreen
{
	long i;

	for(i=numinterpolations-1;i>=0;i--) *curipos[i] = bakipos[i];
}

void animateswitch(short spritenum)
{
	long i;

	for(i=0;i<NUMSWITCHES;i++)
	{
		if (sprite[spritenum].picnum == switchlist[i].picnum1)
			sprite[spritenum].picnum = switchlist[i].picnum2;
		else if (sprite[spritenum].picnum == switchlist[i].picnum2)
			sprite[spritenum].picnum = switchlist[i].picnum1;
	}
	playxysfx(SfxSwitch, 2, sprite[spritenum].x, sprite[spritenum].y);
}

void addlift(short sectnum, int trobtype, int direction)
{
	long i, j;

	for(i=0;i<MAXLIFTS;i++)
	{
		if (liftsector[i] == sectnum) return;

		if (activelifts[i] == 0)
		{
			activelifts[i] = 1;
			liftoldz[i] = sector[sectnum].floorz;
			liftsector[i] = sectnum;
			lifttrob[i] = addtrob(trobtype, sectnum);
			trob[lifttrob[i]].direction = direction;
			if (trobtype < 102)
				playxysfx(SfxElevatorStart, i, trob[lifttrob[i]].centx, trob[lifttrob[i]].centy);
			return;
		}
	}	
	error("addlift(): no more lifts!", 502);
}

void removelift(int lift)
{
	if (activelifts[lift] == 1)
	{
		removetrob(lifttrob[lift]);
		lifttrob[lift] = -1;
		liftsector[lift] = -1;
		liftoldz[lift] = 0;
		activelifts[lift] = 0;
		return;
	}
	error("removelift(): cannot find active lift!", 503);
}

void operatelift(short sectnum, int upordown)
{
	long i;

	for(i=0;i<numtrobs;i++)
	{
		if (trob[i].sectnum == sectnum)
			return;
	}
	addlift(sectnum, 100+upordown, 0);
}

void operatezdoor(short sectnum)
{
	long i;

	for(i=0;i<numtrobs;i++)
	{
		if (trob[i].sectnum == sectnum)
		{
			trob[i].direction ^= 1;
			trob[i].type = 111;
			return;
		}
	}
	i = addtrob(111, sectnum);
	playxysfx(SfxDoor, 3, trob[i].centx, trob[i].centy);
}

void operatecombzdoor(short sectnum)
{
	long i;

	for(i=0;i<numtrobs;i++)
	{
		if (trob[i].sectnum == sectnum)
		{
			trob[i].direction ^= 1;
			trob[i].type = 115;
			return;
		}
	}
	i = addtrob(115, sectnum);
	playxysfx(SfxDoor, 3, trob[i].centx, trob[i].centy);
}

void operateblazezdoor(short sectnum)
{
	long i;

	for(i=0;i<numtrobs;i++)
	{
		if (trob[i].sectnum == sectnum)
		{
			trob[i].direction ^= 1;
			trob[i].type = 113;
			return;
		}
	}
	i = addtrob(113, sectnum);
	playxysfx(SfxDoor, 3, trob[i].centx, trob[i].centy);
}

void smooshdoor(short sectnum)
{
	long i, j, k;
	int sound;
	short startwall, endwall;

	for(i=0;i<numtrobs;i++)
	{
		if (trob[i].sectnum == sectnum)
		{
			trob[i].direction ^= 1;
			if (trob[i].direction == 1) sound = SfxFoldDoorClose;
			else sound = SfxFoldDoorOpen;
			playxysfx(sound, i, trob[i].centx, trob[i].centy);
			return;
		}
	}
	startwall = sector[sectnum].wallptr;
	endwall = startwall + sector[sectnum].wallnum;
	//find any points with either same x or same y coordinate
	//  as center (centx, centy) - should be 2 points found.
	wallfind[sectnum][0] = -1;
	wallfind[sectnum][1] = -1;
	i = addtrob(200, sectnum);
	for(k=startwall;k<endwall;k++)
		if ((wall[k].x == trob[i].centx) || (wall[k].y == trob[i].centy))
		{
			if (wallfind[sectnum][0] == -1)
				wallfind[sectnum][0] = k;
			else
				wallfind[sectnum][1] = k;
		}
		for(j=0;j<2;j++)
		{
			if ((wall[wallfind[sectnum][j]].x == trob[i].centx) && (wall[wallfind[sectnum][j]].y == trob[i].centy))
				trob[i].direction = 0;
			else
				trob[i].direction = 1;
		}
	if (trob[i].direction == 1) sound = SfxFoldDoorClose;
	else sound = SfxFoldDoorOpen;
	playxysfx(sound, i, trob[i].centx, trob[i].centy);
}

void slidedoor(short sectnum)
{
	long i, j;
	short startwall, endwall;

	for(i=0;i<numtrobs;i++)
	{
		if (trob[i].sectnum == sectnum)
		{
			trob[i].direction ^= 1;
			return;
		}
	}
	startwall = sector[sectnum].wallptr;
	endwall = startwall + sector[sectnum].wallnum;

	//get 2 closest line segments to center (dax, day)
	wallfind[sectnum][0] = -1;
	wallfind[sectnum][1] = -1;
	for(i=startwall;i<endwall;i++)
		if (wall[i].lotag == 6)
		{
			if (wallfind[sectnum][0] == -1)
				wallfind[sectnum][0] = i;
			else
				wallfind[sectnum][1] = i;
		}
	i = addtrob(201, sectnum);
	for(j=0;j<2;j++)
	{
		if ((((wall[wallfind[sectnum][j]].x+wall[wall[wallfind[sectnum][j]].point2].x)>>1) == trob[i].centx) && (((wall[wallfind[sectnum][j]].y+wall[wall[wallfind[sectnum][j]].point2].y)>>1) == trob[i].centy))
		     //door was closed
			trob[i].direction = 0;
		else //door was not closed
			trob[i].direction = 1;
	}
	playxysfx(SfxDoorSlab, i, trob[i].centx, trob[i].centy);
}

void swingdoor(short sectnum)
{
	long i, j;

	for(i=0;i<swingcnt;i++)
	{
		if (swingsector[i] == sectnum)
		{
			if (swinganginc[i] == 0)
			{
				if (swingang[i] == swingangclosed[i])
				{
					swinganginc[i] = swingangopendir[i];
					playxysfx(SfxSwingOpen, swingsector[i], swingx[i][0], swingy[i][0]);
				}
				else
					swinganginc[i] = -swingangopendir[i];
			}
			else
				swinganginc[i] = -swinganginc[i];

			for(j=1;j<=3;j++)
			{
				setinterpolation(&wall[swingwall[i][j]].x);
				setinterpolation(&wall[swingwall[i][j]].y);
			}
		}
	}
}

void operatesector(short sectnum)
{
	long i;

	switch(sector[sectnum].lotag)
	{
		case 100:
		operatelift(sectnum, sector[sectnum].hitag);
		break;
		case 111:
		if (sector[sectnum].hitag)
		{
			if (!havekey[sector[sectnum].hitag-1])
			{
				putmessage(string[StringNeedKey]);
				return;
			}
		}
		operatezdoor(sectnum);
		break;
		case 112:
		if (sector[sectnum].hitag)
		{
			if (!havekey[sector[sectnum].hitag-1])
			{
				putmessage(string[StringNeedKey]);
				return;
			}
		}
		sector[sectnum].lotag = 0;
		i = addtrob(112, sectnum);
		playxysfx(SfxDoor, 3, trob[i].centx, trob[i].centy);
		break;
		case 116:
		if (sector[sectnum].hitag)
		{
			if (!havekey[sector[sectnum].hitag-1])
			{
				putmessage(string[StringNeedKey]);
				return;
			}
		}
		swingdoor(sectnum);
		break;
		case 117:
		if (sector[sectnum].hitag)
		{
			if (!havekey[sector[sectnum].hitag-1])
			{
				putmessage(string[StringNeedKey]);
				return;
			}
		}
		operateblazezdoor(sectnum);
		break;
		case 118:
		if (sector[sectnum].hitag)
		{
			if (!havekey[sector[sectnum].hitag-1])
			{
				putmessage(string[StringNeedKey]);
				return;
			}
		}
		sector[sectnum].lotag = 0;
		i = addtrob(114, sectnum);
		playxysfx(SfxDoor, 3, trob[i].centx, trob[i].centy);
		break;
		case 119:
		if (sector[sectnum].hitag)
		{
			if (!havekey[sector[sectnum].hitag-1])
			{
				putmessage(string[StringNeedKey]);
				return;
			}
		}
		smooshdoor(sectnum);
		break;
		case 120:
		slidedoor(sectnum);
		break;
		case 121:
		operatecombzdoor(sectnum);
		break;
	}
}

void operatewall(short wallnum)
{
	long i;

	switch(wall[wallnum].lotag)
	{
		case 101:
		for(i=0;i<numsectors;i++)
		{
			if (sector[i].lotag == wall[wallnum].hitag)
				operatelift(i, sector[i].hitag);
		}
		break;
		case 111:
		for(i=0;i<numsectors;i++)
		{
			if (sector[i].lotag == wall[wallnum].hitag)
				operatezdoor(i);
		}
		break;
	}
}

void operatesprite(short spritenum)
{
	long i, j, k;

	switch(sprite[spritenum].lotag)
	{
		case 10:
		exitsecretlevel();
		break;
		case 24:
		case 25:
		if (sprite[spritenum].lotag == 25) sprite[spritenum].lotag = 0;
		for(i=0;i<numsectors;i++)
		{
			if (sector[i].lotag == sprite[spritenum].hitag)
			{
				if (sprite[spritenum].lotag == 24)
					operatezdoor(i);
				else
				{
					j = addtrob(112, i);
					playxysfx(SfxDoor, 3, trob[j].centx, trob[j].centy);
				}
			}
		}	
		animateswitch(spritenum);
		break;
		case 26:
		sprite[spritenum].lotag = 0;
		for(i=0;i<numsectors;i++)
		{
			if (sector[i].lotag == sprite[spritenum].hitag)
				addtrob(26, i);
		}
		animateswitch(spritenum);
		break;
		case 31:
		case 32:
		if (sprite[spritenum].lotag == 32) sprite[spritenum].lotag = 0;
		for(i=0;i<numsectors;i++)
		{
			if (sector[i].lotag == sprite[spritenum].hitag)
			{
				if (sprite[spritenum].lotag == 31)
					operatelift(i, sector[i].hitag);
				else
					addlift(i, 102, sector[i].hitag);
			}
		}
		animateswitch(spritenum);
		break;
		case 34:
		sprite[spritenum].lotag = 0;
		for(i=0;i<numsectors;i++)
		{
			if (sector[i].lotag == sprite[spritenum].hitag)
			{
				j = addtrob(111, i);
				trob[j].direction = 1;
			}
		}
		animateswitch(spritenum);
		break;
		case 42:
		sprite[spritenum].lotag = 0;
		for(i=0;i<numsectors;i++)
		{
			if (sector[i].lotag == sprite[spritenum].hitag)
			{
				j = nextsectorneighborz(i,sector[i].floorz,-1,sector[i].hitag);
				sector[i].floorpicnum = sector[j].floorpicnum;
				addlift(i, 102, sector[i].hitag);
			}
		}	
		animateswitch(spritenum);
		break;
		case 57:
		sprite[spritenum].lotag = 0;
		for(i=0;i<numsectors;i++)
		{
			if (sector[i].lotag == sprite[spritenum].hitag)
			{
				crusholdz[crushcnt] = sector[i].ceilingz;
				crushsector[crushcnt] = i;
				addtrob(63, i);
				crushcnt++;
			}
		}
		animateswitch(spritenum);
		break;
		case 58:
		sprite[spritenum].lotag = 0;
		for(i=0;i<numsectors;i++)
		{
			if (sector[i].lotag == sprite[spritenum].hitag)
			{
				for(j=0;j<crushcnt;j++)
				{
					if (crushsector[j] == i)
					{
						for(k=0;k<numtrobs;k++)
						{
							if (trob[k].sectnum == i)
								removetrob(k);
						}
						crushcnt--;
						if (crushcnt != j)
						{
							crusholdz[j] = crusholdz[crushcnt];
							crushsector[j] = crushsector[crushcnt];
						}
					}
				}
			}
		}
		animateswitch(spritenum);
		break;
		case 105:
		if (havekey[0] == 0)
		{
			putmessage(string[StringNeedKey]);
			return;
		}
		sprite[spritenum].lotag = 0;
		for(i=0;i<numsectors;i++)
		{
			if (sector[i].lotag == sprite[spritenum].hitag)
				addlift(i, 102, sector[i].hitag);
		}
		animateswitch(spritenum);
		break;
		case 106:
		if (havekey[1] == 0)
		{
			putmessage(string[StringNeedKey]);
			return;
		}
		sprite[spritenum].lotag = 0;
		for(i=0;i<numsectors;i++)
		{
			if (sector[i].lotag == sprite[spritenum].hitag)
				addlift(i, 102, sector[i].hitag);
		}
		animateswitch(spritenum);
		break;
		case 107:
		if (havekey[2] == 0)
		{
			putmessage(string[StringNeedKey]);
			return;
		}
		sprite[spritenum].lotag = 0;
		for(i=0;i<numsectors;i++)
		{
			if (sector[i].lotag == sprite[spritenum].hitag)
				addlift(i, 102, sector[i].hitag);
		}
		animateswitch(spritenum);
		break;
		case 110:
		/*for(i=0;i<gatecnt;i++)
		{
			if (gatesprite[i] == spritenum)
			{
				if (gatetrob[i] == -1)
					gatetrob[i] = addtrob(66, sprite[spritenum].sectnum);
				else
				{
					if (trob[gatetrob[i]].direction != 1)
						trob[gatetrob[i]].direction = 1;
				}
			}
		}*/
		break;
		case 130:
		sprite[spritenum].lotag = 0;
		for(i=0;i<numsectors;i++)
		{
			if (sector[i].lotag == sprite[spritenum].hitag)
				addlift(i, 109, 0);
		}
		animateswitch(spritenum);
		break;
		case 500:
		sprite[spritenum].lotag = 0;
		for(i=0;i<MAXSPRITES;i++)
		{
			if (sprite[i].lotag == 501)
			{
				sprite[i].lotag = 502;
				tag502cnt++;
			}
		}
		animateswitch(spritenum);
		break;
		case 502:
		sprite[spritenum].lotag = 0;
		tag502cnt--;
		if (tag502cnt < 1)
		{
			tag502time = 0;
			tag502active = 1;
		}
		animateswitch(spritenum);
		break;
	}
}
