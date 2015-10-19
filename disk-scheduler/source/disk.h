/*
This code by Prof. Douglas Thain at the University of Notre Dame
is licensed under a Creative Commons Attribution 4.0 International License.
You are welcome to reuse and adapt it as long as you give proper credit.
*/

#ifndef DISK_H
#define DISK_H

#define BLOCK_SIZE 4096

/*
Create a new virtual disk in the file "filename", with the given number of blocks.
Returns a pointer to a new disk object, or null on failure.
*/

struct disk * disk_create( const char *filename, int blocks );

/*
Write exactly BLOCK_SIZE bytes to a given block on the virtual disk.
"d" must be a pointer to a virtual disk, "block" is the block number,
and "data" is a pointer to the data to write.  Only one thread may
call this function at once, or the disk will crash!
*/

void disk_write( struct disk *d, int block, const char *data );

/*
Read exactly BLOCK_SIZE bytes from a given block on the virtual disk.
"d" must be a pointer to a virtual disk, "block" is the block number,
and "data" is a pointer to where the data will be placed.  Only one thread may
call this function at once, or the disk will crash!
*/

void disk_read( struct disk *d, int block, char *data );

/*
Return the number of blocks in the virtual disk.
*/

int disk_nblocks( struct disk *d );

/*
Close the virtual disk.
*/

void disk_close( struct disk *d );

#endif
