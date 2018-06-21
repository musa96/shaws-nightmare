typedef enum
{
	Harmable,
	SpawnPuff,
	DropOff,
	Float,
	CountKill,

	NumBits
} MobBits;

typedef struct
{
	int tics, frame;
} SeqPoint_T;

typedef struct
{
	short picnum;
	char name[13], spin;
} VoxelInfo_T;

typedef struct
{
	int shawedlotag;
	int starthealth;
	int standpicnum;
	int walkseq;
	int meeleseq;
	int missileseq;
	int hurtseq;
	int deathseq;
	int xdeathseq;
	int hurtsfx;
	int spotsfx;
	int deathsfx;
	int bits[NumBits];
} MobInfo_T;

typedef enum
{
	MobPlayer,
	MobBoy,
	MobFat,
	MobCat,
	MobDog,
	MobKali,
	MobHugeCat,
	MobBaby,
	MobFlyingFish,
	MobKinene,
	MobChui,
	MobClement,
	MobHen,
	MobAcid,
	MobCrimson,
	MobBabyRay,
	MobCPhoto,
	MobKaliBall,
	MobFusion,
	MobChuiBall,
	MobPuff,
	MobBlood,
	MobGlassShard,
	MobCargo,
	MobHiddenExploder,
	MobBrainExplode,
	MobHeadGiblet,
	MobLegGiblet,
	MobHandGiblet,
	MobTeleFlash,
	MobDroplet,
	MobDuck,
	MobMilkCarton,
	MobFusionSpark,
	MobFusionSpray,
	MobGirl,
	MobPlopper,
	MobLITFGuy,
	MobFatuma,
	MobFireball,
	MobBoss,
	MobBossEye,
	MobBossSpit,
	MobKnight,
	MobOgre,
	MobKariba,
	MobLilKinene,
	MobBanFireball,
	MobSpaceShip,
	MobTracer,
	MobZombieGripper,
	MobZombieSpirit,

	NumMobTypes
} MobTypes;

extern MobInfo_T mobinfo[NumMobTypes];
extern VoxelInfo_T voxelinfo[MAXTILES];
