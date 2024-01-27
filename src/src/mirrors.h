#define MAXMIRRORS 64

extern short mirrorwall[MAXMIRRORS], mirrorsector[MAXMIRRORS], mirrorcnt;

void initmirrors(void);
void drawmirrors(long cposx, long cposy, long cposz, long choriz, short cang);
