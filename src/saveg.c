#include <fcntl.h>
#include <dos.h>
#include <io.h>
#include <sys\stat.h>
#include <sys\types.h>
#include "sn.h"
#include "graphics.h"
#include "controls.h"
#include "menus.h"
#include "demo.h"
#include "maps.h"
#include "levels.h"
#include "mirrors.h"
#include "weapon.h"
#include "player.h"
#include "movers.h"
#include "sound.h"
#include "str.h"
#include "triggers.h"

int quickslot = -1;
char slotname[9][16], savidf = 0x2f, vcheck[10];
long saveclock;

void loadgame(int slot)
{
	char tempbuf[80];
	long i, j, handle;

	killallsfx();

	sprintf(tempbuf, "GAME%d.SAV", slot);
	if ((handle = kopen4load(tempbuf, 0)) == -1)
		error("Cannot load game", 250);

	klseek(handle, 16L, SEEK_SET);
	clearbufbyte(vcheck,sizeof(vcheck),0L);
	kdfread(&vcheck,10,1,handle);
	if (stricmp(vcheck,VERSION) != 0)
	{
		kclose(handle);
		return;
	}
	// Read player varibles
	kdfread(&posx,4,1,handle);
	kdfread(&posy,4,1,handle);
	kdfread(&posz,4,1,handle);
	kdfread(&horiz,4,1,handle);
	kdfread(&ang,2,1,handle);
	kdfread(&cursectnum,2,1,handle);

	kdfread(&hvel,2,1,handle);
	kdfread(&fallcnt,2,1,handle);
	
	kdfread(&damagecnt,2,1,handle);
	kdfread(&bonuscnt,2,1,handle);

	kdfread(havekey,2,3,handle);

	kdfread(&armor,2,1,handle);
	kdfread(&armortype,2,1,handle);
	kdfread(powerup,4,NumPowerUps,handle);
	kdfread(&playersprite,2,1,handle);
	kdfread(&playermob,2,1,handle);
	kdfread(&death,2,1,handle);
	kdfread(&bob,2,1,handle);
	kdfread(&boompack,2,1,handle);
	kdfread(&cheated,2,1,handle);
	kdfread(&undead,2,1,handle);
	kdfread(&noclip,2,1,handle);
	kdfread(&teleporterhold,1,1,handle);
	kdfread(&onteleportsector,1,1,handle);
	kdfread(&kills,2,1,handle);
	kdfread(&secretsfound,2,1,handle);
	kdfread(&crouching,1,1,handle);
	kdfread(&foundsupersecret,1,1,handle);
	kdfread(&dragx,4,1,handle);
	kdfread(&dragy,4,1,handle);
	kdfread(&aim_mode,2,1,handle);

	kdfread(ammo,4,NumAmmoTypes,handle);
	kdfread(ammomax,4,NumAmmoTypes,handle);

	kdfread(haveweapon,2,NumWeapons,handle);
	kdfread(&curweapon,2,1,handle);
	kdfread(&weapstate,2,1,handle);
	kdfread(&weapseqpoint,sizeof(SeqPoint_T),1,handle);
	kdfread(&newweap,2,1,handle);

	kdfread(&ticks,2,1,handle);
	kdfread(&seconds,2,1,handle);
	kdfread(&minutes,2,1,handle);
	kdfread(&numpalookups,2,1,handle);
	kdfread(&sync,sizeof(INPUT),1,handle);

	kdfread(&gameepisode,2,1,handle);
	kdfread(&gamelevel,2,1,handle);
	kdfread(&gameskill,2,1,handle);
	kdfread(&custom_map,2,1,handle);
	kdfread(boardfilename,13,1,handle);

	// Read world varibles
	kdfread(&numsectors,2,1,handle);
	kdfread(sector,sizeof(sectortype),numsectors,handle);

	kdfread(&numwalls,2,1,handle);
	kdfread(wall,sizeof(walltype),numwalls,handle);

	kdfread(sprite,sizeof(spritetype),MAXSPRITES,handle);
	kdfread(headspritesect,2,MAXSECTORS+1,handle);
	kdfread(headspritestat,2,MAXSTATUS+1,handle);
	kdfread(prevspritesect,2,MAXSPRITES,handle);
	kdfread(prevspritestat,2,MAXSPRITES,handle);
	kdfread(nextspritesect,2,MAXSPRITES,handle);
	kdfread(nextspritestat,2,MAXSPRITES,handle);

	kdfread(show2dsector,sizeof(char),numsectors>>3,handle);
	kdfread(show2dwall,sizeof(char),numwalls>>3,handle);
	kdfread(show2dsprite,sizeof(char),MAXSPRITES>>3,handle);
	
	// Read movers
	kdfread(&numtrobs,2,1,handle);
	kdfread(trob,sizeof(Trob_T),numtrobs,handle);
	
	kdfread(&nummobs,2,1,handle);
	kdfread(mob,sizeof(Mob_T),nummobs,handle);

	// Read specials
	kdfread(&revolvecnt,2,1,handle);
	kdfread(revolvesector,2,revolvecnt,handle);
	kdfread(revolvex,4,4*16,handle);
	kdfread(revolvey,4,4*16,handle);
	kdfread(revolvepivotx,4,revolvecnt,handle);
	kdfread(revolvepivoty,4,revolvecnt,handle);

	kdfread(&dragsectorcnt,2,1,handle);
	kdfread(dragsector,2,dragsectorcnt,handle);
	kdfread(dragxdir,2,dragsectorcnt,handle);
	kdfread(dragydir,2,dragsectorcnt,handle);
	kdfread(dragx1,4,dragsectorcnt,handle);
	kdfread(dragy1,4,dragsectorcnt,handle);
	kdfread(dragx2,4,dragsectorcnt,handle);
	kdfread(dragy2,4,dragsectorcnt,handle);
	kdfread(dragfloorz,4,dragsectorcnt,handle);

	kdfread(&pansectorcnt,2,1,handle);
	kdfread(pansprite,2,pansectorcnt,handle);
	kdfread(panx,4,pansectorcnt,handle);
	kdfread(pany,4,pansectorcnt,handle);
	kdfread(pansector,2,pansectorcnt,handle);

	kdfread(&crushcnt,2,1,handle);
	kdfread(crushsector,2,crushcnt,handle);
	kdfread(crusholdz,4,crushcnt,handle);
	
	kdfread(wallfind,4,numsectors,handle);
	
	kdfread(activelifts,2,MAXLIFTS,handle);
	kdfread(lifttrob,2,MAXLIFTS,handle);
	kdfread(liftoldz,4,MAXLIFTS,handle);
	kdfread(liftsector,2,MAXLIFTS,handle);

	kdfread(&swingcnt,2,1,handle);
	kdfread(swingsector,2,swingcnt,handle);
	kdfread(swingwall,2,32*5,handle);
	kdfread(swingangopen,2,swingcnt,handle);
	kdfread(swingangclosed,2,swingcnt,handle);
	kdfread(swingangopendir,2,swingcnt,handle);
	kdfread(swingang,2,swingcnt,handle);
	kdfread(swinganginc,2,swingcnt,handle);
	kdfread(swingx,4,32*8,handle);
	kdfread(swingy,4,32*8,handle);
	kdfread(&tag502cnt,2,1,handle);
	kdfread(&tag502active,2,1,handle);
	kdfread(&tag502time,2,1,handle);

	kdfread(&numtyres,2,1,handle);

	kdfread(&numkills,2,1,handle);
	kdfread(&numsecrets,2,1,handle);
	kdfread(&didsecret,2,1,handle);

	kdfread(&mirrorcnt,2,1,handle);
	kdfread(mirrorwall,2,mirrorcnt,handle);
	kdfread(mirrorsector,2,mirrorcnt,handle);

	kdfread(lightstart,2,numsectors,handle);
	kdfread(lightend,2,numsectors,handle);

	kdfread(&saveclock,4,1,handle);
	totalclock = saveclock;
	kdfread(&numframes,4,1,handle);
	kdfread(&randomseed,4,1,handle);

	kdfread(&savidf,1,1,handle);

	kclose(handle);
	if (savidf != 0x2f) error("Bad savegame", 251);
	gamestate = GAME;
	playerabouttoexit = 0;
	demoplayback = 0;
	menustatus = 0;
	cachelevel();
	setviewsize();
	if (gamemode != sequel)
		playsong(gamemusic[(gameepisode-1)*9+gamelevel-1], 1);
	else
		playsong(gamemusic2[gamelevel-1], 1);
	putmessage(string[StringGameLoaded]);
}

