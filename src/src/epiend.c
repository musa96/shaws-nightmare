#include "sn.h"
#include "menus.h"
#include "sfx.h"
#include "seq.h"
#include "kbd.h"
#include "movers.h"
#include "sound.h"
#include "str.h"

#define TEXTSPEED 5
#define TEXTWAIT 500

extern Seq_T mobseq[SeqEnd];
int textcount, writecount;
char textwritten[512];
short endtext;
short endtile;
int endcnt, endstage;
long cx, cy;

void startend(void)
{
	long i, j;

	gamestate = ENDOFEPISODE;
	switch (gamemode)
	{
		case demomode:
		case fullversion:
		playsong(sng_s30, 1);
		switch (gameepisode)
		{
			case 1:
			endtile = 245;
			endtext = StringEndText1;
			break;
			case 2:
			endtile = 7;
			endtext = StringEndText2;
			break;
			case 3:
			endtile = 384;
			endtext = StringEndText3;
			break;
			case 4:
			endtile = 449;
			endtext = StringEndText4;
			break;
			case 5:
			endtile = 6;
			endtext = StringEndText5;
			break;
		}
		break;
		case sequel:
		playsong(sng_Commend, 1);
		switch (gamelevel)
		{
			case 6:
			endtile = Lava;
			endtext = StringC1Text;
			break;
			case 11:
			endtile = 1242;
			endtext = StringC2Text;
			break;
			case 15:
			endtile = 1324;
			endtext = StringC5Text;
			break;
			case 20:
			endtile = 949;
			endtext = StringC3Text;
			break;
			case 30:
			endtile = 1329;
			endtext = StringC4Text;
			break;
			case 31:
			endtile = 1328;
			endtext = StringC6Text;
			break;
		}
		break;
	}
	endcnt = 0;
	endstage = 0;
	textcount = 0;
	writecount = 0;
	cx = 10;
	cy = 20;
	for(i=0;i<512;i++)
		textwritten[i] = 0;
}

extern int nextlevel;

void updateend(void)
{
	endcnt++;
	if (endstage == 0 && writecount < strlength[endtext])
	{
		textcount++;
		if (textcount > TEXTSPEED)
		{
			textcount = 0;
			textwritten[writecount] = string[endtext][writecount];
			writecount++;
		}
	}
	if (gamemode == sequel && endcnt > 50 && endstage == 0)
	{
		if (keystatus[28] > 0)
		{		
			keystatus[28] = 0;
			if (gamelevel == 30) startcast();
			else
			{
				startlevel(gameepisode, nextlevel, gameskill);
				gamestate = GAME;
			}
		}
	}
	if (endstage == 2)
	{
		updatecast();
		if (!menustatus) castkeypress();
	}

	if (gamemode == sequel)
		return;

	if (endstage == 0 && (endcnt > strlength[endtext]*TEXTSPEED + TEXTWAIT))
	{
		endcnt = 0;
		endstage = 1;
		if (gameepisode == 4)
			playsong(sng_b4, 0);
		if (gameepisode == 5)
		{
			loadcredits();
			gamestate = INTRO;
			menu = 8;
			menustatus = 1;
		}
	}
}

void textwrite(void)
{
	long i, x, y;
	short fontpic;

	for(y=0;y<ydim;y+=tilesizy[endtile])
		for(x=0;x<xdim;x+=tilesizx[endtile])
			rotatesprite(x<<16,y<<16,65536L,0,endtile,0,0,8+16+64+128,windowx1,windowy1,windowx2,windowy2);
	cx = 10;
	cy = 20;
	for(i=0;i<writecount;i++)
	{
		if(textwritten[i] == ' ') { cx += 5; continue; }
        		fontpic = 0;
		if(textwritten[i] == '*') { cx = 10; cy += 10; }
        	if(textwritten[i] >= '0' && textwritten[i] <= '9')
			fontpic = textwritten[i] - '0' + GNUM;
        	else if(textwritten[i] >= 'a' && textwritten[i] <= 'z')
            		fontpic = toupper(textwritten[i]) - 'A' + GFONT;
        	else if(textwritten[i] >= 'A' && textwritten[i] <= 'Z')
            		fontpic = textwritten[i] - 'A' + GFONT;
		else switch (textwritten[i])
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
			case '\'':
			fontpic = GASTRP;
			break;
			default:
			cx += 5;
			continue;
		}
		rotatesprite(cx<<16,cy<<16,65536L,0,fontpic,0,0,10+16,0,0,xdim-1,ydim-1);
		cx += tilesizx[fontpic]+1;
	}
}

