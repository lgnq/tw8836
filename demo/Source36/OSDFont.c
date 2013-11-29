/**
 * @file
 * OSDFont.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	low level FontOSD layer
 *
 * errata
 *	MultiFont can not be overlapped.
 */

#include "config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"
#include "global.h"

#include "CPU.h"
#include "printf.h"
#include "Util.h"

#include "i2c.h"
#include "spi.h"

#include "FOsd.h"
#include "SpiFlashMap.h"
#include "SOsdMenu.h"
#include "settings.h"
#include "inputCtrl.h"

//=====================================
// default LUT(palette) color
//=====================================
/**
* FOSD HW 16 default color table
*/
code WORD FOsdHwDefPaletteBpp1[16] =
{
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

/**
* FOSD SW 16 default color table
*/
code WORD FOsdSwDefPaletteBpp1[16] =
{
	/*0:Black*/			FOSD_COLOR_VALUE_BLACK,
	/*9:Blue*/			FOSD_COLOR_VALUE_BLUE,
	/*A:Lime*/			FOSD_COLOR_VALUE_LIME,
	/*B:Cyan*/			FOSD_COLOR_VALUE_CYAN,
	/*V:Red*/			FOSD_COLOR_VALUE_RED,
	/*D:Magenta*/		FOSD_COLOR_VALUE_MAGENTA,
	/*E:Yellow*/		FOSD_COLOR_VALUE_YELLOW,
	/*F:White*/			FOSD_COLOR_VALUE_WHITE, 

	/*7:Gray*/			FOSD_COLOR_VALUE_GRAY,
	/*1:DarkBlue*/		FOSD_COLOR_VALUE_DBLUE,
	/*2:Green*/			FOSD_COLOR_VALUE_GREEN,
	/*3:DarkCyan*/		FOSD_COLOR_VALUE_DCYAN,
	/*4:DarkRed*/		FOSD_COLOR_VALUE_DRED,
	/*5:DarkMagenta*/	FOSD_COLOR_VALUE_DMAGENTA,
	/*6:DarkYellow*/	FOSD_COLOR_VALUE_DYELLOW,
	/*8:Silver*/		FOSD_COLOR_VALUE_SILVER
};

/**
* FOSD 2 BPP default color table
*/
code WORD FOsdDefPaletteBpp2[4] =
{	
	0xF7DE, 0x0000, 0x5AAB, 0xC000
}; 

/**
* FOSD 3 BPP default color table
*/
code WORD FOsdDefPaletteBpp3[8] =
{	
	0xFFFF, 0x0000, 0xDEDB, 0x9492, 0x6B6D, 0xB5B6, 0x4A49, 0x2124		//consolas22_16x26
};

/**
* FOSD 3 BPP Alpha default color table
*/
code BYTE FOsdDefPaletteBpp3Alpha[8] = 
{
	1, 7, 6, 4, 3, 5, 2, 0 
};		//consolas22_16x26
 
/**
* FOSD 3 BPP default color table
*/
code WORD FOsdDefPaletteBpp4[16] =
{
	0xD6BA, 0x20E3, 0xF79E, 0x62E8, 0xE104, 0xA944, 0x39A6, 0x7BAC,
	0x51A6, 0xC617, 0x9CD1, 0xB5B5, 0x9BC9, 0xDD85, 0xF643, 0xAC87
};

#ifdef SUPPORT_UDFONT
#include "data/RamFontData.inc"
#endif

/**
* FOSD FontInfo_s
*/
struct FontInfo_s
{
	char name[16];	//con, con+gray, def
	BYTE w, h;
	BYTE bpp;
	BYTE loc;		//fontram location

	WORD bpp2, bpp3, bpp4, max;								
								
	BYTE bpp2_attr;		//for MultiBPP	(LUT index >> 2)
	BYTE bpp3_attr;		//for MultiBPP	(LUT index >> 2)
	BYTE bpp4_attr;		//for MultiBPP	(LUT index >> 2)
};

/**
* FOSD FontWinInfo_s
*/
struct FontWinInfo_s
{
	WORD osdram;		//osdram start offset, 0~511.
	WORD sx, sy;			//pixel base
	BYTE column, line;  	//char base
	BYTE x, y;			//last position. char base.

	BYTE bpp1_attr;		//(bgColor << 4) | fgColor
};

/**
* FOSD FontOsdInfo_s
*/
struct FontOsdInfo_s
{
	struct FontInfo_s font;
	struct FontWinInfo_s win[FOSD_MAX_OSDWIN];
} FontOsdInfo;

//R307[7:0]R308[7:0]	OSD RAM Data Port
//R309					Font RAM Address
//R30A					Font RAM Data Port
//R30B					Multi-Color Start Position
//R30C					Font OSD Control
//R30D[7:0]R30E[7:0]	Character Color Look-up table data port

#define DMA_SIZE	0x8000L

#define	FONTWIN1_ST		REG310
#define	FONTWIN2_ST		REG320
#define	FONTWIN3_ST		REG330
#define	FONTWIN4_ST		REG340
#define	FONTWIN5_ST		REG350
#define	FONTWIN6_ST		REG360
#define	FONTWIN7_ST		REG370
#define	FONTWIN8_ST		REG380

#define	FONTWIN_ENABLE	0X00
#define	FONT_ALPHA		0x01

#ifdef MODEL_TW8835
code WORD FOSDWinBase[] = 
{ 
	FONTWIN1_ST,
	FONTWIN2_ST,
	FONTWIN3_ST,
	FONTWIN4_ST
};
#else	//TW8836
code WORD FOSDWinBase[] =
{ 
	FONTWIN1_ST,
	FONTWIN2_ST, 
	FONTWIN3_ST,
	FONTWIN4_ST,
	FONTWIN5_ST,
	FONTWIN6_ST,
	FONTWIN7_ST,
	FONTWIN8_ST
};
#endif

//BKTODO: move default font in this bank. 
//it have a BANK issue.
	 BYTE BPP3_alpha_lut_offset[8];					//need base_lut to use.
code BYTE BPP3_alpha_value[8] = 
{
	0, 2, 3, 4, 5, 6, 7, 8
};

/**
* print FontInfo
*/
void DumpFontInfo(void)
{
	ePrintf("\nFont:%s", FontOsdInfo.font.name);
	ePrintf(" %bdx%bd", FontOsdInfo.font.w, FontOsdInfo.font.h);
	ePrintf(" bpp2:%x", FontOsdInfo.font.bpp2);
	ePrintf(" bpp3:%x", FontOsdInfo.font.bpp3);
	ePrintf(" bpp4:%x", FontOsdInfo.font.bpp4);
	ePrintf(" end:%x", FontOsdInfo.font.max);
}

//=============================================================================
//r300[0]
//=============================================================================
/**
* set FOSD OsdRam FIFO
*
* OsdRam uses a PCLK domain.
*
* If FIFO is ONed, HW has a 8 FIFO, and updata OsdRam on HBlank(Horizontal).
* If FW wants to update the OsdRam related registers more than 8,
* FW has to wait one HBlank.
*
* If FIFO is OFFed, FW has to execute the updata on VBlank(Vertical).
* If FW turns on FIFO before HW updates a OsdRam related registers,
* HW can write data|attr at the ramdom position.
* So, FW has to wait 1 VBlank.
*
* oldname:FOsdRamFifoBypass
* 
* HW default: FIFO ON
* SW prefer: FIFO OFF
*
*	register
*	r300[0]	1:Bypass OSD RAM FIFO.
*/
void FOsdRamSetFifo(BYTE fOn, BYTE vdelay)
{
	if (vdelay)
		WaitVBlank(vdelay);

	WriteTW88Page(PAGE3_FOSD);
	if (fOn)
		WriteTW88(REG300, ReadTW88(REG300) & ~0x01);	//turn off bypass, so FIFO will be ON.
	else
		WriteTW88(REG300, ReadTW88(REG300) | 0x01);		//turn on bypass, so FIFO will be OFF.
}

/**
* set FOSD FontRam Fifo
*
* FIFO has to be OFF if FW uses a DMA.
* If FW uses IO method, FW has to wait HBlank every 8 data.
*
* HW default: FIFO ON
* SW default: FIFO OFF
*
*	register
*	r300[1]	1:Bypass FONTRAM FIFO
* 
*/
void FOsdFontSetFifo(BYTE fOn)
{
	WriteTW88Page(PAGE3_FOSD);

	if (fOn)
		WriteTW88(REG300, ReadTW88(REG300) & ~0x02);	//turn off bypass, so FIFO will be ON.
	else
		WriteTW88(REG300, ReadTW88(REG300) |  0x02);	//turn on bypass, so FIFO will be OFF.
}

/**
* set FONT Width & Height
*
*	r300[4]		0: CharWidth 12. 1:CharWidth 16.
*	r350[4:0]	(Font OSD Char Height ) >> 1
*	r351[6:0]	Sub-Font Total Count. used bytes for one font. if 12x18, use 27.
*/
void FOsdSetFontWidthHeight(BYTE width, BYTE height)
{
	BYTE value;
	WriteTW88Page(PAGE3_FOSD);

	value = ReadTW88(REG300);	
	if (width == 16)
		value |= 0x10;	   				//width 16
	else
		value &= 0xEF;					//   or 12
	WriteTW88(REG300, value); 

	WriteTW88(REG_FOSD_CHEIGHT, height >> 1); 					//Font height(2~32)
	WriteTW88(REG_FOSD_MUL_CON, (width >> 2) * (height >> 1));	//sub-font total count.
}

/**
* calculate and set FontOSD DE value
*
* Font OSD DE delay calculation 
* HDE = REG(0x210[7:0])
* PCLKO = REG(0x20d[1:0]) {0,1,1,1}
* Mixing = REG(0x400[1:1])
*	
* result = HDE + PCLKO - (Mixing*2 + 36)
*
* NOTE: minimum DE value.
*	- 1BPP: 3
*	- 2BPP: 4
*	- 3BPP: 5
*	- 4BPP: 6
*
*	r303[7:0]
*/ 
void FOsdSetDeValue(void)
{
#if 1 //NEW, But NG
	XDATA BYTE temp;
	BYTE HDE, PCLKO, Mixing;

	WriteTW88Page(PAGE2_SCALER);
	HDE = ReadTW88(REG210);				// HDE
	PCLKO = ReadTW88(REG20D) & 0x03;
	//if(PCLKO == 3)
		PCLKO = 1;

	WriteTW88Page(PAGE4_SOSD);
	Mixing = ReadTW88(REG400) & 0x02 ? 1 : 0;

	if ((HDE + PCLKO) < (Mixing*2 + 36)) //I don't want a negative value.
	{
		//temp = 0;
		temp = (Mixing*2 + 36) - HDE - PCLKO + 1;
	}
	else
		temp = HDE + PCLKO - (Mixing*2 + 36);

	WriteTW88Page(PAGE3_FOSD);
	WriteTW88(REG303, temp);   				// write FONT OSD DE value
#else //Old, But Working
	XDATA	WORD temp;

	WriteTW88Page(PAGE2_SCALER );
	temp = ReadTW88(REG210 );				// HDE
	if( ReadTW88(REG20d) & 0x03 )
		temp += 1;

	temp -= 37;
	WriteTW88Page(PAGE4_SOSD );
	if ( ReadTW88(REG400 ) & 2 ) {			// check Mixing
		temp -= 2;
	}	

	WriteTW88Page(PAGE3_FOSD );
	WriteTW88(REG303, temp );   				// write FONT OSD DE value
#endif

#ifdef DEBUG_FOSD
	dPrintf("\nFontOsdDE:%02bx", temp);
#endif
}

/**
* set FOSD blink attribute
*
* this will effect only when FW writes data on OsdRam.
* the background color will fill out when it blink.
*
*	r304[7] Blink
*/
void FOsdBlinkOnOff(BYTE fOn)
{
	WriteTW88Page(PAGE3_FOSD);

	if (fOn)
		WriteTW88(REG304, ReadTW88(REG304) | 0x80);
	else
		WriteTW88(REG304, ReadTW88(REG304) & ~0x80);
}

#if 0
//please do not use it. it has lot of bugs.
//desc: enable the char effect(border or Shadow)
//		this will effect only when you write data on OsdRam.
// r304[4]
void FOsdEnableCharEffect(BYTE fOn)
{
..
}
#endif

/**
* set FOSD OsdRam Write Mode
*
* @param fMode
*	0:Normal, 1:DataOrAttr, 3:Data(with prev attr)
*
*	register
*	r304[3:2] 00:Normal. 01:Data or Attr Addr auto mode. 11:Font Data auto mode
*/
void FOsdRamSetWriteMode(BYTE fMode)
{
	BYTE value;
	
	WriteTW88Page(PAGE3_FOSD);

	value = ReadTW88(REG304) & 0xF3;
	value |= (fMode << 2);

	WriteTW88(REG304, value);
}

#if 0
/**
* get FOSD OsdRam Write Mode
*
*/
BYTE FOsdRamGetWriteMode(void)
{
	WriteTW88Page(PAGE3_FOSD);
	return (ReadTW88(REG304) >> 2) & 0x03;
}
#endif

#if 0
/**
* Clear all data as 0 with current attr value at r308[7:0].
*
* If customer wants to use this feature, the font 0 has to be a blank data.
*
*	r304[1]
*/
void FOsdRamClearAllData(void)
{
	WriteTW88Page(PAGE3_FOSD);
	WriteTW88(REG304, ReadTW88(REG304) | 0x02);
}
#endif

/**
* set FOSD AccessMode
*
* FW selects FontRAM before it starts a FontRam download
* and selects OsdRam when it finish a FontRam download.
*
* @param fType   
*	- 0: OSD Ram Access.  default
*	- 1: FONT RAM access
*
*	register
*	r304[0]
*/
void FOsdSetAccessMode(BYTE fType)
{
	WriteTW88Page(PAGE3_FOSD);

	if (fType)
		WriteTW88(REG304, ReadTW88(REG304) | 0x01);	// Font Ram Access
	else
		WriteTW88(REG304, ReadTW88(REG304) & 0xFE);	// Osd Ram Access
}

/**
* set OsdRam address
*
* @param addr max 511.
*	r305[0]r306[7:0]
*/
void FOsdRamSetAddress(WORD addr)
{
	WriteTW88Page(PAGE3_FOSD);
	WriteTW88(REG305, (ReadTW88(REG305) & 0xFE) | (addr >> 8));
	WriteTW88(REG306, (BYTE)addr);
}

/**
* set OsdRam data
*
*	r304[5]r307[7:0]
*	NOTE:FontRam also use r304[5]
*/
void FOsdRamSetData(WORD dat)
{
	WriteTW88Page(PAGE3_FOSD);

	if (dat & 0x100)
		WriteTW88(REG304,ReadTW88(REG304) |  0x20); 
	else
		WriteTW88(REG304,ReadTW88(REG304) & ~0x20);

	WriteTW88(REG307, (BYTE)dat);
}

/**
* set OsdRam attribute
*
*	r308[7:0]
*	1BPP: 	r308[7:4] bgColor
*			r308[3:0] fgColor
*	MBPP:   r308[3:0] (LUT offset / 4)
*/ 
void FOsdRamSetAttr(BYTE attr)
{
	WriteTW88Page(PAGE3_FOSD);
	WriteTW88(REG308, attr);
}

#if 0
/**
*	FontRam Serial write
*
* NOTE: You need FOsdSetAccessMode(FOSD_ACCESS_FONTRAM),
*       FOsdSetAccessMode(FOSD_ACCESS_OSDRAM)
* NOTE: You need a FIFO delay if you are using FIFO. 
*       So, the better higher function is a FOsdDownloadFontBySerial().
*
*	r304[5]r309[7:0]		//NOTE: OsdRam also use r304[5] 
*	r30A[7:0]
*/
void FOsdFontWrite(WORD start, BYTE *dat, BYTE bytesperfont, BYTE size)
{
	BYTE i, j;
	WORD addr;

	WriteTW88Page(PAGE3_FOSD);
	addr = start;
	for (i = 0; i < size; i++)
	{
		if (addr & 0x100)
			WriteTW88(REG304, ReadTW88(REG304) |  0x20); 	//Upper256
		else
			WriteTW88(REG304, ReadTW88(REG304) & ~0x20);

		WriteTW88(REG309, (BYTE)addr);
		for (j=0; j < bytesperfont; j++)
			WriteTW88(REG30A, *dat++);
		addr++;		
	}
}
#endif

//r30B[7:0]
//	MADD2

/**
* OnOff FontOSD
*
*	r30C[6]
*
* @return
*	- 1: changed
*/
BYTE FOsdOnOff(BYTE fOnOff, BYTE vdelay)
{
	BYTE value;
	
	WriteTW88Page(PAGE3_FOSD);

	value = ReadTW88(REG30C);
	if (fOnOff)
	{
		if (value & 0x40)
		{
			if (vdelay)
				WaitVBlank(vdelay);
			WriteTW88(REG30C, value & ~0x40);

			return 1;
		}
	}
	else
	{
		if ((value & 0x40) == 0)
		{
			if (vdelay)
				WaitVBlank(vdelay);
			WriteTW88(REG30C, value | 0x40);

			return 1;
		}
	}
	
	return 0;
}

//r30C[5:0]
//r30D[7:0]
//r30E[7:0]
//=============================================================================
// FOsdPalette
//=============================================================================
/**
* set FOSD Palette color
*
* old code use index 2 for normal foreground	   Now 6bit.(64).
* TW8835 needs PCLK divider2
*/
void FOsdSetPaletteColor(BYTE start, WORD color, BYTE size, BYTE vdelay)
{
	BYTE i;
	BYTE r30c;

	if (vdelay)
		WaitVBlank(vdelay);

	McuSpiClkToPclk(CLKPLL_DIV_2P0);

	WriteTW88Page(PAGE3_FOSD);

	r30c = ReadTW88(REG30C) & 0xC0;
	for (i = start; i < (start+size); i++)
	{
		WriteTW88(REG30C, r30c | i );
		WriteTW88(REG30D, (BYTE)(color>>8));
		WriteTW88(REG30E, (BYTE)color);
	}
	
	McuSpiClkRestore();
}

/**
* set FOSD Palette color
*
* TW8835 needs PCLK divider2
*/
void FOsdSetPaletteColorArray(BYTE index, WORD *array, BYTE size, BYTE vdelay)
{
	BYTE i;
	BYTE r30c;

	//dPrintf("\nFOsdSetPaletteColorArray index:%bd",index);
	//for(i=0; i < size; i++)
	//	dPrintf(" %04x",array[i]);

	if (vdelay)
		WaitVBlank(vdelay);

#if defined(TW8836_CHIPDEBUG)
	//BK120615. for test clock source
#else
	McuSpiClkToPclk(CLKPLL_DIV_2P0);
#endif

	WriteTW88Page(PAGE3_FOSD);

	r30c = ReadTW88(REG30C) & 0xC0;
	for (i = 0; i < size; i++)
	{
		WriteTW88(REG30C, (index+i) | r30c);
		WriteTW88(REG30D, (BYTE)(array[i] >> 8));
		WriteTW88(REG30E, (BYTE)array[i]);
	}
	
#if defined(TW8836_CHIPDEBUG)
	//BK120615. for test clock source
#else
	McuSpiClkRestore();
#endif
}

/**
* set Address and Attribute
*
* we assign the attr and then wirte a start font index.
* after this, you can just assign the font index data. 
* @param OsdRamAddr 0 to 511
* @param attr
*	- attr for 1BPP:	(bgColor << 4) | fgColor
*	- attr for MultiBPP	(LUT index >> 2)
*/
void FOsdRamSetAddrAttr(WORD OsdRamAddr, BYTE attr)
{
	FOsdRamSetWriteMode(FOSD_OSDRAM_WRITE_NORMAL);
	FOsdRamSetAddress(OsdRamAddr); 
	FOsdRamSetAttr(attr);																			

	FOsdRamSetWriteMode(FOSD_OSDRAM_WRITE_DATA);
	FOsdRamSetAddress(OsdRamAddr);	//NOTE: HW needs it.
	//							
	//now use FOsdRamSetData(dat).	
}

/**
* write BYTE string to OsdRam
*
* assume low char  
* assume FIFO OFF
*/
void FOsdRamWriteByteStr(BYTE *str, BYTE len)
{
	BYTE i;
	BYTE w_count;

	WriteTW88Page(PAGE3_FOSD);
	WriteTW88(REG304, ReadTW88(REG304) & ~0x20);
	
	w_count = 1;
	for (i = 0; i < len; i++)
	{
		WriteTW88(REG307, *str++);
		w_count++;
		if (w_count == 6/*8*/)
		{	 //NOTE
			delay1ms(1);
			w_count=0;
		}
	}
}

#if 0
/**
* Write data on ByPassFIFO mode.
*
* example
*	FOsdRamSetFifo(OFF,0)
*	FOsdRamSetAddrAttr(,);
*	FOsdRamWriteByteStrBypassFifo("   ",len);
*	FOsdRamSetFifo(ON,1)
*/
void FOsdRamWriteByteStrBypassFifo(BYTE *str, BYTE len)
{
	BYTE i;
	BYTE w_count;

	WriteTW88Page(PAGE3_FOSD);
	WriteTW88(REG304, ReadTW88(REG304)& ~0x20);
	
	for(i=0; i < len; i++) {
		WriteTW88(REG307, *str++);
	}
}
#endif

//=============================================================================
// FOsdWin
//=============================================================================
/**
* enable FOSD Window
*
*	R310[7],R320[7],,,
*/
void FOsdWinEnable(BYTE winno, BYTE en)
{
	XDATA WORD index;
	XDATA BYTE dat;

	index = FOSDWinBase[winno] + FONTWIN_ENABLE;

	WriteTW88Page(PAGE3_FOSD);
	dat = ReadTW88(index);
	if (en)
		dat |= 0x80;
	else
		dat &= 0x7F;
	WriteTW88(index, dat);
}

/**
* toggle FOSD Window enable bit
*
* @see FOsdWinEnable
*/
void FOsdWinToggleEnable(BYTE winno)
{
	XDATA WORD index;
	XDATA BYTE dat;

	index = FOSDWinBase[winno] + FONTWIN_ENABLE;

	WriteTW88Page(PAGE3_FOSD);
	dat = ReadTW88(index);
	if (dat & 0x80)
		WriteTW88(index, dat & 0x7F); 	//ON->OFF
	else
		WriteTW88(index, dat | 0x80);	//OFF->ON
}

/**
* turn off all FontOSD Window
*/
void FOsdWinOffAll(void)
{
	BYTE i;

	for (i=0; i< FOSD_MAX_OSDWIN; i++)
		FOsdWinEnable(i, OFF);
}

/**
* enable MultiColor attribute
*
* pls, DO NOT TURN OFF
*
*	R310[6],R320[6],,,
*/
void FOsdWinMulticolor(BYTE winno, BYTE en)
{
	XDATA WORD index;
	XDATA BYTE dat;

	index = FOSDWinBase[winno] + FONTWIN_ENABLE;

	WriteTW88Page(PAGE3_FOSD);
	dat = ReadTW88(index);
	if ( en )
		dat |= 0x40;
	else
		dat &= 0xBF;	 
	WriteTW88(index, dat);
}

//R310[5],R320[5],,,
#if 0
void FOsdWinEnableVerticalExtensior(BYTE fOn)
{ .. }
#endif

/**
* set Horizontal Vertical Zoom
*
*	R310[3:2],R320[3:2],,,
*	R310[1:0],R320[1:0],,,
*/
void FOsdWinZoom(BYTE winno, BYTE zoomH, BYTE zoomV)
{
	XDATA WORD index;
	XDATA BYTE temp;

	index = FOSDWinBase[winno];

	WriteTW88Page(PAGE3_FOSD );

	temp = (zoomH << 2) + zoomV;
	temp += (ReadTW88(index ) & 0xf0);
	WriteTW88(index, temp );
}

//------------------------------------------------------------------------
//		void FOsdWinAlphaPixel(BYTE winno, BYTE color, BYTE alpha)
//------------------------------------------------------------------------
/**
* set FOSD Window Alpha color and Amount
*
*	r311[3:0] r321[3:0] & r352[]
*
* @todo divide it with AlphaColor & AlphaAmount
*/
void FOsdWinAlphaPixel(BYTE winno, BYTE lut, BYTE alpha)
{
	WORD index = FOSDWinBase[winno] + FONT_ALPHA;

	WriteTW88Page(PAGE3_FOSD);
	WriteTW88(REG352, lut);				// first, select color index
	WriteTW88(index, alpha);			// second, write alpha value
}

//array { index , alpha}
#if 0
//we have a bank issue(we can not use the call by reference on CodeSegment).
//so, pls use FontOsdBpp3Alpha_setLutOffset() and FOsdWinSetBpp3Alpha()
void FOsdWinAlphaPixelArray(BYTE winno, BYTE *array, BYTE size)
{
	WORD index = FOSDWinBase[winno] + FONT_ALPHA;
	BYTE i;

	WriteTW88Page(PAGE3_FOSD );
	for(i=0; i < size; i++) {
		WriteTW88(REG_FOSD_ALPHA_SEL, array[i*2]); 	// first, select color index
		WriteTW88(index,   array[i*2+1] );			// second, write alpha value
	}
}
#endif

/**
* set alpah value for 3BPP Alpha font
*
* extern
* BPP3_alpha_lut_offset[].
* BPP3_alpha_value[].
*	
* @param base_lut: start position of LUT
*/
void FOsdWinSetBpp3Alpha(BYTE winno, BYTE base_lut)
{
	WORD index = FOSDWinBase[winno] + FONT_ALPHA;
	BYTE i;

	WriteTW88Page(PAGE3_FOSD);
	for (i=0; i < 8; i++)
	{
		WriteTW88(REG_FOSD_ALPHA_SEL, base_lut+BPP3_alpha_lut_offset[i]);	// first, select color index
		WriteTW88(index, BPP3_alpha_value[i]);								// second, write alpha value
	}
}

/**
* set Screen Position
*
*	r312[6:4]r313[7:0]	H-Start
*	r312[1:0]r314[7:0]	V-Start
*/	
void FOsdWinScreenXY(BYTE winno, WORD x, WORD y)
{
	XDATA WORD index;
	XDATA BYTE temp;

	index = FOSDWinBase[winno];

	WriteTW88Page(PAGE3_FOSD);

	temp = x >> 8;
	temp <<= 4;
	temp += (y >> 8);
	WriteTW88(index+2, temp);			// upper bit for position x, y
	WriteTW88(index+3, x);				// position x
	WriteTW88(index+4, y);				// position y

	FontOsdInfo.win[winno].sx = x;
	FontOsdInfo.win[winno].sy = y;
}

/**
* set screen width and height
*
*	r315[5:0]			V-Height
*	r316[5:0]			H-width
*/
void FOsdWinScreenWH(BYTE winno, BYTE w, BYTE h)
{
	XDATA WORD index;

	index = FOSDWinBase[winno];

	WriteTW88Page(PAGE3_FOSD);
	WriteTW88(index+5, h);
	WriteTW88(index+6, w);

	FontOsdInfo.win[winno].column = w;
	FontOsdInfo.win[winno].line = h;
}

//-----------------------------------------------------------------------------
//		WORD FOsdWinGetX(BYTE winno)	  		: winno 1~4
//		void FOsdWinSetX(BYTE winno, WORD x)	: winno 1~4
//		WORD FOsdWinGetY(BYTE winno)	  		: winno 1~4
//		void FOsdWinSetY(BYTE winno, WORD y)	: winno 1~4
//		void FOsdWinSetW(BYTE winno, WORD w)	: winno 1~4
//-----------------------------------------------------------------------------
#ifdef SUPPORT_FOSD_MENU
/**
* get window x position
*/
WORD FOsdWinGetX(BYTE winno)
{
	WORD Pos;

	WriteTW88Page(PAGE3_FOSD);
	
	Pos = ReadTW88(FOSDWinBase[winno]+2)&0x70;
	Pos <<= 4;
	Pos += ReadTW88(FOSDWinBase[winno]+3);

	return (Pos);
}

#if 0
void FOsdWinSetX(BYTE winno, WORD x)
{
	WORD index;

	//dPrintf("\nFOsdWinSetX( %bd, %d )", winno, x );

	WriteTW88Page(PAGE3_FOSD );
	index = FOSDWinBase[winno];
	WriteTW88(index+2, (ReadTW88(index+2)&0x8F) | ((x>>4)&0x70));
	WriteTW88(index+3, x);
}
#endif

/**
* get window Y position
*/
WORD FOsdWinGetY(BYTE winno)
{
	WORD index;
	WORD Pos;

	WriteTW88Page(PAGE3_FOSD);
	
	index = FOSDWinBase[winno];
	Pos = ReadTW88(index+2)&0x03;
	Pos <<= 8;
	Pos += ReadTW88(index+4);

	return (Pos);
}

/**
* set window Y position
*/
void FOsdWinSetY(BYTE winno, WORD y)
{
	WORD index;

	//dPrintf("\nFOsdWinSetY( %bd, %d )", winno, y );

	WriteTW88Page(PAGE3_FOSD);
	index = FOSDWinBase[winno];
	WriteTW88(index+2, (ReadTW88(index+2)&0xFC)|(y>>8));
	WriteTW88(index+4, y);
}

/**
* set window width
*/
void FOsdWinSetW(BYTE winno, WORD w)
{
	WORD index;

	//dPrintf("\nFOsdWinSetW( %bd, %d )", winno, w );

	WriteTW88Page(PAGE3_FOSD);
	index = FOSDWinBase[winno];  //????
	WriteTW88(index+6, w);
}
#endif

//r317[3:0]
//void FOsdWinSetBorderColor(BYTe idx)

//r318[7]
//void FOsdWinEnableBorder(fOn)

//r318[4:0]
//void FOsdWinSetBorderWidth(BYTE width)

//r319[6:0]
//void FOsdWinSetHBorder(BYTE width)

//r31A[6:0]
//void FOsdWinSetVBorder(BYTE width)

//r31B[7] 3D effect enable
//r31B[6] 3D top effect
//r31B[5] 3D effect level
//			NOTE: 3D need a Border.
//r31B[4] Select BorderOrShadow. 1=Shadow
//r31B[3:0]	Shadow Color

//note:winno 0~3.
#ifdef SUPPORT_FOSD_MENU
/**
* set 3DControl
*/
void FOsdWinSet3DControl(BYTE win, BYTE value)
{
	WriteTW88Page(PAGE3_FOSD);
	WriteTW88(FOSDWinBase[win]+0x0B,value); 	
}
#endif

//r31C[7]	Enable Shadow
//r31C[4:0]	Shadow Width

//r31C[6]r31D[7:4]		Char H-Space
//r31C[5]r31D[3:0]		Char V-Space

//r31E[7:4]	H/V Border Background Color

//r31E[3:0]	char border/shadow color 

/**
* set OsdRam Start Address
*
*	R3X7[4],R3XF[7:0], WIN1:R317, WIN2:R327, WIN3:R337 WIN4:R347
*/
void FOsdWinSetOsdRamStartAddr(BYTE winno, WORD addr)
{
	WORD index;

	WriteTW88Page(PAGE3_FOSD);
	index = FOSDWinBase[winno];

	if (addr >= 0x100)
		WriteTW88(index+0x07, ReadTW88(index+0x07) |  0x10);
	else
		WriteTW88(index+0x07, ReadTW88(index+0x07) & ~0x10);
	WriteTW88(index+0x0F, (BYTE)addr);

	FontOsdInfo.win[winno].osdram = addr;
}

//=============================================================================
// HAL END
//=============================================================================

//=======================================
// Initialize Functions
//=======================================

/**
* Init FontOSD
*/
void FontOsdInit(void)
{
	BYTE winno;
	BYTE columns = 40;
	BYTE lines = 1;

#ifdef MODEL_TW8835
	FOsdOnOff(OFF, 0);	
#else
	//TW8836 needs a FOSD ON when it download font.
	FOsdOnOff(ON, 0);	
#endif

	//download font set & init Multi-BPP location
	InitFontRamByNum(FONT_NUM_CONGRAY16X28, 0);	// set consolas & graynum and calculate 3BPP alpha

	//init all fontosd windows attributes
	for (winno = 0; winno < FOSD_MAX_OSDWIN; winno++)
	{
		FOsdWinInit(winno);

		FontOsdInfo.win[winno].bpp1_attr = 0x1A;					//BG|FG
		FOsdWinAlphaPixel(winno, FOSD_COLOR_IDX_BLANK, 0x0F);		//BG color alpha

		FOsdWinScreenXY(winno, 0, 30*winno);
		FOsdWinScreenWH(winno, columns, 1/*lines*/);		//0x20, 0x10);		//max 512 = 32x16
 		FOsdWinZoom(winno, 0, 0);				
		FOsdWinSetOsdRamStartAddr(winno, columns*winno /*text_info->osdram*/);
	}

	//clear all OsdRam.
	FOsdRamClearAll(0x020, 0x0F);
}

/* calculated example value.
[01:0] 
[07:43] 
[06:90] 
[04:133] 
[03:180] 
[05:223] 
[02:270] 
[00:313]
*/
//code BYTE calculated_3BPP_alpha_table[8] = { 1,7,6,4,3,5,2,0 };

//read value from FontEditor and place it here.
//code WORD consolas22_16x26_3BPP[8] = {
//	0xFFFF,0x0000,0xDEDB,0x9492,0x6B6D,0xB5B6,0x4A49,0x2124
//}; 

/**
* init 3BPP Alpha table
*
* after download font.
* prepare alpha table for 3BPP
*/
void FOsdInitBpp3AlphaTable(BYTE fCalculate)
{
	BYTE i,j,k;
	WORD Y;
	WORD value;
	WORD alpha_table_Y[8];

	if (fCalculate == 0)
	{
		//use the calculated table and save a time.
		for (i=0; i < 8; i++)
			BPP3_alpha_lut_offset[i]=FOsdDefPaletteBpp3Alpha[i];	
	}
	else
	{
		//prepare alpha
		for (i=0; i < 8; i++)
		{
			value= FOsdDefPaletteBpp3[i];
			//B =	value >> 11;
			//G = (value & 0x07E0) >> 5;
			//R = value & 0x001F;
			//Y = B;
			//Y += (G * 3); //*6 / 2
			//Y += (R * 3);
			j = value >> 11;				//B
			Y = j;
			j = (value & 0x07E0) >> 5;		//G
			Y += (j * 3); 					//*6 / 2
			j = value & 0x001F;				//R
			Y += (j * 3);
	
			//dPrintf("\ni:%02bx BGR:%04X Y:%d",i,FOsdDefPaletteBpp3[i],Y);
	
			if (i)
			{
				BPP3_alpha_lut_offset[i]=i;
				alpha_table_Y[i]=Y;
	
				for (j=0; j < i; j++)
				{
					if (Y < alpha_table_Y[j])
					{
						k    =BPP3_alpha_lut_offset[i];
						value=alpha_table_Y[i];
	
						BPP3_alpha_lut_offset[i] = BPP3_alpha_lut_offset[j];
						alpha_table_Y[i] = alpha_table_Y[j];
	
						BPP3_alpha_lut_offset[j] = k;								
						alpha_table_Y[j] = value;
						Y = value;
					}
				}
	
				//dPrintf("\n  ");
				//for(j=0; j <= i; j++) {
				//	dPrintf(" %bx:%d",BPP3_alpha_lut_offset[j], alpha_table_Y[j]);		
				//}
			}
			else
			{
				BPP3_alpha_lut_offset[0]=0;
				alpha_table_Y[0]=Y;
			}
		}
		//result
		//dPuts("\n3BPP Alpha");
		//for(i=0; i < 8; i++) {
		//	dPrintf(" [%bx:%d]",BPP3_alpha_lut_offset[i], alpha_table_Y[i]);
		//}
	}
}

//=======================================
// Windows Functions
//=======================================

/**
* init FOSD Window register
*
* @param winno	from 0 to 3. TW8836 use 0 to 7
* NOTE:	OsdRamAddr	from 0 to 511
*/
void FOsdWinInit(BYTE winno)
{
	WORD index;

	WriteTW88Page(PAGE3_FOSD);

	index = FOSDWinBase[winno];
	//init all fontosd attributes
	WriteTW88(index, (ReadTW88(index) & 0x7F) | 0x40);		//FOsdWinEnable(winno, OFF) & Enable Multi-Color

	WriteTW88(index+0x07, 0x00);
	WriteTW88(index+0x08, 0x00);
	WriteTW88(index+0x09, 0x00);
	WriteTW88(index+0x0A, 0x00);
	WriteTW88(index+0x0B, 0x00);
	WriteTW88(index+0x0C, 0x00);
	WriteTW88(index+0x0D, 0x00);
	WriteTW88(index+0x0E, 0x00);
	WriteTW88(index+0x0F, 0x00);

	//clear alpha
	FOsdWinAlphaPixel(winno, FOSD_COLOR_IDX_BLANK, 0);
}

//=======================================
// Palette Functions
//=======================================
/**
* set default Palette color
*/
void FOsdSetDefPaletteColor(BYTE mode)
{
	if (mode==0)
	{
		FOsdSetPaletteColorArray(0,									FOsdHwDefPaletteBpp1,16, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP2_ALPHA_A_START,	FOsdDefPaletteBpp2,	 4, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP2_ALPHA_B_START,	FOsdDefPaletteBpp2,	 4, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP3_ALPHA_G_START,	FOsdDefPaletteBpp3,	 8, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP2_START-4,			FOsdDefPaletteBpp2,	 4, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP2_START,			FOsdDefPaletteBpp2,	 4, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP3_START,			FOsdDefPaletteBpp3,	 8, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP4_START,			FOsdDefPaletteBpp4,	16, 0);
	}
	else if (mode==1)
	{
		FOsdSetPaletteColorArray(0,									FOsdSwDefPaletteBpp1,8, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP3_ALPHA_R_START,	FOsdDefPaletteBpp3,8, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP3_ALPHA_B_START,	FOsdDefPaletteBpp3,8, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP3_ALPHA_G_START,	FOsdDefPaletteBpp3,8, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP2_START-4,			FOsdDefPaletteBpp2,4, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP2_START,			FOsdDefPaletteBpp2,4, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP3_START,			FOsdDefPaletteBpp3,8, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP4_START,			FOsdDefPaletteBpp4,16, 0);
	}
	else if (mode==2)
	{
		FOsdSetPaletteColorArray(0,									FOsdSwDefPaletteBpp1,16, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP2_ALPHA_A_START,	FOsdDefPaletteBpp2,4, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP2_ALPHA_B_START,	FOsdDefPaletteBpp2,4, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP3_ALPHA_G_START,	FOsdDefPaletteBpp3,8, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP2_START-4,			FOsdDefPaletteBpp2,4, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP2_START,			FOsdDefPaletteBpp2,4, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP3_START,			FOsdDefPaletteBpp3,8, 0);
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP4_START,			FOsdDefPaletteBpp4,16, 0);
	}
}

