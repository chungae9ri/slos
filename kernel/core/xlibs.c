#include <stdarg.h>

int xitoa(int num, char *buf, int base)
{
	int rem, i=0, j, temp, minus = 0;

	if (num == 0) {
		buf[0] = '0';
		buf[1] = '\0';
		return 2;
	}

	if (num < 0) {
		minus = 1;
		num = num * (-1);
	}

	while (num > 0) {
		rem = num % base;
		buf[i++] = (rem > 9) ? rem-10+'a' : rem+'0';
		num = (int)(num/base);
	}
	for (j=0 ; j<i/2 ; j++) {
		buf[i-j-1] = buf[i-j-1] ^ buf[j];
		buf[j] = buf[j] ^ buf[i-j-1];
		buf[i-j-1] = buf[j] ^ buf[i-j-1];
	}
	buf[i] = '\0';
	if (minus) {
		for (j=i ; j>=0 ; j--) {
			buf[j+1] = buf[j];
		}
		buf[0]= '-';
	}
	return i;
}

int xsprintf(char *buf, const char *fmt, ...)
{
	int i = 0;
	char *pc;
	va_list varg;
	va_start(varg, fmt);

	for (; *fmt != '\0' ; fmt++) {
		if (*fmt == '%') {
			fmt++;
			if(*fmt == 'd') {
				i += xitoa((int)va_arg(varg, int), &buf[i], 10);

			} else if(*fmt == 's') {
				pc = va_arg(varg, char *);
				while (*pc != '\0') {
					buf[i++] = *pc++;
				}
			}

		} else if (*fmt == '\\') {
			fmt++;
			if (*fmt == 'n') {
				buf[i++] = '\n'; /* new line */
				buf[i++] = '\0'; 
				break;
			}
		} else {
			buf[i++] = *fmt;
		}
	}

	va_end(varg);
	return i;
}

int xstrcmp(const char *p, const char *q)
{
	int ret=0;

	while(*p++ == *q++) {
		if (*p == '\0' || *q == '\0') break;
	}
	if(*p=='\0' && *q=='\0') return 0;
	else return 1;
}
