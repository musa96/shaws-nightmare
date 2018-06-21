#define NUMCHEATCODES 19
#define PLAYERHEIGHT (40<<8)

typedef enum
{
	Invulnerability,
	Invisibility,
	Torch,
	Lavaproof,
	ComputerMap,
	Kerzerk,
	SpaceBoard,

	NumPowerUps
} PowerUps;

extern long cameradist, cameraang, cameraclock;
extern long posx, posy, posz, horiz, hvel, goalz;
extern short ang, cursectnum, ocursectnum;
extern int damagecnt, bonuscnt;
extern short havekey[3];
extern int armor, armortype, powerup[NumPowerUps];
extern short playersprite, playermob, death, bob;
extern short boompack, undead, noclip;
extern int temphealth, msgtime;
extern char onteleportsector, teleporterhold;
extern short fallcnt, cheatphase, kills, secretsfound, cheated;
extern char playerabouttoexit, crouching, foundsupersecret;
extern long dragx, dragy;
extern char *message;

extern short numtyres;
extern int ticks, seconds, minutes;

void initplayersprite(void);
void updateplayersprite(void);
void hurtplayer(int amount);
void updatesectorz(long x, long y, long z, short *sectnum);
void player(void);
