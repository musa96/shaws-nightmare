#include <fcntl.h>
#include <stdlib.h>
#include <dos.h>
#include <sys\stat.h>
#include <sys\types.h>
#include "dat.h"
#include "sn.h"
#include "init.h"
#include "map2d.h"
#include "config.h"
#include "kbd.h"
#include "mirrors.h"
#include "weapon.h"
#include "player.h"

int drawfoot;
char lavapal[768], hurt1pal[768], hurt2pal[768], hurt3pal[768];
char bonus1pal[768], bonus2pal[768], bonus3pal[768], invulnpal[768];

long screentilt = 0;
char detailmode = 0;
int screensize, mapmode;
int zoom, brightness;
int borderdraw;
static char plock = 255;

void initgraph(void)
{
	long i;

	i = setgamemode(grafmode,screenwidth,screenheight);
	if (i < 0)
	{
		printf("VESA driver for (%ld * %ld) not found/supported.\n",xdim,ydim);
		printf("   Press ENTER to play in NORMAL mode instead\n");
		printf("   Press ESC to quit to DOS\n");
		keystatus[1] = keystatus[0x1c] = keystatus[0x9c] = 0;
		while (!keystatus[1])
			if (keystatus[0x1c]|keystatus[0x9c])
				{ vidoption = 2; i = setgamemode(2, 320, 200); break; }
	}
	if (i < 0)
	{
		uninit();
		exit(0);
	}
	setviewsize();
}

void putnumber(long x, long y, int num)
{
	short i, j, k, numpic, c;
    	char b[10];

    	ltoa(num,b,10);
    	i = strlen(b);
    	j = 0;

    	for(k=0;k<i;k++)
    	{
        	numpic = BigNumber+*(b+k)-'0';
        	j += tilesizx[numpic]+1;
    	}
    	c = x-(j>>1);

    	j = 0;
    	for(k=0;k<i;k++)
    	{
        	numpic = BigNumber+*(b+k)-'0';
        	rotatesprite((c+j)<<16,y<<16,65536L,0,numpic,0,0,10+16+128,0,0,xdim-1,ydim-1);
        	j += tilesizx[numpic]+1;
    	}
}

void putsmallnumber(long x, long y, int num)
{
	short i, j, k, numpic, c;
    	char b[10];

    	ltoa(num,b,10);
    	i = strlen(b);
    	j = 0;

    	for(k=0;k<i;k++)
    	{
        	numpic = SmallNumber+*(b+k)-'0';
        	j += tilesizx[numpic]+1;
    	}
    	c = x-(j>>1);

    	j = 0;
    	for(k=0;k<i;k++)
    	{
        	numpic = SmallNumber+*(b+k)-'0';
        	rotatesprite((c+j)<<16,y<<16,65536L,0,numpic,0,0,10+16+128,0,0,xdim-1,ydim-1);
        	j += tilesizx[numpic]+1;
    	}
}

void putsmallnumberpal(long x, long y, int num, char pal)
{
	short i, j, k, numpic, c;
    	char b[10];

    	ltoa(num,b,10);
    	i = strlen(b);
    	j = 0;

    	for(k=0;k<i;k++)
    	{
        	numpic = SmallNumber+*(b+k)-'0';
        	j += tilesizx[numpic]+1;
    	}
    	c = x-(j>>1);

    	j = 0;
    	for(k=0;k<i;k++)
    	{
        	numpic = SmallNumber+*(b+k)-'0';
        	rotatesprite((c+j)<<16,y<<16,65536L,0,numpic,0,pal,10+16+128,0,0,xdim-1,ydim-1);
        	j += tilesizx[numpic]+1;
    	}
}

