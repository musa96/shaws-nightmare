#include <fcntl.h>
#include <conio.h>
#include <io.h>
#include <dos.h>
#include <sys\stat.h>
#include <sys\types.h>
#include <string.h>
#include "qaf.h"
#include "dat.h"
#include "build.h"
#include "pragmas.h"
#include "engine.h"

#define KEYFIFOSIZ 64
#define TICRATE (120)

int qafpoint;
int curframe, curlayer;
long gettilezoom = 1;
static char qaffilename[13], oqaffilename[13];
static char plock = 255;
static int qaftoconvertcnt;
static char convertnames[255][128];

Qaf_T qaf;
typedef struct
{
	long x, y, zoom;
	short angle, picnum;
	signed char shade;
	char pal;
	int trigger;
} OldFrame_T;

static Frame_T clipboard[MAXLAYERS];
static OldFrame_T oldframe[MAXFRAMES][MAXLAYERS];
static int oldnumframes, oldnumlayers, oldtpf;

extern char qafheader[4];
extern long frameplace;
static short localartfreq[MAXTILES];
static short localartlookup[MAXTILES], localartlookupnum;

static char scantoasc[128] =
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
char datfilename[80];
void (__interrupt __far *oldkeyhandler)();
void __interrupt __far keyhandler(void);
volatile char keystatus[256], keyfifo[KEYFIFOSIZ], keyfifoplc, keyfifoend;
volatile char readch, oldreadch, extended, keytemp;
volatile long tick;

void (__interrupt __far *oldtimerhandler)();
void __interrupt __far timerhandler(void);

void finddats(void)
{
	if (access("SN.DAT", R_OK) == 0)
	{
		strcpy(datfilename, "SN.DAT");
		return;
	}
	if (access("SN2.DAT", R_OK) == 0)
	{
		strcpy(datfilename, "SN2.DAT");
		return;
	}
}

void printmessage256(char name[82])
{
	char snotbuf[40];
	long i;

	i = 0;
	while ((name[i] != 0) && (i < 38))
	{
		snotbuf[i] = name[i];
		i++;
	}
	while (i < 38)
	{
		snotbuf[i] = 32;
		i++;
	}
	snotbuf[38] = 0;

	printext256(0L,0L,31,0,snotbuf,0);
}

getnumber256(char namestart[80], short num, long maxnumber)
{
	char buffer[80];
	long j, k, n, danum, oldnum;

	danum = (long)num;
	oldnum = danum;
	while ((keystatus[0x1c] != 2) && (keystatus[0x1] == 0))
	{
		clearview(0L);

		sprintf(&buffer,"%s%ld_ ",namestart,danum);
		printmessage256(buffer);
		nextpage();

		for(j=2;j<=11;j++)
			if (keystatus[j] > 0)
			{
				keystatus[j] = 0;
				k = j-1;
				if (k == 10) k = 0;
				n = (danum*10)+k;
				if (n < maxnumber) danum = n;
			}
		if (keystatus[0xe] > 0)    // backspace
		{
			danum /= 10;
			keystatus[0xe] = 0;
		}
		if (keystatus[0x1c] == 1)
		{
			oldnum = danum;
			keystatus[0x1c] = 2;
		}
	}
	keystatus[0x1c] = 0;
	keystatus[0x1] = 0;

	return((short)oldnum);
}

void findfiles(char *path)
{
	long rc;
	struct find_t fileinfo;

	rc = _dos_findfirst(path, _A_SUBDIR, &fileinfo);
	while (rc == 0)
	{
		strcpy(convertnames[qaftoconvertcnt],fileinfo.name);
		qaftoconvertcnt++;
		rc = _dos_findnext(&fileinfo);
	}
}

void loadoldqaf(char *name)
{
	long i, j, handle;

	strcpy(qaffilename,name);
	handle = open(name, O_BINARY|O_RDONLY,S_IREAD);
	if (handle == -1)
		error("Cannot open QAF file", 9100);
	read(handle, &qafheader, 4);
	if (qafheader[0] != 'Q' && qafheader[1] != 'A' && qafheader[2] != 'F'
		&& qafheader[3] != 0x1a)
		error("QAF file is corrupt", 9101);
	read(handle, &oldnumframes, 2);
	read(handle, &oldnumlayers, 2);
	read(handle, &oldtpf, 2);
	for(i=0;i<oldnumlayers;i++)
		for(j=0;j<oldnumframes;j++)
			read(handle, &oldframe[j][i], sizeof(OldFrame_T));
	close(handle);
}

