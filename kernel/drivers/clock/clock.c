#include <stdbool.h>
#include <string.h>
#include <clock.h>
#include <xlibs.h>

static struct clk_list msm_clk_list;

int clk_set_parent(struct clk *clk, struct clk *parent)
{
	if(clk->ops->set_parent) 
		return clk->ops->set_parent(clk, parent);
	else return 0;
}

struct clk *clk_get_parent(struct clk *clk)
{
	if(clk->ops->get_parent) 
		return clk->ops->get_parent(clk);
	else return NULL;
}

/* standard clk functions defined in include/clk.h */
int clk_enable(struct clk *clk)
{
	int ret = 0;
	struct clk *parent;
	if (!clk)
		return 0;
	if (clk->count == 0) {
		parent = clk_get_parent(clk);
		if(!parent) {
			ret = clk_enable(parent);
			if (ret) 
				goto out;
		}
		if (clk->ops->enable)
			ret = clk->ops->enable(clk);
		if (ret) {
			clk_disable(parent);
			goto out;
		}
	}
	clk->count++;
out:
	return ret;
}

void clk_disable(struct clk *clk)
{
	struct clk *parent;

	if (!clk)
		return;

	if (clk->count == 0)
		goto out;
	if (clk->count == 1) {
		if(clk->ops->disable) 
			clk->ops->disable(clk);
		parent = clk_get_parent(clk);
		clk_disable(parent);
	}
	clk->count--;
out:
	return;
}

unsigned long clk_get_rate(struct clk *clk)
{
	if (!clk->ops->get_rate)
		return 0;
	return clk->ops->get_rate(clk);
}

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	if (!clk->ops->set_rate)
		return -1;
	return clk->ops->set_rate(clk, rate);
}

void clk_init(struct clk_lookup *clist, unsigned num)
{
	if(clist && num) {
		msm_clk_list.clist = (struct clk_lookup *)clist;
		msm_clk_list.num = num;
	}
}

struct clk *clk_get(const char *cid)
{
	unsigned i;
	struct clk_lookup *cl = msm_clk_list.clist;
	unsigned num = msm_clk_list.num;
	
	if(!cl || !num) {
		return NULL;
	}
	for (i=0; i<num; i++, cl++) {
		if (!strcmp(cl->con_id, cid)) {
			return cl->clk;
		}
	}
	return NULL;
}

int clk_get_set_enable(char *id, unsigned long rate, bool enable)
{
	int ret = 0;
	struct clk *clk;

	/* get clk */
	clk = clk_get(id);
	if (!clk) {
		ret = -1;
		return ret;
	}
	/* set rate */
	if (rate) {
		ret = clk_set_rate(clk, rate);
		if (ret) {
			ret = -1;
			return ret;
		}
	}
	/* enable clk */
	if (enable) {
		ret = clk_enable(clk);
		return ret;
	}
}
