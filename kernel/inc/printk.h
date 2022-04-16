#ifndef _PRINTK_H_
#define _PRINTK_H_
#include <stdint.h>

void printk(const char *fmt, ...);
void sprintk(char *buf, const char *fmt, ...);
#endif
