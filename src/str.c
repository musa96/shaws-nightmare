/*====================================================================
  This is a private property of Michael Muniko. You may not distribute
  copy or even hack it, without the permission from Michael Muniko.

  Email: michaelmuniko@gmail.com
  ===================================================================*/
#include "dat.h"
#include "str.h"
#include "sn.h"

int numstrings, totalstrings, strlength[NumStrings];
char string[NumStrings][512];

void loadstring(char *name)
{
	long i, fil;

	fil = loaditem(name);
	readitem(fil, &numstrings, 2);
	for(i=0;i<numstrings;i++)
	{
		readitem(fil, &strlength[i+totalstrings], 2);
		readitem(fil, &string[i+totalstrings], strlength[i+totalstrings]);
	}
	totalstrings += numstrings;
}

void loadstrings(void)
{
	long i;

	totalstrings = 0;
	loadstring("MENUS.STR");
	loadstring("AMMO.STR");
	loadstring("ITEMS.STR");
	loadstring("KEYS.STR");
	loadstring("WEAPONS.STR");
	loadstring("CHEATS.STR");
	loadstring("MISC.STR");
	if (gamemode != sequel)
	{
		loadstring("E1NAMES.STR");
		loadstring("E1END.STR");
		loadstring("E2NAMES.STR");
		loadstring("E2END.STR");
		loadstring("E3NAMES.STR");
		loadstring("E3END.STR");
		loadstring("E4NAMES.STR");
		loadstring("E4END.STR");
		loadstring("E5NAMES.STR");
		loadstring("E5END.STR");
	}
	else
	{
		totalstrings += 5*9+6;
		loadstring("MISC2.STR");
		loadstring("MAPNAMES.STR");
		loadstring("C1.STR");
		loadstring("C2.STR");
		loadstring("C3.STR");
		loadstring("C4.STR");
		loadstring("C5.STR");
		loadstring("C6.STR");
		loadstring("CAST.STR");
		string[StringCredits][0] = '5';
		string[StringQuitGame][0] = '6';
	}	
}
