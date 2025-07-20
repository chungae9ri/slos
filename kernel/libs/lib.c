// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_core Core
 * @{
 * @addtogroup kernel_core_misc Miscellaneous kernel modules
 * @{
 *
 * @brief Library function to be linked with littleFS
 *
 */

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include <mm.h>
#include <uart.h>

static struct device *uart_dev = DEVICE_GET_IDX(uart, 0);

int printf(const char *fmt, ...)
{
	uint8_t *pch, *pstr;
	uint32_t u_int, i, u_hex, str_len;
	uint8_t num_str[16] = {0};
	int32_t s_int, s_int_val;
	va_list argp;

	va_start(argp, fmt);

	for (pch = (uint8_t *)fmt; *pch != '\0'; pch++) {
		if (*pch != '%') {
			poll_out(uart_dev, *pch);
			continue;
		}
		switch (*++pch) {
		case 'x':
			u_int = va_arg(argp, uint32_t);
			i = 0;
			do {
				u_hex = (u_int & 0xF);
				if (u_hex < 10) {
					num_str[i++] = u_hex + '0';
				} else {
					num_str[i++] = (u_hex - 10) + 'A';
				}
				u_int >>= 4;
			} while (u_int > 0);

			str_len = i;
			num_str[i] = '\0';
			for (i = 0; i < str_len; i++) {
				poll_out(uart_dev, num_str[str_len - 1 - i]);
			}
			break;

		case 'd':
			s_int = va_arg(argp, int32_t);
			i = 0;
			do {
				s_int_val = (s_int % 10);
				num_str[i++] = s_int_val + '0';
				s_int /= 10;
			} while (s_int > 0);

			str_len = i;
			num_str[i] = '\0';
			for (i = 0; i < str_len; i++) {
				poll_out(uart_dev, num_str[str_len - 1 - i]);
			}
			break;

		case 's':
			pstr = va_arg(argp, uint8_t *);
			while (*pstr != '\0') {
				poll_out(uart_dev, *pstr++);
			}
			break;
		default:
			break;
		}
	}

	va_end(argp);

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

size_t strlen(const char *s)
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
	} else {
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

void *memset(void *block, uint8_t c, size_t sz)
{
	uint32_t i;
	uint8_t *p = (uint8_t *)block;

	for (i = 0; i < sz; i++)
		p[i] = c;

	return p;
}

uint32_t memcmp(const void *s1, const void *s2, size_t len)
{
	uint32_t i;
	uint8_t *ps1 = (uint8_t *)s1;
	uint8_t *ps2 = (uint8_t *)s2;

	for (i = 0; i < len; i++) {
		if (ps1[i] != ps2[i])
			return 1;
	}

	return 0;
}

/* Find the first occurrence of C in S.  */
char *strchr(const char *s, int c_in)
{
	const unsigned char *char_ptr;
	const unsigned long int *longword_ptr;
	unsigned long int longword, magic_bits, charmask;
	unsigned char c;

	c = (unsigned char)c_in;

	/* Handle the first few characters by reading one character at a time.
	   Do this until CHAR_PTR is aligned on a longword boundary.  */
	for (char_ptr = (const unsigned char *)s;
	     ((unsigned long int)char_ptr & (sizeof(longword) - 1)) != 0; ++char_ptr)
		if (*char_ptr == c)
			return (void *)char_ptr;
		else if (*char_ptr == '\0')
			return NULL;

	/* All these elucidatory comments refer to 4-byte longwords,
	   but the theory applies equally well to 8-byte longwords.  */

	longword_ptr = (unsigned long int *)char_ptr;

	/* Bits 31, 24, 16, and 8 of this number are zero.  Call these bits
	   the "holes."  Note that there is a hole just to the left of
	   each byte, with an extra at the end:

	   bits:  01111110 11111110 11111110 11111111
	   bytes: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD

	   The 1-bits make sure that carries propagate to the next 0-bit.
	   The 0-bits provide holes for carries to fall into.  */
	magic_bits = -1;
	magic_bits = magic_bits / 0xff * 0xfe << 1 >> 1 | 1;

	/* Set up a longword, each of whose bytes is C.  */
	charmask = c | (c << 8);
	charmask |= charmask << 16;
	if (sizeof(longword) > 4)
		/* Do the shift in two steps to avoid a warning if long has 32
		 * bits.  */
		charmask |= (charmask << 16) << 16;
	/* slos doesn't have abort()
	if (sizeof(longword) > 8)
	    abort();
	*/

	/* Instead of the traditional loop which tests each character,
	   we will test a longword at a time.  The tricky part is testing
	   if *any of the four* bytes in the longword in question are zero.  */
	for (;;) {
		/* We tentatively exit the loop if adding MAGIC_BITS to
	       LONGWORD fails to change any of the hole bits of LONGWORD.

	       1) Is this safe?  Will it catch all the zero bytes?
	       Suppose there is a byte with all zeros.  Any carry bits
	       propagating from its left will fall into the hole at its
	       least significant bit and stop.  Since there will be no
	       carry from its most significant bit, the LSB of the
	       byte to the left will be unchanged, and the zero will be
	       detected.

	       2) Is this worthwhile?  Will it ignore everything except
	       zero bytes?  Suppose every byte of LONGWORD has a bit set
	       somewhere.  There will be a carry into bit 8.  If bit 8
	       is set, this will carry into bit 16.  If bit 8 is clear,
	       one of bits 9-15 must be set, so there will be a carry
	       into bit 16.  Similarly, there will be a carry into bit
	       24.  If one of bits 24-30 is set, there will be a carry
	       into bit 31, so all of the hole bits will be changed.

	       The one misfire occurs when bits 24-30 are clear and bit
	       31 is set; in this case, the hole at bit 31 is not
	       changed.  If we had access to the processor carry flag,
	       we could close this loophole by putting the fourth hole
	       at bit 32!

	       So it ignores everything except 128's, when they're aligned
	       properly.

	       3) But wait!  Aren't we looking for C as well as zero?
	       Good point.  So what we do is XOR LONGWORD with a longword,
	       each of whose bytes is C.  This turns each byte that is C
	       into a zero.  */

		longword = *longword_ptr++;

		/* Add MAGIC_BITS to LONGWORD.  */
		if ((((longword + magic_bits)

		      /* Set those bits that were unchanged by the addition.  */
		      ^ ~longword)

		     /* Look at only the hole bits.  If any of the hole bits
			are unchanged, most likely one of the bytes was a
			zero.  */
		     & ~magic_bits) != 0 ||

		    /* That caught zeroes.  Now test for C.  */
		    ((((longword ^ charmask) + magic_bits) ^ ~(longword ^ charmask)) &
		     ~magic_bits) != 0) {
			/* Which of the bytes was C or zero?
			   If none of them were, it was a misfire; continue the
			   search.  */

			const unsigned char *cp = (const unsigned char *)(longword_ptr - 1);

			if (*cp == c)
				return (char *)cp;
			else if (*cp == '\0')
				return NULL;
			if (*++cp == c)
				return (char *)cp;
			else if (*cp == '\0')
				return NULL;
			if (*++cp == c)
				return (char *)cp;
			else if (*cp == '\0')
				return NULL;
			if (*++cp == c)
				return (char *)cp;
			else if (*cp == '\0')
				return NULL;
			if (sizeof(longword) > 4) {
				if (*++cp == c)
					return (char *)cp;
				else if (*cp == '\0')
					return NULL;
				if (*++cp == c)
					return (char *)cp;
				else if (*cp == '\0')
					return NULL;
				if (*++cp == c)
					return (char *)cp;
				else if (*cp == '\0')
					return NULL;
				if (*++cp == c)
					return (char *)cp;
				else if (*cp == '\0')
					return NULL;
			}
		}
	}

	return NULL;
}

void *memcpy(void *dst, const void *src, size_t len)
{
	uint32_t i;
	uint8_t *dstp = (uint8_t *)dst;
	uint8_t *srcp = (uint8_t *)src;

	for (i = 0; i < len; i++)
		dstp[i] = srcp[i];

	return dstp;
}

/* Align a value by rounding down to closest size.
 * e.g. Using size of 4096, we get this behavior:
 * {4095, 4096, 4097} = {0, 4096, 4096}.
 */
#define ALIGN_DOWN(base, size) ((base) & -((__typeof__(base))(size)))

/* Same as ALIGN_DOWN(), but automatically casts when base is a pointer. */
#define PTR_ALIGN_DOWN(base, size) ((__typeof__(base))ALIGN_DOWN((uintptr_t)(base), (size)))

/* Return the length of the maximum initial segment of S
 * which contains no characters from REJECT.
 */

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

size_t strcspn(const char *str, const char *reject)
{
	/* Use multiple small memsets to enable inlining on most targets.  */
	unsigned char table[256];
	unsigned char *p = memset(table, 0, 64);
	memset(p + 64, 0, 64);
	memset(p + 128, 0, 64);
	memset(p + 192, 0, 64);

	unsigned char *s = (unsigned char *)reject;
	unsigned char tmp;
	do
		p[tmp = *s++] = 1;
	while (tmp);

	s = (unsigned char *)str;
	if (p[s[0]])
		return 0;
	if (p[s[1]])
		return 1;
	if (p[s[2]])
		return 2;
	if (p[s[3]])
		return 3;

	s = (unsigned char *)PTR_ALIGN_DOWN(s, 4);

	unsigned int c0, c1, c2, c3;
	do {
		s += 4;
		c0 = p[s[0]];
		c1 = p[s[1]];
		c2 = p[s[2]];
		c3 = p[s[3]];
	} while ((c0 | c1 | c2 | c3) == 0);

	size_t count = s - (unsigned char *)str;
	return (c0 | c1) != 0 ? count - c0 + 1 : count - c2 + 3;
}

/* Return the length of the maximum initial segment
 * of S which contains only characters in ACCEPT.
 */
size_t strspn(const char *str, const char *accept)
{
	if (accept[0] == '\0')
		return 0;

	if (unlikely(accept[1] == '\0')) {
		const char *a = str;
		for (; *str == *accept; str++)
			;
		return str - a;
	}

	/* Use multiple small memsets to enable inlining on most targets.  */
	unsigned char table[256];
	unsigned char *p = memset(table, 0, 64);
	memset(p + 64, 0, 64);
	memset(p + 128, 0, 64);
	memset(p + 192, 0, 64);

	unsigned char *s = (unsigned char *)accept;
	/* Different from strcspn it does not add the NULL on the table
	   so can avoid check if str[i] is NULL, since table['\0'] will
	   be 0 and thus stopping the loop check.  */
	do
		p[*s++] = 1;
	while (*s);

	s = (unsigned char *)str;
	if (!p[s[0]])
		return 0;
	if (!p[s[1]])
		return 1;
	if (!p[s[2]])
		return 2;
	if (!p[s[3]])
		return 3;

	s = (unsigned char *)PTR_ALIGN_DOWN(s, 4);

	unsigned int c0, c1, c2, c3;
	do {
		s += 4;
		c0 = p[s[0]];
		c1 = p[s[1]];
		c2 = p[s[2]];
		c3 = p[s[3]];
	} while ((c0 & c1 & c2 & c3) != 0);

	size_t count = s - (unsigned char *)str;
	return (c0 & c1) == 0 ? count + c0 : count + c2 + 2;
}

uint32_t __popcountsi2(uint32_t a)
{
	uint32_t x = a;
	x = x - ((x >> 1) & 0x55555555);
	// Every 2 bits holds the sum of every pair of bits
	x = ((x >> 2) & 0x33333333) + (x & 0x33333333);
	// Every 4 bits holds the sum of every 4-set of bits (3 significant
	// bits)
	x = (x + (x >> 4)) & 0x0F0F0F0F;
	// Every 8 bits holds the sum of every 8-set of bits (4 significant
	// bits)
	x = (x + (x >> 16));
	// The lower 16 bits hold two 8 bit sums (5 significant bits).
	//    Upper 16 bits are garbage
	return (x + (x >> 8)) & 0x0000003F; // (6 significant bits)
}

void __aeabi_uidiv(uint32_t n, uint32_t d)
{
	uint32_t i = 0;

    if (d == 0) {
        while (1);  // Trap or halt
    }

    while (n >= d) {
        n -= d;
        i++;
    }

	/* replace quotient in r0 */
	__asm volatile("mov r0, %[i]" ::[i] "r"(i));
	/* replace remainder in r1 */
	__asm volatile("mov r1, %[n]" ::[n] "r"(n));
}

void __aeabi_uidivmod(uint32_t n, uint32_t d)
{
	__aeabi_uidiv(n, d);

	/* replace r0 with r1 (remainder)*/
	__asm volatile("mov r0, r1" ::);
}

/**
 * @}
 * @}
 * @}
 *
 */