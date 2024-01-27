#define MAXFRAMES 1024
#define MAXLAYERS 8

typedef struct
{
	long x, y, zoom;
	short angle, picnum;
	signed char shade;
	char pal, stat;
	int trigger;
} Frame_T;

typedef struct
{
	Frame_T frame[MAXFRAMES];
} Layer_T;

typedef struct
{
	int numframes, numlayers;
	int tpf;
	Layer_T layer[MAXLAYERS];
} Qaf_T;

void loadqaf(char *name, Qaf_T *qaf);
int playqaf(Qaf_T *qaf, int *timer, int *frame);
void drawqaf(Qaf_T *qaf, int frame, int shade, int stat);
void qaftrigger(int trigger);
