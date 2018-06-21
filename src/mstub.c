// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include "build.h"
#include "config.h"
#include "pragmas.h"
#include "engine.h"
#include "names.h"
#include "names2.h"
#include "dat.h"
#include <stdlib.h>
#include <dos.h>

#define PROG_ID "ShawEd Build v1.66 ["__DATE__"]"

static int spriteselected;
extern int selectedpicnum, selectedcstat;
extern int selectedstatus, selectedxrepeat, selectedyrepeat;
extern short selectedlotag;
int numsecrets, numenemiess12, numenemiess3, numenemiess45;

extern short asksave;
extern long searchx, searchy;                          //search input
extern short grid, gridlock;
extern long whitecol;
extern char keystatus[256];

static char tempbuf[256];
extern long qsetmode;
extern short searchsector, searchwall, searchstat;
extern long zmode, kensplayerheight;
extern short defaultspritecstat;

extern short temppicnum, tempcstat, templotag, temphitag, tempextra;
extern char tempshade, temppal, tempxrepeat, tempyrepeat;
extern char somethingintab;
static char plock = 255;
static char datfilename[80];

#define NUMKEYS 19
extern char buildkeys[NUMKEYS];

extern long frameplace, xdimenscale, ydimen;
extern long posx, posy, posz, horiz;
extern short ang, cursectnum;

extern long stretchhline(long,long,long,long,long,long);
#pragma aux stretchhline parm [eax][ebx][ecx][edx][esi][edi];

//Detecting 2D / 3D mode:
//   qsetmode is 200 in 3D mode
//   qsetmode is 350/480 in 2D mode
//
//You can read these variables when F5-F8 is pressed in 3D mode only:
//
//   If (searchstat == 0)  WALL        searchsector=sector, searchwall=wall
//   If (searchstat == 1)  CEILING     searchsector=sector
//   If (searchstat == 2)  FLOOR       searchsector=sector
//   If (searchstat == 3)  SPRITE      searchsector=sector, searchwall=sprite
//   If (searchstat == 4)  MASKED WALL searchsector=sector, searchwall=wall
//
//   searchsector is the sector of the selected item for all 5 searchstat's
//
//   searchwall is undefined if searchstat is 1 or 2
//   searchwall is the wall if searchstat = 0 or 4
//   searchwall is the sprite if searchstat = 3 (Yeah, I know - it says wall,
//                                      but trust me, it's the sprite number)

long ofinetotalclock, ototalclock, averagefps;
#define AVERAGEFRAMES 32
static long frameval[AVERAGEFRAMES], framecnt = 0;

#pragma aux inittimer42 =\
	"in al, 0x61",\
	"or al, 1",\
	"out 0x61, al",\
	"mov al, 0xb4",\
	"out 0x43, al",\
	"xor al, al",\
	"out 0x42, al",\
	"out 0x42, al",\
	modify exact [eax]\

#pragma aux uninittimer42 =\
	"in al, 0x61",\
	"and al, 252",\
	"out 0x61, al",\
	modify exact [eax]\

#pragma aux gettimer42 =\
	"mov al, 0x84",\
	"out 0x43, al",\
	"in al, 0x42",\
	"shl eax, 24",\
	"in al, 0x42",\
	"rol eax, 8",\
	modify [eax]\

#pragma aux printchrasm =\
	"rep stosw"\
	parm [edi][ecx][eax]\

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
void error(char *msg, int code)
{
	uninittimer();
	uninitkeys();
	ExtUnInit();
	uninitengine();
	setvmode(0x3);
	printf("%s\n", msg);
	printf("Error code: $%d\n", code);
	exit(-1);
}

void loadpal(char *name, char pal[768])
{
	long fil;

	fil = loaditem(name);
	readitem(fil, pal, 768);
}

void loadpalettes(void)
{
	loadpal("SN.PAL", palette);
}

