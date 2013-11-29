/**
 * @file
 * InputCtrl.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	see video input control 
 *
*/
/*
* video input control 
*
*	+-----+ LoSpeed  +-----+  +---------+   +-+           +------+
*	|     | Decoder  |     |=>| decoder |==>| |==========>|      |
*	|     | =======> |     |  +---------+   |I|           |      |
*	|     |          |     |                |n|           |      |
*	|     | HiSpeed  |AMux |                | |           |      |
*	|     | ARGB     |     |  +---------+   |M|           |      |
*	|INPUT| =======> |     |=>|  ARGB   |==>|U|==========>|Scaler|
*	|     |          +-----+  +---------+   |X|           |      |
*	|     | Digital                         | |           |      |
*	|     | DTV                             | |  +-----+  |      |
*	|     | ===============================>| |=>| DTV |=>|      |
*	+-----+                                 +-+  +-----+  +------+
*	                                         |    +--------+
*	                                         +==> |Measure |
*	                                              +--------+
*/

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
#include "spi.h"

#include "main.h"
#include "SOsd.h"
#include "FOsd.h"
#include "decoder.h"
#include "Scaler.h"
#include "InputCtrl.h"
#include "EEPROM.h"
#include "ImageCtrl.h"
#include "Settings.h"
#include "measure.h"
#include "aRGB.h"
#include "dtv.h"
#include "InputCtrl.h"
#include "OutputCtrl.h"
#include "SOsdMenu.h"
#ifdef SUPPORT_HDMI_EP9351
#include "hdmi_EP9351.H"
#endif
#ifdef SUPPORT_HDMI_EP9553
#include "hdmi_EP9553.H"
#endif
#ifdef SUPPORT_HDMI_EP907M
#include "hdmi_EP907M.H"
#endif

#include "BT656.h"
/*

SDTV 480i/60M
	 576i/50	
	 480p SMPTE 267M-1995
HDTV 1080i/60M
	 1080i/50
	 720p/60M
	 720p/50
	 1080p = SMPTE 274M-1995 1080p/24 & 1080p/24M
	                         1080p/50 1080p/60M


			scan lines	 field1 field2	 half
480i/60M	525			 23~262 285~524	 142x
576i/50		625			 23~310 335~622
1080i		1125
720p		750

standard
480i/60M	SMPTE 170M-1994.
			ITU-R BT.601-4
			SMPTE 125M-1995
			SMPTE 259M-1997
*/

//=============================================================================
// INPUT CONTROL
//=============================================================================
// Input Module
// start from 0x040
//0x040~0x049
//R040[1:0]	Input Select		0:InternalDecoder,1:ARGB/YUV(YPbPr),2:DTV(BT656)
//R041[0]	Input data format	0:YCbCr 1:RGB
//=============================================================================

XDATA	BYTE	InputMain;
XDATA	BYTE	InputBT656;
XDATA	BYTE	InputSubMode;

//-----------------------------------------------------------------------------
/**
* Get InputMain value
*
* friend function.
* Other bank, specially Menu Bank(Bank2) needs this InputMain global variable.
*/
BYTE GetInputMain(void)
{
	return InputMain;
}
//-----------------------------------------------------------------------------
/**
* Set InputMain value
*
* @see GetInputMain
*/
void SetInputMain(BYTE input)
{
	InputMain = input;
	//update EE
}

#define VBLANK_WAIT_VALUE	0xFFFE 

/**
* wait Vertical Blank
*
* You can use this function after you turn off the PD_SSPLL(REG0FC[7]).
* 0xFFFE value uses max 40ms on Cache + 72MHz.
*/
void WaitVBlank(BYTE cnt)
{
	XDATA BYTE i;
	WORD loop;
	DECLARE_LOCAL_page

	ReadTW88Page(page);
	WriteTW88Page(PAGE0_GENERAL);

	for (i=0; i<cnt; i++)
	{
		WriteTW88(REG002, 0xff);
		loop = 0;
		while (!(ReadTW88(REG002) & 0x40))
		{
			// wait VBlank
			loop++;
			if (loop > VBLANK_WAIT_VALUE)
			{
				wPrintf("\nERR:WaitVBlank");
				break;
			}
		}		
	}
	WriteTW88Page(page);
}

