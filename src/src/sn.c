#include <fcntl.h>
#include <conio.h>
#include <io.h>
#include <dos.h>
#include <sys\stat.h>
#include <sys\types.h>
#include "sn.h"
#include "config.h"
#include "maps.h"
#include "graphics.h"
#include "weapon.h"
#include "player.h"
#include "controls.h"
#include "demo.h"
#include "epiend.h"
#include "levels.h"
#include "saveg.h"
#include "init.h"
#include "menus.h"
#include "kbd.h"
#include "qaf.h"
#include "sound.h"
#include "seq.h"
#include "sfx.h"
#include "dat.h"
#include "timer.h"

static char *intronames[4] =
{
	"INTRO1.QAF",
	"",
	"INTRO2A.QAF",
	"INTRO2B.QAF"
};
static Qaf_T introqaf;
extern Qaf_T curweaponqaf;
int introtics, introframe;
char datfilename[MAXINDEXES][255];
int curdemo, numdemos, numdatfiles;
int gamestate, oldgamestate, custom_map, nomonsters;
int gameepisode, gamelevel, gameskill;
int paused, gamemode;
int respawnparm;
char soundfilename[255] = "SOUNDS.DAT";
char boardfilename[13], tilefilename[255] = "tiles000.art";
int pagetic = 200;

void display(void)
{
	char tempbuf[80];
	int dream, done;

	switch(gamestate)
	{
		case GAME:
		renderscreen();
		break;
		case INTERMISSION:
		drawintermission();
		break;
		case ENDOFEPISODE:
		drawend();
		break;
		default:
		setview(0L,0L,xdim-1,ydim-1);
		clearview(0L);
		break;
	}
	if (gamestate != oldgamestate && gamestate != GAME)
		setbrightness(brightness, palette);
	oldgamestate = gamestate;
}

void intro2(void)
{
	long i;
	char tempbuf[13];
	int curintroqaf = 0;

	gamestate = INTRO;
	introtics = 0;
	introframe = 0;
	curdemo = 1;
	setview(0L,0L,xdim-1,ydim-1);
	playsong(sng_Theme, 1);
	while (!keystatus[28] && curintroqaf <= 5)
	{
		if (tick > 5)
		{
			tick = 0;
			if (curintroqaf)
				i = playqaf(&introqaf, &introtics, &introframe);
			else
			{
				i = 1;
				if (--pagetic < 0) i = 0;
			}
			if (i == 0)
			{
				 curintroqaf++;
				 if (curintroqaf <= 5)
				 {
					sprintf(tempbuf, "INTRO%d.QAF", curintroqaf);
					loadqaf(tempbuf, &introqaf);
					introframe = 0;
					introtics = 0;
				 }
			}
		}
		clearview(0L);
		if (curintroqaf)
			drawqaf(&introqaf, introframe, 0, 2);
		else
			rotatesprite(0,0,65536L,0,TitlePic,0,0,2+8+16+64,0,0,xdim-1,ydim-1);
		nextpage();
	}
	keystatus[28] = 0;
	menustatus = 1;
}

void intro(void)
{
	long i;
	int curintroqaf = 0;

	gamestate = INTRO;
	introtics = 0;
	introframe = 0;
	curdemo = 1;
	setview(0L,0L,xdim-1,ydim-1);
	loadqaf(intronames[0], &introqaf);
	if (musicsource > 1)
		playsong(sng_s3, 1);
	else
		playsong(sng_s31, 1);
	while (curintroqaf <= 3 && !keystatus[28])
	{
		if (tick > 5)
		{
			tick = 0;
			if (curintroqaf != 1)
				i = playqaf(&introqaf, &introtics, &introframe);
			else
			{
				i = 1;
				if (--pagetic < 0) i = 0;
			}
			if (i == 0)
			{
				curintroqaf++;
				if (curintroqaf <= 3 && curintroqaf != 1)
				{
					loadqaf(intronames[curintroqaf], &introqaf);
					introtics = 0;
					introframe = 0;
				}
			}
		}
		clearview(0L);
		if (curintroqaf != 1)
			drawqaf(&introqaf, introframe, 0, 2);
		else
			rotatesprite(0,0,65536L,0,TitlePic,0,0,2+8+16+64,0,0,xdim-1,ydim-1);
		nextpage();
	}
	keystatus[28] = 0;
	menustatus = 1;
}

void initnew(void)
{
	long i;

	gamestate = GAME;
	temphealth = mobinfo[MobPlayer].starthealth;
	curweapon = MiniCatapult;
	death = 0;
	armor = 0;
	armortype = 0;
	bob = 0;
	boompack = 0;
	haveweapon[Machete] = 1;
	haveweapon[MiniCatapult] = 1;
	for(i=Doberwand;i<NumWeapons;i++)
		haveweapon[i] = 0;
	ammo[Stones] = 50;
	for(i=Crystals;i<NumAmmoTypes;i++)
		ammo[i] = 0;
	horiz = 100;
	weapstate = WEAP_UP;
	clearbufbyte(&weapseqpoint,sizeof(SeqPoint_T),0L);
	clearbufbyte(&curweaponqaf,sizeof(Qaf_T),0L);
	for(i=Stones;i<NumAmmoTypes;i++)
		ammomax[i] = defaultammomax[i];
}

