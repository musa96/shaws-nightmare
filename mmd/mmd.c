#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <conio.h>
#include <dos.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <sys\types.h>
#include "mmd.h"

static unsigned adlibfreq[128] =
{
	0,48,96,128,132,156,187,
	194,205,234,256,272,290,
	320,335,360,385,399,400,
	420,470,500,512,534,556,
	580,600,620,635,645,675,
	712,760,772,790,812,835,
	845,865,896,912,940,950,
	1000,1024,1078,1120,1135,
	1145,1167,1185,1196,1200,
	1245,1267,1278,1290,1300,
	1305,1378,1380,1390,1405,
	1412,1430,1440,1456,1478,
	1490,1500,1515,1530,1545,
	1600,1620,1634,1645,1650,
	1680,1690,1700,1703,1750,
	1760,1790,1800,1810,1820,
	1840,1860,1900,1940,1980,
	2000,2001,2002,2003,2004,
	2005,2006,2007,2008,2009,
	2010,2011,2012,2013,2014,
	2015,2016,2017,2018,2019,
	2020,2021,2022,2023,2024,
	2025,2026,2027,2028,2029,
	2030,2031,2034,2035,2036
};

static int curdriver, fade, curmusicvol = 255, oldvolume, sysexsize;
char sngsig[4], sysex[64];
short numnotes, trinst[MAXCHANNELS], songtempo;
int musicstatus, musicloop;
Note_T note[MAXNOTES];
INSTDATA inst[256];
short songplc, songspeed, timecount;

void sendmidi(int command)
{
	unsigned count;

	count = 256;
	while (count > 0)
	{
		if (!(inp(0x331)&0x40)) break;
		count--;
	}
	outp(0x330, command);
}

void sendmidicommand(int command)
{
	unsigned count;

	count = 0xffff;
	while(count > 0)
	{
		if (!(inp(0x331)&0x40))
		{
			break;
		}
		count--;
	}
	outp(0x331, command);
}

int setuart(void)
{
	unsigned count;
	int port = 0x331;

	sendmidicommand(0x3f);
	count = 0xffff;
	while (count > 0)
	{
		if (!(inp(port)&0x80))
		{
			port--;
			if (inp(port) == 0xfe) return(1);
			port++;
		}
		count--;
	}
	return(0);
}

int initmmd(int driver)
{
	int port = 0x331, count;

	switch (driver)
	{
		case 1:
		if (!fm_detect()) return(-1);
		fm_reset();
		break;
		case 2:
		case 3:
		sendmidicommand(0xff); // reset MPU
		count = 0xffff;
		while (count > 0)
		{
			if (!(inp(port)&0x80))
			{
				port--;
				if (inp(port) == 0xfe) break;
				port++;
			}
			count--;
			return(-1);
		}
		if (!setuart()) return(-1);
		break;
	}
	curdriver = driver;
	loadinst();
	return(0);
}

void uninitmmd(void)
{
	allnotesoff();
	switch (curdriver)
	{	
		case 1:
		fm_reset();
		break;
		case 2:
		case 3:
		sendmidicommand(0xff); // reset MPU
		break;
	}
}

void loadinst(void)
{
	long fil;

	fil = loaditem("INSTS.DAT");
	if (fil == -1) return;
	readitem(fil, &inst[0], sizeof(INSTDATA)*256);
	closeitem(fil);
}

static int calcvolume(int notevol, int vol)
{
	notevol = (notevol*vol) / 255;
	if (notevol > 63) notevol = 63;
	return(notevol);
}

void setmusicvolume(int vol)
{
	long i, j;

	curmusicvol = vol;
	oldvolume = vol;
	for(i=1;i<MAXCHANNELS;i++)
	{
		switch(curdriver)
		{
			case 2:
			case 3:
			sendmidi(0xb0|i);
			sendmidi(0x7);
			sendmidi(curmusicvol / 2);
			break;
		}
	}
	if (curdriver > 1)
	{
		sendmidi(0xb9);
		sendmidi(0x7);
		sendmidi(curmusicvol / 2);
	}
}

void fademusic(void)
{
	oldvolume = curmusicvol;
	fade = 1;
}

void noteon(int freq, int track, int vol)
{
	int i;
	
	switch (curdriver)
	{
		case 1:
		fm_play_tone(track, adlibfreq[freq], vol);
		break;
		case 2:
		case 3:
		if (inst[trinst[track]].geninst > 127)
		{
			sendmidi(0x99);
			sendmidi(inst[trinst[track]].geninst-100);
		}
		else
		{
			sendmidi(0x90|track);
			if (freq < 90) freq += 35;
			sendmidi(freq);
		}
		sendmidi(vol*2);
		break;
	}
}

