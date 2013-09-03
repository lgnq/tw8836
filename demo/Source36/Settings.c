/**
 * @file
 * settings.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	setup system 
*/

//input			
//	CVBS	YIN0
//	SVIDEO	YIN1, CIN0
//	aRGB	G:YIN2 B:CIN0 R:VIN0 
//	aYUV	G:YIN2 B:CIN0 R:VIN0

#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "Global.h"
#include "CPU.h"
#include "Printf.h"
#include "util.h"
#include "Monitor.h"

#include "I2C.h"
#include "SPI.h"

#include "main.h"
#include "SOsd.h"
#include "FOsd.h"
#include "Scaler.h"
#include "decoder.h"
#include "InputCtrl.h"
#include "EEPROM.h"
#include "ImageCtrl.h"
#include "decoder.h"
#include "InputCtrl.h"
#include "OutputCtrl.h"
#include "Settings.h"
#include "measure.h"
//#include "SOsdMenu.h"


//#include "Data\DataInitPC.inc"
//#include "Data\DataInitMonitor.inc"


#if 0
void SW_Reset(void)
{
	DECLARE_LOCAL_page
	ReadTW88Page(page);
	WriteTW88Page(0);
	WriteTW88(REG006, ReadTW88(REG006) | 0x80);	//SW RESET
	WriteTW88Page(page);
}
#endif	


//=============================================================================
//	INPUT CLOCKS			                                               
//=============================================================================

//NeedClock ?
//LLPLL
//PCLKO
//-----------------------------------------------------------------------------
/**
* set High speed clock. only for test
*/
void ClockHigh(void)
{
#if 0
	dPrintf("\nHigh");	

	WriteTW88Page(PAGE0_SSPLL );
	WriteTW88(REG0F6, 0x00 );	// PCLK div by 1

	WriteTW88Page(PAGE2_SCALER );
	WriteTW88(REG20D, 0x81 );	// PCLKO div by 2

	WriteTW88Page(PAGE4_CLOCK);
	WriteTW88(REG4E1, 0xe0 );	// Source=PCLK, Delay=1, Edge=1
	ClkPllSetSpiInputClockLatch(3);	//just for test.

	SPI_SetReadModeByRegister(0x05);	// SPI mode QuadIO, Match DMA mode with SPI-read
#endif
}

//-----------------------------------------------------------------------------
/**
* set Low speed clock. only for test
*/
void ClockLow(void)
{
#if 0
	dPrintf("\nLow");	

	WriteTW88Page(PAGE0_SSPLL );
	WriteTW88(REG0F6, 0x00 );	// PCLK div by 1

	WriteTW88Page(PAGE2_SCALER );
	WriteTW88(REG20D, 0x80 );	// PCLKO div by 1

	WriteTW88Page(PAGE4_CLOCK);
	WriteTW88(REG4E1, 0x20 );	// Source=PCLK, Delay=0, Edge=0

	SPI_SetReadModeByRegister(0x05);	// SPI mode QuadIO, Match DMA mode with SPI-read
#endif
}

//-----------------------------------------------------------------------------
/**
* set 27MHz clock. only for test
*/
void Clock27(void)
{
#if 0
	dPrintf("\n27MHz");	

	WriteTW88Page(PAGE0_SSPLL );
	WriteTW88(REG0F6, 0x00 );	// PCLK div by 1

	WriteTW88Page(PAGE2_SCALER );
	WriteTW88(REG20D, 0x80 );	// PCLKO div by 1

	WriteTW88Page(PAGE4_CLOCK);
	WriteTW88(REG4E1, 0x00 );	// Source=27M

	SPI_SetReadModeByRegister(0x05);	// SPI mode QuadIO, Match DMA mode with SPI-read
#endif
}

//=========================================
// SSPLL
//=========================================

//-----------------------------------------------------------------------------
/**
* power up the SSPLL
*/
void SSPLL_PowerUp(BYTE fOn)
{
	WriteTW88Page(PAGE0_SSPLL);
	if (fOn)
		WriteTW88(REG0FC, ReadTW88(REG0FC) & ~0x80);
	else
		WriteTW88(REG0FC, ReadTW88(REG0FC) |  0x80);
}

//-----------------------------------------------------------------------------
/**
* get PPF(PLL Pixel Frequency) value. SSPLL value
*
* oldname:SspllGetPCLK
*
*	FPLL = REG(0x0f8[3:0],0x0f9[7:0],0x0fa[7:0])
*	POST = REG(0x0fd[7:6])
*	PLL Osc Freq = 108MHz * FPLL / 2^17 / 2^POST
*/
DWORD SspllGetPPF(void)
{
	DWORD ppf, FPLL;
	BYTE  i;

	//read PLL center frequency
	FPLL = SspllGetFreqReg();

	#ifdef DEBUG_PC
	dPrintf("\r\n(GetFBDN) :%ld", FPLL);
	#endif

	i= SspllGetPost();
	ppf = SspllFPLL2FREQ(FPLL, i);
// 	dPrintf("\r\n(GetPPF) :%ld", ppf);

	//test
	//FPLL = SspllFREQ2FPLL(ppf, i);
 	//dPrintf(" test FPLL :0x%lx", FPLL);

	return ppf;
}

//desc
// PLL = 108MHz *FPLL / 2^17
// FPLL = PLL * 2^17 / 108MHz
//		= PLL * 131072 / 108MHz
//		= PLL * 131072 / (108 * 100000 * 10)Hz
//      = (PLL / 100000) * (131072 / 108)* (1/10)
//      = (PLL / 100000) * (1213.6296) * (1/10)
//      = (PLL / 100000) * (1213.6296 *2 ) * (1/10*2) 
//      = (PLL / 100000) * (2427.2592) / 20 
//      = (PLL / 100000) * (2427) / 20 
//@param
//	_PPF: input pixel clock
//oldname:ChangeInternPLL@TW8816
//nickname: SetPclk

//		#define REG0F8_FPLL0			0xF8
//		#define REG0F9_FPLL1			0xF9
//		#define REG0FA_FPLL2			0xFA
//		#define REG0FD_SSPLL_ANALOG	0xFD