//=======================================
// FontRam Functions
//=======================================

/**
* download font set
*
* the dest font index starts from 0.
*
* width: 12 or 16
* height: 2~32
*
* note: use PCLK.
*	turns off FontRAM FIFO and uses PCLK.
*
* @param dest: start addr on FontRam.(0~10*1024) ..Need to Verify
* @param src_loc: location on SpiFlash.
* @param size
*/
void FOsdDownloadFontByDMA(WORD dest_loc, DWORD src_loc, WORD size)
{
#ifdef DEBUG_FOSD
	dPrintf("\nFOsdDownloadFontByDMA(%x,%lx,%x)",dest_loc, src_loc, size);
#endif

	//save clock mode & select PCLK
	WaitVBlank(1);	

#if defined(TW8836_CHIPDEBUG)
	//BK120615. for test clock source
#else
	McuSpiClkToPclk(CLKPLL_DIV_2P0);				//with divider 1=1.5(72MHz). try 2
#endif

	FOsdFontSetFifo(OFF);							//Turn Off FontRam FIFO.
	FOsdSetAccessMode(FOSD_ACCESS_FONTRAM);			//FontRam Access

	SpiFlashDmaRead(DMA_DEST_FONTRAM, dest_loc, src_loc, size);

	WaitVBlank(1);	
	FOsdFontSetFifo(ON);						//Turn On FontRam FIFO. return to default
	FOsdSetAccessMode(FOSD_ACCESS_OSDRAM);		//OsdRam Access Mode. return to default

	//restore clock mode
#if defined(TW8836_CHIPDEBUG)
	//BK120615. for test clock source
#else
	McuSpiClkRestore();
#endif
}

