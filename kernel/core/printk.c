#include <stdint.h>
#include <stdarg.h>
#include <rwbyte.h>

#define NUM_STR_MAX		(16u)

void printk(const char *fmt, ...)
{
	uint8_t *pch, *pstr;
	uint32_t u_int, i, u_hex, str_len;
	uint8_t num_str[NUM_STR_MAX] = {0};
	va_list argp;

	va_start(argp, fmt);

	for (pch = (uint8_t *)fmt; *pch != '\0'; pch++) {
		if (*pch != '%') {
			outbyte(*pch);
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
				outbyte(num_str[str_len - 1 - i]);
			}
			break;

		case 's':
			pstr = va_arg(argp, uint8_t *);
			while (*pstr != '\0') {
				outbyte(*pstr++);
			}
			break;
		default:
			break;
		}
	}

	va_end(argp);
}