void puttext(long x, long y, char *str)
{
	long i;
	short fontpic, centre;

	while(*str)
	{
		if(*str == ' ') { x += 5; str++; continue; }
        	fontpic = 0;
        	if(*str >= '0' && *str <= '9')
			fontpic = *str - '0' + NumStart;
        	else if(*str >= 'a' && *str <= 'z')
            		fontpic = toupper(*str) - 'A' + SmallStart;
        	else if(*str >= 'A' && *str <= 'Z')
            		fontpic = *str - 'A' + FontStart;
		else switch (*str)
		{
			case '!':
			fontpic = FontEx;
			break;
			case '.':
			fontpic = FontDot;
			break;
			case '-':
			fontpic = FontMin;
			break;
			case ':':
			fontpic = FontColon;
			break;
			case '[':
			fontpic = FontLB;
			break;
			case ']':
			fontpic = FontRB;
			break;
			case '/':
			fontpic = FontSlash;
			break;
			case '<':
			fontpic = 627;
			break;
			case '>':
			fontpic = 628;
			break;
			case '?':
			fontpic = FontQ;
			break;
			default:
			x += 5;
			str++;
			continue;
		}
		rotatesprite(x<<16,y<<16,65536L,0,fontpic,0,0,10+16,0,0,xdim-1,ydim-1);

		x += tilesizx[fontpic]+1;
        	str++;
	}
}

void puttextpal(long x, long y, char *str, char pal)
{
	long i;
	short fontpic, centre;

	while(*str)
	{
		if(*str == ' ') { x += 5; str++; continue; }
        	fontpic = 0;
        	if(*str >= '0' && *str <= '9')
			fontpic = *str - '0' + NumStart;
        	else if(*str >= 'a' && *str <= 'z')
            		fontpic = toupper(*str) - 'A' + SmallStart;
        	else if(*str >= 'A' && *str <= 'Z')
            		fontpic = *str - 'A' + FontStart;
		else switch (*str)
		{
			case '!':
			fontpic = FontEx;
			break;
			case '.':
			fontpic = FontDot;
			break;
			case '-':
			fontpic = FontMin;
			break;
			case ':':
			fontpic = FontColon;
			break;
			case '[':
			fontpic = FontLB;
			break;
			case ']':
			fontpic = FontRB;
			break;
			case '/':
			fontpic = FontSlash;
			break;
			case '<':
			fontpic = 627;
			break;
			case '>':
			fontpic = 628;
			break;
			default:
			x += 5;
			str++;
			continue;
		}
		rotatesprite(x<<16,y<<16,65536L,0,fontpic,0,pal,10+16,0,0,xdim-1,ydim-1);

		x += tilesizx[fontpic]+1;
        	str++;
	}
}

void gametext(long x, long y, char *str)
{
	long i;
	short fontpic, centre;

	while(*str)
	{
		if(*str == ' ') { x += 5; str++; continue; }
        	fontpic = 0;
        	if(*str >= '0' && *str <= '9')
			fontpic = *str - '0' + GNUM;
        	else if(*str >= 'a' && *str <= 'z')
            		fontpic = toupper(*str) - 'A' + GFONT;
        	else if(*str >= 'A' && *str <= 'Z')
            		fontpic = *str - 'A' + GFONT;
		else switch (*str)
		{
			case '.':
			fontpic = GPERIOD;
			break;
			case ':':
			fontpic = GCOLON;
			break;
			case '!':
			fontpic = GEXCLAIM;
			break;
			case '-':
			fontpic = GMINUS;
			break;
			case '_':
			fontpic = GENTER;
			break;
			case '\'':
			fontpic = GASTRP;
			break;
			default:
			x += 5;
			str++;
			continue;
		}
		rotatesprite(x<<16,y<<16,65536L,0,fontpic,0,0,10+16,0,0,xdim-1,ydim-1);

		x += tilesizx[fontpic]+1;
        	str++;
	}
}

void loadpal(char *name, char pal[768])
{
	long fil;

	fil = loaditem(name);
	readitem(fil, pal, 768);
}

void loadpalettes(void)
{
	printf("Loading palettes\n");
	loadpal("SN.PAL", palette);
	loadpal("SUIT.PAL", lavapal);
	loadpal("HURT1.PAL", hurt1pal);
	loadpal("HURT2.PAL", hurt2pal);
	loadpal("HURT3.PAL", hurt3pal);
	loadpal("BONUS1.PAL", bonus1pal);
	loadpal("BONUS2.PAL", bonus2pal);
	loadpal("BONUS3.PAL", bonus3pal);
	loadpal("INVULN1.PAL", invulnpal);
}

