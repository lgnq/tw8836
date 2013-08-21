/**
 * @file
 * OSDSPI.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2013 Intersil Corporation
 * @section DESCRIPTION
 *	low level SpiOSD layer
*/
//*****************************************************************************
//
//								OSD.c
//
//*****************************************************************************
//
//
#include "config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"
#include "Global.h"

#include "CPU.h"
#include "printf.h"
#include "Util.h"

#include "I2C.h"
#include "spi.h"

#include "SOsd.h"
#include "FOsd.h"
#include "SpiFlashMap.h"
#include "Settings.h"

#define SPILUTBUFF_WIN

#ifdef SUPPORT_SPIOSD
//==========================================
//TW8836 SpiOSD Registers
//
//win#  addr    Group	description
//----  ----    -----	-----------
//win0: 0x420	A		animation
//win1: 0x440	B		background. low priority.
//win2: 0x430	B		background.	
//win3: 0x440   A
//...
//win8: 0x4B0	A		focused.    high priority.
//----------------------------------------
XDATA BYTE SpiWinBuff[10*0x10];

#ifdef SPILUTBUFF_WIN
//------------------------
//0: use flag
//1: size high
//2: size low
//3: lut offset	(LutOffset >> 6)
//4: lut offset	(LutOffset << 2)
//    normal              4BYTE mode
//    -------             ----------
//5: (address>>16) 		  5: (address>>24)
//6: (address>>8)		  6: (address>>16)
//7: (address)			  7: (address>>8)
//                        8: (address)
//------------------------
BYTE SOsdHwBuff_win[9*9];

//REG410[2] : Group B
//
//REG404[7:4]	RLCA_WIN_GPA
//REG404[1]		RLCA_RESET_GPA
//REG405[7:4]	RLCA_DCNT_GPA
//REG405[3:0]	RLCA_CCNT_GPA
//
//REG406[7:4]	RLCB_WIN_GPB
//REG406[1]		RLCB_RESET_GPB
//REG407[7:4]	RLCB_DCNT_GPB
//REG407[3:0]	RLCB_CCNT_GPB

WORD SOsdHwBuff_alpha_A;
WORD SOsdHwBuff_alpha_B;

BYTE SOsdHwBuff_rle_A_win;
BYTE SOsdHwBuff_rle_A_bpp;
BYTE SOsdHwBuff_rle_A_count;
BYTE SOsdHwBuff_rle_B_win;
BYTE SOsdHwBuff_rle_B_bpp;
BYTE SOsdHwBuff_rle_B_count;
#endif


//TW8836 has 9 windows
code WORD	SpiOsdWinBase[9] = { SPI_WIN0_ST, 
		SPI_WIN1_ST, SPI_WIN2_ST, SPI_WIN3_ST, SPI_WIN4_ST,
		SPI_WIN5_ST, SPI_WIN6_ST, SPI_WIN7_ST, SPI_WIN8_ST
		};

//=============================================================================
//		OSD Window Functions
//=============================================================================


//=============================================================================
//		OSD Window Setup
//=============================================================================


//-----------------------------------------------------------------------------
// R40E[7:4]	OSD Linebuffer MSB



//-----------------------------------------------------------------------------
/**
* Description
*	Set SpiOsd DE value.
*
*	How to calculate DE value.
*	HDE = REG(0x210[7:0])
*	PCLKO = REG(0x20d[1:0]) {0,1,2,2}
*	PCLKO = REG(0x20d[1:0]) {1,1,1,1}  new
*	result = HDE + PCLKO - 17
* @param
* @return
*/
void SpiOsdSetDeValue(void)
{
	XDATA	WORD wTemp;
	BYTE hDE,pclko;

	WriteTW88Page(PAGE2_SCALER);
	hDE = ReadTW88(REG210);
	pclko = ReadTW88(REG20D) & 0x03;
	//if(pclko == 3)
	//	pclko = 2;
	//pclko = 1;
	pclko = 0;

	wTemp = (WORD)hDE + pclko - 18;

	WriteTW88Page(PAGE4_SOSD);
	WriteTW88(REG40E, (BYTE)(wTemp>>8) );	// write SPI OSD DE value(high nibble)
	WriteTW88(REG40F, (BYTE)wTemp );   		// write SPI OSD DE value(low byte)
#ifdef DEBUG_OSD
	dPrintf("\nSpiOsdDe:%04x",wTemp);		
#endif
}


//-----------------------------------------------------------------------------
/**
* Description
*	Enable SpiOsd.
*	It is a HW function.
*
* 	If enable SpiOSD, HW uses CLKPLL.(normally use divider 1.5).
* 	If disable SpiOSD, HW uses 27MHz.
*
*	set I2C_delay_base depend on the MCU clock speed.
*
* On MODEL_TW8835_EXTI2C.
*	Do not toggle MCUSPI clock. Internal MCU can not make a synch.
*
* @param 	en
*	1: Enable SpiOSD, 0:Disable SpiOSD.
* @return	void
* 
* extern
*	I2C_delay_base	
*/
void SpiOsdEnable(BYTE en)
{
	BYTE dat;
	WriteTW88Page(PAGE4_SOSD );
	dat = ReadTW88(REG400);
	if( en ) {
		//
		//SPIOSD mode uses PCLK or PLL108
		//
		McuSpiClkSelect(MCUSPI_CLK_PCLKPLL);				//select MCU/SPI Clock.
		I2C_delay_base = 3;									//assume 108/1.5.

		WriteTW88Page(PAGE4_SOSD );
		WriteTW88(REG400, dat | 0x04);						//enable SpiOSD
	}
	else {
		//
		//normal mode uses 27MHz clock.
		//
		WriteTW88(REG400, dat & ~0x04);						//disable SpiOSD

		McuSpiClkSelect(MCUSPI_CLK_27M); 					//select MCU/SPI Clock.
		I2C_delay_base = 1;
	}
}

//==============================
// Windows Finctions
//==============================

