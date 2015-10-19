/*
This code by Prof. Douglas Thain at the University of Notre Dame
is licensed under a Creative Commons Attribution 4.0 International License.
You are welcome to reuse and adapt it as long as you give proper credit.
*/

/*
Make all of your changes to this file.
*/

#include "disk_scheduler.h"
#include "disk.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

/*
A disk_scheduler object initially contains a pointer to a disk
and a record of the scheduling mode.  You may add additional
fields and data structures here as needed.
*/

struct disk_scheduler {
	struct disk *disk;
	disk_scheduler_mode_t mode;
};

/*
Create a new disk scheduler object and initialize its fields.
*/

struct disk_scheduler * disk_scheduler_create( const char *filename, int nblocks, disk_scheduler_mode_t mode )
{
	struct disk_scheduler *s = malloc(sizeof(*s));
	if(!s) return 0;

	s->disk = disk_create(filename,nblocks);
	if(!s->disk) {
		free(s);
		return 0;
	}

	s->mode = mode;

	return s;
}

/*
Return the number of blocks in the disk.
*/

int disk_scheduler_nblocks( struct disk_scheduler *s )
{
	return disk_nblocks(s->disk);
}

/*
This is just a placeholder for your implementation.
disk_scheduler_write should NOT call disk_write directly.
Instead, it should add a request object to a data structure,
wait for the scheduler thread to service it, and then return
back to the calling program.
*/

void disk_scheduler_write( struct disk_scheduler *s, int block, const char *data )
{
	printf("write block %d starting\n",block);
	disk_write(s->disk,block,data);
	printf("write block %d done\n",block);
}

/*
This is just a placeholder for your implementation.
disk_scheduler_read should work much like disk_scheduler_write.
*/

void disk_scheduler_read( struct disk_scheduler *s, int block, char *data )
{
	printf("read block %d starting\n",block);
	disk_read(s->disk,block,data);
	printf("read block %d done\n",block);
}

/*
This is just a placeholder for your implementation.
disk_scheduler_run should be an infinite loop that waits
for requests to be added to the data structure,
picks the appropriate next request, calls disk_read or disk_write
as needed, and then wakes up the program waiting upon the request.
*/

void * disk_scheduler_run( struct disk_scheduler *s )
{
	return 0;
}
