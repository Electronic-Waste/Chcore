cmake_minimum_required(VERSION 3.11)

add_executable(sd.srv
    "emmc.c"
    "timer.c"
    "sd_server.c"
)

chcore_copy_all_targets_to_ramdisk()

