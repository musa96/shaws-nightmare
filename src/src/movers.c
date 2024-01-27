#include <stdlib.h>
#include "sn.h"
#include "debug.h"
#include "warp.h"
#include "movers.h"
#include "sfx.h"
#include "controls.h"
#include "levels.h"
#include "weapon.h"
#include "player.h"
#include "timer.h"
#include "ai.h"
#include "seq.h"
#include "triggers.h"
#include "maps.h"

extern short hitsprite;
short lightstart[MAXSECTORS], lightend[MAXSECTORS];
int spawnspotcnt, spawnsprite[32];
long spawnx[32], spawny[32];
int numtrobs, nummobs;
Trob_T trob[MAXSECTORS];
Mob_T mob[MAXSPRITES];
Seq_T mobseq[SeqEnd];
static char *seqnames[SeqEnd] =
{
	"BOYWALK.SEQ",
	"BOYATK.SEQ",
	"BOYHURT.SEQ",
	"BOYDIE.SEQ",
	"BOYXPL.SEQ",
	"FATWALK.SEQ",
	"FATATK.SEQ",
	"FATHURT.SEQ",
	"FATDIE.SEQ",
	"CATWALK.SEQ",
	"CATATK.SEQ",
	"CATHURT.SEQ",
	"CATDIE.SEQ",
	"CATXPL.SEQ",
	"DOGWALK.SEQ",
	"DOGBITE.SEQ",
	"DOGHURT.SEQ",
	"DOGDIE.SEQ",
	"KALIWLK.SEQ",
	"KALIATK.SEQ",
	"KALIDIE.SEQ",
	"HUGEWLK.SEQ",
	"HUGEFIR.SEQ",
	"HUGEHRT.SEQ",
	"HUGEDIE.SEQ",
	"BABYFLY.SEQ",
	"BABYATK.SEQ",
	"BABYHURT.SEQ",
	"BABYDIE.SEQ",
	"FISHFLY.SEQ",
	"FISHATK.SEQ",
	"FISHDIE.SEQ",
	"CHUIWALK.SEQ",
	"CHUIFIRE.SEQ",
	"CHUIDIE.SEQ",
	"PLAYWLK.SEQ",
	"PLAYDIE.SEQ",
	"PLAYDUCK.SEQ",
	"EXPLODE.SEQ",
	"CPHIT.SEQ",
	"FEXP.SEQ",
	"FSPRAY.SEQ",
	"ACIDEX.SEQ",
	"CRIMHIT.SEQ",
	"PUFF.SEQ",
	"GLASS.SEQ",
	"LEGGIB.SEQ",
	"HEADGIB.SEQ",
	"GIB.SEQ",
	"DUCK.SEQ",
	"TELEPORT.SEQ",
	"CMTWALK.SEQ",
	"CMTFIRE.SEQ",
	"CMTDIE.SEQ",
	"CMTPORT.SEQ",
	"KINWALK.SEQ",
	"KINATK.SEQ",
	"KINDIE.SEQ",
	"GIRLWALK.SEQ",
	"GIRLATK.SEQ",
	"GIRLDIE.SEQ",
	"GIRLHURT.SEQ",
	"PLOPFLY.SEQ",
	"PLOPFIRE.SEQ",
	"PLOPDIE.SEQ",
	"LITFWLK.SEQ",
	"LITFATK.SEQ",
	"LITFHRT.SEQ",
	"LITFDIE.SEQ",
	"LITFXPL.SEQ",
	"BCLEWALK.SEQ",
	"BCLEFIRE.SEQ",
	"BCLEDIE.SEQ",
	"BALLHIT.SEQ",
	"BOSSSPIT.SEQ",
	"BOSSEYE.SEQ",
	"BOSSDIE.SEQ",
	"KHTWALK.SEQ",
	"KHTATK.SEQ",
	"KHTDIE.SEQ",
	"OGREWALK.SEQ",
	"OGREFIRE.SEQ",
	"OGREATK.SEQ",
	"OGREDIE.SEQ",
	"KARHURT.SEQ",
	"KARDIE.SEQ",
	"LILWALK.SEQ",
	"LILATAK.SEQ",
	"LILDIE.SEQ",
	"BEXP.SEQ",
	"CLAXPL.SEQ",
	"ZGRIP.SEQ",
	"ZOMBWALK.SEQ",
	"ZOMATAK.SEQ",
	"ZOMBDIE.SEQ",
	"SPAWNFX.SEQ"
};

void initmovers(void)
{
	long i, j;

	if (gamemode == sequel) j = 0;
	else j = 38;
	for(i=0;i<SeqEnd-j;i++)
	{
		loadseq(seqnames[i], &mobseq[i]);
	}
	for(i=0;i<NumMobTypes;i++)
	{
		if (waloff[mobinfo[i].standpicnum] == 0)
			loadtile(mobinfo[i].standpicnum);
	}
}

void clearmovers(void)
{
	long i;

	nummobs = 0;
	numtrobs = 0;
	clearbufbyte(&trob[0],sizeof(Trob_T)*MAXSECTORS,0L);
	clearbufbyte(&mob[0],sizeof(Mob_T)*MAXSPRITES,0L);
}

int translatesector(long *x1, long x2)
{
	long i;

	if (i == x2) return(0);
	i = *x1;
	if (i < x2)
		i = min(i+4*TICSPERFRAME,x2);
	else
		i = max(i-4*TICSPERFRAME,x2);

	*x1 = i;

	return(1);
}

int movesectorz(short sectnum, long dest, int speed, int direction, int floororceiling)
{
	long i, j, k, templong;

	switch(floororceiling)
	{
		case 0:
		i = headspritesect[sectnum];
		while (i != -1)
		{
			k = 0;
			for(j=0;j<NUMSWITCHES;j++)
			{
				if (sprite[i].picnum == switchlist[j].picnum1
					|| sprite[i].picnum == switchlist[j].picnum2)
				{
					k = 1;
					break;
				}
			}
			if (sprite[i].picnum == SpawnSpot) k = 1;
			if (k == 1) { i = nextspritesect[i]; continue; }
			templong = getflorzofslope(sectnum,sprite[i].x,sprite[i].y);
			if (sprite[i].cstat&128) templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
			if (sprite[i].z == templong || sprite[i].statnum == 6 || sprite[i].statnum == 2)
			{
				if (direction == 0) sprite[i].z -= (speed<<8);
					else sprite[i].z += (speed<<8);
			}
			i = nextspritesect[i];
		}
		if (cursectnum == sectnum && posz >= sector[sectnum].floorz-PLAYERHEIGHT)
		{
			if (direction == 0) posz -= (speed<<8);
			else posz += (speed<<8);
		}
		if (sector[sectnum].floorz == dest) return(0);
		switch (direction) {
			case 0:
			sector[sectnum].floorz -= (speed<<8);
			break;
			case 1:
			sector[sectnum].floorz += (speed<<8);
			break;
		}
		if (sector[sectnum].floorz == dest) return(0);
		break;
		case 1:
		i = headspritesect[sectnum];
		while (i != -1)
		{
			templong = getceilzofslope(sectnum,sprite[i].x,sprite[i].y);
			templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<2);
			if (sprite[i].cstat&128) templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
			if (sprite[i].z <= templong)
			{
				for(j=0;j<nummobs;j++)
				{
					if (mob[j].spritenum == i)
					{
						if (mob[j].aistate == AIDeath)
						{
							setmobseq(j, -1);
							sprite[i].picnum = Puddle;
						}
					}
				}
			}
			i = nextspritesect[i];
		}
		if (sector[sectnum].ceilingz == dest) return(0);
		switch (direction) {
			case 0:
			sector[sectnum].ceilingz -= (speed<<8);
			break;
			case 1:
			sector[sectnum].ceilingz += (speed<<8);
			break;
		}
		if (sector[sectnum].ceilingz == dest) return(0);
		break;
	}
	return(1);
}

int addtrob(int type, short sectnum)
{
	long i;
	short startwall, endwall;

	if (numtrobs >= MAXSECTORS) error("Trobs Overflow", 315);

	trob[numtrobs].type = type;
	trob[numtrobs].sectnum = sectnum;
	trob[numtrobs].direction = 0;
	trob[numtrobs].centx = 0L;
	trob[numtrobs].centy = 0L;
	startwall = sector[sectnum].wallptr;
	endwall = startwall + sector[sectnum].wallnum;
	for(i=startwall;i<endwall;i++)
	{
		trob[numtrobs].centx += wall[i].x;
		trob[numtrobs].centy += wall[i].y;
	}
	trob[numtrobs].centx /= (endwall-startwall);
	trob[numtrobs].centy /= (endwall-startwall);
	numtrobs++;

	return(numtrobs-1);
}

void removetrob(short trobnum)
{
	long i;

	numtrobs--;
	if (trobnum != numtrobs)
	{
		for(i=0;i<MAXLIFTS;i++)
			if (lifttrob[i] == numtrobs)
				lifttrob[i] = trobnum;
		trob[trobnum] = trob[numtrobs];
	}
}

