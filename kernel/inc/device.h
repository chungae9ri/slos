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
 * @brief Macro definitions for devicetree operations
 *
 */

#ifndef _DEVICE_H_
#define _DEVICE_H_

/** Concatenate */
#define CONCAT_SEC(A, B) A##B
/** Concatenate */
#define CONCAT(A, B) CONCAT_SEC(A, B)
/** Get base address of IDX from devicetree */
#define DT_GET_BASE_ADDR(IDX) CONCAT(DEVICE_DT_COMPAT, _##IDX##_P_BASE_ADDR)
/** Get irq number of IDX from devicetree */
#define DT_GET_IRQ(IDX) CONCAT(DEVICE_DT_COMPAT, _##IDX##_P_INTR)
/** Get compatible string of IDX from devicetree */
#define DT_GET_COMPAT(IDX) CONCAT(DEVICE_DT_COMPAT, _##IDX##_P_COMPAT)

/** Device structure */
struct device {
	uint32_t idx;
	const char *name;   /**< Device name */
	uint32_t base_addr; /**< Device base address */
	uint32_t irq;	    /**< Device irq number */
	void *data;	    /**< Device data */
};

/** Device declaration */
#define DEVICE_DECLARE(DEV) extern struct device dev_##DEV
/** Device definition */
#define DEVICE_DEFINE(DEV) struct device dev_##DEV
/** Device declaration with index */
#define DEVICE_DECLARE_IDX(DEV, IDX) extern struct device dev_##DEV##_##IDX
/** Device definition with index */
#define DEVICE_DEFINE_IDX(DEV, IDX) struct device dev_##DEV##_##IDX

/** Get device struct instance */
#define DEVICE_GET(DEV) &dev_##DEV
/** Get device struct instance with index */
#define DEVICE_GET_IDX(DEV, IDX) &dev_##DEV##_##IDX
/** Get device name from device intance */
#define DEVICE_GET_NAME(DEV) dev_##DEV.name
/** Get device name from device intance with index */
#define DEVICE_GET_NAME_IDX(DEV, IDX) dev_##DEV##_##IDX.name
/** Get device base address from device intance */
#define DEVICE_GET_BASE_ADDR(DEV) dev_##DEV.base_addr
/** Get device base address from device intance with index*/
#define DEVICE_GET_BASE_ADDR_IDX(DEV, IDX) dev_##DEV##_##IDX.base_addr
/** Get device irq from device intance */
#define DEVICE_GET_IRQ(DEV) dev_##DEV.irq
/** Get device irq from device intance with index */
#define DEVICE_GET_IRQ_IDX(DEV, IDX) dev_##DEV##_##IDX.irq

#endif

/**
 * @}
 * @}
 * @}
 *
 */
