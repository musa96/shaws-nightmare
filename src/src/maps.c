#include "sn.h"
#include "weapon.h"
#include "player.h"
#include "kbd.h"
#include "timer.h"
#include "maps.h"
#include "graphics.h"
#include "sfx.h"
#include "sound.h"
#include "str.h"
#include "qaf.h"
#include "levels.h"

static Qaf_T i_qaf;
int animtic, animframe;
int lastlevel, nextlevel;
int mapticks, didsecret;
int numkills, numsecrets;
int i_state;
int i_tics, i_frame;

static MapCoords mapcoords[4][10] =
{
	{
		{42, 132},
		{56, 50},
		{191, 41},
		{280, 45},
		{202, 115},
		{119, 45},
		{118, 100},
		{285, 141},
		{147, 157}
	},

	{
		{17, 47},
		{83, 19},
		{11, 106},
		{172, 41},
		{199, 79},
		{270, 140},
		{168, 108},
		{209, 135},
		{248, 29}
	},

	{
		{76, 81},
		{179, 33},
		{153, 82},
		{243, 25},
		{244, 88},
		{226, 116},
		{140, 140},
		{303, 72},
		{116, 41}
	},

	{
		{36, 92},
		{174, 87},
		{252, 90},
		{153, 158},
		{255, 155},
		{307, 51},
		{33, 160},
		{304, 181},
		{206, 144}
	}
};

static MapAnim_T e1anim[2] =
{
	{16, 1, {168, 36}, {MIRROR, MAPANIM1} },
	{11, 1, {91, 16}, {MAPANIM2, MAPANIM2+1} }
};

static MapAnim_T e2anim[2] =
{
	{15, 1, {73, 0}, {MIRROR, 702} },
	{9, 2, {233, 10}, {MIRROR, 703, 704} }
};

static MapAnim_T e3anim[2] =
{
	{30, 2, {145, 41}, {706, 707, 708} },
	{14, 2, {120, 122}, {MIRROR, 709, 710} }
};

static MapAnim_T e4anim[4] =
{
	{24, 1, {5, 57}, {MIRROR, 712} },
	{46, 2, {19, 158}, {750, 751, 750} },
	{50, 2, {234, 116}, {752, 753, 754} },
	{32, 1, {199, 136}, {MIRROR, 713} }
};

void displayheading(void)
{
	int heading;

	if (i_state == 1)
	{
		if (gamemode == sequel) heading = StringMap01+lastlevel-1;
		else heading = StringE1M1+(gameepisode-1)*10+lastlevel-1;
		puttext(110, 1, "EXITING");
		if (gamelevel != 9)
			puttext(100-(strlen(string[heading])>>1), 17, string[heading]);
		else
			puttext(100-(strlen(string[heading])), 17, string[heading]);
	}
	else if (i_state == 2 && nextlevel != -1)
	{
		if (gamemode == sequel) heading = StringMap01+nextlevel-1;
		else heading = StringE1M1+(gameepisode-1)*10+nextlevel-1;
		puttext(110, 1, "NOW ENTERING");
		puttext(100-(strlen(string[heading])>>1), 17, string[heading]);
	}
}

