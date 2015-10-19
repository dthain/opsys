/*
This code by Prof. Douglas Thain at the University of Notre Dame
is licensed under a Creative Commons Attribution 4.0 International License.
You are welcome to reuse and adapt it as long as you give proper credit.
*/

#ifndef BITMAP_H
#define BITMAP_H

struct bitmap * bitmap_create( int w, int h );
void            bitmap_delete( struct bitmap *b );
struct bitmap * bitmap_load( const char *file );
int             bitmap_save( struct bitmap *b, const char *file );

int   bitmap_get( struct bitmap *b, int x, int y );
void  bitmap_set( struct bitmap *b, int x, int y, int value );
int   bitmap_width( struct bitmap *b );
int   bitmap_height( struct bitmap *b );
void  bitmap_reset( struct bitmap *b, int value );
int  *bitmap_data( struct bitmap *b );

#ifndef MAKE_RGBA
/** Create a 32-bit RGBA value from 8-bit red, green, blue, and alpha values */
#define MAKE_RGBA(r,g,b,a) ( (((int)(a))<<24) | (((int)(r))<<16) | (((int)(g))<<8) | (((int)(b))<<0) )
#endif

#ifndef GET_RED
/** Extract an 8-bit red value from a 32-bit RGBA value. */
#define GET_RED(rgba) (( (rgba)>>16 ) & 0xff )
#endif

#ifndef GET_GREEN
/** Extract an 8-bit green value from a 32-bit RGBA value. */
#define GET_GREEN(rgba) (( (rgba)>>8 ) & 0xff )
#endif

#ifndef GET_BLUE
/** Extract an 8-bit blue value from a 32-bit RGBA value. */
#define GET_BLUE(rgba) (( (rgba)>>0 ) & 0xff )
#endif

#ifndef GET_ALPHA
/** Extract an 8-bit alpha value from a 32-bit RGBA value. */
#define GET_ALPHA(rgba) (( (rgba)>>24 ) & 0xff)
#endif

#endif