//-----------------------------------------------------------------------------
/**
* set SSPLL register value
*/
void SspllSetFreqReg(DWORD FPLL)
{
	dPrintf("\nSspllSetFreqReg(%lx)",FPLL);
	WriteTW88Page(PAGE0_SSPLL);
	WriteTW88(REG0FA_FPLL2, (BYTE)FPLL );
	WriteTW88(REG0F9_FPLL1, (BYTE)(FPLL>>8));
	WriteTW88(REG0F8_FPLL0, (ReadTW88(REG0F8_FPLL0)&0xF0) | (FPLL>>16));
}
//-----------------------------------------------------------------------------
/**
* get SSPLL register value
*
*	register
*	R0F8[3:0]	FPLL[19:16]
*	R0F9[7:0]	FPLL[15:8]
*	R0FA[7:0]	FPLL[7:0]
*/
DWORD SspllGetFreqReg(void)
{
	DWORD dFPLL;
	
	WriteTW88Page(PAGE0_SSPLL);
	dFPLL = ReadTW88(REG0F8_FPLL0)&0x0F;
	dFPLL <<=8;
	dFPLL |= ReadTW88(REG0F9_FPLL1);
	dFPLL <<=8;
	dFPLL |= ReadTW88(REG0FA_FPLL2);

	return dFPLL;
}

//-----------------------------------------------------------------------------
/**
* set SSPLL AnalogControl register
*
*	register
*	R0FD[7:6] POST
*	R0FD[5:4] VCO
*	R0FD[2:0] ChargePump
*/
void SspllSetAnalogControl(BYTE value)
{
	WriteTW88Page(PAGE0_SSPLL);
	WriteTW88(REG0FD_SSPLL_ANALOG, value );
}

//-----------------------------------------------------------------------------
/**
* get SSPLL Post value
*/
BYTE SspllGetPost(void)
{
	BYTE post;

	WriteTW88Page(PAGE0_SSPLL);
	post = ReadTW88(REG0FD);
	return ((post>>6) & 0x03);
}

/*
	PLL Osc Freq = 108MHz * FPLL / 2^17 / 2^POST
	FREQ			= 27000000 * 4 * FPLL / 2^17  / 2^POST
    FPLL 			= FREQ *((2^15) * (2^POST)) / 27000000 			   			
    FPLL 			= (FREQ / 1000) *((2^15) * (2^POST)) / 27000 			   			
    FPLL 			= (FREQ / 1000) *((2^12) * (2^POST)) * (2^3  / 27000)
    FPLL 			= (FREQ / 1000) *((2^12) * (2^POST)) / (3375) 			   			
*/
#if 1 //GOOD
//-----------------------------------------------------------------------------
/**
* get FPLL value from freq
*/
DWORD SspllFREQ2FPLL(DWORD FREQ, BYTE POST)
{
	DWORD FPLL;
	FPLL = FREQ/1000L;
	FPLL <<= POST;
	FPLL <<= 12;
	FPLL = FPLL / 3375L;
	return FPLL;
}
#endif
#if 0	//BKFYI:example
/*
    FPLL 			= FREQ *((2^15) * (2^POST)) / 27000000 			   			
	FPLL 			= FREQ *(32768 * (2^POST)) / 27000000
	FPLL 			= FREQ *(512 * (2^POST)) / 421875
    FPLL            = FREQ * 16 / 421875 * 32 * (2^POST)
*/
//DWORD SspllFREQ2FPLL(DWORD FREQ, BYTE POST)
//{
//	DWORD FPLL;
//	FPLL = FREQ * 16; 
//	FPLL /= 421875;
//	FPLL *= 32;
//	FPLL <<= POST;
//	return FPLL;
//}
#endif

#if 0 //BKFYI: example 
/*
	FREQ			= 27000000 * 4 * FPLL / 2^17  / 2^POST
    Simpilfied FREQ	= 824L * FPLL * 2^POST
*/
//DWORD SspllFPLL2FREQ(DWORD FPLL, BYTE POST)
//{
//	DWORD FREQ;
//	FREQ = FPLL * 824L;
//	FREQ >>= POST;
//	return FREQ;
//}
#endif

/*
    FREQ 			= 27000000 * FPLL / ( (2^15) * (2^POST) )
    FREQ 			= 27000000 * FPLL / ( (2^15) * (2^POST) )
					= 421875 * 64 * FPLL / (64 * 2^9 *(2^POST))
					= 421875 * FPLL / (512 *(2^POST))
	   				= FPLL / 64 * 421875 / 8 / (2^POST)
*/
//-----------------------------------------------------------------------------
/**
* get freq from FPLL
*/
DWORD SspllFPLL2FREQ(DWORD FPLL, BYTE POST)
{
	DWORD FREQ;
	FREQ = FPLL / 64;
	FREQ *= 421875;
	FREQ /= 8;
	FREQ >>= POST;
	return FREQ;
}


//SSPLL Set Frequency And PLL
//R0F8
//R0F9
//R0FA
//R0FD
//R0FC
//R20D[4]	Pixel clock polarity control
//R20D[1:0]	Pixel clock output frequency division control
//
//if PPF is 108M, POST=0. VCO:3 CURR=4

//-----------------------------------------------------------------------------
/**
* set SSPLL freq and Pll
*/
void SspllSetFreqAndPll(DWORD _PPF)
{
	BYTE	ppf, CURR, VCO, POST;
	DWORD	FPLL;
	
	dPrintf("\nSspllSetFreqAndPll(%ld)",_PPF);
	ppf = _PPF/1000000L;		//base:1MHz

	//----- Frequency Range --------------------
	if     ( ppf < 27 )  { VCO=2; CURR=0; POST=2; }		// step = 0.5MHz
	else if( ppf < 54 )  { VCO=2; CURR=1; POST=1; }		// step = 1.0MHz
	else if( ppf < 108 ) { VCO=2; CURR=2; POST=0; }		// step = 1.0MHz
	else                 { VCO=3; CURR=3; POST=0; }		// step = 1.0MHz

	CURR = VCO+1;	//BK110721. Harry Suggest.

	//----- Get FBDN
	FPLL = SspllFREQ2FPLL(_PPF, POST);

	//----- Setting Registers
	SspllSetFreqReg(FPLL);
	SspllSetAnalogControl((VCO<<4) | (POST<<6) | CURR);

	dPrintf("\nPOST:%bx VCO:%bx CURR:%bx",POST, VCO, CURR);

	//adjust pclk divider
	if(ppf >=150) {
		ppf /= 2;
		PclkSetDividerReg(1);	//div2
	}
	else {
		PclkSetDividerReg(0);	//div1:default
	}

	//adjust pclko divider. see SetDefaultPClock()
#if defined(PANEL_AUO_B133EW01)
//	PclkoSetDiv( (ppf+PANEL_PCLK_TYP -1) / PANEL_PCLK_TYP - 1);
//  BKTODO: I need 1.5 divider.
#else
	PclkoSetDiv( (ppf+39) / 40 - 1); //pixel clock polarity : Invert 0:div1, 1:div2, 2:div3
										//BKTODO:move pixel clock polarity...	
#endif
	PclkSetPolarity(1);	//invert

}

