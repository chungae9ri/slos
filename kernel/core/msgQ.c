#include <msgQ.h>

extern void spin_lock_acquire(volatile char *msg);
extern void spin_lock_release(volatile char *msg);

void setmsg(int val, int idx, bool bspin)
{
	if(bspin) {
		spin_lock_acquire(&msglock);
		msgQ[idx] = val;
		spin_lock_release(&msglock);
	} else {
		msgQ[idx] = val;
	}
}

int getmsg(int idx, bool bspin)
{
	int val;
	if (bspin) {
		spin_lock_acquire(&msglock);
		val = msgQ[idx];
		spin_lock_release(&msglock);
	} else {
		val = msgQ[idx];

	}
	return val;
}