//-----------------------------------------------------------------------------
/**
* Description
*	Enable SpiOsd Window. HW function.
* @param	winno
*	0 to 8.
* @param	en
*	1:Enable 0:disable
* @return	void
*/
/**
* 
*/
void SpiOsdWinHWEnable(BYTE winno, BYTE en)
{
	XDATA WORD index;
	XDATA	BYTE dat;

	index = SpiOsdWinBase[winno] + SPI_OSDWIN_ENABLE;

	WriteTW88Page(PAGE4_SOSD );
	dat = ReadTW88(index);
	if( en ) WriteTW88(index, dat | 0x01);
	else     WriteTW88(index, dat & 0xfe);
}

#ifdef MODEL_TW8836____TEST
//-----------------------------------------------------------------------------
/**
* Description
*
* @param
* @return
*
* reg
*	REG420[1] REG440[1] REG450[1],...
*/
void SpiOsdWinHZoom(BYTE winno, BYTE en)
{
	XDATA WORD index;
	XDATA	BYTE dat;

	index = SpiOsdWinBase[winno] + SPI_OSDWIN_HZOOM;

	WriteTW88Page(PAGE4_SOSD );
	dat = ReadTW88(index);
	if( en ) WriteTW88(index, dat | 0x02);
	else     WriteTW88(index, dat & ~0x02);
}
#endif


#if 0
//-----------------------------------------------------------------------------
/**
* Description
*
* @param
* @return
*/
BYTE SpiOsdWinIsOn(BYTE winno)
{
	XDATA WORD index;

	index = SpiOsdWinBase[winno] + SPI_OSDWIN_ENABLE;

	WriteTW88Page(PAGE4_SOSD );
	if( ReadTW88(index) & 0x01 ) return ON;
	else						 return OFF;
}
#endif

//-----------------------------------------------------------------------------
/**
* Description
*	enable SpiOSD window on WinBuff.
* @param
* @return
*/
void SpiOsdWinBuffEnable(BYTE winno, BYTE en)
{
	DATA BYTE XDATA *data_p;

	if(winno) winno++;
	data_p = &SpiWinBuff[winno << 4];

	if(en) *data_p |= 0x01;
	else   *data_p &= 0xfe;
}


//-----------------------------------------------------------------------------
/**
* Description
*	clear all SpiWinBuff[]
* @param	hw
*	1:update HW. 0:skip HW update.
* @return	void
*/
void SOsdWinBuffClean(BYTE hw)
{
	BYTE i,j;

	if (hw)
	{
		SpiOsdWinHWOffAll(1);
	 	SpiOsdRlcReg(0, 0, 0);	//disable RLC
	}

	for (i=0; i < 10; i++)
	{
		for (j=0; j < 0x0E; j++)
			SpiWinBuff[i*16+j]=0;
	}
}

//-----------------------------------------------------------------------------
/**
* Description
* 	write SpiWinBuff to HW registers
*
*	start address for ecah window
*
*	WIN			0	1	2	3	4	5	6	7	8
*   ----        --- --- --- --- --- --- --- --- ---
*	register	420 440 450 460 470 480 490 4A0 4B0
*
* @param  start: start window. between 0 to 8
* @param  end:   end window. between 0 to 8
*
* Note: this function needs a WaitVBlank(1).
*       Execute WaitVBlank(1) before you call this function.
*/
#pragma SAVE
#pragma OPTIMIZE(8,SPEED)
void SOsdWinBuffWrite2Hw(BYTE start, BYTE end)
{
	DATA BYTE i;
	DATA WORD reg_i;
	DATA BYTE XDATA *data_p;

#ifdef DEBUG_OSD
	dPrintf("\nSOsdWinBuffWrite2Hw(%bd,%bd)",start,end);
#endif

	//win0 uses 0x20, if not win0, increase 0x10.
	if(start)	start++;
	if(end)		end++;

#ifdef DEBUG_OSD
	//dump BK130104
	//if(start==2 && end==2) {
	//	data_p = &SpiWinBuff[start << 4];
	//	for(i=0; i < 0x0F; i++)
	//		Printf(" %02bx", data_p[i]);
	//}
#endif
	

#ifdef DEBUG_OSD
	//NOTE: If you print the debug message, we have to check the vblank again.
	WaitVBlank(1);
#endif
	WriteTW88Page(PAGE4_SOSD);
	data_p = &SpiWinBuff[start << 4];

	for(i=start; i <= end; i++) {
		reg_i = 0x400 | (i << 4) + 0x20;
													
		WriteTW88(reg_i++, *data_p++);	//0
		WriteTW88(reg_i++, *data_p++);	//1
		WriteTW88(reg_i++, *data_p++);	//2
		WriteTW88(reg_i++, *data_p++);	//3
		WriteTW88(reg_i++, *data_p++);	//4
		WriteTW88(reg_i++, *data_p++);	//5
		WriteTW88(reg_i++, *data_p++);	//6
		WriteTW88(reg_i++, *data_p++);	//7
		WriteTW88(reg_i++, *data_p++);	//8
		WriteTW88(reg_i++, *data_p++);	//9
		WriteTW88(reg_i++, *data_p++);	//A
		WriteTW88(reg_i++, *data_p++);	//B
		WriteTW88(reg_i++, *data_p++);	//C
		WriteTW88(reg_i++, *data_p++);	//D
		WriteTW88(reg_i++, *data_p++);	//E
		WriteTW88(reg_i++, *data_p++);	//F
		if(i==0) {
			i++;
			WriteTW88(reg_i++, *data_p++);	//10  REG430
			WriteTW88(reg_i++, *data_p++);	//11
			WriteTW88(reg_i++, *data_p++);	//12
			WriteTW88(reg_i++, *data_p++);	//13
			WriteTW88(reg_i++, *data_p++);	//14
			WriteTW88(reg_i++, *data_p++);	//15
			WriteTW88(reg_i++, *data_p++);	//16 REG436
			WriteTW88(reg_i++, *data_p++);	//17 REG437
			reg_i+=8;
			data_p+=8;
		}
	}
}
#pragma RESTORE