void noteoff(int track)
{
	switch (curdriver)
	{
		case 1:
		fm_stop_tone(track);
		break;
		case 2:
		case 3:
		if (inst[trinst[track]].geninst > 127)
		{
			sendmidi(0xb9);
		}
		else
		{
			sendmidi(0xb0|track);
		}
		sendmidi(0x7b);
		sendmidi(0);
		break;
	}
}

void allnotesoff(void)
{
	long i, j;

	for(i=1;i<MAXCHANNELS;i++)
	{
		switch (curdriver)
		{
			case 1:	    
			fm_stop_tone(i);
			break;
			case 2:
			case 3:
			if (inst[trinst[i]].geninst > 127)
			{
				sendmidi(0xb9);
			}
			else
			{
				sendmidi(0xb0|i);
			}
			sendmidi(0x7b);
			sendmidi(0);
			break;
		}
	}
}

void setinstruments(void)
{
	int i;

	for(i=1;i<MAXCHANNELS;i++)
	{
		switch (curdriver)
		{
			case 1:			
		        fm_load_patch(i, &inst[trinst[i]].patch);
			break;
			case 2:
			sendmidi(0xc0|i);
			sendmidi(inst[trinst[i]].mt32inst&0x7f);
			break;
			case 3:
			sendmidi(0xc0|i);
			sendmidi(inst[trinst[i]].geninst&0x7f);
			break;
		}
	}
}

int loadsong(char *filename)
{
	int i, fil;

	musicstatus = 0;
	songspeed = 10;
	songplc = 0;
	timecount = 0;
	fade = 0;
	setmusicvolume(oldvolume);
	allnotesoff();
	fil = loaditem(filename);
	if (fil == -1) return(-1);
	readitem(fil, &sngsig, 4);
	if (sngsig[0] != 'S' && sngsig[1] != 'N' && sngsig[2] != 'G'
		&& sngsig[3] != 0x1a)
		return(-2);
	readitem(fil, &songtempo, 2);
	readitem(fil, &numnotes, 2);
	readitem(fil, &trinst[0], 2*MAXCHANNELS);
	readitem(fil, &note[0], sizeof(Note_T)*numnotes);
	closeitem(fil);
	setinstruments();
	timecount = 0;
	return(0);
}

void mmdupdate(void)
{
	long i, j;

	if (musicstatus == 1)
	{
		while ((songplc < numnotes) && (timecount >= songspeed * 500 / songtempo))
		{
			noteoff(note[songplc].track);
			if (note[songplc].event != 0)
			{
				switch (note[songplc].event)
				{
					case 1: // jump to a particular note
					allnotesoff();
					songplc = note[songplc].paramanter;
					break;
					case 2: // stop a note
					noteoff(note[songplc].paramanter);
					break;
					case 3: // set speed
					songspeed = note[songplc].paramanter;
					break;
					case 4:
					if (curdriver > 1)
					{
						sendmidi(0xb0|note[songplc].paramanter);
						sendmidi(0x79);
						sendmidi(0);
					}
					break;
					case 5:
					if (curdriver > 1)
					{
						sendmidi(0xb0|note[songplc].track);
						sendmidi(0x40);
						sendmidi(note[songplc].paramanter);
					}
					break;
					case 7:
					sysex[sysexsize++] = note[songplc].paramanter;
					break;
					case 8:
					sendmidi(0xff);
					sendmidi(0xf0);
					for(i=0;i<sysexsize;i++)
						sendmidi(sysex[i]);
					sendmidi(0xf7);
					sysexsize = 0;
					break;
				}
			}
			i = calcvolume(note[songplc].volume, curmusicvol);
			if (note[songplc].freq != 0)
				noteon(note[songplc].freq, note[songplc].track, i);

			timecount = 0;
			songplc++;
			if (songplc >= numnotes)
			{
				if (musicloop == 0)
					musicstatus = 0;
				else songplc = 0;
			}
		}
		timecount++;
	}
	if (fade == 1)
	{
		curmusicvol--;
		if (curmusicvol < 0)
		{
			curmusicvol = 0;
			fade = 0;
			allnotesoff();
			musicstatus = 0;
		}
		for(i=1;i<MAXCHANNELS;i++)
		{
			/*if (curdriver == 1)
			{
				j = calcvolume(note[songplc].volume, curmusicvol);
				fm_set_voice_volume(i, j);
			}*/
			if (curdriver > 1)
			{
				sendmidi(0xb0|i);
				sendmidi(0x7);
				sendmidi(curmusicvol / 2);
			}
		}
		if (curdriver > 1)
		{
			sendmidi(0xb9);
			sendmidi(0x7);
			sendmidi(curmusicvol / 2);
		}
	}
}
