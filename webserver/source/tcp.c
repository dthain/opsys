/*
This code by Prof. Douglas Thain at the University of Notre Dame
is licensed under a Creative Commons Attribution 4.0 International License.
You are welcome to reuse and adapt it as long as you give proper credit.
*/

#include "tcp.h"

#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/un.h>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/poll.h>

#define BUFFER_SIZE 65536

#define MIN(x,y) (((x)<(y)) ? (x) : (y))

struct tcp {
	int fd;
	int read;
	int written;
	time_t last_used;
	char buffer[BUFFER_SIZE];
	int buffer_start;
	int buffer_length;
	char raddr[TCP_ADDRESS_MAX];
	int rport;
};

/*
When a tcp is dropped, we do not want to deal with a signal,
but we want the current system call to abort.  To accomplish this, we
send SIGPIPE to a dummy function instead of just blocking or ignoring it.
*/

static void signal_swallow( int num )
{
}

static int tcp_squelch()
{
	signal(SIGPIPE,signal_swallow);
	return 1;
}

int tcp_nonblocking( struct tcp *tcp, int onoff )
{
	int result;

	result = fcntl(tcp->fd,F_GETFL);
	if(result<0) return 0;

	if(onoff) {
		result |= O_NONBLOCK;
	} else {
		result &= ~(O_NONBLOCK);
	}

	result = fcntl(tcp->fd,F_SETFL,result);
	if(result<0) return 0;

	return 1;
}

static int errno_is_temporary( int e )
{
	if( e==EINTR || e==EWOULDBLOCK || e==EAGAIN || e==EINPROGRESS || e==EALREADY || e==EISCONN ) {
		return 1;
	} else {
		return 0;
	}
}

void string_from_ip_address( const unsigned char *bytes, char *str )
{
	sprintf(str,"%u.%u.%u.%u",
		(unsigned)bytes[0],
		(unsigned)bytes[1],
		(unsigned)bytes[2],
		(unsigned)bytes[3]);
}

int  string_to_ip_address( const char * str, unsigned char *bytes )
{
	unsigned a,b,c,d;
	int fields;

	fields = sscanf( str, "%u.%u.%u.%u", &a, &b, &c, &d );
	if(fields!=4) return 0;

	if( a>255 || b>255 || c>255 || d>255 ) return 0;

	bytes[0] = a;
	bytes[1] = b;
	bytes[2] = c;
	bytes[3] = d;

	return 1;
}

static int tcp_internal_sleep( struct tcp *tcp, struct timeval *timeout, int reading, int writing )
{
        int result;
        fd_set rfds, wfds;

        FD_ZERO(&rfds);
        if(reading) FD_SET(tcp->fd,&rfds);
                                                                                
        FD_ZERO(&wfds);
        if(writing) FD_SET(tcp->fd,&wfds);
                                                                                
        while(1) {
                result = select(tcp->fd+1,&rfds,&wfds,0,timeout);
                if(result>0) {
                        if( reading && FD_ISSET(tcp->fd,&rfds)) return 1;
                        if( writing && FD_ISSET(tcp->fd,&wfds)) return 1;
                } else if( result==0 ) {
                        return 0;
                } else if( errno_is_temporary(errno) ) {
                        continue;
                } else {
                        return 0;
                }
	}
}

int tcp_sleep( struct tcp *tcp, time_t stoptime, int reading, int writing )

{
	int timeout;
	struct timeval tm,*tptr;
	
	if(stoptime==0) {
		tptr = 0;
	} else {
		timeout = stoptime-time(0);
		if(timeout<1) timeout = 1;
		tm.tv_sec = timeout;
		tm.tv_usec = 0;
		tptr = &tm;
	}

	return tcp_internal_sleep(tcp,tptr,reading,writing);
}

int tcp_usleep( struct tcp *tcp, int usec, int reading, int writing )
{
        struct timeval tm;

	tm.tv_sec = 0;
	tm.tv_usec = usec;

        return tcp_internal_sleep(tcp,&tm,reading,writing);
}

static struct tcp * tcp_create()
{
	struct tcp *tcp;

	tcp = malloc(sizeof(*tcp));
	if(!tcp) return 0;

	tcp->read = tcp->written = 0;
	tcp->last_used = time(0);
	tcp->fd = -1;
	tcp->buffer_start = 0;
	tcp->buffer_length = 0;
	tcp->raddr[0] = 0;
	tcp->rport = 0;

	return tcp;
}

struct tcp * tcp_attach( int fd )
{
	struct tcp *l = tcp_create();
	if(!l) return 0;

	l->fd = fd;

	if(tcp_address_remote(l,l->raddr,&l->rport)) {
		return l;
	} else {
		l->fd = -1;
		tcp_close(l);
		return 0;
	}
}

struct tcp * tcp_serve_address( const char *addr, int port );

struct tcp * tcp_listen( int port )
{
	return tcp_serve_address(0,port);
}

