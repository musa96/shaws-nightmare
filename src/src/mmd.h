#include "fm.h"

#define MAXNOTES 8192
#define MAXCHANNELS 9
#define NUMINSTS 256

typedef struct
{
	short freq, track;
	short event, paramanter;
} Note_T;

typedef struct
{
	PATCH patch;
	short mt32inst, geninst;
} INSTDATA;

extern short numnotes, trinst[MAXCHANNELS], songtempo;
extern char sngsig[4];
extern int musicstatus, musicloop;
extern Note_T note[MAXNOTES];
extern INSTDATA inst[NUMINSTS];

int initmmd(int driver);
void uninitmmd(void);
void setmusicvolume(int vol);
void fademusic(void);
int loadsong(char *filename);
void mmdupdate(void);
