/*
 * File: frame_pool.H
 */

#ifndef _FRAME_POOL_H_                   // include file only once
#define _FRAME_POOL_H_

/* ARM small page(4KB) frame is used
 * convert frame number to 32bit physical memory address 
 */

#define GB * (0x1 << 30)
#define MB * (0x1 << 20)
#define KB * (0x1 << 10)
#define KERNEL_POOL_START_FRAME ((32 KB) / (4 KB))
#define KERNEL_POOL_FRAME_NUM ((4 MB) / (4 KB)) - KERNEL_POOL_START_FRAME
#define PROCESS_POOL_START_FRAME ((16 MB) / (4 KB))
#define PROCESS_POOL_FRAME_NUM ((128 MB) / (4 KB)) - PROCESS_POOL_START_FRAME

#define FRAMETOPHYADDR(X) ((unsigned long)((X * 4 * (0x1<<10))))

class FramePool {
	private:
		/* -- DEFINE YOUR FRAME POOL DATA STRUCTURE(s) HERE. */
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
	public:

		FramePool(unsigned long _base_frame_no,
				unsigned long _nframes,
				unsigned long _info_frame_no);
		/* Initializes the data structures needed for the management of this
		   frame pool. This function must be called before the paging system
		   is initialized.
		   _base_frame_no is the frame number at the start of the physical memory
		   region that this frame pool manages.
		   _nframes is the number of frames in the physical memory region that this
		   frame pool manages.
		   e.g. If _base_frame_no is 16 and _nframes is 4, this frame pool manages
		   physical frames numbered 16, 17, 18 and 19
		   _info_frame_no is the frame number (within the directly mapped region) of
		   the frame that should be used to store the management information of the
		   frame pool. However, if _info_frame_no is 0, the frame pool is free to
		   choose any frame from the pool to store management information.
		 */

		unsigned long get_frame();
		/* Allocates a frame from the frame pool. If successful, returns the frame
		 * number of the frame. If fails, returns 0. */

		void mark_inaccessible(unsigned long _base_frame_no,
				unsigned long _nframes);
		/* Mark the area of physical memory as inaccessible. The arguments have the
		 * same semanticas as in the constructor.
		 */

		void release_frame(unsigned long _frame_no);
		/* Releases frame back to the given frame pool.
		   The frame is identified by the frame number. 
NOTE: This function is static because there may be more than one frame pool
defined in the system, and it is unclear which one this frame belongs to.
This function must first identify the correct frame pool and then call the frame
pool's release_frame function. */
};
#endif
