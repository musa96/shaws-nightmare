typedef struct
{
	int type;
	short sectnum, direction;
	long centx, centy;
} Trob_T;

typedef struct
{
	int type, xvel, yvel, zvel;
	short spritenum, target, movecount, attacker;
	short tracer;
	char aistate;
	int health, movestat;
	int seq;
	short bits[NumBits];
	long hiz, loz, hihit, lohit;
	SeqPoint_T seqpoint;
	spritetype origsprite; // for nightmare respawn
} Mob_T;

typedef enum
{
	SeqBoyWalk,
	SeqBoyAttack,
	SeqBoyHurt,
	SeqBoyDie,
	SeqBoyExplode,
	SeqFatWalk,
	SeqFatAttack,
	SeqFatHurt,
	SeqFatDie,
	SeqCatWalk,
	SeqCatAttack,
	SeqCatPain,
	SeqCatDie,
	SeqCatExplode,
	SeqDogWalk,
	SeqDogBite,
	SeqDogPain,
	SeqDogDie,
	SeqKaliWalk,
	SeqKaliAttack,
	SeqKaliDie,
	SeqHugeWalk,
	SeqHugeAttack,
	SeqHugePain,
	SeqHugeDie,
	SeqBabyFly,
	SeqBabyAttack,
	SeqBabyPain,
	SeqBabyDie,
	SeqFishFly,
	SeqFishAttack,
	SeqFishDie,
	SeqChuiWalk,
	SeqChuiAttack,
	SeqChuiDie,
	SeqPlayWalk,
	SeqPlayDie,
	SeqPlayDuck,
	SeqExplode,
	SeqCameraExplode,
	SeqFusionExplode,
	SeqFusionSpray,
	SeqAcidExp,
	SeqCrimsonHit,
	SeqPuff,
	SeqGlassShard,
	SeqLegGiblet,
	SeqHeadGiblet,
	SeqGib,
	SeqDuck,
	SeqTeleFlash,
	SeqClementWalk,
	SeqClementFire,
	SeqClementDie,
	SeqClementPort,
	SeqKineneWalk,
	SeqKineneAttack,
	SeqKineneDie,
	SeqGirlWalk,
	SeqGirlAttack,
	SeqGirlDie,
	SeqGirlHurt,
	SeqPlopperFly,
	SeqPlopperFire,
	SeqPlopperDie,
	SeqLITFGuyWalk,
	SeqLITFGuyAttack,
	SeqLITFGuyHurt,
	SeqLITFGuyDie,
	SeqLITFGuyExplode,
	SeqFatumaWalk,
	SeqFatumaFire,
	SeqFatumaDie,
	SeqFireballHit,
	SeqBossSpit,
	SeqBossEyeSee,
	SeqBossDie,
	SeqKnightWalk,
	SeqKnightAttack,
	SeqKnightDie,
	SeqOgreWalk,
	SeqOgreFire,
	SeqOgreAttack,
	SeqOgreDie,
	SeqKaribaHurt,
	SeqKaribaDie,
	SeqLilWalk,
	SeqLilAttack,
	SeqLilDie,
	SeqBrainExplode,
	SeqClFireballHit,
	SeqZombieSpirit,
	SeqZombieWalk,
	SeqZombieAttack,
	SeqZombieDie,
	SeqSpawnEffect,

	SeqEnd
} Seqs;

extern short lightstart[MAXSECTORS], lightend[MAXSECTORS];
extern int spawnspotcnt, spawnsprite[32];
extern long spawnx[32], spawny[32];
extern int numtrobs, nummobs;
extern Trob_T trob[MAXSECTORS];
extern Mob_T mob[MAXSPRITES];

void initmovers(void);
void clearmovers(void);
int movesectorz(short sectnum, long dest, int speed, int direction, int floororceiling);
int addtrob(int type, short sectnum);
void removetrob(short trobnum);
void processtrobs(void);
int addmob(short spritenum, int type);
void removemob(int mobnum);
int findmob(short spritenum);
void explodeprojectile(int mobnum);
int healmob(int mobnum, int amount);
void hurtmob(int mobnum, int amount, int attacker);
void setmobseq(int mobnum, int seq);
movesprite(short spritenum, int mobnum, long dx, long dy, long dz, long ceildist, long flordist, long clipmask);
void processmobs(void);