void loadlookup(void)
{
	long i, fil;

	numpalookups = 32;
	if ((palookup[0] = (char *)kmalloc(numpalookups<<8)) == NULL)
		allocache(&palookup[0],numpalookups<<8,&plock);

	globalpalwritten = palookup[0]; globalpal = 0;
	setpalookupaddress(globalpalwritten);

	fil = loaditem("LOOKUP.DAT");
	if (itemlength("LOOKUP.DAT") != LOOKUPSIZ)
		error("Incorrect lookup size", 212);
	readitem(fil,palookup[globalpal],numpalookups<<8);
	printf("Creating color lookups\n");
	for(i=0;i<NUMPALOOKUPS;i++)
	{
		readitem(fil,&tempbuf[0],256);
		makepalookup(i+1,tempbuf,0,0,0,1);
	}
}

void loadtrans(void)
{
	long fil;

	printf("Loading translucency table\n");
	if ((transluc = (char *)kmalloc(65536L)) == NULL)
		allocache(&transluc,65536,&plock);

	fixtransluscence(FP_OFF(transluc));

	fil = loaditem("TRANS.TBL");
	readitem(fil,transluc,65536);
}

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

void ExtInit(void)
{
	long i, j, fil;

	setvmode(0x3);
	printstr(0,0,"                                                                                ",0x2F);
        printstr(40-(strlen(PROG_ID)>>1),0,PROG_ID,0x2F);
	printstr(0,24,"                           Copyright 2007-2013 Michael Muniko                   ",0x2F);
	printf("\n");
	printf("Initializing resources archive\n");
	finddats();
	loaddat(datfilename);
	if (loadconfig() != 0)
	{
		printf("Error opening configuration. Please run SETUP.\n");
		exit(-1);
	}
	printf("Initializing mouse\n");
	initmouse();
	printf("Initializing 3D engine\n");
	initengine();
	printf("Setting to graphics mode\n");
	vidoption = grafmode; xdim = screenwidth; ydim = screenheight;

	kensplayerheight = 40;
	zmode = 0;
	defaultspritecstat = 0;
	pskyoff[0] = 0; pskyoff[1] = 0; pskybits = 1;
	parallaxtype = 1;
}

void ExtUnInit(void)
{
	uninittimer42();
}

static long daviewingrange, daaspect, horizval1, horizval2;
void ExtPreCheckKeys(void)
{
	long cosang, sinang, dx, dy, mindx, i;

	if (keystatus[0x3e])  //F4 - screen re-size
	{
		keystatus[0x3e] = 0;

			//cycle through all vesa modes, then screen-buffer mode
		getvalidvesamodes();
		if (vidoption == 1)
		{
			for(i=0;i<validmodecnt;i++)
				if ((validmodexdim[i] == xdim) && (validmodeydim[i] == ydim))
				{
					if (i == validmodecnt-1)
						setgamemode(2,320L,200L);
					else
						setgamemode(1,validmodexdim[i+1],validmodeydim[i+1]);
					break;
				}
		}
		else if (validmodecnt > 0)
			setgamemode(1,validmodexdim[0],validmodeydim[0]);

		inittimer42();  //Must init here because VESA 0x4F02 messes timer 2
	}
}

int weaponselection(void)
{
	spriteselected = gettile(0, 1);
}

void ammoselection(void)
{
	spriteselected = gettile(0, 0);
}

void enemyselection(void)
{
	spriteselected = gettile(0, 2);
}


void miscselection(void)
{
	spriteselected = gettile(0, 5);
}

void keyselection(void)
{
	spriteselected = gettile(0, 4);
}

void itemselection(void)
{
	spriteselected = gettile(0, 3);
}

void spritetypeselection(void)
{
	while (keystatus[1] == 0)
	{
		drawrooms(posx,posy,posz,ang,horiz,cursectnum);
		ExtAnalyzeSprites();
		drawmasks();
		printext256(80, 64, whitecol, 0, "1. Ammo", 0);
		printext256(80, 72, whitecol, 0, "2. Weapons", 0);
		printext256(80, 80, whitecol, 0, "3. Enemies", 0);
		printext256(80, 88, whitecol, 0, "4. Items", 0);
		printext256(80, 96, whitecol, 0, "5. Keys", 0);
		printext256(80, 104, whitecol, 0, "6. Miscellenous", 0);
		printext256(80, 120, whitecol, 0, "Press choice (1-6)", 0);
		nextpage();
		if (keystatus[2] > 0)
		{
			keystatus[2] = 0;
			ammoselection();
			return;
		}
		if (keystatus[3] > 0)
		{
			keystatus[3] = 0;
			weaponselection();
			return;
		}
		if (keystatus[4] > 0)
		{
			keystatus[4] = 0;
			enemyselection();
			return;
		}
		if (keystatus[5] > 0)
		{
			keystatus[5] = 0;
			itemselection();
			return;
		}
		if (keystatus[6] > 0)
		{
			keystatus[6] = 0;
			keyselection();
			return;
		}
		if (keystatus[7] > 0)
		{
			keystatus[7] = 0;
			miscselection();
			return;
		}
	}
	keystatus[1] = 0;
	spriteselected = 0;
	return;
}

