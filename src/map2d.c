#include "sn.h"
#include "graphics.h"
#include "str.h"
#include "kbd.h"
#include "player.h"

char followmode = 1;
long followx, followy;
short mapstrings[] =
{
	StringE1M1,
	StringE1M2,
	StringE1M3,
	StringE1M4,
	StringE1M5,
	StringE1M6,
	StringE1M7,
	StringE1M8,
	StringE1M9,

	StringE2M1,
	StringE2M2,
	StringE2M3,
	StringE2M4,
	StringE2M5,
	StringE2M6,
	StringE2M7,
	StringE2M8,
	StringE2M9,

	StringE3M1,
	StringE3M2,
	StringE3M3,
	StringE3M4,
	StringE3M5,
	StringE3M6,
	StringE3M7,
	StringE3M8,
	StringE3M9,

	StringE4M1,
	StringE4M2,
	StringE4M3,
	StringE4M4,
	StringE4M5,
	StringE4M6,
	StringE4M7,
	StringE4M8,
	StringE4M9,

	StringE5M1,
	StringE5M2,
	StringE5M3,
	StringE5M4,
	StringE5M5,
	StringE5M6,
	StringE5M7,
	StringE5M8,
	StringE5M9
};

short mapstrings2[] =
{
	StringMap01,
	StringMap02,
	StringMap03,
	StringMap04,
	StringMap05,
	StringMap06,
	StringMap07,
	StringMap08,
	StringMap09,
	StringMap10,
	StringMap11,
	StringMap12,
	StringMap13,
	StringMap14,
	StringMap15,
	StringMap16,
	StringMap17,
	StringMap18,
	StringMap19,
	StringMap20,
	StringMap21,
	StringMap22,
	StringMap23,
	StringMap24,
	StringMap25,
	StringMap26,
	StringMap27,
	StringMap28,
	StringMap29,
	StringMap30,
	StringMap31,
	StringMap32
};

