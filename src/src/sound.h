#include "mmd.h"

extern int pcstics, curnote;
extern int musicvol;
extern int gamemusic[];
extern int gamemusic2[];

typedef enum
{
	sng_b1,
	sng_b2,
	sng_b3,
	sng_b4,
	sng_b5,

	sng_s1,
	sng_s2,
	sng_s3,
	sng_s4,
	sng_s5,
	sng_s6,
	sng_s7,
	sng_s8,
	sng_s9,
	sng_s10,
	sng_s11,
	sng_s12,
	sng_s13,
	sng_s14,
	sng_s15,
	sng_s16,
	sng_s17,
	sng_s18,
	sng_s19,
	sng_s20,
	sng_s21,
	sng_s22,
	sng_s23,
	sng_s24,
	sng_s25,
	sng_s26,
	sng_s27,
	sng_s28,
	sng_s29,
	sng_s30,
	sng_s31,

	sng_Future,
	sng_Flow,
	sng_Count,
	sng_Running,
	sng_Night,
	sng_Major,
	sng_Town,
	sng_HitTheWall,
	sng_Work,
	sng_Meremo,
	sng_Ikill,
	sng_Horror,
	sng_OnRun,
	sng_Opening,
	sng_LITF,
	sng_Third,
	sng_Theme,
	sng_Commend,

	NUMMUSIC
} Songs;

void initsound(void);
void uninitsound(void);
void playsong(int musicid, int loop);
void updatesounds(void);
