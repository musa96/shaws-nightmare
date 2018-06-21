#include "sn.h"
#include "graphics.h"
#include "weapon.h"
#include "player.h"
#include "qaf.h"
#include "controls.h"
#include "sfx.h"
#include "movers.h"
#include "triggers.h"

#define FBATT 48

long weapx[8], weapy[8];
int bobtable[16] = {0, 2, 4, 6, 4, 2, 0, -2, -4, -6, -8, -6, -4, -2, 0};
short haveweapon[NumWeapons], curweapon, newweap;
int ammo[NumAmmoTypes], ammomax[NumAmmoTypes];
int defaultammomax[NumAmmoTypes] = {250, 50, 50, 488};
int packcontents[NumAmmoTypes] = {50, 8, 2, 48};
int infinteammo, weapstate;
short hitsect, hitwall, hitsprite;
SeqPoint_T weapseqpoint;

static char *weaponqafnames[QAFEnd] =
{
	"MCUP.QAF",
	"MCDOWN.QAF",
	"MCIDLE.QAF",
	"MCPOKE.QAF",

	"MINIUP.QAF",
	"MINIDOWN.QAF",
	"MINIIDLE.QAF",
	"MINIFIRE.QAF",

	"DBUP.QAF",
	"DBDOWN.QAF",
	"DBIDLE.QAF",
	"DBFIRE.QAF",

	"RPDUP.QAF",
	"RPDDOWN.QAF",
	"RPDIDLE.QAF",
	"RPDFIRE.QAF",

	"HGNUP.QAF",
	"HGNDOWN.QAF",
	"HGNIDLE.QAF",
	"HGNFIRE.QAF",

	"CAMUP.QAF",
	"CAMDOWN.QAF",
	"CAMIDLE.QAF",
	"CAMFIRE.QAF",

	"CANUP.QAF",
	"CANDOWN.QAF",
	"CANIDLE.QAF",
	"CANFIRE.QAF",

	"FLAMUP.QAF",
	"FLAMDOWN.QAF",
	"FLAMIDLE.QAF",
	"FLAMFIRE.QAF",

	"SDBUP.QAF",
	"SDBDOWN.QAF",
	"SDBIDLE.QAF",
	"SDBFIRE.QAF"
};
Qaf_T curweaponqaf;
static Qaf_T weaponqaf[QAFEnd];

int checkammo(int weapon)
{
	if (infinteammo) return(0);

	switch(weapon)
	{
		case Machete:
		case Flamer:
		return(0);
		break;
		case MiniCatapult:
		case RapidCatapult:
		if (ammo[Stones] > 0) return(0);
		break;
		case Doberwand:
		case SuperDoberwand:
		if (ammo[Crystals] > 0) return(0);
		break;
		case Launcher:
		if (ammo[HenGrenades] > 0) return(0);
		break;
		case CameraGun:
		if (ammo[Batteries] > 0) return(0);
		break;
		case Cannon:
		if (ammo[Batteries] > FBATT) return(0);
		break;
	}
	return(-1);
}

int giveweapon(int weapon)
{
	int ammonum, amount, gaveweapon, gaveammo;
	
	ammonum = -1;
	switch(weapon)
	{
		case Doberwand:
		ammonum = Crystals;
		amount = 8;
		break;
		case SuperDoberwand:
		ammonum = Crystals;
		amount = 16;
		break;
		case RapidCatapult:
		ammonum = Stones;
		amount = 50;
		break;
		case Launcher:
		ammonum = HenGrenades;
		amount = 2;
		break;
		case CameraGun:
		ammonum = Batteries;
		amount = 48;
		break;
		case Cannon:
		ammonum = Batteries;
		amount = 144;
		break;
	}
	if (ammonum != -1)
	{
		if (addammo(ammonum, amount) == 1) gaveammo = 1;
		else gaveammo = 0;
	}
	else gaveammo = 0;
	
	if (!haveweapon[weapon])
	{
		gaveweapon = 1;
		haveweapon[weapon] = 1;
		changeweapon(weapon);
	}
	else
		gaveweapon = 0;

	return(gaveweapon || gaveammo);
}

