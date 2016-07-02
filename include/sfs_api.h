#ifndef __SFS_API_H
#define __SFS_API_H

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DISK
#define DISK "sfs.disk"
#endif

#ifndef BLOCK_SZ
#define BLOCK_SZ 1024
#endif

#ifndef NUM_BLOCKS
#define NUM_BLOCKS 100000
#endif

#ifndef NUM_INODES
#define NUM_INODES 10000
#endif

#ifndef NUM_FILE_DESCRIPTORS
#define NUM_FILE_DESCRIPTORS 1000
#endif

#ifndef MAXFILENAME
#define MAXFILENAME 60
#endif

#define NUM_DIRECT_BLOCKS 12

#define NUM_INODE_BLOCKS (sizeof(inode_t) * NUM_INODES / BLOCK_SZ + 1)
#define NUM_FILES_PER_BLOCK (BLOCK_SZ / (MAXFILENAME + sizeof(uint64_t)))
#define FREE_BLOCKS_STORAGE ((NUM_BLOCKS / BLOCK_SZ) + 1)
#define MAX_BLOCKS_PER_INODE (NUM_DIRECT_BLOCKS + BLOCK_SZ / sizeof(uint64_t))
#define NUM_INDIRECT_POINTERS (BLOCK_SZ / sizeof(uint8_t))

// Force byte packing on structs stored on disk.
#pragma pack(push, blocks, 1)
typedef struct {
  uint64_t magic;
  uint64_t block_size;
  uint64_t fs_size;
  uint64_t inode_table_len;
  uint64_t root_dir_inode;

  // Pad to block size length.
  uint8_t __RESERVED[BLOCK_SZ - 5 * sizeof(uint64_t)];
} superblock_t;

typedef struct {
  uint64_t mode;
  uint64_t link_cnt;
  uint64_t uid;
  uint64_t gid;
  uint64_t size;
  uint64_t direct_blocks[NUM_DIRECT_BLOCKS];
  uint64_t indirect_block;

  // Pad to block size length.
  uint8_t __RESERVED[BLOCK_SZ - ((6 + NUM_DIRECT_BLOCKS) * sizeof(uint64_t))];
} inode_t;

typedef struct {
  char filename[MAXFILENAME];
  uint64_t inode;
} file_t;

typedef struct {
  uint64_t inode;
  uint64_t rwptr;
} file_descriptor;

extern int errno;

superblock_t sb;
inode_t table[NUM_INODES];
file_descriptor root_dir;
file_descriptor fdt[NUM_FILE_DESCRIPTORS];
uint8_t seen_files[NUM_INODES];
file_t directory[NUM_INODES];
uint8_t free_blocks[NUM_BLOCKS];
#pragma pack(pop, blocks)

void write_directory();
void write_inode_table();
void write_free_blocks();

void mksfs(int format);
int sfs_getnextfilename(char* fname);
int sfs_getfilesize(const char* path);
int sfs_fopen(char* name);
int sfs_fclose(int fileID);
int sfs_fread(int fileID, char* buf, int length);
int sfs_fwrite(int fileID, const char* buf, int length);
int sfs_fseek(int fileID, int loc);
int sfs_remove(char* file);

#include "disk_emu.h"
#include "file.h"
#include "yield.h"

#endif
