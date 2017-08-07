#define REG32(addr) 	((volatile unsigned int *)(addr))

#define writel(v, a) 	((*REG32(a)) = (v))
#define readl(a)	(*REG32(a))
