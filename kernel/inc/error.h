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
 * @brief Error codes
 *
 */

#ifndef _ERROR_H_
#define _ERROR_H_

#include <errno.h>

/** No error */
#define NO_ERR 0
/** File IO error */
#define IO_ERR (__ELASTERROR + 1)
/** File system metadata error */
#define METADATA_ERR (__ELASTERROR + 2)
/** File system no remaining data block */
#define NO_DATABLK_ERR (__ELASTERROR + 3)
/** File system inode error */
#define NO_INODE_ERR (__ELASTERROR + 4)
/** File system parameter error */
#define PARAM_ERR (__ELASTERROR + 5)
/** File system file parameter error */
#define FILE_PARAM_ERR (__ELASTERROR + 6)
/** Null pointer error */
#define NULL_PTR_ERR (__ELASTERROR + 7)
/** File system inode last error */
#define INODE_LAST_ERR (__ELASTERROR + 8)
/** File system list broken error */
#define LIST_BROKEN_ERR (__ELASTERROR + 9)
/** File system file size error */
#define FILE_SIZE_ERR (__ELASTERROR + 10)
/** User application max number error */
#define USER_APP_MAX_ERR (__ELASTERROR + 11)
/** ITAB full error */
#define ERR_ITAB_FULL (__ELASTERROR + 12)
/** Memory frame free failure */
#define FRAMEPOOL_FREE_ERR (__ELASTERROR + 13)

#endif

/**
 * @}
 * @}
 * @}
 *
 */

