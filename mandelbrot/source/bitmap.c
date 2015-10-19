/*
This code by Prof. Douglas Thain at the University of Notre Dame
is licensed under a Creative Commons Attribution 4.0 International License.
You are welcome to reuse and adapt it as long as you give proper credit.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bitmap.h"

struct bitmap {
	int width;
	int height;
	int *data;
};

struct bitmap * bitmap_create( int w, int h )
{
	struct bitmap *m;

	m = malloc(sizeof *m);
	if(!m) return 0;

	m->data = malloc(w*h*sizeof(int));
	if(!m->data) {
		free(m);
		return 0;
	}

	m->width = w;
	m->height = h;

	return m;
}

void bitmap_delete( struct bitmap *m )
{
	free(m->data);
	free(m);
}

void bitmap_reset( struct bitmap *m, int value )
{
	int i;
	for(i=0;i<(m->width*m->height);i++) {
		m->data[i] = value;
	}
}

int bitmap_get( struct bitmap *m, int x, int y )
{
	while(x>=m->width)  x-=m->width;
	while(y>=m->height) y-=m->height;
	while(x<0)         x+=m->width;
	while(y<0)         y+=m->height;

	return m->data[y*m->width+x];
}

void bitmap_set( struct bitmap *m, int x, int y, int value )
{
	while(x>=m->width)  x-=m->width;
	while(y>=m->height) y-=m->height;
	while(x<0)         x+=m->width;
	while(y<0)         y+=m->height;

	m->data[y*m->width+x] = value;
}

int bitmap_width( struct bitmap *m )
{
	return m->width;
}

int bitmap_height( struct bitmap *m )
{
	return m->height;
}

int * bitmap_data( struct bitmap *m )
{
	return m->data;
}

#pragma pack(1)
struct bmp_header {
	char	magic1;
	char	magic2;
	int	size;
	int	reserved;
	int	offset;
	int	infosize;
	int	width;
	int	height;
	short	planes;
	short	bits;
	int	compression;
	int	imagesize;
	int	xres;
	int	yres;
	int	ncolors;
	int	icolors;
};

int bitmap_save( struct bitmap *m, const char *path )
{
	FILE *file;
	struct bmp_header header;
	int i, j;
	unsigned char *scanline, *s;

	file = fopen(path,"wb");
	if(!file) return 0;

	memset(&header,0,sizeof(header));
	header.magic1 = 'B';
	header.magic2 = 'M';
	header.size   = m->width*m->height*3;
	header.offset = sizeof(header);
	header.infosize = sizeof(header)-14;
	header.width = m->width;
	header.height = m->height;
	header.planes = 1;
	header.bits = 24;
	header.compression = 0;
	header.imagesize = m->width*m->height*3;
	header.xres = 1000;
	header.yres = 1000;

	fwrite(&header,1,sizeof(header),file);

	/* if the scanline is not a multiple of four, round it up. */
	int padlength = 4 - (m->width*3)%4;
	if(padlength==4) padlength=0;

	scanline = malloc(header.width*3);

	for(j=0;j<m->height;j++) {
		s = scanline;
		for(i=0;i<m->width;i++) {
			int rgba = bitmap_get(m,i,j);
			*s++ = GET_BLUE(rgba);
			*s++ = GET_GREEN(rgba);
			*s++ = GET_RED(rgba);
		}
		fwrite(scanline,1,m->width*3,file);
		fwrite(scanline,1,padlength,file);
	}

	free(scanline);

	fclose(file);
	return 1;
}

struct bitmap * bitmap( const char *path )
{
	FILE *file;
	int size;
	struct bitmap *m;
	struct bmp_header header;
	int i;

	file = fopen(path,"rb");
	if(!file) return 0;

	fread(&header,1,sizeof(header),file);

	if(header.magic1!='B' || header.magic2!='M') {
		printf("bitmap: %s is not a BMP file.\n",path);
		fclose(file);
		return 0;
	}

	if(header.compression!=0 || header.bits!=24) {
		printf("bitmap: sorry, I only support 24-bit uncompressed bitmaps.\n");
		fclose(file);
		return 0;
	}

	m = bitmap_create(header.width,header.height);
	if(!m) {
		fclose(file);
		return 0;
	}

	size = header.width*header.height;
	for(i=0;i<size;i++) {
		int r,g,b;
		b = fgetc(file);
		g = fgetc(file);
		r = fgetc(file);
		if(b==0 && g==0 && r==0) {
			m->data[i] = 0;
		} else {
			m->data[i] = MAKE_RGBA(r,g,b,255);
		}	
	}

	fclose(file);
	return m;
}
