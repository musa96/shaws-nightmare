#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dos.h>
#include <io.h>
#include <conio.h>
#include <string.h>
#include <graph.h>
#include <sys\stat.h>
#include <sys\types.h>
#include "mmd.h"

typedef struct
{
	PATCH patch;
	char filler[5];
} ibkinst_t;

ibkinst_t ibkpatch[128];
char ibknames[128][9], ibksig[4];
char sngfilename[128] = "NEW.SNG";
int lastfreq;
static char *eventstr[9] = {"",
    			    "Jump to note ",
			    "Stop Note ",
			    "Set Speed ",
			    "Reset all controllers ",
			    "CtrlChng Hold ",
			    "Sysex Start ",
			    "Sysex Data ",
			    "Sysex End "};

static int oldvolume;
static int octave, asksave;
static int curtrack, editmode, cursection, modcar;
static int curinst, askupdate, instnameleng;
static int startnote, endnote; // used for event editor
static int startcopy, endcopy;
char key;
extern short songplc, songspeed, timecount;
static INSTDATA clipinst;
void (__interrupt __far *oldkbhandler)();
void __interrupt __far (*oldhandler)();
void __interrupt __far mmdhandler();

void __interrupt kbhandler(void)
{
	key = inp(0x60);
	outp(0x20, 0x20);
}

void initkbd(void)
{
	oldkbhandler = _dos_getvect(0x9);
	_dos_setvect(0x9, kbhandler);
}

void uninitkbd(void)
{
	key = 0;
	_dos_setvect(0x9, oldkbhandler);
}

void __interrupt __far mmdhandler()
{
	mmdupdate();
	outp(0x20,0x20);
}

void inittimer(void)
{
	outp(0x43,0x34);
	outp(0x40,(1193181/120)&255);
	outp(0x40,(1193181/120)>>8);
	oldhandler = _dos_getvect(0x8);
	_disable(); _dos_setvect(0x8, mmdhandler); _enable();
}

void uninittimer(void)
{
	outp(0x43,0x34); outp(0x40,0); outp(0x40,0);
	_disable(); _dos_setvect(0x8, oldhandler); _enable();
}

int loaditem(char *filename) { return(open(filename,O_BINARY|O_RDONLY,S_IREAD)); }
void readitem(int handle, void *buf, int len) { read(handle, buf, len); }
void closeitem(int handle) { close(handle); }

void saveinst(void)
{
	long fil;

	if ((fil = open("INSTS.DAT", O_BINARY|O_CREAT|O_TRUNC|O_WRONLY,S_IWRITE)) != -1)
	{
		write(fil,&inst[0],sizeof(INSTDATA)*256);
		close(fil);
		printf("INSTS.DAT updated.\n");
	}
}

