#define LOOKUPSIZ 9728
#define NUMPALOOKUPS 6

extern char *transluc;
extern char *globalpalwritten;
extern long globalpal;

initengine();
	/*Initializes many variables for the BUILD engine.  You should call this
	once before any other functions of the BUILD engine are used.*/

uninitengine();
	/*Frees buffers.  You should call this once at the end of the program
	before quitting to dos.*/

loadboard(char *filename, long *posx, long *posy, long *posz, short *ang, short *cursectnum);
saveboard(char *filename, long *posx, long *posy, long *posz, short *ang, short *cursectnum);
	/*Loads/saves the given board file from memory.  Returns -1 if file not
	found.  If no extension is given, .MAP will be appended to the filename.*/

loadpics(char *filename);
	/*Loads the given artwork file into memory for the BUILD engine.
	Returns -1 if file not found.  If no extension is given, .ART will
	be appended to the filename.*/

loadtile(short tilenum);
	/*Loads a given tile number from disk into memory if it is not already in
	memory.  This function calls allocache internally.  A tile is not in the
	cache if (waloff[tilenum] == 0)*/

setgamemode(char vidoption, long xdim, long ydim);
	/*This function sets the video mode to 320*200*256color graphics.
	Since BUILD supports several different modes including mode x,
	mode 13h, and other special modes, I don't expect you to write
	any graphics output functions.  (Soon I have all the necessary
	functions)  If for some reason, you use your own graphics mode,
	you must call this function again before using the BUILD drawing
	functions.

	vidoption can be anywhere from 0-6
	xdim,ydim can be any vesa resolution if vidoption = 1
	xdim,ydim must be 320*200 for any other mode.
		(see graphics mode selection in my setup program)*/

setview(long x1, long y1, long x2, long y2);
	/*Sets the viewing window to a given rectangle of the screen.
	Example: For full screen 320*200, call like this: setview(0L,0L,319L,199L);*/

nextpage();
	/*This function flips to the next video page.  After a screen is prepared,
	use this function to view the screen.*/

drawrooms(long posx, long posy, long posz, short ang, long horiz, short cursectnum);
	/*This function draws the 3D screen to the current drawing page,
	which is not yet shown.  This way, you can overwrite some things
	over the 3D screen such as a gun.  Be sure to call the drawmasks()
	function soon after you call the drawrooms() function.  To view
	the screen, use the nextpage() function.  The nextpage() function
	should always be called sometime after each draw3dscreen()
	function.*/

drawmasks();
	/*This function draws all the sprites and masked walls to the current
	drawing page which is not yet shown.  The reason I have the drawing
	split up into these 2 routines is so you can animate just the
	sprites that are about to be drawn instead of having to animate
	all the sprites on the whole board.  Drawrooms() prepares these
	variables:  spritex[], spritey[], spritepicnum[], thesprite[],
	and spritesortcnt.  Spritesortcnt is the number of sprites about
	to be drawn to the page.  To change the sprite's picnum, simply
	modify the spritepicnum array  If you want to change other parts
	of the sprite structure, then you can use the thesprite array to
	get an index to the actual sprite number.*/

clearview(long col);
	/*Clears the current video page to the given color*/

clearallviews(long col);
	/*Clears all video pages to the given color*/

drawmapview (long x, long y, long zoom, short ang);
	/*Draws the 2-D texturized map at the given position into the viewing window.*/

rotatesprite (long sx, long sy, long z, short a, short picnum,
				  signed char dashade, char dapalnum, char dastat,
				  long cx1, long cy1, long cx2, long cy2);
	/*(sx, sy) is the center of the sprite to draw defined as
		 screen coordinates shifted up by 16.
	(z) is the zoom.  Normal zoom is 65536.
		 Ex: 131072 is zoomed in 2X and 32768 is zoomed out 2X.
	(a) is the angle (0 is straight up)
	(picnum) is the tile number
	(dashade) is 0 normally but can be any standard shade up to 31 or 63.
	(dapalnum) can be from 0-255.
	if ((dastat&1) == 0) - no transluscence
	if ((dastat&1) != 0) - transluscence
	if ((dastat&2) == 0) - don't scale to setview's viewing window
	if ((dastat&2) != 0) - scale to setview's viewing window (windowx1,etc.)
	if ((dastat&4) == 0) - nuttin' special
	if ((dastat&4) != 0) - y-flip image
	if ((dastat&8) == 0) - clip to startumost/startdmost
	if ((dastat&8) != 0) - don't clip to startumost/startdmost
	if ((dastat&16) == 0) - use Editart center as point passed
	if ((dastat&16) != 0) - force point passed to be top-left corner
	if ((dastat&32) == 0) - nuttin' special
	if ((dastat&32) != 0) - use reverse transluscence
	if ((dastat&64) == 0) - masked drawing (check 255's) (slower)
	if ((dastat&64) != 0) - draw everything (don't check 255's) (faster)
	if ((dastat&128) == 0) - nuttin' special
	if ((dastat&128) != 0) - automatically draw to all video pages

	Note:  As a special case, if both ((dastat&2) != 0) and ((dastat&8) != 0)
		then rotatesprite will scale to the full screen (0,0,xdim-1,ydim-1)
		rather than setview's viewing window. (windowx1,windowy1,etc.)  This
		case is useful for status bars, etc.

		Ex: rotatesprite(160L<<16,100L<<16,65536,totalclock<<4,
							  DEMOSIGN,2,50L,50L,270L,150L);
			 This example will draw the DEMOSIGN tile in the center of the
			 screen and rotate about once per second.  The sprite will only
			 get drawn inside the rectangle from (50,50) to (270,150)*/