#ifdef SUPPORT_UDFONT
//use FIFO. We don't need to use PCLK domain.
//param
//	unit_size: if 12x18 font, it is a 27 = 12x18/8
void FOsdDownloadFontBySerial(WORD dest_font_index, BYTE *dat, BYTE unit_size, BYTE unit_num)
{
	BYTE value;
	BYTE w_cnt;
	BYTE i, j;
	WORD addr;

	//assume FontRam FIFO ON.
	FOsdSetAccessMode(FOSD_ACCESS_FONTRAM);		//Select FontRam Access

	addr = dest_font_index;
	w_cnt = 0;
	
	WriteTW88Page(PAGE3_FOSD);
	
	for (i = 0; i < unit_num; i++)
	{
		if (w_cnt >= (8-2))
		{
			delay1ms(1);
			w_cnt = 0;
		}

		value = ReadTW88(REG304);
		if (addr & 0x100)
			value |= 0x20;		//UPPER256
		else
			value &= ~0x20;
		WriteTW88(REG304, value);
		
		WriteTW88(REG309, (BYTE)addr); 		//Font Addr
		addr++;
		w_cnt += 2;

		for (j = 0; j < unit_size; j++)
		{
			WriteTW88(REG30A, *dat++);
			
			w_cnt++;
			if (w_cnt >= 8)
			{
				delay1ms(1);
				w_cnt = 0;
			}
		}
	}

	FOsdSetAccessMode(FOSD_ACCESS_OSDRAM);		//Select OSDRam Access. restore to default
}
#endif