void updatescreen(void)
{
	long i, j, k;
	char tempbuf[80], tempbuf2[80];

	_clearscreen(_GCLEARSCREEN);
	switch (editmode)
	{
		case 0:
		for(i=1;i<MAXCHANNELS;i++)
		{
			if (i == curtrack) _settextcolor(15);
			else _settextcolor(8);
			j = i+1;
			_settextposition(j-1, 0);
			sprintf(tempbuf, "Track %i, Instrument: %d", i, trinst[i]);
			_outtext(tempbuf);
		}
		break;
		case 1:
		_settextcolor(8);
		_settextposition(1, 0);
		sprintf(tempbuf, "Instrument %d", curinst);
		_outtext(tempbuf);
		_settextposition(2, 0);
		_outtext("Modulator");
		_settextposition(2, 40);
		_outtext("Carrier");
		_settextposition(3, 0);
		if (cursection == 0 && modcar == 0)
			_settextcolor(15);
		else
			_settextcolor(8);
		sprintf(tempbuf, "KSL: %d", inst[curinst].patch.ksl_lev[0]);
		_outtext(tempbuf);
		_settextposition(3, 40);
		if (cursection == 0 && modcar == 1)
			_settextcolor(15);
		else
			_settextcolor(8);
		sprintf(tempbuf, "Level: %d", inst[curinst].patch.ksl_lev[1]);
		_outtext(tempbuf);	
		_settextposition(4, 0);
		if (cursection == 1 && modcar == 0)
			_settextcolor(15);
		else
			_settextcolor(8);
		sprintf(tempbuf, "Multiplier: %d", inst[curinst].patch.chars[0]);
		_outtext(tempbuf);
		_settextposition(4, 40);
		if (cursection == 1 && modcar == 1)
			_settextcolor(15);
		else
			_settextcolor(8);
		sprintf(tempbuf, "Multiplier: %d", inst[curinst].patch.chars[1]);
		_outtext(tempbuf);
		_settextposition(5, 0);
		if (cursection == 2 && modcar == 0)
			_settextcolor(15);
		else
			_settextcolor(8);
		sprintf(tempbuf, "Attack: %d", inst[curinst].patch.att_dec[0]);
		_outtext(tempbuf);
		_settextposition(5, 40);
		if (cursection == 2 && modcar == 1)
			_settextcolor(15);
		else
			_settextcolor(8);
		sprintf(tempbuf, "Decay: %d", inst[curinst].patch.att_dec[1]);
		_outtext(tempbuf);
		_settextposition(6, 0);
		if (cursection == 3 && modcar == 0)
			_settextcolor(15);
		else
			_settextcolor(8);
		sprintf(tempbuf, "Sustain: %d", inst[curinst].patch.sus_rel[0]);
		_outtext(tempbuf);
		_settextposition(6, 40);
		if (cursection == 3 && modcar == 1)
			_settextcolor(15);
		else
			_settextcolor(8);
		sprintf(tempbuf, "Release: %d", inst[curinst].patch.sus_rel[1]);
		_outtext(tempbuf);
		_settextposition(7, 0);
		if (cursection == 4 && modcar == 0)
			_settextcolor(15);
		else
			_settextcolor(8);
		sprintf(tempbuf, "Waveform: %d", inst[curinst].patch.wav_sel[0]);
		_outtext(tempbuf);
		_settextposition(7, 40);
		if (cursection == 4 && modcar == 1)
			_settextcolor(15);
		else
			_settextcolor(8);
		sprintf(tempbuf, "Waveform: %d", inst[curinst].patch.wav_sel[1]);
		_outtext(tempbuf);
		_settextposition(8, 0);
		if (cursection == 5)
			_settextcolor(15);
		else
			_settextcolor(8);
		sprintf(tempbuf, "Feedback: %d", inst[curinst].patch.fb_conn);
		_outtext(tempbuf);
		_settextposition(9, 0);
		if (cursection == 6)
			_settextcolor(15);
		else
			_settextcolor(8);
		sprintf(tempbuf, "MT-32 instrument: %d", inst[curinst].mt32inst);
		_outtext(tempbuf);
		_settextposition(10, 0);
		if (cursection == 7)
			_settextcolor(15);
		else
			_settextcolor(8);
		sprintf(tempbuf, "General MIDI instrument: %d", inst[curinst].geninst);
		_outtext(tempbuf);
		break;
		case 2:
		_outtext("Recording......\n");
		sprintf(tempbuf, "Beat: %d", songplc);
		_settextposition(2, 0);
		_outtext(tempbuf);
		break;
		case 3:
		k = 1;
		for(i=startnote;i<endnote;i++)
		{
			if (note[i].freq == 0)
			{
				sprintf(tempbuf, "%.4d No note", i);
				if (note[i].event)
				{
					sprintf(tempbuf2, ", %s %d", eventstr[note[i].event], note[i].paramanter);
					strcat(tempbuf, tempbuf2);
				}
			}	
			else
			{
				sprintf(tempbuf, "%.4d Note at %i, Track %i, volume=%d", i, note[i].freq, note[i].track, note[i].volume);
				if (note[i].event)
				{
					sprintf(tempbuf2, ", %s %d", eventstr[note[i].event], note[i].paramanter);
					strcat(tempbuf, tempbuf2);
				}
			}
			_settextposition(k, 0);
			if (i == cursection)
				_settextcolor(14);
			else
				_settextcolor(6);
			_outtext(tempbuf);
			k++;
			_settextposition(0, 60);
			sprintf(tempbuf, "Number of notes: %d", numnotes);
			_outtext(tempbuf);
		}
		/*sprintf(tempbuf, "Note at %i", note[cursection].freq);
		_settextposition(1, 0);
		_outtext(tempbuf);
		sprintf(tempbuf, "Number of notes: %d", numnotes);
		_settextposition(2, 0);
		_outtext(tempbuf);
		sprintf(tempbuf, "Current note: %d", cursection);
		_settextposition(3, 0);
		_outtext(tempbuf);
		if (note[cursection].event)
		{
			_settextposition(4, 0);
			sprintf(tempbuf, "%s %d", eventstr[note[cursection].event], note[cursection].paramanter);
			_outtext(tempbuf);
		}
		_settextposition(5, 0);
		sprintf(tempbuf, "Octave: %d", octave);
		_outtext(tempbuf);*/
		break;
	}
}

