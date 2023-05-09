#pragma once
#include <chcore/types.h>
#include <chcore/ipc.h>

#define BLOCK_SIZE 512

enum sd_ipc_request {
	SD_IPC_REQ_READ,
	SD_IPC_REQ_WRITE,
	SD_IPC_REQ_MAX
};

struct sd_ipc_data {
	enum sd_ipc_request request;
	int lba;
	char data[BLOCK_SIZE];
};

void chcore_connect_sd_server();
int chcore_sd_read(int lba, char *buffer);
int chcore_sd_write(int lba, const char *buffer);