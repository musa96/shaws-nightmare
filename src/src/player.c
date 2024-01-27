#include "sn.h"
#include "saveg.h"
#include "config.h"
#include "qaf.h"
#include "kbd.h"
#include "controls.h"
#include "map2d.h"
#include "weapon.h"
#include "player.h"
#include "sound.h"
#include "sfx.h"
#include "menus.h"
#include "stbar.h"
#include "str.h"
#include "movers.h"
#include "mirrors.h"
#include "graphics.h"
#include "levels.h"
#include "timer.h"
#include "stbar.h"
#include "warp.h"
#include "triggers.h"
#include "ai.h"
#include "maps.h"
#include "demo.h"

char cheatdef[NUMCHEATCODES][11] =
{
	{"frank"},
	{"ring"},
	{"boost"},
	{"warp##"},
	{"orb"},
	{"stan"},
	{"suit"},
	{"light"},
	{"sweater"},
	{"keys"},
	{"etdie"},
	{"weapons"},
	{"etwalk"},
	{"goonies"},
	{"tina"},
	{"coords"},
	{"view"},
	{"jericho"},
	{"song##"}
};

short numtyres;
long posx, posy, posz, horiz, hvel, goalz;
short ang, cursectnum, ocursectnum;
int damagecnt, bonuscnt;
short havekey[3];
int armor, armortype, powerup[NumPowerUps];
short playersprite, playermob, death, bob;
short boompack, undead, noclip;
int temphealth, msgtime;
char onteleportsector, teleporterhold;
short fallcnt, cheatphase, kills, secretsfound, cheated;
char playerabouttoexit, crouching, foundsupersecret;
long dragx, dragy;
char *message, coordsbuf[80];
long cameradist = -1, cameraang = 0, cameraclock = 0;
long synctics = 0, lockclock = 0;
char cheatbuf[11], cheatbufleng;
int ticks, seconds, minutes;

static short neartagsector, neartagwall, neartagsprite;
static long neartagdist, neartaghitdist;
extern Qaf_T helpqaf;

void initplayersprite(void)
{
	long i;

	spawnsprite(playersprite,posx,posy,posz,1+256,0,0,32,48,48,0,0,mobinfo[MobPlayer].standpicnum,
		ang,0,0,0,-1,cursectnum,0,0,0,0,0,0);

	playermob = addmob(playersprite, MobPlayer);
	mob[playermob].health = temphealth;
	for(i=0;i<NumPowerUps;i++)
		powerup[i] = 0;
	for(i=0;i<3;i++)
		havekey[i] = 0;
	clearbufbyte(&sync,sizeof(INPUT),0L);
	bob = 0;
	infinteammo = 0;
	damagecnt = 0;
	bonuscnt = 0;
	ticks = 0;
	seconds = 0;
	minutes = 0;
	numpalookups = 32;
	cameradist = -1;
	cameraclock = 0;
	playerabouttoexit = 0;
}

void updateplayersprite(void)
{
	if (playerabouttoexit == 1) return;

	setsprite(playersprite,posx,posy,posz+PLAYERHEIGHT);
	sprite[playersprite].ang = ang;
	if (sprite[playersprite].z > mob[playermob].loz)
		sprite[playersprite].z = mob[playermob].loz;

	sprite[playersprite].pal = (powerup[Invulnerability] ? 2 : 0);
}

void hurtplayer(int amount)
{
	damagecnt += amount;
	if (damagecnt > 100) damagecnt = 100;

	if (mob[playermob].health <= 0) death = 1;
}

printmessage256(char name[82])
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

void givepowerup(int power)
{
	switch (power)
	{
		case Invulnerability:
		powerup[Invulnerability] = 30*35;
		sprite[playersprite].pal = 2;
		mob[playermob].bits[Harmable] = 0;
		break;
		case Invisibility:
		powerup[Invisibility] = 30*35;
		sprite[playersprite].cstat |= 2;
		break;
		case Torch:
		powerup[Torch] = 120*35;
		break;
		case Lavaproof:
		powerup[Lavaproof] = 60*35;
		break;
		case ComputerMap:
		powerup[ComputerMap] = 1;
		break;
		case Kerzerk:
		healmob(playermob, 100);
		powerup[Kerzerk] = 1;
		break;
	}
}

testneighborsectors(short sect1, short sect2)
{
	short i, startwall, num1, num2;

	num1 = sector[sect1].wallnum;
	num2 = sector[sect2].wallnum;
	if (num1 < num2) //Traverse walls of sector with fewest walls (for speed)
	{
		startwall = sector[sect1].wallptr;
		for(i=num1-1;i>=0;i--)
			if (wall[i+startwall].nextsector == sect2)
				return(1);
	}
	else
	{
		startwall = sector[sect2].wallptr;
		for(i=num2-1;i>=0;i--)
			if (wall[i+startwall].nextsector == sect1)
				return(1);
	}
	return(0);
}