#if 0
void SspllSetFreqAndPllAndDiv(DWORD _PPF, BYTE div)
{
	BYTE	ppf, CURR, VCO, POST;
	DWORD	FPLL;
	BYTE value;
	BYTE I2C_Buffer[8];
	
	dPrintf("\nSspllSetFreqAndPll(%ld)",_PPF*div);
	ppf = _PPF/1000000L;		//base:1MHz
	//ppf *= div;

	//----- Frequency Range --------------------
	if     ( ppf < 27 )  { VCO=2; CURR=0; POST=2; }		// step = 0.5MHz
	else if( ppf < 54 )  { VCO=2; CURR=1; POST=1; }		// step = 1.0MHz
	else if( ppf < 108 ) { VCO=2; CURR=2; POST=0; }		// step = 1.0MHz
	else                 { VCO=3; CURR=3; POST=0; }		// step = 1.0MHz
	CURR = VCO+1;	//BK110721. Harry Suggest.

	//----- Get FPLL
	// see, FPLL = SspllFREQ2FPLL(_PPF, POST);
	//--------------------------------------------------------
	//	PLL Osc Freq = 108MHz * FPLL / 2^17 / 2^POST
	//	FREQ			= 27000000 * 4 * FPLL / 2^17  / 2^POST
    //	FPLL 			= FREQ *((2^15) * (2^POST)) / 27000000 			   			
    //	FPLL 			= (FREQ / 1000) *((2^15) * (2^POST)) / 27000 			   			
    //	FPLL 			= (FREQ / 1000) *((2^12) * (2^POST)) * (2^3  / 27000)
    //	FPLL 			= (FREQ / 1000) *((2^12) * (2^POST)) / (3375) 			   			
	//--------------------------------------------------------
	FPLL = _PPF / 1000L;
	FPLL <<= POST;
	FPLL <<= 12;
	FPLL = FPLL / 3375L;

	//----- Setting SSPLL Frequency and analog control Registers.
	//see, SspllSetFreqReg() and SspllSetAnalogControl() on TW8835.
	WriteTW88Page(PAGE0_SSPLL);
	I2C_Buffer[0]  = ReadTW88(REG0F8_FPLL0)&0xF0;
	I2C_Buffer[0] |= (BYTE)(FPLL>>16);
	I2C_Buffer[1]  = (BYTE)(FPLL>>8);
	I2C_Buffer[2]  = (BYTE)FPLL;
	//WriteTW88Block(REG0F8, I2C_Buffer, 3);
	WriteTW88(REG0F8,   I2C_Buffer[0]);
	WriteTW88(REG0F8+1, I2C_Buffer[0+1]);
	WriteTW88(REG0F8+2, I2C_Buffer[0+2]);

	WriteTW88(REG0FD_SSPLL_ANALOG, (VCO<<4) | (POST<<6) | CURR );
	dPrintf("\nPOST:%bx VCO:%bx CURR:%bx",POST, VCO, CURR);

	//--------------------------------
	//adjust PCLKO divier and polarity
	//--------------------------------
	//PCLKO divider value = (ppf / 45). Previous used ((ppf+39)/40-1).
	//PCLK polarity. set "1". Why ? It needs to verify.
	WriteTW88Page(PAGE2_SCALER);
	value = ReadTW88(REG20D);
#if 0
	value &= ~0x13;
	value |= ((ppf / 45));		//Pclko09SetDiv( ppf / 45 ).
#else
	value |= (div-1);
#endif
	value |= 0x10;				//Pclk09SetPolarity(1). Invert.
	WriteTW88(REG20D, value);
}
#endif

//=========================================
// PCLK
//=========================================

//-----------------------------------------------------------------------------
/**
* set PCLK divider
*/
void PclkSetDividerReg(BYTE divider)
{
	WriteTW88Page(0);
	WriteTW88(REG0F6, (ReadTW88(REG0F6) & 0xF8) | divider);
}

//-----------------------------------------------------------------------------
/**
* get PCLK frequency
*/
DWORD PclkGetFreq(DWORD sspll)
{
	BYTE divider;
	DWORD temp32;

	WriteTW88Page(0);
	divider = ReadTW88(REG0F6) & 0x03;

	switch(divider) {
	case 0:	temp32 = sspll;			break;
	case 1:	temp32 = sspll / 2;		break;
	case 2:	temp32 = sspll / 4;		break;
	case 3:	temp32 = sspll / 8;		break;
	}
	return temp32;
}
//-----------------------------------------------------------------------------
/**
* get PCLKO frequency
*/
DWORD PclkoGetFreq(DWORD pclk)
{
	BYTE divider;
	DWORD temp32;
	BYTE bTemp;

	WriteTW88Page(2);
	bTemp = ReadTW88(REG20D);
	divider = ReadTW88(REG20D) & 0x03;

	divider++;
	temp32 = pclk / divider;
	if(divider==3) {
		if(bTemp & 0x20)
			temp32 <<= 1;
	}
	return temp32;
}

//-----------------------------------------------------------------------------
/**
* set PCLKO divider and CLK polarity
*
*	R20D[4]	Pixel clock polarity control.
*	R20D[5]		divider for 1.5 (Only work with div 3.
*	R20D[1:0]	Pixel clock output frequency division control.

* @param div - Pixel clock output frequency division
*	0:div 1,	1:div 2,	2:div 3,	3:div 4.
*	4:div 1,	5:div 2,	6:div 1.5,	7:div 4.
*
*/
void PclkoSetDiv(/*BYTE pol,*/ BYTE div)
{
	BYTE value;
	WriteTW88Page(PAGE2_SCALER);
	value = ReadTW88(REG20D) & 0xDC;
	if(div==6)
		value |= 0x20;
	div &= 0x03;
	value |= div;

	WriteTW88(REG20D, value);
}
//-----------------------------------------------------------------------------
/**
* set PCLK polarity
*
* @param pol - Pixel clock output polarity
*	-0:	no inversion
*	-1:	inversion
*	- 0xFF: do not change it. Use previous value
*/
void PclkSetPolarity(BYTE pol)
{
	BYTE value;
	WriteTW88Page(PAGE2_SCALER);
	value = ReadTW88(REG20D);
	if(pol)	value |=  0x10;
	else	value &= ~0x10;
	WriteTW88(REG20D, value);
}


