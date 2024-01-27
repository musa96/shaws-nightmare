#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <dos.h>
#include <sys\stat.h>
#include <sys\types.h>
#include <string.h>
#include "debug.h"
#include "dat.h"

char *reloadfilename, header[4];
Index_T index;
long numitems, itemhandle[MAXITEMS];
Item_T item[MAXITEMS];

void loaddat(char *filename)
{
	long i, handle, storehandle;
	int startitem;

	startitem = numitems;
	if (filename[0] == '~')
	{
		filename++;
		reloadfilename = filename;
	}
	if ((handle = open(filename,O_BINARY|O_RDONLY,S_IREAD)) == -1)
	{
		printf("Cannot load data file %s\n", filename);
		exit(1);
	}
	read(handle,&header,4);
	if ((header[0] != 'D') && (header[1] != 'A') && (header[2] != 'T')
	    && (header[3] != 0x1a))
	{
		close(handle);
		printf("DAT header corrupted\n");
		exit(1);
	}
	read(handle,&index.numitems,2);
	read(handle,&index.offset,4);
	read(handle,&index.size,4);
	lseek(handle, index.offset, SEEK_SET);
	read(handle,&item[startitem],index.size);
	storehandle = reloadfilename ? -1 : handle;
	numitems += index.numitems;
	for(i=startitem;i<numitems;i++)
		itemhandle[i] = storehandle;
	if (reloadfilename)
		close(handle);
}

int checkitemname(char *name)
{
	long i;

	// Search backwards so mods can take precedence
	for(i=numitems;i>=0;i--)
	{
		if (stricmp(name,item[i].name) == 0)
			return(i);
	}
	return(-1);
}

long loaditem(char *name)
{	
	long i, handle, newhandle;
	char tempbuf[80];
	
	i = checkitemname(name);
	if (i == -1)
	{
		sprintf(tempbuf, "loaditem(): %s not found", name);
		error(tempbuf, 863);
	}
	if (itemhandle[i] == -1)
	{
		if ((handle = open(reloadfilename,O_BINARY|O_RDONLY,S_IREAD)) == -1)
			error("loaditem(): Cannot load data file", 862);
	}
	else handle = itemhandle[i];

	lseek(handle, item[i].offset, SEEK_SET);
	return(handle);
}

long itemlength(char *name)
{
	long i, handle;

	i = checkitemname(name);
	return(item[i].size);
}

void readitem(long handle, void *buf, long size)
{
	char tempbuf[80];
	long l;

	l = read(handle, buf, size);
	if (l < size)
	{
		sprintf(tempbuf, "Read only %d of %d", l, size);
		error(tempbuf, 863);
	}
}

// does nothing.... dummy function
void closeitem(long handle)
{
}
