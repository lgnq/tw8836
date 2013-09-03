/**
 * @file
 * Test_FOSD.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	FontOSD Test routine
*/

#include "config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "Global.h"


#ifdef TEST_FOSD

//description
//	test 512B OsdRam
//@param
//	fEnd: 0 - start from 0
//		  else - use the end area
void TestOsdRam(BYTE fEnd)
{...}

//description
//	if Panel is 800x480 and font is 12x16, 
//	the max coluum will be 66 = 800 / 12	
//  the max row will be 30 = 480 / 16
void TestDisplayRamWidth()
{}

void TestSampleString()
{ }


//description
//	the upper 256 char need a UP256, and Font Index on OsdRam has 9bit.
//void TestUpper256Char(BYTE columns, BYTE lines, BYET max, BYTE bpp2, BYTE bpp3, BYTE bpp4)
//{...}




//000 - ascii
//
//128 - 2BPP
//
//256 - 3bpp
//
//384 - 4bpp
//


void MultiBppFontSet()
{}

//description
//	Use diff LUT with same MultiBPP on diff FontOsdWin.
void MultiBppLut()
{}

#endif

//*****************************************************************************
//
//								test_FOSD.c
//
//*****************************************************************************
//
//
#include "config.h"
#include "typedefs.h"
#include "reg.h"
#include "printf.h"
#include "Util.h"
#include "I2C.h"
#include "CPU.h"
#include "TW8836.h"
#include "SOsd.h"
#include "FOsd.h"

#include "spi.h"
#include "SpiFlashMap.h"
#include "SOsdMenu.h"



#if 0
void FontOsdWinPuts2(struct menu_item_info_s *item_info, BYTE *str)
{...}
#endif

struct fosd_menu_item_s temp_fosd_menu_item;

//description
//	draw 16x16 table with different foreground/background color.
void DumpFontOsdPalette(void)
{
//	WORD OsdRamAddr;	
	struct fosd_menu_item_s *fosd;
	BYTE i, j;

	fosd = &temp_fosd_menu_item;
	fosd->win = 0;
	//init DE
	FOsdSetDeValue();

	//disable all FontOsd/SpiOsd Windows
	InitFontRamByNum(FONT_NUM_DEF12X18, 0);	//InitFontRam(0, &default_font,"def");
	//FOsdInitBpp3AlphaTable();
	for (i = 0; i < 4; i++)
	{
		FOsdWinInit(i);
		FOsdWinSetOsdRamStartAddr(i, 0);
	}

	WaitVBlank(1);
	FOsdWinEnable(fosd->win,OFF);	// disable

	FOsdWinScreenXY(fosd->win, 0x10, 0x0D);		//sx,sy, n_char, fosd->h);
	FOsdWinScreenWH(fosd->win, 0x20, 0x10);		//sx,sy, n_char, fosd->h);
 	FOsdWinZoom(fosd->win, 1, 0);					//fosd->zoom>>4, fosd->zoom & 0x0F);
//	FOsdWinAlphaPixel(fosd->win, fosd->alpha_color_index,fosd->alpha_value);

	WriteTW88Page(PAGE3_FOSD);
	//--- Write Display RAM
	//DO NOT USE. TW8835FPGA have a BUG. it will reset the SpiOsd.
	//If the length of string is big(more then 8byte), we have to check HSync.
	//WriteTW88(REG300, 0x03 ); // bypass OSD/Font RAM

	//OsdRam Address
	//OsdRamAddr = 0;  //fosd->ram;	// Addr. 00, 0x40, 0x80, 0xC0
	//OsdRamAddr <<= 1;				//make a word size.

	// set address & write the default attribute
	WriteTW88(REG304, 0x0c); // Auto Inc. use under 256 char index

	for (i = 0; i < 0x10; i++)
	{
		for (j = 0; j < 0x10; j++)
		{
			FOsdRamSetAddrAttr((i*16+j) << 1, (i << 4) | j);  //Note:WORD size(BG|FG).

			if (i >= 10)
				WriteTW88(REG307, 0x41+i-10);	//draw A to F
			else
				WriteTW88(REG307, 0x30+i);	//draw 0 to 9

			if (j >= 10)
				WriteTW88(REG307, 0x41+j-10);	//draw A to F
			else
				WriteTW88(REG307, 0x30+j);	//draw 0 to 9
		}
	}

	//=================================
	WriteTW88(REG304, 0x0c);			// Display RAM Access Mode

	FOsdWinEnable(fosd->win, ON);	//win0 enable
}

