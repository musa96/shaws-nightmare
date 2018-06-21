#include <i86.h>
#include "sn.h"
#include "debug.h"
#include "config.h"
#include "sfx.h"
#include "sound.h"
#include "dat.h"
#include "graphics.h"
#include "weapon.h"
#include "player.h"
#include "init.h"

#define CLIPDIST 16525
#define CLOSEDIST 10000

int sfxvol;
int base, irq, dma8, dma16;
int pcsplaying, pcsfx;
PCSPEAKER pcsound[NumSfx];
SOUND *soundptr[NumSfx];
static char *sfxnames[NumSfx] =
{
	"NONE",
	"BOYDIE",
	"BOYDIE2",
	"BOYROAM", 
  	"BOYSPOT", 
  	"BOYHURT",
	"CATSPOT",
	"CATHURT",
	"CATDIE",
  	"DOGDIE",  
  	"DOGHURT", 
  	"DOGROAM",
	"EVSTART",
	"EVEND",
  	"DOOR",
	"DOOR1",
	"SDOOROPN",
	"SDOORCLS",
	"SLABMOVE",
  	"EXPLODE", 
	"HGNFIRE", 
  	"ITEMUP",
  	"LAND", 
  	"AMMOUP",
  	"CATFIRE",
	"DBRFIRE",
	"WPNUP",
	"SWITCH",
	"SHWPN12",
	"SHWPN15",
	"SHWPN18",
	"SHWPN21",
	"SHWDIE",
	"SHWSCRM",
	"TELEPORT",
	"CGUN",
	"FLAMUP",
	"FLAMIDLE",
	"FLAMFUL",
	"FLAMHIT",
	"GLASS",
	"CATATK",
	"PROJHIT",
	"FEXP",
	"CANFIRE",
	"CHUISPOT",
	"CHUIROAM",
	"CHUIPAIN",
	"CHUIDIE",
	"HGESPOT",
	"HUGEDIE",
	"RAPIDFIR",
	"BSHOOT",
	"CMTSPOT",
	"CMTROAM",
	"CMTPAIN1",
	"CMTPAIN2",
	"CMTPAIN3",
	"CMTPAIN4",
	"CMTPAIN5",
	"CMTPAIN6",
	"FDOOROPN",
	"FDOORCLS",
	"FOOT",
	"SPLAT",
	"KALISPOT",
	"KALIHURT",
	"KALIDIE",
	"KINSPOT",
	"KINHURT",
	"KINDIE",
	"POWERUP",
	"SDBFIR",
	"SDBREL",
	"LITFSPOT",
	"LITFHRT",
	"LITFDIE",
	"BOSSSPOT",
	"BOSSHURT",
	"BOSSDIE",
	"BOSSSPIT",
	"CUBLAND",
	"KARPAIN",
	"KARDIE",
	"ELESPOT",
	"ELEDIE",
	"PLOPSPOT",
	"PLOPHURT",
	"PLOPDIE",
	"SHDIEHI",
	"GIRLSPOT",
	"GIRLSPT2",
	"GIRLHURT",
	"GIRLDIE",
	"OGRESPOT",
	"OGREDIE",
	"ZOMSPOT",
	"ZOMHURT",
	"ZOMDIE",
	"WHOOSH",
	"PUNCH",
	"FLAME",
	"FLAMEST",
	"EVILSPOT",
	"EVILDIE",
	"CLSPOT",
	"CLHURT",
	"CLDIE",
	"OGREFIRE",
	"ZOMATK",
	"FATWALK"
};
volatile char sfxlock[NumSfx];

void initsfx(void)
{
	long siz, fil, i, j, k;
	int rc;
	char tempbuf[13];

	if (gamemode == sequel) j = 0;
	else j = 39;
	if (sfxcard == 1)
	{
		if (!detect_settings(&base, &irq, &dma8, &dma16))
			error("initsfx(): Cannot detect settings for Sound Blaster", 9209);

		if (!init_sb(base, irq, dma8, dma16))
			error("initsfx(): Sound Blaster did not initialize", 9210);

		init_mixing();
		for(i=1;i<NumSfx-j;i++)
		{
			sfxlock[i] = 200;
			strcpy(tempbuf, sfxnames[i]);
			strcat(tempbuf, ".RAW");
			fil = loaditem(tempbuf);
			siz = itemlength(tempbuf);
			if (siz <= 0) continue;
			if (siz > 65536) continue;
			allocache((SOUND *)&soundptr[i],sizeof(SOUND),&sfxlock[i]);
			allocache((signed char *)&soundptr[i]->soundptr,siz,&sfxlock[i]);
			soundptr[i]->soundsize = siz;
			readitem(fil, soundptr[i]->soundptr, soundptr[i]->soundsize);
		}
		set_sound_volume(sfxvol);
	}
	else if (sfxcard == 2)
	{
		for(i=1;i<NumSfx-j;i++)
		{
			strcpy(tempbuf, sfxnames[i]);
			strcat(tempbuf, ".PCS");
			fil = loaditem(tempbuf);
			readitem(fil, &pcsound[i].bps, 1);
			readitem(fil, &pcsound[i].numnotes, 2);
			for(k=0;k<pcsound[i].numnotes;k++)
				readitem(fil, &pcsound[i].note[k], 2);
		}
	}
}

void sfxterm(void)
{
	long i;

	if (sfxcard == 1)
	{
		shutdown_mixing();
		shutdown_sb();
	}
}

int setears(long listenerx, long listenery, long originx, long originy, int *vol)
{
	long soundx, soundy, dist;

	soundx = abs(listenerx - originx);
	soundy = abs(listenery - originy);

	dist = soundx + soundy - ((soundx < soundy ? soundx : soundy)>>1);

	if (dist > CLIPDIST) return(0);

	if (dist < CLOSEDIST)
	{
		*vol = 255;
	}
	else
	{
		*vol = (sfxvol / 15)*((CLIPDIST - dist)>>8);
	}
	return (*vol > 0);
}

void playxysfx(int sfx, int index, long x, long y)
{
	int rc, vol;

	rc = setears(posx, posy, x, y, &vol);
	if (!rc) return;
	playsfx(sfx, index, vol);
}

void playsfx(int sfx, int index, int vol)
{
	char tempbuf[80];
	long i, fil;

	// Check for bad sfx numbers
	if (sfx < 0 || sfx > NumSfx)
	{
		sprintf(tempbuf, "Bad sfx number #%d", sfx);
		error(tempbuf, 9212);
	}

	switch (sfxcard)
	{
		case 1:
		start_sound(soundptr[sfx], index, vol, OFF);
		break;
		case 2:
		pcsfx = sfx;
		curnote = 0;
		pcstics = 0;
		pcsplaying = 1;
		break;
	}
}

void killsfx(int index)
{
	switch (sfxcard)
	{
		case 1:
		stop_sound(index);
		break;
		case 2:
		pcsplaying = 0;
		nosound();
		break;
	}
}

void killallsfx(void)
{
	long i;

	switch (sfxcard)
	{
		case 1:
		for(i=0;i<1024+4096;i++)
			stop_sound(i);
		break;
		case 2:
		pcsplaying = 0;
		nosound();
		break;
	}
}