int selectsprite(void)
{
	spritetypeselection();
	return(spriteselected);
}

#define MAXVOXMIPS 5
extern char *voxoff[][MAXVOXMIPS];
void ExtAnalyzeSprites(void)
{
	long i, k, *longptr;
	spritetype *tspr;

	for(i=0,tspr=&tsprite[0];i<spritesortcnt;i++,tspr++)
	{
		switch(tspr->picnum)
		{
			case Boy:
			case Fat:
			case Cat:
			case Dog:
			case Kali:
			case HugeCat:
			case FlyBaby:
			case FlyingFish:
			case Chui:
			case Clement:
			case Kinene:
			case GirlStand:
			case LITFGuy:
			case Plopper:
			case FatumaStand:
			case KnightStand:
			case Lil:
			case Ogre:
			k = getangle(tspr->x-posx,tspr->y-posy);
			k = (((tspr->ang+3072+128-k)&2047)>>8)&7;
			if (k > 4)
			{
				k = 8-k;
				tspr->cstat &= ~4;
			}
			else {
				tspr->cstat |= 4;
			}
			tspr->picnum += k;
			break;
		}

		tspr->shade += 6;
		if (sector[tspr->sectnum].ceilingstat&1)
			tspr->shade += sector[tspr->sectnum].ceilingshade;
		else
			tspr->shade += sector[tspr->sectnum].floorshade;
	}
}