/**
* wait Vertical Blank
*
* @see WaitVBlank
*/
void WaitOneVBlank(void)
{
	WORD loop;
	DECLARE_LOCAL_page

	ReadTW88Page(page);
	WriteTW88Page(PAGE0_GENERAL );

	WriteTW88(REG002, 0xff );
	loop = 0;
	while (!( ReadTW88(REG002 ) & 0x40 ) ) {
		// wait VBlank
		loop++;
		if(loop > VBLANK_WAIT_VALUE  ) {
			wPrintf("\nERR:WaitVBlank");
			break;
		}
	}
	WriteTW88Page(page);
}

//-----------------------------------------------------------------------------
//class:Input
/**
* Set input path & color domain
*
*	register
*	REG040[1:0]
*	REG041[0]
*
* @param path: input mode
*		- 0:InternalDecoder
*		- 1:AnalogRGB/Component. PC or Component
*		- 2:BT656(DTV). Note, HDTV use BT709.
*		- 3:DTV2 ??   <--TW8835 do not support                                  
* @param format: data format.
*		- 0:YCbCr 1:RGB
*/
void InputSetSource(BYTE path, BYTE format)
{
	BYTE r040, r041;

	WriteTW88Page(PAGE0_GENERAL);
	r040 = ReadTW88(REG040_INPUT_CTRL_I) & ~0x17;	//clear [2] also.
	r041 = ReadTW88(REG041_INPUT_CTRL_II) & ~0x3F;
	r040 |= path;
	r041 |= format;

	if (path == INPUT_PATH_DECODER)		//InternalDecoder
	{
		r041 |= 0x0C;					//input sync detion edge control. falling edge
	}
	else if (path == INPUT_PATH_VADC) 	//ARGB(PC or Component)
	{
		r040 |= 0x10;					//invert clock
		if (InputMain == INPUT_COMP)
		{
			r041 |= 0x20;				//progressive
			r041 |= 0x10;				//implicit DE mode.(Component, don't care)
			r041 |= 0x0C;				//input sync detion edge control. falling edge
			r041 |= 0x02;				//input field inversion
		}
		else
		{
			//??r041 |= 0x20;			//progressive
			r041 |= 0x10;				//implicit DE mode.(Component, don't care)
			r041 |= 0x0C;				//input sync detion edge control. falling edge
		}
	}
	else if (path == INPUT_PATH_DTV)	//DTV
	{
										//clock normal
		r040 |= 0x08;					//INT_4 pin is turn into dtvde pin
		//r041 |= 0x20;					// progressive
		r041 |= 0x10;					//implicit DE mode
		//r041 |= 0x0C;					//input sync detion edge control. falling edge
	}
	else if (path == INPUT_PATH_LVDS) 	//LVDS_RX
	{
		r040 |= 0x08;					//INT_4 pin is turn into dtvde pin
		r041 |= 0x10;					//implicit DE mode
	}
	else if (path == INPUT_PATH_BT656)
	{
		//target r040:0x06 r041:0x00
	}
	
	dPrintf("\nInputSetSource r040:%bx r041:%bx",r040,r041);
	WriteTW88(REG040_INPUT_CTRL_I, r040);
	WriteTW88(REG041_INPUT_CTRL_II, r041);
}

#ifdef UNCALLED_SEGMENT
//-----------------------------------------------------------------------------
//class:Input
void InputSetProgressiveField(fOn)
{
	WriteTW88Page(PAGE0_INPUT);
	if(fOn)	WriteTW88(REG041_INPUT_CTRL_II, ReadTW88(REG041_INPUT_CTRL_II) | 0x20);	    //On Field for Prog
	else	WriteTW88(REG041_INPUT_CTRL_II, ReadTW88(REG041_INPUT_CTRL_II) & ~0x20);	//Off Field for Prog
}
#endif

