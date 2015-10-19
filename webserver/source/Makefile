all: webserver

webserver: webserver.o tcp.o request.o
	gcc webserver.o tcp.o request.o -o webserver -g -lpthread

webserver.o: webserver.c
	gcc -Wall -g -c webserver.c -o webserver.o
 
tcp.o: tcp.c tcp.h
	gcc -Wall -g -c tcp.c -o tcp.o

request.o: request.c request.h
	gcc -Wall -g -c request.c -o request.o

clean:
	rm -f *.o webserver
