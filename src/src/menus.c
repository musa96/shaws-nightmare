/*====================================================================
  This is a private property of Michael Muniko. You may not distribute
  copy or even hack it, without the permission from Michael Muniko.

  Email: michaelmuniko@gmail.com
  ===================================================================*/
#include <fcntl.h>
#include <io.h>
#include <sys\stat.h>
#include <sys\types.h>
#include "sn.h"
#include "timer.h"
#include "controls.h"
#include "graphics.h"
#include "saveg.h"
#include "weapon.h"
#include "player.h"
#include "sound.h"
#include "sfx.h"
#include "init.h"
#include "config.h"
#include "demo.h"
#include "levels.h"
#include "str.h"
#include "kbd.h"
#include "qaf.h"

int selectslot = -1;
int canload[9];
int selectepisode, menustatus, menu;
static char typeslot[16], typeslotleng = 0;
char scantoasc[128] =
{
	0,0,'1','2','3','4','5','6','7','8','9','0','-','=',0,0,
	'q','w','e','r','t','y','u','i','o','p','[',']',0,0,'a','s',
	'd','f','g','h','j','k','l',';',39,'`',0,92,'z','x','c','v',
	'b','n','m',',','.','/',0,'*',0,32,0,0,0,0,0,0,
	0,0,0,0,0,0,0,'7','8','9','-','4','5','6','+','1',
	'2','3','0','.',0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};
