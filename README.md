TinyFS
======
Design decisions
----------------


Superblock - block 0


byte        usage

0           block type
1           0x45
2           pointer to first inode
4-9         bit vector for empty blocks


inode


byte        usage
0           block type
1           0x45
2           next inode in inode list
3           pointer to data (if more exists, null otherwise)
4           file size in number of blocks
5-14        name of file



Data block


byte        usage
0           block type
1           0x45
2           pointer to next data block
4           data starts
