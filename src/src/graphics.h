extern int curplayscreen;
extern int borderdraw;
extern long screentilt;
extern char detailmode;
extern char titlepal[768];
extern int screensize, mapmode;
extern int zoom, brightness;

void initgraph(void);
void putnumber(long x, long y, int num);
void putsmallnumber(long x, long y, int num);
void putsmallnumberpal(long x, long y, int num, char pal);
void puttext(long x, long y, char *str);
void puttextpal(long x, long y, char *str, char pal);
void drawslider(long x, long y, int value, int max);
void drawbackground(void);
void setviewsize(void);
void renderscreen(void);
