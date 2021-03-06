# EXECUTABLE NAME.
EXECUTABLE ?= anassfs

# DISK CONFIGURATION.
DISK ?= $(EXECUTABLE).disk
BLOCK_SZ ?= 1024
NUM_INODES ?= 1000
NUM_BLOCKS ?= 10000
MAXFILENAME ?= 60
NUM_FILE_DESCRIPTORS ?= 1000

# COMPILER FLAGS.
CC = gcc
DEFS  = -DDISK='"$(DISK)"' -DMAXFILENAME=$(MAXFILENAME) -DBLOCK_SZ=$(BLOCK_SZ)
DEFS += -DNUM_BLOCKS=$(NUM_BLOCKS) -DNUM_INODES=$(NUM_INODES)
DEFS += -DNUM_FILE_DESCRIPTORS=$(NUM_FILE_DESCRIPTORS)
LDFLAGS =`pkg-config fuse --cflags --libs`
CFLAGS = -c -g -Wall -std=gnu99 -I'include' $(DEFS)

# MAIN.
MAIN = fuse.c
SRCS = $(shell find src -name '*.c')
SOURCES = $(SRCS) $(MAIN)

OBJECTS = $(SOURCES:.c=.o)

all: $(SOURCES) $(HEADERS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) -g $(OBJECTS) $(DEFS) $(LDFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

clean:
	rm -rf src/*.o *.o $(EXECUTABLE)

format:
	rm -rf $(DISK)

install: $(EXECUTABLE)
	cp $(EXECUTABLE) /usr/local/bin
