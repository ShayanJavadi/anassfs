#include "yield.h"

int yield_fd()
{
  // Get first unused file descriptor.
  int fileno = -1;
  file_descriptor fd;
  for (int i = 0; i < NUM_FILE_DESCRIPTORS; i++) {
    fd = fdt[i];
    if (fd.inode == 0 && fd.rwptr == 0) {
      fileno = i;
      break;
    }
  }
  if (fileno == -1) {
    // Ran out of file descriptors.
    errno = EMFILE;
    perror("yield_fd");
    return -(errno);
  }

  return fileno;
}

int yield_file()
{
  for (int i = 0; i < NUM_INODES; i++) {
    if (directory[i].inode == 0) {
      return i;
    }
  }

  // No more available room.
  errno = ENOSPC;
  perror("yield_file()");
  return -(errno);
}

int yield_inode()
{
  for (int i = 1; i < NUM_INODES; i++) {
    inode_t node = table[i];
    if (node.link_cnt == 0) {
      table[i].link_cnt = 1;
      return i;
    }
  }

  // No free inodes available.
  errno = ENOSPC;
  perror("yield_inode()");
  return -(errno);
}

int yield_block()
{
  for (int i = 0; i < NUM_BLOCKS; i++) {
    if (free_blocks[i]) {
      free_blocks[i] = false;
      write_free_blocks();
      return i;
    }
  }

  // No free blocks available.
  errno = ENOSPC;
  perror("yield_block()");
  return -(errno);
}
