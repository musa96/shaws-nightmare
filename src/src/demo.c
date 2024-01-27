#include <fcntl.h>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <sys\stat.h>
#include <sys\types.h>
#include "sn.h"
#include "debug.h"
#include "levels.h"
#include "weapon.h"
#include "player.h"
#include "controls.h"
#include "demo.h"

extern char vcheck[10];
int demoplayback, demorecording, numdemoframes;
int curdemoframe;
char demofilename[13];
INPUT recsync[MAXDEMOFRAMES];

void playdemo(char *filename)
{
	char tempbuf[80];
	long fil;

	if ((fil = open(filename,O_BINARY|O_RDONLY,S_IREAD)) == -1)
	{
		sprintf(tempbuf, "Error opening demo file %s", filename);
		error(tempbuf, 706);
	}
	read(fil,vcheck,10);
	if (stricmp(vcheck,VERSION) != 0) return;
	read(fil,&numdemoframes,4);
	read(fil,&boardfilename,13);
	read(fil,&gameepisode,2);
	read(fil,&gamelevel,2);
	read(fil,&gameskill,2);
	read(fil,&nomonsters,2);
	read(fil,&recsync[0],sizeof(INPUT)*numdemoframes);
	close(fil);
	initnew();
	loadlevel(boardfilename);
	curdemoframe = 0;
	demoplayback = 1;
}

void savedemo(char *filename)
{
	char tempbuf[80];
	long fil;

	if ((fil = open(filename,O_BINARY|O_CREAT|O_TRUNC|O_WRONLY,S_IWRITE)) == -1)
	{
		sprintf(tempbuf, "Error opening demo file %s", filename);
		error(tempbuf, 707);
	}
	strcpy(tempbuf,VERSION);
	write(fil,&tempbuf,10);
	write(fil,&numdemoframes,4);
	write(fil,&boardfilename,13);
	write(fil,&gameepisode,2);
	write(fil,&gamelevel,2);
	write(fil,&gameskill,2);
	write(fil,&nomonsters,2);
	write(fil,&recsync[0],sizeof(INPUT)*numdemoframes);
	close(fil);
	sprintf(tempbuf, "Demo %s recorded", filename);
	error(tempbuf, -1);
}
