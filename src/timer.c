#include <dos.h>
#include "sn.h"
#include "config.h"
#include "timer.h"
#include "mmd.h"

volatile long tick;
long waitclock;
void (__interrupt __far *oldtimerhandler)();
void __interrupt __far timerhandler(void);

void inittimer(void)
{
	outp(0x43,0x34);
	outp(0x40,(1193181/TICRATE)&255);
	outp(0x40,(1193181/TICRATE)>>8);
	oldtimerhandler = _dos_getvect(0x8);
	_disable(); _dos_setvect(0x8, timerhandler); _enable();
}

void uninittimer(void)
{
	outp(0x43,0x34); outp(0x40,0); outp(0x40,0);
	_disable(); _dos_setvect(0x8, oldtimerhandler); _enable();
}

void __interrupt __far timerhandler()
{
	totalclock++;
	tick++;
	updatesounds();
	mmdupdate();
	outp(0x20,0x20);
}

void bdelay(int time)
{
	waitclock = time;
	while (waitclock > 0)
	{
		if (tick > 5)
		{
			tick = 0;
			waitclock--;
		}
	}
}

void faketimerhandler(void)
{
}