static char timerinited = 0;
void ExtCheckKeys(void)
{
	long i, j, p, y, dx, dy, cosang, sinang, bufplc, tsizy, tsizyup15;
	long dax, day, hitx, hity, hitz;
	short hitsect, hitwall, hitsprite;

	if (qsetmode == 200)    //In 3D mode
	{
		if (keystatus[0xa]) setaspect(viewingrange+(viewingrange>>8),yxaspect+(yxaspect>>8));
		if (keystatus[0xb]) setaspect(viewingrange-(viewingrange>>8),yxaspect-(yxaspect>>8));
		if (keystatus[0xc]) setaspect(viewingrange,yxaspect-(yxaspect>>8));
		if (keystatus[0xd]) setaspect(viewingrange,yxaspect+(yxaspect>>8));

		if (!timerinited)
		{
			timerinited = 1;
			inittimer42();  //Must init here because VESA 0x4F02 messes timer 2
		}
		i = totalclock-ototalclock; ototalclock += i;
		j = ofinetotalclock-gettimer42(); ofinetotalclock -= j;
		i = ((i*(1193181/120)-(j&65535)+32768)&0xffff0000)+(j&65535);
		if (i) { frameval[framecnt&(AVERAGEFRAMES-1)] = 11931810/i; framecnt++; }
			//Print MAX FRAME RATE
		i = frameval[(framecnt-1)&(AVERAGEFRAMES-1)];
		for(j=AVERAGEFRAMES-1;j>0;j--) i = max(i,frameval[j]);
		averagefps = ((averagefps*3+i)>>2);
		sprintf(tempbuf,"%ld.%ld",averagefps/10,averagefps%10);
		printext256(0L,0L,31,-1,tempbuf,1);
		if (keystatus[0x4c]) horiz = 100;
		if (keystatus[56] > 0)
		{
			if (keystatus[31] > 0)
			{
				keystatus[31] = 0;
				selectsprite();
				if (spriteselected)
				{
					dax = 16384;
					day = divscale14(searchx-(xdim>>1),xdim>>1);
					rotatepoint(0,0,dax,day,ang,&dax,&day);

					hitscan(posx,posy,posz,cursectnum,               //Start position
						dax,day,(scale(searchy,200,ydim)-horiz)*2000, //vector of 3D ang
						&hitsect,&hitwall,&hitsprite,&hitx,&hity,&hitz,CLIPMASK1);

					if (hitsect >= 0)
					{
						dax = hitx;
						day = hity;
						if ((gridlock > 0) && (grid > 0))
						{
							if ((searchstat == 0) || (searchstat == 4))
							{
								hitz = (hitz&0xfffffc00);
							}
						}
						else
						{
							dax = ((dax+(1024>>grid))&(0xffffffff<<(11-grid)));
							day = ((day+(1024>>grid))&(0xffffffff<<(11-grid)));
						}
					}

					i = insertsprite(hitsect,selectedstatus);
					sprite[i].x = dax, sprite[i].y = day;
					sprite[i].cstat = selectedcstat;
					sprite[i].shade = 0;
					sprite[i].pal = 0;
					sprite[i].xrepeat = selectedxrepeat, sprite[i].yrepeat = selectedyrepeat;
					sprite[i].xoffset = 0, sprite[i].yoffset = 0;
					sprite[i].ang = 1536;
					sprite[i].xvel = 0; sprite[i].yvel = 0; sprite[i].zvel = 0;
					sprite[i].owner = -1;
					sprite[i].clipdist = 32;
					sprite[i].lotag = selectedlotag;
					sprite[i].hitag = 0;
					sprite[i].extra = 1+2+4;
					sprite[i].picnum = selectedpicnum;
					if ((sprite[i].cstat&128) == 0)
						sprite[i].z = min(max(hitz,getceilzofslope(hitsect,hitx,hity)+(j<<1)),getflorzofslope(hitsect,hitx,hity));
					else
						sprite[i].z = min(max(hitz,getceilzofslope(hitsect,hitx,hity)+j),getflorzofslope(hitsect,hitx,hity)-j);
					updatenumsprites();
					asksave = 1;
				}
			}
		}
		editinput();
	}
	else
	{
		timerinited = 0;
	}
}

void ExtCleanUp(void)
{
}

