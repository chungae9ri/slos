#include <frame_pool.h>

void init_framepool(struct framepool *pframe,
		     unsigned long _base_frame_no, 
		     unsigned long _nframes, 
		     unsigned long _info_frame_no)
{
	int i;
	volatile char *pBitmapEntry;

	pframe->nFrames = _nframes;
	pframe->baseFrameNo = _base_frame_no;

	if(_info_frame_no == 0) {
		/* If _info_frame_no is 0, then base frame of the pool
		 * becomes the info frame which is a Bitmap
		 */
		pframe->pBitmap = (volatile char *)FRAMETOPHYADDR(_base_frame_no);
	} else {
		pframe->pBitmap = (volatile char *)FRAMETOPHYADDR(_info_frame_no);
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
	for(i=0 ; i<pframe->nBitmapEntry ; i++) {
		*pBitmapEntry++ = 0x00;
	}

	/* initialize(clear) the remainder bits */
	if(pframe->nRemainderBitmapEntry) {
		for(i=0 ; i<pframe->nRemainderBitmapEntry ; i++)
			*pBitmapEntry &= ~(0x1 << i);
	}

	if(_info_frame_no == 0) {
		/* first frame of kernel mem pool is 
		 * already allocated for Bitmap
		 */
		*(pframe->pBitmap) = 0x01; 
	}
}

int get_frame(struct framepool *pframe)
{
	int i, j;
	int frameIdx = 0;
	volatile char *pBitmapEntry;

	pBitmapEntry = pframe->pBitmap;
	/* find out the first free frame in Bitmap*/
	for(i=0 ; i<pframe->nBitmapEntry ; i++) {
		/* going through the bits in pBtmapEntry.
		 * char has 8 bits 
		 */
		for(j=0 ; j<8 ; j++) {
			if((*pBitmapEntry & (0x1 << j)) == 0) {
				frameIdx = pframe->baseFrameNo + i * 8 + j;
				/* set the bit for the corresponding frame */
				*pBitmapEntry |= (0x1 << j);
				return frameIdx;
			}
		}
		pBitmapEntry++;
	}
	/* check if there is a free frame in the remainders */
	for(i=0 ; i<pframe->nRemainderBitmapEntry ; i++) {
		if((*pBitmapEntry & (0x1 << i)) == 0) {
			frameIdx = pframe->baseFrameNo + pframe->nBitmapEntry * 8 + i;
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
	pBitmapEntry = pframe->pBitmap + (int)((pframe->inacc_baseFrameNo-pframe->baseFrameNo)/8);

	/* make inaccessible region as already allocated */
	for(i=0 ; i<(int)(pframe->inacc_nFrames/8) ; i++) {
		*pBitmapEntry++ = 0xFF;
	}
}

void release_frame(struct framepool *pframe,
		   unsigned long _frame_no)
{
	unsigned long entryOffset;
	char remainder;
	volatile char *pBitmapEntry;

	entryOffset = (unsigned long)((_frame_no - pframe->baseFrameNo) / 8);
	remainder = _frame_no % 8;
	/* frame for kernel Bitmap(meta data) should not be freed. */
	if((unsigned long)pframe->pBitmap == FRAMETOPHYADDR(_frame_no))
		return;
	/* inaccessible region should not be freed */
	if(_frame_no >= pframe->inacc_baseFrameNo && _frame_no < pframe->inacc_baseFrameNo + pframe->inacc_nFrames)
		return;
	/* region out of bound should not be freed */
	if(_frame_no >= pframe->baseFrameNo + pframe->nFrames || _frame_no < pframe->baseFrameNo)
		return;

	pBitmapEntry = pframe->pBitmap;

	pBitmapEntry += entryOffset;

	/* clear bit corresponding to _frame_no */
	*pBitmapEntry &= ~(0x1 << remainder);
}
