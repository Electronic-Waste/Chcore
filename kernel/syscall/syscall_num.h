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

#pragma once

#define NR_SYSCALL 256

/* Character */
#define SYS_putc 0
#define SYS_getc 1

/* PMO */
/* - single */
#define SYS_create_pmo        10
#define SYS_create_device_pmo 11
#define SYS_map_pmo           12
#define SYS_unmap_pmo         13
#define SYS_write_pmo         14
#define SYS_read_pmo          15
/* - batch */
#define SYS_create_pmos 20
#define SYS_map_pmos    21
/* - address translation */
#define SYS_get_pmo_paddr 30
#define SYS_get_phys_addr 31

/* Capability */
#define SYS_cap_copy_to   60
#define SYS_cap_copy_from 61
#define SYS_transfer_caps 62

/* Multitask */
/* - create & exit */
#define SYS_create_cap_group 80
#define SYS_create_thread    82
#define SYS_thread_exit      83

/* POSIX */
/* - memory */
#define SYS_handle_brk    210
#define SYS_handle_mmap   211
#define SYS_handle_munmap 212