void processtrobs(void)
{
	long i, j, k, l, m, dax, day, dax2, day2, templong;
	short daang, dasector, startwall, endwall;
	char tempbuf[80];

	for(i=0;i<numtrobs;i++)
	{
		switch(trob[i].type)
		{
			case 23:
			sector[trob[i].sectnum].extra++;
			if (sector[trob[i].sectnum].extra > 4)
			{
				sector[trob[i].sectnum].floorshade = lightend[trob[i].sectnum];	
				sector[trob[i].sectnum].ceilingshade = lightend[trob[i].sectnum];
				startwall = sector[trob[i].sectnum].wallptr;
				endwall = startwall + sector[trob[i].sectnum].wallnum;
				for(k=startwall;k<endwall;k++)
				{
					wall[k].shade = lightend[trob[i].sectnum];
				}
			}	
			if (sector[trob[i].sectnum].extra > 12)
			{
				sector[trob[i].sectnum].floorshade = lightstart[trob[i].sectnum];	
				sector[trob[i].sectnum].ceilingshade = lightstart[trob[i].sectnum];
				startwall = sector[trob[i].sectnum].wallptr;
				endwall = startwall + sector[trob[i].sectnum].wallnum;
				for(k=startwall;k<endwall;k++)
				{
					wall[k].shade = lightstart[trob[i].sectnum];
				}
			}
			if (sector[trob[i].sectnum].extra > 32)
				sector[trob[i].sectnum].extra = 0;
			break;
			case 24:
			sector[trob[i].sectnum].extra++;
			if (sector[trob[i].sectnum].extra > 4)
			{
				sector[trob[i].sectnum].floorshade = lightend[trob[i].sectnum];	
				sector[trob[i].sectnum].ceilingshade = lightend[trob[i].sectnum];
				startwall = sector[trob[i].sectnum].wallptr;
				endwall = startwall + sector[trob[i].sectnum].wallnum;
				for(k=startwall;k<endwall;k++)
				{
					wall[k].shade = lightend[trob[i].sectnum];
				}
			}	
			if (sector[trob[i].sectnum].extra > 12)
			{
				sector[trob[i].sectnum].floorshade = lightstart[trob[i].sectnum];	
				sector[trob[i].sectnum].ceilingshade = lightstart[trob[i].sectnum];
				startwall = sector[trob[i].sectnum].wallptr;
				endwall = startwall + sector[trob[i].sectnum].wallnum;
				for(k=startwall;k<endwall;k++)
				{
					wall[k].shade = lightstart[trob[i].sectnum];
				}
			}
			if (sector[trob[i].sectnum].extra > krand())
				sector[trob[i].sectnum].extra = 0;
			break;
			case 25:
			if (trob[i].direction == 0)
			{
				sector[trob[i].sectnum].extra++;
				if (sector[trob[i].sectnum].extra > lightend[trob[i].sectnum]) trob[i].direction = 1;
				startwall = sector[trob[i].sectnum].wallptr;
				endwall = startwall + sector[trob[i].sectnum].wallnum;
				for(k=startwall;k<endwall;k++)
				{
					wall[k].shade = sector[trob[i].sectnum].extra;
				}
				sector[trob[i].sectnum].floorshade = sector[trob[i].sectnum].extra;
				sector[trob[i].sectnum].ceilingshade = sector[trob[i].sectnum].extra;
			}
			else
			{
				sector[trob[i].sectnum].extra--;
				if (sector[trob[i].sectnum].extra < lightstart[trob[i].sectnum]) trob[i].direction = 0;
				startwall = sector[trob[i].sectnum].wallptr;
				endwall = startwall + sector[trob[i].sectnum].wallnum;
				for(k=startwall;k<endwall;k++)
				{
					wall[k].shade = sector[trob[i].sectnum].extra;
				}
				sector[trob[i].sectnum].floorshade = sector[trob[i].sectnum].extra;
				sector[trob[i].sectnum].ceilingshade = sector[trob[i].sectnum].extra;
			}
			break;
			case 26:
			sector[trob[i].sectnum].floorstat |= 2;
			if (sector[trob[i].sectnum].floorheinum >= sector[trob[i].sectnum].hitag)
				sector[trob[i].sectnum].floorheinum -= (1<<8); 
			else
				sector[trob[i].sectnum].floorheinum += (1<<8);			
			if (!(ticks&7))
				playxysfx(SfxSlabMove, i, trob[i].centx, trob[i].centy);
			if (sector[trob[i].sectnum].floorheinum == sector[trob[i].sectnum].hitag)
			{
				killsfx(i);
				playxysfx(SfxElevatorReach, i, trob[i].centx, trob[i].centy);
				removetrob(i);
			}
			break;
			case 48:
			if (!--sector[trob[i].sectnum].extra)
			{
				playxysfx(SfxDoor, i, trob[i].centx, trob[i].centy);
				trob[i].type = 111;
				trob[i].direction = 1;
			}
			break;
			case 49:
			if (!--sector[trob[i].sectnum].extra)
			{
				trob[i].direction = 1;
				playxysfx(SfxElevatorStart, i, trob[i].centx, trob[i].centy);
				if (sector[trob[i].sectnum].hitag == 0) trob[i].type = 100;
				else trob[i].type = 101;
			}
			break;
			case 50:
			if (!--sector[trob[i].sectnum].extra)
			{
				playxysfx(SfxDoor, i, trob[i].centx, trob[i].centy);
				trob[i].type = 113;
				trob[i].direction = 1;
			}
			break;
			case 51:
			if (!--sector[trob[i].sectnum].extra)
			{
				playxysfx(SfxDoor, i, trob[i].centx, trob[i].centy);
				trob[i].type = 115;
				trob[i].direction = 1;
			}
			break;
			case 52:
			if (!--sector[trob[i].sectnum].extra)
			{
				playxysfx(SfxDoor, i, trob[i].centx, trob[i].centy);
				trob[i].type = 112;
			}
			break;
			case 63:
			for(j=0;j<crushcnt;j++)
			{
				if (crushsector[j] == trob[i].sectnum)
				{
					if (trob[i].direction == 0)
					{
						k = sector[trob[i].sectnum].floorz-(8<<8);
						if (!movesectorz(trob[i].sectnum,k,1,1,1))
							trob[i].direction = 1;

						l = headspritesect[trob[i].sectnum];
						while (l != -1)
						{
							templong = getceilzofslope(trob[i].sectnum,sprite[l].x,sprite[l].y);
							templong += ((tilesizy[sprite[l].picnum]*sprite[l].yrepeat)<<2);
							if (sprite[l].cstat&128) templong += ((tilesizy[sprite[l].picnum]*sprite[l].yrepeat)<<1);
							for(m=0;m<nummobs;m++)
							{
								if (mob[m].spritenum == l && sprite[l].z <= templong
								    && mob[m].aistate != AIDeath)
									hurtmob(m, 1000, -1);
							}
							l = nextspritesect[l];
						}
					}
					else
					{
						k = crusholdz[j];
						if (!movesectorz(trob[i].sectnum,k,1,0,1))
							trob[i].direction = 0;
					}
					if (!(ticks&7))
						playxysfx(SfxSlabMove, i, trob[i].centx, trob[i].centy);
				}
			}
			break;
			case 64:
			for(j=0;j<crushcnt;j++)
			{
				if (crushsector[j] == trob[i].sectnum)
				{
					if (trob[i].direction == 0)
					{
						k = sector[trob[i].sectnum].floorz-(8<<8);
						if (!movesectorz(trob[i].sectnum,k,4,1,1))
							trob[i].direction = 1;

						l = headspritesect[trob[i].sectnum];
						while (l != -1)
						{
							templong = getceilzofslope(trob[i].sectnum,sprite[l].x,sprite[l].y);
							templong += ((tilesizy[sprite[l].picnum]*sprite[l].yrepeat)<<2);
							if (sprite[l].cstat&128) templong += ((tilesizy[sprite[l].picnum]*sprite[l].yrepeat)<<1);
							for(m=0;m<nummobs;m++)
							{
								if (mob[m].spritenum == l && sprite[l].z <= templong
								    && mob[m].aistate != AIDeath)
									hurtmob(m, 1000, -1);
							}
							l = nextspritesect[l];
						}
					}
					else
					{
						k = crusholdz[j];
						if (!movesectorz(trob[i].sectnum,k,4,0,1))
							trob[i].direction = 0;
					}
					if (!(ticks&7))
						playxysfx(SfxSlabMove, i, trob[i].centx, trob[i].centy);
				}
			}
			break;
			case 66:
			if (trob[i].direction == 0)
			{
				k = sector[trob[i].sectnum].ceilingz+(2<<8);
				sector[trob[i].sectnum].extra = 30*5;
				for(j=0;j<gatecnt;j++)
				{
					if (gatetrob[j] == i)
					{
						sprite[gatesprite[j]].z -= (2<<8);
						if (sprite[gatesprite[j]].z == k)
							trob[i].direction = -1;
						break;
					}
				}
			}
			else if (trob[i].direction == 1)
			{
				k = sector[trob[i].sectnum].floorz;
				for(j=0;j<gatecnt;j++)
				{
					if (gatetrob[j] == i)
					{
						sprite[gatesprite[j]].z += (2<<8);
						if (sprite[gatesprite[j]].z == k)
						{
							removetrob(i);
							gatetrob[j] = -1;
						}
						break;
					}
				}
			}
			else
			{
				if (!--sector[trob[i].sectnum].extra)
					trob[i].direction = 1;
			}
			break;
			case 100:
			if (trob[i].direction == 0)
			{
				k = nextsectorneighborz(trob[i].sectnum,sector[trob[i].sectnum].floorz,1,0);
				if (!movesectorz(trob[i].sectnum,sector[k].floorz,4,0,0))
				{
					playxysfx(SfxElevatorReach, i, trob[i].centx, trob[i].centy);
					killsfx(i);
					trob[i].type = 49;
				}
			}
			else {
				for(j=0;j<MAXLIFTS;j++)
				{
					if (liftsector[j] == trob[i].sectnum)
						k = liftoldz[j];
				}
				if (!movesectorz(trob[i].sectnum,k,4,1,0))
				{
					for (j=0;j<MAXLIFTS;j++)
					{
						if (lifttrob[j] == i)
						{
							playxysfx(SfxElevatorReach, i, trob[i].centx, trob[i].centy);
							removelift(j);
							break;
						}
					}
				}
			}
			sector[trob[i].sectnum].extra = 30*5;
			break;
			case 101:
			if (trob[i].direction == 0)
			{
				k = nextsectorneighborz(trob[i].sectnum,sector[trob[i].sectnum].floorz,1,1);
				if (!movesectorz(trob[i].sectnum,sector[k].floorz,4,1,0))
				{
					playxysfx(SfxElevatorReach, i, trob[i].centx, trob[i].centy);
					trob[i].type = 49;
				}
			}
			else {
				for(j=0;j<MAXLIFTS;j++)
				{
					if (lifttrob[j] == i)
						k = liftoldz[j];
				}
				if (!movesectorz(trob[i].sectnum,k,4,0,0))
				{
					for (j=0;j<MAXLIFTS;j++)
					{
						if (trob[i].sectnum == liftsector[j])
						{
							playxysfx(SfxElevatorReach, i, trob[i].centx, trob[i].centy);
							removelift(j);
							break;
						}
					}
				}
			}
			sector[trob[i].sectnum].extra = 30*5;
			break;
			case 102:
			if (trob[i].direction == 0)
				k = nextsectorneighborz(trob[i].sectnum,sector[trob[i].sectnum].floorz,1,0);
			else
				k = nextsectorneighborz(trob[i].sectnum,sector[trob[i].sectnum].floorz,1,1);
			if (!(ticks&7))
				playxysfx(SfxSlabMove, i, trob[i].centx, trob[i].centy);
			if (!movesectorz(trob[i].sectnum,sector[k].floorz,1,trob[i].direction,0))
			{
				for (j=0;j<MAXLIFTS;j++)
				{
					if (lifttrob[j] == i)
					{
						killsfx(i);
						playxysfx(SfxElevatorReach, i, trob[i].centx, trob[i].centy);
						removelift(j);
						break;
					}
				}
			}
			break;
			case 103:
			if (trob[i].direction == 0)
				k = nextsectorneighborz(trob[i].sectnum,sector[trob[i].sectnum].floorz,1,0);
			else
				k = nextsectorneighborz(trob[i].sectnum,sector[trob[i].sectnum].floorz,1,1);
			if (!(ticks&7))
				playxysfx(SfxSlabMove, i, trob[i].centx, trob[i].centy);
			if (!movesectorz(trob[i].sectnum,sector[k].floorz-(8<<8),4,trob[i].direction,0))
			{
				for (j=0;j<MAXLIFTS;j++)
				{
					if (lifttrob[j] == i)
					{
						killsfx(i);
						playxysfx(SfxElevatorReach, i, trob[i].centx, trob[i].centy);
						removelift(j);
						break;
					}
				}
			}
			break;
			case 104:
			if (trob[i].direction == 0)
				k = nextsectorneighborz(trob[i].sectnum,sector[trob[i].sectnum].floorz,1,0);
			else
				k = nextsectorneighborz(trob[i].sectnum,sector[trob[i].sectnum].floorz,1,1);
			if (!(ticks&7))
				playxysfx(SfxSlabMove, i, trob[i].centx, trob[i].centy);
			if (!movesectorz(trob[i].sectnum,sector[k].floorz,1,trob[i].direction,0))
			{
				for (j=0;j<MAXLIFTS;j++)
				{
					if (lifttrob[j] == i)
					{
						killsfx(i);
						playxysfx(SfxElevatorReach, i, trob[i].centx, trob[i].centy);
						sector[trob[i].sectnum].floorpicnum = sector[k].floorpicnum;
						removelift(j);
						break;
					}
				}
			}
			break;
			case 109:
			for (j=0;j<MAXLIFTS;j++)
			{
				if (lifttrob[j] == i)
				{
					k = liftoldz[j];
					break;
				}
			}
			if (!(ticks&7))
				playxysfx(SfxSlabMove, i, trob[i].centx, trob[i].centy);
			if (!movesectorz(trob[i].sectnum,k-(128<<8),1,0,0))
			{
				playxysfx(SfxElevatorReach, i, trob[i].centx, trob[i].centy);
				killsfx(i);
				removelift(j);
			}
			break;
			case 111:
			if (trob[i].direction == 0)
			{
				if (!movesectorz(trob[i].sectnum,
						sector[nextsectorneighborz(trob[i].sectnum,sector[trob[i].sectnum].floorz,-1,-1)].ceilingz+(8<<8),
						2,0,1))
					trob[i].type = 48;
			}
			else {
				if (!movesectorz(trob[i].sectnum,sector[trob[i].sectnum].floorz,2,1,1))
					removetrob(i);
			}
			sector[trob[i].sectnum].extra = 30*5;
			break;
			case 112:
			if (!movesectorz(trob[i].sectnum,
				sector[nextsectorneighborz(trob[i].sectnum,sector[trob[i].sectnum].floorz,-1,-1)].ceilingz+(8<<8),
				2,0,1))
				removetrob(i);
			break;
			case 113:
			if (trob[i].direction == 0)
			{
				if (!movesectorz(trob[i].sectnum,
						sector[nextsectorneighborz(trob[i].sectnum,sector[trob[i].sectnum].floorz,-1,-1)].ceilingz+(8<<8),
						8,0,1))
					trob[i].type = 50;
			}
			else {
				if (!movesectorz(trob[i].sectnum,sector[trob[i].sectnum].floorz,8,1,1))
					removetrob(i);
			}
			sector[trob[i].sectnum].extra = 30*5;
			break;
			case 114:
			if (!movesectorz(trob[i].sectnum,
					sector[nextsectorneighborz(trob[i].sectnum,sector[trob[i].sectnum].floorz,-1,-1)].ceilingz+(8<<8),
					8,0,1))
					removetrob(i);
			break;
			case 115:	
			if (trob[i].direction == 0)
			{
				if (!movesectorz(trob[i].sectnum,
						sector[nextsectorneighborz(trob[i].sectnum,sector[trob[i].sectnum].floorz,-1,-1)].ceilingz+(8<<8),
						2,0,1))
				{
					trob[i].type = 51;
				}
				movesectorz(trob[i].sectnum,
					sector[nextsectorneighborz(trob[i].sectnum,sector[trob[i].sectnum].floorz,-1,-1)].floorz+(8<<8),
					2,1,0);
			}
			else {
				j = ((sector[trob[i].sectnum].ceilingz+sector[trob[i].sectnum].floorz)>>1);
				movesectorz(trob[i].sectnum,j,2,0,0);
				if (!movesectorz(trob[i].sectnum,j,2,1,1))
					removetrob(i);
			}
			sector[trob[i].sectnum].extra = 30*5;
			break;
			case 116:
			sector[trob[i].sectnum].extra = 30*5;
			if (!movesectorz(trob[i].sectnum,sector[trob[i].sectnum].floorz,2,1,1))
				trob[i].type = 52;
			break;
			case 200:
			startwall = sector[trob[i].sectnum].wallptr;
			endwall = startwall + sector[trob[i].sectnum].wallnum;
			if (trob[i].direction == 0)
			{
				for(j=0;j<2;j++)
				{
					k = wallfind[trob[i].sectnum][j]-1; if (k < startwall) k = endwall-1;
					dax2 = ((wall[k].x+wall[wall[wallfind[trob[i].sectnum][j]].point2].x)>>1)-wall[wallfind[trob[i].sectnum][j]].x;
					day2 = ((wall[k].y+wall[wall[wallfind[trob[i].sectnum][j]].point2].y)>>1)-wall[wallfind[trob[i].sectnum][j]].y;
					if (dax2 != 0)
					{
						dax2 = wall[wall[wall[wallfind[trob[i].sectnum][j]].point2].point2].x;
						dax2 -= wall[wall[wallfind[trob[i].sectnum][j]].point2].x;
						translatesector(&wall[wallfind[trob[i].sectnum][j]].x,wall[wallfind[trob[i].sectnum][j]].x+dax2);
						translatesector(&wall[k].x,wall[k].x+dax2);
						if (!translatesector(&wall[wall[wallfind[trob[i].sectnum][j]].point2].x,wall[wall[wallfind[trob[i].sectnum][j]].point2].x+dax2))
						{
							removetrob(i);
							break;
						}
					}
					else if (day2 != 0)
					{
						day2 = wall[wall[wall[wallfind[trob[i].sectnum][j]].point2].point2].y;
						day2 -= wall[wall[wallfind[trob[i].sectnum][j]].point2].y;
						translatesector(&wall[wallfind[trob[i].sectnum][j]].y,wall[wallfind[trob[i].sectnum][j]].y+day2);
						translatesector(&wall[k].y,wall[k].y+day2);
						if (!translatesector(&wall[wall[wallfind[trob[i].sectnum][j]].point2].y,wall[wall[wallfind[trob[i].sectnum][j]].point2].y+day2))
						{
							removetrob(i);
							break;
						}
					}
				}
			}
			else
			{
				for (j=0;j<2;j++)
				{
					k = wallfind[trob[i].sectnum][j]-1; if (l < startwall) l = endwall-1;
					dax2 = ((wall[k].x+wall[wall[wallfind[trob[i].sectnum][j]].point2].x)>>1)-wall[wallfind[trob[i].sectnum][j]].x;
					day2 = ((wall[k].y+wall[wall[wallfind[trob[i].sectnum][j]].point2].y)>>1)-wall[wallfind[trob[i].sectnum][j]].y;
					if (dax2 != 0)
					{
						translatesector(&wall[wallfind[trob[i].sectnum][j]].x,trob[i].centx);
						translatesector(&wall[k].x,trob[i].centx+dax2);
						if (!translatesector(&wall[wall[wallfind[trob[i].sectnum][j]].point2].x,trob[i].centx+dax2))
						{
							removetrob(i);
							break;
						}
					}
					else if (day2 != 0)
					{
						translatesector(&wall[wallfind[trob[i].sectnum][j]].y,trob[i].centy);
						translatesector(&wall[k].y,trob[i].centy+day2);
						if (!translatesector(&wall[wall[wallfind[trob[i].sectnum][j]].point2].y,trob[i].centy+day2))
						{
							removetrob(i);
							break;
						}
					}
				}
			}
		break;
		case 201:
		startwall = sector[trob[i].sectnum].wallptr;
		endwall = startwall + sector[trob[i].sectnum].wallnum;
		if (trob[i].direction == 0)
		{
			for(j=0;j<2;j++)
			{
				k = wallfind[trob[i].sectnum][j]-1; if (k < startwall) k = endwall-1;
				dax2 = wall[k].x-wall[wallfind[trob[i].sectnum][j]].x;
				day2 = wall[k].y-wall[wallfind[trob[i].sectnum][j]].y;
				if (dax2 != 0)
				{
					dax2 = wall[wall[wall[wall[wallfind[trob[i].sectnum][j]].point2].point2].point2].x;
					dax2 -= wall[wall[wall[wallfind[trob[i].sectnum][j]].point2].point2].x;
					translatesector(&wall[wallfind[trob[i].sectnum][j]].x,wall[wallfind[trob[i].sectnum][j]].x+dax2);
					if (!translatesector(&wall[k].x,wall[k].x+dax2))
					{
						removetrob(i);
						break;
					}
					translatesector(&wall[wall[wallfind[trob[i].sectnum][j]].point2].x,wall[wall[wallfind[trob[i].sectnum][j]].point2].x+dax2);
					translatesector(&wall[wall[wall[wallfind[trob[i].sectnum][j]].point2].point2].x,wall[wall[wall[wallfind[trob[i].sectnum][j]].point2].point2].x+dax2);
				}
				else if (day2 != 0)
				{
					day2 = wall[wall[wall[wall[wallfind[trob[i].sectnum][j]].point2].point2].point2].y;
					day2 -= wall[wall[wall[wallfind[trob[i].sectnum][j]].point2].point2].y;
					translatesector(&wall[wallfind[trob[i].sectnum][j]].y,wall[wallfind[trob[i].sectnum][j]].y+day2);
					if (!translatesector(&wall[k].y,wall[k].y+day2))
					{
						removetrob(i);
						break;
					}
					translatesector(&wall[wall[wallfind[trob[i].sectnum][j]].point2].y,wall[wall[wallfind[trob[i].sectnum][j]].point2].y+day2);
					translatesector(&wall[wall[wall[wallfind[trob[i].sectnum][j]].point2].point2].y,wall[wall[wall[wallfind[trob[i].sectnum][j]].point2].point2].y+day2);
				}
			}
		}
		else
		{    //door was not closed
			for(j=0;j<2;j++)
			{
				k = wallfind[trob[i].sectnum][j]-1; if (k < startwall) k = endwall-1;
				dax2 = wall[k].x-wall[wallfind[trob[i].sectnum][j]].x;
				day2 = wall[k].y-wall[wallfind[trob[i].sectnum][j]].y;
				if (dax2 != 0)
				{
					translatesector(&wall[wallfind[trob[i].sectnum][j]].x,trob[i].centx);
					if (!translatesector(&wall[k].x,trob[i].centx+dax2))
					{
						removetrob(i);
						break;
					}
					translatesector(&wall[wall[wallfind[trob[i].sectnum][j]].point2].x,trob[i].centx);
					translatesector(&wall[wall[wall[wallfind[trob[i].sectnum][j]].point2].point2].x,trob[i].centx+dax2);
				}
				else if (day2 != 0)
				{
					translatesector(&wall[wallfind[trob[i].sectnum][j]].y,trob[i].centy);
					if (!translatesector(&wall[k].y,trob[i].centy+day2))
					{
						removetrob(i);
						break;
					}
					translatesector(&wall[wall[wallfind[trob[i].sectnum][j]].point2].y,trob[i].centy);
					translatesector(&wall[wall[wall[wallfind[trob[i].sectnum][j]].point2].point2].y,trob[i].centy+day2);
				}
			}
		}
		break;
		case 500:
		if (trob[i].direction == 0)
		{
			if (!movesectorz(trob[i].sectnum,
					sector[nextsectorneighborz(trob[i].sectnum,sector[trob[i].sectnum].floorz,-1,-1)].ceilingz+(8<<8),
					2,0,1))
			{
				playxysfx(SfxElevatorReach, i, trob[i].centx, trob[i].centy);
				trob[i].direction = 1;
			}
		}
		else {
			if (!movesectorz(trob[i].sectnum,sector[trob[i].sectnum].floorz,2,1,1))
			{
				playxysfx(SfxElevatorReach, i, trob[i].centx, trob[i].centy);
				trob[i].direction = 0;
			}
		}
		break;
		
		}
	}
}

