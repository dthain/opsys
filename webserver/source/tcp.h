/*
This code by Prof. Douglas Thain at the University of Notre Dame
is licensed under a Creative Commons Attribution 4.0 International License.
You are welcome to reuse and adapt it as long as you give proper credit.
*/

#ifndef TCP_H
#define TCP_H

#include <time.h>

/*
Note that all of these commands accept an argument "stoptime",
which the absolute time at which the operation will be aborted.
For example, to specify that an operation will be attempted
for sixty seconds, use (time(0)+60) for the stoptime.
*/

/*
Create a tcp object listening on the port number.
Returns a tcp object on success, null on failure.
If another process is listening on this port, tcp_listen will fail.
*/

struct tcp * tcp_listen( int port );

/*
Accept a connecting from a listening tcp connection.
Returns a tcp object on success, null on failure.
*/

struct tcp * tcp_accept( struct tcp *master, time_t stoptime );

/*
Connect to a server at the given address and port number.
Returns a tcp object on success, null on failure.
*/

struct tcp * tcp_connect( const char *addr, int port, time_t stoptime );

/*
Read until end of line or a maximum of "length" bytes from a tcp connection.
Returns the number of bytes actually read.
Returns less than zero on error.
Returns exactly zero if the client closes the tcp connection.
*/

int  tcp_readline( struct tcp *tcp, char *line, int length, time_t stoptime );

/*
Write a printf-style formatted string to a tcp connection.
Returns the number of characters actually written.
*/
int tcp_printf( struct tcp *tcp, const char *fmt, ... );

/*
Read up to "length" bytes from a tcp connection.
Returns the number of bytes actually read.
Returns less than zero on error.
Returns exactly zero if the client closes the tcp connection.
*/

int  tcp_read( struct tcp *tcp, char *data, int length, time_t stoptime );

/*
Write up to "length" bytes from a tcp connection.
Returns the number of bytes actually written.
Returns less than zero on error.
*/

int  tcp_write( struct tcp *tcp, const char *data, int length, time_t stoptime );

/*
Close a tcp connection and delete the associated object.
*/

void tcp_close( struct tcp *tcp );

/*
The maximum number of characters in an address returned by
tcp_address_remote or tcp_address_local, in the form of an IP address:
*/

#define TCP_ADDRESS_MAX 18

/*
Retrieve the address and port number of the remote connection.
Fill in the string "addr" with the address and "port" with the port.
*/

int  tcp_address_remote( struct tcp *tcp, char *addr, int *port );

/*
Retrieve the address and port number of the local side of this connection.
Fill in the string "addr" with the address and "port" with the port.
*/

int  tcp_address_local( struct tcp *tcp, char *addr, int *port );

#endif