#ifdef SUPPORT_UDFONT
void FOsdDownloadUDFontBySerial(void)
{
	FOsdDownloadFontBySerial(0, &RAMFONTDATA[0][0], 27, 0x80);
}
#endif

/**
* replace FontRam
*/
void ReplaceFontRam(WORD dest_font_index, FONT_SPI_INFO_t *font, char *sName)
{
	WORD addr;
	
	dPrintf("\nReplaceFontRam(%x,,%s",dest_font_index,sName);
	addr = dest_font_index *(font->width >> 2) * (font->height >> 1);
	FOsdDownloadFontByDMA(addr, font->loc, font->size);
}

/**
*	download fontset and adjust Multi-font start address
*/
void InitFontRam(WORD dest_font_index, FONT_SPI_INFO_t *font, char *sName)
{
	BYTE value;
	WORD addr;

#ifdef DEBUG_FOSD
	dPrintf("\nInitFontRam(%x,,%s)",dest_font_index,sName);
#endif

	FOsdSetFontWidthHeight(font->width, font->height);

	//download font
	addr = dest_font_index *(font->width >> 2) * (font->height >> 1);
	FOsdDownloadFontByDMA(addr, font->loc, font->size);

	//assign Multi-Color start address.
    WriteTW88Page(PAGE3_FOSD);
	value = ReadTW88(REG305) & 0xF1;
	if ((font->bpp2 +dest_font_index) & 0x100)
		value |= 0x02;	// 2bit-multi-font start. 8th address
	if ((font->bpp3 +dest_font_index) & 0x100)
		value |= 0x04;	// 3bit-multi-font start. 8th address
	if ((font->bpp4 +dest_font_index) & 0x100)
		value |= 0x08;	// 4bit-multi-font start. 8th address
	WriteTW88(REG305, value);	
	WriteTW88(REG30B, (BYTE)(font->bpp2 + dest_font_index)); 			// 2bit-multi-font start
	WriteTW88(REG_FOSD_MADD3, (BYTE)(font->bpp3 +dest_font_index)); 	// 3bit-multi-font start
	WriteTW88(REG_FOSD_MADD4, (BYTE)(font->bpp4 +dest_font_index)); 	// 4bit-multi-font start

	//link palette
	FOsdSetPaletteColorArray(0,	FOsdHwDefPaletteBpp1,16, 0);
	if (font->palette_bpp2 != NULL)
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP2_START,(WORD *)font->palette_bpp2,4, 0);
	if(font->palette_bpp3 != NULL)
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP3_START,(WORD *)font->palette_bpp3,8, 0);
	if(font->palette_bpp4 != NULL)
		FOsdSetPaletteColorArray(FOSD_LUT_MAP_BPP4_START,(WORD *)font->palette_bpp4,16, 0);

	//..USE_FONTOSDINFO
	if (sName != NULL)
		TWstrcpy(FontOsdInfo.font.name, sName);
	else
		TWstrcpy(FontOsdInfo.font.name, "unknown");
	FontOsdInfo.font.w = font->width;
	FontOsdInfo.font.h = font->height;

	FontOsdInfo.font.bpp2 = font->bpp2 +dest_font_index;
	FontOsdInfo.font.bpp3 = font->bpp3 +dest_font_index;
	FontOsdInfo.font.bpp4 = font->bpp4 +dest_font_index;
	FontOsdInfo.font.max  = font->max  +dest_font_index;

	DumpFontInfo();
}

