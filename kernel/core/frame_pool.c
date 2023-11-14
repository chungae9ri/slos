/*
  kernel/core/frame_pool.c frame pool manager
  (C) 2018 Kwangdo Yi <kwangdo.yi@gmail.com>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>
*/

#include <frame_pool.h>
#include <mem_layout.h>

/* one bitmap frame(4KB) can address 4K * 8 * 4KB = 128MB memory
 * one bitmap frame is enough for kernel
 * All frames are preallocated except heap
 */
void init_framepool(struct framepool *pframe, uint32_t base_frame_idx, uint32_t frame_num)
{
	uint32_t free_frame_start;
	uint32_t i;
	uint32_t free_frame_num;
	uint8_t *pBitmapEntry;

	pframe->base_frame_idx = base_frame_idx;
	pframe->nFrames = frame_num;
	pframe->pBitmap = (uint8_t *)(KERNEL_FRAME_BITMAP);
	pframe->nBitmapEntry = (uint32_t)(pframe->nFrames / 8);
	/* if the Bitmap size is not multiply of 8,
	 * then we need a remainder bits
	 */
	pframe->nRemainderBitmapEntry = pframe->nFrames % 8;

	/* Initialize the Bitmap with 0, unallocated.
	 * Each BitmapEntry size is 8bits
	 */
	pBitmapEntry = pframe->pBitmap;

	/* bitmap size is 4KB. 
	 * initialize it with 0xFF, which means
	 * all preallocated except heap. Heap frames
	 * will be set to 0x00.
	 */
	for (i = 0; i < 4096; i++)
		pBitmapEntry[i] = 0xFF;

	/* initialize the heap frames with
	 * with 1'b0 for free frames
	 */
	free_frame_start = HEAP_FRAME_START >> 3;
	free_frame_num = HEAP_FRAME_NUM >> 3;

	for (i = free_frame_start; i < free_frame_num; i++) {
		pframe->pBitmap[i] = 0x00;
	}
}

uint32_t get_frame(struct framepool *pframe)
{
	uint32_t i, j;
	uint32_t frameIdx = 0;
	volatile uint8_t *pBitmapEntry;

	pBitmapEntry = pframe->pBitmap;
	/* find out the first free frame in Bitmap*/
	for (i = 0; i < pframe->nBitmapEntry; i++) {
		/* going through the bits in pBtmapEntry.
		 * char has 8 bits 
		 */
		if (*pBitmapEntry == 0xFF) {
			pBitmapEntry++;
			continue;
		}
		for (j = 0; j < 8; j++) {
			if ((*pBitmapEntry & (0x1 << j)) == 0) {
				frameIdx = pframe->base_frame_idx + i * 8 + j;
				/* set the bit for the corresponding frame */
				*pBitmapEntry |= (0x1 << j);
				return frameIdx;
			}
		}
		pBitmapEntry++;
	}

	/* check if there is a free frame in the remainders */
	for (i = 0; i < pframe->nRemainderBitmapEntry; i++) {
		if((*pBitmapEntry & (0x1 << i)) == 0) {
			frameIdx = pframe->base_frame_idx + pframe->nBitmapEntry * 8 + i;
			/* set the Bitmap for the frame */
			*pBitmapEntry |= (0x1 << i);
			return frameIdx;
		}
	}
	/* there is no free frame */
	return -1;
}

void mark_prealloc_frame(struct framepool *pframe,
		       uint32_t _base_frame_no, 
		       uint32_t _nframes)
{
	volatile uint8_t *pBitmapEntry;
	uint32_t i;

	/* mark the inaccessilbe region */
	pframe->inacc_baseFrameNo = _base_frame_no;
	pframe->inacc_nFrames = _nframes;
	pBitmapEntry = (uint8_t *)(pframe->pBitmap + _base_frame_no / 8);

	/* make inaccessible region as already allocated */
	for (i = 0; i < (uint32_t)(_nframes / 8); i++) {
		*pBitmapEntry++ = 0xFF;
	}

	if ((i = _nframes % 8) != 0) {
		i--;
		while (i >= 0) {
			*pBitmapEntry |= (0x1 << i);
			i--;
		}
	}
}

uint32_t release_frame(struct framepool *pframe, uint32_t _frame_no)
{
	uint32_t entryOffset;
	uint8_t remainder;
	volatile uint8_t *pBitmapEntry;

	entryOffset = (uint32_t)((_frame_no - pframe->base_frame_idx) / 8);
	remainder = _frame_no % 8;

	/* inaccessible region should not be freed */
	if (_frame_no >= pframe->inacc_baseFrameNo&& 
  	    _frame_no < pframe->inacc_baseFrameNo + pframe->inacc_nFrames)
		return 1;

	/* region out of bound should not be freed */
	if (_frame_no >= pframe->base_frame_idx + pframe->nFrames || 
	    _frame_no < pframe->base_frame_idx)
		return 2;

	pBitmapEntry = pframe->pBitmap;
	pBitmapEntry += entryOffset;

	/* clear bit corresponding to _frame_no */
	*pBitmapEntry &= ~(0x1 << remainder);

	return 0;
}
