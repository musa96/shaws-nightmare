#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <conio.h>
#include <io.h>
#include <dos.h>
#include <string.h>
#include <sys\stat.h>
#include <sys\types.h>

#define MAXSTRINGS 256

int numstrings, strlength[MAXSTRINGS];
char string[MAXSTRINGS][512];
char buffer[512];

int main(int argc, char *argv[])
{
	FILE *file1;
	long i, file2;

	if (argc < 3)
	{
		printf("STRGEN v1.2	- By Micheal Muniko\n");
		printf("Syntax: strgen [text file] [string file]\n");
		exit(0);
	}
	file1 = fopen(argv[1], "rb");
	if (!file1)
	{
		printf("Cannot open %s", argv[1]);
		exit(-1);
	}
	while (fgets(buffer, 512, file1) != NULL)
	{
		printf("%s\n", buffer);
		strcpy(string[numstrings], buffer);
		strlength[numstrings] = strlen(string[numstrings]) - 2;
		numstrings++;
		if (numstrings > MAXSTRINGS)
		{
			printf("ERROR! TOO MANY STRINGS!\n");
			fclose(file1);
			exit(-1);
		}
	}
	fclose(file1);
	if ((file2 = open(argv[2], O_BINARY|O_CREAT|O_TRUNC|O_WRONLY,S_IWRITE)) == -1)
	{
		printf("Cannot open string file\n");
		exit(-1);
	}
	write(file2,&numstrings,2);
	for(i=0;i<numstrings;i++)
	{
		write(file2,&strlength[i],2);
		write(file2,&string[i],strlength[i]);
	}
	close(file2);
	printf("Saved to %s\n", argv[2]);
	return(0);
}
