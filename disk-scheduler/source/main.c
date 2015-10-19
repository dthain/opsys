/*
This code by Prof. Douglas Thain at the University of Notre Dame
is licensed under a Creative Commons Attribution 4.0 International License.
You are welcome to reuse and adapt it as long as you give proper credit.
*/

/*
Main program for the disk scheduler project.
You should NOT make any changes to this file.
Modify disk_scheduler.c instead.
*/

#include "disk_scheduler.h"
#include "program.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

int main( int argc, char *argv[] )
{
	if(argc!=4) {
		printf("use: disksched <nthreads> <nblocks> <fifo|sstf|scan>\n");
		return 1;
	}

	int nthreads = atoi(argv[1]);
	int nblocks = atoi(argv[2]);

	disk_scheduler_mode_t mode;

	if(!strcmp(argv[3],"fifo")) {
		mode = DISK_SCHEDULER_MODE_FIFO;
	} else if(!strcmp(argv[3],"sstf")) {
		mode = DISK_SCHEDULER_MODE_SSTF;
	} else if(!strcmp(argv[3],"scan")) {
		mode = DISK_SCHEDULER_MODE_SCAN;
	} else {
		printf("illegal scheduling mode: %s\n",argv[3]);
		return 1;
	}

	srand(time(0));

	struct disk_scheduler *s = disk_scheduler_create("myvirtualdisk",nblocks,mode);
	if(!s) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	}

	pthread_t program_thread[nthreads];
	pthread_t scheduler_thread;
	int i;

	printf("main: starting program threads\n");
	for(i=0;i<nthreads;i++) {
		pthread_create(&program_thread[i],0,(void*)program_run,s);
	}

	printf("main: starting disk scheduler thread\n");
	pthread_create(&scheduler_thread,0,(void*)disk_scheduler_run,s);

	for(i=0;i<nthreads;i++) {
		pthread_join(program_thread[i],0);
	}

	printf("main: all program threads complete.\n");

	return 0;
}
