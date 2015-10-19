
disksched: main.o program.o disk_scheduler.o disk.o
	gcc main.o disk_scheduler.o disk.o program.o -o disksched -lpthread -lm

main.o: main.c
	gcc -Wall -g -c main.c -o main.o

disk_scheduler.o: disk_scheduler.c
	gcc -Wall -g -c disk_scheduler.c -o disk_scheduler.o

disk.o: disk.c
	gcc -Wall -g -c disk.c -o disk.o

program.o: program.c
	gcc -Wall -g -c program.c -o program.o


clean:
	rm -f *.o *~ disksched myvirtualdisk