//description
//	the max font size is a 16x32.
//	If we use 16x32x1x160, it will use whole FontRam size, 10240 Byte.
//	110118. R350[6:0] is for char height and the value*2 is a char height.
void Test16x32FontSet(void)
{
//	WORD OsdRamAddr;	
	struct fosd_menu_item_s *fosd;
	BYTE i,j;
//	BYTE value;
	WORD index;

	BYTE fgColor = 15;
	BYTE bgColor = 1;
	BYTE columns = 16;
	BYTE lines = 10;
	BYTE max = 160;

	fosd = &temp_fosd_menu_item;
	fosd->win = 0;
	//init DE
	FOsdSetDeValue();

	FOsdWinInit(fosd->win);

	WaitVBlank(1);
	FOsdWinEnable(fosd->win,OFF);	// disable

//remove BK120209
//	InitFontRam(0, &test16x32xA0_A0_A0_A0,"test");
	//FOsdInitBpp3AlphaTable();

	FOsdWinSetOsdRamStartAddr(fosd->win, 0);
	FOsdWinAlphaPixel(fosd->win,bgColor,4);		//bgcolor 1 alpha 0

	FOsdWinScreenXY(fosd->win, 0,0);		//0x20, 0x10);		//max 512 = 32x16
	FOsdWinScreenWH(fosd->win, columns, lines);		//0x20, 0x10);		//max 512 = 32x16
 	FOsdWinZoom(fosd->win, 1, 0);						//fosd->zoom>>4, fosd->zoom & 0x0F);

	//set OsdRam.
	WriteTW88Page(PAGE3_FOSD );
	WriteTW88(REG304, 0x0c ); // Auto Inc.

	FOsdRamSetAddrAttr(0, (bgColor << 4) | fgColor);
	 
	for(i=0; i < lines; i++) {
		for(j=0; j < columns; j++) {
			index = i*columns+j;

			if(index >= max) break;

			if(index >= 256)
				WriteTW88(REG304, ReadTW88(REG304) | 0x20);	 
			WriteTW88(REG307, (BYTE)index);
		}
	}
	
	WriteTW88(REG304, ReadTW88(REG304) & 0xDF);		//default: lower 256 char.		
	WriteTW88(REG304, ReadTW88(REG304) & 0xFE);		//OsdRam access mode
	
	FOsdWinEnable(fosd->win,ON);	//win0 enable			
}