//=========================================
// CLKPLL
//=========================================

//-----------------------------------------------------------------------------
/**
* select ClkPLL input
*/
void ClkPllSetSelectReg(BYTE ClkPllSel)
{
	WriteTW88Page(4);
	if(ClkPllSel) WriteTW88(REG4E0, ReadTW88(REG4E0) |  0x01);
	else		  WriteTW88(REG4E0, ReadTW88(REG4E0) & ~0x01);
}

//-----------------------------------------------------------------------------
/**
* Set SPI input clock latch.
* To support a high speed SPI clock, it needs a positive edge & 1 Cycle Delay.
*@param property
*	00b - TW8836 default.
*   01b - one cycle delay.
*   10b - positive edge.(SPIFLASH spec).
*   11b - positive edge, one cycle delay. for HighSpeed.
*
*	*-----+		+-----+		+-----+		+-----+
*	|	  |		|	  |		|	  |		|	  |
*   +	  +-----+	  +-----+	  +-----+	  +-----
*	        -----------	----------- -----------
*	       X    (A)    X    (B)	   X    (C)    X    SPI Flash SPEC.		  
*           -----------	-----------	-----------
*                *          *                    
*                |			|					    case 10b read position.		  27M~36MHz success. 54MHz~108MHz fail
*																				  PCLK 54MHz Fail
*					  *           *												  
*                     |			  |                 case 00b read position.		  27M~72MHz success. 108MHz fail
*                                                                                 PCLK 108MHz working.~ 109Mhz.  109,265KHz failed.           
*
* <<assume the response is slow on the high speed clock>>
*	                ----------- -----------
*	        XXXXXXXX   (A)     X	 (B)     X					  
*                   ----------- -----------									  
*					        *           * 
*					        |			|           case 11b read position        27M~72MHz Fail. 108MHz success. 133M success(2,118,132)
*                                                                                 135MHz fail. I saw system reboot.
*
*							      *           *
*							      |           |     case 01b read position        always fail.
*
* To support 133MHz, 
* 	Select 27MHz.
*	Set SSPLL as  0x02,0x00,0x00.(108MHz)
*   Select PCLK with div 1.5.
*
*	Select CLKPLL, div1.0 + SpiInputClockLatch as 11b.(write 4E1=0xE0);
*	Set SSPLL as  0x02,0x76,0x84.
*
* To move back 27MHz
*	Set SSPLL as  0x02,0x00,0x00.(108MHz)
*	Select CLKPLL, div1.5 + SpiInputClockLatch as 00b.(write 4E1=0x21);
*	Select 27MHz(write 4E1=0x01).
*	
*/



#define SPICLOCK_LOW	0
#define SPICLOCK_HIGH	3
#if 0
void ClkPllSetSpiInputClockLatch(BYTE property)
{
	BYTE bTemp;
	bTemp = ReadTW88(REG4E1) & 0x3F;
	WriteTW88(REG4E1, bTemp | (property << 6));	
}
#endif

//-----------------------------------------------------------------------------
/**
* set ClkPLL divider
*/
void ClkPllSetDividerReg(BYTE divider)
{
	WriteTW88Page(4);
	WriteTW88(REG4E1, (ReadTW88(REG4E1) & ~0x07) | divider);	//CLKPLL Divider
}

//-----------------------------------------------------------------------------
/**
* set ClkPLL input and ClkPLL divider
* 
* only from monitor
*/
void ClkPllSetSelDiv(BYTE ClkPllSel, BYTE ClkPllDiv)
{
	BYTE mcu_sel;
	DWORD clkpll,spi_clk;
	BYTE i=0;

	//check & move MCU CLK source to 27M 
	mcu_sel = McuSpiClkReadSelectReg();
	if(mcu_sel==MCUSPI_CLK_PCLKPLL) {
		McuSpiClkSelect(MCUSPI_CLK_27M);
		I2C_delay_base = 1;	
	}
	//
	//Now, MCU uses 27M or 32K. You can change CLKPLL register without a system hang
	//

	ClkPllSetSelectReg(ClkPllSel);
	do {
		ClkPllSetDividerReg(ClkPllDiv);
		ClkPllDiv++;
		clkpll =ClkPllGetFreq();
		spi_clk=SpiClkGetFreq(clkpll);
		i++;
	} while(spi_clk > 75000000L);	//MAX SPICLK
	if(i!=1)
		ePrintf("\nClkPllSetSelDiv div encreased:%d",i-1);

	//restore MCU CLK source
	if(mcu_sel==MCUSPI_CLK_PCLKPLL)
		McuSpiClkSelect(MCUSPI_CLK_PCLKPLL);	
}

//-----------------------------------------------------------------------------
/**
* get ClkPLL frequency
*/
DWORD ClkPllGetFreq(void)
{
	BYTE temp8;
	DWORD clkpll;
	DWORD temp32;

	WriteTW88Page(4);
	temp8 = ReadTW88(REG4E0) & 0x01;

	if(temp8==0) {
		temp32 = SspllGetPPF();
		clkpll = PclkGetFreq(temp32);
	}
	else {
		clkpll=108000000L;
	}
	WriteTW88Page(4);
	temp8 = ReadTW88(REG4E1) & 0x07;

	switch(temp8) {
	case 0:	temp32 = clkpll;		break;
	case 1:	temp32 = clkpll*2/3;	break;
	case 2:	temp32 = clkpll*2;		break;
	case 3:	temp32 = clkpll*2/5;	break;
	case 4:	temp32 = clkpll*3;		break;
	case 5:	temp32 = clkpll*2/7;	break;
	case 6:	temp32 = clkpll*4;		break;
	case 7:	temp32 = clkpll*5;		break;
	}
	return temp32;
}