#if 0
//-----------------------------------------------------------------------------
/**
* Description
*	check win buff, if HW is enabled and buff is not, disable HW
* @param
* @return
*/
void SpiOsdWinBuffSynchEnable(void)
{
	BYTE winno;
	BYTE buff;
	WORD index;
	BYTE dat;

	WriteTW88Page(PAGE4_SOSD );

	dPrintf("\nSpiOsdWinBuffSynchEnable ");
	for(winno=0; winno <= 8; winno++) {
		if(winno) 	buff = SpiWinBuff[(winno+1) << 4];
		else		buff = SpiWinBuff[winno << 4];

		index = SpiOsdWinBase[winno] + SPI_OSDWIN_ENABLE;
		dat = ReadTW88(index);

		if(buff != dat) {
			dPrintf("win%02bx %02bx->%02bx ",winno, dat, buff);
			WriteTW88(index, dat & 0xfe);
		}
	}
	//RLE will be synch on menu, not here
}
#endif

//-----------------------------------------------------------------------------
/**
* Description
*	turn off all SpiOsd Window.
* @param
* @return
* @see SpiOsdWinHWEnable
*/
void SpiOsdWinHWOffAll(BYTE wait)
{
	BYTE i;

	if(wait)
		WaitVBlank(wait);
	SpiOsdDisableRlcReg(0);
	for(i=0; i<= 8; i++)
		SpiOsdWinHWEnable(i, 0);
}

//-----------------------------------------------------------------------------
/**
* Description
* 	set image location
*
* @param	winno
*	SpiOsd Window number. 0..8
* @param	start:
* @return	void
*
* reg
*	WINx Image Location on SpiFlash	 
*/
void SpiOsdWinImageLoc(BYTE winno, DWORD start)
{	
	DATA BYTE XDATA *data_p;

	if(winno) winno++;
	data_p = &SpiWinBuff[winno << 4];

	data_p += SPI_OSDWIN_BUFFERSTART;

//#ifdef MODEL_TW8835
//	*data_p++ =  (BYTE)(start>>16);				//+0x07
//	*data_p++ =  (BYTE)(start>>8);				//+0x08
//	*data_p++ =  (BYTE)start;					//+0x09
//#else
	//support 32bit SPIFLASH address
	//from 121005. TW8836 uses 4bit align registers.
	*data_p++ =  (BYTE)(start>>20);				//+0x07
	*data_p++ =  (BYTE)(start>>12);				//+0x08
	*data_p   =  (BYTE)(start>>4);				//+0x09
//#endif

#if 0	
	//new patch for low 4bit address. We need new silicon.
	if(winno==0) data_p += 0x0E;				//+0x17	 
	else		 data_p += 6;					//+0x0F
	*data_p = (BYTE)start & 0x0F;	 
#endif
}

//-----------------------------------------------------------------------------
/**
* Description
* 	set image bit location.
*
* @param	winno
*	from 1 to 8. win0 does not have a bit operation.
* @param	start
* @return
*
* reg
*	win0 win1       win2 
*	N/A  0x44A[7:6] 0x45A[7:6].,,,
*/
void SpiOsdWinImageLocBit(BYTE winno, BYTE start)
{
	DATA BYTE XDATA *data_p;

	if(winno==0) {
		//wrong. win0 do not have a bit operation.
		return;
	}

	winno++;
	data_p = &SpiWinBuff[winno << 4];

	data_p += SPI_OSDWIN_BUFFERSTART_BIT;
	*data_p &= 0x3F;
	*data_p |= (start << 6);
}


//-----------------------------------------------------------------------------
/**
* Description
*	set image buffer width
* @param	winno
* @param	w
*	width
* @return	void
*/
static void SpiOsdWinImageSizeW(BYTE winno, WORD w)
{
	DATA BYTE XDATA *data_p;
	BYTE value;

	if(winno) winno++;

	data_p = &SpiWinBuff[winno << 4];
	data_p += SPI_OSDWIN_DISPSIZE;

 	value = *data_p & 0xC0;

	*data_p++ = (BYTE)(w>>8 | value);		//+0x0A
	*data_p++ = (BYTE)w;					//+0x0B
}

//-----------------------------------------------------------------------------
/**
* Description
*	set image width and height
* @param	winno
* @param	w
*	width
* @param	h
*	height
* @return	void
*/
void SpiOsdWinImageSizeWH (BYTE winno, WORD w, WORD h)
{
	DATA BYTE XDATA *data_p;
	BYTE value;

	//WIN1to8 need only Width.
	if(winno) {
		SpiOsdWinImageSizeW(winno,w);
		return; 
	}

	//now only for WIN0
	data_p = SpiWinBuff;				   //Only WIN0
	data_p += SPI_OSDWIN_DISPSIZE;

	value = (BYTE)(h >> 8);
	value <<= 4;
	value |= (BYTE)( w>>8 );
	*data_p++ = value; 		//42A
	*data_p++ = (BYTE)w;	//42B
	*data_p++ = (BYTE)h;	//42C
}


//-----------------------------------------------------------------------------
/**
* Description
*	set window position and size
* @param
* @return
*/
//WINx Screen(win) Pos & Size
void SpiOsdWinScreen(BYTE winno, WORD x, WORD y, WORD w, WORD h)
{
	DATA BYTE XDATA *data_p;
	BYTE value;

	if(winno) winno++;
	data_p = &SpiWinBuff[winno << 4];

	data_p += SPI_OSDWIN_SCREEN;
	value = (y >> 8);
	value <<= 4;
	value |= (x >> 8);
	*data_p++ = value;		//421	441...
	*data_p++ = (BYTE)x;	//422	442... 	
	*data_p++ = (BYTE)y;	//423	443...
	
	value = (h >> 8);
	value <<= 4;
	value |= (w >> 8);
	*data_p++ = value;		//424	444...
	*data_p++ = (BYTE)w;	//425	445...	 	
	*data_p++ = (BYTE)h;	//426	446...	 
}