//description
//	test upper 256 char
//	download the default font, 12x18x1x160 on address 0 and on address 0xa0 to 0x13F.
//	and dump FontRam on 16x20 OsdRam memory
//  it will include the 0x100~0x13F indexed font.
void TestUpper256Char(void)
{
//	WORD OsdRamAddr;	
	struct fosd_menu_item_s *fosd;
	BYTE i,j;
//	BYTE value;
	WORD index;

	BYTE fgColor = 15;
	BYTE bgColor = 1;
	BYTE columns = 16;
	BYTE lines = 20;
	WORD max = 320;
//	BYTE start_offset = 0xA0;
	BYTE start_offset = 0x00;

	fosd = &temp_fosd_menu_item;
	fosd->win = 0;
	//init DE
	FOsdSetDeValue();

	FOsdWinInit(fosd->win);

	WaitVBlank(1);
	FOsdWinEnable(fosd->win,OFF);	// disable

	InitFontRamByNum(FONT_NUM_DEF12X18, 0);	//InitFontRam(0, &default_font,"def");
    //DownloadExtFont(0xA0, default_font.loc, default_font.size, 320,320,320);	//now, we have 160+160 fonts.
	//InitFontRam(0xA0, &default_font,"def");
	//FOsdInitBpp3AlphaTable();
	//FOsdInitBpp3AlphaTable();
	//FOsdWinAlphaPixel(winno,1,0);		//bgcolor 1 alpha 0
#ifdef SUPPORT_UDFONT
FOsdDownloadUDFontBySerial();
#endif

	FOsdWinAlphaPixel(fosd->win,bgColor,4);		//bgcolor 1 alpha 0

	FOsdWinScreenXY(fosd->win, 0,0);		//0x20, 0x10);		//max 512 = 32x16
	FOsdWinScreenWH(fosd->win, columns, lines);		//0x20, 0x10);		//max 512 = 32x16
 	FOsdWinZoom(fosd->win, 1, 0);						//fosd->zoom>>4, fosd->zoom & 0x0F);

	//set OsdRam.
	WriteTW88Page(PAGE3_FOSD );
	WriteTW88(REG304, 0x0c ); // Auto Inc.

	FOsdRamSetAddrAttr(0, (bgColor << 4) | fgColor);

	for(i=0; i < 0x10 /*lines */; i++) {
		for(j=0; j < columns; j++) {
			index = i*columns+j;

			WriteTW88(REG307, (BYTE)index);
		}
	}
	//RTL BUG: If auto inc from 0 to 0xFF, 0xFE and 0xFF position not working.
	//WaitVBlank(1);
	bgColor=4;	//red
	WriteTW88(REG304, 0x0c ); // Auto Inc.
	FOsdRamSetAddrAttr(0xFD, (bgColor << 4) | fgColor);

	//BKFYI. RTL had problem on 0xFE and 0xFF index. Now it is fixed.
	//		 If use other index, it was OK.
	WriteTW88(REG307, 0xFD);
	WriteTW88(REG307, 0xFE);
	WriteTW88(REG307, 0xFF);

	bgColor=2;	//lime

	//working 
	WriteTW88(REG304, ReadTW88(REG304) | 0x20);	//set UP256 before update attr.

	FOsdRamSetAddrAttr(0x100, (bgColor << 4) | fgColor);
	for(i=0x10; i < lines; i++) {
		for(j=0; j < columns; j++) {
			index = i*columns+j + start_offset;

			//if(index >= (max+start_offset)) break;
			if(index >= max)
				break;
			//if(index >= 256)
			//	WriteTW88(REG304, ReadTW88(REG304) | 0x20);	 
			WriteTW88(REG307, (BYTE)index);
		}
	}


	
	WriteTW88(REG304, ReadTW88(REG304) & 0xDF);		//default: lower 256 char.		
	WriteTW88(REG304, ReadTW88(REG304) & 0xFE);		//OsdRam access mode
	
	FOsdWinEnable(fosd->win,ON);	//win0 enable}
}

//description
//	the max font size is a 16x32.
//	If we use 16x32x1x160, it will use whole FontRam size, 10240 Byte.
//	110118. R350[6:0] is for char height and the value*2 is a char height.
code WORD test_16161616[16] = {
0xFFFF,0xF800,0x8410,0x001F,0xC618,0x07E0,0x0000,0x0010,0xFFE0,0x0410,0x07FF,0x8010,0x8400,0x0400,0x8000,0xF81F
};
void TestMultiBPP4(void)
{
//	WORD OsdRamAddr;	
	struct fosd_menu_item_s *fosd;
	BYTE i,j;
//	BYTE value;
	WORD index;

	BYTE fgColor = 15;
	BYTE bgColor = 1;
	BYTE columns = 16;
//	BYTE lines = 10;
	BYTE lines = 4;
	BYTE max = 160;

	fosd = &temp_fosd_menu_item;
	fosd->win = 0;
	//init DE
	FOsdSetDeValue();

	FOsdWinInit(fosd->win);

	WaitVBlank(1);
	FOsdWinEnable(fosd->win,OFF);	// disable

//remove BK120209
//	InitFontRam(0, &test16x32xA0_20_20_20,"test");
	//FOsdInitBpp3AlphaTable();


	FOsdWinScreenXY(fosd->win, 0,0);		//0x20, 0x10);		//max 512 = 32x16
	FOsdWinScreenWH(fosd->win, columns, lines);		//0x20, 0x10);		//max 512 = 32x16
 	FOsdWinZoom(fosd->win, 1, 0);						//fosd->zoom>>4, fosd->zoom & 0x0F);
	FOsdWinAlphaPixel(fosd->win,bgColor,4);					//bgcolor 1 alpha 0

	//set OsdRam.

	WriteTW88Page(PAGE3_FOSD );
	WriteTW88(REG304, 0x0c ); // Auto Inc.

	//---------------
	// 1BPP
	FOsdRamSetAddrAttr(0, (bgColor << 4) | fgColor);
	for(i=0; i < 2; i++) {
		for(j=0; j < columns; j++) {
			index = i*columns+j;

			WriteTW88(REG307, (BYTE)index);
		}
	}
	//---------------
	// 4BPP
	//I have to update LUT value
	FOsdSetPaletteColorArray(32, test_16161616, 16, 0);					//BUG_BANK

	FOsdRamSetAddrAttr(0x20, 32 >> 2);	  // fgColor is used for LUT offset. I will use 32th
	//--------------------
	// assign the start loc of 4BPP char index.
	//--------------------
	for(i=0; i < 2; i++) {
		for(j=0; j < columns; j++) {
			index = i*columns+j;
			index *= 4;
			index += 0x20;
			WriteTW88(REG307, (BYTE)index);
		}
	}
	
	WriteTW88(REG304, ReadTW88(REG304) & 0xDF);		//default: lower 256 char.		
	WriteTW88(REG304, ReadTW88(REG304) & 0xFE);		//OsdRam access mode
	
	FOsdWinEnable(fosd->win,ON);	//win0 enable

	WriteTW88Page(PAGE3_FOSD );
	WriteTW88(REG310, ReadTW88(REG310) | 0x40 ); // temp - enable MultiBPP on Win1
				
}

