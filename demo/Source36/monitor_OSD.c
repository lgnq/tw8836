//*****************************************************************************
//
//								Monitor_OSD.c
//
//*****************************************************************************
//
//
#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "printf.h"
#include "util.h"
#include "CPU.h"
#include "TW8836.h"

#include "I2C.h"
#include "SOsd.h"
#include "FOsd.h"
#include "SPI.h"
#include "monitor.h"
#include "Global.h"
#include "SpeedoMeter.h"
#include "Clock.h"
#include "SlideMenu.h"
#include "Remo.h"
#include "TouchKey.h"
#include "Demo.h"
#include "GridLine.h"
#include "Compass.h"

#include "SpiFlashMap.h"

//#include "OSD_Image\fordlogo_half_RLE_LUT.c"
//#include "OSD_Image\fordlogo_half_RLE.c"

					 
//#include "OSD_Image\Pigeon01_RLE_LUT.c"
//#include "OSD_Image\Pigeon01_RLE.c"
//#include "OSD_Image\Pigeon02_RLE.c"
//#include "OSD_Image\Pigeon03_RLE.c"
//#include "OSD_Image\Pigeon04_RLE.c"
//#include "OSD_Image\Pigeon05_RLE.c"
//#include "OSD_Image\Pigeon06_RLE.c"
//#include "OSD_Image\Pigeon07_RLE.c"
//#include "OSD_Image\Pigeon08_RLE.c"


#include "OSD_Image\Harry_NoRLE_Header.c"



//#include "OSD_Image\Pigeon01_RLE_LUT.c"












void MonOsdHelp(void)
{
	Puts("\n===== Monitor OSD Help =====\n");
	Puts("\nInit           : osd 0");
	Puts("\nBlock Fill(8)  : osd 1 dx dy w h cl");
	Puts("\nBlock Fill(16) : osd 16 dx dy w h cl");
	Puts("\nBlock Transfer : osd 2 sx sy dx dy w h");
	Puts("\nLUT Test(8)    : osd lut w cnt sx sy");
	Puts("\nLUT Test(16)   : osd lut16");
	Puts("\nChange attr all: osd attr winno val");
	Puts("\nFord Logo      : osd ford sx sy");
	Puts("\nFord Logo RGB  : osd RGB sx sy");
	Puts("\nFord Logo aRGB : osd aRGB sx sy");
	Puts("\nFord Logo bRGB : osd bRGB sx sy");
	Puts("\n16bit expansion: osd EXP16 sx sy");
	Puts("\n             --> All numbers are decimal");

	Puts("\nAnimation      : osd ani 1/0");
	Puts("\nDisp Pigeon    : osd pigeon");
	Puts("\nDisp Pigeon Linear  : osd pigeonl");
	Puts("\nRose Test      : osd rose");
	Puts("\n16bit YUV      : osd Harry");
	Puts("\nSpeedometer    : osd sm nn");
	Puts("\nSM init        : osd sm init");
	Puts("\nSM Auto        : osd sm auto");
	Puts("\nSM Jump Step   : osd sm jump nn");
	Puts("\nSM Go speed    : osd sm go nn");

	Puts("\n************* Issues **************");
	Puts("\n 1. Over 0xC000 is unstable?");
	Puts("\n 2. Busy control from OSD?");
	Puts("\n\n");
}

void MonOsdLutTest(BYTE size, BYTE xcnt, WORD sx, WORD sy)
{
	BYTE i, x, y;
	BYTE w=size, h=size;
	
	OsdWriteMode( BLOCKFILL | BLT_NO | CONV_DIS | PIXEL8 | SOURCE0 );

	OsdBlockFill( 0+sx, 0+sy, (w+1)*xcnt + 3, (h+1)*(256/xcnt) + 3, 0xff );
	OsdBlockFill( 1+sx, 1+sy, (w+1)*xcnt + 1, (h+1)*(256/xcnt) + 1, 0x00 );

	i=0;
	for(y=0; y<(256/xcnt); y++) {
		for(x=0; x<xcnt; x++) {
			OsdBlockFill( sx+x*(w+1)+2, sy+y*(h+1)+2, w, h, i++ );
		}
	}
}

void MonOsdLutTest16(void)
{
	WORD x, y, cl=0;
	
	OsdWriteMode( BLOCKFILL | BLT_NO | CONV_DIS | PIXEL16 | SOURCE0 );

	for(y=0; y<256; y++) {
		for(x=0; x<256; x++) {
			OsdBlockFill( x, y, 1, 1, cl++ );
		}
	}
}
const BYTE YSTEP[] = { 0, 51, 102, 153, 204, 255 };
void MonOsdLutTestYUV(void)
{
	WORD    ycb, ycr;
	BYTE	y, u, v;
	
	OsdWriteMode( BLOCKFILL | BLT_NO | CONV_DIS | PIXEL16 | SOURCE0 );

	for ( y=0; y<6; y++ ) {
		u = 0;
		do {
			ycb = YSTEP[y]; 
			ycb <<=8;
			ycb += u;
			v = 0;
			do {
				ycr = YSTEP[y];
				ycr <<= 8;
				ycr += v;
				OsdBlockFill( v + (y%3)*256, u + (y/3)*256, 1, 1, ycb );
				OsdBlockFill( v + (y%3)*256 + 1, u + (y/3)*256, 1, 1, ycr );
				v += 2;
			} while ( v != 0 );
			u++;
		} while ( u!= 0 );
	}
}
const WORD map[] = {  
						0x00, 0x01, 0x04, 0x05, 0x10, 0x11, 0x14, 0x15, 
						0x40, 0x41, 0x44, 0x45, 0x50, 0x51, 0x54, 0x55,
                        0x100, 0x101, 0x104, 0x105, 0x110, 0x111, 0x114, 0x115, 
						0x140, 0x141, 0x144, 0x145, 0x150, 0x151, 0x154, 0x155,
                        0x400, 0x401, 0x404, 0x405, 0x410, 0x411, 0x414, 0x415, 
						0x440, 0x441, 0x444, 0x445, 0x450, 0x451, 0x454, 0x455,
                        0x500, 0x501, 0x504, 0x505, 0x510, 0x511, 0x514, 0x515, 
						0x540, 0x541, 0x544, 0x545, 0x550, 0x551, 0x554, 0x555
					};

