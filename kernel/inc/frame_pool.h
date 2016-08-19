/*
 * File: frame_pool.h
 */

#ifndef _FRAME_POOL_H_                   // include file only once
#define _FRAME_POOL_H_

/* ARM small page(4KB) frame is used
 * convert frame number to 32bit physical memory address 
 */

#define GB * (0x1 << 30)
#define MB * (0x1 << 20)
#define KB * (0x1 << 10)
#define KERNEL_START_FRAME ((0 KB) / (4 KB))
#define KERNEL_FRAME_NUM ((16 MB) / (4 KB)) 
#define KERNEL_INACC_FRAME ((0 KB) / (4 KB))
#define KERNEL_INACC_FRAME_NUM ((8 MB) / (4 KB))
#define KERNEL_HEAP_START_FRAME ((8 MB) / (4 KB))
#define KERNEL_HEAP_FRAME_NUM ((12 MB) / (4 KB))
#define PROCESS_HEAP_START_FRAME ((16 MB) / (4 KB))
#define PROCESS_HEAP_FRAME_NUM ((128 MB) / (4 KB)) - PROCESS_HEAP_START_FRAME

#define FRAMETOPHYADDR(X) ((unsigned long)((X * 4 * (0x1<<10))))

struct framepool {
		/* base frame number(index) */
		unsigned long baseFrameNo;
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

/* Mark the area of physical memory as inaccessible. The arguments have the
 * same semanticas as in the constructor.
 */
void mark_inaccessible(struct framepool *pframe,
		       unsigned long _base_frame_no,
		       unsigned long _nframes);

void release_frame(struct framepool *pframe, unsigned long _frame_no);
#endif
