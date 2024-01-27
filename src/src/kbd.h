#define KEYFIFOSIZ 64

extern volatile char keystatus[256], keyfifo[KEYFIFOSIZ], keyfifoplc, keyfifoend;
extern volatile char readch, oldreadch, extended, keytemp;

void initkeys(void);
void uninitkeys(void);
char getkey(void);