void loadlookup(void)
{
	long i, fil;
	char tempbuf[256];

	numpalookups = 32;
	if ((palookup[0] = (char *)kmalloc(numpalookups<<8)) == NULL)
		allocache((char *)&palookup[0],numpalookups<<8,&plock);

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
		allocache((char *)&transluc,65536,&plock);

	fixtransluscence(FP_OFF(transluc));

	fil = loaditem("TRANS.TBL");
	readitem(fil,transluc,65536);
}

void loadvoxels(void)
{
	long i, fil;

	memset(voxelinfo, -1, sizeof(VoxelInfo_T)*MAXTILES);
	if ((fil = open("VOXELS.DAT", O_BINARY|O_RDONLY,S_IREAD)) != -1)
	{
		read(fil,&voxelinfo[0],filelength(fil));
		close(fil);
	}
	for(i=0;i<MAXTILES;i++)
		if (voxelinfo[i].picnum != -1)
			qloadkvx(i, voxelinfo[i].name);
}

static spritetype *addtsprite(short sectnum, short status, spritetype *origin)
{
	short tsprnum = -1;
	spritetype *tspr;

	tsprnum = (short)spritesortcnt;
	tspr = &tsprite[tsprnum];
	clearbufbyte(tspr,sizeof(spritetype),0L);
	tspr->sectnum = sectnum;
	tspr->statnum = status;
	tspr->xrepeat = 64;
	tspr->yrepeat = 64;
	tspr->cstat = 0;
	tspr->extra = 0;
	tspr->owner = -1;
	spritesortcnt++;

	if (origin)
	{
		tspr->x = origin->x;
		tspr->y = origin->y;
		tspr->z = origin->z;
		tspr->owner = origin->owner;
		tspr->ang = origin->ang;
	}

	return(&tsprite[tsprnum]);
}

void analyzesprites(long dax, long day)
{
	long i, k, *longptr;
	spritetype *tspr, *newtspr;

	for(i=0,tspr=&tsprite[0];i<spritesortcnt;i++,tspr++)
	{
		switch(tspr->picnum)
		{
			case Cat:
			case CatWalk1:
			case CatWalk2:
			case CatFire:
			case Dog:
			case DogWalk1:
			case DogWalk2:
			case DogWalk3:
			case DogWalk4:
			case DogBite:
			case Kali:
			case KaliWalk1:
			case KaliWalk2:
			case Player:
			case PlayWalk1:
			case PlayWalk2:
			case PlayDuck:
			case PlaySpace:
			case Boy:
			case BoyWalk1:
			case BoyWalk2:
			case BoyFire1:
			case BoyFire2:
			case BoyHurt:
			case Fat:
			case FatWalk1:
			case FatWalk2:
			case FatFire1:
			case FatFire2:
			case FatHurt:
			case HugeCat:
			case HugeWalk1:
			case HugeWalk2:
			case HugeFire1:
			case HugeFire2:
			case FlyBaby:
			case BabyAtk:
			case BabyHurt:
			case HenGrenade:
			case ClFireball:
			case FlyingFish:
			case FlyingAtk:
			case Kinene:
			case KineneWalk1:
			case KineneWalk2:
			case KineneKick:
			case Chui:
			case ChuiWalk1:
			case ChuiWalk2:
			case ChuiFire:
			case Clement:
			case ClementWalk1:
			case ClementWalk2:
			case ClementFire1:
			case ClementFire2:
			case GirlStand:
			case GirlWalk1:
			case GirlWalk2:
			case GirlFire1:
			case GirlFire2:
			case GirlHurt:
			case Plopper:
			case PlopperFire:
			case LITFGuy:
			case LITFWalk1:
			case LITFWalk2:
			case LITFAtk1:
			case LITFAtk2:
			case LITFHurt:
			case FatumaStand:
			case FatumaWalk1:
			case FatumaWalk2:
			case FatumaFire1:
			case FatumaFire2:
			case KnightStand:
			case KnightWalk1:
			case KnightWalk2:
			case Ogre:
			case OgreWalk1:
			case OgreWalk2:
			case OgrePunch1:
			case OgrePunch2:
			case OgreAtk2:
			case OgreAtk3:
			case Lil:
			case LilWalk1:
			case LilWalk2:
			case LilFire:
			case Zombie:
			case ZombieWalk1:
			case ZombieWalk2:
			case ZombieGrip1:
			case ZombieGrip2:
			k = getangle(tspr->x-dax,tspr->y-day);
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
			case SmallTorch:
			case BigTorch:
			newtspr = addtsprite(tspr->sectnum, 0, tspr);
			newtspr->z -= (tilesizy[tspr->picnum]<<8);
			newtspr->picnum = Flame;
			newtspr->shade = -32;
			break;
		}
		if ((tspr->picnum == KineneKick) &&
				((klabs(dax-tspr->x)+klabs(day-tspr->y)) < 512)
				&& (tspr->xrepeat == 1 && tspr->yrepeat == 1))
			drawfoot = 1;
		else
			drawfoot = 0;
		for(k=0;k<MAXTILES;k++)
		{
			if (tspr->picnum == voxelinfo[k].picnum)
			{
				if (detailmode == 0)
				{
					tspr->cstat |= 48;
					tspr->picnum = k;
					if (voxelinfo[k].spin)
						tspr->ang = totalclock * 8;
				}
				else
				{
					tspr->cstat &= ~48;
					tspr->picnum = voxelinfo[k].picnum;
				}
			}
		}
		tspr->shade += 6;
		if (sector[tspr->sectnum].ceilingstat&1)
			tspr->shade += sector[tspr->sectnum].ceilingshade;
		else
			tspr->shade += sector[tspr->sectnum].floorshade;
	}
}