//=========================================
// InMux (Input Mux)
//=========================================
//-----------------------------------------------------------------------------
/**
* set input mux format
*
* register
*	R102 - input format.
*	R105.
*	R106.
* @param InputMode
*/
//-----------------------------------------------------------------------------
/**
* set input mux format for AnalogModule(DEC and aRGB)
*
* register
*	R102 - input format.
*	R105.
*	R106.
* @param InputMode
*/
#ifdef MODEL_TW8835
void AMuxSetInput(BYTE InputMode)
{
	BYTE r102, r105, r106;
	WriteTW88Page(PAGE1_DECODER );

	r105 = ReadTW88(REG105) & 0xF0;
	r106 = ReadTW88(REG106) & ~0x03;	//Do not change Y.

	switch(InputMode) {
	case INPUT_CVBS:
		r102 = 0x40;		// 0x40 - FC27:27MHz, IFSEL:Composite, YSEL:YIN0 
		r105 |= 0x0F;		//decoder mode
		r106 |= 0x03;		// C,V adc in Power Down.
		break;
	case INPUT_SVIDEO:
		r102 = 0x54;		// 0x54	- FC27:27MHz, IFSEL:S-Video, YSEL:YIN1, CSEL:CIN0 
		r105 |= 0x0F;		//decoder mode
		r106 |= 0x01;		// V in PowerDown
		break;
	case INPUT_COMP:	//target r102:4A,r105:04 r016:00
						//     ->     4A      00      00		  

		r102 = 0x4A ;		// 0x4A - 	FC27:27MHz, 
							//		  	IFSEL:Composite, We are using aRGB. So composite is a correct value 
							//			YSEL:YIN2, CSEL:CIN1, VSEL:VIN0
		//r105 |= 0x04;		//??? ? someone overwrite as 00. R105[2]=0b is a correct
		//r106 				//C & V adc in normal(not Power Down)
		break;
	case INPUT_PC:	//target r102:4A r105:04 r106:00
		r102 = 0x4A;		// 0x4A - 	FC27:27MHz, 
							//		  	IFSEL:Composite, We are using aRGB. So composite is a correct value 
							//			YSEL:YIN2, CSEL:CIN1, VSEL:VIN0

		//r105 = 			//RGB mode
							//?? I think R105[2] have to be 0. not 1b.
		//r106 				//C & V adc in normal(not Power Down)
		break;
	case INPUT_DVI:			//target ? don't care
	case INPUT_HDMIPC:
	case INPUT_HDMITV:
	case INPUT_LVDS:
		//digital. don't care.
		r102 = 0x00;
		break;
	case INPUT_BT656:
		r102 = 0x40;
		break;
	}
	if(r102) {	//need update?
		WriteTW88(REG102, r102 );
		WriteTW88(REG105, r105 );
		WriteTW88(REG106, r106 );
	}
}
#else
/*
Set Analog Mux.
OLD R105 moved to R1E8.
*/
void AMuxSetInput(BYTE InputMode)
{
	BYTE r102, r106, r1E8;
	
	WriteTW88Page(PAGE1_DECODER);

	r1E8 = ReadTW88(REG1E8) & 0xF0;
	r106 = ReadTW88(REG106) & ~0x03;	//Do not change Y.

	switch (InputMode)
	{
	case INPUT_CVBS:
		r102 = 0x40;		// 0x40 - FC27:27MHz, IFSEL:Composite, YSEL:YIN0 
		r1E8 |= 0x0F;		//decoder mode
		r106 |= 0x03;		// C,V adc in Power Down.
		break;
	case INPUT_SVIDEO:
		r102 = 0x58;		// 0x54	- FC27:27MHz, IFSEL:S-Video, YSEL:YIN2, CSEL:CIN0 
		r1E8 |= 0x0F;		//decoder mode
		r106 |= 0x01;		// V in PowerDown
		break;
	case INPUT_COMP:	//target r102:4A,r105:04 r016:00
						//     ->     4A      00      00		  

#ifdef MODEL_TW8835
		r102 = 0x4A ;		// 0x4A - 	FC27:27MHz, 
							//		  	IFSEL:Composite, We are using aRGB. So composite is a correct value 
							//			YSEL:YIN2, CSEL:CIN1, VSEL:VIN0
		//r105 |= 0x04;		//??? ? someone overwrite as 00. R105[2]=0b is a correct
		//r106 				//C & V adc in normal(not Power Down)
#else
		r102 = 0x4E ;		// TW8836 EVB10 use Y3.
		r1E8 |= 0x20;		// disable YOUT. it shares pin with Y3.
#endif

		break;
	case INPUT_PC:	//target r102:4A r105:04 r106:00
#ifdef MODEL_TW8835
		r102 = 0x4A;		// 0x4A - 	FC27:27MHz, 
							//		  	IFSEL:Composite, We are using aRGB. So composite is a correct value 
							//			YSEL:YIN2, CSEL:CIN1, VSEL:VIN0

		//r105 = 			//RGB mode
							//?? I think R105[2] have to be 0. not 1b.
		//r106 				//C & V adc in normal(not Power Down)
#else
		r102 = 0x4E ;		// TW8836 EVB10 use Y3.
		r1E8 |= 0x20;		// disable YOUT. it shares pin with Y3.
#endif
		break;
	case INPUT_DVI:			//target ? don't care
	case INPUT_HDMIPC:
	case INPUT_HDMITV:
	case INPUT_LVDS:
		//digital. don't care.
		r102 = 0x00;
		break;
	case INPUT_BT656:
		r102 = 0x40;		// 0x40 - FC27:27MHz, IFSEL:Composite, YSEL:YIN0 
		//r1E8 |= 0x01;		//decoder mode
		r1E8 |= 0x0F;		//decoder mode
		r106 |= 0x03;		// C,V adc in Power Down.
		break;
	}
	
	if (r102) 	//need update?
	{
		WriteTW88(REG102, r102);
		WriteTW88(REG1E8, r1E8);
		WriteTW88(REG106, r106);
	}
}
#endif

//=========================================
// MCPSPI
//=========================================
//-----------------------------------
// McuSpiClkToPclk & McuSpiClkRestore
//-----------------------------------

/*
result  register        stepA	stepB	stepC	stepD	stepE
------	--------		-----	-----	-----	-----	-----
fail	PCLK_SEL  		PLL				 		PCLK
		MCUSPI_CLK_SEL		PCLK	27M						PCLK
		MCUSPI_CLK_DIV		1		   		0				1

OK		PCLK_SEL  		PLL				 		PCLK
		MCUSPI_CLK_SEL		PCLK	27M						PCLK
		MCUSPI_CLK_DIV		1		   		 				1

OK		PCLK_SEL  		PLL				 		PCLK
		MCUSPI_CLK_SEL		PCLK
		MCUSPI_CLK_DIV		1
---------------------------------------------------------------
*/
BYTE shadow_r4e0;
BYTE shadow_r4e1;

