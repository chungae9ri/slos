#include <clock.h>
#include <reg.h>
#include <iomap.h>

struct clk_freq_tbl rcg_dummy_freq = F_END;

/* Vote clock ops */
int clock_lib2_vote_clk_enable(struct clk *c)
{
	uint32_t vote_regval;
	uint32_t val;
	struct vote_clk *vclk = to_local_vote_clk(c);

	vote_regval = readl(vclk->vote_reg);
	vote_regval |= vclk->en_mask;
	writel(vote_regval, vclk->vote_reg);
	/* wait until status shows it is enabled */
	do {
		val = readl(vclk->cbcr_reg);
		val &= BRANCH_CHECK_MASK;
	}while((val != BRANCH_ON_VAL) && (val != BRANCH_NOC_FSM_ON_VAL));

	return 0;
}

void clock_lib2_vote_clk_disable(struct clk *c) 
{
	uint32_t vote_regval;
	struct vote_clk *vclk = to_local_vote_clk(c);

	vote_regval = readl(vclk->vote_reg);
	vote_regval &= ~vclk->en_mask;
	writel(vote_regval, vclk->vote_reg);

	/* wait until status shows it is disabled */
	while(!(readl(vclk->cbcr_reg) & CBCR_BRANCH_OFF_BIT));
}

static struct clk_ops clk_ops_vote = {
	.enable = clock_lib2_vote_clk_enable,
	.disable = clock_lib2_vote_clk_disable,
};

static struct vote_clk gcc_blsp1_ahb_clk = {
	.cbcr_reg = (uint32_t *) BLSP1_AHB_CBCR,
	.vote_reg = (uint32_t *) APCS_CLOCK_BRANCH_ENA_VOTE,
	.en_mask = BIT(17),
	.c = {
		.dbg_name = "gcc_blsp1_ahb_clk",
		.ops = &clk_ops_vote,
	},
};
/*
 * Root clock ops
 */

/* Root enable */
int clock_lib2_rcg_enable(struct clk *c)
{
	/* Hardware feedback from branch enable results in root being enabled.
	 * Nothing to do here.
	 */

	return 0;
}

int clock_lib2_rcg_set_rate(struct clk *c, unsigned rate)
{
	struct rcg_clk *rclk = to_rcg_clk(c);
	struct clk_freq_tbl *nf; /* new freq */
	int rc = 0;

	/* ck if new freq is in table */
	for (nf = rclk->freq_tbl; nf->freq_hz != FREQ_END
			&& nf->freq_hz != rate; nf++)
		;

	/* Frequency not found in the table */
	if (nf->freq_hz == FREQ_END)
		return -1;

	/* Check if frequency is actually changed. */
	if (nf == rclk->current_freq)
		return rc;

	/* First enable the source clock for this freq. */
	clk_enable(nf->src_clk);

	/* Perform clock-specific frequency switch operations. */
	/* ASSERT(rclk->set_rate); */
	rclk->set_rate(rclk, nf);

	/* update current freq */
	rclk->current_freq = nf;

	return rc;
}

static struct clk_ops clk_ops_rcg_mnd =
{
	.enable     = clock_lib2_rcg_enable,
	.set_rate   = clock_lib2_rcg_set_rate,
}; 

/* root update config: informs h/w to start using the new config values */
static void clock_lib2_rcg_update_config(struct rcg_clk *rclk)
{
	uint32_t cmd;

	cmd  = readl(rclk->cmd_reg);
	cmd |= CMD_UPDATE_BIT;
	writel(cmd, rclk->cmd_reg);

	/* Wait for frequency to be updated. */
	while(readl(rclk->cmd_reg) & CMD_UPDATE_MASK);
}

/* root set rate for clocks with half integer and MND divider */
void clock_lib2_rcg_set_rate_mnd(struct rcg_clk *rclk, struct clk_freq_tbl *freq)
{
	uint32_t cfg;

	/* Program MND values */
	writel(freq->m_val, rclk->m_reg);
	writel(freq->n_val, rclk->n_reg);
	writel(freq->d_val, rclk->d_reg);

	/* setup src select and divider */
	cfg  = readl(rclk->cfg_reg);
	cfg &= ~(CFG_SRC_SEL_MASK | CFG_SRC_DIV_MASK | CFG_MODE_MASK);
	cfg |= freq->div_src_val;
	if(freq->n_val !=0)
	{
		cfg |= (CFG_MODE_DUAL_EDGE << CFG_MODE_OFFSET);
	}
	writel(cfg, rclk->cfg_reg);

	/* Inform h/w to start using the new config. */
	clock_lib2_rcg_update_config(rclk);
}
/*
 */
