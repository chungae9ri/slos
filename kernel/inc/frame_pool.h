/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_core Core
 * @{
 * @addtogroup kernel_core_mm Memory management
 * @{
 *
 * @file
 * @brief Memory frame pool management functions
 *
 */

#ifndef _FRAME_POOL_H_
#define _FRAME_POOL_H_

#include <stdint.h>

/** ARM small page(4KB) frame is used
 *  convert frame number to 32bit physical memory address
 */
#define FRAMETOPHYADDR(X) ((uint32_t)((X * 4 * (0x1 << 10))))

/** Framepool structure */
struct framepool {
	/** Base frame number(index), kernel memory start's frame number */
	uint32_t base_frame_idx;
	/** Number of frames belonging to this frame pool, kernel memory size */
	uint32_t nFrames;
	/** Base frame number(index) of inaccessible memory */
	uint32_t inacc_baseFrameNo;
	/** Number of frames belonging to inaccessible memory */
	uint32_t inacc_nFrames;
	/** Start address of bit map that is a info frame of pool */
	uint8_t *pBitmap;
	/** Number of entry(size of 8bit) to bit map(info frame) */
	uint32_t nBitmapEntry;
	/** Bitmap entry remainder */
	uint32_t nRemainderBitmapEntry;
};

/**
 * @brief Initialize memory frame pool
 *
 * One bitmap frame(4KB) can address 4K * 8 * 4KB = 128MB memory.
 * One bitmap frame is enough for kernel.
 *
 * @param [in] pframe Pointer to memory frame pool
 * @param [in] _base_frame_no Base frame number which is kernel memory start frame number
 * @param [in] _nframes  Frame number in the pool which is kernel memory frame number
 * @param [in] _info_frame_no Info frame (bitmap frame) number
 */
void init_framepool(struct framepool *pframe, uint32_t _base_frame_no, uint32_t _nframes,
		    uint32_t _info_frame_no);
/**
 * @brief Allocates a frame from the frame pool.
 *
 * @param [in] pframe Frame pool pointer
 * @return int32_t if successful, returns the frame number of the frame. If fails, returns -1.
 */
int32_t get_frame(struct framepool *pframe);

/**
 * @brief Mark allocated for the preallocated memory frames
 *
 * @param [in] pframe Frame pool pointer
 * @param [in] _base_frame_no Base frame number of the frame pool
 * @param [in] _nframes Frame number of frame pool
 */
void mark_prealloc_frame(struct framepool *pframe, uint32_t _base_frame_no, uint32_t _nframes);

/**
 * @brief Release frame from frame pool
 *
 * @param [in] pframe Frame pool pointer
 * @param [in] _frame_no Frame number of the frame pool
 *
 * @return int32_t 0 for success, others for failure
 */
int32_t release_frame(struct framepool *pframe, uint32_t _frame_no);
#endif

/**
 * @}
 * @}
 * @}
 *
 */
