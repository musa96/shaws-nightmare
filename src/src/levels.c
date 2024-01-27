#include "sn.h"
#include "kbd.h"
#include "dat.h"
#include "controls.h"
#include "graphics.h"
#include "str.h"
#include "player.h"
#include "epiend.h"
#include "triggers.h"
#include "seq.h"
#include "mirrors.h"
#include "sfx.h"
#include "sound.h"
#include "movers.h"
#include "maps.h"

extern Seq_T mobseq[SeqEnd];
extern long mapversion;
int secretexit;

void cachelevel(void)
{
	long i, j, k;

	for(i=0;i<numsectors;i++)
	{
		if (waloff[sector[i].floorpicnum] == 0)
			loadtile(sector[i].floorpicnum);
		if (waloff[sector[i].ceilingpicnum] == 0)
			loadtile(sector[i].ceilingpicnum);
		for(j=headspritesect[i];j>=0;j=k)
		{
			k = nextspritesect[j];
			if (!(sprite[j].cstat&0x8000))
				if (waloff[sprite[j].picnum] == 0)
					loadtile(sprite[j].picnum);
		}
	}
	for(i=0;i<numwalls;i++)
	{
		if (waloff[wall[i].picnum] == 0)
			loadtile(wall[i].picnum);
	}
}

