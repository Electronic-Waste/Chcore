#!/usr/bin/python3

import os

SD_IMG_PATH = os.path.dirname(__file__) + "/../../../build/sd.img"
print(SD_IMG_PATH)
BLOCK_SIZE = 512
TEST_BLOCK_NUM = 512

def readblock(lba):
    with open(SD_IMG_PATH, "rb") as f:
        f.seek(lba * BLOCK_SIZE)
        data = f.read(BLOCK_SIZE)
        return data

def check_sd_img():
    for lba in range(TEST_BLOCK_NUM):
        block_data = readblock(lba)
        for k in range(BLOCK_SIZE):
            if block_data[k] != (10 * lba + k) % 120:
                return -1
    return 0

if check_sd_img() == 0:
    print("host_sd_io_test pass!")
else:
    print("host_sd_io_test fail!")