void drawslider(long x, long y, int value, int max)
{
	long i;
	
	for(i=0;i<max;i++)
	{
		rotatesprite(x<<16,y<<16,65536L,0,SlidMid,0,0,10+16,0,0,xdim-1,ydim-1);
		if (i == 0)
			rotatesprite(x<<16,y<<16,65536L,0,SlidBeg,0,0,10+16,0,0,xdim-1,ydim-1);
		if (i == max-1)
			rotatesprite(x<<16,y<<16,65536L,0,SlidEnd,0,0,10+16,0,0,xdim-1,ydim-1);
		if (i == value)
		{
			rotatesprite(x<<16,y<<16,65536L,0,Slider,0,0,10+16,0,0,xdim-1,ydim-1);
			x += (tilesizx[Slider] / 2);
		}
		x += tilesizx[SlidMid];	
	}
}

void view (short snum, long *vx, long *vy, long *vz,
		short *vsectnum, short ang, long horiz)
{
	spritetype *sp;
	long i, nx, ny, nz, hx, hy, hz, hitx, hity, hitz;
	short bakcstat, hitsect, hitwall, hitsprite, daang;

	nx = (sintable[(ang+1536)&2047]>>4);
	ny = (sintable[(ang+1024)&2047]>>4);
	nz = (horiz-100)*128;

	sp = &sprite[snum];

	bakcstat = sp->cstat;
	sp->cstat &= (short)~0x101;

	updatesectorz(*vx,*vy,*vz,vsectnum);
	hitscan(*vx,*vy,*vz,*vsectnum,nx,ny,nz,&hitsect,&hitwall,&hitsprite,&hitx,&hity,&hitz,CLIPMASK1);
	hx = hitx-(*vx); hy = hity-(*vy);
	if (klabs(nx)+klabs(ny) > klabs(hx)+klabs(hy))
	{
		*vsectnum = hitsect;
		if (hitwall >= 0)
		{
			daang = getangle(wall[wall[hitwall].point2].x-wall[hitwall].x,
								  wall[wall[hitwall].point2].y-wall[hitwall].y);

			i = nx*sintable[daang]+ny*sintable[(daang+1536)&2047];
			if (klabs(nx) > klabs(ny)) hx -= mulscale28(nx,i);
										 else hy -= mulscale28(ny,i);
		}
		else if (hitsprite < 0)
		{
			if (klabs(nx) > klabs(ny)) hx -= (nx>>5);
										 else hy -= (ny>>5);
		}
		if (klabs(nx) > klabs(ny)) i = divscale16(hx,nx);
									 else i = divscale16(hy,ny);
		if (i < cameradist) cameradist = i;
	}
	*vx = (*vx)+mulscale16(nx,cameradist);
	*vy = (*vy)+mulscale16(ny,cameradist);
	*vz = (*vz)+mulscale16(nz,cameradist);

	updatesectorz(*vx,*vy,*vz,vsectnum);

	sp->cstat = bakcstat;

	cameradist = min(cameradist+((totalclock-cameraclock)<<10),65536);
	cameraclock = totalclock;
}