void fixmap(void)
{
	long i;

	for(i=0;i<numwalls;i++)
	{
		if (wall[i].extra < 0) wall[i].extra = 0;
	}
	for(i=0;i<MAXSPRITES;i++)
	{
		if (sprite[i].extra < 0)
		{
			sprite[i].extra = 1+2+4;
		}
		switch(sprite[i].picnum)
		{
			case Boy:
			sprite[i].lotag = 203;
			sprite[i].cstat = 1+256;
			sprite[i].xrepeat = 48;
			sprite[i].yrepeat = 48;
			changespritestat(i, 2);
			break;
			case Fat:
			sprite[i].lotag = 204;
			sprite[i].cstat = 1+256;
			changespritestat(i, 2);
			break;
			case Cat:
			sprite[i].lotag = 200;
			sprite[i].cstat = 1+256;
			changespritestat(i, 2);
			break;
			case Dog:	
			sprite[i].lotag = 201;
			sprite[i].cstat |= 1+256;
			changespritestat(i, 2);
			break;
			case Kali:
			sprite[i].lotag = 202;
			sprite[i].cstat = 1+256;
			sprite[i].clipdist = 72;
			changespritestat(i, 2);
			break;
			case HugeCat:
			sprite[i].lotag = 205;
			sprite[i].cstat = 1+256;
			sprite[i].clipdist = 64;
			changespritestat(i, 2);
			break;
			case FlyBaby:
			sprite[i].lotag = 206;
			sprite[i].cstat = 1+256;
			changespritestat(i, 2);
			break;
			case FlyingFish:
			sprite[i].lotag = 207;
			sprite[i].cstat = 1+256;
			changespritestat(i, 2);
			break;
			case Chui:
			sprite[i].lotag = 208;
			sprite[i].cstat = 1+256;
			changespritestat(i, 2);
			break;
			case Clement:
			sprite[i].lotag = 209;
			sprite[i].cstat = 1+256;
			changespritestat(i, 2);
			break;
			case Kinene:
			sprite[i].lotag = 210;
			sprite[i].cstat = 1+256;
			changespritestat(i, 2);
			break;
			case GirlStand:
			sprite[i].lotag = 215;
			sprite[i].cstat = 1+256;
			sprite[i].xrepeat = 48;
			sprite[i].yrepeat = 48;
			changespritestat(i, 2);
			break;
			case Plopper:
			sprite[i].lotag = 216;
			sprite[i].cstat = 1+256;
			changespritestat(i, 2);
			break;
			case LITFGuy:
			sprite[i].lotag = 3004;
			sprite[i].cstat = 1+256;
			changespritestat(i, 2);
			break;
			case FatumaStand:
			sprite[i].lotag = 220;
			sprite[i].cstat = 1+256;
			sprite[i].xrepeat = 48;
			sprite[i].yrepeat = 48;
			changespritestat(i, 2);
			break;
			case KnightStand:
			sprite[i].lotag = 320;
			sprite[i].xrepeat = 48;
			sprite[i].yrepeat = 48;
			sprite[i].cstat = 1+256;
			sprite[i].clipdist = 72;
			changespritestat(i, 2);
			break;
			case Lil:
			sprite[i].lotag = 240;
			sprite[i].xrepeat = 48;
			sprite[i].yrepeat = 48;
			sprite[i].cstat = 1+256;
			changespritestat(i, 2);
			break;
			case Cargo:
			if (sprite[i].lotag != 88)
			{
				sprite[i].lotag = 195;
				sprite[i].cstat = 1+256;
				changespritestat(i, 0);
			}
			break;
			case Ogre:
			sprite[i].lotag = 3002;
			sprite[i].cstat = 1+256;
			sprite[i].xrepeat = 56;
			sprite[i].yrepeat = 56;
			changespritestat(i, 2);
			break;
			case Crystal:
			case CBox:
			case Egg:
			case EggBox:
			case Battery:
			case BattBox:
			case WandSpr:
			case SDbSpr:
			case RapidSpr:
			case HgnSpr:
			case Stone:
			case Cereal:
			case FirstAid:
			case Bread:
			case Maize:
			case Meat:
			case Cupcake:
			case Glass:
			case Key1:
			case Key2:
			case Key3:
			case StoneBox:
			case BoomPack:
			case Milk:
			case Armor1:
			case Armor2:
			case Repellent:
			case Invuln1:
			case InvSpr:
			case TorchSpr:
			case FlamSprite:
			case CannonSpr:
			case KerzerkSpr:
			case Tyre:
			sprite[i].cstat = 0;
			sprite[i].xrepeat = 64;
			sprite[i].yrepeat = 64;
			changespritestat((short)i,3);
			break;
			case MilkCarton:
			sprite[i].lotag = 2018;
			sprite[i].xrepeat = 64;
			sprite[i].yrepeat = 64;
			break;
			case 1220:
			sprite[i].lotag = 2019;
			sprite[i].xrepeat = 64;
			sprite[i].yrepeat = 64;
			break;
		}
	}
}

void ExtLoadMap(const char *mapname)
{
	fixmap();
}

void ExtSaveMap(const char *mapname)
{
}

const char *ExtGetSectorCaption(short sectnum)
{
	if ((sector[sectnum].lotag|sector[sectnum].hitag) == 0)
	{
		tempbuf[0] = 0;
	}
	else
	{
		sprintf(tempbuf,"%hu,%hu",(unsigned short)sector[sectnum].hitag,
			(unsigned short)sector[sectnum].lotag);
	}
	return(tempbuf);
}

const char *ExtGetWallCaption(short wallnum)
{
	if ((wall[wallnum].lotag|wall[wallnum].hitag) == 0)
	{
		tempbuf[0] = 0;
	}
	else
	{
		sprintf(tempbuf,"%hu,%hu",(unsigned short)wall[wallnum].hitag,
					(unsigned short)wall[wallnum].lotag);
	}
	return(tempbuf);
}