//-----------------------------------------------------------------------------
/**
* select PCLK for McuSpi
*
* NOTE: SPIDMA needs a PCLK source
* @see McuSpiClkRestore
*/
BYTE McuSpiClkToPclk(BYTE divider)
{
#if defined(PANEL_AUO_B133EW01)
	BYTE temp = divider;
#endif

	WriteTW88Page(PAGE4_CLOCK);
	shadow_r4e0 = ReadTW88(REG4E0);
	shadow_r4e1 = ReadTW88(REG4E1);

	//test	BK121221
	//WriteTW88(REG0F6, ReadTW88(REG0F6) | 0x01); 

#if defined(PANEL_AUO_B133EW01)
	//BKFYI:130225.
	//I will use SSPLL with 150Mhz.
	//MCUSPI clk use a 150Mhz / 3 = 50MHz
	//
	WriteTW88(REG4E1, 0x24);
	WriteTW88(REG4E0, ReadTW88(REG4E0) & 0xFE);	  
#else
	WriteTW88(REG4E0, shadow_r4e0 & 0xFE);	//select PCLK.
	WriteTW88(REG4E1, 0x20 | divider);		//CLKPLL + divider.
#endif

	return 0;
}

//-----------------------------------------------------------------------------
/**
* restore MCUSPI clock
*
* @see McuSpiClkToPclk
*/

void McuSpiClkRestore(void)
{
	//Printf("\nMcuSpiClkRestore REG4E0:%bx REG4E1:%bx",shadow_r4e0,shadow_r4e1);
//PORT_DEBUG = 0;
 	WriteTW88Page(PAGE4_CLOCK);
	WriteTW88(REG4E0, shadow_r4e0);
	WriteTW88(REG4E1, shadow_r4e1);
//PORT_DEBUG = 1;
	//Printf("-Done");

	//test	BK121221
	//WriteTW88(REG0F6, ReadTW88(REG0F6) & 0xF8); 
}

//-----------------------------------------------------------------------------
/**
* read MCUSPI clock mode
*/
BYTE McuSpiClkReadSelectReg(void)
{
	BYTE value;
	
	WriteTW88Page(PAGE4_CLOCK);
	value = ReadTW88(REG4E1) & 0x30;
	
	return (value >> 4);
}

//-----------------------------------------------------------------------------
/**
* Select McuSpi clock source
*
*	register
*	R4E1[5:4]
* @param McuSpiClkSel clock source
*	-0:27MHz
*	-1:32KHz
*	-2:CLKPLL. << (PLL Divider) << (PCLK or 108M) << (if PCLK, SSPLL with PCLK Divider)
*/
void McuSpiClkSelect(BYTE McuSpiClkSel)
{
#ifdef PANEL_AUO_B133EW01
	//I will use SSPLL. Do not change MCU clock.
	BYTE value=McuSpiClkSel;
#else
	BYTE value;
	WriteTW88Page(PAGE4_CLOCK);
	value = ReadTW88(REG4E1) & 0x0F;
	WriteTW88(REG4E1, (McuSpiClkSel << 4) | value);
#endif
} 

#if 0
//OLD code
//!void McuSpiClkSet(BYTE McuSpiClkSel, BYTE ClkPllSel, BYTE ClkPllDiv) 
//!{
//!	WriteTW88Page(PAGE4_CLOCK);
//!	if(ClkPllSel)	WriteTW88(REG4E0, ReadTW88(REG4E0) |  0x01);
//!	else			WriteTW88(REG4E0, ReadTW88(REG4E0) & ~0x01); 
//!	WriteTW88(REG4E1, (McuSpiClkSel << 4) | ClkPllDiv);
//!}
#endif

//-----------------------------------------------------------------------------
/**
* get MCU clock frequency
*/
DWORD McuGetClkFreq(void)
{
	BYTE temp8;
	DWORD temp32;

	WriteTW88Page(4);
	temp8 = ReadTW88(REG4E1) >> 4;
	temp8 &= 0x03;

	switch (temp8)
	{
	case 0: temp32 = 27000000L;				break;
	case 1:	temp32 = 32000L;				break;
	case 2:	temp32 = ClkPllGetFreq();		break;
	default: //unknown
			temp32 = 27000000L;				break;
	}
	
	return temp32;
}

//=========================================
// SPI CLK
//=========================================

//-----------------------------------------------------------------------------
/**
* get SPI clock frequency
*/
DWORD SpiClkGetFreq(DWORD mcu_clk)
{
	BYTE divider;
	DWORD temp32;
	WriteTW88Page(0);
	divider = ReadTW88(REG0F6) >> 4;
	divider++;
	temp32 = mcu_clk / divider;
	return temp32;
}

//-----------------------------------------------------------------------------
/**
* set LLPLL clock source
*
* use 27M OSD or PLL 
*/
void LLPLLSetClockSource(BYTE use_27M)
{
	WriteTW88Page(PAGE1_LLPLL);
	if (use_27M)
		WriteTW88(REG1C0, ReadTW88(REG1C0) | 0x01); 
	else
		WriteTW88(REG1C0, ReadTW88(REG1C0) & ~0x01);
}

/*
example
#if 0
	WriteTW88Page(PAGE4_CLOCK);
	value = ReadTW88(REG4E1);
	WriteTW88(REG4E1, value & ~0x30);	//select 27M first
	WriteTW88(REG4E0, ReadTW88(REG4E0) | 0x01);		//select 108M PLL clock.
	WriteTW88(REG4E1, (value & ~0x30) | 0x01);			//SPI clock Source=PCLK, Delay=1, Edge=0, PLL(72M) divider:1.5
	WriteTW88(REG4E1, 0x21);	 					//SPI clock Source=PCLK, Delay=1, Edge=0, PLL(72M) divider:1.5
#endif
	do {
		BYTE r4e0,r4e1;
		WriteTW88Page(PAGE4_CLOCK);
		r4e0 = ReadTW88(REG4E0);
		r4e1 = ReadTW88(REG4E1);
		WriteTW88(REG4E1, r4e1 & 0x0F);	//27M
		delay1ms(10);
		WriteTW88(REG4E1, 0x00);			//27M with div1.0
		WriteTW88(REG4E0, r4e0 | 0x01);
		WriteTW88(REG4E1, 0x21);			
		delay1ms(10);
// 		Printf("\nline:%d",__LINE__);
	} while(0);
	//
*/



//-----------------------------------------------------------------------------
//set default SSPLL clock
#if 0
void SetDefaultPClock(void)
{
	WriteTW88Page(PAGE0_SSPLL);
	WriteTW88(REG0F9, 0x50);	  	//SSPLL 70MKz. 0x50:70MHz, 0x3C:66.6MHz..Pls use SspllSetFreqReg(0x015000);
	WriteTW88(REG0F6, 0x00); 		//PCLK div:1. SPI CLK div:1

	WriteTW88Page(PAGE2_SCALER);
	WriteTW88(REG20D, 0x81 );	// PCLKO div by 2. ??Polarity
}
#endif