void MonOsdLutTest565(void)
{
	WORD cl=0;
	BYTE	r, g, b;
	
	OsdWriteMode( BLOCKFILL | BLT_NO | CONV_DIS | PIXEL16 | SOURCE0 );

	g = 0;
	do {
		if ( ( g / 8 ) % 2 )	{	// it means odd number
			for ( b=0; b<32; b++ ) {
				cl = b; 
				cl <<= 6;
				cl += g;
				cl <<= 5;
				for(r=0; r<32; r++) {
					OsdBlockFill( r + (g%8)*32, 31 - b + (g/8)*32, 1, 1, cl++ );
					//OsdBlockFill(255 - ( r + (g%8)*32), 31 - b + (g/8)*32, 1, 1, cl++ );
				}
			}
			g++;
			for ( b=0; b<32; b++ ) {
				cl = b; 
				cl <<= 6;
				cl += g;
				cl <<= 5;
				for(r=0; r<32; r++) {
					OsdBlockFill( 31-r + (g%8)*32, 31 - b + (g/8)*32, 1, 1, cl++ );
					//OsdBlockFill( 255 - (31-r + (g%8)*32), 31 - b + (g/8)*32, 1, 1, cl++ );
				}
			}
			g++;
		}
		else {
			for ( b=0; b<32; b++ ) {
				cl = b; 
				cl <<= 6;
				cl += g;
				cl <<= 5;
				for(r=0; r<32; r++) {
					OsdBlockFill( r + (g%8)*32, b + (g/8)*32, 1, 1, cl++ );
				}
			}
			g++;
			for ( b=0; b<32; b++ ) {
				cl = b; 
				cl <<= 6;
				cl += g;
				cl <<= 5;
				for(r=0; r<32; r++) {
					OsdBlockFill( 31-r + (g%8)*32, b + (g/8)*32, 1, 1, cl++ );
				}
			}
			g++;
		}
	} while ( g<64 ) ;
}

void MonOsdLutTest565_2(void)
{
	WORD cl=0, temp, offset = 256;
	BYTE	r, g, b, step, i;
	
	OsdWriteMode( BLOCKFILL | BLT_NO | CONV_DIS | PIXEL16 | SOURCE0 );

	step = 0; g = 0; r=0; b=0;
	for ( step = 0; step < 64; step++ ) {
		r = step/2; b = 0;
		for ( i=0; i<64; i++ ) {							// R -> RG
			temp = i;
			temp *= step;
			temp /= 64;
			g = temp;
			cl = r; 
			cl <<= 6;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i, step+offset, 1, 1, cl );
		}
		g = step;  b = 0;
		for ( i=0; i<64; i++ ) {							// RG -> G
			temp = (63 - i );
			temp *= step;
			temp /= 64;
			r = temp/2;
			cl = r; 
			cl <<= 6;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i+64, step+offset, 1, 1, cl );
		}
		g = step; r = 0;
		for ( i=0; i<64; i++ ) {							// G -> GB
			temp = i;
			temp *= step;
			temp /= 64;
			b = temp/2;
			cl = r; 
			cl <<= 6;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i+128, step+offset, 1, 1, cl );
		}
		b = step/2;  r = 0;
		for ( i=0; i<64; i++ ) {							// GB -> B
			temp = (63 - i );
			temp *= step;
			temp /= 64;
			g = temp;
			cl = r; 
			cl <<= 6;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i+192, step+offset, 1, 1, cl );
		}
		b = step/2; g = 0;
		for ( i=0; i<64; i++ ) {							// B -> BR
			temp = i;
			temp *= step;
			temp /= 64;
			r = temp/2;
			cl = r; 
			cl <<= 6;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i+256, step+offset, 1, 1, cl );
		}
		r = step/2;  g = 0;
		for ( i=0; i<64; i++ ) {							// BR -> R
			temp = (63 - i );
			temp *= step;
			temp /= 64;
			b = temp/2;
			cl = r; 
			cl <<= 6;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i+320, step+offset, 1, 1, cl );
		}
	}
	for ( step = 0; step < 64; step++ ) {
		r = 31; b = step/2;
		for ( i=0; i<64; i++ ) {							// R -> RG
			temp = i;
			temp *= (63-step);
			temp /= 64;
			g = temp + step;
			cl = r; 
			cl <<= 6;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i, step+64+offset, 1, 1, cl );
		}
		g = 63;  b = step/2;
		for ( i=0; i<64; i++ ) {							// RG -> G
			temp = (63 - i );
			temp *= (63 - step);
			temp /= 64;
			r = temp/2 + step/2;
			cl = r; 
			cl <<= 6;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i+64, step+64+offset, 1, 1, cl );
		}
		g = 63; r = step/2;
		for ( i=0; i<64; i++ ) {							// G -> GB
			temp = i;
			temp *= (63-step);
			temp /= 64;
			b = temp/2 + step/2;
			cl = r; 
			cl <<= 6;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i+128, step+64+offset, 1, 1, cl );
		}
		b = 31;  r = step/2;
		for ( i=0; i<64; i++ ) {							// GB -> B
			temp = (63 - i );
			temp *= (63 - step);
			temp /= 64;
			g = temp + step;
			cl = r; 
			cl <<= 6;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i+192, step+64+offset, 1, 1, cl );
		}
		b = 31; g = step;
		for ( i=0; i<64; i++ ) {							// B -> BR
			temp = i;
			temp *= (63-step);
			temp /= 64;
			r = temp/2 + step/2;
			cl = r; 
			cl <<= 6;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i+256, step+64+offset, 1, 1, cl );
		}
		r = 31;  g = step;
		for ( i=0; i<64; i++ ) {							// BR -> R
			temp = (63 - i );
			temp *= (63 - step);
			temp /= 64;
			b = temp/2 + step/2;
			cl = r; 
			cl <<= 6;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i+320, step+64+offset, 1, 1, cl );
		}
	}
}