//=============================================================================
//		Load LUT
//=============================================================================
//
//-----------------------------------------------------------------------------
/**
* Description
*	set Lut Offset
*	LUT offset use 5bit & 16 unit
* @param
* @return
*/
void SpiOsdWinLutOffset( BYTE winno, WORD table_offset )
{
	DATA BYTE XDATA *data_p;

	if(winno) winno++;
	data_p = &SpiWinBuff[winno << 4];
	data_p += SPI_OSDWIN_LUT_PTR;
	if(!winno) data_p += 4;
	
	//LUT offset use 5bit & 16 unit
	*data_p = table_offset >> 4;
}

		


//=============================================================================
//		Pixel Width
//=============================================================================
//-----------------------------------------------------------------------------
/**
* Description
* 	set pixel width
*
* @param	winno
* @param	bpp
*	0:4bit, 1:6bit others:8bit.
*	7bpp uses 8bit.
* @return	void
*/
void SpiOsdWinPixelWidth(BYTE winno, BYTE bpp)
{
	DATA BYTE XDATA *data_p;
	BYTE mode;

	if(bpp==4)		mode=0;
	else if(bpp==6) mode=1;
	else 			mode=2;

	if(winno) 	winno++;
	data_p = &SpiWinBuff[winno << 4];

	*data_p &= 0x3f;
	*data_p |= (mode <<6);
}
//-----------------------------------------------------------------------------
/**
* Description
* 	fill color
*	color will be an offset from the LUT location that Window have.
*	If window starts LUT from 80, the color value means color+80 indexed color.
* @param	winno
* @param	color
* @return
*/
/**
*/
void SpiOsdWinFillColor( BYTE winno, BYTE color )
{
	WORD index;

	index = SpiOsdWinBase[winno];
	WriteTW88Page(PAGE4_SOSD );

	if ( color ) {
		WriteTW88(index, (ReadTW88(index ) | 0x04));				// en Alpha & Global
	}
	else {
		WriteTW88(index, (ReadTW88(index ) & 0xFB ) );				// dis Alpha & Global
	}
	index = SpiOsdWinBase[winno] + SPI_OSDWIN_FILLCOLOR;
	if(!winno)	index += 8;
	WriteTW88(index, color );
}

//-----------------------------------------------------------------------------
/**
* Description
* 	Enable Global Alpha and set the alpha value
* @param winno
* @param alpha
*	0 to 7F. 0x7F is a higest transparent value.
* @return
*/
/**
* set global alpha
*/
void SpiOsdWinGlobalAlpha( BYTE winno, BYTE alpha )
{
	DATA BYTE XDATA *data_p;

	if(winno) winno++;
	data_p = &SpiWinBuff[winno << 4];

	*data_p &= 0xCF;
	if(alpha) *data_p |= 0x10;

	data_p += SPI_OSDWIN_ALPHA;
	if(!winno)	data_p += 4;
	*data_p = alpha;
}

//-----------------------------------------------------------------------------
/**
* Description
*	set Pixel alpha
* @param winno
* @param alpha
*	0 to 7F. 0x7F is a higest transparent value.
* @return
*/
void	SpiOsdWinPixelAlpha( BYTE winno, BYTE alpha )
{
	DATA BYTE XDATA *data_p;

	if(winno) winno++;
	data_p = &SpiWinBuff[winno << 4];

	if(alpha)	*data_p |= 0x30;
	else		*data_p &= 0xCF;

	data_p += SPI_OSDWIN_ALPHA;
	if(!winno)	data_p += 4;
	*data_p = alpha;
}



//-----------------------------------------------------------------------------
/**
* Description
*	adjust Win0 priority
* 	NOTE:Only fow Win0.
* @param
* @return
*/
void SpiOsdWin0SetPriority(BYTE high)
{
#if 0
	XDATA	BYTE dat;
	WriteTW88Page(PAGE4_SOSD );
	dat = ReadTW88(REG420);
	if( high ) WriteTW88(REG420, dat | 0x02);
	else       WriteTW88(REG420, dat & 0xfd);
#else
	DATA BYTE XDATA *data_p;

	data_p = &SpiWinBuff[0];

	if(high) *data_p |= 0x02;
	else   	*data_p &= 0xfd;
#endif
}

//=============================================================================
//		Animation
//=============================================================================
//-----------------------------------------------------------------------------
/**
* Description
* 	set Animation.
*	Only for WIN0
* @param mode	
*	-0:display one time of the loop and then disappear
*	-1:display one time of the loop and then stay at the last frame
*	-2:Enable looping 
*	-3:static. Show the frame pointed by (0x431 and 0x432)
* @param Duration duration time of each frame (in unit of VSync)
*	- 0: infinite
*	- 1: One VSync period
*	- max 0xFF: 255 VSync period		
* @return
*/
void SpiOsdWin0Animation(BYTE mode, BYTE FrameH, BYTE FrameV, BYTE Duration)
{
	DATA BYTE XDATA *data_p;

	data_p = SpiWinBuff;
	data_p += SPI_OSDWIN_ANIMATION;

	*data_p++ = FrameH;
	*data_p++ = FrameV;
	*data_p++ = Duration;

	*data_p &= 0x3f;
	*data_p |= (mode << 6);
}

//-----------------------------------------------------------------------------
/**
* Description
*	set Win0 X,Y
* @param
* @return
*/
void SpiOsdWin0ImageOffsetXY (WORD x, WORD y)
{
	BYTE value;
	DATA BYTE XDATA *data_p;

	data_p = SpiWinBuff;			//Only WIN0
	data_p += SPI_OSDWIN_DISPOFFSET;

	value  = (BYTE)(y >> 8);
	value <<=4;
	value |= (BYTE)(x >> 8);
	*data_p++ = value;
	*data_p++ = (BYTE)x;
	*data_p++ = (BYTE)y;
}