void spawngibs(Mob_T *mobp, short num)
{
	long i, j, z, floorz;
	short angle, mobtype;

	z = sprite[mobp->spritenum].z;
	floorz = getflorzofslope(sprite[mobp->spritenum].sectnum,
				 sprite[mobp->spritenum].x,
				 sprite[mobp->spritenum].y);
	if (z > (floorz-(8<<8)))
		z = floorz-(8<<8);

	for(j=0;j<num;j++)
	{
		angle = krand()&2047;
		if (j == num / 2) mobtype = MobHeadGiblet;
		else if (j == num / 5) mobtype = MobHandGiblet;
		else mobtype = MobLegGiblet;
		spawnsprite(i,sprite[mobp->spritenum].x+(krand()&255)-128,
			      sprite[mobp->spritenum].y+(krand()&255)-128,
			      z-(krand()&8191),0,0,0,32,64,64,0,0,mobinfo[mobtype].standpicnum,
			      angle,48+(krand()&31),0,-512-(krand()&2047),-1,
			      sprite[mobp->spritenum].sectnum,1,0,0,0,1,mobtype);
	}
}

void spawnsparks(Mob_T *mobp, short num)
{
	long i, j, z, floorz;
	short angle, mobtype;

	z = sprite[mobp->spritenum].z;
	floorz = getflorzofslope(sprite[mobp->spritenum].sectnum,
				 sprite[mobp->spritenum].x,
				 sprite[mobp->spritenum].y);
	if (z > (floorz-(8<<8)))
		z = floorz-(8<<8);

	for(j=0;j<num;j++)
	{
		mobtype = MobFusionSpark;
		spawnsprite(i,sprite[mobp->spritenum].x+(krand()&255)-128,
			      sprite[mobp->spritenum].y+(krand()&255)-128,
			      z-(krand()&8191),0,-32,0,32,64,64,0,0,mobinfo[mobtype].standpicnum,
			      angle,48+(krand()&31),0,-512-(krand()&2047),-1,
			      sprite[mobp->spritenum].sectnum,1,0,0,0,1,mobtype);
	}
}