//-----------------------------------------------------------------------------
/**
* print current clock info
*/
void DumpClock(void)
{
	DWORD ppf, FPLL;
	BYTE  i;
	DWORD pclk, pclko;
	DWORD clkpll, mcu_clk, spi_clk;

	//read PLL center frequency
	FPLL = SspllGetFreqReg();
	i    = SspllGetPost();
	ppf  = SspllFPLL2FREQ(FPLL, i);

	pclk  = PclkGetFreq(ppf);
	pclko = PclkoGetFreq(pclk);

	clkpll = ClkPllGetFreq();

	mcu_clk = McuGetClkFreq();
	spi_clk = SpiClkGetFreq(mcu_clk);

	dPrintf("\nCLOCK SSPLL:%lx POST:%bx FREQ:%ld", FPLL, i, ppf);
	dPrintf("\n      PCLK:%ld PCLKO:%ld", pclk, pclko);
	dPrintf("\n      CLKPLL:%ld", clkpll);
	dPrintf("\n      MCU:%ld SPI:%ld", mcu_clk, spi_clk);
}

//=============================================================================
//				                                               
//=============================================================================
//global


/*
//==========================
// GPIO EXAMPLE
//==========================
//!GPIO_EN	Enable(active high)
//!GPIO_OE	Output Enable(active high)
//!GPIO_OD	Output Data
//!GPIO_ID	Input Data
//!
//!		GPIO_EN	GPIO_OE	GPIO_OD	GPIO_ID
//!GPIO0	R080	R088	R090	R098
//!GPIO1	R081	R089	R091	R099
//!GPIO2	R082	R08A	R092	R09A
//!GPIO3	R083	R08B	R093	R09B
//!GPIO4	R084	R08C	R094	R09C
//!GPIO6	R085	R08D	R095	R09D
//!GPIO7	R086	R08E	R096	R09E
//!
//!bit readGpioInputData(BYTE gpio, BYTE b)
//!{
//!	BYTE reg;
//!	reg = 0x98+gpio;
//!	value = ReadTW88(reg);
//!	if(value & (1<<b))	return 1;
//!	else				return 0;
//!}
//!bit readGpioOutputData(BYTE gpio, BYTE b)
//!{
//!	BYTE reg;
//!	reg = 0x90+gpio;
//!	value = ReadTW88(reg);
//!	if(value & (1<<b))	return 1;
//!	else				return 0;
//!}
//!void writeGpioOutputData(BYTE gpio, BYTE b, BYTE fOnOff)
//!{
//!	BYTE reg;
//!	reg = 0x90+gpio;
//!	value = ReadTW88(reg);
//!	if(fOnOff) value |= (1<<b);
//!	else       value &= ~(1<<b);
//!	WriteTW88(reg,value);
//!}
*/
	


//
//BKFYI110909.
//	We merge step0 and step1, and check the status only at step2.
//	
//	step0 check_status : OK
//	step1 check_status : OK
//	step2 check_status : OK
//
//  step0 & step1 check_status : fail 20%
//	step2         check_status : OK
//-------------------------------------- 

//-----------------------------------------------------------------------------
/**
* turn on DCDC
*/
#if 0
BYTE DCDC_On(BYTE step)
{
	BYTE i;

//	dPrintf("\nDCDC_On(%bx)",step);

	//-------------
	//DCDC ON
	WriteTW88Page(PAGE0_DCDC);
	switch(step) {
	case 0:
#if 0
		WriteTW88(REG0E8, 0x72);	//default. & disable OverVoltage
		WriteTW88(REG0E8, 0x12);	//disable OverCurrent, disable UnderCurrent
		WriteTW88(REG0E8, 0x13);	//enable DC convert digital block
#endif
		WriteTW88(REG0E8, 0xF2);	//Printf("\nREG0E8:F2[%bd]",ReadTW88(REG0EA)>>4);
		WriteTW88(REG0E8, 0x02);	//Printf("\nREG0E8:02[%bd]",ReadTW88(REG0EA)>>4);
		WriteTW88(REG0E8, 0x03);	//Printf("\nREG0E8:03[%bd]",ReadTW88(REG0EA)>>4);
		WriteTW88(REG0E8, 0x01);	//Printf("\nREG0E8:01[%bd]",ReadTW88(REG0EA)>>4);
		break;
	case 1:
		WriteTW88(REG0E8, 0x11);	//powerup DC sense block
		break;
	case 2:
		WriteTW88(REG0E8, 0x71);	//turn on under current feedback control
									//0x11->0x51->0x71
		break;
	//default:
	//	ePuts("\nBUG");
	//	return;
	}
	for(i=0; i < 10; i++) {
		if((ReadTW88(REG0EA) & 0x30)==0x30) {
			//dPrintf("\nDCDC(%bd):%bd",step,i);
			return ERR_SUCCESS;	//break;
		}
		delay1ms(2);
	}
//	Printf("\nDCDC_On(%bd) FAIL",step);
	return ERR_FAIL;
}
#endif

//GPIO43 or expender GPIO[1]
//-----------------------------------------------------------------------------
/**
* set FP_Bias
*/
void FP_BiasOnOff(BYTE fOn)
{
	Printf("\nFP_Bias %s",fOn ? "On" : "Off");

	//WriteTW88Page(PAGE0_GENERAL);
	//WriteTW88(REG084, 0x01);	//disable
	if (fOn)
	{
		WriteI2CByte(I2CID_SX1504, 1, 0);		// output enable
		WriteI2CByte(I2CID_SX1504, 0, ReadI2CByte(I2CID_SX1504, 0) & 0xFD);		// FPBIAS enable.
	}
	else
	{
		WriteI2CByte(I2CID_SX1504, 1, 0);		// output enable
		WriteI2CByte(I2CID_SX1504, 0, ReadI2CByte(I2CID_SX1504, 0) | 0x02);		// FPBIAS disable
	}
}

