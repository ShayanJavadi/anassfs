anassfs
=======

Welcome to `anassfs` -- the Anass File System. This is a simple file system implementation running on FUSE.

Compiling
---------
Just run:

```bash
make
```

Installing
----------
Just run:

```bash
sudo make install
```

Running
-------
Just run:

```bash
anassfs /path/to/folder/to/mount/to
```

if you have installed the executable, or

```bash
./anassfs /path/to/folder/to/mount/to
```

otherwise.

Features
--------
- Create files
- Read files
- Write files
- Append files
- Delete files

TODO
----
- [ ] Multi-level directories
- [ ] Permission levels
- [ ] Timestamps
- [ ] Symbolic links
- [ ] Resizing disk
- [ ] Backdoor