void cheats(void)
{
	char ch, keystate;
	long i, j, k;

	if (demorecording || demoplayback || gameskill == Nightmare) return;

	if (cheatphase == 2)
	{
		while (keyfifoplc != keyfifoend)
		{
			ch = keyfifo[keyfifoplc];
			keystate = keyfifo[(keyfifoplc+1)&(KEYFIFOSIZ-1)];
			keyfifoplc = ((keyfifoplc+2)&(KEYFIFOSIZ-1));

			if (keystate != 0)
			{
				ch = scantoasc[ch];
				if (!((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')))
				{					
					//putmessage(string[StringBadCheat]);
					cheatphase = 0;
					return;
				}
				if (ch != 0) cheatbuf[cheatbufleng++] = ch;
			}
			if (cheatbufleng > 11) cheatphase = 0;
			for(k=0;k<NUMCHEATCODES;k++)
			{
				for(j=0;j<cheatbufleng;j++)
					if (cheatbuf[j] == cheatdef[k][j] ||
						(cheatdef[k][j] == '#') && (ch >= 0 && ch <= '9'))
					{
						if (cheatdef[k][j+1] == 0) goto FOUNDCHEAT;
						if (j == cheatbufleng-1) return;
					}
					else break;
			}
			FOUNDCHEAT:
			switch (k)
			{
				case 0:
				cheatbufleng = 0;
				keyfifoplc = keyfifoend = 0;      //Reset keyboard fifo
				if (undead == 0)
				{
					mob[playermob].bits[Harmable] = 0;
					undead = 1;
					putmessage(string[StringUndeadOn]);
				}
				else
				{
					mob[playermob].bits[Harmable] = 1;
					undead = 0;
					putmessage(string[StringUndeadOff]);
				}
				cheatphase = 0;
				break;
				case 1:
				cheatbufleng = 0;
				keyfifoplc = keyfifoend = 0;      //Reset keyboard fifo
				if (!powerup[Invisibility]) givepowerup(Invisibility);
				else
				{
					powerup[Invisibility] = 0;
					sprite[playersprite].cstat &= ~2;
				}
				cheatphase = 0;
				break;
				case 2:
				cheatbufleng = 0;
				keyfifoplc = keyfifoend = 0;      //Reset keyboard fifo
				mob[playermob].health = 200;
				if (death)
				{
					death = 0;
					sprite[playersprite].cstat |= 257;
					mob[playermob].bits[Harmable] = 1;
					mob[playermob].bits[DropOff] = 0;
				}
				cheatphase = 0;
				break;
				case 3:
				cheatbufleng = 0;
				keyfifoplc = keyfifoend = 0;      //Reset keyboard fifo
				cheatphase = 0;
				j = cheatbuf[4] - '0';
				k = cheatbuf[5] - '0';
				if (gamemode != sequel)
				{
					if ((j >= 1 && j <= 5) && (k >= 1 && k <= 9))
					{
						initnew();
						startlevel(j, k, gameskill);
					}
				}
				else
				{
					j *= 10;
					k += j;
					if (k >= 1 && k <= 32)
					{
						initnew();
						startlevel(0, k, gameskill);
					}
				}
				break;
				case 4:
				cheatbufleng = 0;
				keyfifoplc = keyfifoend = 0;      //Reset keyboard fifo
				if (!powerup[Invulnerability]) givepowerup(Invulnerability);
				else
				{
					mob[playermob].bits[Harmable] = 1;
					powerup[Invulnerability] = 0;
				}
				cheatphase = 0;
				break;
				case 5:
				cheatbufleng = 0;
				keyfifoplc = keyfifoend = 0;      //Reset keyboard fifo
				if (!powerup[Kerzerk]) givepowerup(Kerzerk);
				else powerup[Kerzerk] = 0;
				cheatphase = 0;
				break;
				case 6:
				cheatbufleng = 0;
				keyfifoplc = keyfifoend = 0;      //Reset keyboard fifo
				if (!powerup[Lavaproof]) givepowerup(Lavaproof);
				else powerup[Lavaproof] = 0;
				cheatphase = 0;
				break;
				case 7:
				cheatbufleng = 0;
				keyfifoplc = keyfifoend = 0;      //Reset keyboard fifo
				if (!powerup[Torch]) givepowerup(Torch);
				else powerup[Torch] = 0;
				cheatphase = 0;
				break;
				case 8:
				cheatbufleng = 0;
				keyfifoplc = keyfifoend = 0;      //Reset keyboard fifo
				armor = 200;
				armortype = 2;
				putmessage(string[StringFullArmor]);
				cheatphase = 0;
				break;
				case 9:					
				cheatbufleng = 0;
				keyfifoplc = keyfifoend = 0;      //Reset keyboard fifo
				for(i=0;i<3;i++)
					havekey[i] = 1;
				putmessage(string[StringAllKeys]);
				cheatphase = 0;
				break;
				case 10:
				cheatbufleng = 0;
				keyfifoplc = keyfifoend = 0;      //Reset keyboard fifo
				hurtmob(playermob, 1000, -1);
				cheatphase = 0;
				break;
				case 11:
				cheatbufleng = 0;
				keyfifoplc = keyfifoend = 0;      //Reset keyboard fifo
				for(i=0;i<NumWeapons;i++)
					haveweapon[i] = 1;
				for(i=0;i<NumAmmoTypes;i++)
					ammo[i] = ammomax[i];
				if (gamemode == demomode)
				{
					haveweapon[CameraGun] = 0;
					haveweapon[Cannon] = 0;
					ammo[Batteries] = 0;
				}
				putmessage(string[StringFullAmmo]);
				cheatphase = 0;
				break;
				case 12:
				cheatbufleng = 0;
				keyfifoplc = keyfifoend = 0;      //Reset keyboard fifo
				if (noclip == 0)
				{
					noclip = 1;
					putmessage(string[StringNoClipOn]);
				}
				else
				{
					noclip = 0;
					putmessage(string[StringNoClipOff]);
				}
				cheatphase = 0;
				break;
				case 13:
				cheatbufleng = 0;
				keyfifoplc = keyfifoend = 0;      //Reset keyboard fifo
				for(i=0;i<MAXSECTORS>>3;i++)
					show2dsector[i] = 255;
				for(i=0;i<MAXWALLS>>3;i++)
					show2dwall[i] = 255;
				for(i=0;i<MAXSPRITES>>3;i++)
					show2dsprite[i] = 255;
				putmessage(string[StringFullMap]);
				cheatphase = 0;
				break;
				case 14:
				cheatbufleng = 0;
				keyfifoplc = keyfifoend = 0;      //Reset keyboard fifo
				for(i=0;i<NumWeapons;i++)
					haveweapon[i] = 1;
				for(i=0;i<NumAmmoTypes;i++)
					ammo[i] = ammomax[i];
				if (gamemode == demomode)
				{
					haveweapon[CameraGun] = 0;
					haveweapon[Cannon] = 0;
					ammo[Batteries] = 0;
				}
				infinteammo = 1;
				putmessage(string[StringFullAmmo]);
				cheatphase = 0;
				break;
				case 15:
				cheatbufleng = 0;
				keyfifoplc = keyfifoend = 0;      //Reset keyboard fifo
				sprintf(coordsbuf, "X: %ld, Y: %ld, Z: %ld, Angle: %d", posx, posy,
					posz, ang);
				putmessage(coordsbuf);
				cheatphase = 0;
				break;
				case 16:
				cheatbufleng = 0;
				keyfifoplc = keyfifoend = 0;      //Reset keyboard fifo
				if (cameradist < 0) cameradist = 0; else cameradist = -1;
				cameraang = 0;
				cheatphase = 0;
				break;
				case 17:
				cheatbufleng = 0;
				keyfifoplc = keyfifoend = 0;      //Reset keyboard fifo
				for(i=Stones;i<NumAmmoTypes;i++)
					ammo[i] = 0;
				for(i=Doberwand;i<NumWeapons;i++)
					haveweapon[i] = 0;
				mob[playermob].health = 1;
				weapstate = WEAP_UP;
				curweapon = Machete;
				armor = 0;
				armortype = 0;
				clearbufbyte(&weapseqpoint,0L,sizeof(SeqPoint_T));
				putmessage(string[StringRobbed]);
				cheatphase = 0;
				break;
				case 18:
				cheatbufleng = 0;
				keyfifoplc = keyfifoend = 0;      //Reset keyboard fifo
				cheatphase = 0;
				j = cheatbuf[4] - '0';
				k = cheatbuf[5] - '0';
				j *= 10;
				k += j;
				if (gamemode == sequel)
				{
					k += sng_Future;
					if (k > NUMMUSIC-1) { putmessage(string[StringMusicNotChanged]); break; }
					playsong(k, 1);
					putmessage(string[StringMusicChange]);
				}
				else
				{
					if (k > sng_s31) break;
					playsong(k, (k > sng_b5));
				}
				break;
			}
		}
	}
	else
	{
		if (keystatus[50] > 0)
		{
			keystatus[50] = 0;
			cheatphase++;
			if (cheatphase == 2)
			{
				cheated = 1;
				cheatbufleng = 0;
				for(i=0;i<11;i++) cheatbuf[i] = 0;
				for(i=0;i<KEYFIFOSIZ;i++) keyfifo[i] = 0;
				keyfifoplc = keyfifoend;
				k = -1;
				//putmessage(string[StringTypeCheat]);
			}
			if (cheatphase > 2)
				cheatphase = 2;
		}
	}
}

void process_input(void)
{
	long i, doubvel, xvect, yvect, hihit, lohit;

	getinput();

	if (demorecording)
	{
		numdemoframes++;
		if (keystatus[16] > 0)
		{
			keystatus[16] = 0;
			savedemo(demofilename);
		}
		if (numdemoframes >= MAXDEMOFRAMES)
			savedemo(demofilename);
	}
	if (keystatus[59] > 0)
	{
		keystatus[59] = 0;
		playsfx(SfxSwitch, 0, 255);
		loadqaf("HELP.QAF", &helpqaf);
		menu = 6;
		menustatus = 1;
	}
	if (keystatus[60] > 0 && !demoplayback && !death)
	{
		keystatus[60] = 0;
		playsfx(SfxSwitch, 0, 255);
		selectslot = -1;
		readslots();
		menu = 5;
		menustatus = 1;
	}
	if (keystatus[61] > 0)
	{
		keystatus[61] = 0;
		playsfx(SfxSwitch, 0, 255);
		selectslot = -1;
		readslots();
		menu = 4;
		menustatus = 1;
	}
	if (keystatus[62] > 0)
	{
		keystatus[62] = 0;
		playsfx(SfxSwitch, 0, 255);
		menu = 3;
		menustatus = 1;
	}
	if (keystatus[0x3f] > 0)
	{
		keystatus[0x3f] = 0;
		playsfx(SfxSwitch, 0, 255);
		detailmode++;
		if (detailmode > 2) detailmode = 0;
		putmessage(string[StringHighDetail+detailmode]);
	}
	if (keystatus[64] > 0)
	{
		keystatus[64] = 0;
		if (quickslot != -1 && !death && !demoplayback)
			savegame(quickslot);
		else
		{			
			if (!death && !demoplayback)
			{
				selectslot = -1;
				readslots();
				menu = 5;
				menustatus = 1;
			}
		}
	}
	if (keystatus[65] > 0 && !demoplayback)       //F7
	{
		keystatus[65] = 0;
		playsfx(SfxSwitch, 0, 255);
		menu = 9;
		menustatus = 1;
	}
	if (keystatus[66])  //F8 - screen re-size
	{
		keystatus[66] = 0;

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
					setviewsize();
					break;
				}
		}
		else if (validmodecnt > 0)
		{
			setgamemode(1,validmodexdim[0],validmodeydim[0]);
			setviewsize();
		}
	}
	if (keystatus[67] > 0)
	{
		keystatus[67] = 0;
		if (quickslot != -1)
			loadgame(quickslot);
	}
	if (keystatus[68] > 0)
	{
		keystatus[68] = 0;
		playsfx(SfxSwitch, 0, 255);
		menu = 7;
		menustatus = 1;
	}
	if (keystatus[0x57] > 0)  //F11 - brightness
	{
		keystatus[0x57] = 0;
		brightness++;
		if (brightness > 8) brightness = 0;
		dopalettestuff();
		putmessage(string[StringGammaLevel0+brightness]);
	}
	if (keystatus[88] > 0)   //F12
	{
		keystatus[88] = 0;
		screencapture("captxxxx.pcx",keystatus[0x2a]|keystatus[0x36]);
	}
	if (keystatus[37] > 0)
	{
		keystatus[37] = 0;
		aim_mode ^= 1;
		putmessage(string[StringAimModeOFF+aim_mode]);
	}
	if (sync.bits[Weapon1] > 0)
	{
		if (oflags[Weapon1] == 0)
		{
			newweap = Machete;
			if (newweap == Machete &&
				haveweapon[Flamer] &&
			    !(powerup[Kerzerk] && curweapon == Flamer))
				changeweapon(Flamer);
			else
				changeweapon(Machete);
		}
	}
	if (sync.bits[Weapon2] > 0)
	{
		if (oflags[Weapon2] == 0)
			changeweapon(MiniCatapult);
	}
	if (sync.bits[Weapon3] > 0)
	{
		if (oflags[Weapon3] == 0)
		{
			if (haveweapon[SuperDoberwand] && (gamemode == sequel) && curweapon != SuperDoberwand)
				changeweapon(SuperDoberwand);
			else
				changeweapon(Doberwand);
		}
	}
	if (sync.bits[Weapon4] > 0)
	{
		if (oflags[Weapon4] == 0)
			changeweapon(RapidCatapult);
	}
	if (sync.bits[Weapon5] > 0)
	{
		if (oflags[Weapon5] == 0)
			changeweapon(Launcher);
	}

	if (sync.bits[Weapon6] > 0)
	{
		if (oflags[Weapon6] == 0)
			changeweapon(CameraGun);
	}

	if (sync.bits[Weapon7] > 0)
	{
		if (oflags[Weapon7] == 0)
			changeweapon(Cannon);
	}
	if (keystatus[15] > 0)
	{
		keystatus[15] = 0;
		mapmode++;
		if (mapmode > 2) mapmode = 0;
		if (mapmode == 0) setviewsize();
	}
	if (keystatus[12] > 0)
	{
		if (!mapmode)
		{
			keystatus[12] = 0;
			screensize += 8;
			setviewsize();
			playsfx(SfxSwitch, 0, 255);
		}
		else
		{
			zoom -= 8;
			if (zoom < 48) zoom = 48;
		}
	}

	if (keystatus[13] > 0)
	{
		if (!mapmode)
		{
			keystatus[13] = 0;
			screensize -= 8;
			setviewsize();
			playsfx(SfxSwitch, 0, 255);
		}
		else
		{
			zoom += 8;
			if (zoom > 2048) zoom = 2048;
		}
	}
	cheats();
	if (paused) return;

	if (teleporterhold > 2 || playerabouttoexit || (mapmode && !followmode))
	{
		sync.vel = 0;
		sync.svel = 0;
		sync.angvel = 0;
	}
	if (sync.angvel < 0) sync.angvel = min(sync.angvel+12,0);
	if (sync.angvel > 0) sync.angvel = max(sync.angvel-12,0);
	if (sync.svel < 0) sync.svel = min(sync.svel+5,0);
	if (sync.svel > 0) sync.svel = max(sync.svel-5,0);
	if (sync.vel < 0) sync.vel = min(sync.vel+5,0);
	if (sync.vel > 0) sync.vel = max(sync.vel-5,0);

	if (sync.angvel != 0)          //ang += angvel * constant
	{                              //ENGINE calculates angvel for you
		doubvel = TICSPERFRAME*3;
		if (sync.bits[Run] > 0)  //Lt. shift makes turn velocity 50% faster
			doubvel += (TICSPERFRAME>>1);
		ang += ((sync.angvel*doubvel)>>4);
		ang = (ang+2048)&2047;
	}

	ocursectnum = cursectnum;
	if ((((sync.vel|sync.svel) != 0) || (dragx|dragy)))
	{
		if (death == 0)
		{
			if (crouching == 1)			
				mob[playermob].seq = SeqPlayDuck;
			else
				mob[playermob].seq = mobinfo[MobPlayer].walkseq; // Hack to make player walk
		}
		doubvel = TICSPERFRAME*3;
		bob++;
		if (bob >= 16) bob = 0;
		if (sync.bits[Run] > 0)     //Lt. shift doubles forward velocity
			doubvel += TICSPERFRAME*3;
		xvect = 0, yvect = 0;
		if (sync.vel != 0)
		{
			xvect += ((sync.vel*doubvel*(long)sintable[(ang+2560)&2047])>>3);
			yvect += ((sync.vel*doubvel*(long)sintable[(ang+2048)&2047])>>3);
		}
		if (sync.svel != 0)
		{
			xvect += ((sync.svel*doubvel*(long)sintable[(ang+2048)&2047])>>3);
			yvect += ((sync.svel*doubvel*(long)sintable[(ang+1536)&2047])>>3);
		}
		xvect += dragx<<9;
		yvect += dragy<<9;
		if (noclip == 0)
			clipmove(&posx,&posy,&posz,&cursectnum,xvect,yvect,128L,4L<<8,4L<<8,CLIPMASK0);
		else
		{
			posx += xvect>>14;
			posy += yvect>>14;
			updatesector(posx,posy,&cursectnum);
		}
	}
	else
	{	
		if (bob > 0) bob--;
		if (death == 0 && playerabouttoexit == 0)
		{
			if (crouching == 1)
			{
				mob[playermob].seq = SeqPlayDuck;
			}
			else
			{
				mob[playermob].seq = -1;
				sprite[playersprite].picnum = mobinfo[MobPlayer].standpicnum;
			}
		}
	}
	sprite[playersprite].cstat &= ~1;
	getzrange(posx,posy,posz,cursectnum,&mob[playermob].hiz,&hihit,&mob[playermob].loz,&lohit,128L,CLIPMASK0);
	if (death == 0) sprite[playersprite].cstat |= 1;
	if (noclip == 0)
	{
		if (pushmove(&posx,&posy,&posz,&cursectnum,128L,4<<8L,4<<8L,CLIPMASK0) == -1)
				hurtmob(playermob, 1000, -1);
	}

	if (death == 0)
	{
		goalz = mob[playermob].loz-PLAYERHEIGHT;   //playerheight pixels above floor
		if (goalz < mob[playermob].hiz+(4<<8))
			goalz = mob[playermob].hiz+(4<<8);
		if (!playerabouttoexit)
		{
			if (sync.bits[Jump] > 0 || sprite[playersprite].zvel)                            //A (stand high)
			{
				sprite[playersprite].zvel = 0;
				if (oflags[Jump] == 0)
				{
					if (posz >= mob[playermob].loz-PLAYERHEIGHT)
						goalz -= (51<<8);
				}
			}
			if (sync.bits[Crouch] > 0)                            //Z (stand low)
			{
				goalz += (24<<8);
				crouching = 1;
			}	
			else
				crouching = 0;
		}
		if (goalz != posz)
		{
			if (posz < goalz) hvel += (TICSPERFRAME<<6);
			if (posz > goalz) hvel = ((goalz-posz)>>3);

			posz += hvel;
			if (posz < goalz) // falling
			{
				if (hvel >= (4096+2048)) hvel = (4096+2048);
				if (hvel > 2400 && fallcnt < 255)
				{
					fallcnt++;
					if (fallcnt == 37)
						playsfx(SfxShawScream, 0, 255);
				}
				if ((posz+hvel) >= goalz) // hit the ground
				{
					if (fallcnt > 62) hurtmob(playermob, 1000, -1);
					else if (fallcnt > 9)
					{
						i = fallcnt;
						hurtmob(playermob, i-(krand()&3), -1);
					}
					else if (hvel > 2048) playsfx(SfxLand, 0, 255);
				}
			}
			else
				fallcnt = 0;
			if (posz > mob[playermob].loz-(4<<8)) posz = mob[playermob].loz-(4<<8), hvel = 0;
			if (posz < mob[playermob].hiz+(4<<8)) posz = mob[playermob].hiz+(4<<8), hvel = 0;
		}
		else
			hvel = 0;
	}
	else
	{
		hvel += (TICSPERFRAME<<6);
		posz += hvel;
		if (hvel >= (4096+2048)) hvel = (4096+2048);
		if (posz > mob[playermob].loz-(4<<8))
		{
			posz = mob[playermob].loz-(4<<8);
			hvel = 0;
		}
	}
	if (sync.bits[LookUp] > 0)
		horiz = min(300,horiz+((sync.bits[Run]+1)<<2));

	if (sync.bits[LookDown] > 0)
		horiz = max(-100,horiz-((sync.bits[Run]+1)<<2));

	if (sync.bits[LookCentre] > 0)
		horiz = 100;

	if (sync.bits[Fire] > 0 && !playerabouttoexit)
	{
		if (oflags[Fire] == 0)
			fireweapon();
	}
	
	if (sync.bits[Activate] > 0)
	{
		neartag(posx,posy,posz,cursectnum,ang,&neartagsector,&neartagwall,&neartagsprite,&neartaghitdist,1024L,3);
		if (neartagsector == -1)
		{
			i = cursectnum;
			if ((sector[i].lotag|sector[i].hitag) != 0)
				neartagsector = i;
		}
		if (oflags[Activate] == 0)
		{
			if (neartagsector >= 0)
				operatesector(neartagsector);
			if (neartagwall >= 0)
				operatewall(neartagwall);
			if (neartagsprite >= 0)
				operatesprite(neartagsprite);
		}
	}
	for(i=0;i<NumControls;i++)
		oflags[i] = sync.bits[i];
}

void playerinspecialsector(void)
{
	long i, j, dax, day;

	if (ocursectnum != cursectnum)
	{
		switch(sector[cursectnum].lotag)
		{
			case 20:
			sector[cursectnum].lotag = 0;
			if (sector[cursectnum].hitag == 0)
				i = nextsectorneighborz(cursectnum,sector[cursectnum].floorz,1,0);
			else
				i = nextsectorneighborz(cursectnum,sector[cursectnum].floorz,1,1);
			sector[cursectnum].floorpicnum = sector[i].floorpicnum;
			addlift(cursectnum, 102, sector[cursectnum].hitag);
			break;
			case 21:
			for(i=0;i<numsectors;i++)
			{
				if (sector[i].lotag == sector[cursectnum].hitag)
					operateblazezdoor(i);
			}
			break;
			case 22:
			sector[cursectnum].lotag = 0;
			for(i=0;i<numsectors;i++)
			{
				if (sector[i].lotag == sector[cursectnum].hitag)
				{
					j = addtrob(114, i);
					playxysfx(SfxDoor, 3, trob[j].centx, trob[j].centy);
				}
			}
			break;
			case 25:
			sector[cursectnum].lotag = 0;
			for(i=0;i<numsectors;i++)
			{
				if (sector[i].lotag == sector[cursectnum].hitag)
					addtrob(26, i);
			}
			break;
			case 27:
			case 28:
			if (sector[cursectnum].lotag == 28) sector[cursectnum].lotag = 0;
			for(i=0;i<numsectors;i++)
			{
				if (sector[i].lotag == sector[cursectnum].hitag)
				{
					if (sector[cursectnum].lotag == 27)
						operatelift(i, sector[i].hitag);
					else
						addlift(i, 102, sector[i].hitag);
				}
			}
			break;
			case 31:
			case 32:
			if (sector[cursectnum].lotag == 32) sector[cursectnum].lotag = 0;
			for(i=0;i<numsectors;i++)
			{
				if (sector[i].lotag == sector[cursectnum].hitag)
				{
					if (sector[cursectnum].lotag == 31)
						operatezdoor(i);
					else {
						j = addtrob(112, i);
						playxysfx(SfxDoor, 3, trob[j].centx, trob[j].centy);
					}
				}
			}	
			break;
			case 33:
			sector[cursectnum].lotag = 0;
			for(i=0;i<numsectors;i++)
			{
				if (sector[i].lotag == sector[cursectnum].hitag)
				{
					j = addtrob(116, i);
					playxysfx(SfxDoor, 3, trob[j].centx, trob[j].centy);
				}
			}
			break;
			case 34:
			sector[cursectnum].lotag = 0;
			for(i=0;i<numsectors;i++)
			{
				if (sector[i].lotag == sector[cursectnum].hitag)
				{
					for(j=0;j<numtrobs;j++)
					{
						if (trob[j].sectnum == i)
						{
							trob[j].direction = 1;
							trob[j].type = 111;
							break;
						}
					}
					j = addtrob(111, i);
					trob[j].direction = 1;
				}
			}
			break;
			case 40:
			sector[cursectnum].lotag = 0;
			secretsfound++;
			putmessage(string[StringFoundSecret]);
			break;
			case 41:
			sector[cursectnum].lotag = 0;
			foundsupersecret = 1;
			putmessage(string[StringFoundSecret]);
			break;
			case 42:
			sector[cursectnum].lotag = 0;
			for (i=0;i<numsectors;i++)
			{
				if (sector[cursectnum].hitag == sector[i].lotag)
					addlift(i, 104, sector[i].hitag);
			}
			break;
			case 57:
			sector[cursectnum].lotag = 0;
			for(i=0;i<numsectors;i++)
			{
				if (sector[i].lotag == sector[cursectnum].hitag)
				{
					crusholdz[crushcnt] = sector[i].ceilingz;
					crushsector[crushcnt] = i;
					addtrob(63, i);
					crushcnt++;
				}
			}
			break;
			case 70:
			sector[cursectnum].lotag = 0;
			for(i=0;i<numsectors;i++)
			{
				if (sector[i].lotag == sector[cursectnum].hitag)
					addlift(i, 103, sector[i].hitag);
			}
			break;
			case 108:
			sector[cursectnum].lotag = 0;
			for(i=0;i<numsectors;i++)
			{
				if (sector[i].lotag == sector[cursectnum].hitag)
				{
					for(j=0;j<numtrobs;j++)
					{
						if (trob[j].sectnum == i)
						{
							trob[j].direction = 1;
							trob[j].type = 113;
							break;
						}
					}
					j = addtrob(113, i);
					trob[j].direction = 1;
					playxysfx(SfxDoor, 3, trob[i].centx, trob[i].centy);
				}
			}
			break;
			break;
			case 160:
			for(i=0;i<MAXSPRITES;i++)
			{
				if (sprite[i].lotag == sector[cursectnum].hitag
				    && sprite[i].picnum == Telemark && !onteleportsector)
				{
					onteleportsector = 1;
				    	warp(i);
				}
			}
			break;
			case 190:
			case 191:
			for(i=0;i<MAXSPRITES;i++)
			{
				if (sector[cursectnum].hitag == sprite[i].lotag)
				{
					if (sector[cursectnum].lotag == 191)
						sprite[i].extra = 1;
					else
						sprite[i].hitag = 100;
					changespritestat(i,5);
				}
			}
			break;
			case 65535:
			if (death == 0 && !playerabouttoexit)
			{
				if (sector[cursectnum].hitag < 2)
				{
					i = sector[cursectnum].wallptr;
					dax = wall[i].x-wall[wall[i].point2].x;
					day = wall[i].y-wall[wall[i].point2].y;
					j = ksqrt(dax*dax+day*day);
					ang = getangle(wall[i].x-posx,
							wall[i].y-posy) + (j>>4) / 2;
					sprite[playersprite].ang = ang;
				}
				clearbufbyte(&mob[playermob].seqpoint,sizeof(SeqPoint_T),0L);
				clearbufbyte(&sync,sizeof(INPUT),0L);
				posz = sector[cursectnum].floorz-PLAYERHEIGHT;
				sprite[playersprite].z = sector[cursectnum].floorz;
				cameradist = 0;
				playerabouttoexit = 1;
				mapmode = 0; // exit map
				setviewsize();
			}
			break;
		}
	}
	if (posz < sector[cursectnum].floorz-PLAYERHEIGHT) return;

	switch(sector[cursectnum].floorpicnum)
	{
		case Lava:		
		case 948:
		if ((!(ticks&15)) && (!powerup[Lavaproof]))
			hurtmob(playermob, 5, -1);
		break;
		case Urine:		
		if ((!(ticks&15)) && (!powerup[Lavaproof]))
			hurtmob(playermob, 20, -1);
		break;
	}
	switch(sector[cursectnum].lotag)
	{
		case 1000:
		hurtmob(playermob, 1000, -1);
		break;
	}
}

void tagcode(void)
{
	long i;
	
	if (tag502active)
	{
		tag502time++;
		for(i=0;i<nummobs;i++)
		{
			if (mob[i].type == MobHiddenExploder)
			{
				sprite[mob[i].spritenum].extra++;
				if (sprite[mob[i].spritenum].extra > sprite[mob[i].spritenum].hitag)
				{
					if (mob[i].seq == mobinfo[MobHiddenExploder].deathseq) continue;
					sprite[mob[i].spritenum].xrepeat = 64;
					sprite[mob[i].spritenum].yrepeat = 64;
					sprite[mob[i].spritenum].cstat &= ~0x8000;
					setmobseq(i, mobinfo[mob[i].type].deathseq);
					playxysfx(mobinfo[mob[i].type].deathsfx, 3,
							sprite[mob[i].spritenum].x,
							sprite[mob[i].spritenum].y);
				}
			}
		}
		if (tag502time > 10*35) exitlevel();
	}
}

void checkspecialsectors(void)
{
	long i, j, k, l, p, cnt, good, dax, day;
	short dasector, startwall, endwall;
	short oldang;

	updateinterpolations();

	dragx = 0;
	dragy = 0;
	for(i=0;i<dragsectorcnt;i++)
	{
		dasector = dragsector[i];

		startwall = sector[dasector].wallptr;
		endwall = startwall+sector[dasector].wallnum;

		if (wall[startwall].x+dragxdir[i] < dragx1[i]) dragxdir[i] = 16;
		if (wall[startwall].y+dragydir[i] < dragy1[i]) dragydir[i] = 16;
		if (wall[startwall].x+dragxdir[i] > dragx2[i]) dragxdir[i] = -16;
		if (wall[startwall].y+dragydir[i] > dragy2[i]) dragydir[i] = -16;

		for(j=startwall;j<endwall;j++)
			dragpoint(j,wall[j].x+dragxdir[i],wall[j].y+dragydir[i]);
		j = sector[dasector].floorz;
		sector[dasector].floorz = dragfloorz[i]+(sintable[(lockclock<<4)&2047]>>3);

		if (cursectnum == dasector)
		{
			posx += dragxdir[i];
			posy += dragydir[i];
			updateplayersprite();
		}
		for(k=headspritesect[dragsector[i]];k>=0;k=l)
		{
			l = nextspritesect[k];
			if (k != playersprite)
			{
				sprite[k].x += dragxdir[i];
				sprite[k].y += dragydir[i];
				sprite[k].z = sector[dragsector[i]].floorz;
			}
		}
	}
	for(i=0;i<revolvecnt;i++)
	{
		startwall = sector[revolvesector[i]].wallptr;
		endwall = startwall + sector[revolvesector[i]].wallnum;

		revolveang[i] = ((revolveang[i]-(TICSPERFRAME<<2))&2047);
		for(k=startwall;k<endwall;k++)
		{
			rotatepoint(revolvepivotx[i],revolvepivoty[i],revolvex[i][k-startwall],revolvey[i][k-startwall],revolveang[i],&dax,&day);
			dragpoint(k,dax,day);
		}
		for(k=headspritesect[revolvesector[i]];k>=0;k=nextspritesect[k])
		{
			if (k != playersprite)
			{
				rotatepoint(revolvepivotx[i],revolvepivoty[i],sprite[k].x,sprite[k].y,-(TICSPERFRAME<<2),&dax,&day);
				setsprite(k,dax,day,sprite[k].z);
				sprite[k].ang -= (TICSPERFRAME<<2);
			}
		}
		if (cursectnum == revolvesector[i] && posz >= sector[revolvesector[i]].floorz-PLAYERHEIGHT)
		{
			rotatepoint(revolvepivotx[i],revolvepivoty[i],posx,posy,-(TICSPERFRAME<<2),&dax,&day);
			posx = dax;
			posy = day;
			updatesector(dax, day, &cursectnum);
			ang -= (TICSPERFRAME<<2);
			updateplayersprite();
		}
	}
	for(i=0;i<pansectorcnt;i++)
	{
		if (sector[pansector[i]].lotag == 7 && tag502active) continue;
		dax = (sprite[pansprite[i]].lotag*sintable[(sprite[pansprite[i]].ang+512)&2047])>>9;
		day = (sprite[pansprite[i]].lotag*sintable[sprite[pansprite[i]].ang&2047])>>9;
		j = headspritesect[pansector[i]];
		while (j >= 0)
		{
			k = nextspritesect[j];
			if (sprite[j].z >= sector[pansector[i]].floorz
				&& sprite[j].statnum != 7 && j != playersprite)
			{
				sprite[j].x += dax>>2;
				sprite[j].y += day>>2;
				setsprite(j,sprite[j].x,sprite[j].y,sprite[j].z);
			}
			j = k;
		}
		if (cursectnum == pansector[i] && posz >= sector[pansector[i]].floorz-PLAYERHEIGHT)
		{
			dragx += dax<<3;
			dragy += day<<3;
		}
		sector[pansector[i]].floorxpanning -= dax>>3;
		sector[pansector[i]].floorypanning -= day>>3;
	}
	for(i=0;i<swingcnt;i++)
	{
		if (swinganginc[i] != 0)
		{
			oldang = swingang[i];
			for(j=0;j<(TICSPERFRAME<<2);j++)
			{
				swingang[i] = ((swingang[i]+swinganginc[i])&2047);
				if (swingang[i] == swingangclosed[i])
				{
					playxysfx(SfxSwingClose, swingsector[i], swingx[i][0], swingy[i][0]);
					swinganginc[i] = 0;
				}
				if (swingang[i] == swingangopen[i]) swinganginc[i] = 0;
			}
			for(k=1;k<=3;k++)
				rotatepoint(swingx[i][0],swingy[i][0],swingx[i][k],swingy[i][k],swingang[i],&wall[swingwall[i][k]].x,&wall[swingwall[i][k]].y);

			if (swinganginc[i] != 0)
			{
				if ((cursectnum == swingsector[i]) || (testneighborsectors(cursectnum,swingsector[i]) == 1))
				{
					cnt = 256;
					do
					{
						good = 1;

							//swingangopendir is -1 if forwards, 1 is backwards
						l = (swingangopendir[i] > 0);
						for(k=l+3;k>=l;k--)
							if (clipinsidebox(posx,posy,swingwall[i][k],128L) != 0)
							{
								good = 0;
								break;
							}
						if (good == 0)
						{
							if (cnt == 256)
							{
								swinganginc[i] = -swinganginc[i];
								swingang[i] = oldang;
							}
							else
							{
								swingang[i] = ((swingang[i]-swinganginc[i])&2047);
							}
							for(k=1;k<=3;k++)
								rotatepoint(swingx[i][0],swingy[i][0],swingx[i][k],swingy[i][k],swingang[i],&wall[swingwall[i][k]].x,&wall[swingwall[i][k]].y);
							if (swingang[i] == swingangclosed[i])
							{
								swinganginc[i] = 0;
								break;
							}
							if (swingang[i] == swingangopen[i])
							{
								swinganginc[i] = 0;
								break;
							}
							cnt--;
						}	
					} while ((good == 0) && (cnt > 0));
				}
			}
		if (swinganginc[i] == 0)
			for(j=1;j<=3;j++)
			{
				stopinterpolation(&wall[swingwall[i][j]].x);
				stopinterpolation(&wall[swingwall[i][j]].y);
			}
		}
	}
}

void checkspecialwalls(void)
{
	long i;

	for(i=0;i<numwalls;i++)
	{
		switch(wall[i].lotag)
		{
			case 49:
			wall[i].xpanning += wall[i].hitag;
			break;
			case 50:	
			wall[i].ypanning -= wall[i].hitag;
			break;
		}
	}
}

void checkspecialsprites(void)
{
	long i, nexti, j, k, l, m, mobnum, dax, day, daz;
	int sound, xrepeat, yrepeat;

	for(i=headspritestat[1];i>=0;i=nexti)
	{
		nexti = nextspritestat[i];
		mobnum = findmob(i);
		switch (mob[mobnum].type)
		{
			case MobGlassShard:
			if (sprite[i].zvel > 4096) sprite[i].zvel = 4096;
			if (sprite[i].z == sector[sprite[i].sectnum].floorz
				&& sprite[i].extra < 3)
				{
				sprite[i].zvel = -((3-sprite[i].extra)<<8)-(krand()&511);
				if (rnd(96))
					setsprite(i,sprite[i].x,sprite[i].y,sprite[i].z);
				sprite[i].extra++;
			}
			else if (sprite[i].extra == 3)
			{
				removemob(mobnum);
				deletesprite(i);
				continue;
			}
			if (sprite[i].xvel > 0)
			{
				sprite[i].xvel -= 2;
				mob[mobnum].xvel = (sprite[i].xvel*(sintable[(sprite[i].ang+512)&2047]));
				mob[mobnum].yvel = (sprite[i].xvel*(sintable[sprite[i].ang&2047]));
				sprite[i].cstat = ((sprite[i].xvel&3)<<2);
			}
			else sprite[i].xvel = 0;
			break;
			case MobHeadGiblet:
			case MobLegGiblet:
			case MobHandGiblet:
			case MobFusionSpark:
			if (sprite[i].xvel > 0) sprite[i].xvel -= 2;
			else sprite[i].xvel = 0;
			if (sprite[i].extra < 30*10)
				sprite[i].extra++;
			else { removemob(mobnum); deletesprite((short)i); }
			if (sprite[i].zvel > 1024 && sprite[i].zvel < 1280)
			{
				setsprite(i,sprite[i].x,sprite[i].y,sprite[i].z);
			}
			j = getflorzofslope(sprite[i].sectnum,sprite[i].x,sprite[i].y);
			k = getceilzofslope(sprite[i].sectnum,sprite[i].x,sprite[i].y);
			if (j == k || sprite[i].sectnum < 0 || sprite[i].sectnum >= MAXSECTORS)
			{
				removemob(mobnum);
				deletesprite((short)i);
			}
			if (sprite[i].z < j-(2<<8))
			{
				mob[mobnum].seq = mobinfo[mob[mobnum].type].walkseq;
				if (sprite[i].zvel < 6144)
				{
					sprite[i].zvel += (4<<8);
				}
				sprite[i].x += (sprite[i].xvel*sintable[(sprite[i].ang+512)&2047])>>14;
				sprite[i].y += (sprite[i].xvel*sintable[sprite[i].ang&2047])>>14;
				sprite[i].z += sprite[i].zvel;
			}
			else
			{
				if (sprite[i].sectnum == -1) { removemob(mobnum); deletesprite((short)i); break; }
				if (sector[sprite[i].sectnum].floorstat&2) { removemob(mobnum); deletesprite((short)i); break; }

				j = getflorzofslope(sprite[i].sectnum,sprite[i].x,sprite[i].y);
				sprite[i].z = j-(2<<8);
				sprite[i].xvel = 0;
				if (mob[mobnum].type != MobFusionSpark)
				{
					if (mob[mobnum].seq != SeqGib)
						setmobseq(mobnum, SeqGib);
				}
				else
				{
					removemob(mobnum);
					deletesprite((short)i);
				}	
			}
			break;
		}
	}
	for(i=headspritestat[2];i>=0;i=nexti)
	{
		nexti = nextspritestat[i];
		if ((cansee(posx,posy,posz,cursectnum,sprite[i].x,
				sprite[i].y,sprite[i].z-(tilesizy[sprite[i].picnum]<<7),sprite[i].sectnum) == 1)
				&& (mob[playermob].health) && (!powerup[Invisibility]))
		{
			mobnum = findmob(i);
			if (mobinfo[mob[mobnum].type].spotsfx)
			{
				switch(mobinfo[mob[mobnum].type].spotsfx)
				{
					case SfxBoySpot:
					case SfxBoySpot1:
					sound = SfxBoySpot + krand()%2;
					break;
					case SfxGirlSpot:
					case SfxGirlSpot2:
					sound = SfxGirlSpot + krand()%2;
					break;
					case SfxChuiSpot:
					case SfxChuiRoam:
					sound = SfxChuiSpot + krand()%2;
					break;
					case SfxClementSpot:
					case SfxClementSpt2:
					sound = SfxClementSpot + krand()%2;
					break;
					default:
					sound = mobinfo[mob[mobnum].type].spotsfx;
					break;
				}
				playxysfx(sound, 1024+mobnum, sprite[i].x, sprite[i].y);
			}
			mob[mobnum].aistate = AIWalk;
			mob[mobnum].target = playersprite;
			if (mob[mobnum].type != MobBoss)
				mob[mobnum].movecount = krand()%100;
			else
				mob[mobnum].movecount = 100;
			changespritestat(i, 6);
		}
	}
	for(i=headspritestat[4];i>=0;i=nexti)
	{
		nexti = nextspritestat[i];
		mobnum = findmob(i);
		if (mob[mobnum].movestat == 0 && !(mob[mobnum].seq == mobinfo[mob[mobnum].type].deathseq
							&& mobinfo[mob[mobnum].type].deathseq != -1))
		{
			if (gameskill == Nightmare && sprite[i].owner != playersprite)
				l = 13;
			else
				l = 12;
			if (mob[mobnum].tracer != -1)
			{
				j = getangle(sprite[mob[mobnum].tracer].x-sprite[i].x,sprite[mob[mobnum].tracer].y-sprite[i].y);
				sprite[i].ang = j;
				sprite[i].xvel = (long)sintable[(j+512)&2047]>>6;
				sprite[i].yvel = (long)sintable[j]>>6;
				daz = sprite[mob[mobnum].tracer].z-(40<<8);
				if (mob[mobnum].tracer == playersprite && crouching)
					daz += (24<<8);
				sprite[i].zvel = ((daz-sprite[i].z)<<8) /
						(ksqrt((sprite[mob[mobnum].tracer].x-sprite[i].x) *
						(sprite[mob[mobnum].tracer].x-sprite[i].x) +
						(sprite[mob[mobnum].tracer].y-sprite[i].y) *
						(sprite[mob[mobnum].tracer].y-sprite[i].y))+1);
			}
			mob[mobnum].xvel = ((((long)sprite[i].xvel)*TICSPERFRAME)<<l);
			mob[mobnum].yvel = ((((long)sprite[i].yvel)*TICSPERFRAME)<<l);
			mob[mobnum].zvel = ((((long)sprite[i].zvel)*TICSPERFRAME)>>2);
			if (mob[mobnum].type == MobBabyRay)
			{
				sprite[i].xrepeat++;
				sprite[i].yrepeat++;
			}
			if (mob[mobnum].type == MobTracer)
			{
				if (totalclock & 4) goto bulletskip;
				spawnsprite(j,sprite[i].x,sprite[i].y,sprite[i].z,0,-4,0,32,64,64,0,0,
						Puff,sprite[i].ang,0,0,0,4096,sprite[i].sectnum,7,0,0,0,1,MobPuff);
				k = findmob(j);
				setmobseq(k, mobinfo[MobPuff].walkseq);
			}
		}
		else
		{
			mob[mobnum].xvel = 0;
			mob[mobnum].yvel = 0;
			mob[mobnum].zvel = 0;
			if ((mob[mobnum].movestat&0xc000) == 16384)  //Bullet hit a ceiling
			{
				if (sector[mob[mobnum].movestat&4095].ceilingstat&1)
				{
					// Hack to prevent missiles exploding
					// aganist the sky.
					// Does not handle floor skies.
				    	removemob(mobnum);
					deletesprite(i);
					goto bulletskip;
				}
			}
			else if ((mob[mobnum].movestat&0xc000) == 49152)  //Bullet hit a sprite
			{
				if (mob[mobnum].type != MobHen
				    || mob[mobnum].type != MobFusion)
				{
					for(k=0;k<nummobs;k++)
					{
						if (mob[k].spritenum == (mob[mobnum].movestat&4095))
						{
							switch(mob[mobnum].type)
							{
								case MobKaliBall:
								hurtmob(k, 34, sprite[i].owner);
								break;
								case MobCrimson:
								if (mob[k].type == MobHugeCat) continue;
								hurtmob(k, 20, sprite[i].owner);
								break;
								case MobCPhoto:
								hurtmob(k, 30, sprite[i].owner);
								break;
								case MobTracer:
								hurtmob(k, 15+krand()%10, sprite[i].owner);
								break;
								default:
								if (mob[k].type == MobCat && mob[mobnum].type == MobAcid) continue;
								if (mob[k].type == MobHugeCat && mob[mobnum].type == MobCrimson) continue;
								if (mob[k].type == MobBaby && mob[mobnum].type == MobBabyRay) continue;
								hurtmob(k, 11, sprite[i].owner);
								break;
							}
						}
					}
				}
				if (mob[mobnum].type == MobFlyingFish)
				{
				    	changespritestat(i, 6);
					if (mob[mobnum].health)
					{
						mob[mobnum].aistate = AIWalk;
						setmobseq(mobnum, mobinfo[MobFlyingFish].walkseq);
					}
					goto bulletskip;
				}
				explodeprojectile(mobnum);
				goto bulletskip;
			}
			else if ((mob[mobnum].movestat&0xc000) == 32768)  //Bullet hit a wall
			{
				if (wall[mob[mobnum].movestat&4095].lotag == 100)
				{
					breakwindow(mob[mobnum].movestat&4095);
					mob[mobnum].movestat = 0;
					goto bulletskip;
				}			
				for(j=0;j<mirrorcnt;j++)
				{
					if ((mob[mobnum].movestat&4095) == mirrorwall[j])
					{
						if (wall[mob[mobnum].movestat&4095].overpicnum != 574)
						{
							wall[mob[mobnum].movestat&4095].overpicnum = 574;
							spawnglass(mob[mobnum].movestat&4095, 8);
						}
					}
				}
			}
			if (mob[mobnum].type == MobFlyingFish)
			{
				changespritestat(i, 6);
				if (mob[mobnum].health)
				{
					mob[mobnum].aistate = AIWalk;
					setmobseq(mobnum, mobinfo[MobFlyingFish].walkseq);
				}
				goto bulletskip;
			}
			explodeprojectile(mobnum);
			goto bulletskip;
		}
		bulletskip: continue;
	}
	for(i=headspritestat[5];i>=0;i=nexti)
	{
		nexti = nextspritestat[i];

		sprite[i].hitag++;
		if (sprite[i].hitag > 100)
		{		
			sprite[i].hitag = 0;
			dax = (sintable[(sprite[i].ang+512)&2047]>>6);
			day = (sintable[sprite[i].ang&2047]>>6);
			daz = 0;
			spawnsprite(j,sprite[i].x,sprite[i].y,sprite[i].z,128,0,0,16,64,64,0,0,mobinfo[MobHen].standpicnum,
			sprite[i].ang,dax,day,daz,i,sprite[i].sectnum,4,0,0,0,1,MobHen);
			if (sprite[i].extra != 1) changespritestat(i,0);
		}
	}
	for(i=headspritestat[9];i>=0;i=nexti)
	{
		nexti = nextspritestat[i];
		sprite[i].extra++;
		if (sprite[i].extra > 50)
		{
			sprite[i].extra = 0;
			if (sprite[i].hitag == 0) k = 5;
			else k = 0;
			spawnsprite(j,sprite[i].x,sprite[i].y,sprite[i].z,0,0,k,32,64,64,0,0,mobinfo[MobDroplet].standpicnum,
				   0,0,0,0,-1,sprite[i].sectnum,0,0,0,0,1,MobDroplet);			
		}
	}
	for(i=headspritestat[10];i>=0;i=nexti)
	{
		nexti = nextspritestat[i];
		mobnum = findmob(i);
		mob[mobnum].xvel = ((((long)sprite[i].xvel)*TICSPERFRAME)<<10);
		mob[mobnum].yvel = ((((long)sprite[i].yvel)*TICSPERFRAME)<<10);
		mob[mobnum].zvel = ((((long)sprite[i].zvel)*TICSPERFRAME)>>4);
		if (klabs(sprite[i].x-sprite[mob[mobnum].target].x)+klabs(sprite[i].y-sprite[mob[mobnum].target].y) < 512)
		{
			// kill anything on this spot
			for(j=headspritesect[sprite[i].sectnum];j>=0;j=k)
			{
				k = nextspritesect[j];	
				if ((klabs(sprite[i].x-sprite[j].x)+klabs(sprite[i].y-sprite[j].y)) < 512)
				{
					m = findmob(j);
					if (m != -1) hurtmob(m, 1000, -1);
				}
			}
			if (mob[mobnum].seq == mobinfo[mob[mobnum].type].deathseq) continue;
			xrepeat = 64;
			yrepeat = 64;
			explodeprojectile(mobnum);
			playxysfx(SfxTeleport, 1024+mobnum, sprite[i].x, sprite[i].y);
			j = krand()>>8;
			if (j < 50)
				k = MobCat;
			else if (j < 90)
				k = MobDog;
			else if (j < 120)
				k = MobHugeCat;
			else if (j < 140)
				k = MobPlopper;
			else if (j < 160)
			{
				k = MobFatuma;
				xrepeat = 48;
				yrepeat = 48;
			}
			else if (j < 170)
			{
				k = MobOgre;
				xrepeat = 56;
				yrepeat = 56;
			}
			else if (j < 182)
			{
				k = MobZombieGripper;
			}
			else if (j < 221)
			{
				k = MobLilKinene;
				xrepeat = 48;
				yrepeat = 48;
			}
			else if (j < 245)
			{
				k = MobKnight;
			}
			else
				k = MobKali;
			if (k == MobDog && rnd(196)) l = 2;
			else l = 0;
			spawnsprite(j,sprite[mob[mobnum].target].x,
				sprite[mob[mobnum].target].y,
				sector[sprite[i].sectnum].floorz,1+256|l,0,0,32,
				xrepeat,yrepeat,0,0,mobinfo[k].standpicnum,
				0,0,0,0,-1,sprite[i].sectnum,2,0,0,0,1,k);
			mob[mobnum].xvel = 0;
			mob[mobnum].yvel = 0;
			mob[mobnum].zvel = 0;
			changespritestat(i, 7);
		}
	}
}

void touchspecialsprites(void)
{
	long i, j, k, nexti;
	int sound;

	if (death != 0) return;

	for(i=headspritestat[3];i>=0;i=nexti)
	{
		nexti = nextspritestat[i];
		if (sprite[i].cstat&0x8000) continue;
		if (sprite[i].sectnum != cursectnum) continue;
		if ((klabs(posx-sprite[i].x)+klabs(posy-sprite[i].y) < 512) && (klabs((posz>>8)-((sprite[i].z>>8)-(tilesizy[sprite[i].picnum]>>1))) <= 40))
		{
			sound = SfxItemUp;
			switch(sprite[i].picnum)
			{
				case Stone:
				sound = SfxAmmoUp;
				if (!addammo(Stones, 5))
					return;
				putmessage(string[StringStone]);
				break;
				case StoneBox:
				sound = SfxAmmoUp;
				if (!addammo(Stones, 50))
					return;
				putmessage(string[StringStoneBox]);
				break;
				case Crystal:
				sound = SfxAmmoUp;
				if (!addammo(Crystals, 4))
					return;
				putmessage(string[StringCrystal]);
				break;
				case CBox:
				sound = SfxAmmoUp;
				if (!addammo(Crystals, 20))
					return;
				putmessage(string[StringCBox]);
				break;
				case Egg:
				sound = SfxAmmoUp;
				if (!addammo(HenGrenades, 1))
					return;
				putmessage(string[StringHenGrenade]);
				break;
				case EggBox:
				sound = SfxAmmoUp;
				if (!addammo(HenGrenades, 5))
					return;
				putmessage(string[StringHenBox]);
				break;
				case Battery:
				sound = SfxAmmoUp;
				if (!addammo(Batteries, 48))
					return;
				putmessage(string[StringBattery]);
				break;
				case BattBox:
				sound = SfxAmmoUp;
				if (!addammo(Batteries, 144))
					return;
				putmessage(string[StringBattBox]);
				break;
				case FlamSprite:
				sound = SfxWeaponUp;
				if (!giveweapon(Flamer))
					return;
				putmessage(string[StringFlamer]);
				break;
				case WandSpr:
				sound = SfxWeaponUp;
				if (!giveweapon(Doberwand))
					return;
				putmessage(string[StringDoberwand]);
				break;
				case RapidSpr:
				sound = SfxWeaponUp;
				if (!giveweapon(RapidCatapult))
					return;
				putmessage(string[StringRapid]);
				break;
				case HgnSpr:
				sound = SfxWeaponUp;
				if (!giveweapon(Launcher))
					return;
				putmessage(string[StringLauncher]);
				break;
				case CGunSpr:
				sound = SfxWeaponUp;
				if (!giveweapon(CameraGun))
					return;
				putmessage(string[StringCGun]);
				break;
				case CannonSpr:
				sound = SfxWeaponUp;
				if (!giveweapon(Cannon))
					return;
				putmessage(string[StringCannon]);
				break;
				case SDbSpr:
				sound = SfxWeaponUp;
				if (!giveweapon(SuperDoberwand))
					return;
				putmessage(string[StringSuper]);
				break;
				case FirstAid:
				if (!healmob(playermob, 30))
					return;
				putmessage(string[StringFirstAid]);
				break;
				case Bread:
				if (!healmob(playermob, 25))
					return;
				putmessage(string[StringBread]);
				break;
				case Maize:	
				if (!healmob(playermob, 15))
					return;
				putmessage(string[StringMaize]);
				break;
				case Meat:	
				if (!healmob(playermob, 10))
					return;
				putmessage(string[StringMeat]);
				break;
				case Cupcake:
				if (!healmob(playermob, 8))
					return;
				putmessage(string[StringCupcake]);
				break;
				case Glass:
				mob[playermob].health++;
				if (mob[playermob].health > 200)
					mob[playermob].health = 200;
				putmessage(string[StringGlass]);
				break;
				case Milk:
				sound = SfxPowerUp;
				mob[playermob].health += 100;
				if (mob[playermob].health > 200)
					mob[playermob].health = 200;
				putmessage(string[StringMilk]);
				break;
				case Cereal:
				sound = SfxPowerUp;
				mob[playermob].health = 200;
				armor = 200;
				armortype = 2;
				putmessage(string[StringCereal]);
				break;
				case Key1:
				if (havekey[0]) return;
				havekey[0] = 1;
				putmessage(string[StringGotKey1]);
				break;
				case Key2:
				if (havekey[1]) return;
				havekey[1] = 1;
				putmessage(string[StringGotKey2]);
				break;
				case Key3:
				if (havekey[2]) return;
				havekey[2] = 1;
				putmessage(string[StringGotKey3]);
				break;
				case BoomPack:
				if (!boompack)
				{
					boompack = 1;
					for(j=0;j<NumAmmoTypes;j++)
						ammomax[j] *= 2;
				}
				for(j=0;j<NumAmmoTypes;j++)
					addammo(j, packcontents[j]);
				putmessage(string[StringBOOM]);
				break;
				case Armor1:
				if (armor < 100)
				{
					armor = 100;
					armortype = 1;
					putmessage(string[StringArmor1]);
				}
				else return;
				break;
				case Armor2:
				if (armor < 200)
				{
					armor = 200;
					armortype = 2;
					putmessage(string[StringArmor2]);
				}
				else return;
				break;
				case Repellent:
				armor++;
				if (armor > 200) armor = 200;
				if (!armortype) armortype = 1;
				putmessage(string[StringRepellent]);
				break;
				case Invuln1:
				sound = SfxPowerUp;
				givepowerup(Invulnerability);
				putmessage(string[StringInvulnerability]);
				break;
				case InvSpr:
				sound = SfxPowerUp;
				givepowerup(Invisibility);
				putmessage(string[StringInvisibility]);
				break;
				case TorchSpr:
				sound = SfxPowerUp;
				givepowerup(Torch);
				putmessage(string[StringTorch]);
				break;
				case Suit:
				sound = SfxPowerUp;
				givepowerup(Lavaproof);
				putmessage(string[StringSuit]);
				break;
				case CompMap:
				sound = SfxPowerUp;
				givepowerup(ComputerMap);
				putmessage(string[StringMap]);
				break;
				case KerzerkSpr:
				sound = SfxPowerUp;
				givepowerup(Kerzerk);
				putmessage(string[StringKerzerk]);
				changeweapon(Machete);
				break;
				case Tyre:
				if (!--numtyres)
				{
					for(j=0;j<numsectors;j++)
					{
						if (sector[j].lotag == 777)
						{
							k = addtrob(112, j);
							playxysfx(SfxDoor, 3, trob[k].centx, trob[k].centy);
						}
					}
				}
				putmessage(string[StringSpaceBoard]);
				break;
				default:
				error("Unknown item", 600);
			}
			bonuscnt += 6;
			playsfx(sound, 1, 255);
			deletesprite((short)i);
		}
	}
}

void player(void)
{
	if (menustatus == 1 && !demoplayback) return;
	process_input();
	if (keystatus[28] && demoplayback)
	{
		menu = 0;
		menustatus = 1;
	}
	if (paused) return;

	if (bonuscnt > 0) bonuscnt--;
	if (damagecnt > 0) damagecnt--;

	if (powerup[Invisibility] > 0)
	{
		if (!--powerup[Invisibility])
			sprite[playersprite].cstat &= ~2;
	}
	if (powerup[Invulnerability] > 0)
	{
		if (!--powerup[Invulnerability])
			mob[playermob].bits[Harmable] = 1;
	}
	if (powerup[Torch] > 0)
		powerup[Torch]--;

	if (powerup[Lavaproof] > 0)
		powerup[Lavaproof]--;

	if (powerup[Kerzerk] > 0)
	{
		powerup[Kerzerk]++;
		if (powerup[Kerzerk] > 120)
			powerup[Kerzerk] = 120;
	}
	ticks++;
	if (ticks > 10)
	{
		ticks = 0;
		seconds++;
		if (seconds > 59)
		{
			seconds = 0;
			minutes++;
		}
	}
	updateplayersprite();
	processweapon();
	tagcode();
	checkspecialsectors();
	checkspecialwalls();
	checkspecialsprites();
	playerinspecialsector();
	touchspecialsprites();
	dopalettestuff();
	processtrobs();
	processmobs();
	process_enemies();
	if (death == 1)
	{
		if (mob[playermob].attacker != -1)
		{
			ang = getangle(sprite[mob[playermob].attacker].x-posx,
				       		sprite[mob[playermob].attacker].y-posy);
		}
		horiz = 100;
		if (keystatus[57] > 0 && !demoplayback)
		{
			keystatus[57] = 0;
			initnew();
			loadlevel(boardfilename);
		}
	}
	if (teleporterhold > 0)
	{
		teleporterhold--;
		if (teleporterhold == 0 && sector[cursectnum].lotag == 160)
			teleporterhold = 2;
		else
			onteleportsector = 0;
	}
	if (playerabouttoexit)
	{
		if (sector[cursectnum].hitag < 2)
		{
			if (mob[playermob].movestat == 0)
			{
				 mob[playermob].xvel = sintable[(ang+512)&2047]<<8;
		 	 	 mob[playermob].yvel = sintable[ang&2047]<<8;
			 	 mob[playermob].seq = mobinfo[MobPlayer].walkseq; // Hack to make player walk
				 updatesector(sprite[playersprite].x,sprite[playersprite].y,&cursectnum);
				 sprite[playersprite].z = mob[playermob].loz;
			}
			else
			{
				if (sector[cursectnum].hitag == 0)
					exitlevel();
				else
					exitsecretlevel();
			}
		}
		else
		{
			sprite[playersprite].extra++;
			if (sprite[playersprite].extra == 60)
			{
				sprite[playersprite].z -= PLAYERHEIGHT;
				sprite[playersprite].cstat = 0;
				playsfx(SfxTeleport, 0, 255);
				setmobseq(playermob, mobinfo[MobTeleFlash].walkseq);
			}
			if (sprite[playersprite].extra == 90)
			{
				if (sector[cursectnum].hitag == 2)
					exitlevel();
				else
					exitsecretlevel();
			}
		}
	}
	if (powerup[Torch] > 4*32 || (powerup[Torch]&8)) numpalookups = 1;
	else numpalookups = 32;
	
	if (undead) mob[playermob].bits[Harmable] = 0;
	if (msgtime)
	{
		if (!--msgtime) borderdraw = 1;
	}
	synctics = totalclock-lockclock;
	lockclock += synctics;
}