void setuplevel(void)
{
	long i, j, k, dax, day, dax2, day2;
	short dasector, startwall, endwall;
	int bit;

	randomseed = 17L;

	killallsfx();
	clearmovers();
	for(i=0;i<MAXLIFTS;i++)
	{
		activelifts[i] = 0;
		liftoldz[i] = 0;
		liftsector[i] = -1;
		lifttrob[i] = -1;
	}
	numkills = 0;
	kills = 0;
	secretsfound = 0;
	foundsupersecret = 0;
	numsecrets = 0; // Make a list of secrets
	crushcnt = 0; // Make a list of crushing ceilings
	swingcnt = 0; // Make a list of swinging doors
	pansectorcnt = 0; // Make a list of panning sectors
	revolvecnt = 0; // Make a list of revolving doors
	dragsectorcnt = 0; // Make a list of dragging sectors
	gatecnt = 0; // Make a list of gates

	tag502cnt = 0;
	tag502active = 0;
	tag502time = 0;

	for(i=0;i<numsectors;i++)
	{
		switch(sector[i].lotag)
		{
			case 40:
			numsecrets++;
			break;
			case 56:
			dasector = i;
			dax = 0x7fffffff;
			day = 0x7fffffff;
			dax2 = 0x80000000;
			day2 = 0x80000000;
			startwall = sector[i].wallptr;
			endwall = startwall+sector[i].wallnum;
			for(j=startwall;j<endwall;j++)
			{
				if (wall[j].x < dax) dax = wall[j].x;
				if (wall[j].y < day) day = wall[j].y;
				if (wall[j].x > dax2) dax2 = wall[j].x;
				if (wall[j].y > day2) day2 = wall[j].y;
				if (wall[j].lotag == 48) k = j;
			}
			if (wall[k].x == dax) dragxdir[dragsectorcnt] = -16;
			if (wall[k].y == day) dragydir[dragsectorcnt] = -16;
			if (wall[k].x == dax2) dragxdir[dragsectorcnt] = 16;
			if (wall[k].y == day2) dragydir[dragsectorcnt] = 16;

			dasector = wall[startwall].nextsector;
			dragx1[dragsectorcnt] = 0x7fffffff;
			dragy1[dragsectorcnt] = 0x7fffffff;
			dragx2[dragsectorcnt] = 0x80000000;
			dragy2[dragsectorcnt] = 0x80000000;
			startwall = sector[dasector].wallptr;
			endwall = startwall+sector[dasector].wallnum;
			for(j=startwall;j<endwall;j++)
			{
				if (wall[j].x < dragx1[dragsectorcnt]) dragx1[dragsectorcnt] = wall[j].x;
				if (wall[j].y < dragy1[dragsectorcnt]) dragy1[dragsectorcnt] = wall[j].y;
				if (wall[j].x > dragx2[dragsectorcnt]) dragx2[dragsectorcnt] = wall[j].x;
				if (wall[j].y > dragy2[dragsectorcnt]) dragy2[dragsectorcnt] = wall[j].y;

				setinterpolation(&sector[dasector].floorz);
				setinterpolation(&wall[j].x);
				setinterpolation(&wall[j].y);
				setinterpolation(&wall[wall[j].nextwall].x);
				setinterpolation(&wall[wall[j].nextwall].y);
			}

			dragx1[dragsectorcnt] += (wall[sector[i].wallptr].x-dax);
			dragy1[dragsectorcnt] += (wall[sector[i].wallptr].y-day);
			dragx2[dragsectorcnt] -= (dax2-wall[sector[i].wallptr].x);
			dragy2[dragsectorcnt] -= (day2-wall[sector[i].wallptr].y);
			dragfloorz[dragsectorcnt] = sector[i].floorz;

			dragsector[dragsectorcnt++] = i;
			break;
			case 72:
			startwall = sector[i].wallptr;
			endwall = startwall+sector[i].wallnum;
			dax = 0L;
			day = 0L;
			for(j=startwall;j<endwall;j++)
			{
				dax += wall[j].x;
				day += wall[j].y;
			}
			revolvepivotx[revolvecnt] = dax / (endwall-startwall);
			revolvepivoty[revolvecnt] = day / (endwall-startwall);

			k = 0;
			for(j=startwall;j<endwall;j++)
			{
				revolvex[revolvecnt][k] = wall[j].x;
				revolvey[revolvecnt][k] = wall[j].y;

				setinterpolation(&wall[j].x);
				setinterpolation(&wall[j].y);
				setinterpolation(&wall[wall[j].nextwall].x);
				setinterpolation(&wall[wall[j].nextwall].y);

				k++;
			}
			revolvesector[revolvecnt] = i;
			revolveang[revolvecnt] = 0;
			revolvecnt++;
			break;
			case 109:
			crusholdz[crushcnt] = sector[i].ceilingz;
			crushsector[crushcnt] = i;
			addtrob(63, i);
			crushcnt++;
			break;
			case 110:
			crusholdz[crushcnt] = sector[i].ceilingz;
			crushsector[crushcnt] = i;
			addtrob(64, i);
			crushcnt++;
			break;
			case 116:
			startwall = sector[i].wallptr;
			endwall = startwall+sector[i].wallnum;
			for(j=startwall;j<endwall;j++)
			{
				if (wall[j].lotag == 34)
				{
					k = wall[wall[wall[wall[j].point2].point2].point2].point2;
					if ((wall[j].x == wall[k].x) && (wall[j].y == wall[k].y))
					{     //Door opens counterclockwise
						swingwall[swingcnt][0] = j;
						swingwall[swingcnt][1] = wall[j].point2;
						swingwall[swingcnt][2] = wall[wall[j].point2].point2;
						swingwall[swingcnt][3] = wall[wall[wall[j].point2].point2].point2;
						swingangopen[swingcnt] = 1536;
						swingangclosed[swingcnt] = 0;
						swingangopendir[swingcnt] = -1;
					}
					else
					{     //Door opens clockwise
						swingwall[swingcnt][0] = wall[j].point2;
						swingwall[swingcnt][1] = j;
						swingwall[swingcnt][2] = lastwall(j);
						swingwall[swingcnt][3] = lastwall(swingwall[swingcnt][2]);
						swingwall[swingcnt][4] = lastwall(swingwall[swingcnt][3]);
						swingangopen[swingcnt] = 512;
						swingangclosed[swingcnt] = 0;
						swingangopendir[swingcnt] = 1;
					}
					for(k=0;k<4;k++)
					{
						swingx[swingcnt][k] = wall[swingwall[swingcnt][k]].x;
						swingy[swingcnt][k] = wall[swingwall[swingcnt][k]].y;
					}

					swingsector[swingcnt] = i;
					swingang[swingcnt] = swingangclosed[swingcnt];
					swinganginc[swingcnt] = 0;
					swingcnt++;
				}
			}
			break;
			case 140:
			j = addtrob(500, (short)i);
			trob[j].direction = 1;
			break;
		}
	}
	numtyres = 0;
	for(i=0;i<numwalls;i++)
	{
		if ((wall[i].extra&2))
			wall[i].cstat |= 1024;
	}
	for(i=0;i<MAXSPRITES;i++)
	{
		if (sprite[i].statnum == MAXSTATUS) continue;
		if (gameskill == Baby)
			bit = 1;
		else if (gameskill == Nightmare)
			bit = 4;
		else
			bit = 1<<(gameskill-1);

		if ((!(sprite[i].extra&bit)))
		{
			deletesprite(i);
			continue;
		}
		sprite[i].extra = 0;
		switch(sprite[i].lotag)
		{
			case 76:
			sprite[i].cstat |= 0x8000;
			changespritestat(i, 9);
			break;
			case 110:
			gatesprite[gatecnt] = i;
			gatetrob[gatecnt] = -1;
			gatecnt++;
			break;
		}
		switch(sprite[i].picnum)
		{
			case Pan:
			sprite[i].cstat |= 0x8000;
			pansector[pansectorcnt] = sprite[i].sectnum;
			panx[pansectorcnt] = (sintable[(sprite[i].ang+512)&2047]>>9);
			pany[pansectorcnt] = (sintable[sprite[i].ang&2047]>>9);
			pansprite[pansectorcnt] = i;
			changespritestat(i,7);
			pansectorcnt++;
			break;
			case Light:
			sprite[i].cstat |= 0x8000;
			lightstart[sprite[i].sectnum] = sector[sprite[i].sectnum].floorshade;
			lightend[sprite[i].sectnum] = sprite[i].shade;
			switch (sprite[i].lotag)
			{
				case 0:
				addtrob(23, sprite[i].sectnum);
				break;
				case 1:
				addtrob(24, sprite[i].sectnum);
				break;
				case 2:
				addtrob(25, sprite[i].sectnum);
				break;
			}
			changespritestat(i,7);
			break;
			case Telemark:
			sprite[i].cstat |= 0x8000;
			changespritestat(i,7);
			break;
			case SpawnSpot:
			sprite[i].cstat |= 0x8000;
			spawnx[spawnspotcnt] = sprite[i].x;
			spawny[spawnspotcnt] = sprite[i].y;
			spawnsprite[spawnspotcnt] = i;
			spawnspotcnt++;
			break;
			case Tyre:
			numtyres++;
			break;
		}
		if (nomonsters && sprite[i].lotag == mobinfo[MobFlyingFish].shawedlotag)
		{
			deletesprite((short)i);
			continue;
		}
		for(j=0;j<NumMobTypes;j++)
		{
			if (sprite[i].lotag == mobinfo[j].shawedlotag)
			{
				if (gamemode != sequel)
				{
					// Don't spawn cool new monsters if not in sequel mode.
					switch(j)
					{
						case MobGirl:
						case MobPlopper:
						case MobLITFGuy:
						case MobFatuma:
						case MobKnight:
						case MobZombieGripper:
						case MobLilKinene:
						case MobBoss:
						deletesprite((short)i);
						continue;
					}
				}
				if ((nomonsters == 1) && (mobinfo[j].bits[CountKill] == 1))
				{
					deletesprite((short)i);
					continue;
				}
				if (j == MobBossEye) sprite[i].cstat |= 0x8000;
				addmob(i, j);
				if (mobinfo[j].bits[CountKill])
				{
					//sprite[i].cstat |= 1024;
					numkills++;
				}
				if (sprite[i].lotag == mobinfo[MobHiddenExploder].shawedlotag)
				{
					sprite[i].cstat |= 0x8000;
				}
			}
		}
	}

	for(i=0;i<nummobs;i++)
	{
		if (mob[i].bits[CountKill])
		{
			mob[i].health *= gameskill+1;
		}
	}
	initmirrors();
	initplayersprite();
	if (!custom_map)
	{
		if (gamemode != sequel)
			playsong(gamemusic[(gameepisode-1)*9+gamelevel-1], 1);
		else
			playsong(gamemusic2[gamelevel-1], 1);
	}
	totalclock = 0L;
}

