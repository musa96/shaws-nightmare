#include "kbd.h"
#include "sn.h"
#include "joys.h"
#include "controls.h"
#include "config.h"
#include "weapon.h"
#include "map2d.h"
#include "graphics.h"
#include "player.h"
#include "demo.h"

short oflags[NumControls], oldmousebstatus = 0;
short mousesensitivity = 4, aim_mode;
INPUT sync;

void getinput(void)
{
	short mousx, mousy, bstatus;

	if (demoplayback)
	{
		if (death == 0)
			copybufbyte(&recsync[curdemoframe],&sync,sizeof(INPUT));
		curdemoframe++;
		return;
	}

	if (death != 0 || paused) return;

	if (keystatus[56] == 0)
	{
		if (keystatus[key[2]] > 0) sync.angvel = max(sync.angvel-16,-128);
		if (keystatus[key[3]] > 0) sync.angvel = min(sync.angvel+16,127);
	}
	else
	{
		if (keystatus[key[2]] > 0) sync.svel = min(sync.svel+8,127);
		if (keystatus[key[3]] > 0) sync.svel = max(sync.svel-8,-128);
	}
	if (keystatus[key[0]] > 0) sync.vel = min(sync.vel+8,127);
	if (keystatus[key[1]] > 0) sync.vel = max(sync.vel-8,-128);
	if (keystatus[key[4]] > 0) sync.svel = min(sync.svel+8,127);
	if (keystatus[key[5]] > 0) sync.svel = max(sync.svel-8,-128);

	getmousevalues(&mousx,&mousy,&bstatus);
	sync.bits[Activate] = (keystatus[key[6]]|((bstatus&6)>oldmousebstatus&6));
	sync.bits[Run] = keystatus[key[7]];
	sync.bits[LookUp] = keystatus[key[8]];
	sync.bits[LookDown] = keystatus[key[9]];
	sync.bits[LookCentre] = keystatus[key[10]];
	sync.bits[Jump] = keystatus[key[11]];
	sync.bits[Crouch] = keystatus[key[12]];
	sync.bits[Fire] = (keystatus[key[13]]|((bstatus&1)>oldmousebstatus&1));
	sync.bits[Weapon1] = keystatus[2];
	sync.bits[Weapon2] = keystatus[3];
	sync.bits[Weapon3] = keystatus[4];
	sync.bits[Weapon4] = keystatus[5];
	sync.bits[Weapon5] = keystatus[6];
	sync.bits[Weapon6] = keystatus[7];
	sync.bits[Weapon7] = keystatus[8];

	sync.angvel = min(max(sync.angvel+(mousx*(mousesensitivity+5)/100),-128),127);
	if (aim_mode == 0)
		sync.vel = min(max(sync.vel-(mousy*(mousesensitivity+5)/100),-128),127);
	else
		horiz = min(max(horiz-(mousy*(mousesensitivity+5)/100),-100),300);

	oldmousebstatus = bstatus;
	if (inputdevice == 2)
	{
		if (joystickposition(JoystickAAxisX) > 0)
			sync.angvel = min(sync.angvel+16,127);
		if (joystickposition(JoystickAAxisY) > 0)
			sync.vel = min(sync.vel+8,127);
		if (joystickposition(JoystickBAxisX) > 0)
			sync.angvel = max(sync.angvel-16,-128);
		if (joystickposition(JoystickBAxisY) > 0)
			sync.vel = max(sync.vel-8,127);
		if (joystickbutton(JoystickAButton1) > 0)
			sync.bits[Fire] = 1;
		if (joystickbutton(JoystickAButton2) > 0)
			sync.bits[Activate] = 1;
	}
	if (demorecording)
	{
		copybufbyte(&sync,&recsync[numdemoframes],sizeof(INPUT));
	}
}