#ifdef UNCALLED_SEGMENT
//-----------------------------------------------------------------------------
//class:Input
void InputSetPolarity(BYTE V,BYTE H, BYTE F)
{
	BYTE r041;
	WriteTW88Page( PAGE0_INPUT );

	r041 = ReadTW88(REG041_INPUT_CTRL_II ) & ~0x0E;
	if(V)	r041 |= 0x08;
	if(H)	r041 |= 0x04;
	if(F)	r041 |= 0x02;
	WriteTW88(REG041_INPUT_CTRL_II, r041);
}
//-----------------------------------------------------------------------------
//class:Input
BYTE InputGetVPolarity(void)
{
	BYTE r041;
	WriteTW88Page( PAGE0_INPUT );

	r041 = ReadTW88(REG041_INPUT_CTRL_II );
	if(r041 & 0x08)	return ON;		//detect falling edge
	else			return OFF;		//detect rising edge
}
//-----------------------------------------------------------------------------
//class:Input
BYTE InputGetHPolarity(void)
{
	BYTE r041;
	WriteTW88Page( PAGE0_INPUT );

	r041 = ReadTW88(REG041_INPUT_CTRL_II );
	if(r041 & 0x04)	return ON;		//detect falling edge
	else			return OFF;		//detect rising edge
}
//-----------------------------------------------------------------------------
//class:Input
BYTE InputGetFieldPolarity(void)
{
	BYTE r041;
	WriteTW88Page( PAGE0_INPUT );

	r041 = ReadTW88(REG041_INPUT_CTRL_II );
	if(r041 & 0x02)	return ON;		//input field inversion
	else			return OFF;		//
}
#endif

#if defined(SUPPORT_COMPONENT)
//-----------------------------------------------------------------------------
//class:Input
/**
* set Field Polarity
*
* R041[1] input field control. 1:inversion
*
* 480i & 576i need a "1" on TW8836
*/
void InputSetFieldPolarity(BYTE fInv)
{
	BYTE r041;
	WriteTW88Page( PAGE0_INPUT );

	r041 = ReadTW88(REG041_INPUT_CTRL_II );
	if(fInv)	WriteTW88(REG041_INPUT_CTRL_II, r041 | 0x02);
	else 		WriteTW88(REG041_INPUT_CTRL_II, r041 & ~0x02);
}
#endif

#ifdef UNCALLED_SEGMENT
//-----------------------------------------------------------------------------
//class:Input
/*
* R041[0] Input data format selection 1:RGB
*/
BYTE InputGetColorDomain(void)
{
	BYTE r041;
	WriteTW88Page( PAGE0_INPUT );

	r041 = ReadTW88(REG041_INPUT_CTRL_II );
	if(r041 & 0x01)	return ON;		//RGB color
	else			return OFF;		//YUV color
}
#endif

//-----------------------------------------------------------------------------
//class:Input
/**
* set input crop
*
* input cropping for implicit DE.
* NOTE:InternalDecoder is not an implicit DE.
*
*	register
*	REG040[7:6]REG045[7:0]	HCropStart
*			   REG043[7:0]	VCropStart
*	REG042[6:4]REG044[7:0]	VCropLength
*	REG042[3:0]REG046[7:0]	HCropLength
*/
void InputSetCrop( WORD x, WORD y, WORD w, WORD h )
{
	WriteTW88Page( PAGE0_INPUT );

	WriteTW88(REG040_INPUT_CTRL_I, (ReadTW88(REG040_INPUT_CTRL_I) & 0x3F) | ((x & 0x300)>>2) );
	WriteTW88(REG045, (BYTE)x);
	WriteTW88(REG043, (BYTE)y);

	WriteTW88(REG042, ((h&0xF00) >> 4)|(w >>8) );
	WriteTW88(REG044, (BYTE)h);
	WriteTW88(REG046, (BYTE)w);
	//dPrintf("\nInput Crop Window: x = %d, y = %d, w = %d, h = %d", x, y, w, h );
}

#if 0
//-----------------------------------------------------------------------------
//class:Input
void InputSetCropStart( WORD x, WORD y)
{
	WriteTW88Page( PAGE0_INPUT );
	WriteTW88(REG040, (ReadTW88(REG040) & 0x3F) | ((x & 0xF00)>>2) );
	WriteTW88(REG045, (BYTE)x);
	WriteTW88(REG043, (BYTE)y);
	//dPrintf("\nInput Crop Window: x = %d, y = %d", x, y);
}
#endif