/**
* test InitFontRam routine
*/
void TestInitFontRam(WORD start)
{
	InitFontRam(start, &consolas16x26_graynum,"gray");	   	//set default font set
}

//=====================
// OsdRam function
//=====================

//========================
// FOsdMsg Functions
//========================

//indicate which font.

//BYTE FOsdMsg_win;
//BYTE FOsdMsg_sx;
//BYTE FOsdMsg_sy;

//global

//110727. Use FIFO with delay
/**
* copy message to OsdRam
*/
void FOsdCopyMsgBuff2Osdram(BYTE fOn)
{
//	BYTE i;
	BYTE len;
	BYTE columns=40;
	BYTE attr;
	WORD osdram;
//	DECLARE_LOCAL_page

#ifdef DEBUG_FOSD
	dPrintf("\nFOsdCopyMsgBuff2Osdram(%bx)",fOn);
#endif

	len = TWstrlen(FOsdMsgBuff);
//	dPrintf("\nFOsdCopyMsgBuff2Osdram len:%bd ",len);
//	for(i=0; i < len; i++) {
//		attr=FOsdMsgBuff[i];
//		if(attr >= 0x20 && attr <= 0x7F)
//			dPrintf("%c",attr);
//		else
//		dPrintf(" %02bx ",attr);
//	}
	if (len==0)
		return;
	dPrintf(" len:%bx",len);

	columns = FontOsdInfo.win[FOSD_MSG_WIN].column;
	attr = 	  FontOsdInfo.win[FOSD_MSG_WIN].bpp1_attr;
	osdram =  FontOsdInfo.win[FOSD_MSG_WIN].osdram;

//	dPrintf(" col:%bd attr:%02bx osdram:%x ",columns, attr, osdram);

	//cut end of line
	FOsdWinScreenWH(FOSD_MSG_WIN, len,1);

	//print string
	WaitVBlank(1); //pls. finish within one vblank.
	FOsdRamSetAddrAttr(osdram, attr);	//osdram address, fg|bg, mode:auto
	FOsdRamWriteByteStr(FOsdMsgBuff,len);

	FOsdWinEnable(FOSD_MSG_WIN,fOn);	//WINn enable
}

