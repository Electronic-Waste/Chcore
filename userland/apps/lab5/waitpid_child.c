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

#include <stdio.h>
#include <chcore/internal/raw_syscall.h>

int sleep(int time)
{
        for (int i = 0; i < time; i++) {
                __chcore_sys_yield();
        }
        return 0;
}

int main(int argc, char* argv[])
{
        printf("Child Start!\n");
        sleep(10);
        printf("Child Return!\n");
        return -10;
}