void convqaf(void)
{
	long i, j;
	
	qaf.numlayers = oldnumlayers;
	qaf.numframes = oldnumframes;
	qaf.tpf = oldtpf;
	for(i=0;i<oldnumframes;i++)
	{
		for(j=0;j<oldnumlayers;j++)
		{
			qaf.layer[j].frame[i].x = oldframe[i][j].x;
			qaf.layer[j].frame[i].y = oldframe[i][j].y;
			qaf.layer[j].frame[i].zoom = oldframe[i][j].zoom;
			qaf.layer[j].frame[i].angle = oldframe[i][j].angle;
			qaf.layer[j].frame[i].picnum = oldframe[i][j].picnum;
			qaf.layer[j].frame[i].shade = oldframe[i][j].shade;
			qaf.layer[j].frame[i].pal = oldframe[i][j].pal;
			qaf.layer[j].frame[i].stat = 0;
			qaf.layer[j].frame[i].trigger = oldframe[i][j].trigger;
		}
	}
	saveqaf(qaffilename);
	printf("%s successfully converted.\n", qaffilename);
}

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
	outp(0x20,0x20);
}

void faketimerhandler(void)
{
}

void qaftrigger(int trigger)
{
}

void initkeys(void)
{
	long i;

	keyfifoplc = 0; keyfifoend = 0;
	for(i=0;i<256;i++) keystatus[i] = 0;
	oldkeyhandler = _dos_getvect(0x9);
	_disable(); _dos_setvect(0x9, keyhandler); _enable();
}

void uninitkeys(void)
{
	short *ptr;

	_dos_setvect(0x9, oldkeyhandler);
		//Turn off shifts to prevent stucks with quitting
	ptr = (short *)0x417; *ptr &= ~0x030f;
}

void __interrupt __far keyhandler()
{
	oldreadch = readch; readch = kinp(0x60);
	keytemp = kinp(0x61); koutp(0x61,keytemp|128); koutp(0x61,keytemp&127);
	koutp(0x20,0x20);
	if ((readch|1) == 0xe1) { extended = 128; return; }
	if (oldreadch != readch)
	{
		if ((readch&128) == 0)
		{
			keytemp = readch+extended;
			if (!keystatus[keytemp])
			{
				keystatus[keytemp] = 1;
				keyfifo[keyfifoend] = keytemp;
				keyfifo[(keyfifoend+1)&(KEYFIFOSIZ-1)] = 1;
				keyfifoend = ((keyfifoend+2)&(KEYFIFOSIZ-1));
			}
		}
		else
		{
			keytemp = (readch&127)+extended;
			keystatus[keytemp] = 0;
			keyfifo[keyfifoend] = keytemp;
			keyfifo[(keyfifoend+1)&(KEYFIFOSIZ-1)] = 0;
			keyfifoend = ((keyfifoend+2)&(KEYFIFOSIZ-1));
		}
	}
	extended = 0;
}

void openqaf(char *filename)
{
	long i, j, handle;

	handle = open(filename, O_BINARY|O_RDONLY,S_IREAD);
	if (handle == -1)
	{
		error("Error loading QAF file", 9100);
	}
	read(handle, &qafheader, 4);
	if (qafheader[0] != 'Q' && qafheader[1] != 'A' && qafheader[2] != 'F'
		&& qafheader[3] != 0x1a)
		error("QAF file is corrupt", 9101);
	read(handle, &qaf.numframes, 2);
	read(handle, &qaf.numlayers, 2);
	read(handle, &qaf.tpf, 2);
	for(i=0;i<qaf.numlayers;i++)
		read(handle, &qaf.layer[i].frame[0], sizeof(Frame_T)*qaf.numframes);
	for(i=0;i<qaf.numlayers;i++)
	{
		for(j=0;j<qaf.numframes;j++)
			if (waloff[qaf.layer[i].frame[j].picnum] == 0)
				loadtile(qaf.layer[i].frame[j].picnum);
	}
	close(handle);
}