int getnote(short *freq)
{
	int delta;

	if (key < 50 && key > 16)
	{
		if (key == 28) return(0);
		delta = key-16*octave+1;
		if (octave > 1) delta += 16+key*octave+1;
		*freq = delta;
		key = 0;
		return(1);
	}
	return(0);
}

void edittrack(void)
{
	long i, j;
	short dafreq;
	
	editmode = 3;
	updatescreen();
	while (key != 1)
	{
		if (getnote(&dafreq) == 1)
		{
			asksave = 1;
			if (dafreq > 127) dafreq = 127;
			if (!oldvolume) oldvolume = 50;
			note[cursection].freq = dafreq;
			note[cursection].track = curtrack;
			note[cursection].volume = oldvolume;
			noteoff(curtrack);
			noteon(dafreq, curtrack, note[cursection].volume);
			if (cursection > numnotes) numnotes += cursection - numnotes;
			updatescreen();
		}
		if (key == 0x2)
		{
			key = 0;
			startcopy = cursection;
		}
		if (key == 0x3)
		{
			key = 0;
			endcopy = cursection;
		}
		if (key == 0x4)
		{
			key = 0;
			j = 0;
			for(i=startcopy;i<endcopy;i++)
			{
				if (i+cursection > numnotes) j++;
				note[cursection+(i-startcopy)].freq = note[i].freq;
				note[cursection+(i-startcopy)].track = note[i].track;
				note[cursection+(i-startcopy)].event = note[i].event;
				note[cursection+(i-startcopy)].paramanter = note[i].paramanter;
				note[cursection+(i-startcopy)].volume = note[i].volume;
			}
			numnotes += j;
			updatescreen();
		}
		if (key == 0xc8)
		{
			key = 0;
			if (cursection > 0) cursection--;
			if (cursection < startnote && startnote > 0)
			{
				startnote -= 24;
				endnote -= 24;
			}
			updatescreen();
		}
		if (key == 0xd0)
		{
			key = 0;
			if (cursection < MAXNOTES) cursection++;
			if (cursection > endnote-1 && endnote < MAXNOTES)
			{
				startnote += 24;
				endnote += 24;
			}
			updatescreen();
		}
		if (key == 0x9c)
		{
			key = 0;
			asksave = 1;
			note[cursection].event++;
			if (note[cursection].event == 2)
				noteoff(curtrack);
			if (note[cursection].event > 8)
				note[cursection].event = 0;
			updatescreen();
		}
		if (key == 12)
		{
			key = 0;
			asksave = 1;
			if (note[cursection].event)
			{
				if (note[cursection].paramanter > 0)
					note[cursection].paramanter--;
			}
			else
			{
				if (note[cursection].volume > 1)
					note[cursection].volume--;
				oldvolume = note[cursection].volume;
			}
			updatescreen();
		}
		if (key == 13)
		{
			key = 0;
			asksave = 1;
			if (note[cursection].event)
			{
				if (note[cursection].paramanter < 65535)
					note[cursection].paramanter++;
			}
			else
			{
				if (note[cursection].volume < 63)
					note[cursection].volume++;
				oldvolume = note[cursection].volume;
			}
			updatescreen();
		}
		if (key == 78)
		{
			key = 0;
			if (octave < 2) octave++;
			updatescreen();
		}
		if (key == 74)
		{
			key = 0;
			if (octave > 1) octave--;
			updatescreen();
		}
		if (key == 0xd3)
		{
			key = 0;
			asksave = 1;
			noteoff(curtrack);
			note[cursection].paramanter = 0;
			note[cursection].event = 0;
			note[cursection].freq = 0;
			//note[cursection].track = 0;
			updatescreen();
		}
	}
	key = 0;
	mainscreen();
}