void updatesectorz(long x, long y, long z, short *sectnum)
{
	walltype *wal;
	long i, j, cz, fz;

	getzsofslope(*sectnum,x,y,&cz,&fz);
	if ((z >= cz) && (z <= fz))
		if (inside(x,y,*sectnum) != 0) return;

	if ((*sectnum >= 0) && (*sectnum < numsectors))
	{
		wal = &wall[sector[*sectnum].wallptr];
		j = sector[*sectnum].wallnum;
		do
		{
			i = wal->nextsector;
			if (i >= 0)
			{
				getzsofslope(i,x,y,&cz,&fz);
				if ((z >= cz) && (z <= fz))
					if (inside(x,y,(short)i) == 1)
						{ *sectnum = i; return; }
			}
			wal++; j--;
		} while (j != 0);
	}

	for(i=numsectors-1;i>=0;i--)
	{
		getzsofslope(i,x,y,&cz,&fz);
		if ((z >= cz) && (z <= fz))
			if (inside(x,y,(short)i) == 1)
				{ *sectnum = i; return; }
	}

	*sectnum = -1;
}

void drawbackground(void)
{
     long x, y, x1, y1, x2, y2, topy;
     short backpic;

     y1 = 0; y2 = ydim;

     if (gamemode == sequel)
     	backpic = Background2;
     else
     	backpic = Background;
     for(y=y1;y<y2;y+=128)
          for(x=0;x<xdim;x+=128)
                rotatesprite(x<<16,y<<16,65536L,0,backpic,0,0,8+16+64+128,0,y1,xdim-1,y2-1);

     if(screensize > 8)
     {
          y = 0;

          x1 = max(windowx1-4,0);
          y1 = max(windowy1-4,y);
          x2 = min(windowx2+4,xdim-1);
          y2 = min(windowy2+4,scale(ydim,200-34,200)-1);

          for(y=y1+4;y<y2-4;y+=64)
          {
                rotatesprite(x1<<16,y<<16,65536L,0,VIEWBORDER,0,0,8+16+64+128,x1,y1,x2,y2);
                rotatesprite((x2+1)<<16,(y+64)<<16,65536L,1024,VIEWBORDER,0,0,8+16+64+128,x1,y1,x2,y2);
          }

          for(x=x1+4;x<x2-4;x+=64)
          {
                rotatesprite((x+64)<<16,y1<<16,65536L,512,VIEWBORDER,0,0,8+16+64+128,x1,y1,x2,y2);
                rotatesprite(x<<16,(y2+1)<<16,65536L,1536,VIEWBORDER,0,0,8+16+64+128,x1,y1,x2,y2);
          }

          rotatesprite(x1<<16,y1<<16,65536L,0,VIEWBORDER+1,0,0,8+16+64+128,x1,y1,x2,y2);
          rotatesprite((x2+1)<<16,y1<<16,65536L,512,VIEWBORDER+1,0,0,8+16+64+128,x1,y1,x2,y2);
          rotatesprite((x2+1)<<16,(y2+1)<<16,65536L,1024,VIEWBORDER+1,0,0,8+16+64+128,x1,y1,x2,y2);
          rotatesprite(x1<<16,(y2+1)<<16,65536L,1536,VIEWBORDER+1,0,0,8+16+64+128,x1,y1,x2,y2);
     }
}

void setviewsize(void)
{
	long i, j, ss, x1, x2, y1, y2;

	if (screensize < 0) screensize = 0;
	if (screensize > 64) screensize = 64;

	ss = max(screensize-8,0);
	x1 = scale(ss,xdim,160);
	x2 = xdim-x1;

	y1 = ss;
	y2 = 200;

	if (screensize >= 8) y2 -= (ss+32);

	y1 = scale(y1,ydim,200);
	y2 = scale(y2,ydim,200);

	setview(x1,y1,x2-1,y2-1);
	borderdraw = 1;
}