void addfile(char *filename)
{
	strcpy(datfilename[numdatfiles],filename);
	numdatfiles++;
}

void identifyversion(int argc, char *argv[])
{
	long i;

	for(i=1;i<argc;i++)
	{
		if (stricmp(argv[i], "-seqdev") == 0)
		{
			gamemode = sequel;
			addfile("D:/SN2/SN2.DAT");
			strcpy(tilefilename, "D:/SN2/tiles000.art");
			strcpy(soundfilename, "D:/SN2/sounds.dat");
			return;
		}
		if (stricmp(argv[i], "-sn1dev") == 0)
		{
			gamemode = fullversion;
			addfile("D:/SN/SN.DAT");
			strcpy(tilefilename, "D:/SN/tiles000.art");
			strcpy(soundfilename, "D:/SN/sounds.dat");
			return;
		}
	}
	if (access("SN2.DAT", R_OK) == 0)
	{
		addfile("SN2.DAT");
		gamemode = sequel;
		return;
	}
	if (access("SN.DAT", R_OK) == 0)
	{
		addfile("SN.DAT");
		gamemode = fullversion;
		return;
	}
	if (access("SN0.DAT", R_OK) == 0)
	{
		addfile("SN0.DAT");
		gamemode = demomode;
		return;
	}
}

int main(int argc, char *argv[])
{
	long i, j;
	char tempbuf[80];
	int alternative_level;
	int warpepisode, warplevel, warpskill;
	int slot;

	warpepisode = 1;
	warplevel = 1;
	warpskill = Medium;
	slot = -1;
	identifyversion(argc, argv);
	addfile(soundfilename);
	for(i=1;i<argc;i++)
	{
		if (stricmp(argv[i], "-nomonsters") == 0)
		{
			nomonsters = 1;
		}
		if (stricmp(argv[i], "-e") == 0)
		{
			warpepisode = atol(argv[i+1]);
			if (warpepisode < 1) warpepisode = 1;
			if (warpepisode > 5) warpepisode = 5;
			alternative_level = 1;
		}
		if (stricmp(argv[i], "-m") == 0)
		{
			warplevel = atol(argv[i+1]);
			if (warplevel < 1) warplevel = 1;
			if (gamemode != sequel)
				if (warplevel > 9) warplevel = 9;
			else
				if (warplevel > 32) warplevel = 32;
			alternative_level = 1;
		}
		if (stricmp(argv[i], "-s") == 0)
		{
			warpskill = atol(argv[i+1]);
			if (warpskill < Baby) warpskill = Baby;
			if (warpskill > Nightmare) warpskill = Nightmare;
		}
		if (stricmp(argv[i], "-map") == 0)
		{
			strcpy(boardfilename, argv[i+1]);
			if(strchr(boardfilename,'.') == 0)
				strcat(boardfilename,".map");
			custom_map = 1;
		}
		if (stricmp(argv[i], "-record") == 0)
		{
			strcpy(demofilename, argv[i+1]);
			if(strchr(demofilename,'.') == 0)
				strcat(demofilename,".dem");
			demorecording = 1;
		}
		if (stricmp(argv[i], "-loadgame") == 0)
		{
			slot = atol(argv[i+1]);
		}
		if (stricmp(argv[i], "-file") == 0)
		{
			for(j=argc-1;j>=i+1;j--)
				addfile(argv[j]);
		}
		if (stricmp(argv[i], "-respawn") == 0)
		{
			respawnparm = 1;
		}
		if (stricmp(argv[i], "-art") == 0)
		{
			sprintf(tilefilename,"%s000.ART",argv[i+1]);
		}
	}
	init();
	if (custom_map == 1)
	{
		initnew();
		loadlevel(boardfilename);
		gamestate = GAME;
	}
	else if (slot != -1)
	{
		loadgame(slot);
	}
	else if (alternative_level == 1)
	{
		initnew();
		startlevel(warpepisode, warplevel, warpskill);
		gamestate = GAME;
	}
	else
	{
		if (gamemode != sequel)
			intro();
		else
			intro2();
		PLAYDEMO:
		if (numdemos > 0)
		{
			sprintf(tempbuf, "SN%.2d.DEM", curdemo);
			playdemo(tempbuf);
		}
	}
	while (1)
	{
		switch (gamestate)
		{
			case GAME:
			if (keystatus[0x45] > 0 && !demoplayback)
			{
				keystatus[0x45] = 0;
				paused ^= 1;
			}
			if (tick > 5)
			{
				tick = 0;
				if (demoplayback)
				{
					if (curdemoframe >= numdemoframes)
					{
						curdemo++;
						if (curdemo > numdemos) curdemo = 1;
						goto PLAYDEMO;
					}
				}
				player();
			}
			break;
			case INTERMISSION:
			updateintermission();
			break;
			case ENDOFEPISODE:
			if (tick > 5)
			{
				tick = 0;
				updateend();
			}
			break;
		}
		if (!menustatus)
		{
			if (keystatus[1] > 0)
			{
				keystatus[1] = 0;
				playsfx(SfxSwitch, 0, 255);
				menustatus = 1;
				menu = 0;
			}
		}
		display();
		process_menus();
		nextpage();
	}
	return(0);
}
