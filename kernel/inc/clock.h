#ifndef CLOCK_H
#define CLOCK_H
#include <stdbool.h>
#include <stdint-gcc.h>
#include <stddef.h>
#include <bits.h>
#include <defs.h>

struct clk;
struct clk_ops {
	int (*enable)(struct clk *clk);
	void (*disable)(struct clk *clk);
	void (*auto_off)(struct clk *clk);
	int (*set_rate)(struct clk *clk, unsigned rate);
	unsigned (*get_rate)(struct clk *clk);
	int (*set_parent)(struct clk *clk, struct clk *parent);
	struct clk *(*get_parent)(struct clk *clk);
	int (*is_enabled)(struct clk *clk);
};

struct clk {
	uint32_t flags;
	uint32_t rate;
	struct clk_ops *ops;
	const char *dbg_name;
	unsigned count;
};

/*
 * Generic frequency-definition structs and macros
 */
struct clk_freq_tbl {

	const uint32_t  freq_hz;
	struct clk     *src_clk;
	const uint32_t  div_src_val; 

	/* TODO: find out if sys_vdd is needed. */

	const uint32_t  m_val;
	const uint32_t  n_val; /* not_n_minus_m_val */
	const uint32_t  d_val; /* not_2d_val        */
};

struct clk *clk_get(const char *id);
int clk_enable(struct clk *clk);
void clk_disable(struct clk *clk);
unsigned long clk_get_rate(struct clk *clk);
int clk_set_rate(struct clk *clk, unsigned long rate);
int clk_set_parent(struct clk *clk, struct clk *parent);
struct clk *clk_get_parent(struct clk *clk);
int clk_get_set_enable(char *id, unsigned long rate, bool enable);

struct clk_lookup {
	const char *con_id;
	struct clk *clk;
};
struct clk_list {
	struct clk_lookup *clist;
	unsigned num;
};
void clk_init(struct clk_lookup *clist, unsigned num);
/*
 */

/* Branch clock */
struct branch_clk {

	uint32_t *const bcr_reg;
	uint32_t *const cbcr_reg;

	void   (*set_rate)(struct branch_clk *, struct clk_freq_tbl *);

	struct clk *parent;
	struct clk  c;

	int      has_sibling;
	uint32_t cur_div;
	uint32_t max_div;
	uint32_t halt_check;
};

/*
 * Generic clock-definition struct and macros
 */
struct rcg_clk {

	/* RCG registers for this clock */

	uint32_t *const cmd_reg; /* Command reg */
	uint32_t *const cfg_reg; /* Config  reg */
	uint32_t *const m_reg;   /*       m     */
	uint32_t *const n_reg;   /* not (n-m)   */
	uint32_t *const d_reg;   /* not (2d)    */

	/* set rate function for this clock */
	void   (*set_rate)(struct rcg_clk *, struct clk_freq_tbl *);

	/* freq table */
	struct clk_freq_tbl *const freq_tbl;
	struct clk_freq_tbl *current_freq;

	struct clk c;
};

/* Vote clock */
struct vote_clk {
	uint32_t *const cbcr_reg;
	uint32_t *const vote_reg;
	uint32_t en_mask;
	struct clk c;
};

/*
   pll clock
 */
struct pll_clk {
	unsigned long rate;

	void *const mode_reg;

	struct clk *parent;
	struct clk c;
};

struct pll_vote_clk {
	unsigned long rate;

	void *const en_reg;
	const uint32_t en_mask;

	void *const status_reg;
	const uint32_t status_mask;

	struct clk *parent;
	struct clk c;
};

int pll_vote_clk_enable(struct clk *clk);
void pll_vote_clk_disable(struct clk *clk);
int pll_vote_clk_is_enabled(struct clk *clk);
/*
 */
struct fixed_clk {
	struct clk c;
};
/* Branch clock Bits */
#define CBCR_BRANCH_ENABLE_BIT  BIT(0)
#define CBCR_BRANCH_OFF_BIT     BIT(31)
#define BRANCH_CHECK_MASK       BM(31, 28)
#define BRANCH_ON_VAL           BVAL(31, 28, 0x0)
#define BRANCH_NOC_FSM_ON_VAL   BVAL(31, 28, 0x2)

/* Root Clock Bits */
#define CMD_UPDATE_BIT          BIT(0)
#define CMD_UPDATE_MASK         1

#define CFG_SRC_DIV_OFFSET      0
#define CFG_SRC_DIV_MASK        (0x1F << CFG_SRC_DIV_OFFSET)
        
#define CFG_SRC_SEL_OFFSET      8
#define CFG_SRC_SEL_MASK        (0x3 << CFG_SRC_SEL_OFFSET)
        
#define CFG_MODE_DUAL_EDGE      0x2
        
#define CFG_MODE_OFFSET         12
#define CFG_MODE_MASK           (0x3 << CFG_MODE_OFFSET)

/* Frequency Macros */
#define FREQ_END        -1
#define F_END \
	{ \
		.freq_hz = FREQ_END, \
	}

/* F(frequency, source, div, m, n) */
#define F(f, s, div, m, n) \
	{ \
		.freq_hz = (f), \
		.src_clk = &s##_clk_src.c, \
		.m_val   = (m), \
		.n_val   = ~((n)-(m)) * !!(n), \
		.d_val   = ~(n),\
		.div_src_val = BVAL(4, 0, (int)(2*(div) - 1)) \
		| BVAL(10, 8, s##_source_val), \
	}

#define CLK_LOOKUP(con, c) {.con_id=con, .clk=&c}
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

static inline struct rcg_clk *to_rcg_clk(struct clk *clk)
{       
	return container_of(clk, struct rcg_clk, c);
}
static inline struct branch_clk *to_branch_clk(struct clk *clk)
{               
	return container_of(clk, struct branch_clk, c);
}   
static inline struct vote_clk *to_local_vote_clk(struct clk *clk)
{
	return container_of(clk, struct vote_clk, c);
}
static inline struct pll_vote_clk *to_pll_vote_clk(struct clk *clk)
{
	return container_of(clk, struct pll_vote_clk, c);
}
static inline struct pll_clk *to_pll_clk(struct clk *clk)
{
        return container_of(clk, struct pll_clk, c);
}

/* Mux source select values */
#define cxo_source_val 0
#define gpll0_source_val 1
#endif 