int addammo(int ammotype, int amount)
{
	if (ammo[ammotype] >= ammomax[ammotype]) return(0);

	ammo[ammotype] += amount;
	if (ammo[ammotype] > ammomax[ammotype])
		ammo[ammotype] = ammomax[ammotype];
	return(1);
}

void useammo(int ammotype, int amount)
{
	if (infinteammo) return;

	ammo[ammotype] -= amount;
	if (ammo[ammotype] < 0) ammo[ammotype] = 0;
}

void spawnglass(short wallnum, short n)
{
	long i, j, k, x1, y1, z, xvect, yvect;
	short angle, sectnum;

	j = n+1;

	x1 = wall[wallnum].x;
	y1 = wall[wallnum].y;

	xvect = wall[wall[wallnum].point2].x-x1;
	yvect = wall[wall[wallnum].point2].y-y1;

	x1 -= ksgn(yvect);
	y1 += ksgn(xvect);

	xvect /= j;
	yvect /= j;

	for(j=n;j>0;j--)
	{
		x1 += xvect;
		y1 += yvect;
		updatesector(x1,y1,&sectnum);
		if (sectnum >= 0)
		{
			z = sector[sectnum].floorz-(krand()&(klabs(sector[sectnum].ceilingz-sector[sectnum].floorz)));
			if (z < -(32<<8) || z > (32<<8))
				z = sector[sectnum].floorz-(32<<8)+(krand()&((64<<8)-1));
			spawnsprite(i,x1,y1,z,0,sector[sectnum].floorshade,
				    0,32,64,64,0,0,GlassShard,0,32+(krand()&63),0,-(krand()&1023),-1,sectnum,1,
				    0,0,0,0,0);
			k = addmob(i, MobGlassShard);
			setmobseq(k, SeqGlassShard);
		}
	}
}

void breakwindow(short wallnum)
{
	long i;

	i = wall[wallnum].cstat;
	i &= ~1;
	i &= ~16;
	i &= ~64;
	wall[wallnum].cstat = i;
	wall[wall[wallnum].nextwall].cstat = i;
	wall[wallnum].lotag = 0;
	wall[wall[wallnum].nextwall].lotag = 0;
	playxysfx(SfxGlass, 1, wall[wallnum].x, wall[wallnum].y);
	spawnglass(wallnum, 10);
}

int checkmeelerange(long x1, long x2, long y1, long y2)
{
	if ((klabs(x1-x2)+klabs(y1-y2)) > 512)
		return(0);

	return(1);
}

