/*
//
// Copyright (C) 2009-2016 Jean-Fran�ois DEL NERO
//
// This file is part of the HxCFloppyEmulator file selector.
//
// HxCFloppyEmulator file selector may be used and distributed without restriction
// provided that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// HxCFloppyEmulator file selector is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// HxCFloppyEmulator file selector is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HxCFloppyEmulator file selector; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/

//#define DBGMODE 1


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mint/osbind.h>
#include <time.h>
#include <vt52.h>


#ifdef __VBCC__
# include <tos.h>
#else
# include <mint/osbind.h>
# include <mint/linea.h>
# include "libc/snprintf/snprintf.h"
#endif

#include "conf.h"

#include "keysfunc_defs.h"
#include "keys_defs.h"
#include "keymap.h"
#include "hardware.h"

static unsigned char floppydrive;
static unsigned char datacache[512*9];
static unsigned char valid_cache;
unsigned char g_color;
unsigned long old_physical_adr;

volatile unsigned short io_floppy_timeout;

unsigned char * screen_buffer;
unsigned char screen_buffer_backup[8*1024];

static short  _oldrez = 0xffff;

unsigned short SCREEN_XRESOL;
unsigned short SCREEN_YRESOL;
unsigned short LINE_BYTES;					/* number of bytes per line     */
unsigned short LINE_WORDS;					/* number of words per line     */
unsigned short LINE_CHARS;					/* number of 8x8 chars per line */
unsigned short NB_PLANES;					/* number of planes (1:2 colors */
											/*  4:16 colors, 8: 256 colors) */
unsigned short CHUNK_WORDS;					/* number of words for a 16-    */
											/* pixel chunk =2*NB_PLANES     */
unsigned short PLANES_ALIGNDEC;				/* number of left shifts to
											   transform nbChucks to Bytes  */
__LINEA *__aline;
__FONT  **__fonts;
short  (**__funcs) (void);

static unsigned char CIABPRB_DSKSEL;

static unsigned char * mfmtobinLUT_L;
static unsigned char * mfmtobinLUT_H;

#define MFMTOBIN(W) ( mfmtobinLUT_H[W>>8] | mfmtobinLUT_L[W&0xFF] )

static unsigned short * track_buffer;
static unsigned short * track_buffer_wr;

static unsigned char validcache;

unsigned short sector_pos[16];

unsigned char keyup;

unsigned long timercnt;

#ifndef BMAPTYPEDEF
#define BMAPTYPEDEF

typedef  struct _bmaptype
{
   int type;
   int Xsize;
   int Ysize;
   int size;
   int csize;
   unsigned char * data;
}bmaptype __attribute__ ((aligned (2)));

#endif

void waitus(int centus)
{
}

/*
-------+-----+-----------------------------------------------------+----------
       |     |                                BIT 11111198 76543210|
       |     |                                    543210           |
       |     |                     ST color value .....RRr .GGr.BBb|
       |     |                    STe color value ....rRRR gGGGbBBB|
$FF8240|word |Video palette register 0              Lowercase = LSB|R/W
    :  |  :  |  :      :       :     :                             | :
$FF825E|word |Video palette register 15                            |R/W
-------+-----+-----------------------------------------------------+----------
*/

