/**
 * Configuration for MAX32655-CTBGA.
 *
 * This file was generated using Analog Devices CodeFusion Studio.
 * https://github.com/analogdevicesinc/codefusion-studio
 *
 * Generated at: 2025-12-05T17:57:03.003Z *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2025 Analog Devices, Inc.
 */


MEMORY {
    ROM         (rx)  : ORIGIN = 0x00000000, LENGTH = 0x00010000 /* 64kB ROM */
    PAL_NVM_DB  (r)   : ORIGIN = 0x00000000, LENGTH = 0x00000000 /* Section not defined. */
    RISCV_FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 0x00000000 /* Section not defined. */
    MAILBOX_0   (rw)  : ORIGIN = 0x00000000, LENGTH = 0x00000000 /* Section not defined. */
    MAILBOX_1   (rw)  : ORIGIN = 0x00000000, LENGTH = 0x00000000 /* Section not defined. */

    FLASH (r)   : ORIGIN = 0x10000000, LENGTH = 0x00080000 /* FLASH */
    SRAM  (rw)  : ORIGIN = 0x20000000, LENGTH = 0x00020000 /* SRAM  */
}

INCLUDE max32655.sects.ld