/* branch clock ops */

/* Branch clock enable */
int clock_lib2_branch_clk_enable(struct clk *clk)
{
	int rc = 0;
	uint32_t cbcr_val;
	struct branch_clk *bclk = to_branch_clk(clk);

	cbcr_val  = readl(bclk->cbcr_reg);
	cbcr_val |= CBCR_BRANCH_ENABLE_BIT;
	writel(cbcr_val, bclk->cbcr_reg);

	/* wait until status shows it is enabled */
	while(readl(bclk->cbcr_reg) & CBCR_BRANCH_OFF_BIT);

	return rc;  
}       

/* Branch clock disable */
void clock_lib2_branch_clk_disable(struct clk *clk)
{
	uint32_t cbcr_val;
	struct branch_clk *bclk = to_branch_clk(clk);

	cbcr_val  = readl(bclk->cbcr_reg);
	cbcr_val &= ~CBCR_BRANCH_ENABLE_BIT;
	writel(cbcr_val, bclk->cbcr_reg);

	/* wait until status shows it is disabled */
	while(!(readl(bclk->cbcr_reg) & CBCR_BRANCH_OFF_BIT));
}

/* Branch clock set rate */
int clock_lib2_branch_set_rate(struct clk *c, unsigned rate)
{
	struct branch_clk *branch = to_branch_clk(c);

	if (!branch->has_sibling)
		return clk_set_rate(branch->parent, rate);

	return -1;
}


static struct clk_ops clk_ops_branch =
{
	.enable     = clock_lib2_branch_clk_enable,
	.disable    = clock_lib2_branch_clk_disable,
	.set_rate   = clock_lib2_branch_set_rate,
};
/*
 * Clock sources 
 */
int cxo_clk_enable(struct clk *clk)
{       
	/* Nothing to do. */
	return 0;
}

void cxo_clk_disable(struct clk *clk)
{
	/* Nothing to do. */
	return;
}

static struct clk_ops clk_ops_pll_vote =
{
	.enable     = pll_vote_clk_enable,
	.disable    = pll_vote_clk_disable,
	.auto_off   = pll_vote_clk_disable,
	.is_enabled = pll_vote_clk_is_enabled,
};

static struct clk_ops clk_ops_cxo =
{
	.enable     = cxo_clk_enable,
	.disable    = cxo_clk_disable,
};

static struct fixed_clk cxo_clk_src =
{
	.c = {
		.rate     = 19200000,
		.dbg_name = "cxo_clk_src",
		.ops      = &clk_ops_cxo,
	},
};

static struct pll_vote_clk gpll0_clk_src =
{
	.en_reg       = (void *) APCS_GPLL_ENA_VOTE,
	.en_mask      = BIT(0),
	.status_reg   = (void *) GPLL0_STATUS,
	.status_mask  = BIT(17),
	.parent       = &cxo_clk_src.c,

	.c = {
		.rate     = 600000000,
		.dbg_name = "gpll0_clk_src",
		.ops      = &clk_ops_pll_vote,
	},
};

/* UART Clocks */
static struct clk_freq_tbl ftbl_gcc_blsp1_2_uart1_6_apps_clk[] =
{
	F( 3686400,  gpll0,    1,  96,  15625),
	F( 7372800,  gpll0,    1, 192,  15625),
	F(14745600,  gpll0,    1, 384,  15625),
	F(16000000,  gpll0,    5,   2,     15),
	F(19200000,    cxo,    1,   0,      0), 
	F(24000000,  gpll0,    5,   1,      5), 
	F(32000000,  gpll0,    1,   4,     75),
	F(40000000,  gpll0,   15,   0,      0), 
	F(46400000,  gpll0,    1,  29,    375),
	F(48000000,  gpll0, 12.5,   0,      0), 
	F(51200000,  gpll0,    1,  32,    375),
	F(56000000,  gpll0,    1,   7,     75),
	F(58982400,  gpll0,    1, 1536, 15625),
	F(60000000,  gpll0,   10,   0,      0), 
	F_END
};

