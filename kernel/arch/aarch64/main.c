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

#include <common/kprint.h>
#include <common/vars.h>
#include <common/macro.h>
#include <common/types.h>
#include <io/uart.h>
#include <machine.h>

ALIGN(STACK_ALIGNMENT)
char kernel_stack[PLAT_CPU_NUM][KERNEL_STACK_SIZE];

/*
 * @addr is boot flag addresses for smp;
 */
void main(void *addr)
{
        BUG("[FATAL] Should never be here!\n");
}