/**
* print the indexed message
*/
void FOsdIndexMsgPrint(BYTE index)
{
	if (MenuGetLevel())
		return;

	switch (index)
	{
	case FOSD_STR0_GOOD:	
		FOsdSetInputMainString2FOsdMsgBuff();	//GetInputMainString(FOsdMsgBuff);
		TWstrcat(FOsdMsgBuff,"..Good!");		
		break;
	case FOSD_STR1_TW8835:
		TWstrcpy(FOsdMsgBuff,"Intersil TW8835.");
		break;
	case FOSD_STR2_NOSIGNAL:
		FOsdSetInputMainString2FOsdMsgBuff();	//GetInputMainString(FOsdMsgBuff);
		TWstrcat(FOsdMsgBuff," No Signal");
		break;
	case FOSD_STR3_OUTRANGE:
		FOsdSetInputMainString2FOsdMsgBuff();	//GetInputMainString(FOsdMsgBuff);
		TWstrcat(FOsdMsgBuff," Out of Range");		
		break;
	case FOSD_STR4_INIT_EEPROM:
		TWstrcpy(FOsdMsgBuff,"Initialize EEPROM");
		break;
	case FOSD_STR5_INPUTMAIN:
		FOsdSetInputMainString2FOsdMsgBuff();	//GetInputMainString(FOsdMsgBuff);
		break;
	default:
		TWstrcpy(FOsdMsgBuff,"Unknown");
		break;	
	}
	FOsdCopyMsgBuff2Osdram(ON);
}

#define TEST_OSDRAM_ADDR	120 
/**
* dump the Palette table
*/
void FOsdDumpPalette(BYTE winno)
{
	BYTE palette_group;

	WaitVBlank(1);
	FOsdWinEnable(winno,OFF);	//winno disable

	FOsdWinScreenXY(winno, 0x70,0);	//start 0,0  16 colums 1 line
	FOsdWinScreenWH(winno, 16, 2);	//start 0,0  16 colums 1 line
 	FOsdWinZoom(winno, 1, 0);			//zoom 1,1
	FOsdWinAlphaPixel(winno, 1, 0x00);	//BG color alpha => removed
	FOsdWinMulticolor(winno, ON);
	FOsdWinSetOsdRamStartAddr(winno,TEST_OSDRAM_ADDR);

	for (palette_group=0; palette_group < 16; palette_group++)
	{
		//first line
		FOsdRamSetAddrAttr(TEST_OSDRAM_ADDR+palette_group, palette_group);
		FOsdRamSetData(CH_2BPP_BAR);

		//second line
		FOsdRamSetAddrAttr(120+palette_group+16, ((palette_group) << 4) | palette_group+1);
		WriteTW88(REG304,ReadTW88(REG304) & ~0x20);
		if(palette_group < 0xA)	WriteTW88(REG307, '0'+palette_group);
		else					WriteTW88(REG307, 'A'+palette_group-0x0A);

		delay1ms(1);
	}

	FOsdWinEnable(winno,ON);		//winno enable
}

