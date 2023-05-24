#include <string.h>
#include <stdio.h>

#include "file_ops.h"
#include "block_layer.h"

int naive_fs_access(const char *name)
{
    /* LAB 6 TODO BEGIN */
    /* BLANK BEGIN */
    char dentry_buf[BLOCK_SIZE];
    char name_buf[24];
    sd_bread(1, dentry_buf);

    // Find access to file 
    int start_pos = 0, end_pos = 0;
    while (1) {
        while (dentry_buf[end_pos] != '/' 
                && end_pos < BLOCK_SIZE) end_pos++;
        if (end_pos >= BLOCK_SIZE) {
            printf("naive_fs_access: does not match!\n");
            return -1;
        }
        // printf("start_pos: %d, end_pos: %d\n", start_pos, end_pos);
        memcpy(name_buf, dentry_buf + start_pos, end_pos - start_pos);
        name_buf[end_pos - start_pos] = '\0';
        // printf("name_buf: %s\n", name_buf);
        start_pos = ++end_pos;
        // If Match
        if (strcmp(name, name_buf) == 0) {
            printf("naive_fs_access: match!\n");
            return 0;
            // while (dentry_buf[end_pos] != '/') end_pos++;
            // int inode_num = 0;
            // for (int i = start_pos; i < end_pos; ++i) {
            //     inode_num = inode_num * 10 + (dentry_buf[i] - '0');
            // }
            // return inode_num;
        }
        // If not match, skip following inode num
        else {
            while (dentry_buf[end_pos] != '/') end_pos++;
            start_pos = ++end_pos;
        }
    }
    /* BLANK END */
    /* LAB 6 TODO END */
    return -2;
}

int naive_fs_creat(const char *name)
{
    /* LAB 6 TODO BEGIN */
    /* BLANK BEGIN */
    if (naive_fs_access(name) == 0) return -1; // File already exists
    
    char inode_map[BLOCK_SIZE];
    char dentry_buf[BLOCK_SIZE];
    sd_bread(0, inode_map);
    sd_bread(1, dentry_buf);

    // Find new inode for the file 
    int inode_num = 2;
    for (; inode_num < 64; ++inode_num) {
        if (inode_map[inode_num] == 0) {
            inode_map[inode_num] = 1;
            break;
        }
    }
    // Write dentry to dir
    int prev_pos = 0, cur_pos = 0;
    while (cur_pos < BLOCK_SIZE) {
        while (dentry_buf[cur_pos] != '/' 
                && cur_pos < BLOCK_SIZE) cur_pos++;
        // Dentry dir is empty now, start from the beginning
        if (cur_pos >= BLOCK_SIZE) {
            break;
        }
        // Record last '/''s next character pos
        prev_pos = ++cur_pos;
        // printf("prev_pos: %d, cur_pos: %d\n", prev_pos, cur_pos);
    }
    int len_name = strlen(name);
    // printf("name: %s, len_name: %d\n", name, len_name);
    int len_inode_num = 0;
    char inode_num_buf[2];
    if (inode_num / 10 == 0) {
        len_inode_num = 1;
        inode_num_buf[0] = inode_num + '0';
    }
    else {
        len_inode_num = 2;
        inode_num_buf[0] = inode_num / 10 + '0';
        inode_num_buf[1] = inode_num % 10 + '0';
    }
    memcpy(dentry_buf + prev_pos, name, len_name);
    memcpy(dentry_buf + prev_pos + len_name, "/", 1);
    memcpy(dentry_buf + prev_pos + len_name + 1, inode_num_buf, len_inode_num);
    memcpy(dentry_buf + prev_pos + len_name + len_inode_num + 1, "/", 1);
    // Update inode_map and dentry_buf
    // printf("dentry buf: ");
    // for (int i = 0; i < prev_pos + len_name + len_inode_num + 2; ++i) {
    //     printf("%c", dentry_buf[i]);
    // }
    // printf("\n");
    sd_bwrite(0, inode_map);
    sd_bwrite(1, dentry_buf);
    return 0;
    /* BLANK END */
    /* LAB 6 TODO END */
    return -2;
}

int naive_fs_pread(const char *name, int offset, int size, char *buffer)
{
    /* LAB 6 TODO BEGIN */
    /* BLANK BEGIN */
    if (naive_fs_access(name) != 0) return -1;  // Error file does not exist

    char dentry_buf[BLOCK_SIZE];
    char name_buf[24];
    sd_bread(1, dentry_buf);
    
    // Get inode num
    int start_pos = 0, end_pos = 0;
    int inode_num = 0;
    while (1) {
        while (dentry_buf[end_pos] != '/' 
                && end_pos < BLOCK_SIZE) end_pos++;
        if (end_pos >= BLOCK_SIZE) {
            printf("naive_fs_pread: can't find dentry!\n");
            return -1;
        }
        memcpy(name_buf, dentry_buf + start_pos, end_pos - start_pos);
        name_buf[end_pos - start_pos] = '\0';
        start_pos = ++end_pos;
        // If Match
        if (strcmp(name, name_buf) == 0) {
            printf("naive_fs_pread: match!\n");
            while (dentry_buf[end_pos] != '/') end_pos++;
            for (int i = start_pos; i < end_pos; ++i) {
                inode_num = inode_num * 10 + (dentry_buf[i] - '0');
            }
            break;
        }
        // If not match, skip following inode num
        else {
            while (dentry_buf[end_pos] != '/') end_pos++;
            start_pos = ++end_pos;
        }
    }

    // Read inode & the content
    char inode[BLOCK_SIZE];
    char block[BLOCK_SIZE];
    sd_bread(inode_num, inode);
    int *block_num = inode;
    sd_bread(*block_num, block);
    memcpy(buffer, block + offset, size);
    return size;
    /* BLANK END */
    /* LAB 6 TODO END */
    return -2;
}

