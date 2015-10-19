/*
This code by Prof. Douglas Thain at the University of Notre Dame
is licensed under a Creative Commons Attribution 4.0 International License.
You are welcome to reuse and adapt it as long as you give proper credit.
*/

/*
Do NOT modify this file.
Modify disk_scheduler.c instead.
*/

#ifndef DISK_SCHEDULER_H
#define DISK_SCHEDULER_H

typedef enum {
	DISK_SCHEDULER_MODE_FIFO,
	DISK_SCHEDULER_MODE_SSTF,
	DISK_SCHEDULER_MODE_SCAN
} disk_scheduler_mode_t;

struct disk_scheduler * disk_scheduler_create( const char *filename, int nblocks, disk_scheduler_mode_t mode );

int disk_scheduler_nblocks( struct disk_scheduler *s );

void disk_scheduler_write( struct disk_scheduler *s, int block, const char *data );
void disk_scheduler_read( struct disk_scheduler *s, int block, char *data );

void * disk_scheduler_run( struct disk_scheduler *s );

#endif