void firehitscan(long x, long y, long z, short angle,
		short sectnum, long z2, int damage, int attacker, int range)
{
	short ang2;
	long hitx, hity, hitz, i, j, k;

	ang2 = ((angle + (krand()&31)-16)&2047);

	hitscan(x,y,z,sectnum,                   //Start position
		sintable[(ang2+512)&2047],            //X vector of 3D ang
		sintable[ang2&2047],                  //Y vector of 3D ang
		z2,                                   //Z vector of 3D ang
		&hitsect,&hitwall,&hitsprite,&hitx,&hity,&hitz,CLIPMASK1);


	if (range == CLOSERANGE)
		if (checkmeelerange(x, hitx, y, hity) == 0)
			return;
			
	spawnsprite(j,hitx,hity,hitz,2,-4,0,32,64,64,0,0,
			Puff,angle,0,0,0,4096,hitsect,7,0,0,0,0,0);
	i = addmob(j, MobPuff);
	setmobseq(i, mobinfo[MobPuff].walkseq);

	if (hitsect >= 0)
	{
		if (z2 < 0)
		{
			if (sector[hitsect].ceilingstat&1)
			{
				sprite[j].xrepeat = 0;
				sprite[j].yrepeat = 0;
			}
		}
		else if (z2 > 0)
		{
			if (sector[hitsect].floorstat&1)
			{
				sprite[j].xrepeat = 0;
				sprite[j].yrepeat = 0;
			}
		}
	}
	if (hitsprite >= 0)
	{
		if (sprite[hitsprite].picnum == FTOSHOOT)
			sprite[hitsprite].picnum = FTOSHOOT+1;
		switch (sprite[hitsprite].lotag)
		{
			case 46:
			sprite[hitsprite].lotag = 0;
			for(i=0;i<numwalls;i++)
			{
				if (wall[i].lotag == sprite[hitsprite].hitag)
				{
					j = wall[i].cstat;
					j &= ~1;
					j &= ~16;
					j &= ~64;
					wall[i].cstat = j;
					wall[wall[i].nextwall].cstat = j;
				}
			}
			break;
			case 59:
			sprite[hitsprite].lotag = 0;
			for(i=0;i<numsectors;i++)
			{
				if (sector[i].lotag == sprite[hitsprite].hitag)
					addlift(i, 102, sector[i].hitag);
			}
			break;
		}
		for(i=0;i<nummobs;i++)
		{
			if (mob[i].spritenum == hitsprite)
			{
				sprite[j].xrepeat = 0;
				sprite[j].yrepeat = 0;
				if (!mob[i].bits[SpawnPuff])
				{
					spawnsprite(j,hitx,hity,hitz+(8<<8),0,-4,0,32,64,64,0,0,
						Blood,angle,0,0,0,4096,hitsect,7,0,0,0,1,MobBlood);
				}
				else
				{
					spawnsprite(j,hitx,hity,hitz+(8<<8),0,-4,0,32,64,64,0,0,
						Puff,angle,0,0,0,4096,hitsect,7,0,0,0,0,0);
					k = addmob(j, MobPuff);
					setmobseq(k, mobinfo[MobPuff].walkseq);
				}
				hurtmob(i, damage, attacker);
			}
		}
	}
	if (hitwall >= 0)
	{
		switch (wall[hitwall].lotag)
		{
			case 100:
			sprite[j].xrepeat = 0;
			sprite[j].yrepeat = 0;
			breakwindow(hitwall);
			break;
			case 88:
			for(i=0;i<numsectors;i++)
			{
				if (wall[hitwall].hitag == sector[i].lotag)
					operatezdoor(i);
			}
			break;
		}
	}
}

void fireprojectile(long x, long y, long z, short angle,
		    short sectnum, long dahoriz, int mobtype, int attacker)
{
	short ang2;
	long i, z2;

	ang2 = ((angle + (krand()&31)-16)&2047);
	z2 = ((100-dahoriz)*2000) + ((krand()-32768)>>1);

	spawnsprite(i,x,y,z,1+128,0,0,32,64,64,0,0,mobinfo[mobtype].standpicnum,angle,
		sintable[(angle+512)&2047]>>4,sintable[angle&2047]>>4,
		(100-dahoriz)<<7,attacker,sectnum,4,0,0,0,1,mobtype);
}

void changeweapon(int weapon)
{
	if (haveweapon[weapon] != 1) return;
	if (checkammo(weapon) != 0) return;
	if (weapon == curweapon) return;
	if (weapstate != WEAP_IDLE) return;

	newweap = weapon;
	weapseqpoint.frame = 0;
	weapseqpoint.tics = 0;
	weapstate = WEAP_DOWN;
}

void initweapon(void)
{
	long i, j;

	if (gamemode == sequel) j = 0;
	else j = 4;
	for(i=0;i<QAFEnd-j;i++)
	{
		loadqaf(weaponqafnames[i], &weaponqaf[i]);
	}
}

void daggerpoke(void)
{
	int damage;
	long z2;

	damage = 5 + (powerup[Kerzerk]>>3);
	z2 = ((100-horiz)*2000) + ((krand()-32768)>>1);
	firehitscan(posx,posy,posz,ang,cursectnum, z2, damage, playersprite, CLOSERANGE);
}