void drawnextloc(void)
{
	long i, j;

	if (gameepisode == 5) return;

	if (i_state == 1)
	{
		j = lastlevel-1;
		if (gamelevel == 9) j++;
		for(i=0;i<j;i++)
		{		
			// draw crosses on the past levels
			rotatesprite(mapcoords[gameepisode-1][i].x<<16,mapcoords[gameepisode-1][i].y<<16,65536L,0,CROSS,0,0,10, 0,0,xdim-1,ydim-1);
		}
		if (gamelevel != 9)
		{
			if (!(tick&24))
				rotatesprite(mapcoords[gameepisode-1][lastlevel-1].x<<16,mapcoords[gameepisode-1][lastlevel-1].y<<16,65536L,0,CROSS,0,0,10,0,0,xdim-1,ydim-1);
			if (didsecret)
				rotatesprite(mapcoords[gameepisode-1][8].x<<16,mapcoords[gameepisode-1][8].y<<16,65536L,0,CROSS,0,0,10, 0,0,xdim-1,ydim-1);
		}
		else
		{
			if (!(tick&24))
				rotatesprite(mapcoords[gameepisode-1][gamelevel-1].x<<16,mapcoords[gameepisode-1][gamelevel-1].y<<16,65536L,0,CROSS,0,0,10,0,0,xdim-1,ydim-1);
		}
	}
	else if (i_state == 2)
	{
		for(i=0;i<=lastlevel-1;i++)
		{
			// draw crosses on the past levels
			rotatesprite(mapcoords[gameepisode-1][i].x<<16,mapcoords[gameepisode-1][i].y<<16,65536L,0,CROSS,0,0,10, 0,0,xdim-1,ydim-1);
		}
		if (didsecret)
			rotatesprite(mapcoords[gameepisode-1][8].x<<16,mapcoords[gameepisode-1][8].y<<16,65536L,0,CROSS,0,0,10, 0,0,xdim-1,ydim-1);
		if (!(tick&24))
			rotatesprite(mapcoords[gameepisode-1][nextlevel-1].x<<16,mapcoords[gameepisode-1][nextlevel-1].y<<16,65536L,0,NEXT,0,0,10,0,0,xdim-1,ydim-1);
	}
}