//=============================================================================
// 
//=============================================================================
/**
* prepare InputMain String
*/
char *GetInputMainString(char *p_itoa_buff)
{
	BYTE InputMainIndex;
	BYTE len;

	InputMainIndex = GetInputMain();

	switch (InputMainIndex)
	{
	case INPUT_CVBS:	TWstrcpy(p_itoa_buff,"CVBS");		break;
	case INPUT_SVIDEO:	TWstrcpy(p_itoa_buff,"SVIDEO");		break;
	case INPUT_COMP:	TWstrcpy(p_itoa_buff,"Component");	break;
	case INPUT_PC:		TWstrcpy(p_itoa_buff,"PC");			break;
	case INPUT_DVI:		TWstrcpy(p_itoa_buff,"DVI");		break;
	case INPUT_HDMIPC:	TWstrcpy(p_itoa_buff,"HDMI_PC");	break;
	case INPUT_HDMITV:	TWstrcpy(p_itoa_buff,"HDMI_TV");	break;
	case INPUT_BT656:	TWstrcpy(p_itoa_buff,"BT656");		break;
	case INPUT_LVDS:	TWstrcpy(p_itoa_buff,"LVDS");		break;
	default:			TWstrcpy(p_itoa_buff,"UNKNOWN");	break;
	}
	len = TWstrlen(p_itoa_buff);
	
	return p_itoa_buff+len;
}

/**
* set InputMain String
*
* for Bank issue
*/
void FOsdSetInputMainString2FOsdMsgBuff(void)
{
	GetInputMainString(FOsdMsgBuff);
}

//=============================
// comes from FOsdBasic.c
//=============================
//===============================================================================
//  void WriteStringToAddr(BYTE addr, BYTE *str, BYTE cnt)
//===============================================================================


//FOsdPuts()
//FOsdPutch

//BKTODO:addr need 9bit
#ifdef SUPPORT_FOSD_MENU
/**
* write string to Address
*/
void WriteStringToAddr(WORD addr, BYTE *str, BYTE cnt)
{
	BYTE i;
	BYTE w_cnt;
	WORD dat,start;

	//Printf("\nWriteStringToAddr(%x,,%bx)\n",addr,cnt);
	w_cnt = 0;
	start = 0;

	//default::FOsdSetAccessMode(FOSD_ACCESS_OSDRAM);
	FOsdRamSetWriteMode(FOSD_OSDRAM_WRITE_NORMAL);

	WriteTW88Page(PAGE3_FOSD );
	w_cnt = 2;

	for ( i=0; i<cnt; i++ ) {

		if(*str >= 0xF0) {
			//we have a escape code
			if(*str == FONT_RAM) 		start = FONT_RAM_START;
			else if(*str == FONT_ROM)	start = 0;
			else if(*str == FONT_2BPP)	start = FONT_2BPP_START;
			else if(*str == FONT_3BPP)	start = FONT_3BPP_START;
			else if(*str == FONT_4BPP)	start = FONT_4BPP_START;
			else						start = 0;
			str++;
			continue;
		}
		dat = start + *str++;
		if(w_cnt >=3) {
			delay1ms(1);
			w_cnt = 0;
		}
		//Printf("A:%X D:%X ",addr,dat);

		//addr
		FOsdRamSetAddress(addr);
		//data
		FOsdRamSetData(dat);
		//attr
		if(start==FONT_2BPP_START)
			WriteTW88(REG308, FOSD_LUT_MAP_BPP2_START >>2);
		else if(start==FONT_3BPP_START)
			WriteTW88(REG308, FOSD_LUT_MAP_BPP3_START >>2);
		else if(start==FONT_4BPP_START)
			WriteTW88(REG308, FOSD_LUT_MAP_BPP4_START >>2);
		addr++;

		w_cnt += 5;
	}
}
#endif

//===============================================================================
//  void FOsdRamMemsetAttr(BYTE addr, BYTE color, BYTE cnt)
//===============================================================================
#ifdef SUPPORT_FOSD_MENU
/**
* write only attr.
*
* assume:
* 	- FOSD_ACCESS_OSDRAM.
*	- FIFO on
* NOTE:FOsdRamFifo has 8 buffer.
*/
void FOsdRamMemsetAttr(WORD addr, BYTE attr, BYTE len)
{
	BYTE i;
	BYTE w_count;

	//Printf("\nFOsdRamMemsetAttr(%bx,%bx,%bx)",addr,color,cnt);
	FOsdRamSetWriteMode(FOSD_OSDRAM_WRITE_AUTO);
	FOsdRamSetAddress(addr);
	delay1ms(1);

	w_count = 0;
	for (i = 0; i < len; i++)
	{
		WriteTW88(REG308, attr);
		w_count++;
		if (w_count == 6/*8*/)
		{	//NOTE
			delay1ms(1);
			w_count = 0;
		}
	}
}

/**
* write only data.
*
* assume:
*	- FOSD_ACCESS_OSDRAM.
*	- FIFO on
*/
void FOsdRamMemsetData(WORD addr, WORD dat, BYTE len)
{
	BYTE i;
	BYTE w_count;

	FOsdRamSetWriteMode(FOSD_OSDRAM_WRITE_AUTO);
	FOsdRamSetAddress(addr);
	delay1ms(1);
	
	WriteTW88Page(PAGE3_FOSD);
	if (dat < 0x100)
		WriteTW88(REG304, ReadTW88(REG304)& ~0x20);			// clear Upper 256 it
	else
		WriteTW88(REG304, ReadTW88(REG304)| 0x20);			// upper 256
	w_count = 1;
	for (i=0; i < len; i++)
	{
		WriteTW88(REG307, (BYTE)dat);
		w_count++;
		if (w_count==6/*8*/)
		{	//NOTE
			delay1ms(1);
			w_count=0;
		}
	}
}

/**
* memset for OsdRam
*/
void FOsdRamMemset(WORD addr, WORD dat, BYTE attr, BYTE len)
{
	BYTE i;
	BYTE w_count;

	FOsdRamSetAddrAttr(addr, attr);
	delay1ms(1);
	
	WriteTW88Page(PAGE3_FOSD);
	
	if (dat < 0x100)
		WriteTW88(REG304, ReadTW88(REG304)& ~0x20);			// clear Upper 256 it
	else
		WriteTW88(REG304, ReadTW88(REG304)| 0x20);			// upper 256
	
	w_count = 1;

	for (i=0; i < len; i++)
	{
		WriteTW88(REG307, (BYTE)dat);

		w_count++;
		if (w_count==6/*8*/)
		{	//NOTE
			delay1ms(1);
			w_count=0;
		}
	}
}
#endif

/**
* clear all OsdRam data
*
* NOTE:I will use FOsdRamClearAllData.
*	ex:
*	FOsdRamSetAttr(attr);
*	FOsdRamClearAllData();
*/			
void FOsdRamClearAll(WORD dat, BYTE attr)
{
	WORD i;

	FOsdRamSetFifo(OFF, 0);

	FOsdRamSetAddrAttr(0, attr);

	WriteTW88Page(PAGE3_FOSD);

	if (dat & 0x100)
		WriteTW88(REG304, ReadTW88(REG304) |  0x20); 
	else
		WriteTW88(REG304, ReadTW88(REG304) & ~0x20);

	for (i = 0; i < FOSD_MAX_OSDRAM_SIZE; i++)
	{
		WriteTW88(REG307, dat);
	}

	FOsdRamSetFifo(ON, 1);		//with vdelay 1
}

//=============================================================================
//			Init Menu Window Data initialize 
//=============================================================================
#ifdef SUPPORT_FOSD_MENU
// counter
//		reg win
//		data
// 0 
/**
* init FOSD Menu Window
*/
void InitFOsdMenuWindow(BYTE *ptr)
{
	WORD index;
	BYTE data_cnt;
		
	WriteTW88Page(PAGE3_FOSD);

	//debug message
#if 0
	data_cnt = *ptr;	  //read counter
    while( data_cnt ) {
		ptr++;
		dPrintf("\nOSD data down counts: %bd, WindowNo: %bd", data_cnt, *ptr);
		index = FOSDWinBase[*ptr];				// start register address
		ptr++;
		do {
			dPrintf("\nOSD data down: 0x%03x - 0x%02bx", index, *ptr);
			index++; 
			ptr++;
			data_cnt--;
		} while(data_cnt);
		data_cnt = *ptr; 	//read counter
    };
#endif


	data_cnt = *ptr;	  //read counter
    while (data_cnt)
	{
		ptr++;
		index = FOSDWinBase[*ptr];				// start register address
		ptr++;
		do
		{
			WriteTW88(index, *ptr );
			index++; 
			ptr++;
			data_cnt--;
		} while(data_cnt);
		data_cnt = *ptr; 	//read counter
    };
}
#endif

