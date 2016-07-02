#ifndef __YIELD_H
#define __YIELD_H

#include <errno.h>
#include <stdint.h>

#include "sfs_api.h"

int yield_fd();
int yield_file();
int yield_inode();
int yield_block();

#endif
