/*
 * Copyright (c) 2022 Institute of Parallel And Distributed Systems (IPADS)
 * ChCore-Lab is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *     http://license.coscl.org.cn/MulanPSL
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 * PURPOSE.
 * See the Mulan PSL v1 for more details.
 */

#include "proc.h"
#include <errno.h>
#include <stdio.h>
#include <chcore/internal/raw_syscall.h>

struct proc proc_list[PID_MAX];

void init_process_manager(void)
{
        int i;
        for (i = 0; i < PID_MAX; i++) {
                proc_list[i].state = PROC_INIT;
                proc_list[i].sem = -1;
                proc_list[i].ret = 0;
        }
}

int internal_init_proc(int pid)
{
        int sem;

        if (pid > PID_MAX)
                return -EINVAL;
        if (proc_list[pid].state != PROC_INIT)
                return -EINVAL;
        /* Create per proc semaphore. */
        if ((sem = __chcore_sys_create_sem()) < 0)
                return sem;
        proc_list[pid].sem = sem;
        proc_list[pid].state = PROC_RUNNING;
}

int internal_exit(int pid, int ret)
{
        if (pid > PID_MAX)
                return -EINVAL;
        if (proc_list[pid].state != PROC_RUNNING)
                return -EINVAL;
        proc_list[pid].state = PROC_EXIT;
        proc_list[pid].ret = ret;
        __chcore_sys_signal_sem(proc_list[pid].sem);
}

/* Only support 1 process wait */
int internal_waitpid(int pid)
{
        if (pid > PID_MAX)
                return -EINVAL;

        __chcore_sys_wait_sem(proc_list[pid].sem, 1);
        return proc_list[pid].ret;
}