void flamer(void)
{
	long z2;
	short angle;

	hitsprite = -1;
	z2 = ((100-horiz)*2000) + ((krand()-32768)>>1);
	firehitscan(posx,posy,posz,ang,cursectnum, z2, 5, playersprite, CLOSERANGE);
	if (hitsprite >= 0)
	{
		angle = getangle(posx-sprite[hitsprite].x,
				 posy-sprite[hitsprite].y);
		ang = angle - 1024;
		playsfx(SfxFlamerHit, 0, 255);
	}
	else
		playsfx(SfxFlamerFire, 0, 255);
}

void firecatapult(void)
{
	long z2;

	z2 = ((100-horiz)*2000) + ((krand()-32768)>>1);
	firehitscan(posx,posy,posz,ang,cursectnum, z2, 7, playersprite, FARRANGE);
	useammo(Stones, 1);
	playsfx(SfxCatFire, 0, 255);
}

void firerapid(void)
{
	long z2;

	z2 = ((100-horiz)*2000) + ((krand()-32768)>>1);
	firehitscan(posx,posy,posz,ang,cursectnum, z2, 7, playersprite, FARRANGE);
	useammo(Stones, 1);
	playsfx(SfxRapidFire, 0, 255);
}

void firewand(void)
{
	long i, z2;

	for(i=0;i<7;i++)
	{
		z2 = ((100-horiz)*2000) + ((krand()-32768)>>1);
		firehitscan(posx,posy,posz,ang,cursectnum, z2, 7, playersprite, FARRANGE);
	}
	useammo(Crystals, 1);
	playsfx(SfxWandFire, 0, 255);
}

void firesuperwand(void)
{
	long i, z2;

	for(i=0;i<14;i++)
	{
		z2 = ((100-horiz)*2000) + ((krand()-32768)>>1);
		firehitscan(posx,posy,posz,ang,cursectnum, z2, 7, playersprite, FARRANGE);
	}
	useammo(Crystals, 2);
	playsfx(SfxSuperFire, 0, 255);
}

void firelauncher(void)
{
	fireprojectile(posx, posy, posz, ang, cursectnum,
			horiz, MobHen, playersprite);
	useammo(HenGrenades, 1);
	playsfx(SfxHenFire, 0, 255);
}

void firecamera(void)
{
	fireprojectile(posx, posy, posz, ang, cursectnum,
			horiz, MobCPhoto, playersprite);
	useammo(Batteries, 1);
	playsfx(SfxCameraGun, 0, 255);
}

void firecannon(void)
{
	fireprojectile(posx, posy, posz, ang, cursectnum, horiz, MobFusion,
			playersprite);
	useammo(Batteries, FBATT);
	playsfx(SfxCannonFire, 0, 255);
}

void fireweapon(void)
{
	if (checkammo(curweapon) == -1) return;
	if (weapstate != WEAP_IDLE) return;

	weapseqpoint.tics = 0;
	weapseqpoint.frame = 0;
	weapstate = WEAP_FIRE;
}

void fusionspray(void)
{
	long i, j, k;
	long hitx, hity, hitz;
	short angle, damage;

	for(i=0;i<40;i++)
	{
		angle = ang - 256 + 512/40*i;

		hitscan(posx,posy,posz,cursectnum,                   //Start position
			sintable[(angle+512)&2047],            //X vector of 3D ang
			sintable[angle&2047],                  //Y vector of 3D ang
			((100-horiz)*2000) + ((krand()-32768)>>1),
			&hitsect,&hitwall,&hitsprite,&hitx,&hity,&hitz,CLIPMASK1);
		if (hitsprite < 0) continue;

		spawnsprite(j,hitx,hity,hitz,0,-4,0,32,64,64,0,0,
				Puff,angle,0,0,0,4096,hitsect,7,0,0,0,0,0);
		k = addmob(j, MobFusionSpray);
		setmobseq(k, mobinfo[MobFusionSpray].walkseq);

		damage = 0;
		for(j=0;j<15;j++)
			damage += (krand()&7) + 1;

		k = findmob(hitsprite);
		hurtmob(k, damage, playersprite);
	}
}

