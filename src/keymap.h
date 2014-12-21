#include "LGKeymap.h"

#define MELE_KEY_OK 65293
#define MELE_KEY_UP 65362
#define MELE_KEY_DOWN 65364
#define MELE_KEY_LEFT 65361
#define MELE_KEY_RIGHT 65363
#define MELE_KEY_HOME 65360
#define MELE_KEY_MENU 269025125
#define MELE_KEY_VOL_UP 269025043
#define MELE_KEY_VOL_DOWN 269025041
#define MELE_KEY_FORWARD 269025047
#define MELE_KEY_BACK 269025046
#define MELE_KEY_ESC 65307

#define RIGHT_MOUSE 1
#define LEFT_MOUSE 3

int getTvKey(int);
int getHTPCMouseKey(int key);
bool isKeyWithOnlyRelease(int key);

