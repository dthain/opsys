/*
This code by Prof. Douglas Thain at the University of Notre Dame
is licensed under a Creative Commons Attribution 4.0 International License.
You are welcome to reuse and adapt it as long as you give proper credit.
*/

#include "program.h"
#include "disk_scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void * program_run( struct disk_scheduler *s )
{
	int nblocks = disk_scheduler_nblocks(s);
	char data[4096];
	int i;

	for(i=0;i<100;i++) {
		int readorwrite = rand() % 2;
		int block = rand() % nblocks;

		if(readorwrite) {
			disk_scheduler_read(s,block,data);
		} else {
			disk_scheduler_write(s,block,data);
		}
	}

	return 0;
}