void MonOsdLutTest555(void)
{
	WORD cl=0;
	BYTE	r, g, b;
	
	OsdWriteMode( BLOCKFILL | BLT_NO | CONV_DIS | PIXEL16 | SOURCE0 );

	g = 0;
	do {
		if ( ( g / 8 ) % 2 )	{	// it means odd number
			for ( b=0; b<32; b++ ) {
				cl = b; 
				cl <<= 5;
				cl += g;
				cl <<= 5;
				for(r=0; r<32; r++) {
					OsdBlockFill( r + (g%8)*32, 31 - b + (g/8)*32, 1, 1, cl++ );
					//OsdBlockFill(255 - ( r + (g%8)*32), 31 - b + (g/8)*32, 1, 1, cl++ );
				}
			}
			g++;
			for ( b=0; b<32; b++ ) {
				cl = b; 
				cl <<= 5;
				cl += g;
				cl <<= 5;
				for(r=0; r<32; r++) {
					OsdBlockFill( 31-r + (g%8)*32, 31 - b + (g/8)*32, 1, 1, cl++ );
					//OsdBlockFill( 255 - (31-r + (g%8)*32), 31 - b + (g/8)*32, 1, 1, cl++ );
				}
			}
			g++;
		}
		else {
			for ( b=0; b<32; b++ ) {
				cl = b; 
				cl <<= 5;
				cl += g;
				cl <<= 5;
				for(r=0; r<32; r++) {
					OsdBlockFill( r + (g%8)*32, b + (g/8)*32, 1, 1, cl++ );
				}
			}
			g++;
			for ( b=0; b<32; b++ ) {
				cl = b; 
				cl <<= 5;
				cl += g;
				cl <<= 5;
				for(r=0; r<32; r++) {
					OsdBlockFill( 31-r + (g%8)*32, b + (g/8)*32, 1, 1, cl++ );
				}
			}
			g++;
		}
	} while ( g<32 ) ;
}


void MonOsdLutTest555_2(void)
{
	WORD cl=0, temp, offset = 128;
	BYTE	r, g, b, step, i;
	
	OsdWriteMode( BLOCKFILL | BLT_NO | CONV_DIS | PIXEL16 | SOURCE0 );

	step = 0; g = 0; r=0; b=0;
	for ( step = 0; step < 64; step++ ) {
		r = step/2; b = 0;
		for ( i=0; i<64; i++ ) {							// R -> RG
			temp = i;
			temp *= step;
			temp /= 64;
			g = temp/2;
			cl = r; 
			cl <<= 5;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i, step+offset, 1, 1, cl );
		}
		g = step/2;  b = 0;
		for ( i=0; i<64; i++ ) {							// RG -> G
			temp = (63 - i );
			temp *= step;
			temp /= 64;
			r = temp/2;
			cl = r; 
			cl <<= 5;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i+64, step+offset, 1, 1, cl );
		}
		g = step/2; r = 0;
		for ( i=0; i<64; i++ ) {							// G -> GB
			temp = i;
			temp *= step;
			temp /= 64;
			b = temp/2;
			cl = r; 
			cl <<= 5;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i+128, step+offset, 1, 1, cl );
		}
		b = step/2;  r = 0;
		for ( i=0; i<64; i++ ) {							// GB -> B
			temp = (63 - i );
			temp *= step;
			temp /= 64;
			g = temp/2;
			cl = r; 
			cl <<= 5;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i+192, step+offset, 1, 1, cl );
		}
		b = step/2; g = 0;
		for ( i=0; i<64; i++ ) {							// B -> BR
			temp = i;
			temp *= step;
			temp /= 64;
			r = temp/2;
			cl = r; 
			cl <<= 5;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i+256, step+offset, 1, 1, cl );
		}
		r = step/2;  g = 0;
		for ( i=0; i<64; i++ ) {							// BR -> R
			temp = (63 - i );
			temp *= step;
			temp /= 64;
			b = temp/2;
			cl = r; 
			cl <<= 5;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i+320, step+offset, 1, 1, cl );
		}
	}
	for ( step = 0; step < 64; step++ ) {
		r = 31; b = step/2;
		for ( i=0; i<64; i++ ) {							// R -> RG
			temp = i;
			temp *= (63-step);
			temp /= 64;
			g = temp/2 + step/2;
			cl = r; 
			cl <<= 5;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i, step+64+offset, 1, 1, cl );
		}
		g = 31;  b = step/2;
		for ( i=0; i<64; i++ ) {							// RG -> G
			temp = (63 - i );
			temp *= (63 - step);
			temp /= 64;
			r = temp/2 + step/2;
			cl = r; 
			cl <<= 5;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i+64, step+64+offset, 1, 1, cl );
		}
		g = 31; r = step/2;
		for ( i=0; i<64; i++ ) {							// G -> GB
			temp = i;
			temp *= (63-step);
			temp /= 64;
			b = temp/2 + step/2;
			cl = r; 
			cl <<= 5;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i+128, step+64+offset, 1, 1, cl );
		}
		b = 31;  r = step/2;
		for ( i=0; i<64; i++ ) {							// GB -> B
			temp = (63 - i );
			temp *= (63 - step);
			temp /= 64;
			g = temp/2 + step/2;
			cl = r; 
			cl <<= 5;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i+192, step+64+offset, 1, 1, cl );
		}
		b = 31; g = step/2;
		for ( i=0; i<64; i++ ) {							// B -> BR
			temp = i;
			temp *= (63-step);
			temp /= 64;
			r = temp/2 + step/2;
			cl = r; 
			cl <<= 5;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i+256, step+64+offset, 1, 1, cl );
		}
		r = 31;  g = step/2;
		for ( i=0; i<64; i++ ) {							// BR -> R
			temp = (63 - i );
			temp *= (63 - step);
			temp /= 64;
			b = temp/2 + step/2;
			cl = r; 
			cl <<= 5;
			cl += g;
			cl <<= 5;
			cl += b;
			OsdBlockFill( i+320, step+64+offset, 1, 1, cl );
		}
	}
}

