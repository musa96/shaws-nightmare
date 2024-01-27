#define NUMSWITCHES 9
#define MAXLIFTS 64

typedef struct
{
	short picnum1, picnum2;
} SwitchList_T;

extern SwitchList_T switchlist[];
extern short gatesprite[16], gatetrob[16], gatecnt;
extern short revolvesector[4], revolveang[4], revolvecnt;
extern long revolvex[4][16], revolvey[4][16];
extern long revolvepivotx[4], revolvepivoty[4];
extern short dragsector[16], dragxdir[16], dragydir[16], dragsectorcnt;
extern long dragx1[16], dragy1[16], dragx2[16], dragy2[16], dragfloorz[16];
extern int pansectorcnt;
extern short pansprite[16];
extern long panx[16], pany[16];
extern short pansector[16];
extern int crushcnt;
extern short crushsector[32];
extern long crusholdz[32];
extern short wallfind[MAXSECTORS][2];
extern int activelifts[MAXLIFTS];
extern long liftoldz[MAXLIFTS];
extern short liftsector[MAXLIFTS], lifttrob[MAXLIFTS];
extern short swingcnt, swingwall[32][5], swingsector[32];
extern short swingangopen[32], swingangclosed[32], swingangopendir[32];
extern short swingang[32], swinganginc[32];
extern long swingx[32][8], swingy[32][8];

#define MAXINTERPOLATIONS 1024
extern long numinterpolations, startofdynamicinterpolations;
extern long oldipos[MAXINTERPOLATIONS];
extern long bakipos[MAXINTERPOLATIONS];
extern long *curipos[MAXINTERPOLATIONS];
extern int tag502cnt, tag502active, tag502time;

void setinterpolation(long *posptr);
void stopinterpolation(long *posptr);
void updateinterpolations(void);  //Stick at beginning of domovethings
void dointerpolations(void);       //Stick at beginning of drawscreen
void restoreinterpolations(void);  //Stick at end of drawscreen
void addlift(short sectnum, int trobtype, int direction);
void removelift(int lift);
void operatezdoor(short dasector);
void operateblazezdoor(short sectnum);
void operatesector(short sectnum);
void operatewall(short wallnum);
void operatesprite(short spritenum);