static struct rcg_clk blsp1_uart3_apps_clk_src =
{
	.cmd_reg      = (uint32_t *) BLSP1_UART3_APPS_CMD_RCGR,
	.cfg_reg      = (uint32_t *) BLSP1_UART3_APPS_CFG_RCGR,
	.m_reg        = (uint32_t *) BLSP1_UART3_APPS_M,
	.n_reg        = (uint32_t *) BLSP1_UART3_APPS_N,
	.d_reg        = (uint32_t *) BLSP1_UART3_APPS_D,

	.set_rate     = clock_lib2_rcg_set_rate_mnd,
	.freq_tbl     = ftbl_gcc_blsp1_2_uart1_6_apps_clk,
	.current_freq = &rcg_dummy_freq,

	.c = { 
		.dbg_name = "blsp1_uart3_apps_clk",
		.ops      = &clk_ops_rcg_mnd,
	},  
};

static struct branch_clk gcc_blsp1_uart3_apps_clk = {
	.cbcr_reg = (uint32_t *) BLSP1_UART3_APPS_CBCR,
	.parent = &blsp1_uart3_apps_clk_src.c,
	.c = {
		.dbg_name = "gcc_blsp1_uart3_apps_clk",
		.ops = &clk_ops_branch,
	},
};

/*
 * pll_vote_clk functions
 */
int pll_vote_clk_enable(struct clk *clk)
{
	uint32_t ena;
	struct pll_vote_clk *pll = to_pll_vote_clk(clk);

	ena = readl(pll->en_reg);
	ena |= pll->en_mask;
	writel(ena, pll->en_reg);

	/* Wait until PLL is enabled */
	while ((readl(pll->status_reg) & pll->status_mask) == 0);

	return 0;
}

void pll_vote_clk_disable(struct clk *clk)
{
	uint32_t ena;
	struct pll_vote_clk *pll = to_pll_vote_clk(clk);

	ena = readl(pll->en_reg);
	ena &= ~(pll->en_mask);
	writel(ena, pll->en_reg);
}

unsigned pll_vote_clk_get_rate(struct clk *clk)
{
	struct pll_vote_clk *pll = to_pll_vote_clk(clk);
	return pll->rate;
}

struct clk *pll_vote_clk_get_parent(struct clk *clk)
{
	struct pll_vote_clk *pll = to_pll_vote_clk(clk);
	return pll->parent;
}

int pll_vote_clk_is_enabled(struct clk *clk)
{
	struct pll_vote_clk *pll = to_pll_vote_clk(clk);
	return !!(readl(pll->status_reg) & pll->status_mask);
}

/*
 * PLLs functions
 */
int pll_clk_enable(struct clk *clk)
{
	uint32_t mode;
	struct pll_clk *pll = to_pll_clk(clk);

	mode = readl(pll->mode_reg);
	/* Disable PLL bypass mode. */
	mode |= BIT(1);
	writel(mode, pll->mode_reg);

	/*
	 * H/W requires a 5us delay between disabling the bypass and
	 * de-asserting the reset. Delay 10us just to be safe.
	 */
	/* udelay(10); */

	/* De-assert active-low PLL reset. */
	mode |= BIT(2);
	writel(mode, pll->mode_reg);

	/* Wait until PLL is locked. */
	/* udelay(50); */

	/* Enable PLL output. */
	mode |= BIT(0);
	writel(mode, pll->mode_reg);

	return 0;
}

void pll_clk_disable(struct clk *clk)
{
	uint32_t mode;
	struct pll_clk *pll = to_pll_clk(clk);

	/*
	 * Disable the PLL output, disable test mode, enable
	 * the bypass mode, and assert the reset.
	 */
	mode = readl(pll->mode_reg);
	mode &= ~BM(3, 0);
	writel(mode, pll->mode_reg);
}

unsigned pll_clk_get_rate(struct clk *clk)
{
	struct pll_clk *pll = to_pll_clk(clk);
	return pll->rate;
}

struct clk *pll_clk_get_parent(struct clk *clk)
{
	struct pll_clk *pll = to_pll_clk(clk);
	return pll->parent;
}

/* clock lookup table */
static struct clk_lookup msm_clocks_8226[] = {
	/* {.con_id="uart3_iface_clk", .clk=&gcc_blsp1_ahb_clk.c}, */
	CLK_LOOKUP("uart3_iface_clk", gcc_blsp1_ahb_clk.c),
	CLK_LOOKUP("uart3_core_clk", gcc_blsp1_uart3_apps_clk.c),
};

void platform_clock_init(void)
{
	clk_init(msm_clocks_8226, ARRAY_SIZE(msm_clocks_8226));
}
