#include <dos.h>
#include "sn.h"
#include "graphics.h"
#include "levels.h"
//#include "player.h"

#define MAXMIRRORS 64

short mirrorwall[MAXMIRRORS], mirrorsector[MAXMIRRORS], mirrorcnt;

void initmirrors(void)
{
	long i, j, k, s;
	short startwall, endwall;

	mirrorcnt = 0;
	for(i=0;i<MAXMIRRORS;i++)
	{
		tilesizx[i+MIRRORLABEL] = 0;
		tilesizy[i+MIRRORLABEL] = 0;
	}

	for(i=0;i<numwalls;i++)
	{
		s = wall[i].nextsector;
		if ((s >= 0) && (wall[i].overpicnum == MIRROR) && (wall[i].cstat&32))
		{
			if ((sector[s].floorstat&1) == 0)
			{
				wall[i].overpicnum = MIRRORLABEL+mirrorcnt;
				sector[s].ceilingpicnum = MIRRORLABEL+mirrorcnt;
				sector[s].floorpicnum = MIRRORLABEL+mirrorcnt;
				sector[s].floorstat |= 1;
				mirrorwall[mirrorcnt] = i;
				mirrorsector[mirrorcnt] = s;
				mirrorcnt++;
			}
			else
				wall[i].overpicnum = sector[s].ceilingpicnum;
		}
	}

		//Invalidate textures in sector behind mirror
	for(i=0;i<mirrorcnt;i++)
	{
		k = mirrorsector[i];
		startwall = sector[k].wallptr;
		endwall = startwall + sector[k].wallnum;
		for(j=startwall;j<endwall;j++)
		{
			wall[j].picnum = MIRROR;
			wall[j].overpicnum = MIRROR;
		}
	}
}

void drawmirrors(long cposx, long cposy, long cposz, long choriz, short cang)
{
	spritetype *tspr;
	long i, j, k, tposx, tposy, tposz;
	short tang;
	long *longptr;

	//WARNING!  Assuming (MIRRORLABEL&31) = 0 and MAXMIRRORS = 64
	longptr = (long *)FP_OFF(gotpic[MIRRORLABEL>>3]);
	if (longptr[0]|longptr[1])
		for(i=MAXMIRRORS-1;i>=0;i--)
			if (gotpic[(i+MIRRORLABEL)>>3]&(1<<(i&7)))
			{
				gotpic[(i+MIRRORLABEL)>>3] &= ~(1<<(i&7));

					//Prepare drawrooms for drawing mirror and calculate reflected
					//position into tposx, tposy, and tang (tposz == cposz)
					//Must call preparemirror before drawrooms and
					//          completemirror after drawrooms
				preparemirror(cposx,cposy,cposz,cang,choriz,
						mirrorwall[i],mirrorsector[i],&tposx,&tposy,&tang);


				drawrooms(tposx,tposy,cposz,tang,choriz,mirrorsector[i]|MAXSECTORS);
				for(j=0,tspr=&tsprite[0];j<spritesortcnt;j++,tspr++)
					if ((tspr->cstat&48) == 0) tspr->cstat |= 4;
				analyzesprites(tposx,tposy);
				drawmasks();
				completemirror();   //Reverse screen x-wise in this function

				break;
			}
}
