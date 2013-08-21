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

//-----------------------------------------------------------------------------
/**
* set Low speed clock. only for test
*/

//-----------------------------------------------------------------------------
/**
* set 27MHz clock. only for test
*/

//=========================================
// SSPLL
//=========================================

//-----------------------------------------------------------------------------
/**
* power up the SSPLL
*/
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
//-----------------------------------------------------------------------------
/**
* get SSPLL register value
*
*	register
*	R0F8[3:0]	FPLL[19:16]
*	R0F9[7:0]	FPLL[15:8]
*	R0FA[7:0]	FPLL[7:0]
*/
//-----------------------------------------------------------------------------
/**
* set SSPLL AnalogControl register
*
*	register
*	R0FD[7:6] POST
*	R0FD[5:4] VCO
*	R0FD[2:0] ChargePump
*/

//-----------------------------------------------------------------------------
/**
* get SSPLL Post value
*/

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

//=========================================
// PCLK
//=========================================

//-----------------------------------------------------------------------------
/**
* set PCLK divider
*/

//-----------------------------------------------------------------------------
/**
* get PCLK frequency
*/
//-----------------------------------------------------------------------------
/**
* get PCLKO frequency
*/

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
//-----------------------------------------------------------------------------
/**
* set PCLK polarity
*
* @param pol - Pixel clock output polarity
*	-0:	no inversion
*	-1:	inversion
*	- 0xFF: do not change it. Use previous value
*/


//=========================================
// CLKPLL
//=========================================

//-----------------------------------------------------------------------------
/**
* select ClkPLL input
*/
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

//-----------------------------------------------------------------------------
/**
* set ClkPLL input and ClkPLL divider
* 
* only from monitor
*/

//-----------------------------------------------------------------------------
/**
* get ClkPLL frequency
*/

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

//-----------------------------------------------------------------------------
/**
* restore MCUSPI clock
*
* @see McuSpiClkToPclk
*/


//-----------------------------------------------------------------------------
/**
* read MCUSPI clock mode
*/

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
#if 0
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
#endif

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

//=========================================
// SPI CLK
//=========================================

//-----------------------------------------------------------------------------
/**
* get SPI clock frequency
*/

//-----------------------------------------------------------------------------
/**
* set LLPLL clock source
*
* use 27M OSD or PLL 
*/

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
	if(fOn) {
		WriteI2CByte( I2CID_SX1504, 1, 0 );		// output enable
		WriteI2CByte( I2CID_SX1504, 0, ReadI2CByte(I2CID_SX1504, 0) & 0xFD );		// FPBIAS enable.
	}
	else {
		WriteI2CByte( I2CID_SX1504, 1, 0 );		// output enable
		WriteI2CByte( I2CID_SX1504, 0, ReadI2CByte(I2CID_SX1504, 0) | 0x02 );		// FPBIAS disable
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
	if(fOn) {
		WriteI2CByte( I2CID_SX1504, 1, 0 );									// output enable
		WriteI2CByte( I2CID_SX1504, 0, ReadI2CByte(I2CID_SX1504, 0) & 0xFE );	// FPPWC enable
	}
	else {
		WriteI2CByte( I2CID_SX1504, 1, 0 );									// output enable
		WriteI2CByte( I2CID_SX1504, 0, ReadI2CByte(I2CID_SX1504, 0) | 0x01 );	// FPPWC disable
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


	if(access) {
		//turn off FPPWC & FPBias. make default
		//	0x40 R0 R1 is related with FP_PWC_OnOff
		WriteI2CByte( I2CID_SX1504, 1, 0/*3*/ );	//RegDir:	input 
		WriteI2CByte( I2CID_SX1504, 0, 0xFF/*3*/ );	//RegData:	FPBias OFF. FPPWC disable.
		//WriteI2CByte( I2CID_SX1504, 1, 0xFF/*3*/ );	//RegDir:	input 
		Printf("\nI2CID_SX1504 0:%02bx 1:%bx",ReadI2CByte(I2CID_SX1504, 0), ReadI2CByte(I2CID_SX1504, 1));
	}
}

#define SX1504_DATA_REG		0	//data register index
#define SX1504_DIR_REG		1	//direction register index
//Enable SN65LVDS93A by GPIO Expander IO[4].
//void EnableExtLvdsTxChip(BYTE fOn)

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
	OutputEnablePin(ON,ON);		//Output enable. FP data: enable


#ifdef TW8835_EVB_10
	delay1ms(15);
#endif

	//DCDC final
	//ret=DCDC_On(2);
	ret=ERR_SUCCESS;

	//-------------
	//FPBIAS ON 
	FP_BiasOnOff(ON);

	//disable Blank
	//WriteTW88Page(PAGE2_SCALER);
	//WriteTW88(REG21E, ReadTW88(REG21E) & ~0x01);

//	PrintSystemClockMsg("DCDC_StartUp END");
	if(ret!=ERR_SUCCESS) {
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
	
	Puts("\nDCDC_StartUP start1");
	ret=DCDC_StartUP_sub();
	if(ret == ERR_SUCCESS) {
		Puts("\nDCDC_StartUP SUCCESS");
		return ERR_SUCCESS;
	}

	Puts("\nDCDC_StartUP start2");
	ret=DCDC_StartUP_sub();
	if(ret == ERR_SUCCESS) {
		Puts("\nDCDC_StartUP SUCCESS");
		return ERR_SUCCESS;
	}
	Puts("\nDCDC_StartUP start3");
	ret=DCDC_StartUP_sub();
	if(ret == ERR_SUCCESS)
		Puts("\nDCDC_StartUP SUCCESS");
	return ret;
}