static unsigned short colortable[] = {
	0x002, 0xeee, 0x226, 0x567, // b ok blanc sur bleu nuit (nice)
	0x300, 0xEEE, 0x00f, 0xee4, // b ok blanc sur rouge fonc� (nice)
	0x777, 0x300, 0x00f, 0x5f5, // w noir sur blanc, select vert (nice)
	0xFFF, 0x343, 0x00f, 0x0f0, // w ok vert sombre sur blanc, select vert
	0x000, 0x00F, 0x222, 0xdd1, // b ok bleu sur noir
	0x000, 0xFFF, 0x00f, 0x3f3, // b ok blanc sur noir, select vert
	0x303, 0xFFF, 0x00f, 0xee4, // w ok blanc sur mauve
	0x030, 0xFFF, 0x00f, 0x0f0, // b ok vert
	0x999, 0x000, 0x999, 0x333, // w ok gris sombre
	0x330, 0xFFF, 0x77f, 0xcc0, // b ok caca d'oie
	0xF33, 0xFFF, 0x777, 0xe11, // w ok blanc sur rose et rouge
	0x000, 0xF00, 0x003, 0xd00, // b ok rouge sur noir
	0xF0F, 0xFFF, 0x000, 0x44f, // w ok violet vif
	0x000, 0x0E0, 0x00f, 0x0f0, // b ok vert sur noir
	0xFFF, 0x0F0, 0x4c4, 0x0f0, // w ok vert sur blanc
	0x004, 0xFFF, 0x00e, 0x5f5, // b ok blanc sur bleu marine

	0x036, 0xFFF, 0x00f, 0x0f0, // b
	0x444, 0x037, 0x00f, 0x0f0, // b
	0x000, 0xFF0, 0x00f, 0x0f0, // b
	0x404, 0x743, 0x00f, 0x0f0, // b
	0xFFF, 0x700, 0x00f, 0x0f0, // w
	0x000, 0x222, 0x00f, 0x0c0, // b
	0x000, 0x333, 0x00f, 0x0d0, // b
	0x000, 0x444, 0x00f, 0x0e0, // b
	0x000, 0x555, 0x00f, 0x0f0, // b
	0x000, 0x666, 0x00f, 0x0f0, // b
	0x000, 0x777, 0x00f, 0x0f0, // b
	0x222, 0x000, 0x00f, 0x0c0, // b
	0x333, 0x000, 0x00f, 0x0d0, // w
	0x444, 0x000, 0x00f, 0x0e0, // b
	0x555, 0x000, 0x00f, 0x0f0, // w
	0x666, 0x000, 0x00f, 0x0f0  // b
};

void waitms(int ms)
{
}

void testblink()
{
}

void alloc_error()
{
	hxc_printf_box(0,"ERROR: Memory Allocation Error -> No more free mem ?");
	for(;;);
}

/********************************************************************************
*                              FDC I/O
*********************************************************************************/
int jumptotrack(unsigned char t)
{
	unsigned short i,j;
	unsigned char data[512];

	Floprd( &data, 0, floppydrive, 1, t, 0, 1 );

	return 1;
};

int test_drive(int drive)
{
	return 0;
}

int waitindex()
{
	return 0;
}

unsigned char writesector(unsigned char sectornum,unsigned char * data)
{
	int ret,retry;

	valid_cache =0;
	retry=3;

	ret=1;
	while(retry && ret)
	{
		ret=Flopwr( data, 0, floppydrive, sectornum, 255, 0, 1 );
		retry--;
	}

	if(!ret)
		return 1;
	else
		return 0;
}

unsigned char readsector(unsigned char sectornum,unsigned char * data,unsigned char invalidate_cache)
{
	int ret,retry;

	retry=3;
	ret=0;
	if(!valid_cache || invalidate_cache)
	{
		if(sectornum<10)
		{
			ret=1;
			while(retry && ret)
			{
				ret=Floprd( datacache, 0, floppydrive, 0, 255, 0, 9 );
				retry--;
			}

			memcpy((void*)data,&datacache[sectornum*512],512);
			valid_cache=0xFF;
		}
	}
	else
	{
		memcpy((void*)data,&datacache[sectornum*512],512);
	}

	if(!ret)
		return 1;
	else
		return 0;
}

void init_fdc(unsigned char drive)
{
	unsigned short i,ret;

	valid_cache = 0;
	floppydrive = drive;
	Floprate( floppydrive, 2);

	ret = Floprd( &datacache, 0, floppydrive, 0, 255, 0, 1 );
}

/********************************************************************************
*                          Joystick / Keyboard I/O
*********************************************************************************/

unsigned char Joystick()
{
	return 0;
}


unsigned char Keyboard()
{
	return Cconin()>>16;
}

int kbhit()
{
	return 0;
}

void flush_char()
{
}

void wait_released_key()
{
	while(!(Keyboard()&0x80));
}

