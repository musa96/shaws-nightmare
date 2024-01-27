#include <stdlib.h>
#include "sn.h"
#include "sfx.h"
#include "movers.h"

MobInfo_T mobinfo[NumMobTypes] =
{
	{-1,	// MobPlayer
	 100,
	 Player,
	 SeqPlayWalk,
	 -1,
	 -1,
	 -1,
	 SeqPlayDie,
	 -1,
	 SfxShawPain1,
	 0,
	 SfxShawDie,
	 {1, 0, 0, 0, 0}
	},

	{203,	// MobBoy
	 10,
	 Boy,
	 SeqBoyWalk,
	 -1,
	 SeqBoyAttack,
	 SeqBoyHurt,
	 SeqBoyDie,
	 SeqBoyExplode,
	 SfxBoyHurt,
	 SfxBoySpot,
	 SfxBoyDie,
	 {1, 0, 0, 0, 1}
	},
	
	{204,	// MobFat
	 14,
	 Fat,
	 SeqFatWalk,
	 -1,
	 SeqFatAttack,
	 SeqFatHurt,
	 SeqFatDie,
	 SeqBoyExplode,
	 SfxBoyHurt,
	 SfxBoySpot,
	 SfxBoyDie,
	 {1, 0, 0, 0, 1}
	},

	{200,	// MobCat
	 28,
	 Cat,
	 SeqCatWalk,
	 -1,
	 SeqCatAttack,
	 SeqCatPain,
	 SeqCatDie,
	 SeqCatExplode,
	 SfxCatHurt,
	 SfxCatSpot,
	 SfxCatDie,
	 {1, 0, 0, 0, 1}
	},
	
	{201,	// MobDog
	 35,
	 Dog,
	 SeqDogWalk,
	 SeqDogBite,
	 -1,
	 SeqDogPain,
	 SeqDogDie,
	 -1,
	 SfxDogHurt,
	 0,
	 SfxDogDie,
	 {1, 0, 0, 0, 1}
	},

	{202,	// MobKali
	 1500,
	 Kali,
	 SeqKaliWalk,
	 -1,
	 SeqKaliAttack,
	 -1,
	 SeqKaliDie,
	 -1,
	 SfxKaliHurt,
	 SfxKaliSpot,
	 SfxKaliDie,
	 {1, 0, 0, 0, 1}
	},

	{205,	// MobHugeCat
	 50,
	 HugeCat,
	 SeqHugeWalk,
	 -1,
	 SeqHugeAttack,
	 SeqHugePain,
	 SeqHugeDie,
	 -1,
	 SfxCatHurt,
	 SfxHugeSpot,
	 SfxHugeDie,
	 {1, 0, 0, 0, 1}
	},

	{206,	// MobBaby
	 30,
	 FlyBaby,
	 SeqBabyFly,
	 -1,
	 SeqBabyAttack,
	 SeqBabyPain,
	 SeqBabyDie,
	 -1,
	 SfxBoyHurt,
	 0,
	 SfxProjHit,
	 {1, 0, 0, 1, 1}
	},
	
	{207,	// MobFlyingFish
	 25,
	 FlyingFish,
	 SeqFishFly,
	 -1,
	 SeqFishAttack,
	 -1,
	 SeqFishDie,
	 -1,
	 0,
	 0,
	 0,
	 {1, 0, 0, 1, 0}
	},

	{210,	// MobKinene
	 2500,
	 Kinene,
	 SeqKineneWalk,
	 SeqKineneAttack,
	 -1,
	 -1,
	 SeqKineneDie,
	 -1,
	 SfxKineneHurt,
	 SfxKineneSpot,
	 SfxKineneDie,
	 {1, 0, 0, 0, 1}
	},

	{208,	// MobChui
	 2000,
	 Chui,
	 SeqChuiWalk,
	 -1,
	 SeqChuiAttack,
	 -1,
	 SeqChuiDie,
	 -1,
	 SfxChuiHurt,
	 SfxChuiSpot,
	 SfxChuiDie,
	 {1, 0, 0, 0, 1}
	},

	{209,	// MobClement
	 3000,
	 Clement,
	 SeqClementWalk,
	 -1,
	 SeqClementFire,
	 -1,
	 SeqClementDie,
	 -1,
	 SfxClementPain1,
	 SfxClementSpot,
	 SfxChuiDie,
	 {1, 0, 0, 0, 1}
	},

	{-1,	// MobHen
	 0,
	 HenGrenade,
	 -1,
	 -1,
	 -1,
	 -1,
	 SeqExplode,
	 -1,
	 0,
	 0,
	 SfxExplode,
	 {0, 0, 0, 0, 0}
	},

	{-1,	// MobAcid
	 0,
	 Acid,
	 -1,
	 -1,
	 -1,
	 -1,
	 SeqAcidExp,
	 -1,
	 0,
	 0,
	 SfxProjHit,
	 {0, 0, 0, 0, 0}
	},

	{-1,	// MobCrimson
	 0,
	 Crimson,
	 -1,
	 -1,
	 -1,
	 -1,
	 SeqCrimsonHit,
	 -1,
	 0,
	 0,
	 SfxProjHit,
	 {0, 0, 0, 0, 0}
	},

	{-1,	// MobBabyRay
	 0,
	 BabyRay,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 0,
	 0,
	 0,
	 {0, 0, 0, 0, 0}
	},
	
	{-1,	// MobCPhoto
	 0,
	 CPhoto,
	 -1,
	 -1,
	 -1,
	 -1,
	 SeqCameraExplode,
	 -1,
	 0,
	 0,
	 SfxProjHit,
	 {0, 0, 0, 0, 0}
	},

	{-1,	// MobKaliBall
	 0,
	 KaliBall,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 0,
	 0,
	 SfxProjHit,
	 {0, 0, 0, 0, 0}
	},
	
	{-1,	// MobFusion
	 0,
	 FusionProj,
	 -1,
	 -1,
	 -1,
	 -1,
	 SeqFusionExplode,
	 -1,
	 0,
	 0,
	 SfxFusionExp,
	 {0, 0, 0, 0, 0}
	},

	{-1,	// MobChuiBall
	 0,
	 Crimson,
	 -1,
	 -1,
	 -1,
	 -1,
	 SeqExplode,
	 -1,
	 0,
	 0,
	 SfxExplode,
	 {0, 0, 0, 0, 0}
	},

	{-1,	// MobPuff
	 0,
	 Puff,
	 SeqPuff,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 0,
	 0,
	 0,
	 {0, 0, 0, 0, 0}
	},

	{-1,	// MobBlood
	 0,
	 Blood,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 0,
	 0,
	 0,
	 {0, 0, 1, 0, 0}
	},
	
	{-1,	// MobGlassShard
	 0,
	 GlassShard,
	 SeqGlassShard,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 0,
	 0,
	 0,
	 {0, 0, 1, 0, 0}
	},

	{195,	// MobCargo
	 7,
	 Cargo,
	 -1,
	 -1,
	 -1,
	 -1,
	 SeqExplode,
	 -1,
	 0,
	 0,
	 SfxExplode,
	 {1, 1, 0, 0, 0}
	},

	{88,	// MobHiddenExploder
	 0,
	 Cargo,
	 -1,
	 -1,
	 -1,
	 -1,
	 SeqExplode,
	 -1,
	 0,
	 0,
	 SfxExplode,
	 {0, 0, 0, 0, 0}
	},

	{-1,	// MobBrainExplode
	 0,
	 Cargo,
	 -1,
	 -1,
	 -1,
	 -1,
	 SeqBrainExplode,
	 -1,
	 0,
	 0,
	 SfxExplode,
	 {0, 0, 0, 0, 0}
	},
	
	{-1,	// MobHeadGiblet
	 0,
	 559,
	 SeqHeadGiblet,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 0,
	 0,
	 0,
	 {0, 0, 0, 0, 0}
	},

	{-1,	// MobLegGiblet
	 0,
	 564,
	 SeqLegGiblet,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 0,
	 0,
	 0,
	 {0, 0, 0, 0, 0}
	},

	{-1,	// MobHandGiblet
	 0,
	 573,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 0,
	 0,
	 0,
	 {0, 0, 0, 0, 0}
	},

	{-1,	// MobTeleFlash
	 0,
	 736,
	 SeqTeleFlash,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 0,
	 0,
	 0,
	 {0, 0, 0, 0, 0}
	},

	{-1,	// MobDroplet
	 0,
	 567,
	 -1,
	 -1,
	 -1,
	 -1,
	 SeqGib,
	 -1,
	 0,
	 0,
	 0,
	 {0, 0, 1, 0, 0}
	},

	{84,	// MobDuck
	 1,
	 765,
	 -1,
	 -1,
	 -1,
	 -1,
	 SeqDuck,
	 -1,
	 0,
	 0,
	 0,
	 {1, 1, 1, 0, 0}
	},

	{2018,	// MobMilkCarton
	 0,
	 MilkCarton,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 0,
	 0,
	 0,
	 {0, 1, 1, 0, 0}
	},

	{-1,	// MobFusionSpark
	 0,
	 FusionSpark,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 0,
	 0,
	 0,
	 {0, 0, 0, 0, 0}
	},

	{-1,	// MobFusionSpray
	 0,
	 898,
	 SeqFusionSpray,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 0,
	 0,
	 0,
	 {0, 0, 0, 0, 0}
	},

	{215,	// MobGirl
	 20,
	 GirlStand,
	 SeqGirlWalk,
	 -1,
	 SeqGirlAttack,
	 SeqGirlHurt,
	 SeqGirlDie,
	 SeqBoyExplode,
	 SfxGirlHurt,
	 SfxGirlSpot,
	 SfxGirlDie,
	 {1, 0, 0, 0, 1}
	},

	{216,	// MobPlopper
	 75,
	 Plopper,
	 SeqPlopperFly,
	 -1,
	 SeqPlopperFire,
	 -1,
	 SeqPlopperDie,
	 -1,
	 SfxPlopperHurt,
	 SfxPlopperSpot,
	 SfxPlopperDie,
	 {1, 0, 0, 1, 1}
	},

	{3004,	// MobLITFGuy
	 17,
	 LITFGuy,
	 SeqLITFGuyWalk,
	 SeqLITFGuyAttack,
	 -1,
	 SeqLITFGuyHurt,
	 SeqLITFGuyDie,
	 SeqLITFGuyExplode,
	 SfxLITFGuyHurt,
	 SfxLITFGuySpot,
	 SfxLITFGuyDie,
	 {1, 0, 0, 0, 1}
	},
	
	{220,	// MobFatuma
	 36,
	 FatumaStand,
	 SeqFatumaWalk,
	 -1,
	 SeqFatumaFire,
	 -1,
	 SeqFatumaDie,
	 -1,
	 0,
	 SfxFatumaSpot,
	 SfxFatumaDie,
	 {1, 0, 0, 0, 1}
	},

	{-1,	// MobFireball
	 0,
	 Fireball,
	 -1,
	 -1,
	 -1,
	 -1,
	 SeqFireballHit,
	 -1,
	 -1,
	 -1,
	 SfxProjHit,
	 {0, 0, 0, 0, 0}
	},

	{224,	// MobBoss
	 250,
	 0,
	 -1,
	 -1,
	 -1,
	 -1,
	 SeqBossDie,
	 -1,
	 SfxBossHurt,
	 0,
	 SfxBossDie,
	 {1, 0, 0, 0, 0}
	},

	{229,	// MobBossEye
	 0,
	 0,
	 SeqBossEyeSee,
	 -1,
	 SeqBossSpit,
	 -1,
	 -1,
	 -1,
	 0,
	 SfxBossSpot,
	 0,
	 {0, 0, 0, 0, 0}
	},
	
	{-1,	// MobBossSpit
	 0,
	 951,
	 -1,
	 -1,
	 -1,
	 -1,
	 SeqSpawnEffect,
	 -1,
	 -1,
	 -1,
	 SfxCubeLand,
	 {0, 0, 0, 0, 0}
	},

	{320,	// MobKnight
	 500,
	 KnightStand,
	 SeqKnightWalk,
	 -1,
	 SeqKnightAttack,
	 -1,
	 SeqKnightDie,
	 -1,
	 SfxKineneHurt,
	 SfxKnightSpot,
	 SfxKnightDie,
	 {1, 0, 0, 0, 1}
	},

	{3002,	// MobOgre
	 36,
	 Ogre,
	 SeqOgreWalk,
	 SeqOgreAttack, 
	 SeqOgreFire,
	 -1,
	 SeqOgreDie,
	 -1,
	 SfxBoyHurt,
	 SfxOgreSpot,
	 SfxOgreDie,
	 {1, 0, 0, 0, 1}
	},

	{1995,	// MobKariba
	 15,
	 Kariba,
	 -1,
	 -1,
	 -1,
	 SeqKaribaHurt,
	 SeqKaribaDie,
	 -1,
	 SfxKaribaPain,
	 0,
	 SfxKaribaDie,
	 {1, 0, 0, 0, 0}
	},

	{240,	// MobLilKinene
	 75,
	 Lil,
	 SeqLilWalk,
	 -1,
	 SeqLilAttack,
	 -1,
	 SeqLilDie,
	 -1,
	 SfxClaratinHurt,
	 SfxClaratinSpot,
	 SfxClaratinDie,
	 {1, 0, 0, 0, 1}
	},

	{-1,	// MobClFireball
	 0,
	 ClFireball,
	 -1,
	 -1,
	 -1,
	 -1,
	 SeqClFireballHit,
	 -1,
	 -1,
	 -1,
	 SfxProjHit,
	 {0, 0, 0, 0, 0}
	},

	{2019,	// MobSpaceShip
	 0,
	 1220,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 0,
	 0,
	 0,
	 {0, 1, 1, 0, 0}
	},

	{-1,	// MobTracer
	 0,
	 HenGrenade,
	 -1,
	 -1,
	 -1,
	 -1,
	 SeqClFireballHit,
	 -1,
	 0,
	 0,
	 SfxExplode,
	 {0, 0, 0, 0, 0}
	},

	{300,	// MobZombieGripper
	 36,
	 Zombie,
	 SeqZombieWalk,
	 -1, 
	 SeqZombieAttack,
	 -1,
	 SeqZombieDie,
	 -1,
	 SfxZombieHurt,
	 SfxZombieSpot,
	 SfxZombieDie,
	 {1, 0, 0, 0, 1}
	},
	
	{-1,	// MobZombieSpirit
	 0,
	 ZombieGrip,
	 SeqZombieSpirit,
	 -1,
	 -1,
	 -1,
	 -1,
	 -1,
	 0,
	 0,
	 0,
	 {0, 0, 0, 0, 0}
	}
};

VoxelInfo_T voxelinfo[MAXTILES];