void record(void)
{
	long i;
	short dafreq;

	editmode = 2;
	songplc = 0;
	musicstatus = 1;
	octave = 1;
	asksave = 1;
	updatescreen();
	while (key != 28)
	{
		if (songplc > numnotes)
		{
			updatescreen();
			numnotes++;
			musicstatus = 1;
			if (numnotes > MAXNOTES)
			{
				numnotes = MAXNOTES;
				allnotesoff();
				mainscreen();
			}
		}
		if (getnote(&dafreq) == 1)
		{
			if (lastfreq != dafreq)
			{
				note[songplc].freq = dafreq;
				note[songplc].track = curtrack;
				lastfreq = dafreq;
				//noteon(dafreq, curtrack, 255);
			}
			else
			{
				noteoff(curtrack);
				note[songplc].event = 2;
				note[songplc].paramanter = curtrack;
				lastfreq = 0;
			}
		}
		if (key == 78)
		{
			key = 0;
			if (octave < 2) octave++;
			updatescreen();
		}
		if (key == 74)
		{
			key = 0;
			if (octave > 1) octave--;
			updatescreen();
		}
	}
	musicstatus = 0;
	allnotesoff();
	mainscreen();
}

void instscreen(void)
{
	char ch, tempbuf[80];

	editmode = 1;
	updatescreen();
	while (key != 1)
	{
		switch (key)
		{
			case 50:
			key = 0;
			memcpy(&clipinst, &inst[curinst], sizeof(INSTDATA));
			break;
			case 25:
			key = 0;
			memcpy(&inst[curinst], &clipinst, sizeof(INSTDATA));
			updatescreen();
			askupdate = 1;
			break;
			case 0xd0:
			key = 0;
			cursection++;
			if (cursection > 7) cursection = 7;
			updatescreen();
			break;
			case 0xc8:
			key = 0;
			cursection--;
			if (cursection < 0) cursection = 0;
			updatescreen();
			break;
			case 0xcd:
			key = 0;
			modcar = 1;
			updatescreen();
			break;
			case 0xcb:
			key = 0;
			modcar = 0;
			updatescreen();
			break;
			case 22:
			copyibk();
			updatescreen();
			break;
			case 13:
			key = 0;
			askupdate = 1;
			switch (cursection)
			{
				case 0:
				if (inst[curinst].patch.ksl_lev[modcar] < 255)
					inst[curinst].patch.ksl_lev[modcar]++;
				updatescreen();
				break;
				case 1:	
				if (inst[curinst].patch.chars[modcar] < 255)
					inst[curinst].patch.chars[modcar]++;
				updatescreen();
				break;
				case 2:
				if (inst[curinst].patch.att_dec[modcar] < 255)
					inst[curinst].patch.att_dec[modcar]++;
				updatescreen();
				break;
				case 3:
				if (inst[curinst].patch.sus_rel[modcar] < 255)
					inst[curinst].patch.sus_rel[modcar]++;
				updatescreen();
				break;
				case 4:
				if (inst[curinst].patch.wav_sel[modcar] < 255)
					inst[curinst].patch.wav_sel[modcar]++;
				updatescreen();
				break;
				case 5:
				if (inst[curinst].patch.fb_conn < 255)
					inst[curinst].patch.fb_conn++;
				updatescreen();
				break;
				case 6:
				if (inst[curinst].mt32inst < 128)
					inst[curinst].mt32inst++;
				updatescreen();
				break;
				case 7:	
				if (inst[curinst].geninst < 182)
					inst[curinst].geninst++;
				updatescreen();
				break;
			}
			break;
			case 12:
			key = 0;
			askupdate = 1;
			switch (cursection)
			{
				case 0:
				if (inst[curinst].patch.ksl_lev[modcar] > 0)
					inst[curinst].patch.ksl_lev[modcar]--;
				updatescreen();
				break;
				case 1:	
				if (inst[curinst].patch.chars[modcar] > 0)
					inst[curinst].patch.chars[modcar]--;
				updatescreen();
				break;
				case 2:
				if (inst[curinst].patch.att_dec[modcar] > 0)
					inst[curinst].patch.att_dec[modcar]--;
				updatescreen();
				break;
				case 3:
				if (inst[curinst].patch.sus_rel[modcar] > 0)
					inst[curinst].patch.sus_rel[modcar]--;
				updatescreen();
				break;
				case 4:
				if (inst[curinst].patch.wav_sel[modcar] > 0)
					inst[curinst].patch.wav_sel[modcar]--;
				updatescreen();
				break;
				case 5:
				if (inst[curinst].patch.fb_conn > 0)
					inst[curinst].patch.fb_conn--;
				updatescreen();
				break;
				case 6:
				if (inst[curinst].mt32inst > 0)
					inst[curinst].mt32inst--;
				updatescreen();
				break;
				case 7:	
				if (inst[curinst].geninst > 0)
					inst[curinst].geninst--;
				updatescreen();
				break;
			}
			break;
			case 57:
			key = 0;
			noteoff(curtrack);
			trinst[curtrack] = curinst;
			setinstruments();
			noteon(5, curtrack, 50);
			break;
			case 73:
			key = 0;
			if (curinst < 256)
				curinst++;
			updatescreen();
			break;
			case 81:
			key = 0;
			if (curinst > 0)
				curinst--;
			updatescreen();
			break;
		}
	}
	key = 0;
	trinst[curtrack] = curinst;
	setinstruments();
	mainscreen();
}