#ifdef FUNCTION_VERIFICATION
//no DMA
//@param
//	type:	
//		1:Byte pointer - LUTS type
//		0:Address pointer - LUT type
//void SpiOsdIoLoadLUT_ARRAY(BYTE type, WORD LutOffset, WORD size, BYTE *pData)
//{
//}
#endif





//-----------------------------------------------------------------------------
/**
* Description
*	clear HwBuff.
* @param
* @return
*/
void SOsdHwBuffClean(void)
{
	BYTE i;

	SOsdHwBuff_alpha_A=0xffff;
	SOsdHwBuff_alpha_B=0xffff;
	SOsdHwBuff_rle_A_win=0;
	SOsdHwBuff_rle_B_win=0;

	for(i=0; i<=8; i++) {
		//clear use flag
		SOsdHwBuff_win[i*9]=0;		
	}
}

//-----------------------------------------------------------------------------
/**
* Description
*	set LUT info to HwBuff.
* @param	winno
* @param	LutOffset
*	destination LUT location
* @param	size
* @param	address
*	image location on SpiFlash
* @return
*/
void SOsdHwBuffSetLut(BYTE win, /*BYTE type,*/  WORD LutOffset, WORD size, DWORD address)
{
   	SOsdHwBuff_win[win*9+0] = 1;
	SOsdHwBuff_win[win*9+1] = (BYTE)(size >> 8);
	SOsdHwBuff_win[win*9+2] = (BYTE)size;

	SOsdHwBuff_win[win*9+3] = (BYTE)(LutOffset >> 6);
	SOsdHwBuff_win[win*9+4] = (BYTE)(LutOffset << 2);

	if(SpiFlash4ByteAddr) {
		SOsdHwBuff_win[win*9+5] = (BYTE)(address >> 24);
		SOsdHwBuff_win[win*9+6] = (BYTE)(address >> 16);
		SOsdHwBuff_win[win*9+7] = (BYTE)(address >> 8);
		SOsdHwBuff_win[win*9+8] = (BYTE)(address);
	}
	else {
		SOsdHwBuff_win[win*9+5] = (BYTE)(address >> 16);
		SOsdHwBuff_win[win*9+6] = (BYTE)(address >> 8);
		SOsdHwBuff_win[win*9+7] = (BYTE)(address) ;
	}
}

//-----------------------------------------------------------------------------
/**
* Description
*	set RLE info to HwBuff
* @param
* @return
*/
void SOsdHwBuffSetRle(BYTE win, BYTE bpp, BYTE count)
{
	if(win==1 || win==2) {
		SOsdHwBuff_rle_B_win = win;
		SOsdHwBuff_rle_B_bpp = bpp;
		SOsdHwBuff_rle_B_count = count;
	}
	else {
		SOsdHwBuff_rle_A_win = win;
		SOsdHwBuff_rle_A_bpp = bpp;
		SOsdHwBuff_rle_A_count = count;
	}
}
//-----------------------------------------------------------------------------
/**
* Description
*	set Alpha to HwBuff
* @param
* @return
*/
void SOsdHwBuffSetAlpha(BYTE win, WORD alpha_index)
{
	if(win==1 || win==2)
		SOsdHwBuff_alpha_B = alpha_index;
	else
		SOsdHwBuff_alpha_A = alpha_index;
}

/*
example: volatile & memory register access
volatile BYTE XDATA mm_dev_R1CD	_at_ 0xC1CD;	//use 1 XDATA BYTE
//#define TW8835_R1CD	(*((unsigned char volatile xdata *) (0xc000+0x1CD)))
#define TW8835_R1CD	(*((unsigned char volatile xdata *) (REG_START_ADDRESS+0x1CD) ))
void Dummy_Volatile_memory_register_test(void)
{
	volatile BYTE mode;
	volatile BYTE XDATA *p; // = (BYTE XDATA *)0xC1CD;

	mode = *(volatile BYTE XDATA*)(0xC1CD);

	p = (BYTE XDATA *)0xC1CD;
	mode = *p;

	mode = mm_dev_R1CD;

	mode = TW8835_R1CD;
}
*/

//-----------------------------------------------------------------------------
/**
* Description
*	write H2Buff to real HW
* @param
* @return
*/

