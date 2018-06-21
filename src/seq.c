#include "seq.h"
#include "build.h"
#include "dat.h"
#include "engine.h"
#include "debug.h"

char seqheader[4];

void loadseq(char *name, Seq_T *seq)
{
	long i, handle;

	handle = loaditem(name);
	readitem(handle, &seqheader, 4);
	if (seqheader[0] != 'S' && seqheader[1] != 'E' && seqheader[2] != 'Q'
		&& seqheader[3] != 0x1a)
		error("SEQ file is corrupt", 9103);
	readitem(handle, &seq->numframes, 2);
	readitem(handle, &seq->tpf, 2);
	readitem(handle, &seq->frame[0], sizeof(SeqFrame_T)*seq->numframes);
	for(i=0;i<seq->numframes;i++)
		if (waloff[seq->frame[i].picnum] == 0)
			loadtile(seq->frame[i].picnum);
}