struct tcp * tcp_serve_address( const char *addr, int port )
{
	struct tcp *tcp=0;
	struct sockaddr_in address;
	int success;
	int on;

	tcp = tcp_create();
	if(!tcp) goto failure;

	tcp->fd = socket( PF_INET, SOCK_STREAM, 0 );
	if(tcp->fd<0) goto failure;

	on = 1;
	setsockopt( tcp->fd, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on) );

	if(addr!=0 || port!=0) {
		address.sin_family = AF_INET;
		address.sin_port = htons( port );
		if(addr) {
			string_to_ip_address(addr,(unsigned char*)&address.sin_addr.s_addr);
		} else {
			address.sin_addr.s_addr = htonl(INADDR_ANY);
		}

		success = bind( tcp->fd, (struct sockaddr *) &address, sizeof(address) );
		if(success<0) goto failure;
	}

	success = listen( tcp->fd, 5 );
	if(success<0) goto failure;

	if(!tcp_nonblocking(tcp,1)) goto failure;

	return tcp;

	failure:
	if(tcp) tcp_close(tcp);
	return 0;
}

struct tcp * tcp_accept( struct tcp * master, time_t stoptime )
{
	struct tcp *tcp=0;

	tcp = tcp_create();
	if(!tcp) goto failure;

	while(1) {
		if(!tcp_sleep(master,stoptime,1,0)) goto failure;
		tcp->fd = accept(master->fd,0,0);
		break;
	}

	if(!tcp_nonblocking(tcp,1)) goto failure;
	if(!tcp_address_remote(tcp,tcp->raddr,&tcp->rport)) goto failure;

	return tcp;

	failure:
	if(tcp) tcp_close(tcp);
	return 0;
}

struct tcp * tcp_connect( const char *addr, int port, time_t stoptime )
{
	struct sockaddr_in address;
	struct tcp *tcp = 0;
	int result;
	int save_errno;

	tcp = tcp_create();
	if(!tcp) goto failure;

	tcp_squelch();

	address.sin_family = AF_INET;
	address.sin_port = htons(port);

	if(!string_to_ip_address(addr,(unsigned char *)&address.sin_addr)) goto failure;

	tcp->fd = socket( AF_INET, SOCK_STREAM, 0 );
	if(tcp->fd<0) goto failure;

	/* sadly, cygwin does not do non-blocking connect correctly */
#ifdef CCTOOLS_OPSYS_CYGWIN
	if(!tcp_nonblocking(tcp,0)) goto failure;
#else
	if(!tcp_nonblocking(tcp,1)) goto failure;
#endif

	do {
		result = connect( tcp->fd, (struct sockaddr *) &address, sizeof(address) );

		/* On some platforms, errno is not set correctly. */
		/* If the remote address can be found, then we are really connected. */

		if( result<0 && !errno_is_temporary(errno) ) break;
		if(tcp_address_remote(tcp,tcp->raddr,&tcp->rport)) {

#ifdef CCTOOLS_OPSYS_CYGWIN
			tcp_nonblocking(tcp,1);
#endif
			return tcp;
		}
	} while(tcp_sleep(tcp,stoptime,0,1));

	failure:
	save_errno = errno;
	if(tcp) tcp_close(tcp);
	errno = save_errno;
	return 0;
}

static int fill_buffer( struct tcp *tcp, time_t stoptime )
{
	int chunk;

	if(tcp->buffer_length>0) return tcp->buffer_length;

	while(1) {
		chunk = read(tcp->fd,tcp->buffer,BUFFER_SIZE);
		if(chunk>0) {
			tcp->buffer_start = 0;
			tcp->buffer_length = chunk;
			return chunk;
		} else if(chunk==0) {
			tcp->buffer_start = 0;
			tcp->buffer_length = 0;
			return 0;
		} else {
			if(errno_is_temporary(errno)) {
				if(tcp_sleep(tcp,stoptime,1,0)) {
					continue;
				} else {
					return -1;
				}
			} else {
				return -1;
			}
		}
	}
}

/* tcp_read blocks until all the requested data is available */

int tcp_read( struct tcp *tcp, char *data, int count, time_t stoptime )
{
	ssize_t total=0;
	ssize_t chunk=0;

	/* If this is a small read, attempt to fill the buffer */
	if(count<BUFFER_SIZE) {
		chunk = fill_buffer(tcp,stoptime);
		if(chunk<=0) return chunk;
	}

	/* Then, satisfy the read from the buffer, if any. */

	if(tcp->buffer_length>0) {
		chunk = MIN(tcp->buffer_length,count);
		memcpy(data,&tcp->buffer[tcp->buffer_start],chunk);
		data += chunk;
		total += chunk;
		count -= chunk;
		tcp->buffer_start += chunk;
		tcp->buffer_length -= chunk;
	}

	/* Otherwise, pull it all off the wire. */
		
	while(count>0) {
    	        chunk = read(tcp->fd,data,count);
		if(chunk<0) {
			if( errno_is_temporary(errno) ) {
				if(tcp_sleep(tcp,stoptime,1,0)) {
					continue;
				} else {
					break;
				}
			} else {
				break;
			}
		} else if(chunk==0) {
			break;
		} else {
			total += chunk;
			count -= chunk;
			data += chunk;
		}
	}

	if(total>0) {
		return total;
	} else {
		if(chunk==0) {
			return 0;
		} else {
			return -1;
		}
	}		
}

