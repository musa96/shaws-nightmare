#include <io.h>
#include "joys.h"
#include "sn.h"

char joystickbutton(char buttonnum)
{
	return(inp(JOYSTICKPORT&buttonnum));
}

char joystickposition(char axisnum)
{
	int count;

	_asm
	{
		mov word ptr count, 0
		cli
		mov dx, JOYSTICKPORT
		out dx, al
		inc count
		cmp count, 0xffff
		je @done
		in al, dx
		and al, axisnum
		@done:
		sti
	}
	return(count);
}