void MonOsdLutTest444(void)
{
	WORD cl=0;
	BYTE	a, r, g, b;
	
	OsdWriteMode( BLOCKFILL | BLT_NO | CONV_DIS | PIXEL16 | SOURCE0 );

	for ( a=0; a<16; a++ ) {
		for ( b=0; b<16; b++ ) {
			for(g=0; g<16; g++) {
				cl = a;
				cl <<= 4;
				cl += b; 
				cl <<= 4;
				cl += g;
				cl <<= 4;
				for(r=0; r<16; r++) {
					OsdBlockFill( r + b*16, g + a*16, 1, 1, cl++ );
				}
			}
		}
	}
}

void MonOsdChangeAttr(BYTE winno, BYTE attr)
{
	WORD i;
	
	Printf("\nOSD %bd LUT set to all %02bx", winno, attr);
	WriteTW88(OSD_LUT_WINNO, winno<<6 );
	for(i=0; i<256; i++) {
		WriteTW88(OSD_LUT_INDEX, i );
		WriteTW88(OSD_LUT_ATTR, attr );
	}
}

/*
void MonOsdPigeon(WORD sx, WORD sy, BYTE d)
{
}
*/
/*
void MonOsdPigeonLinear(WORD sx, WORD sy, BYTE d)
}
*/

//=============================================================================
//		Animation Example
//=============================================================================
/*
void AnimationPigeon(void)
{
	BYTE i;
	BYTE kk=0;
	WORD w, h;
	struct st_animation *ani;

	struct _RLE_HEADER *p[8] = {
		Pigeon01_Header, Pigeon02_Header, Pigeon03_Header, Pigeon04_Header, 
		Pigeon05_Header, Pigeon06_Header, Pigeon07_Header, Pigeon08_Header
	};

	Printf("\nPigeon Display");
	OsdLoadLUT( 0, Pigeon01_LUT );

	w = Pigeon01_Header->w;
	h = Pigeon01_Header->h;

	OsdWriteMode( CPUWRITE | BLT_NO | CONV_DIS | PIXEL8 | SOURCE0 );
	for(i=0; i<8; i++) OsdLoadBmpXY( p[i], i*w, h*2 );

	//----- 1
	ani = &AnimationData[0];

	ani->active = 1;
	ani->current = 0;
	ani->speed = 1;
	ani->vcnt = 1;
	ani->x = 0;
	ani->y = 0;
	ani->w = w;
	ani->h = h;
	ani->max = 8;
	for(i=0; i<8; i++) {
		ani->srcx[i] = i*w;
		ani->srcy[i] = h*2;
	}

	//----- 2
	ani = &AnimationData[1];

	ani->active = 1;
	ani->current = 0;
	ani->speed = 2;
	ani->vcnt = 1;
	ani->x = w;
	ani->y = 0;
	ani->w = w;
	ani->h = h;
	ani->max = 8;
	for(i=0; i<8; i++) {
		ani->srcx[i] = i*w;
		ani->srcy[i] = h*2;
	}

	//----- 3
	ani = &AnimationData[2];

	ani->active = 1;
	ani->current = 0;
	ani->speed = 3;
	ani->vcnt = 1;
	ani->x = w*2;
	ani->y = 0;
	ani->w = w;
	ani->h = h;
	ani->max = 8;
	for(i=0; i<8; i++) {
		ani->srcx[i] = i*w;
		ani->srcy[i] = h*2;
	}

	//----- 4
	ani = &AnimationData[3];

	ani->active = 1;
	ani->current = 0;
	ani->speed = 4;
	ani->vcnt = 1;
	ani->x = w*3;
	ani->y = 0;
	ani->w = w;
	ani->h = h;
	ani->max = 8;
	for(i=0; i<8; i++) {
		ani->srcx[i] = i*w;
		ani->srcy[i] = h*2;
	}

	//----- 5
	ani = &AnimationData[4];

	ani->active = 1;
	ani->current = 0;
	ani->speed = 5;
	ani->vcnt = 1;
	ani->x = w*4;
	ani->y = 0;
	ani->w = w;
	ani->h = h;
	ani->max = 8;
	for(i=0; i<8; i++) {
		ani->srcx[i] = i*w;
		ani->srcy[i] = h*2;
	}


	//----- 6
	ani = &AnimationData[5];

	ani->active = 1;
	ani->current = 0;
	ani->speed = 6;
	ani->vcnt = 1;
	ani->x = 0;
	ani->y = h;
	ani->w = w;
	ani->h = h;
	ani->max = 8;
	for(i=0; i<8; i++) {
		ani->srcx[i] = i*w;
		ani->srcy[i] = h*2;
	}

	//----- 7
	ani = &AnimationData[6];

	ani->active = 1;
	ani->current = 0;
	ani->speed = 7;
	ani->vcnt = 1;
	ani->x = w;
	ani->y = h;
	ani->w = w;
	ani->h = h;
	ani->max = 8;
	for(i=0; i<8; i++) {
		ani->srcx[i] = i*w;
		ani->srcy[i] = h*2;
	}

	//----- 8
	ani = &AnimationData[7];

	ani->active = 1;
	ani->current = 0;
	ani->speed = 8;
	ani->vcnt = 1;
	ani->x = w*2;
	ani->y = h;
	ani->w = w;
	ani->h = h;
	ani->max = 8;
	for(i=0; i<8; i++) {
		ani->srcx[i] = i*w;
		ani->srcy[i] = h*2;
	}

	//----- 9
	ani = &AnimationData[8];

	ani->active = 1;
	ani->current = 0;
	ani->speed = 9;
	ani->vcnt = 1;
	ani->x = w*3;
	ani->y = h;
	ani->w = w;
	ani->h = h;
	ani->max = 8;
	for(i=0; i<8; i++) {
		ani->srcx[i] = i*w;
		ani->srcy[i] = h*2;
	}

	//----- 10
	ani = &AnimationData[9];

	ani->active = 1;
	ani->current = 0;
	ani->speed = 10;
	ani->vcnt = 1;
	ani->x = w*4;
	ani->y = h;
	ani->w = w;
	ani->h = h;
	ani->max = 8;
	for(i=0; i<8; i++) {
		ani->srcx[i] = i*w;
		ani->srcy[i] = h*2;
	}
}
*/



 
void MonOsdTestDDR(void)
{
	WORD i;

	OsdWriteMode( BLOCKFILL | BLT_NO | CONV_DIS | PIXEL8 | SOURCE0 );
	OsdBlockFill( 0, 0, 512, 256, 0 );

	for(i=0; i<512; i++) {
		OsdBlockFill( i, 0, 1, i+1, i );
		Printf("\nBlock Fill %d", i);
		delay1ms(50);
	}
	
}		



