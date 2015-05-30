#include <stdbool.h>
#define MSGSHELL_T		'T'
#define MSGSHELL_W		'W'
#define MSGSHELL_D		'D'
#define MSGSHELL_I		'I'
#define MSGSHELL_Z		'Z'
#define MSGSHELL_X		'X'
#define MSGSHELL_H		'H'

#define MSG_SHELL		0
#define MSG_USR0		1
#define MAX_MSG			2

volatile char msglock;
int msgQ[MAX_MSG];
void setmsg(int val, int idx, bool bspin);
int getmsg(int idx, bool bspin);
