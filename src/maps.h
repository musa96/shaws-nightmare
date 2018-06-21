extern int numkills, numsecrets;
extern int i_state, didsecret;

typedef struct
{
	int x, y;
} MapCoords;

typedef struct
{
	int tpf;
	int numframes;
	MapCoords loc;
	short picnum[3];
} MapAnim_T;

void drawintermission(void);
void startintermission(void);
void updateintermission(void);
