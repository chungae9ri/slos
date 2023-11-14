/*
  kernel/inc/frame_pool.h 
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

#ifndef _FRAME_POOL_H_                   // include file only once
#define _FRAME_POOL_H_

#include <stdint.h>

/* ARM small page(4KB) frame is used
 * convert frame number to 32bit physical memory address 
 */

#define FRAMETOPHYADDR(X) ((uint32_t)((X * 4 * (0x1 << 10))))

struct framepool {
		/* base frame number(index) */
		uint32_t base_frame_idx;
		/* number of frames belonging to this frame pool */
		uint32_t nFrames;
		/* base frame number(index) of inaccessible memory */
		uint32_t inacc_baseFrameNo;
		/* number of frames belonging to inaccessible memory */
		uint32_t inacc_nFrames;
		/* start address of bitmap */
		uint8_t *pBitmap;
		/* number of entry(size of 8bit) to bit map(info frame) */
		uint32_t nBitmapEntry;
		/* if nFrames is not divied by 8, then we need to 
		 * consider the remainders for these frames
		 */
		uint32_t nRemainderBitmapEntry;
};

void init_framepool(struct framepool *pframe,
		    uint32_t _base_frame_no,
		    uint32_t _nframes);
/* Allocates a frame from the frame pool. 
 * If successful, returns the frame
 * number of the frame. If fails, returns 0. 
 */
uint32_t get_frame(struct framepool *pframe);

void mark_prealloc_frame(struct framepool *pframe,
		       uint32_t _base_frame_no, 
		       uint32_t _nframes);

uint32_t release_frame(struct framepool *pframe,
		   uint32_t _frame_no);
#endif
