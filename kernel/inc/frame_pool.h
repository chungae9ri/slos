/*
 * File: frame_pool.h
 */

#ifndef _FRAME_POOL_H_                   // include file only once
#define _FRAME_POOL_H_

/* ARM small page(4KB) frame is used
 * convert frame number to 32bit physical memory address 
 */

#define FRAMETOPHYADDR(X) ((unsigned long)((X * 4 * (0x1 << 10))))

struct framepool {
		/* base frame number(index) */
		unsigned long base_frame_idx;
		/* number of frames belonging to this frame pool */
		unsigned long nFrames;
		/* base frame number(index) of inaccessible memory */
		unsigned long inacc_baseFrameNo;
		/* number of frames belonging to inaccessible memory */
		unsigned long inacc_nFrames;
		/* start address of bit map that is a info frame of pool 
		 * pointer must be volatile to point to memory not cache
		 */
		volatile char *pBitmap;
		/* number of entry(size of 8bit) to bit map(info frame) */
		unsigned int nBitmapEntry;
		/* if nFrames is not divied by 8, then we need to 
		 * consider the remainders for these frames
		 */
		unsigned int nRemainderBitmapEntry;
};

void init_framepool(struct framepool *pframe,
		    unsigned long _base_frame_no,
		    unsigned long _nframes,
		    unsigned long _info_frame_no);
/* Allocates a frame from the frame pool. 
 * If successful, returns the frame
 * number of the frame. If fails, returns 0. 
 */
int  get_frame(struct framepool *pframe);

/* Mark the area of physical memory as inaccessible. */
void mark_inaccessible(struct framepool *pframe,
		       unsigned long _base_frame_no,
		       unsigned long _nframes);

int release_frame(struct framepool *pframe,
		   unsigned long _frame_no);
#endif
