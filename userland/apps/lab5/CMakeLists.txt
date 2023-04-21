add_executable(helloworld.bin helloworld.c)
add_executable(waitpid_child.bin waitpid_child.c)
add_executable(waitpid.bin waitpid.c)
add_executable(lab5.bin lab5.c)

chcore_copy_all_targets_to_ramdisk()
chcore_copy_files_to_ramdisk(test.txt)