#ifdef SUPPORT_PC
//-----------------------------------------------------------------------------
//class:Input
/**
* set Horizontal Start at InputCrop
*/
void InputSetHStart( WORD x)
{
	WriteTW88Page( PAGE0_INPUT );

	WriteTW88(REG040, (ReadTW88(REG040) & 0x3F) | ((x & 0xF00)>>2) );
	WriteTW88(REG045, (BYTE)x);
	//dPrintf("\nInput Crop Window: x = %d", x);
}
#endif
//-----------------------------------------------------------------------------
//class:Input
/**
* get Horizontal Start at InputCrop
*/
WORD InputGetHStart(void)
{
	WORD wValue;
	WriteTW88Page( PAGE0_INPUT );

	wValue = ReadTW88(REG040) & 0xC0;
	wValue <<= 2;
	wValue |=  ReadTW88(REG045);
	return wValue;
}

#if 0
//-----------------------------------------------------------------------------
//class:Input
void InputSetVStart( WORD y)
{
	WriteTW88Page( PAGE0_INPUT );

	WriteTW88(REG043, (BYTE)y);
	dPrintf("\nInput Crop Window: y = %d", y);
}
#endif
//-----------------------------------------------------------------------------
//class:Input
WORD InputGetVStart(void)
{
	WORD wValue;
	WriteTW88Page( PAGE0_INPUT );

	wValue = ReadTW88(REG043 );
	return wValue;
}

#if 1
//-----------------------------------------------------------------------------
//class:Input
WORD InputGetHLen(void)
{
	WORD len;
	WriteTW88Page( PAGE0_INPUT );
	len =ReadTW88(REG042) & 0x0F;
	len <<=8;
	len |= ReadTW88(REG046);
	return len;
}
//-----------------------------------------------------------------------------
//class:Input
WORD InputGetVLen(void)
{
	WORD len;
	WriteTW88Page( PAGE0_INPUT );
	len =ReadTW88(REG042) & 0x70;
	len <<=4;
	len |= ReadTW88(REG044);
	return len;
}
#endif

#if 0
//-----------------------------------------------------------------------------
//class:BT656Input
//register
//	R047[7]	BT656 input control	0:External input, 1:Internal pattern generator
void BT656InputSetFreerun(BYTE fOn)
{
	WriteTW88Page(PAGE0_INPUT);
	if(fOn)	WriteTW88(REG047,ReadTW88(REG047) | 0x80);
	else	WriteTW88(REG047,ReadTW88(REG047) & ~0x80);
}
#endif
//-----------------------------------------------------------------------------
//class:BT656Input
/**
* set Freerun and invert clock flag on BT656
*
*	R047[7]
*	R047[5]
*/
void BT656InputFreerunClk(BYTE fFreerun, BYTE fInvClk)
{
	BYTE value;
	WriteTW88Page(PAGE0_INPUT);
	value = ReadTW88(REG047);
	if(fFreerun)	value |= 0x80;
	else			value &= ~0x80;
	
	if(fInvClk)		value |= 0x20;
	else			value &= ~0x20;
	WriteTW88(REG047, value);
}

//-----------------------------------------------------------------------------
/**
* Change Video Input.
*
* @param mode
*	- INPUT_CVBS : ChangeCVBS
*	- INPUT_SVIDEO: ChangeCVBS
*	- INPUT_COMP : ChangeCOMPONENT
*	- INPUT_PC :  ChangePC
*	- INPUT_DVI : ChangeDVI
* 	- INPUT_HDMIPC:
*	- INPUT_HDMITV:	ChangeHDMI
*	- INPUT_BT656: ChangeBT656
* @see ChangeCVBS
*/
void ChangeInput(BYTE mode)
{
	Printf("\nChangeInput:");
	PrintfInput(mode, 0);

	if (getNoSignalLogoStatus())
		RemoveLogo();

	switch (mode)
	{
#ifdef SUPPORT_CVBS
		case INPUT_CVBS:
			ChangeCVBS();
			break;
#endif
#ifdef SUPPORT_SVIDEO
		case INPUT_SVIDEO:
			ChangeSVIDEO();
			break;
#endif
#ifdef SUPPORT_COMPONENT
		case INPUT_COMP:
			ChangeCOMPONENT();
			break;
#endif
#ifdef SUPPORT_PC
		case INPUT_PC:
			ChangePC();
			break;
#endif
#ifdef SUPPORT_DVI
		case INPUT_DVI:
			ChangeDVI();
			break;
#endif
#if defined(SUPPORT_HDMI)
		case INPUT_HDMIPC:
		case INPUT_HDMITV:
			ChangeHDMI();
			break;
#endif
#ifdef SUPPORT_BT656_LOOP
		case INPUT_BT656:
			ChangeBT656Loop();
			break;
#endif
#ifdef SUPPORT_LVDSRX
		case INPUT_LVDS:
			ChangeLVDSRx();
			break;
#endif
		default:
			ChangeCVBS();
			break;
	}
}

