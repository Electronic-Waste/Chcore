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

#include <chcore/procm.h>
#include <chcore/assert.h>
#include <string.h>
#include <stdio.h>

#include "server.h"

int chcore_procm_exit(int ret)
{
        struct ipc_struct *procm_ipc_struct = get_procm_server();
        struct ipc_msg *ipc_msg = ipc_create_msg(
                procm_ipc_struct, sizeof(struct procm_ipc_data), 0);
        chcore_assert(ipc_msg);

        struct procm_ipc_data *procm_ipc_data =
                (struct procm_ipc_data *)ipc_get_msg_data(ipc_msg);
        procm_ipc_data->request = PROCM_IPC_REQ_EXIT;
        procm_ipc_data->exit.ret = ret;
        ret = ipc_call(procm_ipc_struct, ipc_msg);
        ipc_destroy_msg(procm_ipc_struct, ipc_msg);
        return ret;
}