/* tcp_read_avail returns whatever is available, blocking only if nothing is */

int tcp_read_avail( struct tcp *tcp, char *data, int count, time_t stoptime )
{
	ssize_t total=0;
	ssize_t chunk=0;

	/* First, satisfy anything from the buffer. */

	if(tcp->buffer_length>0) {
		chunk = MIN(tcp->buffer_length,count);
		memcpy(data,&tcp->buffer[tcp->buffer_start],chunk);
		data += chunk;
		total += chunk;
		count -= chunk;
		tcp->buffer_start += chunk;
		tcp->buffer_length -= chunk;
	}

	/* Next, read what is available off the wire */
		
	while(count>0) {
    	        chunk = read(tcp->fd,data,count);
		if(chunk<0) {
			/* ONLY BLOCK IF NOTHING HAS BEEN READ */
			if( errno_is_temporary(errno) && total==0 ) {
				if(tcp_sleep(tcp,stoptime,1,0)) {
					continue;
				} else {
					break;
				}
			} else {
				break;
			}
		} else if(chunk==0) {
			break;
		} else {
			total += chunk;
			count -= chunk;
			data += chunk;
		}
	}

	if(total>0) {
		return total;
	} else {
		if(chunk==0) {
			return 0;
		} else {
			return -1;
		}
	}		
}

int tcp_printf( struct tcp *tcp, const char *fmt, ... )
{
	va_list args;
	va_start(args,fmt);

	char line[1024];

	vsnprintf(line,sizeof(line),fmt,args);
	return tcp_write(tcp,line,strlen(line),time(0)+60);

	va_end(args);
}

int tcp_readline( struct tcp *tcp, char *line, int length, time_t stoptime )
{
	while(1) {
		while(length>0 && tcp->buffer_length>0) {
			*line = tcp->buffer[tcp->buffer_start];
			tcp->buffer_start++;
			tcp->buffer_length--;
			if(*line==10) {
				*line = 0;
				return 1;
			} else if(*line==13) {
				continue;
			} else {
				line++;
				length--;
			}
		}
		if(length<=0) break;
		if(fill_buffer(tcp,stoptime)<=0) break;
	}

	return 0;
}

int tcp_write( struct tcp *tcp, const char *data, int count, time_t stoptime )
{
	ssize_t total=0;
	ssize_t chunk=0;

	while(count>0) {
	  if (tcp)
		chunk = write(tcp->fd,data,count);
		if(chunk<0) {
			if( errno_is_temporary(errno) ) {
				if(tcp_sleep(tcp,stoptime,0,1)) {
					continue;
				} else {
					break;
				}
			} else {
				break;
			}
		} else if(chunk==0) {
			break;
		} else {
			total += chunk;
			count -= chunk;
			data += chunk;
		}
	}

	if(total>0) {
		return total;
	} else {
		if(chunk==0) {
			return 0;
		} else {
			return -1;
		}
	}		
}

void tcp_close( struct tcp *tcp )
{
	if(tcp) {
	  	if(tcp->fd>=0) close(tcp->fd);
		free(tcp);
	}
}

int tcp_fd( struct tcp *tcp )
{
	return tcp->fd;
}

#if defined(__GLIBC__) || defined(CCTOOLS_OPSYS_DARWIN)
	#define SOCKLEN_T socklen_t
#else
	#define SOCKLEN_T int
#endif

int tcp_address_local( struct tcp *tcp, char *addr, int *port )
{
	struct sockaddr_in iaddr;
	SOCKLEN_T length;  
	int result;

	length = sizeof(iaddr);
	result = getsockname( tcp->fd, (struct sockaddr*) &iaddr, &length );
	if(result!=0) return 0;
       
	*port = ntohs(iaddr.sin_port);
	string_from_ip_address((unsigned char *)&iaddr.sin_addr,addr);
	
	return 1;
}

int tcp_address_remote( struct tcp *tcp, char *addr, int *port )
{
	struct sockaddr_in iaddr;
	SOCKLEN_T length;
	int result;

	length = sizeof(iaddr);
	result = getpeername( tcp->fd, (struct sockaddr*) &iaddr, &length );
	if(result!=0) return 0;

	*port = ntohs(iaddr.sin_port);
	string_from_ip_address((unsigned char *)&iaddr.sin_addr,addr);

	return 1; 
}
