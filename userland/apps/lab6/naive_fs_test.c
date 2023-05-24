#include <chcore/sd.h>
#include <chcore/assert.h>
#include <stdio.h>
#include <string.h>

#include "naive_fs/block_layer.h"
#include "naive_fs/file_ops.h"

#define TEST_FUNC(name) \
    do { \
        if (name() == 0) { \
            printf(#name" pass!\n"); \
        } else { \
            printf(#name" fail!\n"); \
        } \
    } while (0)

int sd_bread(int lba, char *buffer)
{
    printf("block read %d\n", lba);
    return chcore_sd_read(lba, buffer);
}

int sd_bwrite(int lba, const char *buffer)
{
    printf("block write %d\n", lba);
    return chcore_sd_write(lba, buffer);
}

static int test_naive_fs_empty()
{
    int ret;
    char pn_buf[24];

    strcpy(pn_buf, "testfile0");
    for (int i = 0; i < 10; ++i) {
        ret = naive_fs_access(pn_buf);
        if (ret != -1)
            return -1;
        pn_buf[8]++;
    }

    return 0;
}

static int test_naive_fs_creat()
{
    char pn_buf[24];

    strcpy(pn_buf, "testfile0");
    for (int i = 0; i < 10; ++i) {
        printf("1\n");
        if (naive_fs_access(pn_buf) != -1)
            return -1;
        printf("2\n");
        if (naive_fs_creat(pn_buf) != 0)
            return -1;
        printf("3\n");
        if (naive_fs_access(pn_buf) != 0)
            return -1;
        printf("4\n");
        pn_buf[8]++;
    }

    return 0;
}

static int test_naive_fs_read_write()
{
    char pn_buf[24];
    char buffer[BLOCK_SIZE * 2];
    char read_buffer[BLOCK_SIZE];

    for (int i = 0; i < BLOCK_SIZE * 2; ++i) {
        buffer[i] = i % 131;
    }

    strcpy(pn_buf, "testfile0");
    for (int i = 0; i < 10; ++i) {
        if (naive_fs_pwrite(pn_buf, 0, BLOCK_SIZE, buffer + 14 * i) != BLOCK_SIZE)
            return -1;
        if (naive_fs_pread(pn_buf, 20 + i, BLOCK_SIZE / 2, read_buffer) != BLOCK_SIZE / 2)
            return -1;
        for (int k = 0; k < BLOCK_SIZE / 2; ++k) {
            if (read_buffer[k] != (14 * i + 20 + i + k) % 131) {
                printf("[%d] %d != %d\n", k, read_buffer[k], (14 * i + 20 + i + k) % 131);
                return -1;
            }
        }
        pn_buf[8]++;
    }

    return 0;
}

int main()
{
    chcore_connect_sd_server();

    TEST_FUNC(test_naive_fs_empty);
    TEST_FUNC(test_naive_fs_creat);
    TEST_FUNC(test_naive_fs_read_write);

    return 0;
}