//code FONT_SPI_INFO_t consolas16x26_606C90_test 	= { 0x402000, 0x2080, 16, 26, 0x060, 0x06C, 0x090, 0x0A0 };
/* <NUMBER OF FONTS>=0xA0(160. MAX) */
/* <FONT WIDTH>=16 */
/* <FONT HEIGHT>=26 */
/* <2bit COLOR START>=0x060 */
/* <3bit COLOR START>=0x06C */
/*       Digit = 0x6C       */
/* <4bit COLOR START>=0x090 */
/*       Penguin = 0x98     */

code WORD consolas16x26_606C90_2BPP_test[4] = {
	0xF7DE,0x0000,0x5AAB,0xC000
}; 

code WORD consolas16x26_606C90_3BPP[8] = {
	0xFFFF,0x0000,0xDEDB,0x9492,0x6B6D,0xB5B6,0x4A49,0x2124
}; 
code WORD consolas16x26_606C90_4BPP[16] = {
	0xD6BA,0x20E3,0xF79E,0x62E8,0xE104,0xA944,0x39A6,0x7BAC,
	0x51A6,0xC617,0x9CD1,0xB5B5,0x9BC9,0xDD85,0xF643,0xAC87
};

//BANK issue
void FontOsdSetPaletteColorArray_TEST(BYTE index, WORD *array, BYTE size)
{
	BYTE i;
	BYTE r30c;

	WriteTW88Page(PAGE3_FOSD );
	r30c = ReadTW88(REG30C) & 0xC0;
	//dPrintf("\nFontOsdSetPaletteColorArray index:%bd",index);
	for(i=0; i < size; i++) {
		WriteTW88(REG30C, (index+i) | r30c); //index 2 is for normal foreground	   Now 6bit.(64).
		WriteTW88(REG30D, (BYTE)(array[i] >> 8));
		WriteTW88(REG30E, (BYTE)array[i]);
		//dPrintf(" 0x%02bx%02bx(%04x)",(BYTE)(array[i] >> 8),(BYTE)array[i],array[i]);
	}
}


#ifdef UNCALLED_SEGMENT_CODE
void TestNum1BPP3BPP_OLD(void)
{...}
#endif

