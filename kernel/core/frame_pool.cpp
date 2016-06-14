#include "frame_pool.h"

/* constructor of FramePool */
FramePool::FramePool(unsigned long _base_frame_no, 
		     unsigned long _nframes, 
		     unsigned long _info_frame_no)
{
	int i;
	volatile char *pBitmapEntry;

	nFrames = _nframes;
	baseFrameNo = _base_frame_no;

	if(_info_frame_no == 0) {
		/* If _info_frame_no is 0, then base frame of the pool
		 * becomes the info frame which is a Bitmap
		 */
		pBitmap = (volatile char *)FRAMETOPHYADDR(_base_frame_no);
	} else {
		pBitmap = (volatile char *)FRAMETOPHYADDR(_info_frame_no);
	}

	nBitmapEntry = (int)(nFrames / 8);
	/* if the Bitmap size is not multiply of 8,
	 * then we need a remainder bits
	 */
	nRemainderBitmapEntry = nFrames % 8;

	/* Initialize the Bitmap with 0, unallocated.
	 * Each BitmapEntry size is 8bits
	 */
	pBitmapEntry = pBitmap;
	for(i=0 ; i<nBitmapEntry ; i++) {
		*pBitmapEntry++ = 0x00;
	}

	/* initialize(clear) the remainder bits */
	if(nRemainderBitmapEntry) {
		for(i=0 ; i<nRemainderBitmapEntry ; i++)
			*pBitmapEntry &= ~(0x1 << i);
	}

	if(_info_frame_no == 0) {
		/* first frame of kernel mem pool is 
		 * already allocated for Bitmap
		 */
		*pBitmap = 0x01; 
	}
}

unsigned long FramePool::get_frame()
{
	int i, j;
	unsigned long frameIdx = 0;
	volatile char *pBitmapEntry;

	pBitmapEntry = pBitmap;
	/* find out the first free frame in Bitmap*/
	for(i=0 ; i<nBitmapEntry ; i++) {
		/* going through the bits in pBtmapEntry.
		 * char has 8 bits 
		 */
		for(j=0 ; j<8 ; j++) {
			if((*pBitmapEntry & (0x1 << j)) == 0) {
				frameIdx = baseFrameNo + i * 8 + j;
				/* set the bit for the corresponding frame */
				*pBitmapEntry |= (0x1 << j);
				return frameIdx;
			}
		}
		pBitmapEntry++;
	}
	/* check if there is a free frame in the remainders */
	for(i=0 ; i<nRemainderBitmapEntry ; i++) {
		if((*pBitmapEntry & (0x1 << i)) == 0) {
			frameIdx = baseFrameNo + nBitmapEntry * 8 + i;
			/* set the Bitmap for the frame */
			*pBitmapEntry |= (0x1 << i);
			return frameIdx;
		}
	}
	/* there is no free frame */
	return 0;
}

void FramePool::mark_inaccessible(unsigned long _base_frame_no, 
				  unsigned long _nframes)
{
	volatile char *pBitmapEntry;
	int i;

	/* mark the inaccessilbe region */
	inacc_baseFrameNo = _base_frame_no;
	inacc_nFrames = _nframes;
	pBitmapEntry = pBitmap + (int)((inacc_baseFrameNo-baseFrameNo)/8);

	/* make inaccessible region as already allocated */
	for(i=0 ; i<(int)(inacc_nFrames/8) ; i++) {
		*pBitmapEntry++ = 0xFF;
	}
}

void FramePool::release_frame(unsigned long _frame_no)
{
	unsigned long entryOffset;
	char remainder;
	volatile char *pBitmapEntry;

	entryOffset = (unsigned long)((_frame_no - baseFrameNo) / 8);
	remainder = _frame_no % 8;
	/* frame for kernel Bitmap(meta data) should not be freed. */
	if((unsigned long)pBitmap == FRAMETOPHYADDR(_frame_no))
		return;
	/* inaccessible region should not be freed */
	if(_frame_no >= inacc_baseFrameNo && _frame_no < inacc_baseFrameNo + inacc_nFrames)
		return;
	/* region out of bound should not be freed */
	if(_frame_no >= baseFrameNo + nFrames || _frame_no < baseFrameNo)
		return;

	pBitmapEntry = pBitmap;

	pBitmapEntry += entryOffset;

	/* clear bit corresponding to _frame_no */
	*pBitmapEntry &= ~(0x1 << remainder);
}