void spawnfish(Mob_T *mobp, short num)
{
	long i, j, k, z, floorz;
	short angle, mobtype;

	z = sprite[mobp->spritenum].z;
	floorz = getflorzofslope(sprite[mobp->spritenum].sectnum,
				 sprite[mobp->spritenum].x,
				 sprite[mobp->spritenum].y);
	if (z > (floorz-(8<<8)))
		z = floorz-(8<<8);

	for(j=0;j<num;j++)
	{
		mobtype = MobFlyingFish;
		spawnsprite(i,sprite[mobp->spritenum].x+(krand()&255)-128,
			      sprite[mobp->spritenum].y+(krand()&255)-128,
			      z-(krand()&8191),1+256,0,0,32,64,64,0,0,mobinfo[mobtype].standpicnum,
			      angle,48+(krand()&31),0,-512-(krand()&2047),-1,
			      sprite[mobp->spritenum].sectnum,6,0,0,0,1,mobtype);
		k = findmob(i);
		mob[k].target = mobp->target;
	}
}

void droppickupobject(int mobnum, short item)
{
	long i;
	spritetype *pspr;

	pspr = &sprite[mob[mobnum].spritenum];
	getzrange(pspr->x,pspr->y,pspr->z,pspr->sectnum,
		  &mob[mobnum].hiz,&mob[mobnum].hihit,&mob[mobnum].loz,&mob[mobnum].lohit,128L,CLIPMASK0);
	spawnsprite(i,pspr->x,pspr->y,mob[mobnum].loz,0,0,0,32,64,64,0,0,
			item,0,0,0,0,-1,pspr->sectnum,3,0,0,1,0,0);
}

int addmob(short spritenum, int type)
{
	long i;

	if (nummobs >= MAXSPRITES) error("Mobs Overflow", 316);
	
	mob[nummobs].type = type;
	mob[nummobs].xvel = 0;
	mob[nummobs].yvel = 0;
	mob[nummobs].zvel = 0;
	mob[nummobs].seq = -1;
	mob[nummobs].seqpoint.frame = 0;
	mob[nummobs].seqpoint.tics = 0;
	mob[nummobs].spritenum = spritenum;
	mob[nummobs].health = mobinfo[type].starthealth;
	mob[nummobs].movestat = 0;
	mob[nummobs].attacker = -1;
	mob[nummobs].tracer = -1;
	mob[nummobs].aistate = AIWalk;
	sprite[spritenum].picnum = mobinfo[type].standpicnum;
	for(i=0;i<NumBits;i++)
		mob[nummobs].bits[i] = mobinfo[type].bits[i];
	copybufbyte(&sprite[spritenum],&mob[nummobs].origsprite,sizeof(spritetype));
	nummobs++;

	return(nummobs-1);
}

void removemob(int mobnum)
{
	long i;

	killsfx(1024+mobnum);
	nummobs--;
	if (mobnum != nummobs)
	{
		mob[mobnum] = mob[nummobs];
		if (playermob == nummobs) playermob = mobnum;
	}
}

int findmob(short spritenum)
{
	long i;

	for(i=0;i<nummobs;i++)
	{
		if (mob[i].spritenum == spritenum) return(i);
	}
	return(-1);
}

void setmobseq(int mobnum, int seq)
{
	mob[mobnum].seqpoint.frame = 0;
	mob[mobnum].seqpoint.tics = 0;
	mob[mobnum].seq = seq;
	if (mob[mobnum].seq != -1)
		sprite[mob[mobnum].spritenum].picnum = mobseq[mob[mobnum].seq].frame[0].picnum;
}

int healmob(int mobnum, int amount)
{
	if (mob[mobnum].health >= mobinfo[mob[mobnum].type].starthealth)
		return(0);

	mob[mobnum].health += amount;
	if (mob[mobnum].health > mobinfo[mob[mobnum].type].starthealth)
		mob[mobnum].health = mobinfo[mob[mobnum].type].starthealth;
	return(1);
}

void explodeprojectile(int mobnum)
{
	changespritestat(mob[mobnum].spritenum, 7);
	if (mobinfo[mob[mobnum].type].deathseq == -1)
	{
		deletesprite(mob[mobnum].spritenum);
		removemob(mobnum);
		return;
	}
	setmobseq(mobnum, mobinfo[mob[mobnum].type].deathseq);
	if (mobinfo[mob[mobnum].type].deathsfx)
	{
		playxysfx(mobinfo[mob[mobnum].type].deathsfx, 1024+mobnum,
		sprite[mob[mobnum].spritenum].x,
		sprite[mob[mobnum].spritenum].y);
	}
	sprite[mob[mobnum].spritenum].cstat = 0;
}

void killmob(int mobnum, int deathtype)
{
	int sound;

	killsfx(1024+mobnum);
	mob[mobnum].bits[Harmable] = 0;
	mob[mobnum].aistate = AIDeath;
	mob[mobnum].movecount = 0;
	if (mob[mobnum].bits[CountKill] && mob[mobnum].attacker == playersprite) kills++;
	if (mobinfo[mob[mobnum].type].deathseq == -1)
	{
		deletesprite(mob[mobnum].spritenum);
		removemob(mobnum);
		return;
	}
	if (deathtype == 1 && mobinfo[mob[mobnum].type].xdeathseq != -1)
	{
		setmobseq(mobnum, mobinfo[mob[mobnum].type].xdeathseq);
		spawngibs(&mob[mobnum], 10);
	}
	else
		setmobseq(mobnum, mobinfo[mob[mobnum].type].deathseq);
	if (mobinfo[mob[mobnum].type].deathsfx)
	{
		switch (mobinfo[mob[mobnum].type].deathsfx)
		{
			case SfxShawDie:
			if (deathtype == 1 && gamemode == sequel) sound = SfxShawDieHigh;
			else sound = mobinfo[mob[mobnum].type].deathsfx;
			break;
			case SfxBoyDie:
			sound = SfxBoyDie+krand()%2;
			break;
			default:
			sound = mobinfo[mob[mobnum].type].deathsfx;
			break;
		}
		playxysfx(sound, 1024+mobnum,
		sprite[mob[mobnum].spritenum].x,
		sprite[mob[mobnum].spritenum].y);
	}
	sprite[mob[mobnum].spritenum].cstat &= ~1+256;
	if (mob[mobnum].type != MobBaby)
		mob[mobnum].bits[DropOff] = 1;

	// Drop pick-up objects
	switch (mob[mobnum].type)
	{
		case MobBoy:
		if (rnd(75))
			droppickupobject(mobnum, Stone);
		break;
		case MobFat:
		if (rnd(95))
			droppickupobject(mobnum, WandSpr);
		else if (rnd(50))
			droppickupobject(mobnum, Crystal);
		break;
		case MobCargo:
		if (sprite[mob[mobnum].spritenum].hitag)
			droppickupobject(mobnum, sprite[mob[mobnum].spritenum].hitag);
		break;
		case MobGirl:
		if (rnd(90))
			droppickupobject(mobnum, RapidSpr);
		break;
	}
}

void hurtmob(int mobnum, int amount, int attacker)
{
	int xdeath, sound, saved;
	long i;

	if (!mob[mobnum].bits[Harmable]) return; // Should not happen
	if (mob[mobnum].health == 0) return; // It's already dead

	if (mobnum == playermob) // Specific player stuff
	{
		if (armortype)
		{
			if (armortype == 1)
				saved = amount/2;
			else if (armortype == 2)
				saved = amount/3;

			if (armor <= saved)
			{
				// armor is used up
				saved = armor;
				armortype = 0;
			}
			armor -= saved;
			if (armor < 0) armor = 0;
			amount -= saved;
		}
	}	
	mob[mobnum].attacker = attacker;
	if (mob[mobnum].attacker == mob[mobnum].spritenum) // Suicide
		mob[mobnum].attacker = -1;
	mob[mobnum].target = attacker;
	mob[mobnum].health -= amount;
	if (mob[mobnum].spritenum != playersprite && attacker >= 0)
	{
		sprite[mob[mobnum].spritenum].yvel = getangle(sprite[mob[mobnum].spritenum].x-sprite[attacker].x,
						sprite[mob[mobnum].spritenum].y-sprite[attacker].y);
		sprite[mob[mobnum].spritenum].xvel = amount*3;
		if (sprite[mob[mobnum].spritenum].xvel > 127)
			sprite[mob[mobnum].spritenum].xvel = 127;
		mob[mobnum].xvel = ((sprite[mob[mobnum].spritenum].xvel*(long)sintable[(sprite[mob[mobnum].spritenum].yvel+512)&2047])>>3);
		mob[mobnum].yvel = ((sprite[mob[mobnum].spritenum].xvel*(long)sintable[sprite[mob[mobnum].spritenum].yvel&2047])>>3);
	}
	if (mob[mobnum].health <= 0)
	{
		if (mob[mobnum].health < -50) xdeath = 1;
		else xdeath = 0;
		killmob(mobnum, xdeath);
		mob[mobnum].health = 0;
	}
	else
	{
		killsfx(1024+mobnum);
		switch (mobinfo[mob[mobnum].type].hurtsfx)
		{
			case SfxShawPain1:
			case SfxShawPain2:
			case SfxShawPain3:
			case SfxShawPain4:
			sound = SfxShawPain1 + krand()%4;
			break;
			case SfxClementPain1:
			case SfxClementPain2:
			case SfxClementPain3:
			case SfxClementPain4:
			case SfxClementPain5:
			case SfxClementPain6:
			sound = SfxClementPain1 + krand()%6;
			break;
			default:
			sound = mobinfo[mob[mobnum].type].hurtsfx;
			break;
		}
		if (mobinfo[mob[mobnum].type].hurtseq != -1)
		{
			setmobseq(mobnum, mobinfo[mob[mobnum].type].hurtseq);
			mob[mobnum].aistate = AIPain;
		}
		if (sound > 0)
			playxysfx(sound, 1024+mobnum, sprite[mob[mobnum].spritenum].x,
			  sprite[mob[mobnum].spritenum].y);
	}

	// process effects of damage
	if (mobnum == playermob) hurtplayer(amount);

	if (mob[mobnum].type == MobFlyingFish && sprite[mob[mobnum].spritenum].statnum == 4)
	{
		mob[mobnum].xvel = 0;
		mob[mobnum].yvel = 0;
		mob[mobnum].zvel = 0;
		mob[mobnum].movestat = 0;
		changespritestat(mob[mobnum].spritenum, 6);
	}
	if (sprite[mob[mobnum].spritenum].statnum == 2)
		changespritestat(mob[mobnum].spritenum, 6);
}