FAR CONST struct _RLE_HEADER TestFont8bit_Header[] = {    // File Header
    0x54, 0x57, 0x08, 0x47, 512, 272, 14013, 0 };  

//=============================================================================
//
//=============================================================================
void MonitorOSD(BYTE wbits)
{
	BYTE i;
	WORD dat[10];
	BYTE duration=6, ch;

	for(i=0; i<argc; i++)
		dat[i] = a2i(argv[i+2]);	
	

#ifdef MODEL_TW88TEMP
	WriteTW88Page(OSD_PAGE);
#endif

	if( !stricmp(argv[1], "?") ) {
		MonOsdHelp();
	}
	//---------- Initialize ------------------
	else if( !stricmp(argv[1], "0") ) {
		OsdInit(0);
		OsdInit(1);
		OsdInit(4);
		
		OsdLoadDefaultLUT(0);
		OsdLoadDefaultLUT(1);

		OsdWinEnable(0, 1);
	}
	//---------- Block Fill -------------------
	else if( !stricmp(argv[1], "1") ) {
		if( argc<=2 ) {
			dat[0]=0; dat[1]=0; dat[2]=800; dat[3]=480; dat[4]=0;
		}
		Printf("\nOsdBlockFill(%04x %04x %04x %04x ", dat[0], dat[1], dat[2], dat[3]);
		Printf("%04x)", dat[4]);
		OsdWriteMode( BLOCKFILL | BLT_NO | CONV_DIS | wbits | SOURCE0 );
		OsdBlockFill( dat[0], dat[1], dat[2], dat[3], dat[4]);
	}
	//---------- Block Fill 16 -----------------
	else if( !stricmp(argv[1], "11") ) {
		Printf("\nOsdBlockFill(%04x %04x %04x %04x ", dat[0], dat[1], dat[2], dat[3]);
		Printf("%04x)", dat[4]);
		OsdWriteMode( BLOCKFILL | BLT_NO | CONV_DIS | wbits | SOURCE0 );
		OsdBlockFill( dat[0], dat[1], dat[2], dat[3], dat[4]);
	}
	//---------- Block Transfer ----------------
	else if( !stricmp(argv[1], "2") ) {
		Printf("\nOsdBlockTransfer (%04x %04x ", dat[0], dat[1]);
		Printf("%04x %04x %04x %04x)", dat[2], dat[3], dat[4], dat[5]);
		OsdWriteMode( BLOCKTRANS | BLT_NO | CONV_DIS | wbits | SOURCE0 );
		OsdBlockTransfer( dat[0], dat[1], dat[2], dat[3], dat[4], dat[5]);
	}
	//---------- Block Transfer Linear ---------
	else if( !stricmp(argv[1], "btl") ) {
		Printf("\nOsdBlockTransfer (%04x %04x ", dat[0], dat[1]);
		Printf("%04x %04x %04x %04x)", dat[2], dat[3], dat[4], dat[5]);
		OsdWriteMode( BLOCKTRANSL | BLT_NO | CONV_DIS | wbits | SOURCE0 );
		OsdBlockTransfer( dat[0], dat[1], dat[2], dat[3], dat[4], dat[5]);
	}
	//---------- Bitmap Test "Ford" ------------
	else if( !stricmp(argv[1], "ford") ) {
		WORD sx=0, sy=0;
		if( argc>=3 ) sx=dat[0];
		if( argc>=4 ) sy=dat[1];
		Printf("\nFord Logo Display");
//		OsdLoadBmpXY( wbits, fordlogo_half_Header, sx, sy );
//		OsdLoadLUT( 0, fordlogo_half_LUT );
	}
/*
	//---------- Bitmap Test "Pigeon" ----------

	else if( !stricmp(argv[1], "pigeon") ) {
		WORD sx=0, sy=0, d=4;
		if( argc>=3 ) sx=dat[0];
		if( argc>=4 ) sy=dat[1];
		if( argc>=5 ) d = dat[2];
//		MonOsdPigeon(sx,sy, d);
	}
	//---------- Bitmap Test "Pigeon Linear" ----------
	else if( !stricmp(argv[1], "pigeonL") ) {
		WORD sx=0, sy=0, d=4;
		if( argc>=3 ) sx=dat[0];
		if( argc>=4 ) sy=dat[1];
		if( argc>=5 ) d = dat[2];
		MonOsdPigeonLinear(sx,sy, d);
	}
*/
	//---------- Bitmap Test "Rose Linear from SPI" ----------
	else if( !stricmp(argv[1], "Rose") ) {
		//RoseDisplay();
	}

	//---------- Bitmap Test "Pigeon Linear from SPI" ----------
	else if( !stricmp(argv[1], "PigeonL") ) {
		//PigeonDisplay();
	}

	//---------- Load Font -----------------------------
	else if( !stricmp(argv[1], "Font") ) {

		OsdLoadFont(wbits);

	}

	//---------- Load Font -----------------------------
	else if( !stricmp(argv[1], "FontMV") ) {

		OsdLoadFontMV(wbits);

	}

	//---------- Load Font -----------------------------
	else if( !stricmp(argv[1], "FontTP") ) {			// transparent test

		OsdLoadFontTransparent(wbits);

	}

	//---------- Load 16bit expansion -----------------------------
	else if( !stricmp(argv[1], "EXP16") ) {			// transparent test

		Osd16bitTransparent(wbits);

	}

	//---------- Load Font -----------------------------
	else if( !stricmp(argv[1], "FontTest") ) {

		//OsdLoadTransBmpXYFromSPI(TestFont8bit_Header, OSDFONT8_SPI_START, 0, 0);

	}

	//---------- Load Font -----------------------------
	else if( !stricmp(argv[1], "TestAll") ) {
	/*
		DWORD ddraddr=HARRY_DDR_START, spiaddr=HARRY_SPI_START;
		WORD x=0, y=0;

		OsdInit(0);
		OsdInit(1);
		OsdInit(4);
		
		OsdLoadDefaultLUT(0);
		OsdLoadDefaultLUT(1);

		OsdWinEnable(0, 1);

		if( argc<=2 ) {
			dat[0]=0; dat[1]=0; dat[2]=800; dat[3]=480; dat[4]=0;
		}
		Printf("\nOsdBlockFill(%04x %04x %04x %04x ", dat[0], dat[1], dat[2], dat[3]);
		Printf("%04x)", dat[4]);
		OsdWriteMode( BLOCKFILL | BLT_NO | CONV_DIS | wbits | SOURCE0 );
		OsdBlockFill( dat[0], dat[1], dat[2], dat[3], dat[4]);

		//---------- Bitmap Test "Pigeon Linear from SPI" ----------
		PigeonDisplay();
		ch = 0;
	    //------------------------- TEST ROSE -----------------------------------------------------------------
		RoseDisplay();
		ch = 0;

	    //------------------------- TEST Harry 16bit -----------------------------------------------------------------

		//DWORD ddraddr=0L, spiaddr=0x100000L;

		if( argc>=3 ) x=a2h(argv[2]);
		if( argc>=4 ) y=a2h(argv[3]);
		//if( argc>=3 ) ddraddr=a2h(argv[2]);
		//if( argc>=4 ) spiaddr=a2h(argv[3]);
		
		OsdWriteMode( CPUWRITE | BLT_NO | CONV_DIS | PIXEL16 );
		OsdLoadBmpXYFromSPI(  Harry_Header, spiaddr, x, x );
//		OsdLoadBmpLinearFromSPI( Harry_Header, spiaddr, ddraddr);

//		OsdBltSourceMemoryW64( Harry_Header->w/64 );
//		OsdBltDestMemoryW64  ( Harry_Header->w/64 );

//		OsdWinBuffer( 4, ddraddr, Harry_Header->w/64, Harry_Header->h/64 + 1 );
		OsdWinEnable(0, 0);
		OsdWinEnable(1, 0);
		OsdWinEnable(4, 1);
		Osd16Format(OSD16_YCBCR422);		// 00=YCbCr422, 01=RGB565, 10=RGB4444, 11=RGB1444
		Printf("\nPress ESC to display 8bit to 16bit expansion with transparent");
		while(1) {
				if( RS_ready() ) {
					ch = RS_rx();
					if ( ch == 27 ) break;
				}
		}


		Osd16bitTransparent(wbits);

		Printf("\nPress ESC to display LUT16");
		while(1) {
				if( RS_ready() ) {
					ch = RS_rx();
					if ( ch == 27 ) break;
				}
		}

		MonOsdLutTest16();
	*/
	}

#define COLOR_BLACK		0x00
#define COLOR_WHITE		0xff
#define COLOR_RED		0xe0
#define COLOR_GREEN		0x18
#define COLOR_BLUE		0x07

	//---------- Display String ------------------------
	else if( !stricmp(argv[1], "S") ) {
		BYTE fore=COLOR_WHITE, back=COLOR_BLACK;
		WORD x=0, y=0;
		
		if( argc>=4 ) fore = a2h( argv[3] );
		if( argc>=5 ) back = a2h( argv[4] );
		if( argc>=6 ) x    = a2h( argv[5] );
		if( argc>=7 ) y    = a2h( argv[6] );

		OsdDisplayString( 0, x, y, argv[2], fore, back );
	}

	//---------- Display String ------------------------
	else if( !stricmp(argv[1], "ST") ) {
		BYTE fore=COLOR_WHITE, back=COLOR_BLACK;
		WORD x=0, y=0;
		
		if( argc>=4 ) fore = a2h( argv[3] );
		if( argc>=5 ) back = a2h( argv[4] );
		if( argc>=6 ) x    = a2h( argv[5] );
		if( argc>=7 ) y    = a2h( argv[6] );

		OsdDisplayStringT( 0, x, y, argv[2], fore );
	}

	//---------- Display String ------------------------
	else if( !stricmp(argv[1], "SMV") ) {
		BYTE fore=COLOR_WHITE, back=COLOR_BLACK;
		WORD x=0, y=0;
		
		if( argc>=4 ) fore = a2h( argv[3] );
		if( argc>=5 ) back = a2h( argv[4] );
		if( argc>=6 ) x    = a2h( argv[5] );
		if( argc>=7 ) y    = a2h( argv[6] );

		OsdDisplayStringLinear( x, y, argv[2], fore, back );
	}

	//---------- Display String ------------------------
	else if( !stricmp(argv[1], "STMV") ) {
		BYTE fore=COLOR_WHITE, back=COLOR_BLACK;
		WORD x=0, y=0;
		
		if( argc>=4 ) fore = a2h( argv[3] );
		if( argc>=5 ) back = a2h( argv[4] );
		if( argc>=6 ) x    = a2h( argv[5] );
		if( argc>=7 ) y    = a2h( argv[6] );

		OsdDisplayStringTLinear( x, y, argv[2], fore );
	}

	//---------- Bitmap Test "Harry(OSD422(YCb YCr)) from SPI" ----------
	else if( !stricmp(argv[1], "Harry") ) {	
	/*				 			
		//DWORD ddraddr=0L, spiaddr=0x100000L;
		DWORD ddraddr=HARRY_DDR_START, spiaddr=HARRY_SPI_START;
		WORD x=0, y=0;

		if( argc>=3 ) x=a2h(argv[2]);
		if( argc>=4 ) y=a2h(argv[3]);
		//if( argc>=3 ) ddraddr=a2h(argv[2]);
		//if( argc>=4 ) spiaddr=a2h(argv[3]);
		
		OsdWriteMode( CPUWRITE | BLT_NO | CONV_DIS | PIXEL16 );
		OsdLoadBmpXYFromSPI(  Harry_Header, spiaddr, x, x );
//		OsdLoadBmpLinearFromSPI( Harry_Header, spiaddr, ddraddr);

//		OsdBltSourceMemoryW64( Harry_Header->w/64 );
//		OsdBltDestMemoryW64  ( Harry_Header->w/64 );

//		OsdWinBuffer( 4, ddraddr, Harry_Header->w/64, Harry_Header->h/64 + 1 );
		OsdWinEnable(0, 0);
		OsdWinEnable(1, 0);
		OsdWinEnable(4, 1);
		Osd16Format(OSD16_YCBCR422);		// 00=YCbCr422, 01=RGB565, 10=RGB4444, 11=RGB1444
	*/
	}
	/*
	//---------- Bitmap Test "OSD444(YCbCr:655) from SPI" ----------
	else if( !stricmp(argv[1], "OSD444") ) {
		//DWORD ddraddr=0L, spiaddr=0x100000L;
		DWORD ddraddr=OSD444_DDR_START, spiaddr=OSD444_SPI_START;
		if( argc>=3 ) ddraddr=a2h(argv[2]);
		if( argc>=4 ) spiaddr=a2h(argv[3]);
		
		OsdWriteMode( CPUWRITE | BLT_NO | CONV_DIS | PIXEL16 );
		OsdLoadBmpLinearFromSPI( Harry_Header, spiaddr, ddraddr);

		OsdBltSourceMemoryW64( OSD444_Header->w/64 );
		OsdBltDestMemoryW64  ( OSD444_Header->w/64 );

		OsdWinBuffer( 4, ddraddr, OSD444_Header->w/64, OSD444_Header->h/64 + 1 );
		OsdWinEnable(0, 0);
		OsdWinEnable(1, 0);
		OsdWinEnable(4, 1);
		Osd16Format(OSD16_YCBCR422);		// 00=YCbCr422, 01=RGB565, 10=RGB4444, 11=RGB1444

	}
	*/
	//---------- Bitmap Test "OSDRGB(565) from SPI" ----------
	/*
	else if( !stricmp(argv[1], "RGBn") ) {
		//DWORD ddraddr=0L, spiaddr=0x100000L;
		DWORD ddraddr=OSDRGBn_DDR_START, spiaddr=OSDRGBn_SPI_START;
		WORD x=0, y=0;

		if( argc>=3 ) x=a2h(argv[2]);
		if( argc>=4 ) y=a2h(argv[3]);
		
		OsdWriteMode( CPUWRITE | BLT_NO | CONV_DIS | PIXEL16 );
		OsdLoadBmpXYFromSPI( osdRGBn_Header, spiaddr, x, y );

		OsdWinEnable(0, 0);
		OsdWinEnable(1, 0);
		OsdWinEnable(4, 1);
		Osd16Format(OSD16_RGB565);		// 00=YCbCr422, 01=RGB565, 10=RGB4444, 11=RGB1444

	}
	//---------- Bitmap Test "OSDRGB(565) from SPI" ----------
	else if( !stricmp(argv[1], "RGB") ) {
		//DWORD ddraddr=0L, spiaddr=0x100000L;
		DWORD ddraddr=OSDRGB_DDR_START, spiaddr=OSDRGB_SPI_START;
		WORD x=0, y=0;

		if( argc>=3 ) x=a2h(argv[2]);
		if( argc>=4 ) y=a2h(argv[3]);
		
		OsdWriteMode( CPUWRITE | BLT_NO | CONV_DIS | PIXEL16 );
		OsdLoadBmpXYFromSPI( osdRGB_Header, spiaddr, x, y );

		OsdWinEnable(0, 0);
		OsdWinEnable(1, 0);
		OsdWinEnable(4, 1);
		Osd16Format(OSD16_RGB565);		// 00=YCbCr422, 01=RGB565, 10=RGB4444, 11=RGB1444

	}
	//---------- Bitmap Test "OSDaRGB(4444) from SPI" ----------
	else if( !stricmp(argv[1], "aRGBn") ) {
		//DWORD ddraddr=0L, spiaddr=0x100000L;
		DWORD ddraddr=OSDaRGBn_DDR_START, spiaddr=OSDaRGBn_SPI_START;
		WORD x=0, y=0;

		if( argc>=3 ) x=a2h(argv[2]);
		if( argc>=4 ) y=a2h(argv[3]);
		
		OsdWriteMode( CPUWRITE | BLT_NO | CONV_DIS | PIXEL16 );
		OsdLoadBmpXYFromSPI( osdaRGBn_Header, spiaddr, x, y );

		OsdWinEnable(0, 0);
		OsdWinEnable(1, 0);
		OsdWinEnable(4, 1);
		Osd16Format(OSD16_RGB4444);		// 00=YCbCr422, 01=RGB565, 10=RGB4444, 11=RGB1444

	}
	else if( !stricmp(argv[1], "aRGB") ) {
		//DWORD ddraddr=0L, spiaddr=0x100000L;
		DWORD ddraddr=OSDaRGB_DDR_START, spiaddr=OSDaRGB_SPI_START;
		WORD x=0, y=0;

		if( argc>=3 ) x=a2h(argv[2]);
		if( argc>=4 ) y=a2h(argv[3]);
		
		OsdWriteMode( CPUWRITE | BLT_NO | CONV_DIS | PIXEL16 );
		OsdLoadBmpXYFromSPI( osdaRGB_Header, spiaddr, x, y );

		OsdWinEnable(0, 0);
		OsdWinEnable(1, 0);
		OsdWinEnable(4, 1);
		Osd16Format(OSD16_RGB4444);		// 00=YCbCr422, 01=RGB565, 10=RGB4444, 11=RGB1444

	}
	//---------- Bitmap Test "OSDbRGB(1555) from SPI" ----------
	else if( !stricmp(argv[1], "bRGBn") ) {
		//DWORD ddraddr=0L, spiaddr=0x100000L;
		DWORD ddraddr=OSDbRGBn_DDR_START, spiaddr=OSDbRGBn_SPI_START;
		WORD x=0, y=0;

		if( argc>=3 ) x=a2h(argv[2]);
		if( argc>=4 ) y=a2h(argv[3]);
		
		OsdWriteMode( CPUWRITE | BLT_NO | CONV_DIS | PIXEL16 );
		OsdLoadBmpXYFromSPI( osdbRGBn_Header, spiaddr, x, y );

		OsdWinEnable(0, 0);
		OsdWinEnable(1, 0);
		OsdWinEnable(4, 1);
		Osd16Format(OSD16_RGB1555);		// 00=YCbCr422, 01=RGB565, 10=RGB4444, 11=RGB1444

	}
	else if( !stricmp(argv[1], "bRGB") ) {
		//DWORD ddraddr=0L, spiaddr=0x100000L;
		DWORD ddraddr=OSDbRGB_DDR_START, spiaddr=OSDbRGB_SPI_START;
		WORD x=0, y=0;

		if( argc>=3 ) x=a2h(argv[2]);
		if( argc>=4 ) y=a2h(argv[3]);
		
		OsdWriteMode( CPUWRITE | BLT_NO | CONV_DIS | PIXEL16 );
		OsdLoadBmpXYFromSPI( osdbRGB_Header, spiaddr, x, y );

		OsdWinEnable(0, 0);
		OsdWinEnable(1, 0);
		OsdWinEnable(4, 1);
		Osd16Format(OSD16_RGB1555);		// 00=YCbCr422, 01=RGB565, 10=RGB4444, 11=RGB1444

	}
	*/
	//---------- Animation ---------------------
	else if( !stricmp(argv[1], "ani") ) {
		BYTE en=0;
		if( argc>=3 ) en=dat[0];
		if( en ) {
//			AnimationPigeon();
			AnimationON=1;
		}
		else {
			for(en=0; en<MAX_ANIMATION; en++) AnimationData[en].active = 0;
			AnimationON=0;
		}
	}



	//---------- LUT Test-----------------------
	else if( !stricmp(argv[1], "ATTR") ) {
		Printf("\nChange Attribute in LUT: Window %d = %02x", dat[0], dat[1]);
		MonOsdChangeAttr(dat[0], dat[1]);
	}
	//---------- LUT Test-----------------------
	else if( !stricmp(argv[1], "LUTx") ) {
		Printf("\nset Transparent LUT: Window %d, R:%d, G:%d, B:%d", dat[0], dat[1], dat[2], dat[3]);
		OsdLoadTranparentLUT( dat[0], dat[1], dat[2], dat[3] );
	}
	//---------- LUT Test-----------------------
	else if( !stricmp(argv[1], "LUT") ) {
		BYTE xcnt=16, size=10;
		WORD sx=0, sy=0;
		if( argc>=3 ) size=dat[0];
		if( argc>=4 ) xcnt=dat[1];
		if( argc>=5 ) sx=dat[2];
		if( argc>=6 ) sy=dat[3];
		Printf("\nMonOsdLutTest");
		MonOsdLutTest(size, xcnt, sx, sy);
	}
	//---------- LUT Test 16 -------------------
	else if( !stricmp(argv[1], "LUT16") ) {
		Printf("\nMonOsdLutTest16");
		MonOsdLutTest16();
	}
	//---------- LUT Test 565 -------------------
	else if( !stricmp(argv[1], "LUT565") ) {
		Printf("\nMonOsdLutTestRGB");
		MonOsdLutTest565();
		MonOsdLutTest565_2();
	}
	//---------- LUT Test 555 -------------------
	else if( !stricmp(argv[1], "LUT555") ) {
		Printf("\nMonOsdLutTestbRGB");
		MonOsdLutTest555();
		MonOsdLutTest555_2();
	}
	//---------- LUT Test 444 -------------------
	else if( !stricmp(argv[1], "LUT444") ) {
		Printf("\nMonOsdLutTestaRGB");
		MonOsdLutTest444();
	}
	//---------- LUT Test YCbYCr -------------------
	else if( !stricmp(argv[1], "LUTYUV") ) {
		Printf("\nMonOsdLutTestYCbYCr");
		MonOsdLutTestYUV();
	}
	//---------- DDR  Test ---------------------
	else if( !stricmp(argv[1], "t") ) {
		Printf("\nMonOsdTestDDR");
		MonOsdTestDDR();
	}
	else {
		Printf("\nInvalid command...");
	}
}
