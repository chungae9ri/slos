/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _PLATFORM_MSMCOPPER_IOMAP_H_
#define _PLATFORM_MSMCOPPER_IOMAP_H_

#define MSM_IOMAP_BASE              0xF9000000
#define MSM_IOMAP_END               0xFEFFFFFF

#define KPSS_BASE                   0xF9000000

#define MSM_GIC_DIST_BASE           KPSS_BASE
#define MSM_GIC_CPU_BASE            (KPSS_BASE + 0x2000)
#define GIC_CPU_REG(off)            (MSM_GIC_CPU_BASE  + (off))
#define GIC_DIST_REG(off)           (MSM_GIC_DIST_BASE + (off))

#define GIC_CPU_CTRL                GIC_CPU_REG(0x00)
#define GIC_CPU_PRIMASK             GIC_CPU_REG(0x04)
#define GIC_CPU_BINPOINT            GIC_CPU_REG(0x08)
#define GIC_CPU_INTACK              GIC_CPU_REG(0x0c)
#define GIC_CPU_EOI                 GIC_CPU_REG(0x10)
#define GIC_CPU_RUNNINGPRI          GIC_CPU_REG(0x14)
#define GIC_CPU_HIGHPRI             GIC_CPU_REG(0x18)

#define GIC_DIST_CTRL               GIC_DIST_REG(0x000)
#define GIC_DIST_CTR                GIC_DIST_REG(0x004)
#define GIC_DIST_ENABLE_SET         GIC_DIST_REG(0x100)
#define GIC_DIST_ENABLE_CLEAR       GIC_DIST_REG(0x180)
#define GIC_DIST_PENDING_SET        GIC_DIST_REG(0x200)
#define GIC_DIST_PENDING_CLEAR      GIC_DIST_REG(0x280)
#define GIC_DIST_ACTIVE_BIT         GIC_DIST_REG(0x300)
#define GIC_DIST_PRI                GIC_DIST_REG(0x400)
#define GIC_DIST_TARGET             GIC_DIST_REG(0x800)
#define GIC_DIST_CONFIG             GIC_DIST_REG(0xc00)
#define GIC_DIST_SOFTINT            GIC_DIST_REG(0xf00)

#define KPSS_APCS_F0_QTMR_V1_BASE   (KPSS_BASE + 0x00021000)
#define QTMR_BASE                   KPSS_APCS_F0_QTMR_V1_BASE
#define QTMR_V1_CNTPCT_LO	(QTMR_BASE + 0x00000000)
#define QTMR_V1_CNTPCT_HI	(QTMR_BASE + 0x00000004)
#define QTMR_V1_CNTFRQ		(QTMR_BASE + 0x00000010)
#define QTMR_V1_CNTP_CVAL_LO	(QTMR_BASE + 0x00000020)
#define QTMR_V1_CNTP_CVAL_HI	(QTMR_BASE + 0x00000024)
#define QTMR_V1_CNTP_TVAL	(QTMR_BASE + 0x00000028)
#define QTMR_V1_CNTP_CTL	(QTMR_BASE + 0x0000002C)
/**/
#define PERIPH_SS_BASE              0xF9800000

#define BLSP1_UART0_BASE            (PERIPH_SS_BASE + 0x0011D000)                                            
#define BLSP1_UART1_BASE            (PERIPH_SS_BASE + 0x0011E000)                                            
#define BLSP1_UART2_BASE            (PERIPH_SS_BASE + 0x0011F000)                                            
#define BLSP1_UART3_BASE            (PERIPH_SS_BASE + 0x00120000)                                            
#define BLSP1_UART4_BASE            (PERIPH_SS_BASE + 0x00121000)                                            
#define BLSP1_UART5_BASE            (PERIPH_SS_BASE + 0x00122000)
/**/
#define TLMM_BASE_ADDR              0xFD510000

#define GPIO_CONFIG_ADDR(x)         (TLMM_BASE_ADDR + 0x1000 + (x)*0x10)
#define GPIO_IN_OUT_ADDR(x)         (TLMM_BASE_ADDR + 0x1004 + (x)*0x10)
#define GPIO_OUT_SET_ADDR(x)        (TLMM_BASE_ADDR + 0x3040 + (x/32)*0x04)
#define GPIO_OUT_CLR_ADDR(x)        (TLMM_BASE_ADDR + 0x3020 + (x/32)*0x04)
#define GPIO_OUT_VAL(x)             (1 << (x - (x/32)*32))
#define GPIO_OUT_OE_SET_ADDR(x)     (TLMM_BASE_ADDR + 0x3120 + (x/32)*0x04)
#define GPIO_OUT_OE_VAL(x)          (1 << (x - (x/32)*32))
/**/
#define CLK_CTL_BASE		0xFC400000
/* GPLL */
#define GPLL0_MODE			CLK_CTL_BASE
#define GPLL0_STATUS			(CLK_CTL_BASE + 0x001C)
#define APCS_GPLL_ENA_VOTE		(CLK_CTL_BASE + 0x1480)
#define APCS_CLOCK_BRANCH_ENA_VOTE	(CLK_CTL_BASE + 0x1484)
/* UART */
#define BLSP1_AHB_CBCR			(CLK_CTL_BASE + 0x5C4)
#define BLSP1_UART3_APPS_CBCR		(CLK_CTL_BASE + 0x784)
#define BLSP1_UART3_APPS_CMD_RCGR	(CLK_CTL_BASE + 0x78C)
#define BLSP1_UART3_APPS_CFG_RCGR	(CLK_CTL_BASE + 0x790)
#define BLSP1_UART3_APPS_M		(CLK_CTL_BASE + 0x794)
#define BLSP1_UART3_APPS_N		(CLK_CTL_BASE + 0x798)
#define BLSP1_UART3_APPS_D		(CLK_CTL_BASE + 0x79C)

#endif
