#define MAXSEQFRAMES 1024

typedef struct
{
	short picnum;
	int trigger;
} SeqFrame_T;

typedef struct
{
	int numframes, tpf;
	SeqFrame_T frame[MAXSEQFRAMES];
} Seq_T;

void loadseq(char *name, Seq_T *seq);