void SOsdHwBuffWrite2Hw(void)
{
	BYTE win;
	DATA BYTE XDATA *data_p;
	DATA BYTE i,j;
	DATA WORD reg_i;
	WORD LutOffset;
//	BYTE dcnt,ccnt;
	BYTE bTemp;

#ifdef DEBUG_OSD
	dPuts("\nSOsdHwBuffWrite2Hw.....");
#endif

	WaitVBlank(1);	

	//-----------------------------
	// time cirtical section start
	//-----------------------------
//SFR_EA = 0;
	//save clock mode & select PCLK
#if defined(TW8836_CHIPDEBUG)
	//BK120615. for test clock source
#else
	//McuSpiClkToPclk(CLKPLL_DIV_2P0);	//with divider 1=1.5(72MHz)	try 2	TW8835
	McuSpiClkToPclk(CLKPLL_DIV_3P0);	//with divider 3(36MHz)	try 3	TW8836
#endif


	//----------------------------
	//update LUT
	WriteTW88Page(PAGE4_SOSD );
	WriteTW88(REG410, 0xa0 );    			// LUT Write Mode, En & byte ptr inc. DMA needs it.
	WriteTW88(REG411, 0); 					// LUT addr. set 0 on DMA


	for(win=0; win <= 8; win++) {
		//check Use flag.
		if(SOsdHwBuff_win[win*9] == 0) 
			continue;

//#ifdef DEBUG_OSD
//		dPrintf("SWIN:%bd ",win);
//#endif
		if(win == 1 || win == 2)	//BK121312
			WriteTW88(REG410, 0xa4 );
		else
			WriteTW88(REG410, 0xa0 );

		data_p = &SOsdHwBuff_win[win*9+1];

		//Spi Flash DMA
		WriteTW88(REG4C0_SPIBASE+0x04, 0x00 );	// DMA stop

		WriteTW88(REG4C0_SPIBASE+0x03, 0x80 | SPICMD_x_BYTES + SpiFlash4ByteAddr); //LUT,Increase, 0x0B with 5 commands, 0xeb with 7 commands	           
		WriteTW88(REG4C0_SPIBASE+0x0a, SPICMD_x_READ ); 			// SPI Command=R

		//WriteTW88(REG4C0_SPIBASE+0x1a, 0x00 ); // DMA size
		WriteTW88(REG4C0_SPIBASE+0x08, *data_p++ );	//size0
		WriteTW88(REG4C0_SPIBASE+0x09, *data_p++ );	//size1
		LutOffset = *data_p;
		LutOffset <<= 6;

		WriteTW88(REG4C0_SPIBASE+0x06, *data_p++ ); 	//LutOffset[8:6] -> R4C6[2:0]
		LutOffset |= (*data_p & 0x3F);
		WriteTW88(REG4C0_SPIBASE+0x07, *data_p++ );		//LutOffset[5:0] -> R4C7[7:2] 
		WriteTW88(REG4C0_SPIBASE+0x0b, *data_p++); 		//address0
		WriteTW88(REG4C0_SPIBASE+0x0c, *data_p++ );		//address1
		WriteTW88(REG4C0_SPIBASE+0x0d, *data_p++ ); 	//address2
		WriteTW88(REG4C0_SPIBASE+0x0e, *data_p++ ); 	//address3

		WriteTW88(REG4C0_SPIBASE+0x04, 0x01 ); 			// DMA Start
		//while(ReadTW88Page() != PAGE4_SPI);			//trick. check DONE. BusyWait

	}
	//
	// update alpha
	//
	if(SOsdHwBuff_alpha_B != 0xFFFF) {
		//WriteTW88(REG410, 0xc3 | 0x04);    		// LUT Write Mode, En & byte ptr inc.	with GROUP B
		//if(SOsdHwBuff_alpha_A >> 8)	WriteTW88(REG410, ReadTW88(REG410) | 0x08);	//support 512 palette
		//else            			WriteTW88(REG410, ReadTW88(REG410) & 0xF7);

		bTemp = 0xC3 | 0x04;						   	//same, but select Group B. and only sopport 256.
		WriteTW88(REG410, bTemp);
		WriteTW88(REG411, (BYTE)SOsdHwBuff_alpha_B ); 	// alpha index
		WriteTW88(REG412, 0x7F/*value*/ ); 			// alpha value

		SOsdHwBuff_alpha_B = 0xFFFF;					//clear. BK130107
	}
	if(SOsdHwBuff_alpha_A != 0xFFFF) {
		//WriteTW88(REG410, 0xc3 );    		// LUT Write Mode, En & byte ptr inc.
		//if(SOsdHwBuff_alpha_A >> 8)	WriteTW88(REG410, ReadTW88(REG410) | 0x08);	//support 512 palette
		//else            			WriteTW88(REG410, ReadTW88(REG410) & 0xF7);

		bTemp = 0xC3;									// LUT Write Mode, En & byte ptr inc.
		if(SOsdHwBuff_alpha_A >> 8) bTemp |= 0x08;		//support 512 palette

		WriteTW88(REG410, bTemp);
		WriteTW88(REG411, (BYTE)SOsdHwBuff_alpha_A ); 	// alpha index
		WriteTW88(REG412, 0x7F/*value*/ ); 				// alpha value

		SOsdHwBuff_alpha_A = 0xFFFF;					//clear. BK130107
	}



#if defined(TW8836_CHIPDEBUG)
	//BK120615. for test clock source
#else
	//restore clock mode
	McuSpiClkRestore();
#endif

	//----------------------------
	//update RLE & pixel alpha

	WriteTW88Page(PAGE4_SOSD );
#if 0
//	if(SOsdHwBuff_rle_win) {
//		WriteTW88(REG404, ReadTW88(REG404) | 0x01);
//		WriteTW88(REG405, ((SOsdHwBuff_rle_bpp==7?8:SOsdHwBuff_rle_bpp) << 4) | (SOsdHwBuff_rle_count));
//		WriteTW88(REG406, SOsdHwBuff_rle_win);
//	}
//	else {
//		WriteTW88(REG404, ReadTW88(REG404) & 0xFE);
//		WriteTW88(REG405, 0);
//		WriteTW88(REG406, 0);
//	}
#else
	//temp BK121212.
	//SOsdHwBuff_rle_win = win;
	//SOsdHwBuff_rle_bpp = bpp;
	//SOsdHwBuff_rle_count = count;
	//dcnt = SOsdHwBuff_rle_bpp;
	//if(dcnt==7)
	//	dcnt++;
	//ccnt = SOsdHwBuff_rle_count;
	//if(SOsdHwBuff_rle_win==1 || SOsdHwBuff_rle_win==2) {
	//	WriteTW88(REG407, (dcnt << 4) | (ccnt));
	//	WriteTW88(REG406, (ReadTW88(REG406) & 0x0F) | (SOsdHwBuff_rle_win << 4));
	//}
	//else
	//{
	//	WriteTW88(REG405, (dcnt << 4) | (ccnt));
	//	WriteTW88(REG404, (ReadTW88(REG404) & 0x0F) | (SOsdHwBuff_rle_win << 4));
	//}
	//temp BK121217
	// we have A & B
	if(SOsdHwBuff_rle_A_win) {
		WriteTW88(REG404, SOsdHwBuff_rle_A_win << 4);
		WriteTW88(REG405, ((SOsdHwBuff_rle_A_bpp==7?8:SOsdHwBuff_rle_A_bpp) << 4) | (SOsdHwBuff_rle_A_count));
	}
	else {
		WriteTW88(REG404, 0);
		WriteTW88(REG405, 0);
	}
	if(SOsdHwBuff_rle_B_win) {
		WriteTW88(REG406, SOsdHwBuff_rle_B_win << 4);
		WriteTW88(REG407, ((SOsdHwBuff_rle_B_bpp==7?8:SOsdHwBuff_rle_B_bpp) << 4) | (SOsdHwBuff_rle_B_count));
	}
	else {
		WriteTW88(REG406, 0);
		WriteTW88(REG407, 0);
	}
#endif

	//----------------------------
	//update WIN buffer

	//note: I update only win1 to win8, not win0.
	//      Pls. do not use win0 here.
	//start = 1+1;
	//end = 8+1;
	data_p = &SpiWinBuff[ 2 /*start*/ << 4];
	for(i=2/*start*/; i <= 9/*end*/; i++) {
		reg_i = 0x400 | (i << 4) + 0x20;
		for(j=0; j < 16; j++) {
			WriteTW88(reg_i++, *data_p++);	
		}
	}
	//-----------------------------
	// time cirtical section end
	//-----------------------------
//SFR_EA = 1;
}