//-----------------------------------------------------------------------------
/**
* move to next video input
*/
void	InputModeNext( void )
{
	BYTE next_input;

#if defined(SUPPORT_HDMI)
	if(InputMain==INPUT_HDMIPC)
		next_input = InputMain + 2;
	else
#endif
	next_input = InputMain + 1;

	do {
		if(next_input == INPUT_TOTAL)
			next_input = INPUT_CVBS;	
#ifndef SUPPORT_CVBS
		if(next_input==INPUT_CVBS)
			next_input++;	
#endif
#ifndef SUPPORT_SVIDEO
		if(next_input==INPUT_SVIDEO)
			next_input++;	
#endif
#ifndef SUPPORT_COMPONENT
		if(next_input==INPUT_COMP)
			next_input++;	
#endif
#ifndef SUPPORT_PC
		if(next_input==INPUT_PC)
			next_input++;	
#endif
#ifndef SUPPORT_DVI
		if(next_input==INPUT_DVI)
			next_input++;	
#endif
#if defined(SUPPORT_HDMI)
		if(next_input==INPUT_HDMIPC)
			next_input+=2;	
		else if(next_input==INPUT_HDMITV)
			next_input++;
#endif
#ifndef SUPPORT_BT656_LOOP
		if(next_input==INPUT_COMP)
			next_input++;	
#endif
#if defined(SUPPORT_HDMI)
		if(next_input==INPUT_HDMIPC) {
			if(GetHdmiModeEE())  next_input = INPUT_HDMITV;
		}
#endif
#ifndef SUPPORT_LVDSRX
		if(next_input==INPUT_LVDS)
			next_input++;	
#endif
	} while(next_input==INPUT_TOTAL);

	ChangeInput(next_input);
}


//=============================================================================
// Input Control routine
//=============================================================================

//extern CODE BYTE DataInitNTSC[];


//-----------------------------------------------------------------------------
/**
* prepare video input register after FW download the default init values.
*
*	select input path
*	turnoff freerun manual & turnon freerun auto.
*	assign default freerun Htotal&Vtotal
*
* @see I2CDeviceInitialize
*/
//extern void TEMP_init_BT656(void);
		