int naive_fs_pwrite(const char *name, int offset, int size, const char *buffer)
{
    /* LAB 6 TODO BEGIN */
    /* BLANK BEGIN */
    if (naive_fs_access(name) != 0) return -1;  // Error file does not exist

    char inode_map[BLOCK_SIZE];
    char dentry_buf[BLOCK_SIZE];
    char name_buf[24];
    sd_bread(0, inode_map);
    sd_bread(1, dentry_buf);
    
    // Get inode num
    int start_pos = 0, end_pos = 0;
    int inode_num = 0;
    while (1) {
        while (dentry_buf[end_pos] != '/' 
                && end_pos < BLOCK_SIZE) end_pos++;
        if (end_pos >= BLOCK_SIZE) {
            printf("naive_fs_pwrite: can't find dentry!\n");
            return -1;
        }
        memcpy(name_buf, dentry_buf + start_pos, end_pos - start_pos);
        name_buf[end_pos - start_pos] = '\0';
        start_pos = ++end_pos;
        // If Match
        if (strcmp(name, name_buf) == 0) {
            printf("naive_fs_pwrite: match!\n");
            while (dentry_buf[end_pos] != '/') end_pos++;
            for (int i = start_pos; i < end_pos; ++i) {
                inode_num = inode_num * 10 + (dentry_buf[i] - '0');
            }
            break;
        }
        // If not match, skip following inode num
        else {
            while (dentry_buf[end_pos] != '/') end_pos++;
            start_pos = ++end_pos;
        }
    }

    // Read inode & write to block
    char inode[BLOCK_SIZE];
    char block[BLOCK_SIZE];
    sd_bread(inode_num, inode);
    int *block_num = inode;
    if (*block_num < 64) {
        for (int i = 64; i < 512; ++i) {
            if (inode_map[i] == 0) {
                inode_map[i] = 1;
                *block_num = i;
            }
        }
        sd_bwrite(0, inode_map);
        sd_bwrite(inode_num, inode);
    }
    sd_bread(*block_num, block);
    memcpy(block + offset, buffer, size);
    sd_bwrite(*block_num, block);
    return size;
    /* BLANK END */
    /* LAB 6 TODO END */
    return -2;
}

int naive_fs_unlink(const char *name)
{
    /* LAB 6 TODO BEGIN */
    /* BLANK BEGIN */
    if (naive_fs_access(name) != 0) return -1;  // Error file does not exist

    char inode_map[BLOCK_SIZE];
    char dentry_buf[BLOCK_SIZE];
    char new_dentry_buf[BLOCK_SIZE];
    char name_buf[24];
    sd_bread(0, inode_map);
    sd_bread(1, dentry_buf);
    // Get inode num
    int start_pos = 0, end_pos = 0;
    int inode_num = 0;
    int new_dentry_buf_pos = 0;
    while (1) {
        while (dentry_buf[end_pos] != '/' 
                && end_pos < BLOCK_SIZE) end_pos++;
        if (end_pos >= BLOCK_SIZE) {
            printf("naive_fs_pwrite: can't find dentry!\n");
            return -1;
        }
        memcpy(name_buf, dentry_buf + start_pos, end_pos - start_pos);
        name_buf[end_pos - start_pos] = '\0';
        start_pos = ++end_pos;
        // If Match
        if (strcmp(name, name_buf) == 0) {
            printf("naive_fs_pwrite: match!\n");
            while (dentry_buf[end_pos] != '/') end_pos++;
            for (int i = start_pos; i < end_pos; ++i) {
                inode_num = inode_num * 10 + (dentry_buf[i] - '0');
            }
        }
        // If not match, skip following inode num and copy them to new_dentry_buf
        else {
            while (dentry_buf[end_pos] != '/') end_pos++;
            memcpy(new_dentry_buf + new_dentry_buf_pos, dentry_buf + start_pos, end_pos - start_pos + 1);
            new_dentry_buf_pos += end_pos - start_pos + 1;
            start_pos = ++end_pos;
        }
    }
    
    // Read inode
    char inode[BLOCK_SIZE];
    char block[BLOCK_SIZE];
    sd_bread(inode_num, inode);
    int block_num = * (int *) inode;
    sd_bread(block_num, block);
    memset(inode, 0, BLOCK_SIZE);
    memset(block, 0, BLOCK_SIZE);
    inode_map[inode_num] = 0;
    inode_map[block_num] = 0;

    // Update to sd card
    sd_bwrite(0, inode_num);
    sd_bwrite(1, new_dentry_buf);
    sd_bwrite(inode_num, inode);
    sd_bwrite(block_num, block);
    /* BLANK END */
    /* LAB 6 TODO END */
    return -2;
}