//void TestNumAlpha(void)
//palette map
//
//   64
//	+-------------
//	|60
//	|56
//	|52
//	|48		4BPP  original 16 palette
//	+-------------
//	|44
//	|40		unused area
//	+-------------
//	|36
//	|32		3BPP   original 8 palette
// 	+-------------
//	|28
//	|24		3BPP Lime with Alpha
// 	+-------------
//	|20
//	|16		3BPP Black with Alpha
// 	+-------------
//	|1BPP Area.
//	|15		White
//	|14		Yellow					
//	|13		Magenta					
//	|12		Red					
//	|11		Cyan					
//	|10		Lime					
//	|9		Blue					
//	|8		Silver					
//	|7		Gray					
//	|6		DK Yellow					
//	|5		DK Magenta							
//	|4		DK Red							
//	|3		DK Cyan							
//	|2		Green							
//	|1		DK Blue
//	|0		Black
// 	+-------------
//	OsdRam
//	0..9	0..9  with 3BPP
//	4..5	penguin	with 4BPP
//	10..19
//	20..29	0x10..0x19
//	|25..29		1BPP	(9 << 4 | 10)	bg:LTBlue fg:Lime
//	|20..24		1BPP	(15<<4|0)		bg:white fg:black 
//	30..39	0..9 with 3BPP. Black with Alpha
//  40..49  0..9 with 3BPP. Lime with Alpha
//
void TestNum1BPP3BPP(void)
{
	struct fosd_menu_item_s *fosd;
	BYTE i; //,j,k;
	BYTE palette;
	BYTE fgColor = 15; //color is reversed on consolas16x26_606C90
	BYTE bgColor = 0;
	BYTE columns = 10;
	BYTE lines = 5;

    //select window
	fosd = &temp_fosd_menu_item;
	fosd->win = 0;

	//init DE
	FOsdSetDeValue();
	//assign default
	FOsdWinInit(fosd->win);

	WaitVBlank(1);
	FOsdWinEnable(fosd->win,OFF);	// disable

	//download FontSet
	InitFontRamByNum(FONT_NUM_CONGRAY16X28, 0);	//InitFontRam_consolas(0);

	FOsdInitBpp3AlphaTable(1);

	FOsdWinScreenXY(fosd->win, 50,60);		//0x20, 0x10);		//max 512 = 32x16
 	FOsdWinScreenWH(fosd->win, columns, lines);		//0x20, 0x10);		//max 512 = 32x16
	FOsdWinZoom(fosd->win, 3, 2);						//fosd->zoom>>4, fosd->zoom & 0x0F);
	//FOsdWinAlphaPixel(fosd->win,15 /*AlphaColor*/ ,8);				//bgcolor 1 alpha 0

	//set OsdRam.

	WriteTW88Page(PAGE3_FOSD );
	WriteTW88(REG304, 0x0c ); // Auto Inc.

#if 0
	//first line - 1BPP+4BPP
	FOsdRamSetAddrAttr(0, (bgColor << 4) | fgColor);
	for(i=0; i < 10; i++) {
		WriteTW88(REG307, i+0x10);
	}
	palette = 48;
	FOsdSetPaletteColorArray(palette, consolas16x26_606C90_4BPP, 16, 0);	//BUG_BANK
	FOsdRamSetAddrAttr(4, palette >> 2);  //overwrite
	WriteTW88(REG307, 0x7E);
	WriteTW88(REG307, 0x82); 

	//second line - 3BPP with original BGR palette.
	palette = 32;
	FOsdSetPaletteColorArray(palette, consolas16x26_606C90_3BPP, 8, 0);	//BUG_BANK
	//FOsdWinAlphaPixel(fosd->win, palette+0,8); //NOTE:LUT32 do not support Alpha.
	FOsdRamSetAddrAttr(10, palette >> 2);
	for(i=0; i < 10; i++) {
			WriteTW88(REG307, i*3+BPP3DIGIT_START);
	}
#endif

	//first line - 3BPP with original BGR palette. with 4BPP.
	palette = 32;
	FontOsdSetPaletteColorArray_TEST(palette, consolas16x26_606C90_3BPP, 8);
	//FOsdWinAlphaPixel(fosd->win, palette+0,8); //NOTE:LUT32 do not support Alpha.
	FOsdRamSetAddrAttr(0, palette >> 2);
	for(i=0; i < 10; i++) {
			WriteTW88(REG307, i*3+BPP3DIGIT_START);
	}
	palette = 48;
	FontOsdSetPaletteColorArray_TEST(palette, consolas16x26_606C90_4BPP, 16);
	FOsdRamSetAddrAttr(4, palette >> 2);  //overwrite
	WriteTW88(REG307, BPP4PENGUIN_START);
	WriteTW88(REG307, BPP4PENGUIN_START+4); 

#if 0
	//second line - 3BPP with original BGR palette & background alpha.
	palette = 0;
	//FontOsdSetPaletteColorArray(palette, consolas16x26_606C90_3BPP, 8);
	FOsdSetPaletteColor(palette+0, 0xFFFF,1, 0);
	FOsdSetPaletteColor(palette+1, 0x0000,1, 0);
	FOsdSetPaletteColor(palette+2, 0x8410,1, 0); //or 0xC618(silver)
	FOsdSetPaletteColor(palette+3, 0x8410,1, 0);
	FOsdSetPaletteColor(palette+4, 0x8410,1, 0);
	FOsdSetPaletteColor(palette+5, 0x8410,1, 0);
	FOsdSetPaletteColor(palette+6, 0x8410,1, 0);
	FOsdSetPaletteColor(palette+7, 0x0000,1, 0);
	FOsdWinAlphaPixel(fosd->win, palette+0,8); //NOTE:LUT32 do not support Alpha.
	FOsdRamSetAddrAttr(10, palette >> 2);
	for(i=0; i < 10; i++) {
			WriteTW88(REG307, i*3+BPP3DIGIT_START);
	}
#endif
	//Third Line
	//01234(1BPP white|black) 56789(1BPP LTBlue|Lime)
	//1BPP
	FOsdRamSetAddrAttr(20, 15 << 4 | 0);	  //bg:white fg:black
	for(i=0; i < 5; i++) {
			WriteTW88(REG307, i+0x10);
	}
	FOsdRamSetAddrAttr(25, 9 << 4 | 10);	  //bg:LTBlue fg:Lime
	for(i=0; i < 5; i++) {
			WriteTW88(REG307, i+0x10+5);
	}
	//forth line - 3BPP with Black & alpha.
	palette = FOSD_LUT_MAP_BPP3_START;
	FOsdWinSetBpp3Alpha(fosd->win,palette);	//use BPP3_alpha_lut_offset[].
	FOsdSetPaletteColor(palette, FOSD_COLOR_VALUE_BLACK, 8, 0);	//all black		
	FOsdRamSetAddrAttr(30, palette >> 2);		//multi-bpp
	for(i=0; i < 10; i++) {
			WriteTW88(REG307, i*3+BPP3DIGIT_START);
	}
	//fifth line - 3BPP with Lime & alpha.
	palette = FOSD_LUT_MAP_BPP3_ALPHA_G_START;
	FOsdWinSetBpp3Alpha(fosd->win,palette);	//use BPP3_alpha_lut_offset[].
	FOsdSetPaletteColor(palette, FOSD_COLOR_VALUE_LIME, 8, 0);		
	FOsdRamSetAddrAttr(40, palette >> 2);		//multi-bpp
	for(i=0; i < 10; i++) {
			WriteTW88(REG307, i*3+BPP3DIGIT_START);
	}

	//post fix
	WriteTW88(REG304, ReadTW88(REG304) & 0xDF);		//default: lower 256 char.		
	WriteTW88(REG304, ReadTW88(REG304) & 0xFE);		//OsdRam access mode
	
	FOsdWinEnable(fosd->win,ON);	//win0 enable

	WriteTW88Page(PAGE3_FOSD );
	WriteTW88(REG310, ReadTW88(REG310) | 0x40 ); // temp - enable MultiBPP on Win1
}