void finalscroll(void)
{
	int scrolled;
	int seq;
	static int lastseq;

	scrolled = 320 - (endcnt-90)/2;

	if (scrolled > 320) scrolled = 320;
	if (scrolled < 0) scrolled = 0;

	rotatesprite(0+scrolled<<16,0<<16,65536L,0,PFUB2,0,0,2+8+16+64,windowx1,windowy1,windowx2,windowy2);
	rotatesprite(0+scrolled-320<<16,0<<16,65536L,0,PFUB1,0,0,2+8+16+64,windowx1,windowy1,windowx2,windowy2);

	if (endcnt < 760)
		return;
	if (endcnt < 960)
	{
		rotatesprite(320<<15,200<<15,65536L,0,END,0,0,2+8+16,windowx1,windowy1,windowx2,windowy2);
		lastseq = 0;
		return;
	}		
	seq = (endcnt-960) / 4;
	if (seq > 5)
		seq = 5;
	if (seq > lastseq)
	{
		playsfx(SfxRapidFire, 0, 255);
		lastseq = seq;
	}
	rotatesprite(320<<15,200<<15,65536L,0,END+seq,0,0,2+8+16,windowx1,windowy1,windowx2,windowy2);
}

void drawend(void)
{
	setview(0L,0L,xdim-1,ydim-1);
	if (endstage == 0) textwrite();
	else if (endstage == 1)
	{
		switch (gameepisode)
		{
			case 1:
			rotatesprite(0,0,65536,0,724,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
			break;
			case 2:
			rotatesprite(0,0,65536,0,ENDPIC1,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
			break;
			case 3:
			rotatesprite(0,0,65536,0,ENDPIC2,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
			break;
			case 4:
			finalscroll();
			break;
			case 5:
			clearview(0L);
			break;
		}
	}
	else
		drawcast();
}

// Shaw's Nightmare 2 animation
// Casting by Michael Muniko
//	in no particular order
typedef struct
{
	short name, mobtype;
} castinfo_t;

castinfo_t castinfo[] =
{
	{CCBoy, MobBoy},
	{CCFat, MobFat},
	{CCGirl, MobGirl},
	{CCCat, MobCat},
	{CCDog, MobDog},
	{CCBaby, MobBaby},
	{CCFlyingFish, MobFlyingFish},
	{CCHugeCat, MobHugeCat},
	{CCKnight, MobKnight},
	{CCKali, MobKali},
	{CCFatuma, MobFatuma},
	{CCPlopper, MobPlopper},
	{CCOgre, MobOgre},
	{CCClaratin, MobLilKinene},
	{CCZombieGripper, MobZombieGripper},
	{CCClement, MobClement},
	{CCKinene, MobKinene},
	{CCBigChui, MobChui},
	{CCShaw, MobPlayer}
};
SeqPoint_T castpoint;
Seq_T *castseq;
int castnum, castdeath, castattacking, castonmeele;

void startcast(void)
{
	long i;

	castdeath = 0;
	castnum = 0;
	castattacking = 0;
	castonmeele = 0;
	endstage = 2;
	for(i=0;i<256;i++) keystatus[i] = 0;
	readch = 0;
	oldreadch = 0;
	keytemp = 0;
	keyfifoplc = 0; keyfifoend = 0;
	castseq = &mobseq[mobinfo[castinfo[castnum].mobtype].walkseq];
	clearbufbyte(&castpoint,0L,sizeof(SeqPoint_T));
	playsong(sng_LITF, 1);
}

void updatecast(void)
{
	int i, sfx;
	
	if(++castpoint.tics < castseq->tpf)
		return;

	if (castseq == &mobseq[mobinfo[castinfo[castnum].mobtype].deathseq] && castpoint.frame >= castseq->numframes-1)
	{
		castnum++;
		castdeath = 0;
		if (castinfo[castnum].name < CCBoy || castinfo[castnum].name > CCShaw) 
			castnum = 0;
		if (mobinfo[castinfo[castnum].mobtype].spotsfx)
			playsfx(mobinfo[castinfo[castnum].mobtype].spotsfx, 0, 255);
		castseq = &mobseq[mobinfo[castinfo[castnum].mobtype].walkseq];
		castpoint.frame = 0;
		castpoint.tics = 0;
	}
	else
	{
		castpoint.frame++;
		castpoint.tics = 0;

		switch (castseq->frame[castpoint.frame].trigger)
		{
			case 3:
			sfx = SfxCatAttack;
			break;
			case 4:
			sfx = SfxCatFire;
			break;
			case 5:
			sfx = SfxWandFire;
			break;
			case 7:
			sfx = SfxCatAttack;
			break;
			case 8:
			sfx = SfxBabyShoot;
			break;
			case 16:
			sfx = SfxKineneSplat;
			break;
			case 22:
			sfx = SfxCameraGun;
			break;
			case 24:
			sfx = SfxPunch;
			break;
			case 27:
			sfx = SfxCatAttack;
			break;
			case 29:
			sfx = SfxOgreFire;
			break;
			case 33:
			sfx = SfxWhoosh;
			break;
			case 38:
			sfx = SfxZombieAttack;
			break;
			default:
			sfx = 0;
			break;
		}
		if (sfx)
			playsfx(sfx, 0, 255);
	}
	if (castseq == &mobseq[mobinfo[castinfo[castnum].mobtype].walkseq] && castpoint.frame >= castseq->numframes)
	{
		if (castonmeele)
			castseq = &mobseq[mobinfo[castinfo[castnum].mobtype].meeleseq];
		else	
			castseq = &mobseq[mobinfo[castinfo[castnum].mobtype].missileseq];

		castonmeele ^= 1;
		if (castseq == &mobseq[-1])
		{
			if (castonmeele)
				castseq = &mobseq[mobinfo[castinfo[castnum].mobtype].meeleseq];
			else	
				castseq = &mobseq[mobinfo[castinfo[castnum].mobtype].missileseq];
		}
		castattacking = 1;
		castpoint.frame = 0;
		castpoint.tics = 0;
		if (castnum == CCShaw)
			goto stopattack;
	}
	if (castattacking)
	{
		if (castpoint.frame >= castseq->numframes)
		{
			stopattack:
			castattacking = 0;
			castseq = &mobseq[mobinfo[castinfo[castnum].mobtype].walkseq];
			castpoint.frame = 0;
			castpoint.tics = 0;
		}
	}
}

void castkeypress(void)
{
	if (oldreadch != readch)
	{
		if ((readch&128) != 0)
			return;

		if (castdeath) return;

		castdeath = 1;
		castattacking = 0;
		castseq = &mobseq[mobinfo[castinfo[castnum].mobtype].deathseq];
		castpoint.frame = 0;
		castpoint.tics = 0;
		if (mobinfo[castinfo[castnum].mobtype].deathsfx)
			playsfx(mobinfo[castinfo[castnum].mobtype].deathsfx, 0, 255);
	}
}

void castprint(char *str)
{
	long i, cx, width, w;
	int c;
	short fontpic, centre;
	char *ch;
	
	ch = str;
	width = 0;
	while (ch)
	{
		c = *ch++;
		if (!c)
			break;
		c = toupper(c) - 'A';
		if (c < 'A' || c > 'Z')
		{
			width += 5;
			continue;
		}
		w = tilesizx[GFONT+c]+1;
		width += w;
	}
	cx = 150-width/2;
	while(*str)
	{
		if(*str == ' ') { cx += 5; str++; continue; }
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
			cx += 5;
			str++;
			continue;
		}
		// draw it
		rotatesprite(cx<<16,180<<16,65536L,0,fontpic,0,0,10+16,0,0,xdim-1,ydim-1);

		cx += tilesizx[fontpic]+1;
        	str++;
	}
}

void drawcast(void)
{
    	short ang;
	char flags;
	
	rotatesprite(0,0,65536L,0,YOUWIN,0,0,2+8+16+64,0,0,xdim-1,ydim-1);
	
	castprint(string[castinfo[castnum].name]);
	if (!castdeath)
	{
		ang = 1024;
		flags = 4;
	}
	else
	{
		ang = 0;
		flags = 0;
	}
	rotatesprite(160<<16,150-(tilesizy[castseq->frame[castpoint.frame].picnum]>>2)<<16,65536,ang,castseq->frame[castpoint.frame].picnum,0,0,2+flags,0,0,xdim-1,ydim-1);
}