void saveqaf(char *filename)
{
	long i, handle;

	handle = open(filename, O_BINARY|O_WRONLY|O_CREAT|O_TRUNC,S_IWRITE);
	if (handle == -1)
	{
		error("Error saving QAF file", 9100);
	}
	qafheader[0] = 'Q';
	qafheader[1] = 'A';
	qafheader[2] = 'F';
	qafheader[3] = 0x1a;
	write(handle, &qafheader, 4);
	write(handle, &qaf.numframes, 2);
	write(handle, &qaf.numlayers, 2);
	write(handle, &qaf.tpf, 2);
	for(i=0;i<qaf.numlayers;i++)
		write(handle, &qaf.layer[i].frame[0], sizeof(Frame_T)*qaf.numframes);
	close(handle);
}

void specifyqafname(int saveload)
{
	int bad;
	long i, j;
	char buffer[80];

	bad = 0;
	strcpy(oqaffilename,qaffilename);

	i = 0;
	while ((qaffilename[i] != 0) && (i < 13))
		i++;
	if (qaffilename[i-4] == '.')
		i -= 4;
	qaffilename[i] = 0;
	while (bad == 0)
	{
		clearview(0L);
		if (saveload == 0)
			sprintf(buffer,"Save as: %s",qaffilename);
		else
			sprintf(buffer,"Load: %s", qaffilename);
		printext256(0, 0, 31, -1, buffer, 0);
		nextpage();

		if (keystatus[1] > 0) bad = 1;
		if (keystatus[0x1c] > 0) bad = 2;

		if (i > 0)
		{
			if (keystatus[0xe] > 0)
			{
				keystatus[0xe] = 0;
				i--;
				qaffilename[i] = 0;
			}
		}
		if (i < 8)
		{
			if ((keystatus[0x2a]|keystatus[0x36]) > 0)
			{
				for(j=0;j<128;j++)
				if (scantoascwithshift[j] > 0)
					if (keystatus[j] > 0)
					{
						keystatus[j] = 0;
						qaffilename[i++] = scantoascwithshift[j];
						qaffilename[i] = 0;
					}
				}
				else
				{
					for(j=0;j<128;j++)
						if (scantoasc[j] > 0)
							if (keystatus[j] > 0)
							{
								keystatus[j] = 0;
								qaffilename[i++] = scantoasc[j];
								qaffilename[i] = 0;
							}
						}
					}
				}
				if (bad == 1)
				{
					strcpy(qaffilename,oqaffilename);
					keystatus[1] = 0;
				}
				if (bad == 2)
				{
					keystatus[0x1c] = 0;

					qaffilename[i] = '.';
					qaffilename[i+1] = 'q';
					qaffilename[i+2] = 'a';
					qaffilename[i+3] = 'f';
					qaffilename[i+4] = 0;
					if (saveload == 0) saveqaf(qaffilename);
					else openqaf(qaffilename);
					bad = 0;
				}
}

void loadpalettes(void)
{
	long fil;

	fil = loaditem("SN.PAL");
	readitem(fil, palette, 768);
}

void loadlookup(void)
{
	long i, fil;
	char tempbuf[256];

	numpalookups = 32;
	if ((palookup[0] = (char *)kmalloc(numpalookups<<8)) == NULL)
		allocache(&palookup[0],numpalookups<<8,&plock);

	globalpalwritten = palookup[0]; globalpal = 0;
	setpalookupaddress(globalpalwritten);

	fil = loaditem("LOOKUP.DAT");
	if (itemlength("LOOKUP.DAT") != LOOKUPSIZ)
		error("Incorrect lookup size", 212);
	readitem(fil,palookup[globalpal],numpalookups<<8);
	for(i=0;i<NUMPALOOKUPS;i++)
	{
		readitem(fil,&tempbuf[0],256);
		makepalookup(i+1,tempbuf,0,0,0,1);
	}
}

