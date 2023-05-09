#include <chcore/ipc.h>
#include <chcore/sd.h>
#include <chcore/procm.h>
#include <chcore/internal/server_caps.h>
#include <stdio.h>
#include <string.h>
#include <chcore/assert.h>

static struct ipc_struct *sd_ipc_struct = NULL;
static struct ipc_struct *procm_ipc_struct = NULL;

static void connect_procm_server(void)
{
        int procm_cap = __chcore_get_procm_cap();
        chcore_assert(procm_cap >= 0);
        procm_ipc_struct = ipc_register_client(procm_cap);
        chcore_assert(procm_ipc_struct);
}

void chcore_connect_sd_server()
{
    int ret;
    if (!procm_ipc_struct) {
        connect_procm_server();
    }
    if (!sd_ipc_struct) {
        struct ipc_msg *ipc_msg = ipc_create_msg(
                procm_ipc_struct, sizeof(struct procm_ipc_data), 0);
        chcore_assert(ipc_msg);
        struct procm_ipc_data *procm_ipc_data =
                (struct procm_ipc_data *)ipc_get_msg_data(ipc_msg);
        procm_ipc_data->request = PROCM_IPC_REQ_GET_SD_SERVER_CAP;
        ret = ipc_call(procm_ipc_struct, ipc_msg);
        chcore_assert(!ret);

        sd_ipc_struct = ipc_register_client(ipc_get_msg_cap(ipc_msg, 0));
        chcore_assert(sd_ipc_struct);

        ipc_destroy_msg(procm_ipc_struct, ipc_msg);
    }
}

int chcore_sd_read(int lba, char *buffer)
{
    int ret;

    struct ipc_msg *ipc_msg = ipc_create_msg(
            sd_ipc_struct, sizeof(struct sd_ipc_data), 0);
    chcore_assert(ipc_msg);

    struct sd_ipc_data *sd_ipc_data =
            (struct sd_ipc_data *)ipc_get_msg_data(ipc_msg);
    sd_ipc_data->request = SD_IPC_REQ_READ;
    sd_ipc_data->lba = lba;
    ret = ipc_call(sd_ipc_struct, ipc_msg);
    if (ret == 0) {
        memcpy(buffer, sd_ipc_data->data, BLOCK_SIZE);
    }

    ipc_destroy_msg(sd_ipc_struct, ipc_msg);

    return ret;
}

int chcore_sd_write(int lba, const char *buffer)
{
    int ret;

    struct ipc_msg *ipc_msg = ipc_create_msg(
            sd_ipc_struct, sizeof(struct sd_ipc_data), 0);
    chcore_assert(ipc_msg);

    struct sd_ipc_data *sd_ipc_data =
            (struct sd_ipc_data *)ipc_get_msg_data(ipc_msg);
    sd_ipc_data->request = SD_IPC_REQ_WRITE;
    sd_ipc_data->lba = lba;
    memcpy(sd_ipc_data->data, buffer, BLOCK_SIZE);
    ret = ipc_call(sd_ipc_struct, ipc_msg);

    ipc_destroy_msg(sd_ipc_struct, ipc_msg);
    return ret;
}