const char *ExtGetSpriteCaption(short spritenum)
{
	if ((sprite[spritenum].lotag|sprite[spritenum].hitag) == 0)
	{
		tempbuf[0] = 0;
	}
	else
	{
		switch(sprite[spritenum].lotag)
		{
			case 195:
			strcpy(tempbuf, "Cargo");
			break;
			case 200:
			strcpy(tempbuf, "Spider Cat");
			break;
			case 201:
			strcpy(tempbuf, "Dog");
			break;
			case 202:
			strcpy(tempbuf, "Kali");
			break;
			case 203:
			strcpy(tempbuf, "Former Scout Boy");
			break;
			case 204:
			strcpy(tempbuf, "Former Scout Leader");
			break;
			case 205:
			strcpy(tempbuf, "Huge Cat");
			break;
			case 206:
			strcpy(tempbuf, "Flying Baby");
			break;
			case 207:
			strcpy(tempbuf, "Flying Fish");
			break;
			case 208:
			strcpy(tempbuf, "Big Chui");
			break;
			case 209:
			strcpy(tempbuf, "Clement");
			break;
			case 210:
			strcpy(tempbuf, "Kinene");
			break;
			case 215:
			strcpy(tempbuf, "Girl");
			break;
			case 216:
			strcpy(tempbuf, "Plopper Cat");
			break;
			case 240:
			strcpy(tempbuf, "Bancouver");
			break;
			case 300:
			strcpy(tempbuf, "Zombie Gripper");
			break;
			case 3004:
			strcpy(tempbuf, "LITF guy");
			break;
			case 3002:
			strcpy(tempbuf, "Offian");
			break;
			case 220:
			strcpy(tempbuf, "Fatuma");
			break;
			case 320:
			strcpy(tempbuf, "Evil Knight");
			break;
			default:
			sprintf(tempbuf,"%hu,%hu",(unsigned short)sprite[spritenum].hitag,
									  (unsigned short)sprite[spritenum].lotag);
			break;
		}
	}
	return(tempbuf);
}

//printext16 parameters:
//printext16(long xpos, long ypos, short col, short backcol,
//           char name[82], char fontsize)
//  xpos 0-639   (top left)
//  ypos 0-479   (top left)
//  col 0-15
//  backcol 0-15, -1 is transparent background
//  name
//  fontsize 0=8*8, 1=3*5

//drawline16 parameters:
// drawline16(long x1, long y1, long x2, long y2, char col)
//  x1, x2  0-639
//  y1, y2  0-143  (status bar is 144 high, origin is top-left of STATUS BAR)
//  col     0-15

void ExtShowSectorData(short sectnum)   //F5
{
	long i;

	numenemiess12 = 0;
	numenemiess3 = 0;
	numenemiess45 = 0;
	numsecrets = 0;
	
	if (qsetmode == 200)    //In 3D mode
	{
	}
	else
	{
		clearmidstatbar16();             //Clear middle of status bar

		for(i=headspritestat[2];i>=0;i=nextspritestat[i])
		{
			if (sprite[i].extra&1) numenemiess12++;
			if (sprite[i].extra&2) numenemiess3++;
			if (sprite[i].extra&4) numenemiess45++;
		}
		for(i=0;i<numsectors;i++)
		{
			if (sector[i].lotag == 40) numsecrets++;
		}
		sprintf(tempbuf, "Enemies (skill 1-2): %d", numenemiess12);
		printext16(8,32,11,-1,tempbuf,0);	
		sprintf(tempbuf, "Enemies (skill 3): %d", numenemiess3);
		printext16(8,40,11,-1,tempbuf,0);
		sprintf(tempbuf, "Enemies (skill 4-5): %d", numenemiess45);
		printext16(8,48,11,-1,tempbuf,0);
		sprintf(tempbuf, "Secrets: %d", numsecrets);
		printext16(8,56,11,-1,tempbuf,0);
	}
}

void ExtShowWallData(short wallnum)       //F6
{
	if (qsetmode == 200)    //In 3D mode
	{
	}
	else
	{
		clearmidstatbar16();             //Clear middle of status bar
		sprintf(tempbuf,"Wall %d",wallnum);
		printext16(8,32,11,-1,tempbuf,0);
		if (wall[wallnum].extra&1)
			printext16(8,48,11,-1,"1. [x] Show red in map",0);
		else
			printext16(8,48,11,-1,"1. [ ] Show red in map",0);			
		if (wall[wallnum].extra&2)
			printext16(8,56,11,-1,"2. [x] Block monsters",0);
		else
			printext16(8,56,11,-1,"2. [ ] Block monsters",0);
	}
}

