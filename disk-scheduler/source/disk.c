/*
This code by Prof. Douglas Thain at the University of Notre Dame
is licensed under a Creative Commons Attribution 4.0 International License.
You are welcome to reuse and adapt it as long as you give proper credit.
*/

#include "disk.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

extern ssize_t pread (int __fd, void *__buf, size_t __nbytes, __off_t __offset);
extern ssize_t pwrite (int __fd, const void *__buf, size_t __nbytes, __off_t __offset);

#define ABS(x) ( (x)>=0 ? (x) : -(x) )

struct disk {
	int fd;
	int block_size;
	int nblocks;
	int busy;
	int last_block;
	pthread_mutex_t mutex;
};

struct disk * disk_create( const char *diskname, int nblocks )
{
	struct disk *d;

	d = malloc(sizeof(*d));
	if(!d) return 0;

	d->fd = open(diskname,O_CREAT|O_RDWR,0777);
	if(d->fd<0) {
		free(d);
		return 0;
	}

	d->block_size = BLOCK_SIZE;
	d->nblocks = nblocks;
	d->busy = 0;
	d->last_block = 0;

	pthread_mutex_init(&d->mutex,0);

	if(ftruncate(d->fd,d->nblocks*d->block_size)<0) {
		close(d->fd);
		free(d);
		return 0;
	}

	return d;
}

static void disk_seek( struct disk *d, int block )
{
	usleep( 1000*ABS(d->last_block-block) );
	d->last_block = block;
}

void disk_write( struct disk *d, int block, const char *data )
{
	pthread_mutex_lock(&d->mutex);
	if(d->busy) {
		fprintf(stderr,"DISK CRASH: two threads tried to use the disk at once!\n");
		abort();
	}
	d->busy = 1;
	pthread_mutex_unlock(&d->mutex);

	disk_seek(d,block);

	if(block<0 || block>=d->nblocks) {
		fprintf(stderr,"DISK CRASH: invalid block #%d\n",block);
		abort();
	}

	int actual = pwrite(d->fd,data,d->block_size,block*d->block_size);
	if(actual!=d->block_size) {
		fprintf(stderr,"DISK CRASH: failed to write block #%d: %s\n",block,strerror(errno));
		abort();
	}

	pthread_mutex_lock(&d->mutex);
	d->busy = 0;
	pthread_mutex_unlock(&d->mutex);
}

void disk_read( struct disk *d, int block, char *data )
{
	pthread_mutex_lock(&d->mutex);
	if(d->busy) {
		fprintf(stderr,"DISK CRASH: two threads tried to use the disk at once!\n");
		abort();
	}
	d->busy = 1;
	pthread_mutex_unlock(&d->mutex);

	disk_seek(d,block);

	if(block<0 || block>=d->nblocks) {
		fprintf(stderr,"DISK CRASH: invalid block #%d\n",block);
		abort();
	}

	int actual = pread(d->fd,data,d->block_size,block*d->block_size);
	if(actual!=d->block_size) {
		fprintf(stderr,"DISK CRASH: failed to read block #%d: %s\n",block,strerror(errno));
		abort();
	}

	pthread_mutex_lock(&d->mutex);
	d->busy = 0;
	pthread_mutex_unlock(&d->mutex);
}

int disk_nblocks( struct disk *d )
{
	return d->nblocks;
}

void disk_close( struct disk *d )
{
	close(d->fd);
	free(d);
}
