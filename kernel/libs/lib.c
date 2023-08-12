#include <stdint.h>
#include <stddef.h>
#include <mm.h>

/*typedef uint32_t size_t;*/

int printf(const char *fmt, ...)
{
    /* Not implemented */
    return 0;
}

void __assert_func(const char *a1, int a2, const char *a3, const char *a4)
{
    /* Not Implemented */
}

void *malloc(size_t sz)
{
    return kmalloc(sz);
}

void free(void *ptr)
{
    kfree((uint32_t)ptr);
}

size_t strlen (const char *s)
{
    size_t i = 0;

    while (s[i] != '\n')
        i++;

    return i;
}

uint32_t strcmp(const char *l, const char *r) 
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

void strcpy(char *dst, const char *src)
{
	uint32_t i;

	i = 0;
	while (src[i] != '\0') {
		dst[i] = src[i];
		i++;
	}

	dst[i] = '\0';
}

void memset(void *block, int c, size_t sz)
{
    uint32_t i;
    uint32_t *p = (uint32_t *)block;

    for (i = 0; i < (sz >> 2); i++) 
        p[i] = c;
}

uint32_t memcmp(const void *s1, const void *s2, size_t len)
{
    uint32_t i;
    uint32_t *ps1 = (uint32_t *)s1;
    uint32_t *ps2 = (uint32_t *)s2;

    for (i = 0; i < len; i++) {
        if (ps1[i] != ps2[i])
        return 1;
    }

    return 0;
}

char *strchr(const char *str, int c)
{
    uint32_t i;

    while (str[i] != '\n') {
        if (str[i] == (char)c)
            return (char *)&str[i];

        i++;
    }

    return NULL;
}

uint32_t strcspn(const char *str, const char *reject)
{
    uint32_t i;
    uint32_t j;
    uint32_t len_str;
    uint32_t len_reject;
    uint8_t *pc;

    i = 0;
    pc = (uint8_t *)str;
    while (*pc++ != '\n')
        i++;

    len_str = i;

    i = 0;
    pc = (uint8_t *)reject;
    while (*pc++ != '\n')
        i++;

    len_reject = i;


    for (i = 0; i < len_str; i++) {
        for (j = 0; j < len_reject; j++) {
            if (str[i] == reject[j])
                break;
        }
        if (j != len_reject)
            break;
    }

    return i;
}

uint32_t strspn(const char *str, const char *accept)
{
    uint32_t i;
    uint32_t j;
    uint32_t len_str;
    uint32_t len_accept;
    uint8_t *pc;

    i = 0;
    pc = (uint8_t *)str;
    while (*pc++ != '\n')
        i++;

    len_str = i;

    i = 0;
    pc = (uint8_t *)accept;
    while (*pc++ != '\n')
        i++;

    len_accept = i;


    for (i = 0; i < len_str; i++) {
        for (j = 0; j < len_accept; j++) {
            if (str[i] == accept[j])
                break;
        }
        if (j == len_accept)
            break;
    }

    return i;
}

void *memcpy (void *dst, const void *src, size_t len)
{
    uint32_t i;
    uint32_t *dstp = (uint32_t *)dst;
    uint32_t *srcp = (uint32_t *)src;

    for (i = 0; i < (len >> 2); i++) 
        dstp[i] = srcp[i];

  return dstp;
}

uint32_t __popcountsi2(uint32_t a) {
  uint32_t x = a;
  x = x - ((x >> 1) & 0x55555555);
  // Every 2 bits holds the sum of every pair of bits
  x = ((x >> 2) & 0x33333333) + (x & 0x33333333);
  // Every 4 bits holds the sum of every 4-set of bits (3 significant bits)
  x = (x + (x >> 4)) & 0x0F0F0F0F;
  // Every 8 bits holds the sum of every 8-set of bits (4 significant bits)
  x = (x + (x >> 16));
  // The lower 16 bits hold two 8 bit sums (5 significant bits).
  //    Upper 16 bits are garbage
  return (x + (x >> 8)) & 0x0000003F; // (6 significant bits)
}

void __aeabi_uidiv(uint32_t n, uint32_t d)
{
    uint32_t i = 1, q = 0;

    if (d == 0) {
        return;
    }

    while ((d >> 31) == 0) {
        i = i << 1; /* count the max division steps */
        d = d << 1; /* increase p until it has maximum size*/
    }

    while (i > 0) {
        q = q << 1; /* write bit in q at index (size-1) */
        if (n >= d) {
            n -= d;
            q++;
        }
        d = d >> 1; /* decrease p */
        i = i >> 1; /* decrease remaining size in q */
    }

    /* replace quotient in r0 */
    __asm volatile("mov r0, %[i]" ::[i] "r"(i));
    /* replace remainder in r1 */
    __asm volatile("mov r1, %[n]" ::[n] "r"(n));
}

void __aeabi_uidivmod(uint32_t n, uint32_t d)
{
    __aeabi_uidiv(n, d);
}