void InitInputAsDefault(void)
{
	//---------------------------------
	//step1:
	//Before FW starts the ChangeInput, 
	//		link ISR & turnoff signal interrupt & NoSignal task,
	//		turn off LCD.
	FOsdIndexMsgPrint(FOSD_STR5_INPUTMAIN);		//prepare InputMain string

	LinkCheckAndSetInput();						//link CheckAndSetInput
	Interrupt_enableVideoDetect(OFF);			//turnoff Video Signal Interrupt
	TaskNoSignal_setCmd(TASK_CMD_DONE);			//turnoff NoSignal Task
	LedBackLight(OFF);							//turnoff LedBackLight

	//---------------------------------
	//step2:
	//set system default
	//	Download the default register values.
	//	set sspll
	//	select MCU/SPI Clock
 	//dPuts("\nI2CDownload DataInitNTSC");
	//I2CDeviceInitialize( DataInitNTSC, 0 );	 //Pls, do not use this ....

	Init8836AsDefault(InputMain, 0);

	if (SpiFlashVendor == SFLASH_VENDOR_MX)
	{
		WriteTW88Page(PAGE4_CLOCK);
		WriteTW88(REG4E1, (ReadTW88(REG4E1) & 0xF8) | 0x02);	//if Macronix SPI Flash, SPI_CK_DIV[2:0]=2
	}		
	//If you want CKLPLL, select MCUSPI_CLK_PCLKPLL 
	//??BK120709
	//	McuSpiClkSelect(MCUSPI_CLK_PCLKPLL);


	//---------------------------------
	//step3:
	//	InputSource=>InMux=>Decoder=>aRGB=>BT656=>DTV=>Scaler=>Measure
	//-------------------

	//InputSource  (InMux)
	switch (InputMain)
	{
	case INPUT_CVBS:
	case INPUT_SVIDEO:
		InputSetSource(INPUT_PATH_DECODER, INPUT_FORMAT_YCBCR);
		break;
	case INPUT_COMP:	//target R040:31 R041:3E
		InputSetSource(INPUT_PATH_VADC, INPUT_FORMAT_YCBCR);		
		break;
	case INPUT_PC:		//target R040:31 R041:1D
		InputSetSource(INPUT_PATH_VADC, INPUT_FORMAT_RGB);		
		break;
	case INPUT_DVI:		//target R040:2A R041:11. Note:DtvInitDVI() overwite R040.
		InputSetSource(INPUT_PATH_DTV, INPUT_FORMAT_RGB);		
		break;
	case INPUT_HDMIPC:
	case INPUT_HDMITV:	//target R040: R041:
		InputSetSource(INPUT_PATH_DTV, INPUT_FORMAT_RGB);		
		break;
	case INPUT_BT656:	//target R040:2A R041:00
		InputSetSource(INPUT_PATH_BT656, INPUT_FORMAT_YCBCR);	 
		break;
	case INPUT_LVDS:	//target R040:2A R041:00
		InputSetSource(INPUT_PATH_LVDS, INPUT_FORMAT_RGB);	 
//		InputSetSource(INPUT_PATH_DTV,INPUT_FORMAT_RGB);	 
		break;
	}

	//Analog Mux
	AMuxSetInput(InputMain);

	//Decoder	
	DecoderFreerun(DECODER_FREERUN_AUTO);	//component,pc,dvi removed

	//aRGB(VAdc)
	aRGB_SetDefaultFor();

//	if(InputMain==INPUT_BT656)
		BT656OutputEnable(ON, 0);			//R007[3]=1.DataInitNTSC clear it.
		BT656_A_SelectOutput(0, 0, 0, 1);
		BT656_A_SelectCLKO(0, 0);
//	else
//		BT656OutputEnable(OFF, 1);

	//BT656Input
	switch (InputMain)
	{
	case INPUT_CVBS:
	case INPUT_SVIDEO:
	case INPUT_COMP:
	case INPUT_PC:
		break;
	case INPUT_DVI:
		BT656InputFreerunClk(OFF, OFF);		//BT656 turnoff FreeRun
		break;
	case INPUT_HDMIPC:
	case INPUT_HDMITV:
	case INPUT_LVDS:
//#ifdef SUPPORT_HDMI_EP907M
//		BT656InputFreerunClk(OFF, ON);		//BT656 turnoff FreeRun. CLK invert.
//#else
		BT656InputFreerunClk(OFF, OFF);		//BT656 turnoff FreeRun
//#endif
		break;
	case INPUT_BT656:
		BT656InputFreerunClk(OFF, ON);		//off freerun, on invert_clk
		break;
	}

	//DTV
	switch (InputMain)
	{
	case INPUT_CVBS:
	case INPUT_SVIDEO:
	case INPUT_COMP:
	case INPUT_PC:
		break;
#ifdef SUPPORT_DVI
	case INPUT_DVI:
		DtvSetClockDelay(1);
		DtvSetVSyncDelay(4);

		DtvSetFieldDetectionRegion(ON,0x11);	// set Det field by WIN
		DtvSetPolarity(0,0);
		DtvSetRouteFormat(DTV_ROUTE_YPbPr,DTV_FORMAT_RGB565);
		break;
#endif
	case INPUT_HDMIPC:
	case INPUT_HDMITV:
	case INPUT_LVDS:			//LVDS need a HDMI measure.
#if defined(SUPPORT_HDMI)

#if defined(SUPPORT_HDMI_EP9351) || defined(SUPPORT_HDMI_EP9553) || defined(SUPPORT_HDMI_EP907M) 
		//default is RGB565. test RGB24.
//#ifdef SUPPORT_HDMI_24BIT
//#if defined(SUPPORT_HDMI_EP907M)
//		DtvSetRouteFormat(DTV_ROUTE_PbYPr,DTV_FORMAT_RGB); //RGB24. 120823
//#else
//		DtvSetRouteFormat(DTV_ROUTE_PrYPb,DTV_FORMAT_RGB); //RGB24. 120720
//#endif
//#else	//24
//		DtvSetRouteFormat(DTV_ROUTE_YPbPr,DTV_FORMAT_RGB565);
//#endif	//24
#endif	//EP

		//BK121213. TW8836 EVB10 has reversed ORDER..
		DtvSetRouteFormat(DTV_ROUTE_565_MSB_B_LSB_R_REVERSED,DTV_FORMAT_RGB565);
		DtbSetReverseBusOrder(1);


		DtvSetClockDelay(1);	//BK111201
		DtvSetVSyncDelay(4);	//BK111201

		DtvSetFieldDetectionRegion(ON,0x11);	// set Det field by WIN
#endif	//..SUPPORT_HDMI
		break;
	case INPUT_BT656:
#ifdef SUPPORT_BT656_LOOP
//		DtvSetRouteFormat(DTV_ROUTE_PrYPb,DTV_FORMAT_INTERLACED_ITU656);
//EVB1.0
		DtvSetRouteFormat(DTV_ROUTE_PbYPr,DTV_FORMAT_INTERLACED_ITU656);
		DtbSetReverseBusOrder(1);
#endif
		break;

	}
//BKTODO:120423. power down the external chip.
//#if defined(SUPPORT_HDMI_EP9351)
//	if(InputMain != INPUT_HDMIPC && InputMain != INPUT_HDMITV) {
//		//power down
//	}
//#endif


	//LVDSRx
	switch (InputMain)
	{
	case INPUT_CVBS:
	case INPUT_SVIDEO:
	case INPUT_COMP:
	case INPUT_PC:
	case INPUT_DVI:
	case INPUT_HDMIPC:
	case INPUT_HDMITV:
		EnableExtLvdsTxChip(OFF);	//GPIO EXPANDER IO[4]
		WriteTW88(REG648, ReadTW88(REG648) & ~0x02);
		WriteTW88(REG64C, ReadTW88(REG648) & ~0xC0);
		break;

	case INPUT_LVDS:
		EnableExtLvdsTxChip(ON);	//GPIO EXPANDER IO[4]
		WriteTW88(REG648, 0x07);
		WriteTW88(REG649, 0x01);
		WriteTW88(REG64A, 0x00);
		WriteTW88(REG64B, 0x30);
		WriteTW88(REG64C, 0x40);
		WriteTW88(REG64D, 0x17);
		WriteTW88(REG64E, 0x00);
		break;
	}

	//scaler
	ScalerSetFreerunManual( OFF );		//component,pc,dvi removed

	//measure
	switch (InputMain)
	{
	case INPUT_CVBS:
	case INPUT_SVIDEO:
	case INPUT_COMP:
	case INPUT_PC:
		//BKFYI. CVBS&SVIDEO was MeasSetWindow( 0, 0, 2200,1125 );
		MeasSetWindow( 0, 0, 0xfff, 0xfff );	//set dummy window. 1600x600
		MeasSetField(2);						// field:Both
		MeasEnableDeMeasure(OFF);				// Disable DE Measure
		MeasSetThreshold(0x40);					// Threshold active detection
		break;
	case INPUT_DVI:
	case INPUT_HDMIPC:
	case INPUT_HDMITV:
	case INPUT_LVDS:
		MeasSetWindow( 0, 0, 0xfff, 0xfff );	//set dummy window. 1600x600
		MeasSetField(2);						//field:Both
		MeasEnableDeMeasure(ON);				//Enable DE Measure
		MeasSetThreshold(0x40);					//Threshold active detection
		MeasSetErrTolerance(4);					//tolerance set to 32
		MeasEnableChangedDetection(ON);			//set EN. Changed Detection
		break;
	case INPUT_BT656:

//		TEMP_init_BT656(0);
//		TEMP_init_BT656(1);
//		TEMP_init_BT656(2);
//		TEMP_init_BT656(6);
		MeasSetWindow( 0, 0, 0xfff, 0xfff );	//set dummy window. 1600x600
		MeasSetField(2);						//field:Both
		MeasEnableDeMeasure(ON);				//Enable DE Measure
		MeasSetThreshold(0x40);					//Threshold active detection
		MeasSetErrTolerance(4);					//tolerance set to 32
		MeasEnableChangedDetection(ON);			//set EN. Changed Detection
		break;
	}

	//image effect
	SetImage(InputMain);	//set saved image effect(contrast,....)

	//---------------------------
	//BT656 Output
	//---------------------------
}

