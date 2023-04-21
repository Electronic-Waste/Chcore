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

#include "server.h"

#include <chcore/types.h>
#include <chcore/assert.h>
#include <chcore/internal/server_caps.h>

static struct ipc_struct *procm_ipc_struct = NULL;

struct ipc_struct *get_procm_server(void)
{
        if (!procm_ipc_struct) {
                int procm_cap = __chcore_get_procm_cap();
                chcore_assert(procm_cap >= 0);
                procm_ipc_struct = ipc_register_client(procm_cap);
                chcore_assert(procm_ipc_struct);
        }
        return procm_ipc_struct;
}
