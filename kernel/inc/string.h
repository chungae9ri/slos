/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

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

#ifndef _STRING_H_
#define _STRING_H_

#include <stdint.h>
#include <stddef.h>

/**
 * @brief print string to uart0
 *
 * @param [in] fmt Formatted string
 * @param [in] ... Varidic argument
 * @return int 0 for success
 */
int printf(const char *fmt, ...);

void __assert_func(const char *a1, int a2, const char *a3, const char *a4);

/**
 * @brief Allocated memory heap via kmalloc
 *
 * @param [in] sz Size of allocation
 * @return void* Allocated memory pointer
 */
void *malloc(size_t sz);

/**
 * @brief Free allocated memory
 *
 * @param [in] ptr Pointer to the memory freed
 */
void free(void *ptr);

/**
 * @brief Calcuate string length
 *
 * @param [in] s Input string
 * @return size_t Length of input string
 */
size_t strlen(const char *s);

/**
 * @brief String compare
 *
 * @param [in] l Input string l
 * @param [in] r Input string r
 * @return uint32_t 0 for success
 */
uint32_t strcmp(const char *l, const char *r);

/**
 * @brief Copy string
 *
 * @param [in] dst Destination buffer pointer
 * @param [in] src Source string pointer
 */
void strcpy(char *dst, const char *src);

/**
 * @brief Set memory region
 *
 * @param [in] block Memory block pointer
 * @param [in] c Set value
 * @param [in] sz Size of memory block
 */
void memset(void *block, int c, size_t sz);

/**
 * @brief Compare memory block content
 *
 * @param [in] s1 Memory block
 * @param [in] s2 Memory block
 * @param [in] len Memory block size
 * @return uint32_t 0 for equal memory content 1 for not equal
 */
uint32_t memcmp(const void *s1, const void *s2, size_t len);

/**
 * @brief Find first occurrance of input character
 *
 * @param [in] str Input search string
 * @param [in] c Character to be found
 * @return char* Pointer to first occurrance
 */
char *strchr(const char *str, int c);

/**
 * @brief Find the length of first segment that delimited by substring
 *
 * @param [in] str Search string
 * @param [in] reject Delimitor string
 * @return uint32_t Length of first segment
 */
uint32_t strcspn(const char *str, const char *reject);

/**
 * @brief Find the length of first segment that is contained in substring
 *
 * @param [in] str Search string
 * @param [in] accept Substring set for searching
 * @return uint32_t Length of segment contains substring
 */
uint32_t strspn(const char *str, const char *accept);

/**
 * @brief Copy memory content
 *
 * @param [in] dst Pointer to destination
 * @param [in] src Pointer to source
 * @param [in] len Copy length
 * @return void* Destination pointer
 */
void *memcpy(void *dst, const void *src, size_t len);

uint32_t __popcountsi2(uint32_t a);

/**
 * @brief Software implementation for unsigned integer division
 *
 * @param [in] n Numerator
 * @param [in] d Denominator
 */
void __aeabi_uidiv(uint32_t n, uint32_t d);

/**
 * @brief Software implementation for unsigned integer modulo operation
 *
 * @param [in] n Numerator
 * @param [in] d Denominator
 */
void __aeabi_uidivmod(uint32_t n, uint32_t d);

#endif

/**
 * @}
 * @}
 * @}
 *
 */
