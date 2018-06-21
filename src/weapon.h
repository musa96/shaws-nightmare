#define WEAP_IDLE 0
#define WEAP_FIRE 1
#define WEAP_UP 2
#define WEAP_DOWN 3

#define CLOSERANGE 0
#define FARRANGE 1

typedef enum
{
	Machete,
	MiniCatapult,
	Doberwand,
	RapidCatapult,
	Launcher,
	CameraGun,
	Cannon,
	Flamer,
	SuperDoberwand,

	NumWeapons
} Weapons;

typedef enum
{
	Stones,
	Crystals,
	HenGrenades,
	Batteries,

	NumAmmoTypes
} AmmoTypes;

typedef enum
{
	QAFMacheteUp,
	QAFMacheteDown,
	QAFMacheteIdle,
	QAFMachetePoke,

	QAFMiniUp,
	QAFMiniDown,
	QAFMiniIdle,
	QAFMiniFire,

	QAFWandUp,
	QAFWandDown,
	QAFWandIdle,
	QAFWandFire,

	QAFRapidUp,
	QAFRapidDown,
	QAFRapidIdle,
	QAFRapidFire,

	QAFLauncherUp,
	QAFLauncherDown,
	QAFLauncherIdle,
	QAFLauncherFire,

	QAFCameraUp,
	QAFCameraDown,
	QAFCameraIdle,
	QAFCameraFire,
	
	QAFCannonUp,
	QAFCannonDown,
	QAFCannonIdle,
	QAFCannonFire,

	QAFFlamerUp,
	QAFFlamerDown,
	QAFFlamerIdle,
	QAFFlamerFire,

	QAFSuperUp,
	QAFSuperDown,
	QAFSuperIdle,
	QAFSuperFire,

	QAFEnd
} WeaponQafs;

extern int defaultammomax[NumAmmoTypes];
extern int ammo[NumAmmoTypes], ammomax[NumAmmoTypes];
extern short haveweapon[NumWeapons], curweapon, newweap;
extern int packcontents[NumAmmoTypes];
extern int infinteammo, weapstate;
extern SeqPoint_T weapseqpoint;

void initweapon(void);
int giveweapon(int weapon);
int addammo(int ammotype, int amount);
int checkmeelerange(long x1, long x2, long y1, long y2);
void firehitscan(long x, long y, long z, short angle,
		short sectnum, long dahoriz, int damage, int attacker, int range);
void fireprojectile(long x, long y, long z, short angle,
		    short sectnum, long dahoriz, int mobtype, int attacker);
void changeweapon(int weapon);
void fusionspray(void);
void fireweapon(void);
void calcswing(void);
void checkwallhit(short wallnum);
void processweapon(void);
void drawweapon(void);
