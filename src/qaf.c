#include "qaf.h"
#include "build.h"
#include "dat.h"
#include "engine.h"
#include "debug.h"

char qafheader[4];

void loadqaf(char *name, Qaf_T *qaf)
{
	long i, j, handle;

	handle = loaditem(name);
	readitem(handle, &qafheader, 4);
	if (qafheader[0] != 'Q' && qafheader[1] != 'A' && qafheader[2] != 'F'
		&& qafheader[3] != 0x1a)
		error("QAF file is corrupt", 9101);
	readitem(handle, &qaf->numframes, 2);
	readitem(handle, &qaf->numlayers, 2);
	readitem(handle, &qaf->tpf, 2);
	for(i=0;i<qaf->numlayers;i++)
	{
		readitem(handle, &qaf->layer[i].frame[0], sizeof(Frame_T)*qaf->numframes);
		for(j=0;j<qaf->numframes;j++)
			if (waloff[qaf->layer[i].frame[j].picnum] == 0)
				loadtile(qaf->layer[i].frame[j].picnum);
	}
}

int playqaf(Qaf_T *qaf, int *timer, int *frame)
{
	long i, j, k;

	i = *timer;
	i++;
	*timer = i;
	j = *frame;
	if (j >= qaf->numframes-1) return(0);
	if (i > qaf->tpf)
	{
		i = 0;
		for(k=0;k<qaf->numlayers;k++)
			qaftrigger(qaf->layer[k].frame[j].trigger);
		j++;
		*frame = j;
	}
	return(1);
}

void drawqaf(Qaf_T *qaf, int frame, int shade, int stat)
{
	long i;

	for(i=0;i<qaf->numlayers;i++)
	{
		rotatesprite(qaf->layer[i].frame[frame].x,
			     qaf->layer[i].frame[frame].y,
			     qaf->layer[i].frame[frame].zoom,
			     qaf->layer[i].frame[frame].angle,
			     qaf->layer[i].frame[frame].picnum,
			     qaf->layer[i].frame[frame].shade+shade,
			     qaf->layer[i].frame[frame].pal,
			     stat|qaf->layer[i].frame[frame].stat,windowx1,windowy1,windowx2,windowy2);
	}
}
