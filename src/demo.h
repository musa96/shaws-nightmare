#define MAXDEMOFRAMES 16384

extern int demoplayback, demorecording, numdemoframes;
extern int curdemoframe;
extern char demofilename[13];
extern INPUT recsync[MAXDEMOFRAMES];

void playdemo(char *filename);
void savedemo(char *filename);