void drawepisodemap(void)
{	
	int i;
	
	switch (gameepisode)
	{
		case 1:
		rotatesprite(0,0,65536L,0,E1MAP,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
		for(i=0;i<2;i++)
		{	
			if (!(tick&e1anim[i].tpf)) animframe++;
			if (animframe > e1anim[i].numframes) animframe = 0;
			rotatesprite(e1anim[i].loc.x<<16,e1anim[i].loc.y<<16,65536L,0,e1anim[i].picnum[animframe],0,0,8+10,0,0,xdim-1,ydim-1);
		}
		break;
		case 2:
		rotatesprite(0,0,65536L,0,701,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
		for(i=0;i<2;i++)
		{
			if (!(tick&e2anim[i].tpf)) animframe++;
			if (animframe > e2anim[i].numframes) animframe = 0;
			rotatesprite(e2anim[i].loc.x<<16,e2anim[i].loc.y<<16,65536L,0,e2anim[i].picnum[animframe],0,0,8+10,0,0,xdim-1,ydim-1);
		}
		break;
		case 3:
		rotatesprite(0,0,65536L,0,705,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
		for(i=0;i<2;i++)
		{
			if (!(tick&e3anim[i].tpf)) animframe++;
			if (animframe > e3anim[i].numframes) animframe = 0;
			rotatesprite(e3anim[i].loc.x<<16,e3anim[i].loc.y<<16,65536L,0,e3anim[i].picnum[animframe],0,0,8+10,0,0,xdim-1,ydim-1);
		}
		break;
		case 4:	
		rotatesprite(0,0,65536L,0,E4MAP,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
		for(i=0;i<4;i++)
		{
			if (!(tick&e4anim[i].tpf)) animframe++;
			if (animframe > e4anim[i].numframes) animframe = 0;
			rotatesprite(e4anim[i].loc.x<<16,e4anim[i].loc.y<<16,65536L,0,e4anim[i].picnum[animframe],0,0,8+10,0,0,xdim-1,ydim-1);
		}
		break;
		case 5:
		drawqaf(&i_qaf, i_frame, 0, 2);
		break;
	}
	drawnextloc();
	displayheading();
}

void internextstate(void)
{
	i_state++;
	if (i_state == 1)
	{
		if (custom_map)
		{
			loadlevel(boardfilename);
			gamestate = GAME;
			return;
		}
		lastlevel = gamelevel;
		if (gamemode != sequel || gameepisode < 5)
			mapticks = 400;
		else
			mapticks = 1;
		if (secretexit)
		{
			if (gamemode != sequel)
		 	   	nextlevel = 9;
			else
			{
				if (gamelevel != 31)
					nextlevel = 31;
				else
					nextlevel = 32;
			}
		}
		else if (gamelevel == 9 && gamemode != sequel)
		{
			switch (gameepisode)
			{
				case 1:
				lastlevel = 4;
				nextlevel = 5;
				break;
				case 2:
				lastlevel = 5;
				nextlevel = 6;
				break;
				case 3:
				lastlevel = 2;
				nextlevel = 3;
				break;
				case 4:
				lastlevel = 6;
				nextlevel = 7;
				break;
				case 5:
				lastlevel = 2;
				nextlevel = 3;
				break;
				default:
				// Ouch.
				break;
			}
			didsecret = 1;
		}
		else if (gamelevel == 30)
		{
			nextlevel = -1;
			lastlevel = -1;
		}
		else if (gamelevel > 30)
		{
			lastlevel = 15;
			nextlevel = 16;
		}
		else nextlevel = gamelevel+1;
	}
	if (i_state == 2)
	{
		if (gamelevel != 30) mapticks = 400;
		else mapticks = 1;
		if (gameepisode == 4 && nextlevel == 8)
		{
			playsong(sng_b5, 0);
			mapticks = 800;
		}
		playsfx(SfxWeaponUp, 0, 255);
	}
	if (i_state == 3)
	{
		if (gamemode == sequel)
		{
			switch (gamelevel)
			{
				case 15:
				case 31:
				if (!secretexit) break;
				case 6:
				case 11:
				case 20:
				case 30:
				startend();
				return;
			}
		}
		startlevel(gameepisode, nextlevel, gameskill);
		gamestate = GAME;
	}
}

void startintermission(void)
{
	i_state = 0;
	if (gamemode != sequel)
		loadqaf("INTER.QAF", &i_qaf);
	gamestate = INTERMISSION;
	fademusic();
}

void updateintermission(void)
{
	long i;
	
	if (keystatus[28] > 0)
	{
		keystatus[28] = 0;
		internextstate();
	}
	if (i_state == 0 || gameepisode == 5)
	{
		if (gamemode != sequel)
		{
			if (tick > 5)
			{
		    		tick = 0;
				i = playqaf(&i_qaf, &i_tics, &i_frame);
				if (i == 0)
				{
					i_tics = 0;
					i_frame = 0;
				}
			}
		}
	}
	else
	{
		mapticks--;
		if (mapticks < 0) internextstate();
	}
}

void drawintermission(void)
{
	char tempbuf[80];
	long i, y;
	
	switch (i_state)
	{
		case 0:
		setview(0,0,xdim-1,ydim-1);
		if (gamemode != sequel)
			drawqaf(&i_qaf, i_frame, 0, 2);
		else
			rotatesprite(0,0,65536L,0,InterPic,0,0,2+8+16+64,0,0,xdim-1,ydim-1);
		if (cheated)
			puttext(125, 8, ">>YOU CHEATED!<<");
		puttext(145, 24, "COMPLETED");
		sprintf(tempbuf, "KILLS %d / %d", kills, numkills);
		puttext(80, 48, tempbuf);
		sprintf(tempbuf, "SECRETS %d / %d", secretsfound, numsecrets);
		puttext(80, 64, tempbuf);
		if (foundsupersecret)
			puttextpal(80, 80, "YOU FOUND A SUPER SECRET!", 6);
		puttext(80, 104, "PRESS ENTER TO CONTINUE");
		sprintf(tempbuf, "TIME: %.2d:%.2d", minutes, seconds);
		puttext(80, 192, tempbuf);
		break;
		default:
		if (gamemode == sequel)
		{
			rotatesprite(0,0,65536L,0,InterPic,0,0,2+8+16+64,0,0,xdim-1,ydim-1);
			displayheading();
		}
		else
			drawepisodemap();
		break;
	}
}
