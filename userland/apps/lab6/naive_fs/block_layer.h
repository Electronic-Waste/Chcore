#pragma once

#define BLOCK_SIZE 512

int sd_bread(int lba, char *buffer);
int sd_bwrite(int lba, const char *buffer);