void nightmarerespawn(int mobnum)
{
	long i, j, k, x, y, z;
	short angle, sectnum;
	short tmob, tflash;

	i = mob[mobnum].spritenum;

	x = mob[mobnum].origsprite.x;
	y = mob[mobnum].origsprite.y;
	z = mob[mobnum].origsprite.z;
	angle = mob[mobnum].origsprite.ang;
	sectnum = mob[mobnum].origsprite.sectnum;

	spawnsprite(tflash,sprite[i].x,sprite[i].y,sprite[i].z-(12<<8),0,-32,0,32,64,64,0,0,mobinfo[MobTeleFlash].standpicnum,
		    0,0,0,0,-1,sprite[i].sectnum,7,0,0,0,0,0);
	tmob = addmob(tflash, MobTeleFlash);
	setmobseq(tmob, mobinfo[MobTeleFlash].walkseq);
	playxysfx(SfxTeleport, tflash+1024, sprite[i].x, sprite[i].y);
	
	spawnsprite(tflash,x,y,z-(12<<8),0,-32,0,32,64,64,0,0,mobinfo[MobTeleFlash].standpicnum,
		    0,0,0,0,-1,sectnum,7,0,0,0,0,0);
	tmob = addmob(tflash, MobTeleFlash);
	setmobseq(tmob, mobinfo[MobTeleFlash].walkseq);
	playxysfx(SfxTeleport, tflash+1024, x, y);

	spawnsprite(j,x,y,z,mob[mobnum].origsprite.cstat,0,0,32,
		mob[mobnum].origsprite.xrepeat,mob[mobnum].origsprite.yrepeat,
		0,0,mobinfo[MobTeleFlash].standpicnum,
		0,0,0,0,-1,sectnum,mob[mobnum].origsprite.statnum,0,0,0,1,mob[mobnum].type);
	sprite[j].clipdist = mob[mobnum].origsprite.clipdist;
	k = findmob(j);
	mob[k].health *= gameskill+1;

	deletesprite((short)i);
	removemob(mobnum);
}

long dist3d(spritetype *s1, spritetype *s2)
{
	long dist;

	dist = (s1->x-s2->x)*(s1->x-s2->x);
	dist += (s1->y-s2->y)*(s1->y-s2->y);
	dist += ((s1->z-s2->z)>>4)*((s1->z-s2->z)>>4);

	return(dist);
}

void spiritexplode(long i)
{
	short mobnum;
	long j, k, l, dist;
	
	mobnum = findmob(i);
	for(j=0;j<MAXSPRITES;j++)
	{
		dist = dist3d(&sprite[j], &sprite[i]);
		if (dist < 3274215)
			if (cansee(sprite[i].x,sprite[i].y,sprite[i].z-(tilesizy[sprite[i].picnum]<<7),sprite[i].sectnum,sprite[j].x,sprite[j].y,sprite[j].z,sprite[j].sectnum) == 1)
			{
				k = ((2048/((dist>>16)+4))>>5);
				for(l=0;l<nummobs;l++)
					if (mob[l].spritenum == j)
					{
						if (mob[l].spritenum == i) continue;
						if (mob[l].spritenum == sprite[i].owner) continue;
						hurtmob(l,k,sprite[i].owner);
					}
			}
	}
}

void explode(long i)
{
	long dax, day, j, k, l, dist;
	short daang;
	int mobnum;

	mobnum = findmob(i);
	sprite[i].shade = -31;
	for(j=0;j<MAXSPRITES;j++)
	{
		dist = dist3d(&sprite[j], &sprite[i]);
		if (dist < 4194305)
			if (cansee(sprite[i].x,sprite[i].y,sprite[i].z-(tilesizy[sprite[i].picnum]<<7),sprite[i].sectnum,sprite[j].x,sprite[j].y,sprite[j].z,sprite[j].sectnum) == 1)
			{
				k = ((32768/((dist>>16)+4))>>5);
				if (j == playersprite && !death)
				{
					daang = getangle(sprite[j].x-sprite[i].x,sprite[j].y-sprite[i].y);
					dax = ((k*sintable[(daang+512)&2047])>>14);
					day = ((k*sintable[daang&2047])>>14);
					sync.vel += ((dax*sintable[(sprite[j].ang+512)&2047]+day*sintable[sprite[j].ang&2047])>>14);
					sync.svel += ((day*sintable[(sprite[j].ang+512)&2047]-dax*sintable[sprite[j].ang&2047])>>14);
				}
				for(l=0;l<nummobs;l++)
					if (mob[l].spritenum == j)
					{
						if (mob[l].spritenum == i) continue;
						if (mob[mobnum].type == MobCargo) sprite[i].owner = mob[mobnum].attacker;
						if (mob[l].type == MobChui && sprite[i].owner == mob[l].spritenum) continue;
						hurtmob(l,k,sprite[i].owner);
					}
			}
	}
}

void fusionexplode(long i)
{
	long dax, day, j, k, l, dist;
	short daang;

	sprite[i].shade = -31;
	j = findmob(i);
	spawnsparks(&mob[j], 10);
	for(j=0;j<MAXSPRITES;j++)
	{
		dist = dist3d(&sprite[j], &sprite[i]);
		if (dist < 4300021)
			if (cansee(sprite[i].x,sprite[i].y,sprite[i].z-(tilesizy[sprite[i].picnum]<<7),sprite[i].sectnum,sprite[j].x,sprite[j].y,sprite[j].z,sprite[j].sectnum) == 1)
			{
				k = ((32768/((dist>>24)+4))>>5);
				for(l=0;l<nummobs;l++)
					if (mob[l].spritenum == j)
					{
						if (mob[l].spritenum == i) continue;
						if (sprite[i].owner == mob[l].spritenum) continue;
						hurtmob(l,k,sprite[i].owner);
					}
			}
	}
	fusionspray();
}

movesprite(short spritenum, int mobnum, long dx, long dy, long dz, long ceildist, long flordist, long clipmask)
{
	long i, daz, zoffs, templong;
	short retval, dasectnum, datempshort;
	spritetype *spr;

	spr = &sprite[spritenum];
	
	if ((spr->cstat&128) == 0)
		zoffs = -((tilesizy[spr->picnum]*spr->yrepeat)<<1);
	else
		zoffs = 0;

	dasectnum = spr->sectnum;  //Can't modify sprite sectors directly becuase of linked lists
	daz = spr->z+zoffs;  //Must do this if not using the new centered centering (of course)
	retval = clipmove(&spr->x,&spr->y,&daz,&dasectnum,dx,dy,
							((long)spr->clipdist)<<2,ceildist,flordist,clipmask);

	if (dasectnum < 0) retval = -1;

	if ((dasectnum != spr->sectnum) && (dasectnum >= 0))
		changespritesect(spritenum,dasectnum);

		//Set the blocking bit to 0 temporarly so getzrange doesn't pick up
		//its own sprite
	datempshort = spr->cstat; spr->cstat &= ~1;
	getzrange(spr->x,spr->y,spr->z-1,spr->sectnum,
				 &mob[mobnum].hiz,&mob[mobnum].hihit,&mob[mobnum].loz,&mob[mobnum].lohit,
				 ((long)spr->clipdist)<<2,clipmask);
	spr->cstat = datempshort;

	daz = spr->z+zoffs + dz;
	if ((daz <= mob[mobnum].hiz) || (daz > mob[mobnum].loz))
	{
		if (retval != 0) return(retval);
		if (daz <= mob[mobnum].hiz) return(16384+dasectnum);
		else return(8192+dasectnum);
	}
	spr->z = daz-zoffs;
	return(retval);
}