static char scantoascwithshift[128] =
{
	0,0,'!','@','#','$','%','^','&','*','(',')','_','+',0,0,
	'Q','W','E','R','T','Y','U','I','O','P','{','}',0,0,'A','S',
	'D','F','G','H','J','K','L',':',34,'~',0,'|','Z','X','C','V',
	'B','N','M','<','>','?',0,'*',0,32,0,0,0,0,0,0,
	0,0,0,0,0,0,0,'7','8','9','-','4','5','6','+','1',
	'2','3','0','.',0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

int helptics, helpframe, credtics, credframe;
static Qaf_T credqaf;
Qaf_T helpqaf;

void drawhelp(void)
{
	setview(0L,0L,xdim-1,ydim-1);
	drawqaf(&helpqaf, helpframe, 0, 2+128);
}

void drawcredits(void)
{
	setview(0L,0L,xdim-1,ydim-1);
	drawqaf(&credqaf, credframe, 0, 2+128);
}

void drawmenuback(long z)
{
	long x;
	
	if (z == 0) x = 94;
	else x = 65;
	rotatesprite(x<<16,37<<16,65536,0,Menu+z,0,0,10+16+64,0,0,xdim-1,ydim-1);
}

void readslots(void)
{
	long i, fil;
	char tempbuf[80];

	for(i=0;i<9;i++)
	{
		sprintf(tempbuf, "GAME%d.SAV", i);
		if ((fil = open(tempbuf, O_BINARY|O_RDONLY,S_IREAD)) != -1) 
		{
			read(fil, &slotname[i], 16);
			close(fil);
			canload[i] = 1;
			continue;
		}
		else
		{
			canload[i] = 0;
			strcpy(slotname[i], string[StringEmptySlot]);
		}
	}	
}

void drawslots(void)
{
	long i, y;

	y = 48;
	for(i=0;i<9;i++)
	{
		if (selectslot == i) { y += 10; continue; }
		gametext(80, y, slotname[i]);
		y += 10;
	}
	gametext(80, y+10, string[StringSlots]);
}

void typesaveslot(int slot)
{
	int charsperline;
	char ch, keystate;
	long i, j, y;
	char tempbuf[80];

	if (slot != -1 && menu == 5)
	{
		drawslots();
		y = 48+slot*10;
		for(i=0;i<=typeslotleng;i+=charsperline)
		{
			charsperline = 40;
			for(j=0;j<charsperline;j++)
				tempbuf[j] = typeslot[i+j];
			if (typeslotleng < i+charsperline)
			{
				tempbuf[(typeslotleng-i)] = '_';
				tempbuf[(typeslotleng-i)+1] = 0;
			}
			else
				tempbuf[charsperline] = 0;
			gametext(80L,y,tempbuf);
			while (keyfifoplc != keyfifoend)
			{
				ch = keyfifo[keyfifoplc];
				keystate = keyfifo[(keyfifoplc+1)&(KEYFIFOSIZ-1)];
				keyfifoplc = ((keyfifoplc+2)&(KEYFIFOSIZ-1));

				if (keystate != 0)
				{
					if (ch == 0xe)   //Backspace
					{
						typeslot[typeslotleng] = 0;
						if (typeslotleng == 0) { menustatus = 0; break; }
						typeslotleng--;
					}
					if (ch == 0xf)
					{
						keystatus[0xf] = 0;
						menustatus = 0;
						break;
					}
					if ((ch == 0x1c) || (ch == 0x9c))  //Either ENTER
					{
						keystatus[0x1c] = 0; keystatus[0x9c] = 0;
						if (typeslotleng > 0)
						{
							clearbufbyte(&slotname[slot],16L,0L);
							strcpy(slotname[slot], typeslot);
							savegame(slot);
							menustatus = 0;
						}
						break;
					}

				if ((typeslotleng < 15) && (ch < 128))
				{
					if (keystatus[0x2a]|keystatus[0x36])
						ch = scantoascwithshift[ch];
					else
						ch = scantoasc[ch];

					if (ch != 0) typeslot[typeslotleng++] = ch;
				}
			}
			//Here's a trick of making key repeat after a 1/2 second
		if (keystatus[0xe])
		{
			if (keystatus[0xe] < 30)
				keystatus[0xe] += TICSPERFRAME;
			else
			{
				if (typeslotleng == 0)
					menustatus = 0;
				else
					typeslotleng--;
			}
		}
		}
		}
	}
}

void slotmenu(void)
{
	drawslots();
	if (keystatus[1] > 0)
	{
		borderdraw = 1;
		playsfx(SfxSwitch, 0, 255);
		menustatus = 0;
	}
	if (selectslot == -1)
	{
		if (keystatus[2] > 0)
		{
			keystatus[2] = 0;
			selectslot = 0;
			if (menu == 5)
			{
				keyfifoplc = keyfifoend;      //Reset keyboard fifo
				typeslotleng = strlen(slotname[selectslot]);
				strcpy(typeslot, slotname[selectslot]);
			}
		}
		if (keystatus[3] > 0)
		{
			keystatus[3] = 0;
			selectslot = 1;
			if (menu == 5)
			{
				keyfifoplc = keyfifoend;      //Reset keyboard fifo
				typeslotleng = strlen(slotname[selectslot]);
				strcpy(typeslot, slotname[selectslot]);
			}
		}	
		if (keystatus[4] > 0)
		{
			keystatus[4] = 0;
			selectslot = 2;
			if (menu == 5)
			{
				keyfifoplc = keyfifoend;      //Reset keyboard fifo
				typeslotleng = strlen(slotname[selectslot]);
				strcpy(typeslot, slotname[selectslot]);
			}
		}
		if (keystatus[5] > 0)
		{
			keystatus[5] = 0;
			selectslot = 3;
			if (menu == 5)
			{
				keyfifoplc = keyfifoend;      //Reset keyboard fifo
				typeslotleng = strlen(slotname[selectslot]);
				strcpy(typeslot, slotname[selectslot]);
			}
		}
		if (keystatus[6] > 0)
		{
			keystatus[6] = 0;
			selectslot = 4;
			if (menu == 5)
			{
				keyfifoplc = keyfifoend;      //Reset keyboard fifo
				typeslotleng = strlen(slotname[selectslot]);
				strcpy(typeslot, slotname[selectslot]);
			}
		}
		if (keystatus[7] > 0)
		{
			keystatus[7] = 0;
			selectslot = 5;
			if (menu == 5)
			{
				keyfifoplc = keyfifoend;      //Reset keyboard fifo
				typeslotleng = strlen(slotname[selectslot]);
				strcpy(typeslot, slotname[selectslot]);
			}
		}
		if (keystatus[8] > 0)
		{
			keystatus[8] = 0;
			selectslot = 6;
			if (menu == 5)
			{
				keyfifoplc = keyfifoend;      //Reset keyboard fifo
				typeslotleng = strlen(slotname[selectslot]);
				strcpy(typeslot, slotname[selectslot]);
			}
		}
		if (keystatus[9] > 0)
		{
			keystatus[9] = 0;
			selectslot = 7;
			if (menu == 5)
			{
				keyfifoplc = keyfifoend;      //Reset keyboard fifo
				typeslotleng = strlen(slotname[selectslot]);
				strcpy(typeslot, slotname[selectslot]);
			}
		}
		if (keystatus[10] > 0)
		{	
			keystatus[10] = 0;
			selectslot = 8;
			if (menu == 5)
			{
				keyfifoplc = keyfifoend;      //Reset keyboard fifo
				typeslotleng = strlen(slotname[selectslot]);
				strcpy(typeslot, slotname[selectslot]);
			}
		}
	}
	if (selectslot != -1)
	{
		if (menu == 5)
		{
			typesaveslot(selectslot);
		}
		else
		{
			if (canload[selectslot])
			{
				loadgame(selectslot);
				menustatus = 0;
			}
			else
				selectslot = -1;
		}
	}
}

void skillmenu(void)
{
	long i;

	drawmenuback(1);
	puttext(80, 48, string[StringChooseSkill]);
	puttext(80, 64, string[StringSkill1]);
	puttext(80, 80, string[StringSkill2]);
	puttext(80, 96, string[StringSkill3]);
	puttext(80, 108, string[StringSkill4]);
	puttext(80, 124, string[StringSkill5]);
	if (keystatus[1] > 0)
	{
		keystatus[1] = 0;
		borderdraw = 1;
		playsfx(SfxSwitch, 0, 255);
		menustatus = 0;
	}
	if (keystatus[2] > 0)
	{
		keystatus[2] = 0;
		initnew();
		demoplayback = 0;
		startlevel(selectepisode, 1, Baby);
		menustatus = 0;
	}
	if (keystatus[3] > 0)
	{
		keystatus[3] = 0;
		initnew();
		demoplayback = 0;
		startlevel(selectepisode, 1, Easy);
		menustatus = 0;
	}
	if (keystatus[4] > 0)
	{
		keystatus[4] = 0;
		initnew();
		demoplayback = 0;
		startlevel(selectepisode, 1, Medium);
		menustatus = 0;
	}
	if (keystatus[5] > 0)
	{
		keystatus[5] = 0;
		initnew();
		demoplayback = 0;
		startlevel(selectepisode, 1, Hard);
		menustatus = 0;
	}
	if (keystatus[6] > 0)
	{
		keystatus[6] = 0;
		playsfx(SfxSwitch, 0, 255);
		borderdraw = 1;
		menu = 10;
	}
}

void episodemenu(void)
{
	drawmenuback(1);
	puttext(80, 48, string[StringWhichEpisode]);
	puttext(80, 64, string[StringEpisode1]);
	puttext(80, 80, string[StringEpisode2]);
	puttext(80, 96, string[StringEpisode3]);
	puttext(80, 108, string[StringEpisode4]);
	puttext(80, 120, string[StringEpisode5]);
	if (keystatus[1] > 0)
	{
		keystatus[1] = 0;
		borderdraw = 1;
		menu = 0;
	}
	if (keystatus[2] > 0)
	{
		keystatus[2] = 0;
		selectepisode = 1;
		borderdraw = 1;
		playsfx(SfxSwitch, 0, 255);
		menu = 2;
	}
	if (gamemode != demomode)
	{
		if (keystatus[3] > 0)
		{
			keystatus[3] = 0;
			selectepisode = 2;
			borderdraw = 1;
			playsfx(SfxSwitch, 0, 255);
			menu = 2;
		}
		if (keystatus[4] > 0)
		{
			keystatus[4] = 0;
			selectepisode = 3;
			borderdraw = 1;
			playsfx(SfxSwitch, 0, 255);
			menu = 2;
		}
		if (keystatus[5] > 0)
		{
			keystatus[5] = 0;
			selectepisode = 4;
			borderdraw = 1;
			playsfx(SfxSwitch, 0, 255);
			menu = 2;
		}
		if (keystatus[6] > 0)
		{
			keystatus[6] = 0;
			selectepisode = 5;
			borderdraw = 1;
			playsfx(SfxSwitch, 0, 255);
			menu = 2;
		}
	}
}	

void options(void)
{
	rotatesprite(50<<16,37<<16,72238,0,Menu+1,0,0,10+16+64,0,0,xdim-1,ydim-1);
	if (detailmode == 0)
		puttext(65, 48, string[StringDetailHigh]);
	else if (detailmode == 1)
		puttext(65, 48, string[StringDetailMedium]);
	else
		puttext(65, 48, string[StringDetailLow]);
	puttext(65, 64, string[StringSoundVolume]);
	drawslider(65, 72, sfxvol / 16, 16);
	puttext(65, 80, string[StringMusicVolume]);
	drawslider(65, 88, musicvol / 16, 16);
	puttext(65, 96, string[StringEndGame]);
	puttext(65, 112, string[StringScreenSize]);
	drawslider(65, 120, screensize / 8, 9);
	puttext(65, 128, string[StringGamma]);
	drawslider(65, 136, brightness, 9);
	puttext(65, 144, string[StringMouse]);
	drawslider(65, 152, mousesensitivity, 9);
	if (keystatus[1] > 0)
	{
		keystatus[1] = 0;
		borderdraw = 1;
		playsfx(SfxSwitch, 0, 255);
		menu = 0;
	}
	if (keystatus[2] > 0)
	{
		keystatus[2] = 0;
		detailmode++;
		if (detailmode > 2) detailmode = 0;
		playsfx(SfxSwitch, 0, 255);
	}
	if (keystatus[3] > 0)
	{
		keystatus[3] = 0;
		sfxvol += 8;
		if (sfxvol > 255) sfxvol = 0;
		set_sound_volume(sfxvol);
		playsfx(SfxSwitch, 0, 255);
	}
	if (keystatus[4] > 0)
	{
		keystatus[4] = 0;
		musicvol += 8;
		if (musicvol > 255) musicvol = 0;
		setmusicvolume(musicvol);
	}
	if (keystatus[5] > 0)
	{
		keystatus[5] = 0;
		if (gamestate == GAME && !demoplayback)
		{
			playsfx(SfxSwitch, 0, 255);
			menu = 9;
		}
	}
	if (keystatus[6] > 0)
	{
		keystatus[6] = 0;
		if (gamestate == GAME)
		{
			screensize += 8;
			if (screensize > 64) screensize = 0;
			playsfx(SfxSwitch, 0, 255);
			setviewsize();
		}
	}
	if (keystatus[7] > 0)
	{
		keystatus[7] = 0;
		brightness++;
		if (brightness > 8) brightness = 0;
		playsfx(SfxSwitch, 0, 255);
		dopalettestuff();
	}
	if (keystatus[8] > 0)
	{
		keystatus[8] = 0;
		mousesensitivity++;
		if (mousesensitivity > 8) mousesensitivity = 0;
		playsfx(SfxSwitch, 0, 255);
	}
}

void loadcredits(void)
{
	loadqaf("CREDITS.QAF", &credqaf);
}

void mainmenu(void)
{
	drawmenuback(0);
	puttext(110, 48, string[StringNewGame]);
	puttext(110, 64, string[StringLoadGame]);
	puttext(110, 80, string[StringSaveGame]);
	puttext(110, 96, string[StringOptions]);
	if (gamemode != sequel)
	{
		puttext(110, 112, string[StringHelp]);
		puttext(110, 128, string[StringCredits]);
		puttext(110, 144, string[StringQuitGame]);
	}
	else
	{
		puttext(110, 112, string[StringCredits]);
		puttext(110, 128, string[StringQuitGame]);
	}
	if (keystatus[1] > 0)
	{
		keystatus[1] = 0;
		borderdraw = 1;
		playsfx(SfxSwitch, 0, 255);
		menustatus = 0;
	}	
	if (keystatus[2] > 0)
	{
		keystatus[2] = 0;
		borderdraw = 1;
		playsfx(SfxSwitch, 0, 255);
		if (gamemode == sequel) menu = 2;
		else menu = 1;
	}
	if (keystatus[3] > 0)
	{
		keystatus[3] = 0;
		borderdraw = 1;
		selectslot = -1;
		readslots();
		playsfx(SfxSwitch, 0, 255);
		menu = 4;
	}
	if (keystatus[4] > 0)
	{
		keystatus[4] = 0;
		if (gamestate == GAME && !demoplayback && !death)
		{
			borderdraw = 1;
			selectslot = -1;
			readslots();
			playsfx(SfxSwitch, 0, 255);
			menu = 5;
		}
	}
	if (keystatus[5] > 0)
	{
		keystatus[5] = 0;
		borderdraw = 1;
		playsfx(SfxSwitch, 0, 255);
		menu = 3;
	}
	if (keystatus[6] > 0)
	{
		keystatus[6] = 0;
		borderdraw = 1;
		playsfx(SfxSwitch, 0, 255);
		switch (gamemode)
		{
			case fullversion:
			case demomode:
			loadqaf("HELP.QAF", &helpqaf);
			menu = 6;
			break;
			case sequel:
			loadcredits();
			menu = 8;
			break;
		}
	}
	if (keystatus[7] > 0)
	{
		keystatus[7] = 0;
		borderdraw = 1;
		playsfx(SfxSwitch, 0, 255);
		switch (gamemode)
		{
			case fullversion:
			case demomode:
			loadqaf("CREDITS.QAF", &credqaf);
			menu = 8;
			break;
			case sequel:
			borderdraw = 1;
			playsfx(SfxSwitch, 0, 255);
			menu = 7;
			break;
		}
	}
	if (keystatus[8] > 0)
	{
		keystatus[8] = 0;
		if (gamemode != sequel)
		{
			borderdraw = 1;
			playsfx(SfxSwitch, 0, 255);
			menu = 7;
		}
	}
}

void process_menus(void)
{
	if (menustatus)
	{
		switch (menu)
		{
			case 0:
			mainmenu();
			break;
			case 1:
			episodemenu();
			break;
			case 2:
			skillmenu();
			break;
			case 3:
			options();
			break;
			case 4:
			case 5:
			slotmenu();
			break;
			case 6:
			if (tick > 5)
			{
				tick = 0;
				if (playqaf(&helpqaf, &helptics, &helpframe) == 0)
				{
					helptics = 0;
					helpframe = 0;
				}
			}
			if (keystatus[28] > 0)
			{
				keystatus[28] = 0;
				playsfx(SfxSwitch, 0, 255);
				menu = 0;
			}
			drawhelp();
			break;
			case 7:
			drawmenuback(1);
			puttext(100, 80, string[StringQuit]);
			puttext(130, 96, string[StringYN]);
			if (keystatus[21] > 0)
			{
				keystatus[21] = 0;
				if (demorecording) savedemo(demofilename);
				uninit();
				exit(0);
			}
			if (keystatus[49] > 0 || keystatus[1] > 0)
			{
				keystatus[49] = 0;
				keystatus[1] = 0;
				playsfx(SfxSwitch, 0, 255);
				menustatus = 0;
			}
			break;
			case 8:
			if (tick > 5)
			{
				tick = 0;
				if (playqaf(&credqaf, &credtics, &credframe) == 0)
				{
					credtics = 0;
					credframe = 0;
				}
			}
			if (keystatus[28] > 0)
			{
				keystatus[28] = 0;
				playsfx(SfxSwitch, 0, 255);
				menu = 0;
			}
			drawcredits();
			break;
			case 9:
			drawmenuback(1);
			puttext(100, 80, string[StringEndQuestion1]);
			puttext(100, 96, string[StringEndQuestion2]);
			puttext(130, 112, string[StringYN]);
			if (keystatus[21] > 0)
			{
				keystatus[21] = 0;
				playsfx(SfxSwitch, 0, 255);
				if (gamemode != sequel)
					intro();
				else
					intro2();
				menustatus = 0;
			}
			if (keystatus[49] > 0 || keystatus[1] > 0)
			{
				keystatus[49] = 0;
				keystatus[1] = 0;
				playsfx(SfxSwitch, 0, 255);
				menustatus = 0;
			}
			break;
			case 10:
			drawmenuback(1);
			puttext(100, 80, string[StringFairQuestion1]);
			puttext(100, 96, string[StringFairQuestion2]);
			puttext(100, 108, string[StringFairQuestion3]);
			puttext(130, 120, string[StringYN]);
			if (keystatus[21] > 0)
			{
				keystatus[21] = 0;
				initnew();
				demoplayback = 0;
				startlevel(selectepisode, 1, Nightmare);
				menustatus = 0;
			}
			if (keystatus[49] > 0 || keystatus[1] > 0)
			{
				keystatus[49] = 0;
				keystatus[1] = 0;
				playsfx(SfxSwitch, 0, 255);
				menu = 2;
			}
			break;
		}
	}
}
