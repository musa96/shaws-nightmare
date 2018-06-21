#define CFGFILE "SN.SET"

extern char key[14];
extern int grafmode, sfxcard, musicsource, inputdevice;
extern int screenwidth, screenheight;
extern long setupver;

int loadconfig();
int saveconfig();