void seqtrigger(int mobnum, int trigger)
{
	long i, j, k, dax, day, daz;
	short daang, dasect, spirit;
	spritetype *spr;
	
	i = 0;
	j = 0;
	switch (trigger)
	{
		case 1:
		explode(mob[mobnum].spritenum);
		break;
		case 2:
		deletesprite(mob[mobnum].spritenum);
		removemob(mobnum);
		break;
		case 3:
		dax = (sintable[(sprite[mob[mobnum].spritenum].ang+512)&2047]>>6);
		day = (sintable[sprite[mob[mobnum].spritenum].ang&2047]>>6);
		daz = sprite[mob[mobnum].target].z-(40<<8);
		if (mob[mobnum].target == playersprite && crouching)
			daz += (24<<8);
		spawnsprite(i,sprite[mob[mobnum].spritenum].x,
		sprite[mob[mobnum].spritenum].y,sprite[mob[mobnum].spritenum].z-(40<<8),
		128,0,0,16,64,64,0,0,mobinfo[MobAcid].standpicnum,
		sprite[mob[mobnum].spritenum].ang,dax,day,
		        ((daz+(8<<8)-sprite[i].z)<<8) /
			(ksqrt((sprite[mob[mobnum].target].x-sprite[i].x) *
			(sprite[mob[mobnum].target].x-sprite[i].x) +
			(sprite[mob[mobnum].target].y-sprite[i].y) *
			(sprite[mob[mobnum].target].y-sprite[i].y))+1),
		mob[mobnum].spritenum,sprite[mob[mobnum].spritenum].sectnum,4,0,0,0,1,MobAcid);
		playxysfx(SfxCatAttack, 1024+mobnum, sprite[mob[mobnum].spritenum].x,
						     sprite[mob[mobnum].spritenum].y);
		break;
		case 4:
		daz = ((sprite[mob[mobnum].target].z+(8<<8)-sprite[mob[mobnum].spritenum].z)<<8) /
			(ksqrt((sprite[mob[mobnum].target].x-sprite[mob[mobnum].spritenum].x) *
			(sprite[mob[mobnum].target].x-sprite[mob[mobnum].spritenum].x) +
			(sprite[mob[mobnum].target].y-sprite[mob[mobnum].spritenum].y) *
			(sprite[mob[mobnum].target].y-sprite[mob[mobnum].spritenum].y))+1);
		if (mob[mobnum].target == playersprite && crouching)
			j = (24<<8);
		else
			j = 0;
		firehitscan(sprite[mob[mobnum].spritenum].x,
			    sprite[mob[mobnum].spritenum].y,
			    sprite[mob[mobnum].spritenum].z-(40<<8)+j,
			    sprite[mob[mobnum].spritenum].ang,
			    sprite[mob[mobnum].spritenum].sectnum,
			    daz<<6,7,mob[mobnum].spritenum,FARRANGE);
		playxysfx(SfxCatFire, 1024+mobnum, sprite[mob[mobnum].spritenum].x,
						     sprite[mob[mobnum].spritenum].y);
		break;
		case 5:
		for(i=0;i<3;i++)
		{
			daz = ((sprite[mob[mobnum].target].z+(8<<8)-sprite[mob[mobnum].spritenum].z)<<8) /
				(ksqrt((sprite[mob[mobnum].target].x-sprite[mob[mobnum].spritenum].x) *
				(sprite[mob[mobnum].target].x-sprite[mob[mobnum].spritenum].x) +
				(sprite[mob[mobnum].target].y-sprite[mob[mobnum].spritenum].y) *
				(sprite[mob[mobnum].target].y-sprite[mob[mobnum].spritenum].y))+1);
			if (mob[mobnum].target == playersprite && crouching)
				j = (24<<8);
			else
				j = 0;
			firehitscan(sprite[mob[mobnum].spritenum].x,
			    sprite[mob[mobnum].spritenum].y,
			    sprite[mob[mobnum].spritenum].z-(40<<8)+j,
			    sprite[mob[mobnum].spritenum].ang,
			    sprite[mob[mobnum].spritenum].sectnum,
			    daz<<6,7,mob[mobnum].spritenum,FARRANGE);
		}
		playxysfx(SfxWandFire, 1024+mobnum, sprite[mob[mobnum].spritenum].x,
						     sprite[mob[mobnum].spritenum].y);
		break;
		case 6:
		dax = (sintable[(sprite[mob[mobnum].spritenum].ang+512)&2047]>>6);
		day = (sintable[sprite[mob[mobnum].spritenum].ang&2047]>>6);
		daz = sprite[mob[mobnum].target].z-(40<<8);
		spawnsprite(i,sprite[mob[mobnum].spritenum].x,
		sprite[mob[mobnum].spritenum].y,sprite[mob[mobnum].spritenum].z-(40<<8),
		128,0,0,16,64,64,0,0,mobinfo[KaliBall].standpicnum,
		sprite[mob[mobnum].spritenum].ang,dax,day,
		        ((daz+(8<<8)-sprite[i].z)<<8) /
			(ksqrt((sprite[mob[mobnum].target].x-sprite[i].x) *
			(sprite[mob[mobnum].target].x-sprite[i].x) +
			(sprite[mob[mobnum].target].y-sprite[i].y) *
			(sprite[mob[mobnum].target].y-sprite[i].y))+1),
		mob[mobnum].spritenum,sprite[mob[mobnum].spritenum].sectnum,4,0,0,0,1,MobKaliBall);
		break;
		case 7:
		dax = (sintable[(sprite[mob[mobnum].spritenum].ang+512)&2047]>>6);
		day = (sintable[sprite[mob[mobnum].spritenum].ang&2047]>>6);
		daz = sprite[mob[mobnum].target].z-(40<<8);
		if (mob[mobnum].target == playersprite && crouching)
			daz += (24<<8);
		spawnsprite(i,sprite[mob[mobnum].spritenum].x,
		sprite[mob[mobnum].spritenum].y,sprite[mob[mobnum].spritenum].z-(40<<8),
		128,0,0,16,64,64,0,0,mobinfo[MobCrimson].standpicnum,
		sprite[mob[mobnum].spritenum].ang,dax,day,
		        ((daz+(8<<8)-sprite[i].z)<<8) /
			(ksqrt((sprite[mob[mobnum].target].x-sprite[i].x) *
			(sprite[mob[mobnum].target].x-sprite[i].x) +
			(sprite[mob[mobnum].target].y-sprite[i].y) *
			(sprite[mob[mobnum].target].y-sprite[i].y))+1),
		mob[mobnum].spritenum,sprite[mob[mobnum].spritenum].sectnum,4,0,0,0,1,MobCrimson);
		playxysfx(SfxCatAttack, 1024+mobnum, sprite[mob[mobnum].spritenum].x,
						     sprite[mob[mobnum].spritenum].y);
		break;
		case 8:
		sprite[mob[mobnum].spritenum].ang = getangle(sprite[mob[mobnum].target].x-sprite[mob[mobnum].spritenum].x,
								sprite[mob[mobnum].target].y-sprite[mob[mobnum].spritenum].y);
		dax = (sintable[(sprite[mob[mobnum].spritenum].ang+512)&2047]>>6);
		day = (sintable[sprite[mob[mobnum].spritenum].ang&2047]>>6);
		daz = sprite[mob[mobnum].target].z-(40<<8);
		if (mob[mobnum].target == playersprite && crouching)
			daz += (24<<8);
		spawnsprite(i,sprite[mob[mobnum].spritenum].x,
		sprite[mob[mobnum].spritenum].y,sprite[mob[mobnum].spritenum].z-(40<<8),
		128,0,0,16,64,64,0,0,mobinfo[MobBabyRay].standpicnum,
		sprite[mob[mobnum].spritenum].ang,dax,day,
		        ((daz+(8<<8)-sprite[i].z)<<8) /
			(ksqrt((sprite[mob[mobnum].target].x-sprite[i].x) *
			(sprite[mob[mobnum].target].x-sprite[i].x) +
			(sprite[mob[mobnum].target].y-sprite[i].y) *
			(sprite[mob[mobnum].target].y-sprite[i].y))+1),
		mob[mobnum].spritenum,sprite[mob[mobnum].spritenum].sectnum,4,0,0,0,1,MobBabyRay);
		playxysfx(SfxBabyShoot, 1024+mobnum, sprite[mob[mobnum].spritenum].x,
						     sprite[mob[mobnum].spritenum].y);
		break;
		case 9:
		daz = ((sprite[mob[mobnum].target].z+(8<<8)-sprite[mob[mobnum].spritenum].z)<<8) /
			(ksqrt((sprite[mob[mobnum].target].x-sprite[mob[mobnum].spritenum].x) *
			(sprite[mob[mobnum].target].x-sprite[mob[mobnum].spritenum].x) +
			(sprite[mob[mobnum].target].y-sprite[mob[mobnum].spritenum].y) *
			(sprite[mob[mobnum].target].y-sprite[mob[mobnum].spritenum].y))+1),
		firehitscan(sprite[mob[mobnum].spritenum].x,
			    sprite[mob[mobnum].spritenum].y,
			    sprite[mob[mobnum].spritenum].z-(40<<8),
			    sprite[mob[mobnum].spritenum].ang,
			    sprite[mob[mobnum].spritenum].sectnum,
			    daz<<6,15,mob[mobnum].spritenum,CLOSERANGE);
		break;
		case 10:
		fusionexplode(mob[mobnum].spritenum);
		break;
		case 11:
		spr = &sprite[mob[mobnum].spritenum];
		spr->ang = getangle(sprite[mob[mobnum].target].x-sprite[mob[mobnum].spritenum].x,
						sprite[mob[mobnum].target].y-sprite[mob[mobnum].spritenum].y);
		daz = sprite[mob[mobnum].target].z;
		spr->xvel = (sintable[(sprite[mob[mobnum].spritenum].ang+512)&2047]>>6);
		spr->yvel = (sintable[sprite[mob[mobnum].spritenum].ang&2047]>>6);
		spr->zvel = ((daz-sprite[mob[mobnum].spritenum].z)<<8) /
			(ksqrt((sprite[mob[mobnum].target].x-sprite[mob[mobnum].spritenum].x) *
			(sprite[mob[mobnum].target].x-sprite[mob[mobnum].spritenum].x) +
			(sprite[mob[mobnum].target].y-sprite[mob[mobnum].spritenum].y) *
			(sprite[mob[mobnum].target].y-sprite[mob[mobnum].spritenum].y))+1),
		mob[mobnum].movestat = 0L;
		changespritestat(mob[mobnum].spritenum, 4);
		break;
		case 12:
		dax = (sintable[(sprite[mob[mobnum].spritenum].ang+512)&2047]>>6);
		day = (sintable[sprite[mob[mobnum].spritenum].ang&2047]>>6);
		daz = sprite[mob[mobnum].target].z-(40<<8);
		if (mob[mobnum].target == playersprite && crouching)
			daz += (24<<8);
		spawnsprite(i,sprite[mob[mobnum].spritenum].x,
		sprite[mob[mobnum].spritenum].y,sprite[mob[mobnum].spritenum].z-(40<<8),
		128,0,4,16,64,64,0,0,mobinfo[MobChuiBall].standpicnum,
		sprite[mob[mobnum].spritenum].ang,dax,day,
		        ((daz+(8<<8)-sprite[i].z)<<8) /
			(ksqrt((sprite[mob[mobnum].target].x-sprite[i].x) *
			(sprite[mob[mobnum].target].x-sprite[i].x) +
			(sprite[mob[mobnum].target].y-sprite[i].y) *
			(sprite[mob[mobnum].target].y-sprite[i].y))+1),
		mob[mobnum].spritenum,sprite[mob[mobnum].spritenum].sectnum,4,0,0,0,1,MobChuiBall);
		break;
		case 13:
		// scan the mobs to see if all the bosses are dead.
		for(i=0;i<nummobs;i++)
		{
			if (mob[i].type == mob[mobnum].type && mob[i].health > 0
				&& i != mobnum)
				return; // there is one alive
		}
		if (mob[mobnum].type == MobFatuma && gamelevel == 7)
		{
			for(i=0;i<numsectors;i++)
			{
				if (sector[i].lotag == 778)
					addtrob(26, i);
			}
		}
		if (mob[mobnum].type == MobKinene && gamelevel == 6)
		{
			for(i=0;i<numsectors;i++)
			{
				if (sector[i].lotag == 777)
				{
					sector[i].lotag = 0;
					addlift(i, 102, sector[i].hitag);
				}
			}
		}
		if (gamelevel == 9 || gamelevel == 32)
		{
			for(i=0;i<numsectors;i++)
			{
				if (sector[i].lotag == 777)
					addtrob(112, i);
			}
		}
		if (gamelevel == 8 && gamemode != sequel)
		{
			if (gameepisode == 5 && mob[mobnum].type == MobClement)
			{
				for(i=0;i<numsectors;i++)
				{
					if (sector[i].lotag == 888)
					{
						sector[i].lotag = 0;
						j = addtrob(114, i);
						playxysfx(SfxDoor, 3, trob[j].centx, trob[j].centy);
					}
				}
			}
			else
				exitlevel();
		}
		break;
		case 14:
		// scan the remaining mobs
		// to see if all of the ducks are gone.
		for(i=0;i<nummobs;i++)
		{
			if (mob[i].type != mob[mobnum].type) continue;
			if (i != mobnum && mob[i].health > 0
				&& sprite[mob[mobnum].spritenum].hitag == sprite[mob[i].spritenum].hitag)
			{
				deletesprite(mob[mobnum].spritenum);
				removemob(mobnum);
				return;
			}
		}
		for(i=0;i<numsectors;i++)
		{
			if (sprite[mob[mobnum].spritenum].hitag == sector[i].lotag)
				addlift(i, 102, sector[i].hitag);
		}
		deletesprite(mob[mobnum].spritenum);
		removemob(mobnum);
		break;
		case 15:
		sprite[mob[mobnum].spritenum].ang = getangle(sprite[mob[mobnum].target].x-sprite[mob[mobnum].spritenum].x,sprite[mob[mobnum].target].y-sprite[mob[mobnum].spritenum].y);
		if ((cansee(posx,posy,posz,cursectnum,sprite[mob[mobnum].spritenum].x,
			sprite[mob[mobnum].spritenum].y,sprite[mob[mobnum].spritenum].z-(tilesizy[sprite[mob[mobnum].spritenum].picnum]<<7),
			sprite[mob[mobnum].spritenum].sectnum) == 1)
			&& (mob[playermob].health) && (!powerup[Invisibility]))
		{
			mob[mobnum].seqpoint.tics = 0;
			mob[mobnum].seqpoint.frame = 0;
		}
		else
		{
			mob[mobnum].aistate = AIWalk;
			setmobseq(mobnum, mobinfo[mob[mobnum].type].walkseq);
		}
		break;
		case 16:
		sprite[mob[mobnum].spritenum].xrepeat = 64;
		sprite[mob[mobnum].spritenum].yrepeat = 64;
		if ((klabs(sprite[mob[mobnum].spritenum].x-sprite[mob[mobnum].target].x)+
			klabs(sprite[mob[mobnum].spritenum].y-sprite[mob[mobnum].target].y)) < 512)
		{			
			i = findmob(mob[mobnum].target);
			hurtmob(i, 30, mobnum);
			playxysfx(SfxKineneSplat, 1024+mobnum,
			sprite[mob[mobnum].spritenum].x,
			sprite[mob[mobnum].spritenum].y);
		}
		break;
		case 17:
		if (((klabs(sprite[mob[mobnum].spritenum].x-sprite[mob[mobnum].target].x)+
			klabs(sprite[mob[mobnum].spritenum].y-sprite[mob[mobnum].target].y)) < 512)
			&& mob[mobnum].target == playersprite)
		{
			sprite[mob[mobnum].spritenum].xrepeat = 1;
			sprite[mob[mobnum].spritenum].yrepeat = 1;
		}
		else
		{
			mob[mobnum].aistate = AIWalk;
			setmobseq(mobnum, mobinfo[mob[mobnum].type].walkseq);
		}
		break;
		case 18:
		playxysfx(SfxKineneFoot, 1024+mobnum,
		sprite[mob[mobnum].spritenum].x,
		sprite[mob[mobnum].spritenum].y);
		break;
		case 19:
		j = 0;
		// see how many flying fish are in the level
		for(i=0;i<nummobs;i++)
		{
			if (mob[i].type == FlyingFish && mob[i].aistate != AIDeath) j++;
		}
		// If there are less than 30 flying fish then
		// spit some.
		if (j < 30)
		{
			dax = (sintable[(sprite[mob[mobnum].spritenum].ang+512)&2047]>>6);
			day = (sintable[sprite[mob[mobnum].spritenum].ang&2047]>>6);
			daz = sprite[mob[mobnum].target].z;
			spawnsprite(i,sprite[mob[mobnum].spritenum].x,
			sprite[mob[mobnum].spritenum].y,sprite[mob[mobnum].spritenum].z-(40<<8),
			1+256,0,0,16,64,64,0,0,mobinfo[MobFlyingFish].standpicnum,
			sprite[mob[mobnum].spritenum].ang,dax,day,
		 		((daz+(8<<8)-sprite[i].z)<<8) /
				(ksqrt((sprite[mob[mobnum].target].x-sprite[i].x) *
				(sprite[mob[mobnum].target].x-sprite[i].x) +
				(sprite[mob[mobnum].target].y-sprite[i].y) *
				(sprite[mob[mobnum].target].y-sprite[i].y))+1),
			mob[mobnum].spritenum,sprite[mob[mobnum].spritenum].sectnum,6,0,0,0,1,MobFlyingFish);
			j = findmob(i);
			mob[j].target = mob[mobnum].target;
		}
		break;
		case 20:
		spawnfish(&mob[mobnum], 5);
		deletesprite(mob[mobnum].spritenum);
		removemob(mobnum);
		break;
		case 21:
		daz = ((sprite[mob[mobnum].target].z+(8<<8)-sprite[mob[mobnum].spritenum].z)<<8) /
			(ksqrt((sprite[mob[mobnum].target].x-sprite[mob[mobnum].spritenum].x) *
			(sprite[mob[mobnum].target].x-sprite[mob[mobnum].spritenum].x) +
			(sprite[mob[mobnum].target].y-sprite[mob[mobnum].spritenum].y) *
			(sprite[mob[mobnum].target].y-sprite[mob[mobnum].spritenum].y))+1),
		firehitscan(sprite[mob[mobnum].spritenum].x,
			    sprite[mob[mobnum].spritenum].y,
			    sprite[mob[mobnum].spritenum].z-(40<<8),
			    sprite[mob[mobnum].spritenum].ang,
			    sprite[mob[mobnum].spritenum].sectnum,
			    daz<<6,15,mob[mobnum].spritenum,CLOSERANGE);
		break;
		case 22:
		dax = (sintable[(sprite[mob[mobnum].spritenum].ang+512)&2047]>>6);
		day = (sintable[sprite[mob[mobnum].spritenum].ang&2047]>>6);
		daz = sprite[mob[mobnum].target].z-(40<<8);
		if (mob[mobnum].target == playersprite && crouching)
			daz += (24<<8);
		spawnsprite(i,sprite[mob[mobnum].spritenum].x,
		sprite[mob[mobnum].spritenum].y,sprite[mob[mobnum].spritenum].z-(40<<8),
		128,0,0,16,64,64,0,0,mobinfo[MobFireball].standpicnum,
		sprite[mob[mobnum].spritenum].ang,dax,day,
		        ((daz+(8<<8)-sprite[i].z)<<8) /
			(ksqrt((sprite[mob[mobnum].target].x-sprite[i].x) *
			(sprite[mob[mobnum].target].x-sprite[i].x) +
			(sprite[mob[mobnum].target].y-sprite[i].y) *
			(sprite[mob[mobnum].target].y-sprite[i].y))+1),
		mob[mobnum].spritenum,sprite[mob[mobnum].spritenum].sectnum,4,0,0,0,1,MobFireball);
		playxysfx(SfxCameraGun, 1024+mobnum, sprite[mob[mobnum].spritenum].x,
						     sprite[mob[mobnum].spritenum].y);
		break;
		case 23:
		for(i=0;i<nummobs;i++)
		{
			if (mob[i].type == MobBossSpit) return;
		}
		k = krand()%spawnspotcnt;
		daang = getangle(spawnx[k]-sprite[mob[mobnum].spritenum].x,
				spawny[k]-sprite[mob[mobnum].spritenum].y);
		dax = (sintable[(daang+512)&2047]>>5);
		day = (sintable[daang&2047]>>5);
		daz = sprite[spawnsprite[k]].z-(40<<8);
		spawnsprite(i,sprite[mob[mobnum].spritenum].x,
		sprite[mob[mobnum].spritenum].y+(1<<8),sprite[mob[mobnum].spritenum].z-(40<<8),
		128,0,0,16,64,64,0,0,mobinfo[MobBossSpit].standpicnum,
		sprite[mob[mobnum].spritenum].ang,dax,day,
		        ((daz+(8<<8)-sprite[i].z)<<8) /
			(ksqrt((spawnx[k]-sprite[i].x) *
			(spawnx[k]-sprite[i].x) +
			(spawny[k]-sprite[i].y) *
			(spawny[k]-sprite[i].y))+1),
		mob[mobnum].spritenum,sprite[mob[mobnum].spritenum].sectnum,10,0,0,0,0,0);
		j = addmob(i, MobBossSpit);
		mob[j].target = spawnsprite[k];
		playxysfx(SfxBossSpit, 1024+mobnum, sprite[mob[mobnum].spritenum].x,
						     sprite[mob[mobnum].spritenum].y);
		break;
		case 24:
		daz = ((sprite[mob[mobnum].target].z+(8<<8)-sprite[mob[mobnum].spritenum].z)<<8) /
			(ksqrt((sprite[mob[mobnum].target].x-sprite[mob[mobnum].spritenum].x) *
			(sprite[mob[mobnum].target].x-sprite[mob[mobnum].spritenum].x) +
			(sprite[mob[mobnum].target].y-sprite[mob[mobnum].spritenum].y) *
			(sprite[mob[mobnum].target].y-sprite[mob[mobnum].spritenum].y))+1),
		firehitscan(sprite[mob[mobnum].spritenum].x,
			    sprite[mob[mobnum].spritenum].y,
			    sprite[mob[mobnum].spritenum].z-(40<<8),
			    sprite[mob[mobnum].spritenum].ang,
			    sprite[mob[mobnum].spritenum].sectnum,
			    daz<<6,25,mob[mobnum].spritenum,CLOSERANGE);
		playxysfx(SfxPunch, 1024+mobnum, sprite[mob[mobnum].spritenum].x,
						 sprite[mob[mobnum].spritenum].y);
		break;
		case 25:
		for(dax=sprite[mob[mobnum].spritenum].x-(7<<8);dax<sprite[mob[mobnum].spritenum].x+(9<<8);dax+=(1<<8))
		{
			day = sprite[mob[mobnum].spritenum].y+(7<<8);
			daz = sprite[mob[mobnum].spritenum].z-(krand()>>8)-(54<<8);
			updatesector(dax,day,&dasect);
			spawnsprite(i,dax,day,daz,0,-32,0,32,64,64,0,0,
				mobinfo[MobBrainExplode].standpicnum,0,0,0,0,
				-1,dasect,0,0,0,1,1,MobBrainExplode);
			j = findmob(i);
			mob[j].zvel = -(krand()>>7);
			setmobseq(j, mobinfo[MobBrainExplode].deathseq);
		}
		break;
		case 26:
		sprite[mob[mobnum].spritenum].ang = getangle(sprite[mob[mobnum].target].x-sprite[mob[mobnum].spritenum].x,
							sprite[mob[mobnum].target].y-sprite[mob[mobnum].spritenum].y);
		break;
		case 27:
		sprite[mob[mobnum].spritenum].ang = getangle(sprite[mob[mobnum].target].x-sprite[mob[mobnum].spritenum].x,
								sprite[mob[mobnum].target].y-sprite[mob[mobnum].spritenum].y);
		dax = (sintable[(sprite[mob[mobnum].spritenum].ang+512)&2047]>>6);
		day = (sintable[sprite[mob[mobnum].spritenum].ang&2047]>>6);
		daz = sprite[mob[mobnum].target].z-(38<<8);
		if (mob[mobnum].target == playersprite && crouching)
			daz += (24<<8);
		spawnsprite(i,sprite[mob[mobnum].spritenum].x,
		sprite[mob[mobnum].spritenum].y,sprite[mob[mobnum].spritenum].z-(40<<8),
		128,0,0,16,64,64,0,0,mobinfo[MobBanFireball].standpicnum,
		sprite[mob[mobnum].spritenum].ang,dax,day,
		        ((daz+(8<<8)-sprite[i].z)<<8) /
			(ksqrt((sprite[mob[mobnum].target].x-sprite[i].x) *
			(sprite[mob[mobnum].target].x-sprite[i].x) +
			(sprite[mob[mobnum].target].y-sprite[i].y) *
			(sprite[mob[mobnum].target].y-sprite[i].y))+1),
		mob[mobnum].spritenum,sprite[mob[mobnum].spritenum].sectnum,4,0,0,0,1,MobBanFireball);
		playxysfx(SfxCatAttack, 1024+mobnum, sprite[mob[mobnum].spritenum].x,
						     sprite[mob[mobnum].spritenum].y);
		break;
		case 28:
		// scan the remaining mobs
		// to see if all of the Karibas are dead.
		for(i=0;i<nummobs;i++)
		{
			if (mob[i].type != mob[mobnum].type) continue;
			if (i != mobnum && mob[i].health > 0)
				return;
		}
		for(i=0;i<numsectors;i++)
		{
			if (sector[i].lotag == 888)
			{
				sector[i].lotag = 0;
				j = addtrob(114, i);
				playxysfx(SfxDoor, 3, trob[j].centx, trob[j].centy);
			}
		}
		break;
		case 29:
		dax = (sintable[(sprite[mob[mobnum].spritenum].ang+512)&2047]>>6);
		day = (sintable[sprite[mob[mobnum].spritenum].ang&2047]>>6);
		daz = sprite[mob[mobnum].target].z-(40<<8);
		if (mob[mobnum].target == playersprite && crouching)
			daz += (24<<8);
		spawnsprite(i,sprite[mob[mobnum].spritenum].x,
		sprite[mob[mobnum].spritenum].y,sprite[mob[mobnum].spritenum].z-(40<<8),
		128,0,0,16,24,24,0,0,mobinfo[MobTracer].standpicnum,
		sprite[mob[mobnum].spritenum].ang,dax,day,
		        ((daz+(8<<8)-sprite[i].z)<<8) /
			(ksqrt((sprite[mob[mobnum].target].x-sprite[i].x) *
			(sprite[mob[mobnum].target].x-sprite[i].x) +
			(sprite[mob[mobnum].target].y-sprite[i].y) *
			(sprite[mob[mobnum].target].y-sprite[i].y))+1),
		mob[mobnum].spritenum,sprite[mob[mobnum].spritenum].sectnum,4,0,0,0,1,MobTracer);
		j = findmob(i);
		if (rnd(192)) mob[j].tracer = mob[mobnum].target;
		playxysfx(SfxOgreFire, 1024+mobnum, sprite[mob[mobnum].spritenum].x,
						     sprite[mob[mobnum].spritenum].y);
		break;
		case 30:
		j = mob[mobnum].target;
		spawnsprite(i,sprite[j].x,sprite[j].y,sprite[j].z-(32<<8),0,-32,0,32,64,64,0,0,mobinfo[MobZombieSpirit].standpicnum,
		 	   sprite[j].ang,0,0,0,mob[mobnum].spritenum,sprite[j].sectnum,7,0,0,0,1,MobZombieSpirit);
		k = findmob(i);
		mob[k].target = mob[mobnum].target;
		mob[k].tracer = mob[mobnum].spritenum;
		mob[mobnum].tracer = i;
		setmobseq(k, mobinfo[mob[k].type].walkseq);
		break;
		case 36:
		playxysfx(SfxFlame, 1024+mobnum, sprite[mob[mobnum].spritenum].x,
						sprite[mob[mobnum].spritenum].y);
		goto grip;
		case 37:
		playxysfx(SfxFlameStart, 1024+mobnum, sprite[mob[mobnum].spritenum].x,
						sprite[mob[mobnum].spritenum].y);
		case 31:
		grip:
		if (mob[mobnum].tracer < 0) return;
		i = findmob(mob[mobnum].target);
		// Don't move it if the zombie lost sight
		if ((cansee(sprite[mob[mobnum].target].x,
			    sprite[mob[mobnum].target].y,
			    sprite[mob[mobnum].target].z,
			    sprite[mob[mobnum].target].sectnum,sprite[mob[mobnum].tracer].x,
			    sprite[mob[mobnum].tracer].y,
			    sprite[mob[mobnum].tracer].z-(tilesizy[sprite[mob[mobnum].tracer].picnum]<<7),
			    sprite[mob[mobnum].tracer].sectnum) != 1)
			    || (!mob[i].health))
				return;
		dax = (100*sintable[(sprite[mob[mobnum].target].ang+512)&2047]>>14);
		day = (100*sintable[sprite[mob[mobnum].target].ang&2047]>>14);
		setsprite(mob[mobnum].spritenum,sprite[mob[mobnum].target].x+dax,
			sprite[mob[mobnum].target].y+day,
			sprite[mob[mobnum].target].z-(32<<8));
		break;
		case 32:
		i = findmob(mob[mobnum].target);
		if ((cansee(sprite[mob[mobnum].target].x,
			    sprite[mob[mobnum].target].y,
			    sprite[mob[mobnum].target].z,
			    sprite[mob[mobnum].target].sectnum,sprite[mob[mobnum].spritenum].x,
			    sprite[mob[mobnum].spritenum].y,
			    sprite[mob[mobnum].spritenum].z-(tilesizy[sprite[mob[mobnum].spritenum].picnum]<<7),
			    sprite[mob[mobnum].spritenum].sectnum) != 1)
			    || (!mob[i].health))
				return;
		playxysfx(SfxExplode, mobnum+1024, sprite[mob[mobnum].spritenum].x, sprite[mob[mobnum].spritenum].y);
		hurtmob(findmob(mob[mobnum].target), 20, mob[mobnum].spritenum);
		if (mob[mobnum].target == playersprite)
			sprite[mob[mobnum].target].zvel = (51<<8);
		else
			mob[findmob(mob[mobnum].target)].zvel = -(48<<8);
		spirit = mob[mobnum].tracer;
		if (spirit < 0) return;
		dax = (100*sintable[(sprite[mob[mobnum].target].ang+512)&2047]>>14);
		day = (100*sintable[sprite[mob[mobnum].target].ang&2047]>>14);
		setsprite(spirit,sprite[mob[mobnum].target].x+dax,
			sprite[mob[mobnum].target].y+day,
			sprite[mob[mobnum].target].z-(32<<8));
		spiritexplode(spirit);
		break;
		case 33:
		playxysfx(SfxWhoosh, 1024+mobnum, sprite[mob[mobnum].spritenum].x,
						     sprite[mob[mobnum].spritenum].y);
		break;
		case 34:
		playsfx(SfxBossSpot, 0, 255);
		break;
		case 35:
		exitlevel();
		break;
		case 38:
		playxysfx(SfxZombieAttack, 1024+mobnum, sprite[mob[mobnum].spritenum].x,
						sprite[mob[mobnum].spritenum].y);
		break;
		case 39:
		playxysfx(SfxFatumaWalk, 1024+mobnum, sprite[mob[mobnum].spritenum].x,
						sprite[mob[mobnum].spritenum].y);
		break;
	}
}

