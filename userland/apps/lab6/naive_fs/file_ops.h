#pragma once

int naive_fs_access(const char *name);
int naive_fs_creat(const char *name);
int naive_fs_pread(const char *name, int offset, int size, char *buffer);
int naive_fs_pwrite(const char *name, int offset, int size, const char *buffer);
int naive_fs_unlink(const char *name);