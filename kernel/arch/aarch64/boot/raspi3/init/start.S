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

#include "consts.h"
#include <common/asm.h>

.extern arm64_elX_to_el1
.extern boot_cpu_stack
.extern init_c

BEGIN_FUNC(_start)
	mrs	x8, mpidr_el1
	and	x8, x8,	#0xFF
	cbz	x8, primary

	/* hang all secondary processors before we introduce smp */
	b 	.

primary:
	/* Turn to el1 from other exception levels. */
	bl 	arm64_elX_to_el1

	/* Prepare stack pointer and jump to C. */
	ldr 	x0, =boot_cpu_stack
	add 	x0, x0, #INIT_STACK_SIZE
	mov 	sp, x0

	bl 	init_c

	/* Should never be here */
	b	.
END_FUNC(_start)
