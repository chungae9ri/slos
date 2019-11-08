#include <stdint.h>
#include <slos_error.h>
#include <iostream.h>
#include <regops.h>
#include <mem_layout.h>

int32_t init_io_device(void)
{
	/* out device initialization */
	return ERR_NO;
}

int32_t start_ostream(void)
{
	uint32_t ctrl;

	ctrl = readl(OSTREAM_REG_CTRL);
	ctrl |= CTRL_GBL_START_MASK;
	writel(ctrl, OSTREAM_REG_CTRL);
	return ERR_NO;
}

int32_t stop_ostream(void)
{
	uint32_t ctrl;

	ctrl = readl(OSTREAM_REG_CTRL);
	ctrl &= ~CTRL_GBL_START_MASK;
	writel(ctrl, OSTREAM_REG_CTRL);
	return ERR_NO;
}

int32_t put_to_itab(uint32_t sAddr, uint32_t sLen)
{
	uint32_t ctrl, status;

	status = readl(OSTREAM_REG_STATUS);
	/* ITAB is full, return error */
	if (status & STAT_ITAB_FULL_MASK)
		return ERR_ITAB_FULL;

	writel(sAddr, OSTREAM_REG_ADDR);
	writel(sLen, OSTREAM_REG_LEN);

	ctrl = readl(OSTREAM_REG_CTRL);
	ctrl |= CTRL_IN_TRANS_MASK;
	writel(ctrl, OSTREAM_REG_CTRL);
	
	/* spin forever until ostream core saves the data to Itab entry */
	while (!(readl(OSTREAM_REG_STATUS) & STAT_TRANSFER_DONE_MASK)) {
		ctrl = readl(OSTREAM_REG_CTRL);
		/* if stop ostream, then exit */
		if (!(ctrl & CTRL_GBL_START_MASK))
			return ERR_NO;
	}

	/* clear the CTRL_IN_TRANS_MASK bit */
	ctrl = readl(OSTREAM_REG_CTRL);
	ctrl &= ~CTRL_IN_TRANS_MASK;
	writel(ctrl, OSTREAM_REG_CTRL);

	return ERR_NO;
}
