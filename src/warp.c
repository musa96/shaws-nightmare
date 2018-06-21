#include "sn.h"
#include "sfx.h"
#include "movers.h"
#include "player.h"

void warp(int spritenum)
{
	long oldx, oldy, oldz;
	short oldsect, tflash, tmob;
	long xshift, yshift;

	oldx = posx;
	oldy = posy;
	oldz = posz;
	oldsect = cursectnum;
	posx = sprite[spritenum].x;
	posy = sprite[spritenum].y;
	posz = sprite[spritenum].z-PLAYERHEIGHT;
	ang = sprite[spritenum].ang;
	cursectnum = sprite[spritenum].sectnum;
	updateplayersprite();
	teleporterhold = 22;
	// spawn the teleporter flash at both locations
	spawnsprite(tflash,oldx,oldy,oldz,0,-32,0,32,64,64,0,0,mobinfo[MobTeleFlash].standpicnum,
		    0,0,0,0,-1,oldsect,7,0,0,0,0,0);
	tmob = addmob(tflash, MobTeleFlash);
	setmobseq(tmob, mobinfo[MobTeleFlash].walkseq);
	playxysfx(SfxTeleport, tflash+1024, oldx, oldy);

	xshift = (100*sintable[(sprite[spritenum].ang+512)&2047]>>14);
	yshift = (100*sintable[sprite[spritenum].ang&2047]>>14);
	spawnsprite(tflash,posx+xshift,posy+yshift,posz+(12<<8),0,-32,0,32,64,64,0,0,mobinfo[MobTeleFlash].standpicnum,
		    ang,0,0,0,-1,cursectnum,7,0,0,0,0,0);
	tmob = addmob(tflash, MobTeleFlash);
	setmobseq(tmob, mobinfo[MobTeleFlash].walkseq);

	playxysfx(SfxTeleport, tflash+1024, sprite[spritenum].x, sprite[spritenum].y);
}

void warpsprite(int spritetowarp, int spritenum)
{
	long oldx, oldy, oldz;
	short tflash, tmob, oldsect;

	if (sprite[spritetowarp].statnum == 7) return;
	
	oldx = sprite[spritetowarp].x;
	oldy = sprite[spritetowarp].y;
	oldz = sprite[spritetowarp].z-PLAYERHEIGHT;
	oldsect = sprite[spritetowarp].sectnum;
	setsprite(spritetowarp,sprite[spritenum].x,sprite[spritenum].y,sprite[spritenum].z);
	// spawn the teleporter flash at both locations
	spawnsprite(tflash,oldx,oldy,oldz,0,-32,0,32,64,64,0,0,mobinfo[MobTeleFlash].standpicnum,
		    0,0,0,0,-1,oldsect,7,0,0,0,0,0);
	tmob = addmob(tflash, MobTeleFlash);
	setmobseq(tmob, mobinfo[MobTeleFlash].walkseq);
	playxysfx(SfxTeleport, tflash+1024, oldx, oldy);

	spawnsprite(tflash,sprite[spritenum].x,sprite[spritenum].y,sprite[spritenum].z-PLAYERHEIGHT,0,-32,0,32,64,64,0,0,mobinfo[MobTeleFlash].standpicnum,
		    0,0,0,0,-1,sprite[spritenum].sectnum,7,0,0,0,0,0);
	tmob = addmob(tflash, MobTeleFlash);
	setmobseq(tmob, mobinfo[MobTeleFlash].walkseq);

	playxysfx(SfxTeleport, tflash+1024, sprite[spritenum].x, sprite[spritenum].y);
}