//=============================================================================
//				   void FontLUT( void )
//=============================================================================
#ifdef SUPPORT_FOSD_MENU
//Bank issue  
void FOsdDefaultLUT( void )
{
	FOsdSetPaletteColorArray(0, FOsdHwDefPaletteBpp1, 16, 1); //with 1 vdelay
}
#endif

//=============================================================================
//=============================================================================
/**
* init FontRam by number
*
* bank issue
*/
void InitFontRamByNum(BYTE FontMode, WORD start)
{
	switch (FontMode)
	{
	case 0:
		InitFontRam(start, &default_font, "def");
		//FOsdSetDefPaletteColor(0);
		break;
	case 1:
		//InitFontRam(0, &SPI_FONT1_TEMP, "FONT1");
		//FOsdSetPaletteColorArray(16, SPI_FONT1, 40 , 1);
		InitFontRam(start, &consolas16x26_606C90, "con");
		break;
	case 2:
		//InitFontRam(0, &SPI_LOGO_TEMP, "LOGO");
		InitFontRam(start, &consolas16x26_graynum, "con+gray");
		FOsdInitBpp3AlphaTable(0);
		break;
	case 3:
		InitFontRam(start, &consolas16x26_606C90, "con");
		InitFontRam(start+0x80, &consolas16x26_graynum, "con+gray");
		FOsdInitBpp3AlphaTable(0);
		//FOsdSetDefPaletteColor(1);
		break;
	case 4:
		InitFontRam(start, &default_font, "def");
		ReplaceFontRam(start+0xA0, &kor_font, "def+kor");
		break;
	default:
		//dPuts("\nDOWNLOAD SPI_FONT0");
		//InitFontRam(0, &SPI_FONT0_TEMP,"FONT0");
		InitFontRam(start, &default_font, "def");
		break;
	}
}

/**
* print Font Info
*/
static void FontInfo(WORD start, FONT_SPI_INFO_t *font)
{
	BYTE i;
	WORD *wptr;

	Printf("\nLoc:%lx", font->loc );
	Printf(" size:%x", font->size);
	Printf(" %bdx%bd", font->width, font->height);
	Printf(" 2BPP:%x 3BPP:%x 4BPP:%x MAX:%x",font->bpp2+start,font->bpp3+start,font->bpp4+start,font->max+start);
	if (font->palette_bpp2 != NULL)
	{
		Printf("\n\t2BPP:");
		wptr = (WORD *)font->palette_bpp2;
		for (i=0; i < 4; i++)
			Printf("%04x ", *wptr++);
	}
	if (font->palette_bpp3 != NULL)
	{
		Printf("\n\t3BPP:");
		wptr = (WORD *)font->palette_bpp3;
		for (i=0; i < 8; i++)
			Printf("%04x ",*wptr++);
	}
	if (font->palette_bpp4 != NULL)
	{
		Printf("\n\t4BPP:");
		wptr = (WORD *)font->palette_bpp4;
		for (i=0; i < 8; i++)
			Printf("%04x ",*wptr++);
		Printf("\n\t    :");
		for (i=0; i < 8; i++)
			Printf("%04x ",*wptr++);
	}
}

/**
* print Font Info by number
*/
void FontInfoByNum(BYTE FontMode)
{
	switch ( FontMode )
	{
		case 0:	
			Puts("\ndefailt_font");
			FontInfo(0, &default_font);			
			break;
		case 1:
			Puts("\nconsolas");
			FontInfo(0, &consolas16x26_606C90);
			break;
		case 2:
			Puts("\nconsolas_graynum");
			FontInfo(0, &consolas16x26_graynum);
			break;
		case 3:
			Puts("\nconsolas+graynum");
			FontInfo(0, &consolas16x26_606C90);
			FontInfo(0+0x80, &consolas16x26_graynum);
			break;
		case 4:
			Puts("\ndef+kor");
			FontInfo(0, &default_font);
			FontInfo(0+0xA0, &kor_font);
			break;
		default:
			break;
	}
}

/**
* dump current downloaded font
*/
void DumpFont(void)
{
	WORD start, next, size;
	WORD dat, addr;
	BYTE value;
	BYTE w_cnt;
	WORD Y;

	Printf("\nname:%s", FontOsdInfo.font.name);
	Printf(" %bdx%bd", 	FontOsdInfo.font.w,FontOsdInfo.font.h);

	//WIN0 for 1BPP
	FOsdWinInit(0);
	start = 0;
	next = FontOsdInfo.font.bpp2;
	size = next - start;
	Printf("\n1BPP start:%d end:%d size:%d",start,next-1,size);
	FOsdRamSetAddrAttr(start, 0x0F);
	delay1ms(1);
	w_cnt = 0;
	for (addr = 0; addr < next; addr++)
	{
		FOsdRamSetData(addr); //same as data
		w_cnt++;
		if (w_cnt == 4)
		{
			w_cnt = 0;
			delay1ms(1);
		}
	}
	FOsdWinScreenXY(0, 0, 0);
	FOsdWinScreenWH(0, (size >=16 ? 16: size), (size >> 4) + (size & 0x0F ? 1 : 0));	Y = ((size >> 4) + 1)* FontOsdInfo.font.h + 10;
	FOsdWinZoom(0, 1/*0*/, 0);
	FOsdWinMulticolor(0, ON);
	FOsdWinSetOsdRamStartAddr(0, start);
	FOsdWinEnable(0, ON);

	//WIN1 for 2BPP
	FOsdWinInit(1);
	start = next;
	next = FontOsdInfo.font.bpp3;
	size = next - start;
	size /= 2;
	Printf("\n2BPP start:%d end:%d size:%d",start,next,size);
	FOsdRamSetAddrAttr(start, 36>>2);
	for (addr=start,dat=start; dat < next; addr++,dat+=2)
	{
		FOsdRamSetAddress(addr);
		FOsdRamSetAttr(36>>2);
		FOsdRamSetAddress(addr);
		FOsdRamSetData(dat);	
		delay1ms(1);
	}
	FOsdWinScreenXY(1, 0, Y);
	FOsdWinScreenWH(1, (size >=16 ? 16: size), (size >> 4) + (size & 0x0F ? 1 : 0));	Y += ((size >> 4) + 1)* FontOsdInfo.font.h + 10;
	FOsdWinZoom(1, 1/*0*/, 0);
	FOsdWinMulticolor(1, ON);
	FOsdWinSetOsdRamStartAddr(1, start);
	FOsdWinEnable(1, ON);

	//WIN2 for 3BPP
	FOsdWinInit(2);
	start = next;
	next = FontOsdInfo.font.bpp4;
	size = next - start;
	size /= 3;
	Printf("\n3BPP start:%d end:%d size:%d",start,next,size);
	FOsdRamSetWriteMode(FOSD_OSDRAM_WRITE_NORMAL);
	for (addr=start,dat=start; dat < next; addr++, dat+=3)
	{
		FOsdRamSetAddress(addr);
		FOsdRamSetAttr(40>>2);
		FOsdRamSetAddress(addr);
		FOsdRamSetData(dat);	
		delay1ms(1);
	}
	FOsdWinScreenXY(2, 0, Y);
	FOsdWinScreenWH(2, (size >=16 ? 16: size), (size >> 4) + (size & 0x0F ? 1 : 0));	Y += ((size >> 4) + 1)* FontOsdInfo.font.h + 10;
	FOsdWinZoom(2, 1/*0*/, 0);
	FOsdWinMulticolor(2, ON);
	FOsdWinSetOsdRamStartAddr(2, start);
	FOsdWinEnable(2, ON);

	//WIN3 for 4BPP
	FOsdWinInit(3);
	start = next;
	value = ReadTW88(REG_FOSD_MUL_CON) & 0x7F; //bytes per font
	next = 10*1024 / value;
	size = next - start;
	size /= 4;
	Printf("\n4BPP start:%d end:%d size:%d",start,next,size);
	FOsdRamSetAddrAttr(start, 48>>2);
	for (addr=start,dat=start; dat < next; addr++, dat+=4)
	{
		FOsdRamSetAddress(addr);
		FOsdRamSetAttr(48>>2);
		FOsdRamSetAddress(addr);
		FOsdRamSetData(dat);	
		delay1ms(1);
	}
	FOsdWinScreenXY(3, 0, Y);
	FOsdWinScreenWH(3, (size >=16 ? 16: size), (size >> 4) + (size & 0x0F ? 1 : 0));
	FOsdWinZoom(3, 2, 3/*0,0*/);
	FOsdWinMulticolor(3, ON);
	FOsdWinSetOsdRamStartAddr(3, start);
	FOsdWinEnable(3, ON);
}

