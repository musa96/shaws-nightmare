#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include "config.h"

int grafmode, sfxcard, musicsource, inputdevice;
int screenwidth, screenheight;
long setupver;
char key[14];
FILE *cfgfil;

int getsymbol(char *str)
{
	char buffer[80], tmp[80], tmp2[80];
	int value;
	
	while (fgets(buffer, 80, cfgfil) != NULL)
	{
		sscanf(buffer, "%s", tmp);
		if (strstr(str, tmp) != 0)
		{
			sscanf(buffer, "%s = %s", tmp, tmp2);
			value = atol(tmp2);
			return(value);
		}
	}
	return(-1);
}

void writesymbol(char *str, int value)
{
	fprintf(cfgfil, "%s = %d\n", str, value);
}

int loadconfig()
{
	int i;
	char tempbuf[80];

	cfgfil = fopen(CFGFILE, "r");
	if (cfgfil == NULL) return -1;
	setupver = getsymbol("setupver");
        if (setupver != 0x0f) {
                printf("This is from an older version of Setup!\n");
                printf("Your settings will not be read.\n");
        printf("Please re-run SETUP.\n");
        fclose(cfgfil);
        return -1;
        }
        else {
	grafmode = getsymbol("grafmode");
	sfxcard = getsymbol("sfx");
	musicsource = getsymbol("music");
	inputdevice = getsymbol("input");
	screenwidth = getsymbol("screenwidth");
	screenheight = getsymbol("screenheight");
	for(i=0;i<14;i++)
	{
		sprintf(tempbuf, "key%i", i);
		key[i] = getsymbol(tempbuf);
	}
        fclose(cfgfil);
        }
        return 0;
}

int saveconfig()
{
        int cfgfil;

    if ((cfgfil = open(CFGFILE, O_BINARY|O_CREAT|O_WRONLY,S_IWRITE)) == -1) return -1;
        setupver = 0x0c;
        write(cfgfil, &setupver, 1);
        write(cfgfil, &grafmode, 1);
        write(cfgfil, &sfxcard, 1);
        write(cfgfil, &musicsource, 1);
        write(cfgfil, &inputdevice, 1);
        close(cfgfil);
        return 0;
}
