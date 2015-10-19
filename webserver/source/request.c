/*
This code by Prof. Douglas Thain at the University of Notre Dame
is licensed under a Creative Commons Attribution 4.0 International License.
You are welcome to reuse and adapt it as long as you give proper credit.
*/

#include "tcp.h"
#include "request.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define REQUEST_LINE_MAX 1024

struct request * request_create( struct tcp *conn )
{
	struct request *r;
	time_t stoptime = time(0)+300;

	char line[REQUEST_LINE_MAX];
	char url[REQUEST_LINE_MAX];
	char action[REQUEST_LINE_MAX];
	char path[REQUEST_LINE_MAX];
	char hostport[REQUEST_LINE_MAX];

	r = malloc(sizeof(*r));
	memset(r,0,sizeof(*r));

	r->conn = conn;

	if(!tcp_readline(r->conn,line,sizeof(line),stoptime)) {
		request_delete(r);
		return 0;
	}

	if(sscanf(line,"%s %s",action,url)!=2) {
		request_delete(r);
		return 0;
	}

	while(1) {
		if(!tcp_readline(conn,line,sizeof(line),stoptime)) {
			request_delete(r);
			return 0;
		}
		if(!line[0]) break;
	}

	if(sscanf(url,"http://%[^/]%s",hostport,path)==2) {
		// continue on
	} else {
		strcpy(path,url);
	}

	if(!path[1]) {
		r->filename = strdup("index.html");
	} else {
		r->filename = strdup(&path[1]);
	}

	return r;
}

void request_handle( struct request *r )
{
	FILE *file;
	const char *mimetype=0;
	char line[REQUEST_LINE_MAX];
	int length, actual;

	signal(SIGPIPE,SIG_IGN);

	char *filetype = strrchr(r->filename,'.');
	if(filetype) {
		filetype++;
		if(!strcmp(filetype,"jpg")) {
			mimetype = "image/jpeg";
		} else if(!strcmp(filetype,"html")) {
			mimetype = "text/html";
		} else if(!strcmp(filetype,"txt")) {
			mimetype = "text/plain";
		} else {
			mimetype = "application/binary";
		}
	} else {
		mimetype = "application/binary";
	}

	time_t current = time(0);

	file = fopen(r->filename,"r");
	if(file) {
		tcp_printf(r->conn,"HTTP/1.1 200 OK\n");
		tcp_printf(r->conn,"Date: %s",ctime(&current));
		tcp_printf(r->conn,"Server: wwwserver\n");
		tcp_printf(r->conn,"Connection: close\n");
		tcp_printf(r->conn,"Expires: 0\n");
		tcp_printf(r->conn,"Content-type: %s\n\n",mimetype);

		// artificially delay approx every three requests
		if(rand()%3==0) sleep(5);

		while(1) {
			length = fread(line,1,sizeof(line),file);
			if(length<=0) break;
			actual = tcp_write(r->conn,line,length,current+60);
			if(actual!=length) {
				printf("error sending data: %s\n",strerror(errno));
				break;
			}
		}

		fclose(file);
	} else {
		tcp_printf(r->conn,"HTTP/1.1 404 File Not Found\n");
		tcp_printf(r->conn,"Date: %s",ctime(&current));
		tcp_printf(r->conn,"Server: wwwserver\n");
		tcp_printf(r->conn,"Connection: close\n");
		tcp_printf(r->conn,"Content-type: text/html\n\n");
		tcp_printf(r->conn,"<b>File not found.</b>\n");
	}
}

void request_delete( struct request *r )
{
	if(r) {
		if(r->filename) free(r->filename);
		if(r->conn)     tcp_close(r->conn);
		free(r);
	}
}