void dopalettestuff(int playscreen)
{
	char *pal;
	long i;

	if ((powerup[Invulnerability] > 4*32
		|| powerup[Invulnerability]&8)
		&& !mapmode)
		pal = invulnpal;
	else if (powerup[Kerzerk] > 0 &&
		powerup[Kerzerk] < 120)
	{
		if (powerup[Kerzerk] > 0) pal = hurt3pal;
		if (powerup[Kerzerk] > 60) pal = hurt2pal;
		if (powerup[Kerzerk] > 90) pal = hurt1pal;
	}	 
	else if (damagecnt)
	{
		i = (damagecnt+7)>>3;
		if (i == 1) pal = hurt1pal;
		if (i == 2) pal = hurt2pal;
		if (i >= 3) pal = hurt3pal;
	}
	else if (bonuscnt)
	{	
		i = (bonuscnt+7)>>3;
		if (i == 1) pal = bonus1pal;
		if (i == 2) pal = bonus2pal;
		if (i >= 3) pal = bonus3pal;
	}
	else if (powerup[Lavaproof] > 4*32
		|| powerup[Lavaproof]&8)
		pal = lavapal;
	else
		pal = palette;

	setbrightness(brightness, pal);
}

void renderscreen(void)
{
	long i, j, k, l;
	long cposx, cposy, cposz, choriz;
	short cang, csect;

	dointerpolations();

	cposx = posx;
	cposy = posy;
	cposz = posz;
	choriz = horiz;
	cang = ang;
	csect = cursectnum;
	
	automapping = 1;
	if (borderdraw == 1)
	{
		if (screensize > 8) drawbackground();
		borderdraw = 0;
	}
	if (detailmode == 2)
	{
		walock[MAXTILES-2] = 255;
		if (waloff[MAXTILES-2] == 0)
			allocache(&waloff[MAXTILES-2],320L*320L,&walock[MAXTILES-2]);
		setviewtotile(MAXTILES-2,200L>>1,320L>>1);
		i = 0;
		i = sintable[512]*8 + sintable[i]*5L;
		setaspect(i>>1,yxaspect);
	}

	if (gotpic[38>>3]&(1<<(38&7)))
	{
		gotpic[38>>3] &= ~(1<<(38&7));
		pskyoff[0] = 0; pskyoff[1] = 0; pskyoff[2] = 0; pskyoff[3] = 0;
		pskyoff[4] = 0; pskyoff[5] = 0; pskyoff[6] = 0; pskyoff[7] = 0;
		pskybits = 3;
	}
	else
	{
		pskyoff[0] = 0; pskyoff[1] = 0; pskybits = 1;
	}
	if (cameradist >= 0)
	{
		cang += cameraang;
		view(playersprite,&cposx,&cposy,&cposz,&csect,cang,choriz);
	}
	drawmirrors(cposx, cposy, cposz, choriz, cang);
	drawrooms(cposx,cposy,cposz,cang,choriz,csect);
	analyzesprites(cposx,cposy);
	drawmasks();
	if (cameradist == -1) drawweapon();

	//Finish for screen rotation
	if (detailmode == 2)
	{
		setviewback();
		i = 0;
		i = sintable[512]*8 + sintable[i]*5L;
		if (detailmode == 0) i >>= 1;
		rotatesprite(320<<15,200<<15,i,512,MAXTILES-2,0,0,2+4+64,windowx1,windowy1,windowx2,windowy2);
		walock[MAXTILES-2] = 1;
	}
	if (drawfoot)
		rotatesprite(320<<15,0<<16,65536L,0,Foot,0,0,10+16+128,
			0,0,xdim-1,ydim-1);
	if (mapmode) drawmap();
	if ((screensize >= 8) || (mapmode)) drawstatusbar();
	if (paused)
		puttext(120, 10, "PAUSED");
	if (msgtime)
		gametext(0, 0, message);
	restoreinterpolations();
}
