#include <msgQ.h>

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