unsigned char get_char()
{
	unsigned char buffer;
	unsigned char key,i,c;
	unsigned char function_code,key_code;

	function_code=FCT_NO_FUNCTION;
	while(!(Keyboard()&0x80));

	do
	{
		c=1;
		do
		{
			do
			{
				key=Keyboard();
				if(key&0x80)
				{
					c=1;
				}
			}while(key&0x80);
			waitms(55);
			c--;

		}while(c);

		i=0;
		do
		{
			function_code=char_keysmap[i].function_code;
			key_code=char_keysmap[i].keyboard_code;
			i++;
		}while((key_code!=key) && (function_code!=FCT_NO_FUNCTION) );

	}while(function_code==FCT_NO_FUNCTION);

	return function_code;
}


unsigned char wait_function_key()
{
	unsigned char key,joy,i,c;
	unsigned char function_code,key_code;

	function_code=FCT_NO_FUNCTION;

	if( keyup == 1 )
	{
		waitms(250);
	}

	do
	{
		c=1;
		do
		{
			do
			{
				key=Keyboard();
				joy=Joystick();
				if(key&0x80 && !joy)
				{
					c=1;

					keyup = 2;
				}
			}while(key&0x80 && !joy);

			waitms(55);

			c--;

		}while(c);

		if(keyup)
			keyup--;

		if(joy)
		{
			if(joy&0x10)
			{
				while(Joystick()&0x10);

				return FCT_SELECT_FILE_DRIVEA;
			}
			if(joy&2)
				return FCT_DOWN_KEY;
			if(joy&1)
				return FCT_UP_KEY;
			if(joy&4)
				return FCT_RIGHT_KEY;
			if(joy&8)
				return FCT_LEFT_KEY;
		}

		i=0;
		do
		{
			function_code=keysmap[i].function_code;
			key_code=keysmap[i].keyboard_code;
			i++;
		}while((key_code!=key) && (function_code!=FCT_NO_FUNCTION) );

	}while(function_code==FCT_NO_FUNCTION);

	return function_code;
}

/********************************************************************************
*                              Display Output
*********************************************************************************/

void setvideomode(int mode)
{

}

void initpal()
{
	volatile unsigned short * ptr;

	ptr=(unsigned short *)0xFF8240;
	*ptr=colortable[((g_color&0xF)*4)+0];
	ptr=(unsigned short *)0xFF8242;
	*ptr=colortable[((g_color&0xF)*4)+2];
	ptr=(unsigned short *)0xFF8244;
	*ptr=colortable[((g_color&0xF)*4)+3];
	ptr=(unsigned short *)0xFF8246;
	*ptr=colortable[((g_color&0xF)*4)+1];
}

void set_color_scheme(unsigned char color)
{
	unsigned short * palette;
	short tmpcolor;
	int i,j;
	int nbcols;
	static unsigned short initialpalette[4] = {0xffff, 0xffff, 0xffff, 0xffff};

	if (0xff == color) {
		// cycle
		color = g_color+1;
		if ( color >= (sizeof(colortable))>>3 ) {
			// reset to first
			color = 0;
		}
	}
	if (0xfe == color) {
		// restore
		palette = initialpalette;
	} else {
		g_color = color;
		palette = &colortable[g_color<<2];
	}
	nbcols = 2<<(NB_PLANES-1);

	for (i=0; i<4 && i<nbcols; i++) {
		j = i;
		if (i>=2) {
			// the first two colors are always pal[0] and pal[1]
			// the last two colors may be pal[2] and pal[3] in 2 planes, or pal[14] and pal[15] in 4 planes
			j = nbcols - 4 + i;
		}
		tmpcolor = Setcolor(j, palette[i]);
		if (0xffff == initialpalette[i]) {
			initialpalette[i] = tmpcolor;
		}
	}

	return g_color;
}

int init_display()
{
	unsigned short loop,yr;
	unsigned long k,i;

	linea0();

	// Line-A : Hidemouse
	// do not do : __asm__("dc.w 0xa00a"); (it clobbers registry)
	lineaa();

	if (V_X_MAX < 640) {
		/*Blitmode(1) */;
		_oldrez = Getrez();
		Setscreen((unsigned char *) -1, (unsigned char *) -1, 1 );
	}

	SCREEN_XRESOL = V_X_MAX;
	SCREEN_YRESOL = V_Y_MAX;

	LINE_BYTES    = V_BYTES_LIN;
	LINE_WORDS    = V_BYTES_LIN/2;
	LINE_CHARS    = SCREEN_XRESOL/8;
	NB_PLANES     = __aline->_VPLANES;
	CHUNK_WORDS   = NB_PLANES<<1;

	for (i=NB_PLANES, k=0; i!=0; i>>=1, k++);

	PLANES_ALIGNDEC = k;

	screen_buffer = (unsigned long)Physbase();
	memset(screen_buffer, 0, SCREEN_YRESOL * LINE_BYTES);

	set_color_scheme(0);

	// Number of free line to display the file list.
	disablemousepointer();

	return 0;
}