void weaponupdatestate(void)
{
	if (weapstate == WEAP_FIRE)
	{
		switch (curweapon)
		{
			case Machete:
			curweaponqaf = weaponqaf[QAFMachetePoke];
			break;
			case MiniCatapult:
			curweaponqaf = weaponqaf[QAFMiniFire];
			break;
			case Doberwand:
			curweaponqaf = weaponqaf[QAFWandFire];
			break;
			case RapidCatapult:
			curweaponqaf = weaponqaf[QAFRapidFire];
			break;
			case Launcher:
			curweaponqaf = weaponqaf[QAFLauncherFire];
			break;
			case CameraGun:
			curweaponqaf = weaponqaf[QAFCameraFire];
			break;
			case Cannon:	
			curweaponqaf = weaponqaf[QAFCannonFire];
			break;
			case Flamer:
			curweaponqaf = weaponqaf[QAFFlamerFire];
			break;
			case SuperDoberwand:	
			curweaponqaf = weaponqaf[QAFSuperFire];
			break;
		}
	}
	else if (weapstate == WEAP_IDLE)
	{
		switch (curweapon)
		{
			case Machete:
			curweaponqaf = weaponqaf[QAFMacheteIdle];
			break;
			case MiniCatapult:
			curweaponqaf = weaponqaf[QAFMiniIdle];
			break;
			case Doberwand:
			curweaponqaf = weaponqaf[QAFWandIdle];
			break;
			case RapidCatapult:
			curweaponqaf = weaponqaf[QAFRapidIdle];
			break;
			case Launcher:
			curweaponqaf = weaponqaf[QAFLauncherIdle];
			break;
			case CameraGun:
			curweaponqaf = weaponqaf[QAFCameraIdle];
			break;
			case Cannon:	
			curweaponqaf = weaponqaf[QAFCannonIdle];
			break;
			case Flamer:
			curweaponqaf = weaponqaf[QAFFlamerIdle];
			break;
			case SuperDoberwand:	
			curweaponqaf = weaponqaf[QAFSuperIdle];
			break;
		}
	}
	else if (weapstate == WEAP_DOWN)
	{
		switch (curweapon)
		{
			case Machete:
			curweaponqaf = weaponqaf[QAFMacheteDown];
			break;
			case MiniCatapult:
			curweaponqaf = weaponqaf[QAFMiniDown];
			break;
			case Doberwand:
			curweaponqaf = weaponqaf[QAFWandDown];
			break;
			case RapidCatapult:
			curweaponqaf = weaponqaf[QAFRapidDown];
			break;
			case Launcher:
			curweaponqaf = weaponqaf[QAFLauncherDown];
			break;
			case CameraGun:
			curweaponqaf = weaponqaf[QAFCameraDown];
			break;
			case Cannon:	
			curweaponqaf = weaponqaf[QAFCannonDown];
			break;
			case Flamer:
			curweaponqaf = weaponqaf[QAFFlamerDown];
			break;
			case SuperDoberwand:	
			curweaponqaf = weaponqaf[QAFSuperDown];
			break;
		}
	}
	else if (weapstate == WEAP_UP)
	{
		switch (curweapon)
		{
			case Machete:
			curweaponqaf = weaponqaf[QAFMacheteUp];
			break;
			case MiniCatapult:
			curweaponqaf = weaponqaf[QAFMiniUp];
			break;
			case Doberwand:
			curweaponqaf = weaponqaf[QAFWandUp];
			break;
			case RapidCatapult:
			curweaponqaf = weaponqaf[QAFRapidUp];
			break;
			case Launcher:
			curweaponqaf = weaponqaf[QAFLauncherUp];
			break;
			case CameraGun:
			curweaponqaf = weaponqaf[QAFCameraUp];
			break;
			case Cannon:	
			curweaponqaf = weaponqaf[QAFCannonUp];
			break;
			case Flamer:
			curweaponqaf = weaponqaf[QAFFlamerUp];
			break;
			case SuperDoberwand:	
			curweaponqaf = weaponqaf[QAFSuperUp];
			break;
		}
	}

}

