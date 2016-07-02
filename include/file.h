#ifndef __FILE_H
#define __FILE_H

#include <errno.h>
#include <stdint.h>

#include "sfs_api.h"

uint64_t get_file(const char* name);
void clear_block(void* buf, int start, int end);
int get_block_at(inode_t* inode, int i, bool create);
uint64_t read_file(file_descriptor* fd, void* buffer, uint64_t length);
uint64_t write_file(file_descriptor* fd, void* buffer, uint64_t length);

#endif
