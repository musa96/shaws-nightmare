typedef enum
{
	Activate,
	Run,
	Fire,
	LookUp,
	LookCentre,
	LookDown,
	FireWeapon,
	Jump,
	Crouch,
	Weapon1,
	Weapon2,
	Weapon3,
	Weapon4,
	Weapon5,
	Weapon6,
	Weapon7,

	NumControls
} Controls;

typedef struct
{
	long vel, svel, angvel;
	short bits[NumControls];
} INPUT;

extern short oflags[NumControls];
extern short mousesensitivity, aim_mode;
extern INPUT sync;

void getinput(void);