void loadtrans(void)
{
	long fil;

	if ((transluc = (char *)kmalloc(65536L)) == NULL)
		allocache(&transluc,65536,&plock);

	fixtransluscence(FP_OFF(transluc));

	fil = loaditem("TRANS.TBL");
	readitem(fil,transluc,65536);
}

void puttext(long x, long y, char *str)
{
	printext256(x, y, 31, -1, str, 1);
}

drawtilescreen(long pictopleft, long picbox)
{
	long i, j, vidpos, vidpos2, dat, wallnum, xdime, ydime, cnt, pinc;
	long dax, day, scaledown, xtiles, ytiles, tottiles;
	char *picptr, snotbuf[80];

	xtiles = (xdim>>6); ytiles = (ydim>>6); tottiles = xtiles*ytiles;

	pinc = ylookup[1];
	clearview(0L);
	for(cnt=0;cnt<(tottiles<<(gettilezoom<<1));cnt++)         //draw the 5*3 grid of tiles
	{
		wallnum = cnt+pictopleft;
		if (wallnum < localartlookupnum)
		{
			wallnum = localartlookup[wallnum];
			if ((tilesizx[wallnum] != 0) && (tilesizy[wallnum] != 0))
			{
				if (waloff[wallnum] == 0) loadtile(wallnum);
				picptr = (char *)(waloff[wallnum]);
				xdime = tilesizx[wallnum];
				ydime = tilesizy[wallnum];

				dax = ((cnt%(xtiles<<gettilezoom))<<(6-gettilezoom));
				day = ((cnt/(xtiles<<gettilezoom))<<(6-gettilezoom));
				vidpos = ylookup[day]+dax+frameplace;
				if ((xdime <= (64>>gettilezoom)) && (ydime <= (64>>gettilezoom)))
				{
					for(i=0;i<xdime;i++)
					{
						vidpos2 = vidpos+i;
						for(j=0;j<ydime;j++)
						{
							*(char *)vidpos2 = *picptr++;
							vidpos2 += pinc;
						}
					}
				}
				else                          //if 1 dimension > 64
				{
					if (xdime > ydime)
						scaledown = ((xdime+(63>>gettilezoom))>>(6-gettilezoom));
					else
						scaledown = ((ydime+(63>>gettilezoom))>>(6-gettilezoom));

					for(i=0;i<xdime;i+=scaledown)
					{
						if (waloff[wallnum] == 0) loadtile(wallnum);
						picptr = (char *)(waloff[wallnum]) + ydime*i;
						vidpos2 = vidpos;
						for(j=0;j<ydime;j+=scaledown)
						{
							*(char *)vidpos2 = *picptr;
							picptr += scaledown;
							vidpos2 += pinc;
						}
						vidpos++;
					}
				}
				if (localartlookupnum < MAXTILES)
				{
					dax = ((cnt%(xtiles<<gettilezoom))<<(6-gettilezoom));
					day = ((cnt/(xtiles<<gettilezoom))<<(6-gettilezoom));
					sprintf(snotbuf,"%ld",localartfreq[cnt+pictopleft]);
					printext256(dax,day,31,-1,snotbuf,1);
				}
			}
		}
	}

	cnt = picbox-pictopleft;    //draw open white box
	dax = ((cnt%(xtiles<<gettilezoom))<<(6-gettilezoom));
	day = ((cnt/(xtiles<<gettilezoom))<<(6-gettilezoom));

	for(i=0;i<(64>>gettilezoom);i++)
	{
		plotpixel(dax+i,day,31);
		plotpixel(dax+i,day+(63>>gettilezoom),31);
		plotpixel(dax,day+i,31);
		plotpixel(dax+(63>>gettilezoom),day+i,31);
	}

	i = localartlookup[picbox];
	sprintf(snotbuf,"%ld",i);
	printext256(0L,ydim-8,31,-1,snotbuf,0);

	return(0);
}

