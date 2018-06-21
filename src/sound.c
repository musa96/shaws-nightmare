#include <i86.h>
#include "config.h"
#include "sn.h"
#include "sound.h"
#include "sfx.h"

int pcstics, curnote;
int musicvol;
static char *sn2music[] =
{
	"FUTUR",
	"FLOW",
	"COUN2",
	"RUNNIN",
	"NIGHT",
	"MAJOR",
	"TOWN",
	"HITWL",
	"WORK",
	"MEREMO",
	"IKILL",
	"HORROR",
	"ON_RUN",
	"OPENIN",
	"LITF",
	"THIRD",
	"THEME",
	"COMMEN"
};
int gamemusic[] =
{
	sng_s4,
	sng_s5,
	sng_s11,
	sng_s14,
	sng_s2,
	sng_s6,
	sng_s7,
	sng_s1,
	sng_s14,

	sng_s9,
	sng_s17,
	sng_s24,
	sng_s21,
	sng_s11,
	sng_s15,
	sng_s25,
	sng_s23,
	sng_s12,

	sng_s19,
	sng_s13,
	sng_s16,
	sng_s24,
	sng_s22,
	sng_s7,
	sng_s24,
	sng_s23,
	sng_s28,

	sng_s14,
	sng_s10,
	sng_s20,
	sng_s14,
	sng_s4,
	sng_s13,
	sng_s24,
	sng_s29,
	sng_s14,

	sng_s4,
	sng_s28,
	sng_s14,
	sng_s10,
	sng_s25,
	sng_s21,
	sng_s15,
	sng_s2,
	sng_s14
};

int gamemusic2[] =
{
	sng_Future,
	sng_Flow,
	sng_Count,
	sng_Running,
	sng_Town,
	sng_OnRun,
	sng_Major,
	sng_HitTheWall,
	sng_Work,
	sng_Horror,
	sng_Flow,
	sng_OnRun,
	sng_Town,
	sng_HitTheWall,
	sng_Future,
	sng_Horror,
	sng_Flow,
	sng_Meremo,
	sng_Running,	
	sng_Ikill,
	sng_Count,
	sng_Running,
	sng_Night,
	sng_Major,
	sng_Flow,
	sng_Meremo,
	sng_Running,
	sng_Night,
	sng_Major,
	sng_Opening,
	sng_LITF,
	sng_Third
};
void initsound(void)
{
	char tempbuf[255];

	initsfx();
	if (musicsource)
	{
		if (initmmd(musicsource) == -1)
		{
			strcpy(tempbuf, "Music driver did not initialize");
			if (musicsource > 1)
				strcat(tempbuf, "\nMake sure the MPU port is 330.");
			error(tempbuf, 9211);
		}
		if (musicsource > 1)
		{
			if (musicsource == 2)
				strcpy(tempbuf, "INITMT.SNG");
			else
				strcpy(tempbuf, "INITGM.SNG");
			loadsong(tempbuf);
			musicloop = 0;
			musicstatus = 1;
			while (musicstatus == 1);
		}
		setmusicvolume(musicvol);
	}
}

void uninitsound(void)
{
	sfxterm();
	if (sfxcard == 2) nosound();
	if (musicsource) uninitmmd();
}

void playsong(int musicid, int loop)
{
	char tempbuf[80], filename[13];

	if (!musicsource) return;

	if (musicid < 0 || musicid > NUMMUSIC)
	{
		sprintf(tempbuf, "Bad music number #%d", musicid);
		error(tempbuf, 9213);
	}
	if (musicid < sng_s1)
		sprintf(filename, "B%d.SNG", musicid+1);
	else if (musicid < sng_Future)
		sprintf(filename, "S%d.SNG", musicid - sng_b5);
	else
		sprintf(filename, "S_%s.SNG", sn2music[musicid-sng_Future]);

	loadsong(filename);
	musicloop = loop;
	musicstatus = 1;
}

void updatesounds(void)
{
	if (sfxcard == 2 && pcsplaying)
	{
		pcstics++;
		if (pcstics > pcsound[pcsfx].bps / 8)
		{
			pcstics = 0;
			curnote++;
		}	
		if (curnote >= pcsound[pcsfx].numnotes)
		{
			pcsplaying = 0;
			nosound();
		}
		sound(pcsound[pcsfx].note[curnote]);
		if (pcsound[pcsfx].note[curnote] == 0) nosound();
	}
}
