// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#ifndef _STRING_H_
#define _STRING_H_

#include <stdint.h>
#include <stddef.h>

int printf(const char *fmt, ...);

void __assert_func(const char *a1, int a2, const char *a3, const char *a4);

void *malloc(size_t sz);

void free(void *ptr);

size_t strlen(const char *s);

uint32_t strcmp(const char *l, const char *r);

void strcpy(char *dst, const char *src);

void memset(void *block, int c, size_t sz);

uint32_t memcmp(const void *s1, const void *s2, size_t len);

char *strchr(const char *str, int c);

uint32_t strcspn(const char *str, const char *reject);

uint32_t strspn(const char *str, const char *accept);

void *memcpy(void *dst, const void *src, size_t len);

uint32_t __popcountsi2(uint32_t a);

void __aeabi_uidiv(uint32_t n, uint32_t d);

void __aeabi_uidivmod(uint32_t n, uint32_t d);

#endif