void ExtShowSpriteData(short spritenum)   //F6
{
	if (qsetmode == 200)    //In 3D mode
	{
	}
	else
	{
		clearmidstatbar16();             //Clear middle of status bar

		sprintf(tempbuf,"Sprite %d",spritenum);
		printext16(8,32,11,-1,tempbuf,0);
		if (sprite[spritenum].extra&1)
			printext16(8,48,11,-1,"1. [x] Skill 1-2",0);
		else
			printext16(8,48,11,-1,"1. [ ] Skill 1-2",0);

		if (sprite[spritenum].extra&2)
			printext16(8,56,11,-1,"2. [x] Skill 3",0);
		else
			printext16(8,56,11,-1,"2. [ ] Skill 3",0);

		if (sprite[spritenum].extra&4)
			printext16(8,64,11,-1,"3. [x] Skill 4-5",0);
		else
			printext16(8,64,11,-1,"3. [ ] Skill 4-5",0);
	}
}

void ExtEditSectorData(short sectnum)    //F7
{
	short nickdata;

	if (qsetmode == 200)    //In 3D mode
	{
	}
	else                    //In 2D mode
	{
		sprintf(tempbuf,"Sector (%ld) Nick's variable: ",sectnum);
		nickdata = 0;
		nickdata = getnumber16(tempbuf,nickdata,65536L);

		printmessage16("");              //Clear message box (top right of status bar)
		ExtShowSectorData(sectnum);
	}
}

void ExtEditWallData(short wallnum)       //F8
{
	short nickdata;

	if (qsetmode == 200)    //In 3D mode
	{
	}
	else
	{
		ExtShowWallData(wallnum);
		while (keystatus[1] == 0)
		{
			if (keystatus[2] > 0)
			{
				keystatus[2] = 0;
				wall[wallnum].extra ^= 1;
				ExtShowWallData(wallnum);
			}
			if (keystatus[3] > 0)
			{
				keystatus[3] = 0;
				wall[wallnum].extra ^= 2;
				ExtShowWallData(wallnum);
			}
		}
		keystatus[1] = 0;
	}
}

void ExtEditSpriteData(short spritenum)   //F8
{
	short nickdata;

	if (qsetmode == 200)    //In 3D mode
	{
	}
	else
	{
		ExtShowSpriteData(spritenum);
		while (keystatus[1] == 0)
		{
			if (keystatus[2] > 0)
			{
				keystatus[2] = 0;
				sprite[spritenum].extra ^= 1;
				ExtShowSpriteData(spritenum);
			}

			if (keystatus[3] > 0)
			{
				keystatus[3] = 0;
				sprite[spritenum].extra ^= 2;
				ExtShowSpriteData(spritenum);
			}
			if (keystatus[4] > 0)
			{
				keystatus[4] = 0;
				sprite[spritenum].extra ^= 4;
				ExtShowSpriteData(spritenum);
			}
		}
		keystatus[1] = 0;
	}
}

void faketimerhandler(void)
{
}

	//Just thought you might want my getnumber16 code
/*
getnumber16(char namestart[80], short num, long maxnumber)
{
	char buffer[80];
	long j, k, n, danum, oldnum;

	danum = (long)num;
	oldnum = danum;
	while ((keystatus[0x1c] != 2) && (keystatus[0x1] == 0))  //Enter, ESC
	{
		sprintf(&buffer,"%s%ld_ ",namestart,danum);
		printmessage16(buffer);

		for(j=2;j<=11;j++)                //Scan numbers 0-9
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
		if (keystatus[0x1c] == 1)   //L. enter
		{
			oldnum = danum;
			keystatus[0x1c] = 2;
			asksave = 1;
		}
	}
	keystatus[0x1c] = 0;
	keystatus[0x1] = 0;
	return((short)oldnum);
}
*/
