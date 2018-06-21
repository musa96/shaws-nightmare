#define TICRATE (120)
#define TICSPERFRAME 3

extern volatile long tick;

void inittimer(void);
void bdelay(int time);
void uninittimer(void);