//draw default HW LUT & SW LUT
//test method
//	Boot with NoInit mode
//	type	init core
//			ee find
//			init
//			fosd test1  <-- low 8 is a HW
//	Boot with NoInit mode
//	type	init core
//			ee find
//			init
//			fosd test2	<-- high 8 is a HW
void FOsdTest_1(BYTE mode)
{
	BYTE WIN0_OSDRAM = 0;
	BYTE WIN1_OSDRAM = 0x10;
	BYTE WIN2_OSDRAM = 0x20;
	BYTE i;

	//
	WORD temp_FOsdDefPaletteBpp1[16] = {
	/*0:Black*/			FOSD_COLOR_VALUE_BLACK,
	/*1:DarkBlue*/		FOSD_COLOR_VALUE_DBLUE,
	/*2:Green*/			FOSD_COLOR_VALUE_GREEN,
	/*3:DarkCyan*/		FOSD_COLOR_VALUE_DCYAN,
	/*4:DarkRed*/		FOSD_COLOR_VALUE_DRED,
	/*5:DarkMagenta*/	FOSD_COLOR_VALUE_DMAGENTA,
	/*6:DarkYellow*/	FOSD_COLOR_VALUE_DYELLOW,
	/*7:Gray*/			FOSD_COLOR_VALUE_GRAY,

	/*8:Silver*/		FOSD_COLOR_VALUE_SILVER,
	/*9:Blue*/			FOSD_COLOR_VALUE_BLUE,
	/*A:Lime*/			FOSD_COLOR_VALUE_LIME,
	/*B:Cyan*/			FOSD_COLOR_VALUE_CYAN,
	/*V:Red*/			FOSD_COLOR_VALUE_RED,
	/*D:Magenta*/		FOSD_COLOR_VALUE_MAGENTA,
	/*E:Yellow*/		FOSD_COLOR_VALUE_YELLOW,
	/*F:White*/			FOSD_COLOR_VALUE_WHITE 
	};

	InitFontRamByNum(FONT_NUM_DEF12X18, 0);	//InitFontRam(0, &default_font,"def");


	FOsdRamSetFifo(ON, 0);	//FontOsdSetOsdRamFifo(OFF);						//default

	//set OsdRam data&attr
	//FontOsdSetAutoIncMode(FOSD_AUTOINC_NORMAL);
	WriteTW88(REG304, 0x00);
	WriteTW88(REG305, 0x00);
	for(i=0; i < 8; i++) {
		WriteTW88(REG306, WIN0_OSDRAM+i);
		WriteTW88(REG307, FOSD_ASCII_BLANK);
		WriteTW88(REG306, WIN0_OSDRAM+i);
		WriteTW88(REG308, i << 4 );			//change BG color. LOW 8
		//if(i==7)
		//	delay1ms(1);
	}
	for(i=0; i < 8; i++) {
		WriteTW88(REG306, WIN1_OSDRAM+i);
		WriteTW88(REG307, FOSD_ASCII_BLANK);
		WriteTW88(REG306, WIN1_OSDRAM+i);
		WriteTW88(REG308, (i << 4) + 0x80);	//change BG color. High 8
		//if(i==7)
		//	delay1ms(1);
	}
	for(i=0; i < 16; i++) {
		WriteTW88(REG306, WIN2_OSDRAM+i);
		WriteTW88(REG307, FOSD_ASCII_BLANK);
		WriteTW88(REG306, WIN2_OSDRAM+i);
		WriteTW88(REG308, (i << 4) );			//change BG color.
		if(i==7)
			delay1ms(1);
	}

	//test palette from 16 to 31
	if(mode==0) {
		for(i=8;i<16;i++) 
			FOsdSetPaletteColor(i, temp_FOsdDefPaletteBpp1[i-8],1, 0);
	}
	else {
		for(i=0;i<8;i++) 
			FOsdSetPaletteColor(i, temp_FOsdDefPaletteBpp1[i+8],1, 0);
	}

	//enable
	FOsdWinScreenXY(FOSD_WIN0,80,0);
	FOsdWinScreenWH(FOSD_WIN0,8,1);
	FOsdWinZoom(FOSD_WIN0, 2, 1);
	FOsdWinSetOsdRamStartAddr(FOSD_WIN0,WIN0_OSDRAM);
	FOsdWinEnable(FOSD_WIN0,ON);	

	FOsdWinScreenXY(FOSD_WIN1,80,37);
	FOsdWinScreenWH(FOSD_WIN1,8,1);
	FOsdWinZoom(FOSD_WIN1, 2, 1);
	FOsdWinSetOsdRamStartAddr(FOSD_WIN1,WIN1_OSDRAM);
	FOsdWinEnable(FOSD_WIN1,ON);

	FOsdWinScreenXY(FOSD_WIN2,80,37*2);
	FOsdWinScreenWH(FOSD_WIN2,16,1);
	FOsdWinZoom(FOSD_WIN2, 2, 1);
	FOsdWinSetOsdRamStartAddr(FOSD_WIN2,WIN2_OSDRAM);
	FOsdWinEnable(FOSD_WIN2,ON);

	FOsdOnOff(ON, 0);	//with vdelay 0	
}
//void FOsdTest_3(void)
//{
//}