//-----------------------------------------------------------------------------
/**
* Description
* 	set alpha attribute on LUT (A or B).
* @param
* @return
*/
//BKTODO: If you donot using alpha, disable alpha.
void SpiOsdPixelAlphaAttr(BYTE win, WORD lutloc, BYTE value)
{
	BYTE bTemp;
	
	WriteTW88Page(PAGE4_SOSD );

	bTemp = 0xC3;						//en Write. ADDR type. Select Attr.
	if(win==1 || win==2)
		bTemp |= 0x04;					//select Group B.
	else if(lutloc >> 8)			
		bTemp |= 0x08;					//support 512 palette on Group A

	WriteTW88(REG410, bTemp);
	WriteTW88(REG411, (BYTE)lutloc ); 	// LUT addr
	WriteTW88(REG412, value ); 			// LUT addr
}



//-----------------------------------------------------------------------------
/**
* Description
* 	download LUT
*
* NOTE BK110330:after we support 512 palette, we donot support the address method.
* We need a width and a height info. but RTL only supports a size info.
* So, if you want to use the address method, use a PIO method.
*
* NOTE Do not add WaitVBlank() here.
*
* @param type	
*	- 1:Byte pointer - LUTS type. LUTTYPE_BYTE
*	- 0:Address pointer - LUT type. LUTTYPE_ADDR
*	if 0, use LutOffset:0, size:0x400.
* @param alpha
*	- 0xFF: no alpha. 
*	- other: use alpha value 0x7F. location is LutOffset+alpha.
* @see SpiOsdIoLoadLUT
* @see McuSpiClkToPclk
* @see McuSpiClkRestore
* @see SpiFlashDmaStart
*/
void SpiOsdLoadLUT(BYTE _winno, BYTE _type, WORD LutOffset, WORD size, DWORD address, BYTE alpha)
{
	BYTE reg;
	BYTE winno = _winno;
	BYTE type  = _type;

	if(type==LUTTYPE_ADDR) {
//BUG130124
//		if((winno != 1) && (winno != 2)) {
			SpiOsdIoLoadLUT(winno, type, LutOffset, size, address, alpha);
			return;
//		}
	}	
#ifdef DEBUG_OSD
	dPrintf("\nSpiOsdLoadLUT%s win:%bd, LutLoc:0x%x size:%d at:0x%06lx", type == LUTTYPE_ADDR ? "S":" ", _winno, LutOffset, size, address);
#endif

	//-------------------------------
	//save clock mode & select PCLK	
	//-------------------------------
#if defined(TW8836_CHIPDEBUG)
	//BK120615. for test clock source.
#else
	//McuSpiClkToPclk(CLKPLL_DIV_2P0);	//with divider 1=1.5(72MHz)	try 2. TW8835
	McuSpiClkToPclk(CLKPLL_DIV_3P0);	//with divider 3(36MHz)	try 3	TW8836
#endif

	WriteTW88Page(PAGE4_SOSD);

	//--- SPI-OSD config
	reg = 0x80;										// LUT Write Mode.
	if(type==LUTTYPE_ADDR)	reg |= 0x40;			// address ptr inc						
	else					reg |= 0x20;			// byte ptr inc.

	if(LutOffset & 0x0F00)  reg |= 0x08;			//select 512.										
	if(winno == 1 || winno == 2)					// if win1 or win2, 
		reg |= 0x04;								// select group LUT_B. Note Max 256.
	WriteTW88(REG410, reg);
	WriteTW88(REG411, (BYTE)LutOffset ); 			// LUT addr. set 0 on DMA


	//Spi Flash DMA
	WriteTW88(REG4C0_SPIBASE+0x04, 0x00 );	// DMA stop	

	WriteTW88(REG4C0_SPIBASE+0x03, 0x80 | SPICMD_x_BYTES + SpiFlash4ByteAddr); //LUT,Increase, 0x0B with 5 commands, 0xeb with 7 commands	           
	WriteTW88(REG4C0_SPIBASE+0x0a, SPICMD_x_READ ); 			// SPI Command=R

	if(SpiFlash4ByteAddr) {
		WriteTW88(REG4C0_SPIBASE+0x0b, (BYTE)(address>>24) );
		WriteTW88(REG4C0_SPIBASE+0x0c, (BYTE)(address>>16) );
		WriteTW88(REG4C0_SPIBASE+0x0d, (BYTE)(address>>8) );
		WriteTW88(REG4C0_SPIBASE+0x0e, (BYTE)(address) );
	}
	else {
		WriteTW88(REG4C0_SPIBASE+0x0b, (BYTE)(address>>16) );
		WriteTW88(REG4C0_SPIBASE+0x0c, (BYTE)(address>>8) );
		WriteTW88(REG4C0_SPIBASE+0x0d, (BYTE)(address) );
	}
	//d		h		addr	 addr  byte
	//0    0x00     0x000		0	0 	
	//128  0x80		0x200	   80   0
	//192  0xC0		0x300	   c0   0
	if(type==LUTTYPE_ADDR) {
		//if use addrss ptr inc.
		//addr_ptr = LutOffset;
		//byte_ptr  0;
		WriteTW88(REG4C0_SPIBASE+0x06, (BYTE)(LutOffset >> 8));		//LutOffset[8]  ->R4C6[0]
		WriteTW88(REG4C0_SPIBASE+0x07, (BYTE)LutOffset);			//LutOffset[7:0]->R4C7[7:0]		
	}
	else {
		//if use byte ptr inc.
		WriteTW88(REG4C0_SPIBASE+0x06, (BYTE)(LutOffset >> 6) ); 	//LutOffset[8:6] -> R4C6[2:0]
		WriteTW88(REG4C0_SPIBASE+0x07, (BYTE)(LutOffset << 2) );	//LutOffset[5:0] -> R4C7[7:2] 
		                                                        	//					R4C7[1:0]  start of byte ptr
	}

	WriteTW88(REG4C0_SPIBASE+0x1a, 0x00 ); // DMA size
	WriteTW88(REG4C0_SPIBASE+0x08, (BYTE)(size >> 8) );
	WriteTW88(REG4C0_SPIBASE+0x09, (BYTE)size );


	WriteTW88(REG4C0_SPIBASE+0x04, 0x01 ); // DMA Start
//P1_3 =0;
//P1_4 = !P1_4;
//	while(ReadTW88Page() != PAGE4_SPI);			//trick. check DONE. BusyWait
//EA = 1;

	
	//update pixel alpha value.
	if(alpha!=0xFF) {
		SpiOsdPixelAlphaAttr(winno,LutOffset+alpha, 0x7F);	
	}

#ifdef DEBUG_OSD
	dPrintf("\nSpiOsdLoadLUT --END");
#endif

	//-------------------------------
	//restore clock mode
	//-------------------------------
#if defined(TW8836_CHIPDEBUG)
//BK120619 test
#else
	McuSpiClkRestore();
#endif
}


