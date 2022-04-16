#include <stdint.h>

static inline uint32_t strcmp(const char *l, const char *r) 
{
	uint32_t i;

	i = 0;
	while (l[i] != '\0') {
		if (l[i] != r[i]) {
			break;
		}
		i++;
	}

	if (l[i] == '\0' && r[i] == '\0') {
		return 0;
	}
	else {
		return 1;
	}
}

static inline void strcpy(char *dst, const char *src)
{
	uint32_t i;

	i = 0;
	while (src[i] != '\0') {
		dst[i] = src[i];
		i++;
	}

	dst[i] = '\0';
}