//FrontPanel PowerControl ON - GPIO42. or expender GPIO[0]
//-----------------------------------------------------------------------------
/**
* set FP_PWC
*/
void FP_PWC_OnOff(BYTE fOn)
{
	Printf("\nFP_PWC %s",fOn ? "On" : "Off");
	I2C_delay_base = 3;									//assume 108/1.5.

	WriteTW88Page(PAGE0_GENERAL);
	WriteTW88(REG084, 0x01);											//disable
	if (fOn)
	{
		WriteI2CByte(I2CID_SX1504, 1, 0);									// output enable
		WriteI2CByte(I2CID_SX1504, 0, ReadI2CByte(I2CID_SX1504, 0) & 0xFE);	// FPPWC enable
	}
	else
	{
		WriteI2CByte(I2CID_SX1504, 1, 0);									// output enable
		WriteI2CByte(I2CID_SX1504, 0, ReadI2CByte(I2CID_SX1504, 0) | 0x01);	// FPPWC disable
	}
}

//-----------------------------------------------------------------------------
/**
* set GPIO for FP
*/
void FP_GpioDefault(void)
{
	WriteTW88Page(PAGE0_GPIO);
	//IR uses PORT1_4(GPIO40). To disable TCPOLN output, we need to enable it as GPIO and uses it as input port with INT11.
	WriteTW88(REG084, 0x01);	//GPIO 4x Enable     	- GPIO40 enable
	WriteTW88(REG08C, 0x00);	//GPIO 4x direction		- GPIO40 input
	WriteTW88(REG094, 0x00);	//GPIO 4x output data	- GPIO40 outdata as 0.

	if (access)
	{
		//turn off FPPWC & FPBias. make default
		//	0x40 R0 R1 is related with FP_PWC_OnOff
		WriteI2CByte(I2CID_SX1504, 1, 0/*3*/ );	//RegDir:	input 
		WriteI2CByte(I2CID_SX1504, 0, 0xFF/*3*/ );	//RegData:	FPBias OFF. FPPWC disable.
		//WriteI2CByte( I2CID_SX1504, 1, 0xFF/*3*/ );	//RegDir:	input 
		Printf("\nI2CID_SX1504 0:%02bx 1:%bx", ReadI2CByte(I2CID_SX1504, 0), ReadI2CByte(I2CID_SX1504, 1));
	}
}

#define SX1504_DATA_REG		0	//data register index
#define SX1504_DIR_REG		1	//direction register index
//Enable SN65LVDS93A by GPIO Expander IO[4].
void EnableExtLvdsTxChip(BYTE fOn)
{
	BYTE bTemp;
	WriteTW88Page(PAGE0_GENERAL);
	WriteTW88(REG084, 0x01);	//disable
	bTemp = ReadI2CByte(I2CID_SX1504, 0);
	if(fOn)
		WriteI2CByte( I2CID_SX1504, 0, bTemp | 0x10 );		//  enable LVDS Rx chip
	else
		WriteI2CByte( I2CID_SX1504, 0, bTemp & ~0x10 );		// disable LVDS Rx chip
	WriteI2CByte( I2CID_SX1504, 1, ReadI2CByte(I2CID_SX1504, 1) & ~0x10 );		// output enable
}

//-----------------------------------------------------------------------------
/**
* init default NTSC value
*/

//-----------------------------------------------------------------------------
/**
* turn on DCDC
*
*	DCDC startup step
*	SSPLL ON
*	FP PWC ON
*	call DCDC_on(0) & DCDC_on(1)
*	delay
*	wait VBlank
*	Enable FP Data Out
*	delay
*	call DCDC_on(2)
*	FP Bias On
*
* DCDC data out needs more then 200ms delay after SSPLL_PowerUp(ON).
*/
BYTE DCDC_StartUP_sub(void)
{
	BYTE ret;

#if 0
		ret = ReadTW88(REG008);
		Printf("\nREG008:%bx FPData:%bd Output:%bx",ret, ret&0x20?0:1, ret&0x10?0:1 );
		Printf("\nREG0E8:%bx", ReadTW88(REG0E8));
		Printf("\nREG0EA:%bx", ReadTW88(REG0EA));
		ret = ReadTW88(REG0FC);
		Printf("\nREG0FC:%bx PD_SSPLL:%bd", ret, ret & 0x80);
		Printf("\nREG21E:%bx", ReadTW88(REG21E));
#endif

	//-------------
	//FPPWC ON
	FP_PWC_OnOff(ON);

	//ret=DCDC_On(0);
	//ret=DCDC_On(1);

	//-------------
	// wait
#ifdef TW8835_EVB_10
	delay1ms(100);
#endif

	WaitVBlank(1);
	//-------------
	//FP Data Out
	OutputEnablePin(ON, ON);		//Output enable. FP data: enable

#ifdef TW8835_EVB_10
	delay1ms(15);
#endif

	//DCDC final
	//ret=DCDC_On(2);
	ret = ERR_SUCCESS;

	//-------------
	//FPBIAS ON 
	FP_BiasOnOff(ON);

	//disable Blank
	//WriteTW88Page(PAGE2_SCALER);
	//WriteTW88(REG21E, ReadTW88(REG21E) & ~0x01);

//	PrintSystemClockMsg("DCDC_StartUp END");
	if (ret != ERR_SUCCESS)
	{
		Puts(" FAIL");

		//WriteTW88Page(PAGE0_DCDC);
		//WriteTW88(REG0E8, 0xF2);	Printf("\nREG0E8:F2[%bd]",ReadTW88(REG0EA)>>4);
		//WriteTW88(REG0E8, 0x02);	Printf("\nREG0E8:02[%bd]",ReadTW88(REG0EA)>>4);
		//WriteTW88(REG0E8, 0x03);	Printf("\nREG0E8:03[%bd]",ReadTW88(REG0EA)>>4);
		//WriteTW88(REG0E8, 0x01);	Printf("\nREG0E8:01[%bd]",ReadTW88(REG0EA)>>4);
		//WriteTW88(REG0E8, 0x11);	Printf("\nREG0E8:11[%bd]",ReadTW88(REG0EA)>>4);
		//WriteTW88(REG0E8, 0x71);	Printf("\nREG0E8:71[%bd]",ReadTW88(REG0EA)>>4);
	}

	return ret;
}

//-----------------------------------------------------------------------------
/**
* turn on DCDC
*
* @see DCDC_StartUP_sub
*/
BYTE DCDC_StartUP(void)
{
	BYTE ret;
	
//	Puts("\nDCDC_StartUP start");
	ret = DCDC_StartUP_sub();
	if (ret == ERR_SUCCESS)
		return ERR_SUCCESS;

	ret = DCDC_StartUP_sub();
	if (ret == ERR_SUCCESS)
		return ERR_SUCCESS;

	ret = DCDC_StartUP_sub();
	return ret;
}


