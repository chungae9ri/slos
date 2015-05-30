#ifndef _DEFS_H_
#define _DEFS_H_

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

#define container_of(ptr, type, member) \
	((type *)((unsigned int)ptr-offsetof(type, member)))
#endif
