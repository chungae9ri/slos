#include <stdint-gcc.h>
#include <frame_pool.h>
#include <mem_layout.h>

/* 
 * one bitmap frame(4KB) can address 4K * 8 * 4KB = 128MB memory
 * one bitmap is enough for kernel
 */
void init_framepool(struct framepool *pframe,
		unsigned long base_frame_idx,
		unsigned long frame_num,
		unsigned long bitmap_frame_idx)
{
	unsigned char prealloc_mask;
	int i, j;
	unsigned long prealloc_num;
	volatile char *pBitmapEntry;

	pframe->base_frame_idx = base_frame_idx;
	pframe->nFrames = frame_num;

	if (bitmap_frame_idx == 0) {
		pframe->pBitmap = (volatile char *)(KERNEL_FRAME_BITMAP);
	} else {
		pframe->pBitmap = (volatile char *)FRAMETOPHYADDR(bitmap_frame_idx);
	}

	pframe->nBitmapEntry = (int)(pframe->nFrames / 8);
	/* if the Bitmap size is not multiply of 8,
	 * then we need a remainder bits
	 */
	pframe->nRemainderBitmapEntry = pframe->nFrames % 8;

	/* Initialize the Bitmap with 0, unallocated.
	 * Each BitmapEntry size is 8bits
	 */
	pBitmapEntry = pframe->pBitmap;
	/* bitmap size is 4KB. 
	 * initialize it with 0x00
	 */
	for (i = 0; i < 4096; i++) {
		pBitmapEntry[i] = 0;
	}

	/* initialize(clear) the remainder bits */
	if (pframe->nRemainderBitmapEntry) {
		for (i = 0; i < pframe->nRemainderBitmapEntry; i++) {
			*pBitmapEntry &= ~(0x1 << i);
		}
	}

	if (bitmap_frame_idx == 0) {
		/* 
		 * initialize the kernel frames with 
		 * preallocated memory frames.
		 * mark it with 1'b0 if it is prealloced.
		 */
		prealloc_num = ((KERN_PGD_START_BASE) / (4 KB));

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

int get_frame(struct framepool *pframe)
{
	int i, j;
	int frameIdx = 0;
	volatile char *pBitmapEntry;

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

void mark_inaccessible(struct framepool *pframe,
		       unsigned long _base_frame_no, 
		       unsigned long _nframes)
{
	volatile char *pBitmapEntry;
	int i;

	/* mark the inaccessilbe region */
	pframe->inacc_baseFrameNo = _base_frame_no;
	pframe->inacc_nFrames = _nframes;
	pBitmapEntry = (char *)(pframe->pBitmap + _base_frame_no / 8);

	/* make inaccessible region as already allocated */
	for (i = 0; i < (int)(_nframes / 8); i++) {
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

int release_frame(struct framepool *pframe,
		   unsigned long _frame_no)
{
	unsigned long entryOffset;
	char remainder;
	volatile char *pBitmapEntry;

	entryOffset = (unsigned long)((_frame_no - pframe->base_frame_idx) / 8);
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