void savegame(int slot)
{
	char tempbuf[80], tempbuf2[10];
	long i, j;
	FILE *handle;

	sprintf(tempbuf, "GAME%d.SAV", slot);
	if ((handle = fopen(tempbuf,"wb")) == 0)
		error("Cannot save game", 250);

	fwrite(&slotname[slot],16,1,handle);

	// Write player varibles
	strcpy(tempbuf2,VERSION);
	dfwrite(&tempbuf2,10,1,handle);
	dfwrite(&posx,4,1,handle);
	dfwrite(&posy,4,1,handle);
	dfwrite(&posz,4,1,handle);
	dfwrite(&horiz,4,1,handle);
	dfwrite(&ang,2,1,handle);
	dfwrite(&cursectnum,2,1,handle);

	dfwrite(&hvel,2,1,handle);
	dfwrite(&fallcnt,2,1,handle);
	
	dfwrite(&damagecnt,2,1,handle);
	dfwrite(&bonuscnt,2,1,handle);

	dfwrite(havekey,2,3,handle);

	dfwrite(&armor,2,1,handle);
	dfwrite(&armortype,2,1,handle);
	dfwrite(powerup,4,NumPowerUps,handle);
	dfwrite(&playersprite,2,1,handle);
	dfwrite(&playermob,2,1,handle);
	dfwrite(&death,2,1,handle);
	dfwrite(&bob,2,1,handle);
	dfwrite(&boompack,2,1,handle);
	dfwrite(&cheated,2,1,handle);
	dfwrite(&undead,2,1,handle);
	dfwrite(&noclip,2,1,handle);
	dfwrite(&teleporterhold,1,1,handle);
	dfwrite(&onteleportsector,1,1,handle);
	dfwrite(&kills,2,1,handle);
	dfwrite(&secretsfound,2,1,handle);
	dfwrite(&crouching,1,1,handle);
	dfwrite(&foundsupersecret,1,1,handle);
	dfwrite(&dragx,4,1,handle);
	dfwrite(&dragy,4,1,handle);
	dfwrite(&aim_mode,2,1,handle);

	dfwrite(ammo,4,NumAmmoTypes,handle);
	dfwrite(ammomax,4,NumAmmoTypes,handle);

	dfwrite(haveweapon,2,NumWeapons,handle);
	dfwrite(&curweapon,2,1,handle);
	dfwrite(&weapstate,2,1,handle);
	dfwrite(&weapseqpoint,sizeof(SeqPoint_T),1,handle);
	dfwrite(&newweap,2,1,handle);

	dfwrite(&ticks,2,1,handle);
	dfwrite(&seconds,2,1,handle);
	dfwrite(&minutes,2,1,handle);
	dfwrite(&numpalookups,2,1,handle);
	dfwrite(&sync,sizeof(INPUT),1,handle);

	dfwrite(&gameepisode,2,1,handle);
	dfwrite(&gamelevel,2,1,handle);
	dfwrite(&gameskill,2,1,handle);
	dfwrite(&custom_map,2,1,handle);
	dfwrite(boardfilename,13,1,handle);

	// Write world varibles
	dfwrite(&numsectors,2,1,handle);
	dfwrite(sector,sizeof(sectortype),numsectors,handle);

	dfwrite(&numwalls,2,1,handle);
	dfwrite(wall,sizeof(walltype),numwalls,handle);

	dfwrite(sprite,sizeof(spritetype),MAXSPRITES,handle);
	dfwrite(headspritesect,2,MAXSECTORS+1,handle);
	dfwrite(headspritestat,2,MAXSTATUS+1,handle);
	dfwrite(prevspritesect,2,MAXSPRITES,handle);
	dfwrite(prevspritestat,2,MAXSPRITES,handle);
	dfwrite(nextspritesect,2,MAXSPRITES,handle);
	dfwrite(nextspritestat,2,MAXSPRITES,handle);

	dfwrite(show2dsector,sizeof(char),numsectors>>3,handle);
	dfwrite(show2dwall,sizeof(char),numwalls>>3,handle);
	dfwrite(show2dsprite,sizeof(char),MAXSPRITES>>3,handle);
	
	// Write movers
	dfwrite(&numtrobs,2,1,handle);
	dfwrite(trob,sizeof(Trob_T),numtrobs,handle);

	dfwrite(&nummobs,2,1,handle);
	dfwrite(mob,sizeof(Mob_T),nummobs,handle);

	// Write specials
	dfwrite(&revolvecnt,2,1,handle);
	dfwrite(revolvesector,2,revolvecnt,handle);
	dfwrite(revolvex,4,4*16,handle);
	dfwrite(revolvey,4,4*16,handle);
	dfwrite(revolvepivotx,4,revolvecnt,handle);
	dfwrite(revolvepivoty,4,revolvecnt,handle);

	dfwrite(&dragsectorcnt,2,1,handle);
	dfwrite(dragsector,2,dragsectorcnt,handle);
	dfwrite(dragxdir,2,dragsectorcnt,handle);
	dfwrite(dragydir,2,dragsectorcnt,handle);
	dfwrite(dragx1,4,dragsectorcnt,handle);
	dfwrite(dragy1,4,dragsectorcnt,handle);
	dfwrite(dragx2,4,dragsectorcnt,handle);
	dfwrite(dragy2,4,dragsectorcnt,handle);
	dfwrite(dragfloorz,4,dragsectorcnt,handle);

	dfwrite(&pansectorcnt,2,1,handle);
	dfwrite(pansprite,2,pansectorcnt,handle);
	dfwrite(panx,4,pansectorcnt,handle);
	dfwrite(pany,4,pansectorcnt,handle);
	dfwrite(pansector,2,pansectorcnt,handle);

	dfwrite(&crushcnt,2,1,handle);
	dfwrite(crushsector,2,crushcnt,handle);
	dfwrite(crusholdz,4,crushcnt,handle);

	dfwrite(wallfind,4,numsectors,handle);

	dfwrite(activelifts,2,MAXLIFTS,handle);
	dfwrite(lifttrob,2,MAXLIFTS,handle);
	dfwrite(liftoldz,4,MAXLIFTS,handle);
	dfwrite(liftsector,2,MAXLIFTS,handle);

	dfwrite(&swingcnt,2,1,handle);
	dfwrite(swingsector,2,swingcnt,handle);
	dfwrite(swingwall,2,32*5,handle);
	dfwrite(swingangopen,2,swingcnt,handle);
	dfwrite(swingangclosed,2,swingcnt,handle);
	dfwrite(swingangopendir,2,swingcnt,handle);
	dfwrite(swingang,2,swingcnt,handle);
	dfwrite(swinganginc,2,swingcnt,handle);
	dfwrite(swingx,4,32*8,handle);
	dfwrite(swingy,4,32*8,handle);
	dfwrite(&tag502cnt,2,1,handle);
	dfwrite(&tag502active,2,1,handle);
	dfwrite(&tag502time,2,1,handle);

	dfwrite(&numtyres,2,1,handle);

	dfwrite(&numkills,2,1,handle);
	dfwrite(&numsecrets,2,1,handle);
	dfwrite(&didsecret,2,1,handle);
	
	dfwrite(&mirrorcnt,2,1,handle);
	dfwrite(mirrorwall,2,mirrorcnt,handle);
	dfwrite(mirrorsector,2,mirrorcnt,handle);

	dfwrite(lightstart,2,numsectors,handle);
	dfwrite(lightend,2,numsectors,handle);

	saveclock = totalclock;
	dfwrite(&saveclock,4,1,handle);
	dfwrite(&numframes,4,1,handle);
	dfwrite(&randomseed,4,1,handle);

	dfwrite(&savidf,1,1,handle);

	fclose(handle);

	quickslot = slot;
	putmessage(string[StringGameSaved]);
}
