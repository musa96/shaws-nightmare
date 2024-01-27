#define CLIPMASK_MONST ((1L<<(16+10))+((1L)<<16)+(1L<<10)+1L)

typedef enum
{
	AIWalk,
	AIAttack,
	AIPain,
	AIDeath
} AIState;

int findplayers(long i);
void process_enemies(void);
