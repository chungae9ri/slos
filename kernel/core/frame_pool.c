// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

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

#include <stdint.h>
#include <error.h>
#include <frame_pool.h>
#include <mem_layout.h>

/* One bitmap frame(4KB) can address 4K * 8 * 4KB = 128MB memory.
 * One bitmap frame is enough for kernel.
 */
void init_framepool(struct framepool *pframe, uint32_t base_frame_idx, uint32_t frame_num,
		    uint32_t bitmap_frame_idx)
{
	uint8_t prealloc_mask;
	int32_t i, j;
	uint32_t prealloc_num;
	uint8_t *pBitmapEntry;

	pframe->base_frame_idx = base_frame_idx;
	pframe->nFrames = frame_num;

	if (bitmap_frame_idx == 0) {
		pframe->pBitmap = (uint8_t *)(KERNEL_FRAME_BITMAP);
	} else {
		pframe->pBitmap = (uint8_t *)FRAMETOPHYADDR(bitmap_frame_idx);
	}

	pframe->nBitmapEntry = (uint32_t)(pframe->nFrames / 8);
	/* If the Bitmap size is not multiply of 8,
	 * then we need a remainder bits.
	 */
	pframe->nRemainderBitmapEntry = pframe->nFrames % 8;

	/* Initialize the Bitmap with 0, unallocated.
	 * Each BitmapEntry size is 8bits.
	 */
	pBitmapEntry = pframe->pBitmap;
	/* Bitmap size is 4KB.
	 * Initialize it with 0x00.
	 */
	for (i = 0; i < 4096; i++) {
		pBitmapEntry[i] = 0;
	}

	if (bitmap_frame_idx == 0) {
		/* Initialize the kernel frames with
		 * preallocated memory frames.
		 * Mark it with 1'b0 if it is prealloced.
		 */
		prealloc_num = PREALLOC_FRAME_NUM;

		for (i = 0; i < prealloc_num / 8; i++) {
			pframe->pBitmap[i] = 0xFF;
		}

		prealloc_num %= 8;
		prealloc_mask = 0x00;
		for (j = 0; j < prealloc_num; j++) {
			prealloc_mask |= (0x1 << j);
		}

		pframe->pBitmap[i] |= prealloc_mask;
	}
}

int32_t get_frame(struct framepool *pframe)
{
	int32_t i, j;
	int32_t frameIdx = 0;
	uint8_t *pBitmapEntry;

	pBitmapEntry = pframe->pBitmap;
	/* Find out the first free frame in Bitmap*/
	for (i = 0; i < pframe->nBitmapEntry; i++) {
		/* Going through the bits in pBtmapEntry.
		 * Char has 8 bits
		 */
		if (*pBitmapEntry == 0xFF) {
			pBitmapEntry++;
			continue;
		}
		for (j = 0; j < 8; j++) {
			if ((*pBitmapEntry & (0x1 << j)) == 0) {
				frameIdx = pframe->base_frame_idx + i * 8 + j;
				/* Set the bit for the corresponding frame */
				*pBitmapEntry |= (0x1 << j);
				return frameIdx;
			}
		}
		pBitmapEntry++;
	}
	/* Check if there is a free frame in the remainders */
	for (i = 0; i < pframe->nRemainderBitmapEntry; i++) {
		if ((*pBitmapEntry & (0x1 << i)) == 0) {
			frameIdx = pframe->base_frame_idx + pframe->nBitmapEntry * 8 + i;
			/* Set the Bitmap for the frame */
			*pBitmapEntry |= (0x1 << i);
			return frameIdx;
		}
	}

	/* There is no free frame */
	return -1;
}

void mark_prealloc_frame(struct framepool *pframe, uint32_t _base_frame_no, uint32_t _nframes)
{
	uint8_t *pBitmapEntry;
	int32_t i;

	/* Mark the inaccessilbe region */
	pframe->inacc_baseFrameNo = _base_frame_no;
	pframe->inacc_nFrames = _nframes;
	pBitmapEntry = (uint8_t *)(pframe->pBitmap + _base_frame_no / 8);

	/* Make inaccessible region as already allocated */
	for (i = 0; i < (int32_t)(_nframes / 8); i++) {
		*pBitmapEntry++ = 0xFF;
	}

	i = _nframes % 8;
	if (i != 0) {
		i--;
		while (i >= 0) {
			*pBitmapEntry |= (0x1 << i);
			i--;
		}
	}
}

int32_t release_frame(struct framepool *pframe, unsigned long _frame_no)
{
	uint32_t entryOffset;
	uint8_t remainder;
	uint8_t *pBitmapEntry;

	entryOffset = (uint32_t)((_frame_no - pframe->base_frame_idx) / 8);
	remainder = _frame_no % 8;

	/* Inaccessible region should not be freed */
	if (_frame_no >= pframe->inacc_baseFrameNo &&
	    _frame_no < pframe->inacc_baseFrameNo + pframe->inacc_nFrames)
		return FRAMEPOOL_FREE_ERR;

	/* Region out of bound should not be freed */
	if (_frame_no >= pframe->base_frame_idx + pframe->nFrames ||
	    _frame_no < pframe->base_frame_idx)
		return FRAMEPOOL_FREE_ERR;

	pBitmapEntry = pframe->pBitmap;
	pBitmapEntry += entryOffset;

	/* Clear bit corresponding to _frame_no */
	*pBitmapEntry &= ~(0x1 << remainder);

	return 0;
}

/**
 * @}
 * @}
 * @}
 *
 */
