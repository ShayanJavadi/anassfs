#include "file.h"

uint64_t get_file(const char* name)
{
  for (int i = 0; i < NUM_INODES; i++) {
    file_t f = directory[i];
    if (strncmp(f.filename, name, MAXFILENAME) == 0) {
      return i;
    }
  }

  // No such file.
  return -1;
}

void clear_block(void* buf, int start, int end)
{
  for (int i = start; i < end; i++) {
    *(uint8_t*)(buf + i) = 0;
  }
}

int get_block_at(inode_t* inode, int i, bool create)
{
  int block_index = -1;

  // Get direct block.
  if (i < NUM_DIRECT_BLOCKS) {
    if (inode->direct_blocks[i]) {
      // It exists.
      return inode->direct_blocks[i];
    } else if (create) {
      // Yield free block.
      block_index = yield_block();
      if (block_index < 0) {
        errno = ENOSPC;
        return block_index;
      }

      // Add to inode.
      inode->direct_blocks[i] = block_index;
      write_inode_table();

      return block_index;
    }
  }

  // Not enough blocks.
  if (i >= MAX_BLOCKS_PER_INODE) {
    // File too big.
    errno = EFBIG;
    return block_index;
  }

  // Otherwise, get indirect block.
  if (i >= NUM_DIRECT_BLOCKS) {
    uint64_t* block;
    if (inode->indirect_block || create) {
      // Create buffer in memory.
      block = (uint64_t*)malloc(BLOCK_SZ);
      clear_block(block, 0, BLOCK_SZ);

      if (create && inode->indirect_block == 0) {
        // Create if needed.
        int indirect_block_index = yield_block();
        if (indirect_block_index < 0) {
          free(block);
          errno = ENOSPC;
          return indirect_block_index;
        }

        // Write to inode.
        inode->indirect_block = indirect_block_index;
        write_inode_table();
      }

      uint64_t indirect_block_ptr = i - NUM_DIRECT_BLOCKS;
      read_blocks(inode->indirect_block, 1, block);
      if (block[indirect_block_ptr] > 0) {
        block_index = block[indirect_block_ptr];
      } else if (create) {
        // Create if needed.
        block_index = yield_block();
        if (block_index > 0) {
          // Succeeded, so write to disk.
          block[indirect_block_ptr] = block_index;
          write_blocks(inode->indirect_block, 1, block);
        }
      }

      // Free memory.
      free(block);
    }
  }

  if (block_index < 0) {
    errno = EFAULT;
  }
  return block_index;
}

uint64_t read_file(file_descriptor* fd, void* buf, uint64_t length)
{
  // Store number of bytes read.
  uint64_t read = 0;
  int read_error;

  // Pull inode.
  inode_t* inode = &table[fd->inode];

  // Allocate room for a block size in memory.
  void* block = (void*)malloc(BLOCK_SZ);

  // Read from block at RW pointer until length is complete.
  int starting_block_index = fd->rwptr / BLOCK_SZ;
  int current_block_count = starting_block_index;
  while (length) {
    // Get block index.
    int block_index = get_block_at(inode, current_block_count, false);
    if (block_index < 0) {
      perror("read_file()");
      break;
    }

    // Read from block.
    clear_block(block, 0, BLOCK_SZ);
    read_error = read_blocks(block_index, 1, block);
    if (read_error < 0) {
      perror("read_file(): read chunk from block");
      break;
    }

    // Modify block in memory.
    for (int j = fd->rwptr % BLOCK_SZ; j < BLOCK_SZ; j++) {
      // Check if EOF.
      if (fd->rwptr == inode->size) {
        // Reached end of file. Done.
        break;
      }

      // Copy current byte into buffer.
      memcpy(buf, block + j, 1);

      // Increment pointers.
      fd->rwptr += 1;
      buf += 1;

      // Increment number of bytes read.
      read += 1;

      // Decrement length.
      length -= 1;
      if (length == 0) {
        // Done in the middle of a block.
        break;
      }
    }

    // Increment block.
    current_block_count += 1;
  }

  // Free memory buffer.
  free(block);

  // Return bytes read.
  return read;
}

uint64_t write_file(file_descriptor* fd, void* buf, uint64_t length)
{
  // Store number of bytes written.
  uint64_t written = 0;
  int read_error, write_error;

  // Pull inode.
  inode_t* inode = &table[fd->inode];

  // Allocate room for a block size in memory.
  void* block = (void*)malloc(BLOCK_SZ);

  // Write from block at RW pointer until length is complete.
  int starting_block_index = fd->rwptr / BLOCK_SZ;
  int current_block_count = starting_block_index;
  while (length) {
    // Get block index.
    int block_index = get_block_at(inode, current_block_count, true);
    if (block_index < 0) {
      perror("write_file()");
      break;
    }

    // Read from block.
    clear_block(block, 0, BLOCK_SZ);
    read_error = read_blocks(block_index, 1, block);
    if (read_error < 0) {
      perror("write_file(): read chunk from block");
      break;
    }

    // Modify block in memory.
    for (int j = fd->rwptr % BLOCK_SZ; j < BLOCK_SZ; j++) {
      // Copy current byte into buffer.
      memcpy(block + j, buf, 1);

      // Increment pointers.
      fd->rwptr += 1;
      buf += 1;

      // Increment number written.
      written += 1;

      // Decrement length.
      length -= 1;
      if (length == 0) {
        // Done in the middle of a block.
        break;
      }
    }

    // Write block.
    write_error = write_blocks(block_index, 1, block);
    if (write_error < 0) {
      perror("write_file(): write chunk to block");
      break;
    }

    // Increment block.
    current_block_count += 1;
  }

  // Free block.
  free(block);

  // Update inode size.
  if (fd->rwptr > inode->size) {
    inode->size = fd->rwptr;
  }

  // Write inodes and free blocks.
  write_inode_table();
  write_free_blocks();

  // Return number of bytes written.
  return written;
}
