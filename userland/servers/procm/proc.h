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

#pragma once

#define PID_MAX (1 << 10)
#define PID_MIN 10 /* reserved */

enum proc_state {
        PROC_INIT,
        PROC_RUNNING,
        PROC_EXIT,
        PROC_STATE_NUM,
};

struct proc {
        int sem;
        int ret;
        enum proc_state state;
};

void init_process_manager(void);
int internal_init_proc(int pid);
int internal_exit(int pid, int ret);
int internal_waitpid(int pid);