void savesong(char *filename)
{
	long fil;

	fil = open(filename, O_BINARY|O_CREAT|O_TRUNC|O_WRONLY,S_IWRITE);
	if (fil == -1)
	{
		printf("Song cannot be saved!\n");
		exit(-1);
	}
	sngsig[0] = 'S';
	sngsig[1] = 'N';
	sngsig[2] = 'G';
	sngsig[3] = 0x1a;
	write(fil, &sngsig, 4);
	write(fil, &songtempo, 2);
	write(fil, &numnotes, 2);
	write(fil, &trinst[0], 2*MAXCHANNELS);
	write(fil, &note[0], sizeof(Note_T)*numnotes);
	close(fil);
}

void specifysngname(int saveload)
{
	char filename[128];

	_clearscreen(_GCLEARSCREEN);
	uninitkbd();
	printf("Name: ");
	scanf("%s", filename);
	if (saveload == 0)
	{
		asksave = 0;
		savesong(filename);
	}
	else
		loadsong(filename);
	strcpy(sngfilename, filename);
	initkbd();
	updatescreen();
}

void copyibk(void)
{
	char filename[128];
	long i, j, fil, curpatch;
	int inststart, instend;

	uninitkbd();
	_clearscreen(_GCLEARSCREEN);
	_settextcolor(8);
	_settextposition(1, 0);
	_outtext("IBK file: ");
	scanf("%s", filename);
	initkbd();
	if ((fil = open(filename, O_BINARY|O_RDONLY,S_IREAD)) == -1)
		return;

	read(fil, &ibksig[0], 4);
	if (strcmp(ibksig, "IBK\x1A") != 0)
		return;
	read(fil, &ibkpatch[0], sizeof(ibkinst_t)*128);
	read(fil, &ibknames[0], 9*128);
	close(fil);

	key = 0;
	inststart = 0;
	instend = 24;
	curpatch = 0;
	_clearscreen(_GCLEARSCREEN);
	while (1)
	{
		j = 1;
		for(i=inststart;i<instend;i++)
		{
			_settextposition(j, 0);
			if (curpatch == i) _settextcolor(15);
			else _settextcolor(8);
			_outtext(ibknames[i]);
			j++;
		}
		if (key == 0xc8)
		{
			key = 0;
			if (curpatch > 0) curpatch--;
			if (curpatch < inststart)
			{
				instend--;
				inststart--;
				_clearscreen(_GCLEARSCREEN);
			}
		}
		if (key == 0xd0)
		{
			key = 0;
			if (curpatch < 128) curpatch++;
			if (curpatch > instend-1)
			{
				instend++;
				inststart++;
				_clearscreen(_GCLEARSCREEN);
			}
		}
		if (key == 0x1c)
		{
			key = 0;
			inst[curinst].patch = ibkpatch[curpatch].patch;
			askupdate = 1;
			break;
		}
		if (key == 0x1)
		{
			key = 0;
			break;
		}
	}
}