gettile(long tilenum)
{
	char snotbuf[80];
	long i, j, k, otilenum, topleft, gap, temp, templong;
	long xtiles, ytiles, tottiles;

	xtiles = (xdim>>6); ytiles = (ydim>>6); tottiles = xtiles*ytiles;
	otilenum = tilenum;

	keystatus[0x2f] = 0;
	for(i=0;i<MAXTILES;i++)
	{
		if (tilenum < localartlookupnum)
			tilenum = localartlookup[tilenum];
		else
			tilenum = 0;
		localartlookupnum = MAXTILES;
		for(i=0;i<MAXTILES;i++)
			localartlookup[i] = i;
	}
	gap = (MAXTILES>>1);
	do
	{
		for(i=0;i<MAXTILES-gap;i++)
		{
			temp = i;
			while ((localartfreq[temp] < localartfreq[temp+gap]) && (temp >= 0))
			{
				templong = localartfreq[temp];
				localartfreq[temp] = localartfreq[temp+gap];
				localartfreq[temp+gap] = templong;
				templong = localartlookup[temp];
				localartlookup[temp] = localartlookup[temp+gap];
				localartlookup[temp+gap] = templong;

				if (tilenum == temp)
					tilenum = temp+gap;
				else if (tilenum == temp+gap)
					tilenum = temp;

				temp -= gap;
			}
		}
		gap >>= 1;
	}
	while (gap > 0);
	localartlookupnum = 0;
	while (localartfreq[localartlookupnum] > 0)
		localartlookupnum++;

	if (localartfreq[0] == 0)
	{
		tilenum = otilenum;
		localartlookupnum = MAXTILES;
		for(i=0;i<MAXTILES;i++)
			localartlookup[i] = i;
	}

	topleft = ((tilenum/(xtiles<<gettilezoom))*(xtiles<<gettilezoom))-(xtiles<<gettilezoom);
	if (topleft < 0) topleft = 0;
	if (topleft > MAXTILES-(tottiles<<(gettilezoom<<1))) topleft = MAXTILES-(tottiles<<(gettilezoom<<1));
	while ((keystatus[0x1c]|keystatus[1]) == 0)
	{
		drawtilescreen(topleft,tilenum);
		//if ((vidoption != 2) && ((vidoption != 1) || (vgacompatible == 1))) limitrate();

		nextpage();

		if ((keystatus[0x37] > 0) && (gettilezoom < 2))
		{
			gettilezoom++;
			topleft = ((tilenum/(xtiles<<gettilezoom))*(xtiles<<gettilezoom))-(xtiles<<gettilezoom);
			if (topleft < 0) topleft = 0;
			if (topleft > MAXTILES-(tottiles<<(gettilezoom<<1))) topleft = MAXTILES-(tottiles<<(gettilezoom<<1));
			keystatus[0x37] = 0;
		}
		if ((keystatus[0xb5] > 0) && (gettilezoom > 0))
		{
			gettilezoom--;
			topleft = ((tilenum/(xtiles<<gettilezoom))*(xtiles<<gettilezoom))-(xtiles<<gettilezoom);
			if (topleft < 0) topleft = 0;
			if (topleft > MAXTILES-(tottiles<<(gettilezoom<<1))) topleft = MAXTILES-(tottiles<<(gettilezoom<<1));
			keystatus[0xb5] = 0;
		}
		if ((keystatus[0xcb] > 0) && (tilenum > 0))
			tilenum--, keystatus[0xcb] = 0;
		if ((keystatus[0xcd] > 0) && (tilenum < MAXTILES-1))
			tilenum++, keystatus[0xcd] = 0;
		if ((keystatus[0xc8] > 0) && (tilenum >= (xtiles<<gettilezoom)))
			tilenum-=(xtiles<<gettilezoom), keystatus[0xc8] = 0;
		if ((keystatus[0xd0] > 0) && (tilenum < MAXTILES-(xtiles<<gettilezoom)))
			tilenum+=(xtiles<<gettilezoom), keystatus[0xd0] = 0;
		if ((keystatus[0xc9] > 0) && (tilenum >= (xtiles<<gettilezoom)))
		{
			tilenum-=(tottiles<<(gettilezoom<<1));
			if (tilenum < 0) tilenum = 0;
			keystatus[0xc9] = 0;
		}
		if ((keystatus[0xd1] > 0) && (tilenum < MAXTILES-(xtiles<<gettilezoom)))
		{
			tilenum+=(tottiles<<(gettilezoom<<1));
			if (tilenum >= MAXTILES) tilenum = MAXTILES-1;
			keystatus[0xd1] = 0;
		}
		if (keystatus[0x22] > 0)       //G (goto)
		{
			if (tilenum < localartlookupnum)         //Automatically press 'V'
				tilenum = localartlookup[tilenum];
			else
				tilenum = 0;
			localartlookupnum = MAXTILES;
			for(i=0;i<MAXTILES;i++)
				localartlookup[i] = i;

			keystatus[0x22] = 0;

			j = tilenum;
			while (keystatus[1] == 0)
			{
				drawtilescreen(topleft,tilenum);
				//if ((vidoption != 2) && ((vidoption != 1) || (vgacompatible == 1))) limitrate();
				sprintf(&snotbuf,"Goto tile: %d_ ",j);
				printext256(0,0,31,0,snotbuf,0);
				nextpage();

				for(k=0;k<10;k++)
					if (keystatus[((k+9)%10)+2] > 0)
					{
						keystatus[((k+9)%10)+2] = 0;
						i = (j*10)+k;
						if (i < MAXTILES) j = i;
					}
				if (keystatus[0xe] > 0)
				{
					keystatus[0xe] = 0;
					j /= 10;
				}
				if (keystatus[0x1c] > 0)
				{
					keystatus[0x1c] = 0;
					tilenum = j;
					break;
				}
			}
		}
		while (tilenum < topleft) topleft -= (xtiles<<gettilezoom);
		while (tilenum >= topleft+(tottiles<<(gettilezoom<<1))) topleft += (xtiles<<gettilezoom);
		if (topleft < 0) topleft = 0;
		if (topleft > MAXTILES-(tottiles<<(gettilezoom<<1))) topleft = MAXTILES-(tottiles<<(gettilezoom<<1));
	}

	if (keystatus[0x1c] == 0)
	{
		tilenum = otilenum;
	}
	else
	{
		if (tilenum < localartlookupnum)
		{
			tilenum = localartlookup[tilenum];
			if ((tilesizx[tilenum] == 0) || (tilesizy[tilenum] == 0))
				tilenum = otilenum;
		}
		else
			tilenum = otilenum;
	}
	keystatus[0x1] = 0;
	keystatus[0x1c] = 0;
	return(tilenum);
}

