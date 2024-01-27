#include <stdio.h>

void error(char *msg, int code)
{
	uninit();
	printf("%s\n", msg);
	if (code != -1)
		printf("Error code: $%d\n", code);
	exit(-1);
}