void processweapon(void)
{
	int rc;
	long i, j;

	if (death != 0 || playerabouttoexit) return;

	weaponupdatestate();

	rc = playqaf(&curweaponqaf, &weapseqpoint.tics, &weapseqpoint.frame);
	if (rc == 0)
	{
		switch (weapstate)
		{
			case WEAP_FIRE:
			if ((sync.bits[Fire] > 0) && (checkammo(curweapon) == 0))
			{
				weapseqpoint.tics = 0;
				weapseqpoint.frame = 0;
			}
			else
			{
				weapseqpoint.frame = 0;
				weapseqpoint.tics = 0;
				weapstate = WEAP_IDLE;
				if (checkammo(curweapon) != 0)
				{
					if (haveweapon[CameraGun]
						&& ammo[Batteries])
					{
						changeweapon(CameraGun);
					}
					else if (haveweapon[RapidCatapult]
						&& ammo[Stones])
					{
						changeweapon(RapidCatapult);
					}
					else if (haveweapon[Cannon]
						&& ammo[Batteries] > FBATT)
					{
						changeweapon(Cannon);
					}
					else if (haveweapon[Launcher]
						&& ammo[HenGrenades])
					{
						changeweapon(Launcher);
					}						
					else if (haveweapon[Doberwand]
						&& ammo[Crystals])
					{
						changeweapon(Doberwand);
					}
					else if (haveweapon[MiniCatapult]
						&& ammo[Stones])
					{
						changeweapon(MiniCatapult);
					}
					else if (haveweapon[Flamer])
					{
						changeweapon(Flamer);
					}
					else
						changeweapon(Machete);
				}
			}
			break;
			case WEAP_UP:
			weapstate = WEAP_IDLE;
			break;
			case WEAP_DOWN:
			weapstate = WEAP_UP;
			if (newweap == Flamer)
				playsfx(SfxFlamerUp, 0, 255);

			curweapon = newweap;
			break;
		}
		weaponupdatestate();
		weapseqpoint.frame = 0;
		weapseqpoint.tics = 0;
	}
	if (weapstate == WEAP_IDLE)
	{
		if (curweapon == Flamer && weapseqpoint.frame == 0)
			playsfx(SfxFlamerIdle, 0, 255);

		for (i=0;i<curweaponqaf.numlayers;i++)
		{
			weapx[i] = curweaponqaf.layer[i].frame[weapseqpoint.frame].x + (bobtable[bob]<<16);
			weapy[i] = curweaponqaf.layer[i].frame[weapseqpoint.frame].y + (bobtable[bob]<<16);
		}
	}
}

void qaftrigger(int trigger)
{
	long i;

	switch (trigger)
	{
		case 1:
		firecatapult();
		break;
		case 2:
		firewand();
		break;
		case 3:
		firelauncher();
		break;
		case 4:
		firecamera();
		break;
		case 5:
		daggerpoke();
		break;
		case 6:
		flamer();
		break;
		case 7:
		firecannon();
		break;
		case 8:
		firerapid();
		break;
		case 9:
		firesuperwand();
		break;
		case 10:
		playsfx(SfxSuperReload, 0, 255);
		break;
	}
}

void drawweapon(void)
{
	long i;
	char stat;

	if (death != 0) return;

	if (powerup[Invisibility] > 4*32
	    || powerup[Invisibility]&8)
		stat = 1+2;
	else
		stat = 2;
	if (weapstate == WEAP_IDLE)
	{
		for(i=0;i<curweaponqaf.numlayers;i++)
		{
			curweaponqaf.layer[i].frame[weapseqpoint.frame].x = weapx[i];
			curweaponqaf.layer[i].frame[weapseqpoint.frame].y = weapy[i];
		}
	}
	drawqaf(&curweaponqaf, weapseqpoint.frame,
		sector[cursectnum].floorshade, stat);
}
