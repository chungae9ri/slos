// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#ifndef _DEVICE_H_
#define _DEVICE_H_

#define CONCAT_SEC(A, B) A ## B
#define CONCAT(A, B) CONCAT_SEC(A, B)

#define DEVICE_GET_BASE_ADDR(IDX) CONCAT(DEVICE_DT_COMPAT, _##IDX##_P_BASE_ADDR)
#define DEVICE_GET_IRQ(IDX) CONCAT(DEVICE_DT_COMPAT, _##IDX##_P_INTR)

#endif