//-----------------------------------
// RLC(RunLengthCompress) functions
//
//-----------------------------------

//-----------------------------------------------------------------------------
/**
* Description
* 	set RLC register
*
* @param winno win number.
*		winno 0 means disable.
* @param dcnt Data BPP
*	4:4bit, 6:6bit, others:8bit
*	7 means 8BPP with 128 color.	
* @param ccnt counter value.
*	- 4:4bit,5:5bit,..15:16bit, others:16bit
*/
void SpiOsdRlcReg(BYTE winno,BYTE dcnt, BYTE ccnt)
{
	//BKFYI: If you want to add a debug message, add WaitVBlank().
	//dPrintf("\nSpiOsdRlcReg(%bd,%bd,%bd)",winno,dcnt,ccnt);
	//WaitVBlank(1);
	
	if(dcnt==7)
		dcnt++;

	WriteTW88Page(PAGE4_SOSD );
	if(winno==0) {
		//clean A & B
		WriteTW88(REG405, (dcnt << 4) | (ccnt));
		WriteTW88(REG404, (ReadTW88(REG404) & 0x0F) | (winno << 4));
		WriteTW88(REG407, (dcnt << 4) | (ccnt));
		WriteTW88(REG406, (ReadTW88(REG406) & 0x0F) | (winno << 4));
	}
	else if(winno==1 || winno==2) {
		WriteTW88(REG407, (dcnt << 4) | (ccnt));
		WriteTW88(REG406, (ReadTW88(REG406) & 0x0F) | (winno << 4));
	}
	else
	{
		WriteTW88(REG405, (dcnt << 4) | (ccnt));
		WriteTW88(REG404, (ReadTW88(REG404) & 0x0F) | (winno << 4));
	}
}


//-----------------------------------------------------------------------------
/**
* Description
* 	disable RLC	Register
* @param winno
* 	win0 means disable both RLE_A and RLE_B.
* @return
*/
void SpiOsdDisableRlcReg(BYTE winno)
{
	WriteTW88Page(PAGE4_SOSD );
	if(winno==0) {
		WriteTW88(REG404, (ReadTW88(REG404) & 0x0F));
		WriteTW88(REG406, (ReadTW88(REG406) & 0x0F));
	}
	else if(winno==1 || winno==2)
		WriteTW88(REG406, (ReadTW88(REG406) & 0x0F));
	else
		WriteTW88(REG404, (ReadTW88(REG404) & 0x0F));
}

/*
BK130124..
struct SpiOsd_LutBuff_s {	//size 9
	BYTE	dirty;
	//WORD	size;
	BYTE	size;		// real size / 4. so it can support max...we can assume it as number of color.
	BYTE	bpp;
	WORD	lut_dest;
	//BYTE	dest;		// dest / 16 aligned value;	 usre REG431[4:0], REG44D[4:0]....
	DWORD	src;		//SpiFlash address
}
struct SpiOsd_WinBuff_s {	//8 ??
	BYTE	dirty;		//dirty flag for winbuffer.
	BYTE	*pWinBuff;	//buffer pointer   ==> link to SpiWinBuff[]
	SpiOsd_LutBuff_s	*pLUT;	//LUT info page.
};
struct SpiOsd_RlcBuff_s {	//4
	BYTE dirty;		//dirty flag
	BYTE win;
	BYTE data;
	BYTE count;
};
struct SpiOsd_Buff_s {
	struct SpiOsd_WinBuff_s win[9];
	struct SpiOsd_RlcBuff_s rlc[2];	//group B and Group A.
};

void init()
{
	BYTE i,j;
	for(i=0,j=0; i <= 8; i++) {
		pWin = SpiOsd->win[i];
		pWin->pBuff =  &SpiWinBuff[(i+j)*0x10];
		if(i==0)
			j++;
		pWin->dirty = 0;
		pWin->pLUT = ;
		if(!rlc)
			pWin->RLC = NULL;
		else {
			if(i==1 || i==2) pWin->RLC = SpiOsd->rlc[0];	//RLC_B;
			else             pWin->RLC = SpiOsd->rlc[1];	//RLC_A;
		}
	}	
}
void Update()
{
	1. win off
	2. RLC off
	3. LUT update+Alpha update
	4. RLC update
	5. WinBiff update
}
*/




#endif //..SUPPORT_SPIOSD