drawline256(long x1, long y1, long x2, long y2, char col);
	/*Draws a solid line from (x1,y1) to (x2,y2) with color (col)
	For this function, screen coordinates are all shifted up 16 for precision.*/

printext256(long xpos, long ypos, short col, short backcol,
				char *message, char fontsize);
	/*Draws a text message to the screen.
	(xpos,ypos) - position of top left corner
	col - color of text
	backcol - background color, if -1, then background is transparent
	message - text message
	fontsize - 0 - 8*8 font
				  1 - 4*6 font*/

clipmove(long *x, long *y, long *z, short *sectnum, long xvect, long yvect,
			long walldist, long ceildist, long flordist, unsigned long cliptype);
		/*Moves any object (x, y, z) in any direction at any velocity and will
	make sure the object will stay a certain distance from walls (walldist)
		 Pass the pointers of the starting position (x, y, z).  Then
	pass the starting position's sector number as a pointer also.
	Also these values will be modified accordingly.  Pass the
	direction and velocity by using a vector (xvect, yvect).
	If you don't fully understand these equations, please call me.
			xvect = velocity * cos(angle)
			yvect = velocity * sin(angle)
		Walldist tells how close the object can get to a wall.  I use
	 128L as my default.  If you increase walldist all of a sudden
	 for a certain object, the object might leak through a wall, so
	 don't do that!
		 Cliptype is a mask that tells whether the object should be clipped
	 to or not.  The lower 16 bits are anded with wall[].cstat and the higher
	 16 bits are anded with sprite[].cstat.

	  Clipmove can either return 0 (touched nothing)
										  32768+wallnum (wall first touched)
										  49152+spritenum (sprite first touched)*/

pushmove (long *x, long *y, long *z, short *sectnum,
			 long walldist, long ceildist, long flordist, unsigned long cliptype);
		/*This function makes sure a player or monster (defined by x, y, z, sectnum)
	is not too close to a wall.  If it is, then it attempts to push it away.
	If after 256 tries, it is unable to push it away, it returns -1, in which
	case the thing should gib.*/

getzrange(long x, long y, long z, short sectnum,
						long *ceilz, long *ceilhit,
						long *florz, long *florhit,
						long walldist, unsigned long cliptype);

		/*Use this in conjunction with clipmove.  This function will keep the
	player from falling off cliffs when you're too close to the edge.  This
	function finds the highest and lowest z coordinates that your clipping
	BOX can get to.  It must search for all sectors (and sprites) that go
	into your clipping box.  This method is better than using
	sector[cursectnum].ceilingz and sector[cursectnum].floorz because this
	searches the whole clipping box for objects, not just 1 point.
		Pass x, y, z, sector normally.  Walldist can be 128.  Cliptype is
	defined the same way as it is for clipmove.  This function returns the
	z extents in ceilz and florz. It will return the object hit in ceilhit
	and florhit.  Ceilhit and florhit will also be either:
											16384+sector (sector first touched) or
											49152+spritenum (sprite first touched)*/


