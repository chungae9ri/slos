#include <stdint.h>
#include <slos_error.h>
#include <odev.h>
#include <regops.h>
#include <mem_layout.h>

int32_t init_odev(void)
{
	/* out device initialization */
	return ERR_NO;
}

int32_t start_odev(void)
{
	uint32_t ctrl;

	ctrl = readl(ODEV_REG_CTRL);
	ctrl |= CTRL_GBL_START_MASK;
	writel(ctrl, ODEV_REG_CTRL);

	return ERR_NO;
}

int32_t start_odev_stream(void)
{
	uint32_t ctrl;

	ctrl = readl(ODEV_REG_CTRL);
	ctrl |= CTRL_OSTREAM_START_MASK;
	writel(ctrl, ODEV_REG_CTRL);

	return ERR_NO;
}

int32_t stop_odev(void)
{
	uint32_t ctrl;

	ctrl = readl(ODEV_REG_CTRL);
	ctrl &= ~CTRL_GBL_START_MASK;
	writel(ctrl, ODEV_REG_CTRL);
	return ERR_NO;
}

int32_t stop_odev_stream(void)
{
	uint32_t ctrl;

	ctrl = readl(ODEV_REG_CTRL);
	ctrl &= ~CTRL_OSTREAM_START_MASK;
	writel(ctrl, ODEV_REG_CTRL);

	return ERR_NO;
}

int32_t put_to_itab(uint32_t sAddr, uint32_t sLen)
{
	uint32_t ctrl, status;

	status = readl(ODEV_REG_STATUS);
	/* ITAB is full, return error */
	if (status & STAT_ITAB_FULL_MASK)
		return ERR_ITAB_FULL;

	writel(sAddr, ODEV_REG_ADDR);
	writel(sLen, ODEV_REG_LEN);

	ctrl = readl(ODEV_REG_CTRL);
	ctrl |= CTRL_IN_TRANS_MASK;
	writel(ctrl, ODEV_REG_CTRL);
	
	/* spin forever until ODEV core saves the data to Itab entry */
	while (!(readl(ODEV_REG_STATUS) & STAT_TRANSFER_DONE_MASK)) {
		ctrl = readl(ODEV_REG_CTRL);
		/* if stop ODEV, then exit */
		if (!(ctrl & CTRL_GBL_START_MASK))
			return ERR_NO;
	}

	/* clear the CTRL_IN_TRANS_MASK bit */
	ctrl = readl(ODEV_REG_CTRL);
	ctrl &= ~CTRL_IN_TRANS_MASK;
	writel(ctrl, ODEV_REG_CTRL);

	return ERR_NO;
}
