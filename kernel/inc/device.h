// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#ifndef _DEVICE_H_
#define _DEVICE_H_

#define CONCAT_SEC(A, B) A ## B
#define CONCAT(A, B) CONCAT_SEC(A, B)

#define DT_GET_BASE_ADDR(IDX) CONCAT(DEVICE_DT_COMPAT, _##IDX##_P_BASE_ADDR)
#define DT_GET_IRQ(IDX) CONCAT(DEVICE_DT_COMPAT, _##IDX##_P_INTR)
#define DT_GET_COMPAT(IDX) CONCAT(DEVICE_DT_COMPAT, _##IDX##_P_COMPAT)

struct device {
    char name[32];
    uint32_t base_addr;
    uint32_t irq;
};

#define DEVICE_DEFINE(DEV, NAME, BASE_ADDR, IRQ) \
        struct device dev_##DEV = { \
            .name = NAME, \
            .base_addr = BASE_ADDR, \
            .irq = IRQ, \
        }

#define DEVICE_GET(DEV) dev_##DEV
#define DEVICE_GET_NAME(DEV) dev_##DEV.name
#define DEVICE_GET_BASE_ADDR(DEV) dev_##DEV.base_addr
#define DEVICE_GET_IRQ(DEV) dev_##DEV.irq

#endif