void processmobs(void)
{
	long i, j;
	int clipmask;
	short sectnum;
	
	for(i=0;i<nummobs;i++)
	{
		if (mob[i].bits[DropOff])
		{
			if (sprite[mob[i].spritenum].statnum != 1)
				getzrange(sprite[mob[i].spritenum].x,
				sprite[mob[i].spritenum].y,
				sprite[mob[i].spritenum].z-(1<<8),
				sprite[mob[i].spritenum].sectnum,
				&mob[i].hiz,&mob[i].hihit,&mob[i].loz,&mob[i].lohit,128L,CLIPMASK0);
			else
			{
				mob[i].hiz = sector[sprite[mob[i].spritenum].sectnum].ceilingz;
				mob[i].loz = sector[sprite[mob[i].spritenum].sectnum].floorz;
			}
			if (sprite[mob[i].spritenum].zvel < 6144)
				sprite[mob[i].spritenum].zvel += (6<<8);
			else
				sprite[mob[i].spritenum].zvel = 6144;
			sprite[mob[i].spritenum].z += sprite[mob[i].spritenum].zvel;
			if (sprite[mob[i].spritenum].z > mob[i].loz)
			{
				sprite[mob[i].spritenum].z = mob[i].loz;
				if ((mob[i].type == MobBlood) || (mob[i].type == MobDroplet))
				{
					if (mob[i].seq != mobinfo[mob[i].type].deathseq
						|| mobinfo[mob[i].type].deathseq == -1)
						killmob(i, 0);
				}
			} 
		}
		sectnum = sprite[mob[i].spritenum].sectnum;
		if (sector[sectnum].lotag == 160)
		{
			for(j=0;j<MAXSPRITES;j++)
			{
				if (sprite[j].lotag == sector[sectnum].hitag
					&& sprite[j].picnum == Telemark && sprite[mob[i].spritenum].statnum != 4
					&& i != playermob && mob[i].type != MobFusionSpark)
					warpsprite(mob[i].spritenum, j);
			}
		}
		if (sprite[mob[i].spritenum].xvel > 0 &&
			(sprite[mob[i].spritenum].statnum != 4 && sprite[mob[i].spritenum].statnum != 1 && sprite[mob[i].spritenum].statnum != 7
			&& sprite[mob[i].spritenum].statnum != 10) &&
			!(mob[i].type == MobFlyingFish && mob[i].seq == mobinfo[MobFlyingFish].missileseq))
		{
			mob[i].xvel = ((sprite[mob[i].spritenum].xvel*(long)sintable[(sprite[mob[i].spritenum].yvel+512)&2047])>>3);
			mob[i].yvel = ((sprite[mob[i].spritenum].xvel*(long)sintable[sprite[mob[i].spritenum].yvel&2047])>>3);
			if (!--sprite[mob[i].spritenum].xvel || mob[i].movestat != 0)
			{
				mob[i].xvel = 0;
				mob[i].yvel = 0;
			}
		}
		if (sprite[mob[i].spritenum].statnum == 4 || sprite[mob[i].spritenum].statnum == 10) clipmask = CLIPMASK1;
		else if (mob[i].bits[CountKill] || mob[i].type == MobFlyingFish) clipmask = CLIPMASK_MONST;
		else clipmask = CLIPMASK0;
		if (mob[i].xvel || mob[i].yvel || (mob[i].zvel || sprite[mob[i].spritenum].z != mob[i].loz))
		{
			mob[i].movestat = movesprite(mob[i].spritenum,i,mob[i].xvel,mob[i].yvel,mob[i].zvel,4L<<8,4L<<8,clipmask);
			if (mob[i].movestat != 0 && sprite[mob[i].spritenum].statnum != 4 && sprite[mob[i].spritenum].statnum != 10)
			{
				mob[i].xvel = 0;
				mob[i].yvel = 0;
				mob[i].zvel = 0;
			}
			if (sprite[mob[i].spritenum].z > mob[i].loz)
			{
				sprite[mob[i].spritenum].z = mob[i].loz;
				mob[i].zvel = 0;
			}
			else if (sprite[mob[i].spritenum].statnum == 6 && !mobinfo[mob[i].type].bits[Float])
			{
				if (mob[i].zvel == 0)
					mob[i].zvel = (8<<8);
				else
					mob[i].zvel += (4<<8);
			}
			else if (sprite[mob[i].spritenum].z < mob[i].hiz+(1<<8))
			{
				sprite[mob[i].spritenum].z = mob[i].hiz+(1<<8);
				mob[i].zvel = 0;
			}
		}
		/*if (i != playermob && sprite[mob[i].spritenum].statnum != 4)
		{
			pushmove(&sprite[mob[i].spritenum].x,
				&sprite[mob[i].spritenum].y,
				&sprite[mob[i].spritenum].z,
				&sectnum,128L,4<<8L,4<<8L,clipmask);
			changespritesect(mob[i].spritenum, sectnum);
		}*/
		if (mob[i].seq != -1)
		{
			mob[i].seqpoint.tics++;
			if (mob[i].seqpoint.tics == 1) seqtrigger(i, mobseq[mob[i].seq].frame[mob[i].seqpoint.frame].trigger);
			if (mob[i].seqpoint.tics > mobseq[mob[i].seq].tpf)
			{
				mob[i].seqpoint.tics = 0;
				mob[i].seqpoint.frame++;
				if (mob[i].seqpoint.frame > mobseq[mob[i].seq].numframes-1)
				{
					if (mob[i].seq == mobinfo[mob[i].type].hurtseq)
					{
						if (mobinfo[mob[i].type].walkseq != -1)
						{
							mob[i].aistate = AIWalk;
							setmobseq(i, mobinfo[mob[i].type].walkseq);
						}
						else
						{
							setmobseq(i, -1);
							sprite[mob[i].spritenum].picnum = mobinfo[mob[i].type].standpicnum;
							continue;
						}
					}
					/*else if (mob[i].seq == mobinfo[mob[i].type].deathseq
						|| mob[i].seq == mobinfo[mob[i].type].xdeathseq)
						setmobseq(i, -1);*/
					else if (mob[i].seq == mobinfo[mob[i].type].missileseq &&
						 mob[i].type == MobFlyingFish)
					{
						mob[i].seqpoint.frame = mobseq[mob[i].seq].numframes-1;
					}
					else if (mob[i].seq == mobinfo[mob[i].type].missileseq
					         || mob[i].seq == mobinfo[mob[i].type].meeleseq)
					{
						mob[i].aistate = AIWalk;
						setmobseq(i, mobinfo[mob[i].type].walkseq);
					}
					else if (mob[i].seq == mobinfo[mob[i].type].walkseq)
						mob[i].seqpoint.frame = 0;
					else
						mob[i].seqpoint.frame = mobseq[mob[i].seq].numframes-1;
				}
				sprite[mob[i].spritenum].picnum = mobseq[mob[i].seq].frame[mob[i].seqpoint.frame].picnum;
			}
		}
		if ((gameskill == Nightmare || respawnparm) && mob[i].aistate == AIDeath)
		{
			if (!(mob[i].bits[CountKill]))
				continue;

			mob[i].movecount++;

			if (mob[i].movecount < 10*35)
				continue;
			if (ticks&20)
				continue;
			if (krand()%10 > 5)
				continue;

			nightmarerespawn(i);
		}
	}
}
