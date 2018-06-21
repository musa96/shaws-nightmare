#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "build.h"
#include "pragmas.h"
#include "einfo.h"
#include "engine.h"
#include "names.h"
#include "names2.h"

#define VERSION "SN/ver1.8"

typedef enum
{
	INTRO,
	GAME,
	INTERMISSION,
	ENDOFEPISODE

} gamestate_t;

typedef enum
{
	fullversion,
	demomode,
	sequel

} gamemode_t;

extern int numdemos;
extern int gamestate, custom_map, nomonsters;
extern int gameepisode, gamelevel, gameskill;
extern int paused, gamemode;
extern int respawnparm;
extern char boardfilename[13];
extern char tilefilename[255];

typedef enum
{
	Baby,
	Easy,
	Medium,
	Hard,
	Nightmare
} Skill;

void initnew(void);

#define rnd(X) ((krand()>>8)>=(255-(X)))
#define spawnsprite(newspriteindex2,x2,y2,z2,cstat2,shade2,pal2,       \
		clipdist2,xrepeat2,yrepeat2,xoffset2,yoffset2,picnum2,ang2,      \
		xvel2,yvel2,zvel2,owner2,sectnum2,statnum2,lotag2,hitag2,extra2,spawnmob,mobtype) \
{                                                                      \
	spritetype *spr2;                                                   \
	newspriteindex2 = insertsprite(sectnum2,statnum2);                  \
	spr2 = &sprite[newspriteindex2];                                    \
	spr2->x = x2; spr2->y = y2; spr2->z = z2;                           \
	spr2->cstat = cstat2; spr2->shade = shade2;                         \
	spr2->pal = pal2; spr2->clipdist = clipdist2;                       \
	spr2->xrepeat = xrepeat2; spr2->yrepeat = yrepeat2;                 \
	spr2->xoffset = xoffset2; spr2->yoffset = yoffset2;                 \
	spr2->picnum = picnum2; spr2->ang = ang2;                           \
	spr2->xvel = xvel2; spr2->yvel = yvel2; spr2->zvel = zvel2;         \
	spr2->owner = owner2;                                               \
	spr2->lotag = lotag2; spr2->hitag = hitag2; spr2->extra = extra2;   \
	show2dsprite[newspriteindex2>>3] |= (1<<(newspriteindex2&7));    \
	if (spawnmob) addmob(newspriteindex2, mobtype);			\
}                                                                      \
