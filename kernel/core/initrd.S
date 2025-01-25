/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

.section ".initrd", "a"

			.incbin "../ramdisk/ramdisk.img"