void drawmap(void)
{
	long i, j, k, l, x1, y1, x2, y2, x3, y3, x4, y4, ox, oy, xoff, yoff;
	long dax, day, cosang, sinang, xspan, yspan, sprx, spry;
	long xrepeat, yrepeat, z1, z2, startwall, endwall, tilenum, daang;
	long xvect, yvect, xvect2, yvect2, cposx, cposy, czoom;
	short cang;
	char col;
	walltype *wal, *wal2;
	spritetype *spr;

	if (followmode == 1)
	{
		cposx = posx;
		cposy = posy;
		cang = ang;
	}
	else
	{
		cposx = followx;
		cposy = followy;
		cang = 0;
	}
	czoom = zoom;
	setview(0,0,xdim-1,ydim-1);
	clearview(0L);
	
	if (!followmode)
	{
		if (keystatus[0xc8] > 0)
		{
			followx += (8<<4);
		}
		if (keystatus[0xd0] > 0)
		{
			followx -= (8<<4);
		}
		if (keystatus[0xcb] > 0)
		{
			followy -= (8<<4);
		}
		if (keystatus[0xcd] > 0)
		{
			followy += (8<<4);
		}
	}	
	if (mapmode == 1)
	{
		xvect = sintable[(0)&2047] * czoom;
		yvect = sintable[(1536)&2047] * czoom;
	}
	else
	{
		xvect = sintable[(-cang)&2047] * czoom;
		yvect = sintable[(1536-cang)&2047] * czoom;
	}
	xvect2 = mulscale16(xvect,yxaspect);
	yvect2 = mulscale16(yvect,yxaspect);

	if (mapmode == 2)
		drawmapview(cposx,cposy,czoom,cang);

		//Draw brown lines
	for(i=0;i<numsectors;i++)
	{
		startwall = sector[i].wallptr;
		endwall = sector[i].wallptr + sector[i].wallnum;

		z1 = sector[i].ceilingz; z2 = sector[i].floorz;

		for(j=startwall,wal=&wall[startwall];j<endwall;j++,wal++)
		{
			k = wal->nextwall; if (k < 0) continue;

			if ((show2dwall[j>>3]&(1<<(j&7))) == 0) continue;
			if ((k > j) && ((show2dwall[k>>3]&(1<<(k&7))) > 0)) continue;

			if (sector[wal->nextsector].ceilingz == z1)
				if (sector[wal->nextsector].floorz == z2)
					if (((wal->cstat|wall[wal->nextwall].cstat)&(16+32)) == 0) continue;

			if (!(wal->extra&1))
			{
				col = 210;

				if (mapmode == 2)
					continue;
			}
			else
			{
				if (mapmode == 1)
					col = 40;
				else
					col = 31;
			}

			ox = wal->x-cposx; oy = wal->y-cposy;
			x1 = dmulscale16(ox,xvect,-oy,yvect)+(xdim<<11);
			y1 = dmulscale16(oy,xvect2,ox,yvect2)+(ydim<<11);

			wal2 = &wall[wal->point2];
			ox = wal2->x-cposx; oy = wal2->y-cposy;
			x2 = dmulscale16(ox,xvect,-oy,yvect)+(xdim<<11);
			y2 = dmulscale16(oy,xvect2,ox,yvect2)+(ydim<<11);

			drawline256(x1,y1,x2,y2,col);
		}
	}

		//Draw red lines
	for(i=0;i<numsectors;i++)
	{
		startwall = sector[i].wallptr;
		endwall = sector[i].wallptr + sector[i].wallnum;

		k = -1;
		for(j=startwall,wal=&wall[startwall];j<endwall;j++,wal++)
		{
			if (wal->nextwall >= 0) continue;

			if (((show2dwall[j>>3]&(1<<(j&7))) == 0)
				&& (!powerup[ComputerMap])) continue;

			if (tilesizx[wal->picnum] == 0) continue;
			if (tilesizy[wal->picnum] == 0) continue;

			if (j == k)
				{ x1 = x2; y1 = y2; }
			else
			{
				ox = wal->x-cposx; oy = wal->y-cposy;
				x1 = dmulscale16(ox,xvect,-oy,yvect)+(xdim<<11);
				y1 = dmulscale16(oy,xvect2,ox,yvect2)+(ydim<<11);
			}

			k = wal->point2; wal2 = &wall[k];
			ox = wal2->x-cposx; oy = wal2->y-cposy;
			x2 = dmulscale16(ox,xvect,-oy,yvect)+(xdim<<11);
			y2 = dmulscale16(oy,xvect2,ox,yvect2)+(ydim<<11);

			col = 40;
			if (((show2dwall[j>>3]&(1<<(j&7))) == 0)
				&& (powerup[ComputerMap])) col = 20;
			else if (mapmode == 2) col = 31;
			
			drawline256(x1,y1,x2,y2,col);
		}
	}

	for(i=0;i<numsectors;i++)
		for(j=headspritesect[i];j>=0;j=nextspritesect[j])
			if ((show2dsprite[j>>3]&(1<<(j&7))) > 0)
			{
				spr = &sprite[j]; if (spr->cstat&0x8000) continue;
				col = 138;
				if (spr->cstat&1) col = 200;
				if (j == k) col = 31;

				sprx = spr->x;
				spry = spr->y;

				switch (spr->cstat&48)
				{
					case 0:
						ox = sprx-cposx; oy = spry-cposy;
						x1 = dmulscale16(ox,xvect,-oy,yvect);
						y1 = dmulscale16(oy,xvect2,ox,yvect2);

						if (mapmode == 1)
						{
							ox = (sintable[(spr->ang+512)&2047]>>7);
							oy = (sintable[(spr->ang)&2047]>>7);
							x2 = dmulscale16(ox,xvect,-oy,yvect);
							y2 = dmulscale16(oy,xvect,ox,yvect);

							/*if (j == playersprite)
							{
								x2 = -oy;
								y2 = -(czoom<<5);
							}*/
							x3 = mulscale16(x2,yxaspect);
							y3 = mulscale16(y2,yxaspect);

							drawline256(x1-x2+(xdim<<11),y1-y3+(ydim<<11),
											x1+x2+(xdim<<11),y1+y3+(ydim<<11),col);
							drawline256(x1-y2+(xdim<<11),y1+x3+(ydim<<11),
											x1+x2+(xdim<<11),y1+y3+(ydim<<11),col);
							drawline256(x1+y2+(xdim<<11),y1-x3+(ydim<<11),
											x1+x2+(xdim<<11),y1+y3+(ydim<<11),col);
						}
						break;
				}
			}
	if (mapmode == 2 && followmode)
		rotatesprite(320<<15,200<<15,mulscale16(czoom*64,yxaspect),0,sprite[playersprite].picnum,0,0,2+8,0,0,xdim-1,ydim-1);
	if (mapmode == 1 && !custom_map)
	{
		if (gamemode != sequel)
			gametext(0, 157, string[mapstrings[(gameepisode-1)*9+gamelevel-1]], 0);
		else
			gametext(0, 157, string[mapstrings2[gamelevel-1]], 0);
	}
	if (keystatus[33] > 0)
	{
		keystatus[33] = 0;
		followmode ^= 1;
		if (followmode == 0)
		{
			followx = posx;
			followy = posy;
		}
		putmessage(string[StringFollowModeOFF+followmode]);
	}
}