//-----------------------------------------------------------------------------
/**
* enable video Output 
*
* call when CheckAndSet[Input] is successed.
*/
extern BYTE SW_Video_Status;
void VInput_enableOutput(BYTE fRecheck)
{
	if(fRecheck) {
		dPrintf("====Found Recheck:%d",VH_Loss_Changed);
		// do not turn on here. We need a retry.
	}
	else {
		ScalerSetMuteManual( OFF );		//TurnOn Video Output. Remove Black Video
		ScalerSetFreerunManual( OFF );	//FreeRunOff,UseVideoInputSource
		ScalerSetFreerunValue(0);		//caclulate freerun value

		SpiOsdSetDeValue();
		FOsdSetDeValue();

		LedBackLight(ON);				//TurnOn Display
	}
	TaskNoSignal_setCmd(TASK_CMD_DONE);
	
	Interrupt_enableVideoDetect(ON);

	//BK130204. enable sync on HDMI. if it is a PC mode, we need a sync.
	if(InputMain == INPUT_DVI
	//|| InputMain == INPUT_HDMIPC
	//|| InputMain == INPUT_HDMITV
	|| InputMain == INPUT_BT656
	|| InputMain == INPUT_LVDS ) {	
		//digital input.
		; //SKIP
	}
	else
		Interrupt_enableSyncDetect(ON);


	//BK130102
	if(InputMain == INPUT_LVDS )	
		SW_Video_Status = 1;
}

