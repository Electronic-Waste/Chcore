/*
 * Copyright (c) 2022 Institute of Parallel And Distributed Systems (IPADS)
 * ChCore-Lab is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *     http://license.coscl.org.cn/MulanPSL
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 * PURPOSE.
 * See the Mulan PSL v1 for more details.
 */

#include <stdio.h>
#include <string.h>
#include <chcore/types.h>
#include <chcore/fsm.h>
#include <chcore/tmpfs.h>
#include <chcore/ipc.h>
#include <chcore/internal/raw_syscall.h>
#include <chcore/internal/server_caps.h>
#include <chcore/procm.h>
#include <chcore/fs/defs.h>

typedef __builtin_va_list va_list;
#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v)      __builtin_va_end(v)
#define va_arg(v, l)   __builtin_va_arg(v, l)
#define va_copy(d, s)  __builtin_va_copy(d, s)


extern struct ipc_struct *fs_ipc_struct;

/* You could add new functions or include headers here.*/
/* LAB 5 TODO BEGIN */
// #include <FILE.h>

int next_fd = 0;

int get_fd() {
	return next_fd++;
}

struct ipc_msg *create_ipc_msg_with_fr(struct fs_request *fr, const void *src, size_t size) {
	if (fr == NULL || fr->req <= FS_REQ_UNDEFINED || fr->req > FS_REQ_GET_FS_CAP) {
		debug("error in fs_request\n");
		return NULL;
	}
	if (fr->req != FS_REQ_WRITE && src != NULL) {
		debug("error: only fwrite needs src");
		return NULL;
	}
	size_t msg_len = (fr->req == FS_REQ_WRITE) ? sizeof(struct fs_request) + size : sizeof(struct fs_request);
	struct ipc_msg *msg = ipc_create_msg(fs_ipc_struct, msg_len, 0);
	ipc_set_msg_data(msg, (void *)fr, 0, sizeof(struct fs_request));
	/* fwrite only */
	if (fr->req == FS_REQ_WRITE) {
		ipc_set_msg_data(msg, (void *)src, sizeof(struct fs_request), size);
	}
	return msg;
}
/* LAB 5 TODO END */


FILE *fopen(const char * filename, const char * mode) {

	/* LAB 5 TODO BEGIN */
	/* Fill in fs_request & ipc_msg */
	struct fs_request *fr = calloc(1, sizeof(struct fs_request));
	struct ipc_msg *msg = NULL;
	fr->req = FS_REQ_OPEN;
	fr->open.mode = *(unsigned int *) mode;
	fr->open.new_fd = get_fd();
	fr->open.flags = 0;
	memcpy(fr->open.pathname, filename, strlen(filename) + 1);
	msg = create_ipc_msg_with_fr(fr, NULL, 0);
	if (msg == NULL) {
		debug("error in create ipc_msg");
		return NULL;
	}
	debug("msg addr: %llx\n", msg);
	struct fs_request *debug_fr = (struct fs_request *) ipc_get_msg_data(msg);
	debug("fr type: %d, fd: %d, pathname: %s\n", debug_fr->req, debug_fr->open.new_fd, debug_fr->open.pathname);

	/* Send ipc_call and construct FILE */
	FILE *f = calloc(1, sizeof(FILE));
	s64 ret = ipc_call(fs_ipc_struct, msg);
	if (ret != fr->open.new_fd) {
		debug("error in ipc_call\n");
		return NULL;
	}
	f->fd = ret;
	free(fr);
	free(msg);
	return f;
	/* LAB 5 TODO END */
    return NULL;
}

size_t fwrite(const void * src, size_t size, size_t nmemb, FILE * f) {

	/* LAB 5 TODO BEGIN */
	/* Fill in fs_request & ipc_msg */
	struct fs_request *fr = calloc(1, sizeof(struct fs_request));
	struct ipc_msg *msg = NULL;
	fr->req = FS_REQ_WRITE;
	fr->write.fd = f->fd;
	fr->write.count = size;
	msg = create_ipc_msg_with_fr(fr, src, size * nmemb);
	if (msg < 0) {
		debug("error in create ipc_msg");
		return -1;
	}

	/* Send ipc_call and construct FILE */
	s64 bytes_written = ipc_call(fs_ipc_struct, msg);
	return (size_t) bytes_written;
	/* LAB 5 TODO END */
    return 0;

}

size_t fread(void * destv, size_t size, size_t nmemb, FILE * f) {

	/* LAB 5 TODO BEGIN */
	/* Fill in fs_request & ipc_msg */
	struct fs_request *fr = calloc(1, sizeof(struct fs_request));
	struct ipc_msg *msg = NULL;
	fr->req = FS_REQ_READ;
	fr->read.fd = f->fd;
	fr->read.count = size * nmemb;
	msg = create_ipc_msg_with_fr(fr, NULL, 0);
	if (msg < 0) {
		debug("error in create ipc_msg");
		return -1;
	}

	/* Send ipc_call and construct FILE */
	s64 bytes_read = ipc_call(fs_ipc_struct, msg);
	memcpy(destv, fr, bytes_read);
	return (size_t) bytes_read;
	/* LAB 5 TODO END */
    return 0;

}

int fclose(FILE *f) {

	/* LAB 5 TODO BEGIN */
	/* Fill in fs_request & ipc_msg */
	struct fs_request *fr = calloc(1, sizeof(struct fs_request));
	struct ipc_msg *msg = NULL;
	fr->close.fd = f->fd;
	msg = create_ipc_msg_with_fr(fr, NULL, 0);
	if (msg < 0) {
		debug("error in create ipc_msg");
		return -1;
	}

	/* Send ipc_call and construct FILE */
	int ret = ipc_call(fs_ipc_struct, msg);
	if (ret != 0) {
		debug("error in ipc_call");
		return ret;
	}
	/* LAB 5 TODO END */
    return 0;

}

/* Need to support %s and %d. */
int fscanf(FILE * f, const char * fmt, ...) {

	/* LAB 5 TODO BEGIN */
	// char *buf = calloc(1, FS_READ_BUF_SIZE);
	// size_t bytes_read = fread(buf, FS_READ_BUF_SIZE, 1, f);
	
	/* LAB 5 TODO END */
    return 0;
}

/* Need to support %s and %d. */
int fprintf(FILE * f, const char * fmt, ...) {

	/* LAB 5 TODO BEGIN */
	
	
	/* LAB 5 TODO END */
    return 0;
}

