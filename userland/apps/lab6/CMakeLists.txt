add_subdirectory(naive_fs)

add_executable(sd_driver_test.bin sd_driver_test.c)

add_executable(naive_fs_test.bin naive_fs_test.c)
target_link_libraries(naive_fs_test.bin PRIVATE fs_layer)

chcore_copy_all_targets_to_ramdisk()