void init(void)
{
	finddats();
	loaddat(datfilename);
	initengine();
	inittimer();
	initkeys();
	if (loadpics("tiles000.art") == -1)
		error("Cannot load tile file", 6000);
	setgamemode(2, 320, 200);
}

void uninit(void)
{
	uninitengine();
	uninitkeys();
	uninittimer();
	setvmode(0x3);
}

void process_input(void)
{
	long i;

	if (keystatus[15] > 0)
	{
		keystatus[15] = 0;
		for(i=0;i<qaf.numlayers;i++)	
			copybufbyte(&qaf.layer[i].frame[curframe],&clipboard[i],sizeof(Frame_T));
	}
	if (keystatus[0x9c] > 0)
	{
		keystatus[0x9c] = 0;
		for(i=0;i<qaf.numlayers;i++)	
			copybufbyte(&clipboard[i],&qaf.layer[i].frame[curframe],sizeof(Frame_T));
	}
	if (keystatus[0xd2] > 0)
	{
		keystatus[0xd2] = 0;
		qaf.numlayers++;
		if (qaf.numlayers > MAXLAYERS)
			qaf.numlayers = MAXLAYERS;
		curlayer = qaf.numlayers-1;
	}
	if (keystatus[0xd3] > 0)
	{
		keystatus[0xd3] = 0;
		qaf.numlayers--;
		if (qaf.numlayers < 0) qaf.numlayers = 0;
	}
	if (keystatus[73] > 0)
	{
		keystatus[73] = 0;
		curlayer++;
		if (curlayer > qaf.numlayers-1)
			curlayer = qaf.numlayers-1;
	}
	if (keystatus[81] > 0)
	{
		keystatus[81] = 0;
		curlayer--;
		if (curlayer < 0) curlayer = 0;
	}
	if (keystatus[0xc8] > 0)
	{
		qaf.layer[curlayer].frame[curframe].y -= (8<<12);
	}
	if (keystatus[0xd0] > 0)
	{
		qaf.layer[curlayer].frame[curframe].y += (8<<12);
	}
	if (keystatus[0xcb] > 0)
	{
		qaf.layer[curlayer].frame[curframe].x -= (8<<12);
	}
	if (keystatus[0xcd] > 0)
	{
		qaf.layer[curlayer].frame[curframe].x += (8<<12);
	}
	if (keystatus[47] > 0)
	{
		qaf.layer[curlayer].frame[curframe].picnum = gettile(qaf.layer[curlayer].frame[curframe].picnum);
	}
	if (keystatus[82] > 0)
	{
		keystatus[82] = 0;
		qaf.numframes++;
		if (qaf.numframes > MAXFRAMES) qaf.numframes = MAXFRAMES;
		for(i=0;i<qaf.numlayers;i++)
			copybufbyte(&qaf.layer[i].frame[qaf.numframes-1],&qaf.layer[i].frame[qaf.numframes],sizeof(Frame_T));
		curframe = qaf.numframes;
	}
	if (keystatus[83] > 0)
	{
		keystatus[83] = 0;
		qaf.numframes--;
		if (qaf.numframes < 0) qaf.numframes = 0;
	}
	if (keystatus[0xc9] > 0)
	{
		keystatus[0xc9] = 0;
		curframe++;
		if (curframe > qaf.numframes-1) curframe = qaf.numframes-1;
	}
	if (keystatus[0xd1] > 0)
	{
		keystatus[0xd1] = 0;
		curframe--;
		if (curframe < 0) curframe = 0;
	}
	if (keystatus[28] > 0)
	{
		keystatus[28] = 0;
		curframe = 0;
		i = 1;
		qafpoint = 0;
		while (i == 1)
		{
			if (keystatus[1] > 0)
			{
				keystatus[1] = 0;
				return;
			}
			if (tick > 5)
			{
				tick = 0;
				clearview(0L);
				i = playqaf(&qaf, &curframe, &qafpoint);
				drawqaf(&qaf, curframe, 0, 2);
				nextpage();
			}
		}
	}
	if (keystatus[31] > 0)
	{
		keystatus[31] = 0;
		specifyqafname(0);
	}
	if (keystatus[38] > 0)
	{
		keystatus[38] = 0;
		specifyqafname(1);
	}
	if (keystatus[74] > 0)
	{
		keystatus[74] = 0;
		qaf.tpf--;
		if (qaf.tpf < 0) qaf.tpf = 0;
	}
	if (keystatus[78] > 0)
	{
		keystatus[78] = 0;
		qaf.tpf++;
		if (qaf.tpf > 12) qaf.tpf = 12;
	}
	if (keystatus[12] > 0)
	{
		keystatus[12] = 0;
		qaf.layer[curlayer].frame[curframe].shade++;
	}
	if (keystatus[13] > 0)
	{
		keystatus[13] = 0;
		qaf.layer[curlayer].frame[curframe].shade--;
	}
	if (keystatus[51] > 0)
	{
		keystatus[51] = 0;
		qaf.layer[curlayer].frame[curframe].angle -= 8;
	}

	if (keystatus[52] > 0)
	{
		keystatus[52] = 0;
		qaf.layer[curlayer].frame[curframe].angle += 8;
	}
	if (keystatus[26] > 0)
	{
		keystatus[26] = 0;
		qaf.layer[curlayer].frame[curframe].zoom /= 2;
	}
	if (keystatus[27] > 0)
	{
		keystatus[27] = 0;
		qaf.layer[curlayer].frame[curframe].zoom *= 2;
	}
	if (keystatus[25] > 0)
	{
		keystatus[25] = 0;
		qaf.layer[curlayer].frame[curframe].pal++;
		if (qaf.layer[curlayer].frame[curframe].pal > NUMPALOOKUPS)
			qaf.layer[curlayer].frame[curframe].pal = 0;
	}
	if (keystatus[33] > 0)
	{
		keystatus[33] = 0;
		qaf.layer[curlayer].frame[curframe].trigger = getnumber256("Trigger flag: ",
								qaf.layer[curlayer].frame[curframe].trigger,
								256L);
	}
	if (keystatus[20] > 0)
	{
		keystatus[20] = 0;
		if ((qaf.layer[curlayer].frame[curframe].stat&1) == 0)
			qaf.layer[curlayer].frame[curframe].stat |= 1;	
		else if ((qaf.layer[curlayer].frame[curframe].stat&32) == 0)
			qaf.layer[curlayer].frame[curframe].stat |= 32;
		else
			qaf.layer[curlayer].frame[curframe].stat &= ~(1+32);
	}
	if (keystatus[21] > 0)
	{
		keystatus[21] = 0;
		qaf.layer[curlayer].frame[curframe].stat ^= 4;
	}
	if (keystatus[0xb5] > 0)
	{
		keystatus[0xb5] = 0;
		qaf.layer[curlayer].frame[curframe].zoom /= 4;
	}
	if (keystatus[55] > 0)
	{
		keystatus[55] = 0;
		qaf.layer[curlayer].frame[curframe].zoom *= 4;
	}
}

