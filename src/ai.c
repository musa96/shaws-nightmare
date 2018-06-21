#include "sn.h"
#include "str.h"
#include "ai.h"
#include "weapon.h"
#include "player.h"
#include "movers.h"

void process_enemies(void)
{
	long i, j, deltaz, nexti, dax, day, daz, dax2, day2;
	short osectnum, movestat, daang, daang2;
	int mobnum, doubvel;

	for(i=headspritestat[6];i>=0;i=nexti)
	{
		nexti = nextspritestat[i];

		mobnum = findmob(i);
		getzrange(sprite[i].x,sprite[i].y,sprite[i].z,sprite[i].sectnum,
			&mob[mobnum].hiz,&mob[mobnum].hihit,&mob[mobnum].loz,&mob[mobnum].lohit,128L,CLIPMASK0);

		if (mob[mobnum].target == mobnum && mob[mobnum].health)
		{
			if ((cansee(posx,posy,posz,cursectnum,sprite[i].x,
					sprite[i].y,sprite[i].z-(tilesizy[sprite[i].picnum]<<7),sprite[i].sectnum) == 1)
					&& (mob[playermob].health) && (!powerup[Invisibility]))
				mob[mobnum].target = playersprite;
			else
			{
				setmobseq(mobnum, -1);
				sprite[i].picnum = mobinfo[mob[mobnum].type].standpicnum;
				changespritestat(i, 2);
			}
		}
		if (mob[mobnum].target != playersprite)
		{
			j = findmob(mob[mobnum].target);
			if (mob[j].aistate == AIDeath)
			{
				if ((cansee(posx,posy,posz,cursectnum,sprite[i].x,
						sprite[i].y,sprite[i].z-(tilesizy[sprite[i].picnum]<<7),sprite[i].sectnum) == 1)
						&& (mob[playermob].health) && (!powerup[Invisibility]))
					mob[mobnum].target = playersprite;
			}
		}
		if (((mob[mobnum].target < 0) || (mob[mobnum].target > MAXSPRITES)) && (mob[mobnum].health > 0))
		{
			setmobseq(mobnum, -1);
			sprite[i].picnum = mobinfo[mob[mobnum].type].standpicnum;
			changespritestat(i, 2);
		}
		if ((!mob[mobnum].movecount) && (mob[mobnum].health > 0) && (mobinfo[mob[mobnum].type].missileseq != -1))
		{
			j = findmob(mob[mobnum].target);
			if (mob[mobnum].target == playersprite)
			{
				daz = sprite[mob[mobnum].target].z-PLAYERHEIGHT;
				if (crouching)
					daz += (24<<8);
			}
			else
				daz = sprite[mob[mobnum].target].z;
			if ((cansee(sprite[mob[mobnum].target].x,
			     sprite[mob[mobnum].target].y,
			     daz,sprite[mob[mobnum].target].sectnum,
			     sprite[i].x,sprite[i].y,sprite[i].z-(tilesizy[sprite[i].picnum]<<7),sprite[i].sectnum) == 0)
			     || (!mob[j].health) || (j == playermob && sprite[mob[mobnum].target].cstat&2))
			{
				if (!mob[mobnum].movecount && mob[mobnum].type != MobBossEye)
				{
					setmobseq(mobnum, -1);
					sprite[i].picnum = mobinfo[mob[mobnum].type].standpicnum;
					changespritestat(i,2);
				}
			}
			else
			{
				sprite[i].ang = getangle(sprite[mob[mobnum].target].x-sprite[i].x,sprite[mob[mobnum].target].y-sprite[i].y);
				mob[mobnum].aistate = AIAttack;
				setmobseq(mobnum, mobinfo[mob[mobnum].type].missileseq);
				mob[mobnum].movecount = krand()%100;
			}
		}
		else if (mob[mobnum].aistate == AIWalk)
		{
			if (mob[mobnum].seq != mobinfo[mob[mobnum].type].walkseq)
				setmobseq(mobnum, mobinfo[mob[mobnum].type].walkseq);
			j = findmob(mob[mobnum].target);
			dax = sprite[i].x;   //Back up old x&y if stepping off cliff
			day = sprite[i].y;
			osectnum = sprite[i].sectnum;

			if (mob[mobnum].type != MobDog || mob[mobnum].type != MobKinene || mob[mobnum].type != MobOgre)
				doubvel = max(mulscale7(sprite[i].xrepeat,sprite[i].yrepeat),4);
			else
				doubvel = 96;
			if ((mobinfo[mob[mobnum].type].meeleseq != -1)
				&& (mob[j].health > 0) && !(mob[findmob(mob[mobnum].target)].type == MobPlayer && sprite[mob[mobnum].target].cstat&2))
			{
				daang2 = getangle(sprite[mob[mobnum].target].x-sprite[i].x,sprite[mob[mobnum].target].y-sprite[i].y);
				sprite[i].ang = daang2;
				dax2 = (long)sintable[(daang2+512)&2047]*doubvel;
				day2 = (long)sintable[daang2]*doubvel;
				if ((klabs(sprite[i].x-sprite[mob[mobnum].target].x)+klabs(sprite[i].y-sprite[mob[mobnum].target].y)) < 512)
				{
					movestat = 0;
					sprite[i].ang = getangle(sprite[mob[mobnum].target].x-sprite[i].x,sprite[mob[mobnum].target].y-sprite[i].y);
					mob[mobnum].aistate = AIAttack;
					setmobseq(mobnum, mobinfo[mob[mobnum].type].meeleseq);
				}		
			}
			else
			{
				dax2 = (long)sintable[(sprite[i].ang+512)&2047]*doubvel;
				day2 = (long)sintable[sprite[i].ang]*doubvel;
			}
			if (mob[mobnum].type != MobBossEye)
				movestat = movesprite((short)i,mobnum,dax2,day2,0L,4L<<8,4L<<8,CLIPMASK_MONST);
			if (!mob[mobnum].bits[Float])
			{	
				if (mob[mobnum].loz > sprite[i].z+(48<<8))
				{
					sprite[i].x = dax;
					sprite[i].y = day;
					changespritesect(i, osectnum);
					movestat = 1;
				}
				else
					sprite[i].z = mob[mobnum].loz-tilesizy[sprite[i].picnum];
			}
			else
			{
				// float down towards target if too close
				if ((klabs(sprite[i].x-sprite[mob[mobnum].target].x)+klabs(sprite[i].y-sprite[mob[mobnum].target].y)) < 1024)
				{
					if (sprite[i].z > sprite[mob[mobnum].target].z)
						sprite[i].z -= (3<<8);
					else if (sprite[i].z < sprite[mob[mobnum].target].z)
						sprite[i].z += (3<<8);
				}
				if (sprite[i].z > mob[mobnum].loz)
					sprite[i].z = mob[mobnum].loz;
				else if (sprite[i].z < mob[mobnum].hiz)
					sprite[i].z = mob[mobnum].hiz;

			}
			if (((movestat != 0) || (!--mob[mobnum].movecount)) && (mob[mobnum].type != MobBossEye))
			{
				if (sprite[i].ang == (sprite[i].extra&2047))
				{
					daang = (getangle(sprite[mob[mobnum].target].x-sprite[i].x,sprite[mob[mobnum].target].y-sprite[i].y)&2047);
					daang = ((daang+(krand()&1023)-512)&2047);
					sprite[i].extra = ((sprite[i].extra&(~2047))|daang);
				}
				if ((sprite[i].extra-sprite[i].ang)&1024)
				{
					sprite[i].ang = ((sprite[i].ang-32)&2047);
					if (!((sprite[i].extra-sprite[i].ang)&1024)) sprite[i].ang = (sprite[i].extra&2047);
				}
				else
				{
					sprite[i].ang = ((sprite[i].ang+32)&2047);
					if (((sprite[i].extra-sprite[i].ang)&1024)) sprite[i].ang = (sprite[i].extra&2047);
				}
			}
		}
	}
}
