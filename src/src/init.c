/*====================================================================
  This is a private property of Michael Muniko. You may not distribute
  copy or even hack it, without the permission from Michael Muniko.

  Email: michaelmuniko@gmail.com
  ===================================================================*/
#include <fcntl.h>
#include <malloc.h>
#include <i86.h>
#include <dos.h>
#include <io.h>
#include <sys\stat.h>
#include <sys\types.h>
#include "dat.h"
#include "sn.h"
#include "config.h"
#include "controls.h"
#include "sound.h"
#include "sfx.h"
#include "init.h"
#include "weapon.h"
#include "player.h"
#include "str.h"
#include "kbd.h"
#include "timer.h"
#include "stbar.h"
#include "graphics.h"

#pragma aux printchrasm =\
	"rep stosw"\
	parm [edi][ecx][eax]\

#define PROG_ID "Shaw's Nightmare Fotom Pak System Startup 1.9 ["__DATE__"]"
#define PROG_ID2 "Shaw's Nightmare II 1.2 ["__DATE__"]"

extern char datfilename[MAXINDEXES][255];
extern int numdatfiles;
extern long dommxoverlay;

struct meminfo {
	unsigned LargestBlockAvail;
	unsigned MaxUnlokcedPage;
	unsigned LargestLockablePage;
	unsigned LinAddrSpace;
	unsigned NumFreePagesAvail;
	unsigned NumPhysicalPagesFree;
	unsigned TotalPhysicalPages;
	unsigned FreeLinAddrSpace;
	unsigned SizeOfPageFile;
	unsigned Reserved[3];
} MemInfo;

void printstr(short x, short y, char string[81], char attribute)
{
        char character;
        short i, pos;

        pos = (y*80+x)<<1;
        i = 0;
        while (string[i] != 0)
        {
                character = string[i];
                printchrasm(0xb8000+(long)pos,1L,((long)attribute<<8)+(long)character);
                i++;
                pos+=2;
        }
}

int loadopt(void)
{
	long fil;

	if ((fil = open("SN.OPT",O_BINARY|O_RDONLY,S_IREAD)) == -1) return(0);
	read(fil,&detailmode,1);
	read(fil,&screensize,2);
	read(fil,&brightness,2);
	read(fil,&sfxvol,2);
	read(fil,&musicvol,2);
	read(fil,&mousesensitivity,2);
	close(fil);
	return(1);
}

void saveopt(void)
{
	long fil;

	if ((fil = open("SN.OPT",O_BINARY|O_CREAT|O_TRUNC|O_WRONLY,S_IWRITE)) == -1) return;
	write(fil,&detailmode,1);
	write(fil,&screensize,2);
	write(fil,&brightness,2);
	write(fil,&sfxvol,2);
	write(fil,&musicvol,2);
	write(fil,&mousesensitivity,2);
	close(fil);
}

void init(void)
{
	long i, fil;
	char tempbuf[80];
	union REGS regs;
	struct SREGS sregs;

	setvmode(0x3);
	switch (gamemode)
	{
		case sequel:
		strcpy(tempbuf, PROG_ID2);
		break;
		default:
		strcpy(tempbuf, PROG_ID);
		break;
	}
	printstr(0,0,"                                                                                ",0x7E);
        printstr(40-(strlen(tempbuf)>>1),0,tempbuf,0x7E);
	printstr(0,24,"                           Copyright 2007-2017 Mickey Productions                   ",0x7E);
	regs.x.eax = 0x00000500;
	memset(&sregs,0,sizeof(sregs));
	sregs.es = FP_SEG(&MemInfo);
	regs.x.edi = FP_OFF(&MemInfo);
	int386x(0x31, &regs, &regs, &sregs);
	MemInfo.LargestBlockAvail /= 1048576;
	printf("\n%luMB memory available\n", MemInfo.LargestBlockAvail);
	if (MemInfo.LargestBlockAvail < 5)
	{
		printf("You do not have enough memory to run\n");
		printf("Shaw's Nightmare. We can let you continue\n");
		printf("but do not complain if it crashes.\n");
		printf("PRESS ANY KEY TO CONTINUE\n");
		getch();
	}
	if (loadconfig() != 0)
	{
		printf("Error opening configuration. Please run SETUP.\n");
		exit(-1);
	}
	if (!loadopt())
	{
		screensize = 8;
		sfxvol = 240;
		musicvol = 240;
		mousesensitivity = 4;
	}
	printf("Initializing resources archive\n");
	for(i=0;i<numdatfiles;i++)
		loaddat(datfilename[i]);
	printf("Loading strings\n");
	loadstrings();
	printf("Initializing Build 3D engine\n");
	dommxoverlay = 0;
	initengine();
	printf("Initializing keyboard\n");
	initkeys();
	printf("Initializing timer handler\n");
	inittimer();
	printf("Loading tiles\n");
	if (loadpics(tilefilename) == -1)
		error("Cannot load tile file", 6000);
	tilesizx[MIRROR] = 0;
	tilesizy[MIRROR] = 0;
	loadvoxels();
	printf("Initializing status bar\n");
	initstatusbar();
	printf("Initializing weapon animations\n");
	initweapon();
	initmovers();
	if (inputdevice == 1) initmouse();
	for(i=1;i<100;i++)
	{
		sprintf(tempbuf, "SN%.2d.DEM", i);
		if (access(tempbuf, R_OK) == 0)
			numdemos++;		
	}
	printf("There are %d demo(s) in the loop\n", numdemos);
	printf("Initializing sound engine\n");
	initsound();

	initgraph();

	zoom = 768;
	pskyoff[0] = 0; pskyoff[1] = 0; pskybits = 1;
	parallaxtype = 1;
}

void uninit(void)
{
	clearview(0L);
	uninitengine();
	uninitkeys();
	uninittimer();
	uninitsound();
	saveopt();
	setvmode(0x3);
}
