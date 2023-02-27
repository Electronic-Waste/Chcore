/*
 * Copyright (c) 2022 Institute of Parallel And Distributed Systems (IPADS)
 * ChCore-Lab is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan
 * PSL v1. You may obtain a copy of Mulan PSL v1 at:
 *     http://license.coscl.org.cn/MulanPSL
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PSL v1 for more details.
 */

#include "image.h"

typedef unsigned long u64;
typedef unsigned int u32;

/* Physical memory address space: 0-1G */
#define PHYSMEM_START   (0x0UL)
#define PERIPHERAL_BASE (0x3F000000UL)
#define PHYSMEM_END     (0x40000000UL)

/* The number of entries in one page table page */
#define PTP_ENTRIES 512
/* The size of one page table page */
#define PTP_SIZE 4096
#define ALIGN(n) __attribute__((__aligned__(n)))
u64 boot_ttbr0_l0[PTP_ENTRIES] ALIGN(PTP_SIZE);
u64 boot_ttbr0_l1[PTP_ENTRIES] ALIGN(PTP_SIZE);
u64 boot_ttbr0_l2[PTP_ENTRIES] ALIGN(PTP_SIZE);

u64 boot_ttbr1_l0[PTP_ENTRIES] ALIGN(PTP_SIZE);
u64 boot_ttbr1_l1[PTP_ENTRIES] ALIGN(PTP_SIZE);
u64 boot_ttbr1_l2[PTP_ENTRIES] ALIGN(PTP_SIZE);

#define IS_VALID (1UL << 0)
#define IS_TABLE (1UL << 1)

#define UXN            (0x1UL << 54)
#define ACCESSED       (0x1UL << 10)
#define NG             (0x1UL << 11)
#define INNER_SHARABLE (0x3UL << 8)
#define NORMAL_MEMORY  (0x4UL << 2)
#define DEVICE_MEMORY  (0x0UL << 2)

#define SIZE_2M (2UL * 1024 * 1024)

#define GET_L0_INDEX(x) (((x) >> (12 + 9 + 9 + 9)) & 0x1ff)
#define GET_L1_INDEX(x) (((x) >> (12 + 9 + 9)) & 0x1ff)
#define GET_L2_INDEX(x) (((x) >> (12 + 9)) & 0x1ff)

void init_boot_pt(void)
{
        /* TODO: will be implemented in lab 2 */
}
