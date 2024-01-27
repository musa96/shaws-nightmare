#define MAXITEMS 4096
#define MAXHANDLES 64
#define MAXINDEXES 8

typedef struct
{
	char name[13];
	unsigned long size, offset;
} Item_T;

typedef struct
{
	long numitems, size, offset;
} Index_T;

extern Item_T item[MAXITEMS];

void loaddat(char *filename);
long loaditem(char *name);
long itemlength(char *name);
void readitem(long handle, void *buf, long size);