void loadlevel(char *filename)
{
	long i, fil;
	short numsprites;
	char tempbuf[80];

	strcpy(boardfilename,filename);
	if (loadboard(filename, &posx, &posy, &posz, &ang, &cursectnum) == -1)
	{
		fil = loaditem(filename);
		readitem(fil,&mapversion,4);
		if (mapversion != 7L) error("Wrong map version", 100);

		initspritelists();
	
		clearbuf((long)(&show2dsector[0]),(long)((MAXSECTORS+3)>>5),0L);
		clearbuf((long)(&show2dsprite[0]),(long)((MAXSPRITES+3)>>5),0L);
		clearbuf((long)(&show2dwall[0]),(long)((MAXWALLS+3)>>5),0L);

		readitem(fil,&posx,4);
		readitem(fil,&posy,4);
		readitem(fil,&posz,4);
		readitem(fil,&ang,2);
		readitem(fil,&cursectnum,2);

		readitem(fil,&numsectors,2);
		readitem(fil,&sector[0],sizeof(sectortype)*numsectors);

		readitem(fil,&numwalls,2);
		readitem(fil,&wall[0],sizeof(walltype)*numwalls);

		readitem(fil,&numsprites,2);
		readitem(fil,&sprite[0],sizeof(spritetype)*numsprites);

		for(i=0;i<numsprites;i++)
			insertsprite(sprite[i].sectnum,sprite[i].statnum);

			//Must be after loading sectors, etc!
		updatesector(posx,posy,&cursectnum);
	}
	setviewsize();
	setuplevel();
	cachelevel();
}

void startlevel(int episode, int level, int skill)
{
	char tempbuf[80];
	
	custom_map = 0;
	gameepisode = episode;
	gamelevel = level;
	gameskill = skill;
	if (gamemode != sequel)
		sprintf(tempbuf, "E%dM%d.MAP", episode, level);
	else
		sprintf(tempbuf, "MAP%.2d.MAP", level);
	loadlevel(tempbuf);
}

void exitlevel(void)
{
	long i;

	if (gamelevel == 8 && gamemode != sequel) { startend(); return; }
	secretexit = 0;
	temphealth = mob[playermob].health;
	startintermission();
}

void exitsecretlevel(void)
{
	long i;

	secretexit = 1;
	i_state = 0;
	temphealth = mob[playermob].health;
	startintermission();
}