unsigned short get_vid_mode()
{
	return 0;
}

void disablemousepointer()
{

}

void print_char8x8(unsigned char * membuffer, bmaptype * font,unsigned short x, unsigned short y,unsigned char c)
{
	unsigned short j,k;
	unsigned char *ptr_src;
	unsigned char *ptr_dst;
	unsigned long base_offset;

	ptr_dst = membuffer;
	ptr_src=(unsigned char*)&font->data[0];

	k=((c>>4)*(8*8*2))+(c&0xF);

	base_offset=((unsigned long) y*LINE_BYTES) + ((x>>4)<<PLANES_ALIGNDEC) + ((x&8)==8);
	// in a 16-pixel chunk, there are 2 8-pixel chars, hence the x&8==8

	for(j=0;j<8;j++)
	{
		ptr_dst[base_offset] = ptr_src[k];
		k=k+(16);
		base_offset += LINE_BYTES;
	}
}

void display_sprite(unsigned char * membuffer, bmaptype * sprite,unsigned short x, unsigned short y)
{
	unsigned short i,j,k;
	unsigned short *ptr_src;
	unsigned short *ptr_dst;
	unsigned long  base_offset, l;

	ptr_dst=(unsigned short*)membuffer;
	ptr_src=(unsigned short*)&sprite->data[0];

	k=0;

	base_offset=( ((unsigned long) y*LINE_BYTES) + ((x>>4)<<PLANES_ALIGNDEC) )/2;
	for(j=0;j<(sprite->Ysize);j++)
	{
		l = base_offset;
		for (i=0; i<(sprite->Xsize>>4); i++)
		{
			ptr_dst[l]=ptr_src[k];
			l += NB_PLANES;
			k++;
		}
		base_offset += LINE_WORDS;
	}

}

void h_line(unsigned short y_pos,unsigned short val)
{
	unsigned short * ptr_dst;
	unsigned short i;

	ptr_dst=(unsigned short *) screen_buffer;
	ptr_dst += (unsigned long) LINE_WORDS * y_pos;

	for(i=0; i<LINE_WORDS; i+=NB_PLANES)
	{
		*(ptr_dst) = val;
		ptr_dst += NB_PLANES;
	}

}

void invert_line(unsigned short x_pos,unsigned short y_pos)
{
	unsigned short i;
	unsigned char j;
	unsigned char  *ptr_dst;
	unsigned short *ptr_dst2;

	ptr_dst   = screen_buffer;
	ptr_dst  += (unsigned long) LINE_BYTES* (y_pos);

	ptr_dst2 = (unsigned short *)ptr_dst;

	for(j=0;j<8;j++)
	{
		for(i=0; i<LINE_WORDS; i+=1)
		{
			//*ptr_dst = (*ptr_dst ^ 0xFFFF);
			*ptr_dst2 = (*ptr_dst2 ^ 0xFFFF);
			ptr_dst2++;
		}
	}
}

void save_box()
{
	memcpy(screen_buffer_backup,&screen_buffer[160*70], 8*1024);
}

void restore_box()
{
	memcpy(&screen_buffer[160*70],screen_buffer_backup, 8*1024);
}

void su_reboot()
{
	asm("move.l #4,A6");
	asm("move.l (A6),A0");
	asm("move.l A0,-(SP)");
	asm("rts");
}

void reboot()
{
	Supexec(su_reboot);
}

void ithandler(void)
{
	timercnt++;

	io_floppy_timeout++;

	if( ( Keyboard() & 0x80 )  && !Joystick())
	{
		keyup  = 2;
	}
}

void init_timer()
{
}

void sleep(int secs)
{

}