hitscan(long xs, long ys, long zs, short sectnum, long vx, long vy, long vz,
	short *hitsect, short *hitwall, short *hitsprite,
	long *hitx, long *hity, long *hitz, unsigned long cliptype);
	/*Pass the starting 3D position:
		  (xstart, ystart, zstart, startsectnum)
	Then pass the 3D angle to shoot (defined as a 3D vector):
		  (vectorx, vectory, vectorz)
	Then set up the return values for the object hit:
		  (hitsect, hitwall, hitsprite)
	and the exact 3D point where the ray hits:
		  (hitx, hity, hitz)

	How to determine what was hit:
		* Hitsect is always equal to the sector that was hit (always >= 0).

		* If the ray hits a sprite then:
			  hitsect = thesectornumber
			  hitsprite = thespritenumber
			  hitwall = -1

		 * If the ray hits a wall then:
			  hitsect = thesectornumber
			  hitsprite = -1
			  hitwall = thewallnumber

		 * If the ray hits the ceiling of a sector then:
			  hitsect = thesectornumber
			  hitsprite = -1
			  hitwall = -1
			  vectorz < 0
			  (If vectorz < 0 then you're shooting upward which means
				  that you couldn't have hit a floor)

		 * If the ray hits the floor of a sector then:
			  hitsect = thesectornumber
			  hitsprite = -1
			  hitwall = -1
			  vectorz > 0
			  (If vectorz > 0 then you're shooting downard which means
				  that you couldn't have hit a ceiling)*/

neartag(long x, long y, long z, short sectnum, short ang,  //Starting position & angle
		  short *neartagsector,   //Returns near sector if sector[].tag != 0
		  short *neartagwall,     //Returns near wall if wall[].tag != 0
		  short *neartagsprite,   //Returns near sprite if sprite[].tag != 0
		  long *neartaghitdist,   //Returns actual distance to object (scale: 1024=largest grid size)
		  long neartagrange,      //Choose maximum distance to scan (scale: 1024=largest grid size)
		  char tagsearch);         //1-lotag only, 2-hitag only, 3-lotag&hitag
		/*Neartag works sort of like hitscan, but is optimized to
	scan only close objects and scan only objects with
	tags != 0.  Neartag is perfect for the first line of your space bar code.
	It will tell you what door you want to open or what switch you want to
	flip.*/

cansee(long x1, long y1, long z1, short sectnum1,
		 long x2, long y2, long z2, short sectnum2);  //returns 0 or 1
	/*This function determines whether or not two 3D points can "see" each
	other or not.  All you do is pass it the coordinates of a 3D line defined
	by two 3D points (with their respective sectors)  The function will return
	a 1 if the points can see each other or a 0 if there is something blocking
	the two points from seeing each other.  This is how I determine whether a
	monster can see you or not. Try playing DOOM1.DAT to fully enjoy this
	great function!*/

updatesector(long x, long y, short *sectnum);
	/*This function updates the sector number according to the x and y values
	passed to it.  Be careful when you use this function with sprites because
	remember that the sprite's sector number should not be modified directly.
	If you want to update a sprite's sector, I recomment using the setsprite
	function described below.*/

inside(long x, long y, short sectnum);
	/*Tests to see whether the overhead point (x, y) is inside sector (sectnum)
	Returns either 0 or 1, where 1 means it is inside, and 0 means it is not.*/

clipinsidebox(long x, long y, short wallnum, long walldist);
	/*Returns TRUE only if the given line (wallnum) intersects the square with
	center (x,y) and radius, walldist.*/

dragpoint(short wallnum, long newx, long newy);
	/*This function will drag a point in the exact same way a point is dragged
	in 2D EDIT MODE using the left mouse button.  Simply pass it which wall
	to drag and then pass the new x and y coordinates for that point.
	Please use this function because if you don't and try to drag points
	yourself, I can guarantee that it won't work as well as mine and you
	will get confused.  Note:  Every wall of course has 2 points.  When you
	pass a wall number to this function, you are actually passing 1 point,
	the left side of the wall (given that you are in the sector of that wall)
	Got it?*/

krand();
	//Random number function - returns numbers from 0-65535

ksqrt(long num);
	//Returns the integer square root of the number.

getangle(long xvect, long yvect);
	/*Gets the angle of a vector (xvect,yvect)
	These are 2048 possible angles starting from the right, going clockwise*/

rotatepoint(long xpivot, long ypivot, long x, long y,
				short daang, long *x2, long *y2);
		/*This function is a very convenient and fast math helper function.
	Rotate points easily with this function without having to juggle your
	cosines and sines.  Simply pass it:

		 Input:   1. Pivot point     (xpivot,ypivot)
					 2. Original point  (x,y)
					 3. Angle to rotate (0 = nothing, 512 = 90ø CW, etc.)
		 Output:  4. Rotated point   (*x2,*y2)*/

lastwall(short point);
		/*Use this function as a reverse function of wall[].point2.  In order
	to save memory, my walls are only on a single linked list.*/

nextsectorneighborz(short sectnum, long thez, short topbottom, short direction);
	/*This function is used to tell where elevators should stop.  It searches
	nearby sectors for the next closest ceilingz or floorz it should stop at.
	sectnum - elevator sector
	thez - current z to start search from
	topbottom - search ceilingz's/floorz's only
	direction - search upwards/downwards*/

