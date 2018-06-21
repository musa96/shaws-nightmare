#include "smix.h"
#include "detect.h"

#define MAXPCNOTES 256

typedef enum
{
	SfxNone,
	SfxBoyDie,
	SfxBoyDie2,
	SfxBoySpot,
	SfxBoySpot1,
	SfxBoyHurt,
	SfxCatSpot,
	SfxCatHurt,
	SfxCatDie,
	SfxDogDie,
	SfxDogHurt,
	SfxDogRoam,
	SfxElevatorStart,
	SfxElevatorReach,
	SfxDoor,
	SfxDoorSlab,
	SfxSwingOpen,
	SfxSwingClose,
	SfxSlabMove,
	SfxExplode,
	SfxHenFire,
	SfxItemUp,
	SfxLand,
	SfxAmmoUp,
	SfxCatFire,
	SfxWandFire,
	SfxWeaponUp,
	SfxSwitch,
	SfxShawPain1,
	SfxShawPain2,
	SfxShawPain3,
	SfxShawPain4,
	SfxShawDie,
	SfxShawScream,
	SfxTeleport,
	SfxCameraGun,
	SfxFlamerUp,
	SfxFlamerIdle,
	SfxFlamerFire,
	SfxFlamerHit,
	SfxGlass,
	SfxCatAttack,
	SfxProjHit,
	SfxFusionExp,
	SfxCannonFire,
	SfxChuiSpot,
	SfxChuiRoam,
	SfxChuiHurt,
	SfxChuiDie,
	SfxHugeSpot,
	SfxHugeDie,
	SfxRapidFire,
	SfxBabyShoot,
	SfxClementSpot,
	SfxClementSpt2,
	SfxClementPain1,
	SfxClementPain2,
	SfxClementPain3,
	SfxClementPain4,
	SfxClementPain5,
	SfxClementPain6,
	SfxFoldDoorOpen,
	SfxFoldDoorClose,
	SfxKineneFoot,
	SfxKineneSplat,
	SfxKaliSpot,
	SfxKaliHurt,
	SfxKaliDie,
	SfxKineneSpot,
	SfxKineneHurt,
	SfxKineneDie,
	SfxPowerUp,
	SfxSuperFire,
	SfxSuperReload,
	SfxLITFGuySpot,
	SfxLITFGuyHurt,
	SfxLITFGuyDie,
	SfxBossSpot,
	SfxBossHurt,
	SfxBossDie,
	SfxBossSpit,
	SfxCubeLand,
	SfxKaribaPain,
	SfxKaribaDie,
	SfxFatumaSpot,
	SfxFatumaDie,
	SfxPlopperSpot,
	SfxPlopperHurt,
	SfxPlopperDie,
	SfxShawDieHigh,
	SfxGirlSpot,
	SfxGirlSpot2,
	SfxGirlHurt,
	SfxGirlDie,
	SfxOgreSpot,
	SfxOgreDie,
	SfxZombieSpot,
	SfxZombieHurt,
	SfxZombieDie,
	SfxWhoosh,
	SfxPunch,
	SfxFlame,
	SfxFlameStart,
	SfxKnightSpot,
	SfxKnightDie,
	SfxClaratinSpot,
	SfxClaratinHurt,
	SfxClaratinDie,
	SfxOgreFire,
	SfxZombieAttack,
	SfxFatumaWalk,
	
	NumSfx
} Sfx;

typedef struct
{
	int bps, numnotes;
	int note[MAXPCNOTES];
} PCSPEAKER;

extern PCSPEAKER pcsound[NumSfx];
extern int sfxvol;
extern int pcsplaying, pcsfx;

void initsfx(void);
void sfxterm(void);
void playxysfx(int sfx, int index, long x, long y);
void playsfx(int sfx, int index, int vol);
void killsfx(int index);
void killallsfx(void);