//-----------------------------------------------------------------------------
/**
* goto Freerun move
*
* call when CheckAndSet[Input] is failed.
* oldname: VInputGotoFreerun
* input
*	reason
*		0: No Signal
*		1: No STD
*		2: Out of range
*/
void VInput_gotoFreerun(BYTE reason)
{
	ScalerCheckPanelFreerunValue();

	//Freerun
	if(InputMain == INPUT_BT656) {
		//??WHY
	}
	else {
		DecoderFreerun(DECODER_FREERUN_60HZ);
	}
	ScalerSetFreerunManual( ON );

	if(InputMain == INPUT_HDMIPC 
	|| InputMain == INPUT_HDMITV
	|| InputMain == INPUT_BT656
	|| InputMain == INPUT_LVDS) {
		//??WHY
	}
	else {
		ScalerSetMuteManual( ON );
	}
	// Prepare NoSignal Task...
	if(reason==0 && MenuGetLevel()==0) { //0:NoSignal 1:NO STD,...
		if(access) {
			FOsdSetDeValue();
			FOsdIndexMsgPrint(FOSD_STR2_NOSIGNAL);
			tic_task = 0;

//#ifdef NOSIGNAL_LOGO
//			if(getNoSignalLogoStatus() == 0)
//				InitLogo1();						
//#endif
			TaskNoSignal_setCmd(TASK_CMD_WAIT_VIDEO);
		}
	}

	if(InputMain == INPUT_PC) {
		//BK111019. I need a default RGB_START,RGB_VDE value for position menu.
		RGB_HSTART = InputGetHStart();
		RGB_VDE = ScalerReadVDEReg();
	}

	LedBackLight(ON);

	Interrupt_enableVideoDetect(ON);
}




