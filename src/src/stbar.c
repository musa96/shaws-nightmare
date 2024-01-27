#include "sn.h"
#include "graphics.h"
#include "weapon.h"
#include "player.h"
#include "movers.h"

int statusclock, statusrandomnumber;

void initstatusbar(void)
{
	loadtile(STATUSBAR);
}

void drawface(void)
{
	short facepic;
	
	if (!mob[playermob].health) facepic = FaceDead;
	if (mob[playermob].health > 0) facepic = Face5+(statusrandomnumber%3);
	if (mob[playermob].health >= 20) facepic = Face4+(statusrandomnumber%3);
	if (mob[playermob].health >= 40) facepic = Face3+(statusrandomnumber%3);
	if (mob[playermob].health >= 60) facepic = Face2+(statusrandomnumber%3);
	if (mob[playermob].health >= 80) facepic = Face1+(statusrandomnumber%3);
	if ((undead == 1) || (powerup[Invulnerability])) facepic = FaceUndead;

	rotatesprite(149<<16,(200-32)<<16,65536L,0,facepic,0,0,10+16+128,
				0,0,xdim-1,ydim-1);
}

void drawstatusbar(void)
{
	int i, j, curammo;
	char pal[NumWeapons];

	statusclock++;
	if (statusclock > 100)
	{
		statusclock = 0;
		statusrandomnumber = rand();
	}
	rotatesprite(0,(200-32)<<16,65536L,0,STATUSBAR,0,0,10+16+64+128,
			0,0,xdim-1,ydim-1);

	switch(curweapon)
	{
		case MiniCatapult:
		case RapidCatapult:
		curammo = ammo[Stones];
		break;
		case Doberwand:
		case SuperDoberwand:
		curammo = ammo[Crystals];
		break;
		case Launcher:
		curammo = ammo[HenGrenades];
		break;
		case CameraGun:
		case Cannon:
		curammo = ammo[Batteries];
		break;
		default:
		curammo = -1;
		break;
	}
	if (curammo != -1)
		putnumber(19,(200-27),curammo);

	// show health
	putnumber(70, (200-27), mob[playermob].health);

	// draw keys
	if (havekey[0])	
		rotatesprite(231<<16,(200-31)<<16,65536L,0,StatKey1,0,0,10+16+128,0,0,xdim-1,ydim-1);

	if (havekey[1])	
		rotatesprite(231<<16,(200-20)<<16,65536L,0,StatKey2,0,0,10+16+128,0,0,xdim-1,ydim-1);

	if (havekey[2])	
		rotatesprite(231<<16,(200-7)<<16,65536L,0,StatKey3,0,0,10+16+128,0,0,xdim-1,ydim-1);

	// show arms
	for(i=MiniCatapult;i<NumWeapons;i++)
	{
		if (haveweapon[i]) pal[i] = 0;
		else pal[i] = 1;
	}
	putsmallnumberpal(113, 200-27, 2, pal[MiniCatapult]);
	putsmallnumberpal(121, 200-27, 3, pal[Doberwand]);
	putsmallnumberpal(129, 200-27, 4, pal[RapidCatapult]);
	putsmallnumberpal(113, 200-18, 5, pal[Launcher]);
	putsmallnumberpal(121, 200-18, 6, pal[CameraGun]);
	putsmallnumberpal(129, 200-18, 7, pal[Cannon]);

	// show armor
	putnumber(210, (200-27), armor);

	for(i=0;i<NumAmmoTypes;i++)
	{
		putsmallnumber(280, 200-(32-i*8), ammo[i]);
		putsmallnumber(305, 200-(32-i*8), ammomax[i]);
	}
	drawface();
}

void putmessage(char *msg)
{
	msgtime = 20*11;
	message = msg;
	borderdraw = 1;
}
