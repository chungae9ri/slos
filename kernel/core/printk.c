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
 * @file
 *
 * @brief Print message through stdout
 *
 */

#include <stdint.h>
#include <stdarg.h>

#include <uart.h>

#define NUM_STR_MAX (16u)

static struct device *uart_dev = DEVICE_GET_IDX(uart, 0);

void printk(const char *fmt, ...)
{
	uint8_t *pch, *pstr;
	uint32_t u_int, i, u_hex, str_len;
	uint8_t num_str[NUM_STR_MAX] = {0};
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
}

void sprintk(uint8_t *buf, const char *fmt, ...)
{
	uint8_t *pch, *pstr;
	uint32_t u_int, i, j, u_hex, str_len;
	uint8_t num_str[NUM_STR_MAX] = {0};
	va_list argp;

	va_start(argp, fmt);
	for (pch = (uint8_t *)fmt, i = 0; *pch; pch++) {
		if (*pch != '%') {
			buf[i++] = *pch;
			continue;
		}
		switch (*++pch) {
		case 'x':
			u_int = va_arg(argp, uint32_t);
			j = 0;
			do {
				u_hex = (u_int & 0xF);
				if (u_hex < 10) {
					num_str[j++] = u_hex + '0';
				} else {
					num_str[j++] = (u_hex - 10) + 'A';
				}
				u_int >>= 4;
			} while (u_int > 0);

			str_len = j;
			num_str[j] = '\0';
			for (j = 0; j < str_len; j++) {
				buf[i++] = num_str[str_len - 1 - j];
			}
			break;

		case 's':
			pstr = va_arg(argp, uint8_t *);
			while (*pstr != '\0') {
				buf[i++] = *pstr++;
			}
			buf[i] = '\0';
			break;
		default:
			break;
		}
	}

	va_end(argp);
}

/**
 * @}
 * @}
 * @}
 *
 */