void mainscreen(void)
{
	char tempbuf[80];
	long dax, day, i;

	while (keystatus[1] == 0)
	{
		clearview(0);
		drawqaf(&qaf, curframe, 0, 2);
		dax = (qaf.layer[curlayer].frame[curframe].x>>16)-32;
		day = (qaf.layer[curlayer].frame[curframe].y>>16)-32;
		sprintf(tempbuf, "X: %d", qaf.layer[curlayer].frame[curframe].x);
		puttext(0, 0, tempbuf);
		sprintf(tempbuf, "Y: %d", qaf.layer[curlayer].frame[curframe].y);
		puttext(60, 0, tempbuf);
		sprintf(tempbuf, "Zoom: %d", qaf.layer[curlayer].frame[curframe].zoom);
		puttext(120, 0, tempbuf);
		sprintf(tempbuf, "Angle: %d", qaf.layer[curlayer].frame[curframe].angle);
		puttext(180, 0, tempbuf);
		sprintf(tempbuf, "Shade: %d", qaf.layer[curlayer].frame[curframe].shade);
		puttext(240, 0, tempbuf);
		sprintf(tempbuf, "Pal: %d", qaf.layer[curlayer].frame[curframe].pal);
		puttext(280, 0, tempbuf);
		sprintf(tempbuf, "TPF: %d", qaf.tpf);
		puttext(0, 10, tempbuf);
		if (qaf.layer[curlayer].frame[curframe].trigger)
		{
			sprintf(tempbuf, "Trigger: %d", qaf.layer[curlayer].frame[curframe].trigger);
			puttext(30, 10, tempbuf);
		}
		nextpage();
		process_input();
	}
	keystatus[1] = 0;
}

int main(int argc, char *argv[])
{
	long i, j;

	printf("QAFEDIT v1.3	- By Michael Muniko\n");
	strcpy(qaffilename, "NEW.QAF");
	qaf.tpf = 6;
	if (stricmp(argv[1],"-c") == 0)
	{
		findfiles(argv[2]);
		for(i=0;i<qaftoconvertcnt;i++)
		{
			loadoldqaf(convertnames[i]);
			convqaf();
		}
		exit(0);
	}
	for(i=0;i<MAXLAYERS;i++)
	{
		for(j=0;j<MAXFRAMES;j++)
		{
			qaf.layer[i].frame[j].x = (320<<15);
			qaf.layer[i].frame[j].y = (200<<15);
			qaf.layer[i].frame[j].zoom = 65536L;
		}
	} 
	delay(300);
	init();
	if (argc >= 2)
	{
		strcpy(&qaffilename,argv[1]);
		if (strchr(qaffilename,'.') == 0)
			strcat(qaffilename,".qaf");
		openqaf(&qaffilename);
	}
	mainscreen();
	uninit();
	return(0);
}