getceilzofslope(short sectnum, long x, long y);
getflorzofslope(short sectnum, long x, long y);
getzsofslope(short sectnum, long x, long y, long *ceilz, long *florz);
	/*These 3 functions get the height of a ceiling and/or floor in a sector
	at any (x,y) location.  Use getzsofslope only if you need both the ceiling
	and floor.*/

alignceilslope(short sectnum, long x, long y, long z);
alignflorslope(short sectnum, long x, long y, long z);
	/*Given a sector and assuming it's first wall is the pivot wall of the slope,
	this function makes the slope pass through the x,y,z point.  One use of
	this function is used for sin-wave floors.*/

insertsprite(short sectnum, short statnum);   //returns (short)spritenum;
	/*Whenever you insert a sprite, you must pass it the sector
	number, and a status number (statnum).  The status number can be any
	number from 0 to MAXSTATUS-1.  Insertsprite works like a memory
	allocation function and returns the sprite number.*/

deletesprite(short spritenum);
	//Deletes the sprite.

changespritesect(short spritenum, short newsectnum);
	/*Changes the sector of sprite (spritenum) to the
	newsector (newsectnum).  This function may become
	internal to the engine in the movesprite function.  But
	this function is necessary since all the sectors have
	their own doubly-linked lists of sprites.*/

changespritestat(short spritenum, short newstatnum);
	/*Changes the status of sprite (spritenum) to status
	(newstatus).  Newstatus can be any number from 0 to MAXSTATUS-1.
	You can use this function to put a monster on a list of active sprites
	when it first sees you.*/

setsprite(short spritenum, long newx, long newy, long newz);
		/*This function simply sets the sprite's position to a specified
	coordinate (newx, newy, newz) without any checking to see
	whether the position is valid or not.  You could directly
	modify the sprite[].x, sprite[].y, and sprite[].z values, but
	if you use my function, the sprite is guaranteed to be in the
	right sector.*/

initmultiplayers(char damultioption, char dacomrateoption, char dapriority);
	// The parameters are ignored - just pass 3 0's
uninitmultiplayers();    // Does nothing

sendpacket(long other, char *bufptr, long messleng);
	/* other - who to send the packet to
	bufptr - pointer to message to send
	messleng - length of message */
	
short getpacket (short *other, char *bufptr);
	/* returns the number of bytes of the packet received, 0 if no packet
	other - who the packet was received from
	bufptr - pointer to message that was received */

sendlogon();            // Does nothing
sendlogoff();
	/*Sends a packet to everyone else where the
	first byte is 255, and the
	second byte is myconnectindex*/

getoutputcirclesize();  //Does nothing - just a stub function, returns 0
setsocket(short newsocket); //Does nothing

flushpackets();
	//Clears all packet buffers

VBE_setPalette(long start, long num, char *palettebuffer);
VBE_getPalette(long start, long num, char *palettebuffer);
	/*Set (num) palette palette entries starting at (start)
	palette entries are in a 4-byte format in this order:
		0: Blue (0-63)
		1: Green (0-63)
		2: Red (0-63)
		3: Reserved*/

makepalookup(long palnum, char *remapbuf,
				 signed char r, signed char g, signed char b,
				 char dastat);
	/*This function allows different shirt colors for sprites.  First prepare
	remapbuf, which is a 256 byte buffer of chars which the colors to remap.
	Palnum can be anywhere from 1-15.  Since 0 is where the normal palette is
	stored, it is a bad idea to call this function with palnum=0.
	In BUILD.H notice I added a new variable, spritepal[MAXSPRITES].
	Usually the value of this is 0 for the default palette.  But if you
	change it to the palnum in the code between drawrooms() and drawmasks
	then the sprite will be drawn with that remapped palette.  The last 3
	parameters are the color that the palette fades to as you get further
	away.  This color is normally black (0,0,0).  White would be (63,63,63).
	if ((dastat&1) == 0) then makepalookup will allocate & deallocate
	the memory block for use but will not waste the time creating a palookup
	table (assuming you will create one yourself)*/

setbrightness(char gammalevel, char *dapal);
	/*Use this function to adjust for gamma correction.
	Gammalevel - ranges from 0-15, 0 is darkest, 15 brightest. Default: 0
	dapal: standard VGA palette (768 bytes)*/

void setviewback(void);
void preparemirror(long dax, long day, long daz, short daang, long dahoriz, short dawall, short dasector, long *tposx, long *tposy, short *tang);
void completemirror(void);