void changetempo(void)
{
	_clearscreen(_GCLEARSCREEN);
	uninitkbd();
	printf("Tempo: ");
	scanf("%d", &songtempo);
	asksave = 1;
	initkbd();
	mainscreen();
}

void mainscreen(void)
{
	long i;
	char ch;

	editmode = 0;
	updatescreen();
	while (key != 1)
	{
		switch (key)
		{
			case 49:
			key = 0;
			uninitkbd();
			_clearscreen(_GCLEARSCREEN);
			_settextposition(0, 0);
			_outtext("Are you sure you want to start a new song?");
			ch = getch();
			if (ch == 'y' || ch == 'Y')
			{
				numnotes = 0;
				songtempo = 120;
				memset(note, 0, sizeof(Note_T)*MAXNOTES);
				for(i=0;i<MAXCHANNELS;i++)
					trinst[i] = 0;
			}
			initkbd();
			updatescreen();
			break;
			case 0xc8:
			curtrack--;
			if (curtrack < 1) curtrack = 1;
			updatescreen();
			key = 0;
			break;
			case 0xd0:
			curtrack++;
			if (curtrack > MAXCHANNELS-1) curtrack = MAXCHANNELS-1;
			updatescreen();
			key = 0;
			break;
			case 23:
			key = 0;
			curinst = trinst[curtrack];
			instscreen();
			break;
			case 19:
			key = 0;
			allnotesoff();
			setinstruments();
			record();
			break;
			case 31:
			key = 0;
			specifysngname(0);
			break;
			case 38:
			key = 0;
			specifysngname(1);
			break;
			case 18:
			key = 0;
			cursection = startnote;
			octave = 1;
			setinstruments();
			edittrack();
			break;
			case 25:
			key = 0;
			allnotesoff();
			songplc = 0;
			musicloop = 0;
			musicstatus ^= 1;
			break;
			case 20:
			changetempo();
			break;
		}
	}
	key = 0;
	uninit();	
	exit(0);
}

void uninit(void)
{
	char ch;
	
	uninittimer();
	uninitmmd();
	uninitkbd();
	_setvideomode(_DEFAULTMODE);
	if (askupdate)
	{
		printf("Update INSTS.DAT?\n");
		ch = getch();
		if (ch == 'y' || ch == 'Y')
			saveinst();
	}
	if (asksave)
	{
		printf("Save changes?\n");
		ch = getch();
		if (ch == 'y' || ch == 'Y')
		{
			savesong(sngfilename);
		}
	}
}

int main(int argc, char *argv[])
{
	int mdriver = 1;

	if (argc > 1)
	{
		loadsong(argv[1]);
	}
	if (argc > 2)
	{
		mdriver = atol(argv[2]);
		if (mdriver > 3) mdriver = 3;
	}
	if (initmmd(mdriver) == -1)
	{
		printf("Cannot find sound card or error with sound card itself.\n");
		exit(-1);
	}
	initkbd();
	outp(0x43,0); outp(0x40,1193181/120&255); outp(0x40,1193181/120>>8);
	songspeed = 10;
	songplc = 0;
	timecount = 0;
	songtempo = 120;
	curtrack = 1;
	startnote = 0;
	endnote = 24;
	setmusicvolume(255);
	inittimer();
	_setvideomode(_TEXTC80);
	mainscreen();
	return(0);
}
