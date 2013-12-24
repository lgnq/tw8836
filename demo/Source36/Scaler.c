/**
 * @file
 * scaler.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	video scaler module 
 *
 * input => (scale down) => line buff => (scale up) =>  output	panel
 */

#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"
#include "Global.h"

//#include "main.h"
#include "Printf.h"
#include "Monitor.h"
#include "I2C.h"
#include "CPU.h"
#include "SOsd.h"
#include "FOsd.h"

#include "InputCtrl.h"
#include "aRGB.h"
#include "PC_modes.h"
#include "measure.h"
#include "settings.h"
#define INCLUDE_DTV_TABLE
#include "scaler.h"
#include "eeprom.h"
#include "util.h"

#include "decoder.h"
#include "dtv.h"
#include "main.h"

BYTE VideoAspect;

#define XSCALE_DN_BASE	0x0400L
#define XSCALE_UP_BASE	0x2000L	
#define YSCALE_BASE		0x2000L
#define PXSCALE_BASE	0x0800L

#define SCALER_HDE_BASE			0x30
#define SCALER_HDELAY2_BASE		0x10

//=============================================================================
// Register Functions
//=============================================================================

//static function prototypes

//-----------------------------------------------------------------------------
//LNFIX		R201[2]		on/off
//			1 = Fix the scaler output line number defined by register LNTT.
//			0 = Output line number determined by scaling factor.
//LNTT		R20D[7:6]R219[7:0]	lines
//			It controls the scaler total output lines when LNFIX=1. It is used in special case. A 10-bit register.
//other name: Limit V Total
#ifdef UNCALLED_SEGMENT
void ScalerSetOutputFixedVline(BYTE onoff)
{
	WriteTW88Page(PAGE2_SCALER);

	if (onoff)
		WriteTW88(REG201, ReadTW88(REG201) | 0x04);
	else
		WriteTW88(REG201, ReadTW88(REG201) & 0xFB);
}
#endif

//register
//	R202[5:0]
#ifdef UNCALLED_SEGMENT
//-----------------------------------------------------------------------------
void ScalerSetFieldOffset(BYTE fieldOffset)
{
	WriteTW88Page(PAGE2_SCALER);
	WriteTW88(REG202, (ReadTW88(REG202) & 0xC0) | filedOffset);
}
#endif

//-------------------------------------
//Scaler scale ratio
//-------------------------------------

/*
XSCALE
*	register
*	R204[7:0]R203[7:0]	XSCALE UP
*	R20A[3:0]R209[7:0]	X-Down
*/

/**
* write Horizontal Up Scale register
*
* Up scaling ratio control in X-direction. 
* A 16-bit register. 
* The scaling ratio is defined as 2000h / XSCALE
*
*	register
*	R204[7:0]R203[7:0]	XSCALE UP
*/
void ScalerWriteXUpReg(WORD value)
{
	WriteTW88Page(PAGE2_SCALER);

	WriteTW88(REG204, (BYTE)(value>>8));		
	WriteTW88(REG203, (BYTE)value);
}

#ifdef UNCALLED_SEGMENT
WORD ScalerReadXUpReg(void)
{
	WORD wValue;

	WriteTW88Page(PAGE2_SCALER);
	Read2TW88(REG204,REG203, wValue);

	return wValue;
}
#endif

/**
* write Horizontal Down Scale register
*
* Down scaling ratio control in X-direction. 
* A 12-bit register. 
* The down scaling ratio is defined as 400h / HDSCALE
*	register
*	R204[7:0]R203[7:0]	XSCALE UP
*	R20A[3:0]R209[7:0]	X-Down
*/
void ScalerWriteXDownReg(WORD value)
{
	WriteTW88Page(PAGE2_SCALER);

	WriteTW88(REG20A, (ReadTW88(REG20A) & 0xF0) | (value >> 8));
	WriteTW88(REG209, (BYTE)value);
}

#if defined(SUPPORT_PC) || defined(SUPPORT_DVI) || defined(SUPPORT_HDMI)
//-----------------------------------------------------------------------------
/**
* read Horizontal Down Scale register
*
* @see ScalerWriteXDownReg
*/
#ifndef PANEL_AUO_B133EW01
WORD ScalerReadXDownReg(void)
{
	WORD HDown;

	WriteTW88Page(PAGE2_SCALER);

	HDown = ReadTW88(REG20A) & 0x0F;
	HDown <<= 8;
	HDown += ReadTW88(REG209 );

	return HDown;
}
#endif
#endif

#ifdef UNCALLED_SEGMENT
void ScalerSetHScaleReg(WORD down, WORD up)
{
	ScalerWriteXDownReg(down);
	ScalerWriteXUpReg(up);
}
#endif

//internal
/**
* set Horizontal Scaler value on FULL mode
*
* @see ScalerSetHScale
*/
void ScalerSetHScale_FULL(WORD Length)	
{
	DWORD	temp;

	//BK130131. Now HDE need some min value, because FOSD DE.
	//LineBuffDelay also related with HDE.
	//I temporary increase it.

	ScalerWriteLineBufferDelay(SCALER_HDELAY2_BASE);
	ScalerPanoramaOnOff(OFF);

	WriteTW88Page(PAGE2_SCALER);
	if (PANEL_H >= Length)
	{ 					
		//UP SCALE
		temp = Length * 0x2000L;
		//temp += (PANEL_H/2);			//roundup	
		temp /= PANEL_H;
		ScalerWriteXUpReg(temp);				//set up scale
		ScalerWriteXDownReg(0x0400);			//clear down scale
//		dPrintf("\nScalerSetHScale(%d) DN:0x0400 UP:0x%04lx",Length, temp);
		if (InputMain==INPUT_COMP || InputMain==INPUT_DVI || InputMain==INPUT_HDMITV || InputMain==INPUT_HDMIPC || InputMain==INPUT_LVDS)
			ScalerSetLineBufferSize(Length);
		
		if (InputMain==INPUT_PC)
			ScalerSetLineBufferSize(PANEL_H);
	}
	else
	{										
		//DOWN SCALE
		if (InputMain==INPUT_PC || InputMain==INPUT_COMP || InputMain==INPUT_DVI || InputMain==INPUT_HDMITV || InputMain==INPUT_HDMIPC || InputMain==INPUT_LVDS)
		{
			temp = Length * 0x0400L;						
			temp += 0x0200L;	//roundup	
		}
		else
		{
			Length++;		//BK110613
			temp = Length * 0x0400L;
		}						
		temp /= PANEL_H;
		ScalerWriteXUpReg(0x2000);			//clear up scale
		ScalerWriteXDownReg(temp);			//set down scale
//		dPrintf("\nScalerSetHScale(%d) DN:0x%04lx UP:0x2000",Length, temp);
		if (InputMain==INPUT_COMP || InputMain==INPUT_DVI || InputMain==INPUT_PC || InputMain==INPUT_HDMIPC || InputMain==INPUT_HDMITV || InputMain==INPUT_LVDS)
			ScalerSetLineBufferSize(PANEL_H);
	}
}

//internal
/**
* set Horizontal Scaler value on Panorama mode
*
* @see ScalerSetHScale
*/
void ScalerSetHScale_Panorama(WORD Length)	
{
	DWORD	temp;
	WORD	X1;
	WORD 	linebuff;

	X1 = Length;
	X1 += 32;

	WriteTW88Page(PAGE2_SCALER);
	if (PANEL_H >= X1)
	{
		//
		//UP SCALE
		//
		X1 = Length;
		X1 += 34;				//32+2
		linebuff = Length+1;

//		dPrintf("\nScalerSetHScale(%d->%d) ",Length,X1); 

		temp = X1 * 0x2000L;					//8192
		temp /= PANEL_H;						//800
		ScalerWriteXUpReg(temp);				//set up scale
		ScalerWriteXDownReg(0x0400);			//clear down scale
//		dPrintf("DN:0x0400 UP:0x%04lx lbuff:%d", temp,linebuff);
		ScalerSetLineBufferSize(linebuff);
	}
	else
	{
		//
		// DOWN SCALE
		//
		linebuff = PANEL_H - 34*2;				//(32+2)*2
		temp = Length * 0x0400L;				//1024						
		temp /= (linebuff - 1);					//target 800->731
		ScalerWriteXUpReg(0x2000);				//clear up scale
		ScalerWriteXDownReg(temp);				//set down scale
//		dPrintf("DN:0x%04lx UP:0x2000 lbuff:%d", temp,linebuff);
		ScalerSetLineBufferSize(linebuff+1);
	}
	
	ScalerSetPanorama(0x400, 0x20);
	ScalerPanoramaOnOff(ON);
}

/**
* set Horizontal Scaler value with ratio
*
*/
void ScalerSetHScaleWithRatio(WORD Length, WORD ratio)	
{
	DWORD temp;
	WORD new_Length;

//	dPrintf("\nScalerSetHScaleWithRatio(%d,%d)",Length,ratio);
	ScalerWriteLineBufferDelay(SCALER_HDELAY2_BASE);
	ScalerPanoramaOnOff(OFF);

	WriteTW88Page(PAGE2_SCALER);

	temp = Length;
	temp *= ratio;
	temp /= 100;	//new length
	new_Length = temp;

	dPrintf("\nHLength %d->%d", Length, new_Length);
	if (ratio < 100)
	{
		//down scale
		ScalerWriteXUpReg(0x2000);			//clear up scale
		temp = 0x0400L;
		temp *= ratio;
		temp /= 100;
		ScalerWriteXDownReg(temp);
	}
	else
	{
		//upscale
		ScalerWriteXDownReg(0x0400);		// clear down scale
		temp = 0x2000L;
		temp *= ratio;
		temp /= 100;
		ScalerWriteXUpReg(temp);
	}

	if (new_Length < PANEL_H)
	{
		//adjust buffer output delay
		ScalerWriteLineBufferDelay((PANEL_H - new_Length) / 2 +SCALER_HDELAY2_BASE);
	}
}

/**
* set Horizontal Scaler value
*
* @see ScalerSetHScale_FULL
* @see ScalerSetHScale_Panorama
*/
void ScalerSetHScale(WORD Length)	
{
	if (InputMain == INPUT_PC)
		VideoAspect = GetAspectModeEE();	//BK1100914

	if ((InputMain == INPUT_CVBS || InputMain == INPUT_SVIDEO) && VideoAspect == VIDEO_ASPECT_NORMAL)
	{
		ScalerWriteLineBufferDelay(SCALER_HDELAY2_BASE);	//BK110916 test. Normal needs it
		ScalerPanoramaOnOff(OFF);

		WriteTW88Page(PAGE2_SCALER);

		//only at CVBS
		ScalerWriteXDownReg(0x0400);		//clear down scale
		ScalerWriteXUpReg(0x2000);			//clear up scale
		//adjust buffer output delay
		ScalerWriteLineBufferDelay((PANEL_H - Length) / 2 +SCALER_HDELAY2_BASE);

		ScalerSetLineBufferSize(Length); //BK120111
	}
	else if (VideoAspect == VIDEO_ASPECT_PANO)
		ScalerSetHScale_Panorama(Length);
	else 
		//	 VideoAspect == VIDEO_ASPECT_FULL
		//of VideoAspect == VIDEO_ASPECT_ZOOM
		ScalerSetHScale_FULL(Length);
}

//YSCALE
/**
* Up / down scaling ratio control in Y-direction. 
*
* The scaling ratio is defined as 2000h / YSCALE.
* A 16-bit register. 
*
*	register
*	R206[7:0]R205[7:0]	YSCALE	
*/
void ScalerWriteVScaleReg(WORD value)
{
	WriteTW88Page(PAGE2_SCALER);

	WriteTW88(REG206, (BYTE)(value>>8));
	WriteTW88(REG205, (BYTE)value);
}

/**
* read Vertical Scale register value
*
* @see ScalerWriteVScaleReg
*/
WORD ScalerReadVScaleReg(void)
{
	WORD VScale;

	WriteTW88Page(PAGE2_SCALER);

	Read2TW88(REG206,REG205, VScale);

	return VScale;
}

/**
* set Vertical Scale with Ratio
*/
void ScalerSetVScaleWithRatio(WORD Length, WORD ratio)
{
	DWORD temp;
	WORD new_Length;

//	dPrintf("\nScalerSetVScaleWithRatio(%d,%d)",Length,ratio);
	temp = Length;
	temp *= ratio;
	temp /= 100;	//new length
	new_Length = temp - Length;			//offset.
	new_Length = Length - new_Length;	//final 

	dPrintf("\nVLength %d->%d", Length, new_Length);

	temp = new_Length * 0x2000L;
	temp /= PANEL_V;
	ScalerWriteVScaleReg(temp);
}

/**
* set Vertical Scale
*
* output_length = input_length * 0x2000 / YSCALE_REG
* YSCALE_REG = input_length * 0x2000 / output_length
* YSCALE_REG = input_length * 0x2000 / PANEL_V
* YSCALE_REG = (input_length * 0x2000 + r) / PANEL_V. r = PANEL_V/2
*/
void ScalerSetVScale(WORD Length)
{
	DWORD temp;

	WriteTW88Page(PAGE2_SCALER);

    if ((InputMain==INPUT_CVBS || InputMain==INPUT_SVIDEO) && VideoAspect == VIDEO_ASPECT_ZOOM)
	{
		//rate 720->800
		//
		dPrintf("\nLength:%d",Length);
		temp = Length;	
		temp = temp * 800 / 720;
		temp = temp - Length;	
		Length = Length - temp;
		dPrintf("=>%d",Length);
	}
	//else 
	{
		temp = Length * 0x2000L;
		if (/*InputMain==INPUT_PC */ InputMain ==INPUT_COMP || InputMain ==INPUT_DVI)
		{
			if (Length > PANEL_V)
			{		// down scaling //BK110916??
				temp += 0x1000L;			// round up.
			}
		}
		else
		{
			//temp += (PANEL_V / 2);	//roundup
		}
		temp /= PANEL_V;
	
//		dPrintf("\nScalerSetVScale(%d) 0x%04lx",Length, temp);
	
		ScalerWriteVScaleReg(temp);
	}
}

//-------------------------------------
//Scaler Panorama
//-------------------------------------

/**
* set Panorama mode
*
*	register
*	R201[6]	Enable Panorama/waterglass display
*/
void ScalerPanoramaOnOff(BYTE fOn)
{
	WriteTW88Page(PAGE2_SCALER);

	if (fOn)
		WriteTW88(REG201, ReadTW88(REG201) | 0x40);
	else
		WriteTW88(REG201, ReadTW88(REG201) & ~0x40);
}

/**
* set the panorama parameters
*
* @param px_scale
* @px_inc 
*
*	register
*	R207[7:0]R212[7:4]	PXSCALE.
*	R217[7:4]R208[7:0]	PXINC.	2's complement.
*/
void ScalerSetPanorama(WORD px_scale, short px_inc)
{
	BYTE temp;

	WriteTW88Page(PAGE2_SCALER);

	WriteTW88(REG207, px_scale >> 4);
	WriteTW88(REG212, (ReadTW88(REG212) & 0x0F) | (BYTE)(px_scale & 0x0F));

	temp = (BYTE)(px_inc >> 8);
	temp <<= 4;

	WriteTW88(REG217, (ReadTW88(REG217) & 0x0F) | temp);
	WriteTW88(REG208, (BYTE)px_inc);
}

//-------------------------------------
//	Scaler LineBuffer
//-------------------------------------
//Output Buffer Delay R20B[7;0]
//Output Buffer Length R20E[6:4]R20C[7:0]
//See the Horizontal Timing on "AN-TW8832,33 Scaler & TCON".
//set the "Output Delay" and "Output Length" of the Line Buffer Output on Horizontal Timming Flow.

/**
* Write scaler LineBuffer output delay
*
* HDE is related with this delay value.
*
*	register
*	R20B
*	R20B[7:0]			HDELAY2	
*		Scaler buffer data output delay in number of pixels in relation to the H sync.
*/
void ScalerWriteLineBufferDelay(BYTE delay)
{
	WriteTW88Page(PAGE2_SCALER);
	
	WriteTW88(REG20B, delay);
}

/**
* Read scaler LineBuffer output delay
*
* @see ScalerWriteLineBufferDelay
*/
BYTE ScalerReadLineBufferDelay(void)
{
	WriteTW88Page(PAGE2_SCALER);

	return ReadTW88(REG20B);
}

/**
* set Scaler OutputLength that is related with the line buffer size. 
*
* max 1024.
* 
*	register
*	R20E[6:4]:R20C[7:0]		HACTIVE
*		Scaler data output length in number of pixels. 
*		A 10-bit register.==>11
*/
void ScalerSetLineBufferSize(WORD len)
{
	if (len > PANEL_H)
		len = PANEL_H;

	WriteTW88Page(PAGE2_SCALER);

	WriteTW88(REG20E, (ReadTW88(REG20E) & 0x8F) | ((len & 0x700) >> 4));
	WriteTW88(REG20C, (BYTE)len);
}

//#ifdef UNCALLED_SEGMENT
////-----------------------------------------------------------------------------
//void ScalerSetLineBuffer(BYTE delay, WORD len)
//{
//	ScalerWriteLineBufferDelay(delay);
//	ScalerSetLineBufferSize(len);
//}
//#endif

//only for LCOS
//#if 0
////-----------------------------------------------------------------------------
//void ScalerSetFPHSOutputPolarity(BYTE fInvert)
//{
//	WriteTW88Page(PAGE2_SCALER);
//	if(fInvert) WriteTW88(REG20D, ReadTW88(REG20D) |  0x04);
//	else		WriteTW88(REG20D, ReadTW88(REG20D) & ~0x04);
//}
//#endif

//HPADJ		R20E[3:0]R20F[7:0]
//			Blanking H period adjustment. A 12-bit 2's complement register
//#ifdef UNCALLED_SEGMENT
////-----------------------------------------------------------------------------
//void ScalerWriteOutputHBlank(WORD length)
//{
//	WriteTW88Page(PAGE2_SCALER );
//	WriteTW88(REG20E, (ReadTW88(REG20E) & 0xF0) | (length >> 8) );
//	WriteTW88(REG20F, (BYTE)length);
//}
//#endif

/**
* set Horizontal DE position(DEstart) & length(active, DEwidth).
*
*	REG221[5:4]R210[7:0]	HA_POS
*				Output DE position control relative to the internal reference in number of output clock
* set the "DEstart" and "DEwidth" on Horizontal Timming Flow.
*
* This register is related with HDELAY2.
* We have to control it together.
*/
void ScalerWriteHDEReg(WORD pos)
{
	//do not add debugmsg, it will makes a blink.
	//dPrintf("\nScalerWriteHDEReg pos:%bd",pos);
	WriteTW88Page(PAGE2_SCALER);

	WriteTW88(REG221, (ReadTW88(REG221) & 0xCF) | ((pos >> 4) & 0x30));
	WriteTW88(REG210, (BYTE)pos);
}

/**
* read HDE register
*
* @see ScalerWriteHDEReg
*/
WORD ScalerReadHDEReg(void)
{
	WORD wTemp;

	WriteTW88Page(PAGE2_SCALER);

	wTemp = ReadTW88(REG221) & 0x30; wTemp <<= 4;
	wTemp |= ReadTW88(REG210);

	return wTemp;
}

/**
* Calculate HDE value.
*
*	method
*	Buffer_Delay = REG(0x20b[7:0])
*	result = Buffer_Delay + 32
*/
WORD ScalerCalcHDE(void)
{
	WORD wTemp;

	wTemp = ScalerReadLineBufferDelay();

	return wTemp+32;
}

//-----------------------------------------------------------------------------
//register
//	R212[3:0]R211[7:0]	HA_LEN
//						Output DE length control in number of the output clocks. A 12-bit register
//						output height. normally PANEL_V
/**
* Read output witdh
*/
WORD ScalerReadOutputWidth(void)
{
	WORD HActive;

	WriteTW88Page(PAGE2_SCALER);

	HActive = ReadTW88(REG212) & 0x0F;
	HActive <<= 8;
	HActive |= ReadTW88(REG211);

	return HActive;
}

//set Scaler.Output.Width
//#ifdef UNCALLED_SEGMENT
////-----------------------------------------------------------------------------
//void ScalerWriteOutputWidth(WORD width)
//{
//	WriteTW88Page(PAGE2_SCALER );
//	WriteTW88(REG212, (ReadTW88(REG212) & 0xF0) | (BYTE)(width>>8));
//	WriteTW88(REG211, (BYTE)width);
//}
//#endif

//HS_POS	R221[1:0]R213[7:0]	HSynch pos
//			Output H sync position relative to internal reference in number of output clocks.
//HS_LEN	R214[3:0]	HSynch width
//			Output H sync length in number of output clocks
//HSstart 
//HSwidth
//set the "HSstart" and "HSwidth" on Horizontal Timming Flow.
// Scaler set output HSynch
#ifdef UNCALLED_SEGMENT
//-----------------------------------------------------------------------------
void ScalerSetHSyncPosLen(WORD pos, BYTE len)
{
	WriteTW88Page(PAGE2_SCALER);
	
	WriteTW88(REG213, (BYTE)pos);
	WriteTW88(REG221, (ReadTW88(REG221) & 0xFC) | (pos>>8));
	WriteTW88(REG214, (ReadTW88(REG214) & 0xF0) | len);
}
#endif

//VA_POS	REG220[5:4]R215[7:0]		VDE pos 
//			Output DE position control relative to the internal reference in number of output lines
//VA_LEN	R217[3:0]R216[7:0]	  width
//			Output DE control in number of the output lines. A 12-bit register

/**
* Read Vertical DE register
*/
WORD ScalerReadVDEReg(void)
{
	WORD wTemp;

	WriteTW88Page(PAGE2_SCALER);

	wTemp = ReadTW88(REG220) & 0x30;
	wTemp <<= 4;
	wTemp |= ReadTW88(REG215);

	return wTemp;
}

/**
* Write Vertical DE register
*/
void ScalerWriteVDEReg(WORD pos)
{
	//dPrintf("\nScalerSetVDEAndWidth pos:%bd len:%d",pos,len);
	WriteTW88Page(PAGE2_SCALER);

	WriteTW88(REG220, (ReadTW88(REG220) & 0xCF) | ((pos >> 4) & 0x30));
	WriteTW88(REG215, (BYTE)pos);
}

//-----------------------------------------------------------------------------
//register
//	R217[3:0]R216[7:0]	output width. normally PANLE_H+1
#ifdef UNCALLED_SEGMENT
void ScalerWriteOutputHeight(WORD height)
{
	WriteTW88Page(PAGE2_SCALER );
	WriteTW88(REG217, (ReadTW88(REG217) & 0xF0) | (BYTE)(height>>8));
	WriteTW88(REG216, (BYTE)height);
}
#endif

/**
* Read Outout Height register
*/
WORD ScalerReadOutputHeight(void)
{
 	WORD height;

	WriteTW88Page(PAGE2_SCALER );

	height = ReadTW88(REG217) & 0x0F;
	height <<= 8;
	height |= ReadTW88(REG216);	//V Width

	return height;
}

#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC) || defined(SUPPORT_DVI)
//-----------------------------------------------------------------------------
/**
* Calculate VDE value.
*
*	method
*	VStart = REG(0x536[7:0],0x537[7:0])
*	VPulse = REG(0x52a[7:0],0x52b[7:0])
*	VPol = REG(0x041[3:3])
*	VScale = REG(0x206[7:0],0x205[7:0])
*	result = ((VStart - (VPulse * VPol)) * 8192 / VScale) + 1
*/
#if 0
WORD ScalerCalcVDE(void)
{
	BYTE VPol;
	WORD VStart,VPulse,VScale;
	DWORD dTemp;;

	WriteTW88Page(PAGE5_MEAS);
	Read2TW88(REG536,REG537, VStart);	//incorrect....
	Read2TW88(REG52A,REG52B, VPulse);

#ifdef SUPPORT_DVI
	if(InputMain==INPUT_DVI) {
		WriteTW88Page(PAGE0_DTV);
		VPol = ReadTW88(REG050) & 0x02 ? 1: 0;
	}
	else 
#endif
	{
		WriteTW88Page(PAGE0_INPUT);
		VPol = ReadTW88(REG041) & 0x08 ? 1: 0;
	}
	VScale = ScalerReadVScaleReg();

	dTemp = VStart;
	if(VPol) {
		if(dTemp < VPulse) {
			ePrintf("\nBugBug: dTemp:%ld < VPulse:%d",dTemp,VPulse);
			WriteTW88Page(PAGE5_MEAS);
			Read2TW88(REG538,REG539, VStart);
			dTemp = VStart;
		}

		dTemp -= VPulse;
	}
	dTemp = (dTemp * 8192L) / VScale;
	dTemp++;
	
	return (WORD)dTemp;
}
#endif

WORD ScalerCalcVDE2(WORD vStart, char vde_offset)
{
	BYTE VPol;
	WORD VPulse,VScale;	 //VStart
	DWORD dTemp;;

	Printf("\nvBPorch:%d, offset:%bd",vStart,vde_offset);

	WriteTW88Page(PAGE5_MEAS);
	//Read2TW88(REG536,REG537, VStart);	//incorrect....
	Read2TW88(REG52A, REG52B, VPulse);

	WriteTW88Page(PAGE0_INPUT);
	VPol = ReadTW88(REG041) & 0x08 ? 1: 0;

	VScale = ScalerReadVScaleReg();

	dTemp = vStart;
	dTemp = (dTemp * 8192L) / VScale;
	dTemp += vde_offset;
	
	Printf(" vDe:%d",(WORD)dTemp);

	return (WORD)dTemp;
}
#endif

//-----------------------------------------------------------------------------
//set the "VDEstart" and "VDEwidth" on Vertical Timming Flow.
#if 0
void ScalerSetVDEPosHeight(BYTE pos, WORD len)
{
	dPrintf("\nScalerSetVDEPosHeight pos:%bd len:%d",pos,len);
	ScalerWriteVDEReg(pos);
	ScalerWriteOutputHeight(len);
}
#endif

//-----------------------------------------------------------------------------
//register
//	R212[3:0]R211[7:0]	HA_LEN
//						Output DE length control in number of the output clocks. A 12-bit register
//						output height. normally PANEL_V
//	R217[3:0]R216[7:0]	output width. normally PANLE_H+1
#ifdef UNCALLED_SEGMENT
void ScalerSetOutputWidthAndHeight(WORD width, WORD height)
{
	ScalerWriteOutputWidth(width);
	ScalerWriteOutputHeight(height);
}
#endif

//-----------------------------------------------------------------------------
//VS_LEN		R218[7:6]	VSyhch width
//VS_POS		R220[3:0]R218[5:0]	VSyhch pos
//set the "VSstart" and "VSwidth" on Vertical Timming Flow.
//Scaler set output VSynch
#ifdef UNCALLED_SEGMENT
void ScalerSetVSyncPosLen(WORD pos, BYTE len)
{
	WriteTW88Page(PAGE2_SCALER );
	WriteTW88(REG218, len << 6 | (pos & 0x3F));
	WriteTW88(REG220, ReadTW88(REG220) & 0xF0 | (pos >> 6));
}
#endif

/**
* Write Freerun VTotal value
*
*	R20D[7:6]:R219[7:0]
*/
void ScalerWriteFreerunVtotal(WORD value)
{
	WriteTW88Page(PAGE2_SCALER);
	
	WriteTW88(REG20D, (ReadTW88(REG20D)&0x3F)|(value>>2)&0xC0);
	WriteTW88(REG219, (BYTE)value );
}

/**
* Read Freerun VTotal value
*
* @see ScalerWriteFreerunVtotal
*/
WORD ScalerReadFreerunVtotal(void)
{
	WORD value;

	WriteTW88Page(PAGE2_SCALER );

	value = ReadTW88(REG20D) & 0xC0;
	value <<= 2;
	value |= ReadTW88(REG219);

	return value;
}

/**
* calcualte Freerun VTotal value
*
*	method
*	VPN    = REG(0x522[7:0],0x523[7:0])
*	Vscale = REG(0x206[7:0],0x205[7:0])
*	result = VPN / (Vscale / 8192)
*
* NOTE: It needs a MeasStart.
*/
WORD ScalerCalcFreerunVtotal(void)
{
	WORD VScale;
	DWORD temp32;
	
	VScale = ScalerReadVScaleReg();
	
	temp32 = MeasGetVPeriod();
	temp32 *= 8192L;
	temp32 /= VScale;
	
	return (WORD)temp32;
}

//-----------------------------------------------------------------------------
//DM_TOP	R21A[7:0]	top 	 
//DM_BOT	R21B[7:0]	bottom 
//set number of data masked lines from the top of DE and the end of DE.
#ifdef UNCALLED_SEGMENT
void ScalerSetVDEMask(BYTE top, BYTE bottom)
{
	WriteTW88Page(PAGE2_SCALER );
	WriteTW88(REG21A, top);
	WriteTW88(REG21B, bottom);
}
#endif

/**
* Write Freerun Htotal value 
*
*	R21C[7:4]:R21D[7:0]
*/
void ScalerWriteFreerunHtotal(WORD value)
{
	WriteTW88Page(PAGE2_SCALER);

	WriteTW88(REG21C, (ReadTW88(REG21C)&0x0F)|(value>>4)&0xF0);
	WriteTW88(REG21D, (BYTE)value);
}

/**
* read Freerun HTotal value
*
* @see ScalerWriteFreerunHtotal
*/
WORD ScalerReadFreerunHtotal(void)
{
	WORD value;

	WriteTW88Page(PAGE2_SCALER);

	value = ReadTW88(REG21C) & 0xF0;
	value <<= 4;
	value |= ReadTW88(REG21D);

	return value;
}

/**
* calculate the Freerun HTotal value
*
*	method
*	HPN    = REG(0x524[7:0],0x525[7:0])
*	Vscale = REG(0x206[7:0],0x205[7:0])
*	PCLKO  = REG(0x20d[1:0])
*	result = (HPN * Vscale / 8192) / (PCLKO+1)
*/
WORD ScalerCalcFreerunHtotal(void)
{
	WORD VScale;
	BYTE PCLKO;
	DWORD temp32;

	VScale = ScalerReadVScaleReg();
	WriteTW88Page(PAGE2_SCALER);
	PCLKO = ReadTW88(REG20D)&0x03;

	temp32 = MeasGetHPeriod();
	temp32 *= VScale;
	temp32 /= 8192L;
	temp32 /= (PCLKO+1);

	dPrintf("\nScalerCalcFreerunHtotal temp:%ld",temp32);
	
	return (WORD)temp32;
}

//RRUN		R21C[2]
//			Panel free run control. 1 = free run with HTOTAL and LNTT.
//-----------------------------------------------------------------------------
/**
* Force scaler data output to all 0’s
*
* comes fromTW8823.
* free run control R21C[2] -Panel free run control. 1 = free run with HTOTAL(R21C[7:4]R21D[7:0] and LNTT(R20D[7:6]R219[7:0]).
* free run on the condition of input loss R21C[1].
*/
void ScalerSetFreerunManual(BYTE on)
{
	WriteTW88Page(PAGE2_SCALER);

	if (on)
		WriteTW88(REG21C, (ReadTW88(REG21C) | 0x04));			//on manual freerun
	else
		WriteTW88(REG21C, (ReadTW88(REG21C) & ~0x04));		//off manual freerun
}

#ifdef UNCALLED_CODE
BYTE ScalerIsFreerunManual(void)
{
	BYTE value;

	WriteTW88Page(PAGE2_SCALER);

	value = ReadTW88(REG21C);

	if (value & 0x04)
		return 1;

	return 0;
}
#endif

/**
* set FreerunAuto and FreerunManual
*/
void ScalerSetFreerunAutoManual(BYTE fAuto, BYTE fManual)
{
	BYTE value;
	
	WriteTW88Page(PAGE2_SCALER);
	value = ReadTW88(REG21C);
	if (fAuto != 0x02)
	{
		if (fAuto)
			value |= 0x02;		//on auto freerun
		else
			value &= ~0x02;		//off auto freerun		
	}
	if (fManual != 0x02)
	{
		if (fManual)
			value |= 0x04;		//on manual freerun
		else
			value &= ~0x04;		//off manual freerun		
	}
	WriteTW88(REG21C, value);
}

/**
* set MuteAuto & MuteManual
*/
void ScalerSetMuteAutoManual(BYTE fAuto, BYTE fManual)
{
	BYTE value;
	
	WriteTW88Page(PAGE2_SCALER );
	value = ReadTW88(REG21E);
	if (fAuto != 0x02)
	{
		if (fAuto)
			value |= 0x02;		//on auto mute
		else
			value &= ~0x02;		//off auto mute		
	}
	if (fManual != 0x02)
	{
		if (fManual)
			value |= 0x01;		//on manual mute
		else
			value &= ~0x01;		//off manual mute		
	}
	WriteTW88(REG21E, value);
}

/**
* set MuteManual
*/
void ScalerSetMuteManual(BYTE on)
{
	WaitOneVBlank();
	WriteTW88Page(PAGE2_SCALER);

	if (on)
		WriteTW88(REG21E, (ReadTW88(REG21E) | 0x01));			//on manual mute
	else
		WriteTW88(REG21E, (ReadTW88(REG21E) & ~0x01));		//off manual mute
}

//PanelFreerun value
//
//
//      component					CVBS
//mode	Htotal	Vtotal XYScale		Htotal	Vtotal	XYScale
//----	------	------ -------		------	------	-------
//480i	1069	551	   1B5C	0F33	1069	551(553)1C00 0F33
//576i	1294	548	   1B5C	1244	1299	544		1C00 1255
//480p	1069	552
//576p	1292	548
//1080i	1122	525
//720p	1122	526
//1080p	????	???
//
//BKFYI
//	I saw the measure block can detect the Period value on CVBS & DVI.
//	And, CVBS(SVideo) use 27MKz fixed clock, so I don't need to consider InputMain.
//

//BKFYI:HW default HTOTAL:1024,VTOTAL:512. I don't need a force mode.
//-----------------------------------------------------------------------------
/**
* set Scaler Freerun value
*/
void ScalerSetFreerunValue(BYTE fForce)
{
	WORD HTotal;
	WORD VTotal;
	BYTE ret;

	if (fForce)
	{
		//scaled NTSC Freerun value
		HTotal = 1085;  
		VTotal = 553;
	}
	else
	{
		//Before measure, disable an En.Change Detection. and then start a measure.
		//MeasStartMeasure will capture a reference value for "En.Change detection".
		MeasEnableChangedDetection(OFF);

		//call measure once to update the value or use a table value
		ret = MeasStartMeasure();
		if (ret)
		{
			dPrintf("\nFreerunValue failed!!");
			HTotal = 1085;	//1100;	//1018;	//1107;
			VTotal = 553;	//553;	//542;
		}
		else
		{
			HTotal = ScalerCalcFreerunHtotal();
			VTotal = ScalerCalcFreerunVtotal();
		}
		//turn on the En.Changed Detection.
		MeasEnableChangedDetection(ON);
	}
	
	dPrintf("\nFreerunValue(%bd) HTotal:%d VTotal:%d",fForce,HTotal,VTotal);
	ScalerWriteFreerunHtotal(HTotal);
	ScalerWriteFreerunVtotal(VTotal);
}

/**
* check the freerun value before we go into the freerun mode.
*	
* Hmin = HDE+HWIDTH
*
* Vmin = VDE+VWIdth
*/
void ScalerCheckPanelFreerunValue(void)
{
	WORD Total, Min;
	BYTE changed;

	WriteTW88Page(PAGE2_SCALER);
	changed = 0;

	// Horizontal
	Total = ScalerReadFreerunHtotal();
	Min = ScalerReadOutputWidth();	//H Width
	Min += ScalerReadHDEReg();		//H-DE
	Min += 2;
	if (Total < Min)
	{
		dPrintf("\nScaler Freerun HTotal %d->%d",Total, Min);
		Total = Min;
		ScalerWriteFreerunHtotal(Total);
		changed++;
	}

	//Vertical
	Total = ScalerReadFreerunVtotal();
	Min = ScalerReadOutputHeight();	//V Width
	Min += ScalerReadVDEReg();		//V-DE
	Min += 2;
	if (Total < Min)
	{
		dPrintf("\nScaler Freerun VTotal %d->%d",Total, Min);
		Total = Min;
		ScalerWriteFreerunVtotal(Total);
		changed++;
	}

	if (changed)
	{
		SpiOsdSetDeValue();	//BK111013
		FOsdSetDeValue();
	}
}

//===================================================
// New Scaler routines comes from TW8809.
// Under developing.
//===================================================

/**
* update Scaler ScaleRate
*
* ex: Scaler09SetScaleRate(720,240, 720,480)
*     Scaler09SetScaleRate(1280,720, 800, 480)
*/
void ScalerSetScaleRate(WORD hIn, WORD vIn, WORD hOut, WORD vOut)
{
	DWORD dTemp;
	WORD hdScale,xScale,yScale;

	// X scale
	if (hIn > hOut)
	{
		//X downscale
		xScale = 0x2000;
		dTemp = hIn;
		dTemp *= 0x400L;
		dTemp /= hOut;
		hdScale = dTemp;
	}
	else
	{
		//X upscale
		hdScale = 0x400;
		dTemp = hIn;
		dTemp *= 0x2000L;
		dTemp /= hOut;
		xScale = dTemp;
	}
	// Y scale
	dTemp = vIn;
	dTemp *= 0x2000L;
	dTemp /= vOut;
	yScale = dTemp;

	//update HW register
	WriteTW88Page(2);
	I2C_Buffer[0] = (BYTE)xScale;
	I2C_Buffer[1] = (BYTE)(xScale >> 8);
	I2C_Buffer[2] = (BYTE)yScale;
	I2C_Buffer[3] = (BYTE)(yScale >> 8);
	WriteBlockTW88(REG203, I2C_Buffer, 4);
	I2C_Buffer[0] = (BYTE)(hdScale);
	I2C_Buffer[1] = ReadTW88(REG20A) & 0xF0;
	I2C_Buffer[1] |= (BYTE)(hdScale>>8);
	WriteBlockTW88(REG209, I2C_Buffer, 2);
}

/**
* update Scaler Output.
*
* hSync 		hSync Width
* hBPorch		BackPorch width
* hActive		Active Width
* hFPorch		FrontPorch width
* hPol			hsync polarity
*
*/
#define SCALER_TW8835_HDE_DELAY		32
void ScalerSetOutputTimeRegs(BYTE index, WORD InputVDeStart,WORD InputVDEWidth) 
{
	struct s_DTV_table *pVid;

							//------------------------------
							//TW8809 BT656 Output registers
//	WORD bt656enc_hdly;		//R068{1:0]R06C[7:0] 
//	WORD bt656enc_hact;		//R069[3:0]R06D[7:0]
//	BYTE bt656enc_vdelay;	//         R06A[7:0]
//	WORD bt656enc_vact;		//R069[6:4]R06B[7:0]
							//------------------------------
							//TW8835 Scaler Output registers
	WORD hdelay2; 			//         R20B[7:0]
	WORD hactive2;			//R20E[6:4]R20C[7:0]
	WORD hpadj;				//R20E[3:0]R20F[7:0]
	WORD ha_pos; 			//R211[5:4]R210[7:0]
	WORD ha_len;			//R212[3:0]R211[7:0]
	WORD hs_pos;			//R211[1:0]R213[7:0]
	BYTE hs_len;			//         R214[7:0]
	WORD va_pos;			//R220[5:4]R215[7:0]
	WORD va_len;			//R217{3:0]R216[7:0]
	BYTE vs_len;			//         R218[7:6]
	WORD vs_pos;			//R220[3:0]R218[5:0]

	pVid = &DTV_table[index];
	Printf("\nScalerSetOutputTimeRegs(%bd,%d,%d)",index,InputVDeStart,InputVDEWidth);
	Printf("\nOutput DTV_table[%bd]	",index);
	Printf("%bd %dx%d%s@%bd POL:%02bx   %d,%d,%d,%d   %d,%bd,%bd,%bd,%bd %ld",
		pVid->vid,
		pVid->hDeWidth,pVid->vDeWidth,	pVid->fIorP == 1 ? "I" : "P", 
		pVid->vfreq == FREQ_60 ? 60 : pVid->vfreq == FREQ_50 ? 50 : 0,
		pVid->Pol,
		pVid->hTotal,	(WORD)pVid->hFPorch,(WORD)pVid->hSyncWidth,(WORD)pVid->hBPorch,
		pVid->vTotal,	pVid->vPad,pVid->vSyncStart,pVid->vSyncWidth,pVid->vBPorch,
		pVid->pixelfreq
		);

	//
	//horizontal related value.
	//
	hpadj = DTV_table[index].hFPorch;
	hs_pos=0;	
	hs_len=DTV_table[index].hSyncWidth-1;	//HW starts from 0.
	ha_pos = hs_pos + (hs_len + 1) + DTV_table[index].hBPorch;
	ha_pos -= SCALER_TW8835_HDE_DELAY; /* adjust internal delay. It is a minimum value */
	if(InputMain==INPUT_PC) {
		hs_len = ReadTW88(REG1DC) & 0x3F;
		//ha_pos = DTV_table[index].hBPorch -  ReadTW88(REG1DC) & 0x3F;
		ha_pos = 33;
	}
	ha_len = DTV_table[index].hDeWidth;
#if 0
//	hdelay2 = DTV_table[index].hBPorch;
//	hdelay2 -= 27; /*adjust internal delay */
//	if(InputMain==INPUT_CVBS)
//		hdelay2 -= 2;	
//	else if(index==VID_800X480P_IDX)
//		hdelay2 = (WORD)hdelay2 * 169 / 100;  /* 74175000L / 44000000L */
//	hdelay2 += hs_pos;
#else
	hdelay2 = ha_pos - SCALER_TW8835_HDE_DELAY;
	if(InputMain==INPUT_CVBS) {
		hdelay2 += 3; //BK121129
	}
#endif

	//if(hdelay2 & 0x01) hdelay2+=1;	only for TW8809
	hactive2 = DTV_table[index].hDeWidth;

	//new
	hpadj = pVid->hFPorch;
	hs_pos=0;	
	hs_len=pVid->hSyncWidth-1;	//HW starts from 0.
	ha_pos = hs_pos + (hs_len + 1) + pVid->hBPorch;
	ha_pos -= SCALER_TW8835_HDE_DELAY; /* adjust internal delay. It is a minimum value */
	if(InputMain==INPUT_PC || InputMain==INPUT_COMP) {
		hs_len = ReadTW88(REG1DC) & 0x3F;
		//ha_pos = DTV_table[index].hBPorch -  ReadTW88(REG1DC) & 0x3F;
		ha_pos = 33;
	}
	ha_len = pVid->hDeWidth;
	hdelay2 = ha_pos - SCALER_TW8835_HDE_DELAY;
	if(InputMain==INPUT_CVBS) {
		hdelay2 += 3; //BK121129
	}
	hactive2 = pVid->hDeWidth;



//	bt656enc_hdly = 0;
//	bt656enc_hact = hactive2;

	//
	//vertical related value
	//
	if(index==VID_800X480P_IDX) {
		vs_pos = 0; //DTV_table[index].vPad + DTV_table[index].vSyncStart;
		vs_len = 0; //DTV_table[index].vSyncWidth-1;
	}
	else {
		vs_pos = DTV_table[index].vPad + DTV_table[index].vSyncStart;
		vs_len = DTV_table[index].vSyncWidth-1;
		if(vs_len > 3) {
			vs_len = 3;
			vs_pos += (DTV_table[index].vSyncWidth-1 - 3);
		}
		if(InputMain==INPUT_CVBS) {
			vs_pos += 10;
		}
	}

	if(index==VID_800X480P_IDX) {
#if 0
		va_pos = vs_pos;
		va_pos += (vs_len+1);
		va_pos += DTV_table[index].vBPorch;
		va_pos -= 13; //11
#else		
		//use scaled value.
		//va_pos = vs_len+1;
		//va_pos += DTV_table[index].vBPorch;
		//va_pos *= 480;
		//va_pos /= DTV_table[index].vDeWidth;			
		va_pos = InputVDeStart;
		va_pos *= 480;
		va_pos /= InputVDEWidth;			
#endif
	}
	else {
		va_pos = DTV_table[index].vBPorch;
		if(InputMain==INPUT_CVBS) {
			va_pos += 8;
		}
	}
	va_len = DTV_table[index].vDeWidth;

//	bt656enc_vdelay = DTV_table[index].vPad + DTV_table[index].vSyncStart + DTV_table[index].vSyncWidth;
//	bt656enc_vact = va_len;


	//
	// update HW registers
	//
	WriteTW88Page(2);
	I2C_Buffer[0]  = hdelay2;									//REG20B
	I2C_Buffer[1]  = (BYTE)hactive2;							//REG20C
	I2C_Buffer[2]  = ReadTW88(REG20D);							//REG20D
	I2C_Buffer[3]  = ((hactive2 & 0xF00) >> 4) | (hpadj >> 8);	//REG20E
	I2C_Buffer[4]  = (BYTE)hpadj;								//REG20F
	I2C_Buffer[5]  = (BYTE)ha_pos;								//REG210
	I2C_Buffer[6]  = (BYTE)ha_len;								//REG211
	I2C_Buffer[7]  = (ReadTW88(REG212) & 0xF0) | (ha_len >> 8);	//REG212
	I2C_Buffer[8]  = (BYTE)hs_pos;								//REG213
	I2C_Buffer[9]  = hs_len;									//REG214
	I2C_Buffer[10] = (BYTE)va_pos;								//REG215
	I2C_Buffer[11] = (BYTE)va_len;								//REG216
	I2C_Buffer[12] = (ReadTW88(REG217) & 0xF0) | (va_len >> 8);	//REG217
	I2C_Buffer[13] = (vs_len << 6) | (BYTE)(vs_pos & 0x3F);		//REG218
	WriteBlockTW88(REG20B, I2C_Buffer, 14);

	I2C_Buffer[0]  = ((va_pos & 0x300)>>4) | (vs_pos>>8); 		//REG220
	I2C_Buffer[1]  = ((ha_pos & 0x300)>>4) | (hs_pos>>8); 		//REG221
	WriteBlockTW88(REG220, I2C_Buffer, 2);

//	WriteTW88Page(0);
//	I2C_Buffer[0]  = (ReadTW88(REG068) & 0xFC) | (bt656enc_hdly >> 8);		//REG068
//	I2C_Buffer[1]  = (bt656enc_vact & 0x700) >> 4 | (bt656enc_hact >>8);	//REG069
//	I2C_Buffer[2]  = bt656enc_vdelay;										//REG06A
//	I2C_Buffer[3]  = (BYTE)bt656enc_vact;									//REG06B
//	I2C_Buffer[4]  = (BYTE)bt656enc_hdly;									//REG06C
//	I2C_Buffer[5]  = (BYTE)bt656enc_hact;									//REG06D
//	WriteBlockTW88(REG068, I2C_Buffer, 6);
}

#define SCALER_MODE_FOSSIL		0
#define SCALER_MODE_FULL		1
#define SCALER_MODE_OVERSCAN	2
#define SCALER_MODE_720X480P	3	//for BT656 TEST

void ScalerTest_Decoder(BYTE scaler_mode)
{
	BYTE cvbs_mode;
	cvbs_mode = DecoderCheckSTD(100);
	if(cvbs_mode & 0x80) {
	    ePrintf("\nScalerTest_Decoder NoSTD");
		return; //( 2 );
	}
	cvbs_mode >>= 4;
	if(cvbs_mode==0) {
		//NTSC
		if(scaler_mode==SCALER_MODE_FULL) {
			//scaler full mode
			//DecoderSetInputCrop(720,240,9,18+2)
			//DecoderSetVDelay(18+2); //spec+delay
			//DecoderSetVActive(240);
			//DecoderSetHDelay(9); //workaround value
			//DecoderSetHActive(720);
			DecoderSetInputCrop(9,720,18+2,240);
	
			ScalerSetScaleRate(720,240,800,480);
			ScalerSetOutputTimeRegs(VID_800X480P_IDX,/*DTV_table[VID_800X480P_IDX].vBPorch=>*/20, 240);
			//ScalerSetOutputTimeRegs(VID_720X480P_IDX);
		}
		else if(scaler_mode==SCALER_MODE_OVERSCAN) {
			//scaler overscan mode
			//DecoderSetInputCrop(720,240,9,18+2)
			//DecoderSetVDelay(18+2); //spec+delay
			//DecoderSetVActive(240);
			//DecoderSetHDelay(9); //workaround value
			//DecoderSetHActive(720);
			DecoderSetInputCrop(9,720,18+2,240);
	
			ScalerSetScaleRate(720,228,800,480);
			ScalerSetOutputTimeRegs(VID_800X480P_IDX,/*DTV_table[VID_800X480P_IDX].vBPorch=>*/20+6, 228);
			//ScalerSetOutputTimeRegs(VID_720X480P_IDX);
		}
		else {
			Puts("\nSorry! only support 0,1,2");
		}
	} 
	else {
		//assume PAL
		if(scaler_mode==1) {
			//DecoderSetInputCrop(720,288,6,20+2+1)
			//DecoderSetVDelay(22+2);	//spec+delay
			//DecoderSetVActive(288);
			//DecoderSetHDelay(11); //workground value
			//DecoderSetHActive(720);
			DecoderSetInputCrop(11,720,22+2,288);
	
			ScalerSetScaleRate(720,288,800,480);
			ScalerSetOutputTimeRegs(VID_800X480P_IDX,/*DTV_table[VID_800X480P_IDX].vBPorch=>*/22+2, 288);
		} 
		else if(scaler_mode==2) {
			//overscan mode. use 95%
			//DecoderSetInputCrop(720,288,6,20+2+1)
			//DecoderSetVDelay(22+2);	//spec+delay
			//DecoderSetVActive(288);
			//DecoderSetHDelay(11); //workground value
			//DecoderSetHActive(720);
			DecoderSetInputCrop(11,720,22+2,288);
	
			ScalerSetScaleRate(720,274,800,480);
			ScalerSetOutputTimeRegs(VID_800X480P_IDX,/*DTV_table[VID_800X480P_IDX].vBPorch=>*/22+2+7, 274); 
		}
	}
}

//typedef struct s_VIDEO_TIME {
//	WORD hDeWidth,vDeWidth;		BYTE fIorP,vfreq;
//	BYTE Pol;
//	WORD hTotal;	BYTE hFPorch, hSyncWidth, hBPorch;  
//	WORD vTotal; 	BYTE vPad, vSyncStart, vSyncWidth, vBPorch;
//} t_VIDEO_TIME;
t_VIDEO_TIME VideoTime;

void ScalerTest_Component(BYTE scaler_mode)
{

	WORD hActive, hStart;	//hTotal  ,hStart,hSync
	WORD vTotal,vActive, vStart; //,,vSync,vBPorch;  //

	BYTE index;
	BYTE vfreq;
	//WORD table_hTotal;
	//WORD wTemp;

//	t_VIDEO_TIME *pVidTime;
	struct s_DTV_table *pVid;

	if(MeasStartMeasure()) {
		//something wrong.
		dPrintf("==>FAIL!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		return;
	}			
	//===========================================
	// read measured value & adjust it.
	// we only have a vfreq & vtotal
	//===========================================
	vfreq  = MeasGetVFreq();
	if ( vfreq < 55 ) vfreq = 50;
	else  vfreq = 60;
	vTotal = MeasGetVPeriod();;

	//print measured value
	Printf("\nMeas vTotal:%d vfreq:%bd", vTotal,vfreq);
	
	//search from table
	for(index=0; index < DTV_TABLE_MAX; index++) {
		if(index==0)	continue;

		pVid = &DTV_table[index];
		if((vTotal-2) <= pVid->vTotal && (vTotal+2) >= pVid->vTotal) {
			//==check active. If ext device give an active low sync polarity, we can not use a hTotal.
			//table_hTotal = pVid->hDeWidth;
			//if((hActive-2) <=  table_hTotal && (hActive+2) >= table_hTotal) {
			//	ePuts("\n=3=>");
			//	break;
			//}
			if(vfreq == pVid->vfreq)
				break;
		}
	}
	if(index >= DTV_TABLE_MAX) {
		Printf("\n=>GiveUp");
		return;
	}

	//add "1" for overscan on vActive
	hStart = ReadTW88(REG530); hStart <<= 8; hStart |= ReadTW88(REG531);
	vStart = ReadTW88(REG538); hStart <<= 8; vStart |= ReadTW88(REG539);
	InputSetCrop(hStart-16, vStart+1+1, pVid->hDeWidth, pVid->vDeWidth+1);
	Printf("\nCrop H Pol:%bd Start:%d Active:%d", 0, pVid->hBPorch-16, pVid->hDeWidth);
	Printf("\n     V Pol:%bd Start:%d Active:%d", 0, pVid->vBPorch+1,pVid->vDeWidth+1);

				
	Printf("\nInput DTV_table[%bd]	",index);
	Printf("%bd %dx%d%s@%bd POL:%02bx   %d,%d,%d,%d   %d,%bd,%bd,%bd,%bd %ld",
		pVid->vid,
		pVid->hDeWidth,pVid->vDeWidth,	pVid->fIorP == 1 ? "I" : "P", 
		pVid->vfreq == FREQ_60 ? 60 : pVid->vfreq == FREQ_50 ? 50 : 0,
		pVid->Pol,
		pVid->hTotal,	(WORD)pVid->hFPorch,(WORD)pVid->hSyncWidth,(WORD)pVid->hBPorch,
		pVid->vTotal,	pVid->vPad,pVid->vSyncStart,pVid->vSyncWidth,pVid->vBPorch,
		pVid->pixelfreq
		);

	
	hActive = pVid->hDeWidth;
	vActive = pVid->vDeWidth;
	vStart =  pVid->vSyncWidth + pVid->vBPorch;


	if(scaler_mode==SCALER_MODE_720X480P) {
		ScalerSetScaleRate(hActive,vActive,720,480);
		ScalerSetOutputTimeRegs(VID_720X480P_IDX, vStart, vActive);
	}
	else {
		ScalerSetScaleRate(hActive,vActive,800,480);
		ScalerSetOutputTimeRegs(VID_800X480P_IDX, vStart, vActive);
	}	
}

void ScalerTest_PC(BYTE scaler_mode)
{
	WORD hActive; // hTotal, ,hStart,hSync
	WORD vTotal,vActive,vStart; //,vSync,vBPorch;//

	BYTE index;
	BYTE vfreq;
//	WORD table_hTotal;
	WORD wTemp;

//	t_VIDEO_TIME *pVidTime;
	struct s_VESA_table *pVid;

	Printf("\nScalerTest_PC(%bd)",scaler_mode);

	if (MeasStartMeasure())
	{
		//something wrong.
		dPrintf("==>FAIL!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		return;
	}			
	//===========================================
	// read measured value & adjust it.
	// vtotal, vactive, vfreq
	// hactive
	//===========================================
	
	//Horizontal
	//hTotal = MeasGetVsyncRisePos();
	hActive = MeasGetHActive(&wTemp);
	vfreq  = MeasGetVFreq();
	//hSync = MeasGetHSyncRiseToFallWidth();
	//if(hTotal < hActive)
	//	//if input is an interlaced, it can be a measured value on the odd field.
	//	//in this case, make it double.
	//	hTotal *=2;	
	//hStart = wTemp+4; //add HW offset value. hStart can be hBPorch or hSync+hBPorch.

	//Vertical
	vTotal = MeasGetVPeriod();;
	vActive = MeasGetVActive(&wTemp);
	//vSync = MeasGetVSyncRiseToFallWidth();
	//vStart = wTemp-1; //subtract HW offset value. vStart can be vBPorch or vSync+vBPorch.
	//vBPorch = vStart - vSync;

	//print measured value
	Printf("\nMeas hActive:%d vActive:%d vTotal:%d vfreq:%bd", hActive, vActive,vTotal, vfreq);

	//search from table
	for (index=0; index < VESA_TABLE_MAX; index++)
	{
		pVid = &VESA_table[index];
		if (pVid->hDeWidth == 0xFFFF && pVid->vDeWidth==0xFFFF)
		{
			//return 0xFF;	//give up
			index = 0xFF;
			break;
		}
		if (vTotal < (pVid->vTotal-1) || vTotal > (pVid->vTotal+1))
			continue;
		if (vfreq < (pVid->vfreq-1) || vfreq > (pVid->vfreq+1))
			continue;
		if (vActive < (pVid->vDeWidth-2) || vActive >  (pVid->vDeWidth+2))
			continue;
		//found
		break;
	}

	if (index == 0xFF)
	{
		Printf("\n=>GiveUp");
		return;
	}
	Printf("\nInput VESA_table[%bd]	",index);
	Printf("%bd %dx%d@%bd POL:%02bx   %d,%d,%d,%d   %d,%bd,%bd,%bd %ld",
		pVid->vid,
		pVid->hDeWidth,pVid->vDeWidth,	//pVid->fIorP == 1 ? "I" : "P", 
		pVid->vfreq == FREQ_60 ? 60 : pVid->vfreq == FREQ_50 ? 50 : 0,
		pVid->Pol,
		pVid->hTotal,	(WORD)pVid->hFPorch,(WORD)pVid->hSyncWidth,(WORD)pVid->hBPorch,
		pVid->vTotal,	pVid->vFPorch,pVid->vSyncWidth,pVid->vBPorch,
		pVid->pixelfreq
		);

	//add "1" for overscan on vActive
	InputSetCrop(pVid->hBPorch-16, pVid->vBPorch+1, pVid->hDeWidth, pVid->vDeWidth+1);
	Printf("\nCrop H Pol:%bd Start:%d Active:%d", 0, pVid->hBPorch-16, pVid->hDeWidth);
	Printf("\n     V Pol:%bd Start:%d Active:%d", 0, pVid->vBPorch+1,pVid->vDeWidth+1);

	hActive = pVid->hDeWidth;
	vActive = pVid->vDeWidth;
	vStart =  pVid->vBPorch +1;

	if (scaler_mode == SCALER_MODE_720X480P)
	{
		ScalerSetScaleRate(hActive, vActive, 720, 480);
		ScalerSetOutputTimeRegs(VID_720X480P_IDX, vStart, vActive);
	}
	else
	{
		ScalerSetScaleRate(hActive, vActive, 800, 480);
		ScalerSetOutputTimeRegs(VID_800X480P_IDX, vStart, vActive);
	}	
}

void ScalerTest_DTV(BYTE scaler_mode)
{
	WORD hTotal,hActive,hStart,hSync;
	WORD vTotal,vActive,vStart,vSync,vBPorch;

	BYTE index;
	BYTE vfreq;
	WORD table_hTotal;
	WORD wTemp;

	t_VIDEO_TIME *pVidTime;
	struct s_DTV_table *pVid;

	if(MeasStartMeasure()) {
		//something wrong.
		dPrintf("==>FAIL!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		return;
	}			
	pVidTime = &VideoTime;
			

	//===========================================
	// read measured value & adjust it.
	//===========================================
	
	//Horizontal
	hTotal = MeasGetVsyncRisePos();
	hActive = MeasGetHActive(&wTemp);
	vfreq  = MeasGetVFreq();
	hSync = MeasGetHSyncRiseToFallWidth();
	if(hTotal < hActive)
		//if input is an interlaced, it can be a measured value on the odd field.
		//in this case, make it double.
		hTotal *=2;	
	hStart = wTemp+4; //add HW offset value. hStart can be hBPorch or hSync+hBPorch.

	//Vertical
	vTotal = MeasGetVPeriod();;
	vActive = MeasGetVActive(&wTemp);
	vSync = MeasGetVSyncRiseToFallWidth();
	vStart = wTemp-1; //subtract HW offset value. vStart can be vBPorch or vSync+vBPorch.
	vBPorch = vStart - vSync;

	//print measured value
	Printf("\nMeas H hActive:%d hTotal:%d hStart:%d hSync:%d ", 					hActive,hTotal,hStart,hSync);
	Printf("\n     V vActive:%d vTotal:%d vStart:%d vSync:%d vBPorch:%d vfreq:%bd", vActive,vTotal,vStart,vSync, vBPorch, vfreq);

	DtvSetPolarity(0,0);
	//add "1" for overscan on vActive
	InputSetCrop(hStart, vStart, hActive, vActive+1);
	Printf("\nCrop H Pol:%bd Start:%d Active:%d", 0, hStart,hActive);
	Printf("\n     V Pol:%bd Start:%d Active:%d", 0, vStart,vActive+1);

	if(scaler_mode==SCALER_MODE_720X480P) {
		ScalerSetScaleRate(hActive,vActive,720,480);
		ScalerSetOutputTimeRegs(VID_720X480P_IDX, vStart, vActive);
	}
	else {
		ScalerSetScaleRate(hActive,vActive,800,480);
		ScalerSetOutputTimeRegs(VID_800X480P_IDX, vStart, vActive);
	}
	//debug.....
	//search
	for(index=0; index < DTV_TABLE_MAX; index++) {
		pVid = &DTV_table[index];
		if((vTotal-2) <= pVid->vTotal && (vTotal+2) >= pVid->vTotal) {
			table_hTotal = pVid->hTotal;
			if((hTotal-2) <=  table_hTotal && (hTotal+2) >= table_hTotal) {
				ePuts("\n=1=>");
				break;
			}
			if(pVid->fIorP==1) {
				table_hTotal >>= 1;
				if((hTotal-2) <=  table_hTotal && (hTotal+2) >= table_hTotal) {
					ePuts("\n=2=>");
					break;
				}

			}
			//==check active. If ext device give an active low sync polarity, we can not use a hTotal.
			table_hTotal = pVid->hDeWidth;
			if((hActive-2) <=  table_hTotal && (hActive+2) >= table_hTotal) {
				ePuts("\n=3=>");
				break;
			}
		}
	}
	if(index >= DTV_TABLE_MAX) {
		Printf("\n=>GiveUp");
		return;
	}			
	//pVid = &DTV_table[index];

	Printf("\nInput DTV_table[%bd]	",index);
	Printf("%bd %dx%d%s@%bd POL:%02bx   %d,%d,%d,%d   %d,%bd,%bd,%bd,%bd %ld",
		pVid->vid,
		pVid->hDeWidth,pVid->vDeWidth,	pVid->fIorP == 1 ? "I" : "P", 
		pVid->vfreq == FREQ_60 ? 60 : pVid->vfreq == FREQ_50 ? 50 : 0,
		pVid->Pol,
		pVid->hTotal,	(WORD)pVid->hFPorch,(WORD)pVid->hSyncWidth,(WORD)pVid->hBPorch,
		pVid->vTotal,	pVid->vPad,pVid->vSyncStart,pVid->vSyncWidth,pVid->vBPorch,
		pVid->pixelfreq
		);

//	InputSetCrop(hStart, vStart, hActive, vActive+1);		
	Printf("\nCrop H Pol:%bd start:%d hActive:%d", DTV_table[index].Pol >> 4,   hStart,hActive);
	Printf("\n     V Pol:%bd vstart:%d vActive:%d",DTV_table[index].Pol & 0x0F, vStart,vActive);
	if(scaler_mode==SCALER_MODE_720X480P) {
		ScalerSetScaleRate(hActive,vActive,720,480);
		ScalerSetOutputTimeRegs(VID_720X480P_IDX, vStart, vActive);
	}
	else {
		ScalerSetScaleRate(hActive,vActive,800,480);
		ScalerSetOutputTimeRegs(VID_800X480P_IDX, vStart, vActive);
	}

}
#if 0 //working....121128
void ScalerTest_DTV(void)
{
	WORD hstart,vstart;
	WORD hActive,vActive;
	WORD htotal,vtotal;


//	BYTE offset, VPulse, HPulse;
//	BYTE HPol, VPol;
//	WORD Meas_HPulse,Meas_VPulse;


	BYTE index;
	BYTE vfreq;
	WORD table_hTotal;
//	BYTE bTemp;
//	BYTE speed;
	WORD wTemp;

	t_VIDEO_TIME *pVidTime;
	pVidTime = &VideoTime;

	if(MeasStartMeasure()) {
		//something wrong.
		dPrintf("==>FAIL!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		return;
	}			
			
	DtvSetPolarity(0,0);
	//we have a measured value.
	//read hTotal and vTotal then find out DTV mode.
	htotal = MeasGetVsyncRisePos();
	vtotal = MeasGetVPeriod();
	vfreq  = MeasGetVFreq();
	Printf("\nMeas %dx%d@%bd:", htotal,vtotal,vfreq);


	//===========================================
	// read measured value & adjust it.
	//===========================================

	pVidTime->hDeWidth = MeasGetHActive(&wTemp);
	pVidTime->hBPorch = wTemp;

	pVidTime->vDeWidth = MeasGetVActive(&wTemp);
	pVidTime->vBPorch = wTemp;

	//pVidTime->fIorP = 
	pVidTime->vfreq = vfreq;
	//pVidTime->Pol = 

	pVidTime->hTotal = htotal;
	pVidTime->hFPorch = htotal - MeasGetHSyncRiseToHActiveEnd();
	pVidTime->hSyncWidth = MeasGetHSyncRiseToFallWidth();
	
	pVidTime->vTotal = vtotal;
	pVidTime->vSyncWidth = MeasGetVSyncRiseToFallWidth();
	pVidTime->vPad = vtotal - pVidTime->vSyncWidth - pVidTime->vBPorch - pVidTime->vDeWidth;
	pVidTime->vSyncStart = 0;

	dPrintf("\nMeasured");
	dPrintf("\nH total:%4d FPorch:%d Sync:%d BPorch:%d active:%d",  
		pVidTime->hTotal,
		(WORD)pVidTime->hFPorch,	   	//temp
		(WORD)pVidTime->hSyncWidth,	//temp
		(WORD)pVidTime->hBPorch,
		pVidTime->hDeWidth);
	dPrintf("\nV total:%4d FPorch:%d Sync:%d BPorch:%d active:%d",  
		pVidTime->vTotal,
		(WORD)pVidTime->vPad + pVidTime->vSyncStart,	   //temp
		(WORD)pVidTime->vSyncWidth,
		(WORD)pVidTime->vBPorch,
		pVidTime->vDeWidth);		

	//
	//start adjust the measured value.
	//
	//check vfreq
	if(pVidTime->vfreq==59) 	
		pVidTime->vfreq = FREQ_60;
	//check interlaced......1080i incorrect.
	pVidTime->fIorP = 0; //'P';
	if(htotal < pVidTime->hDeWidth) {
		pVidTime->fIorP = 1; //'I';
		htotal *= 2;
		pVidTime->hFPorch = htotal - MeasGetHSyncRiseToHActiveEnd();
	}
	//check polarity.
	if ( pVidTime->hSyncWidth > (pVidTime->hTotal/2) ) {
		//pVidTime->hSyncWidth = pVidTime->hTotal - pVidTime->hSyncWidth;
		pVidTime->hBPorch = pVidTime->hDeWidth - MeasGetHSyncRiseToFallWidth();
		pVidTime->hFPorch = MeasGetHSyncRiseToFallWidth() - pVidTime->hDeWidth - pVidTime->hBPorch;
		//FAIL:::pVidTime->hSyncWidth = ??
		//FAIL:::oVidTime->htotal==??
		//You have to use invert H polarity on DTV and assign only hBPorch.
		pVidTime->Pol = 0x10;	
	}
	else  {
		pVidTime->hBPorch += 4;
		pVidTime->hBPorch -= pVidTime->hSyncWidth;

		pVidTime->Pol = 0x00;
	}
	if ( pVidTime->vSyncWidth > (pVidTime->vTotal/2) ) {
		pVidTime->vSyncWidth = pVidTime->vTotal - MeasGetVSyncRiseToFallWidth();
		MeasGetVActive(&wTemp);
		pVidTime->vBPorch = wTemp - 1;
		//pVidTime->vFPorch = pVidTime->vTotal - pVidTime->vSyncWidth - pVidTime->vBPorch - pVidTime->vDeWidth;
		pVidTime->vPad = pVidTime->vTotal - pVidTime->vSyncWidth - pVidTime->vBPorch - pVidTime->vDeWidth;
		pVidTime->Pol |= 0x01;
	}
	else  {
		pVidTime->vBPorch -= 1;
		pVidTime->vBPorch -= pVidTime->vSyncWidth;
		pVidTime->vPad = pVidTime->vTotal - pVidTime->vSyncWidth - pVidTime->vBPorch - pVidTime->vDeWidth;
	}
	//pVidTime->vPad = pVidTime->vTotal - pVidTime->vSyncWidth - pVidTime->vBPorch - pVidTime->vDeWidth;
	//
	//dPrintf("\nAdjusted1");
	//dPrintf("\nH total:%4d FPorch:%d Sync:%d BPorch:%d active:%d Pol:%bx",  
	//	pVidTime->hTotal,
	//	(WORD)pVidTime->hFPorch,	   	//temp
	//	(WORD)pVidTime->hSyncWidth,	//done
	//	(WORD)pVidTime->hBPorch,
	//	pVidTime->hDeWidth,
	//	pVidTime->Pol >> 4);
	//dPrintf("\nV total:%4d FPorch:%d Sync:%d BPorch:%d active:%d Pol:%bx",  
	//	pVidTime->vTotal,
	//	(WORD)pVidTime->vPad + pVidTime->vSyncStart,
	//	(WORD)pVidTime->vSyncWidth,	//done
	//	(WORD)pVidTime->vBPorch,
	//	pVidTime->vDeWidth,
	//	pVidTime->Pol & 0x0F);		

	//adjust measure module delay value.
	//pVidTime->hBPorch += 4;
	//pVidTime->hBPorch -= pVidTime->hSyncWidth;
	//pVidTime->vBPorch -= 1;
	//pVidTime->vBPorch -= pVidTime->vSyncWidth;
	//pVidTime->vPad = pVidTime->vTotal - pVidTime->vSyncWidth - pVidTime->vBPorch - pVidTime->vDeWidth;

	dPrintf("\nAdjusted2 ");
	if(pVidTime->Pol & 0x10) {
		dPrintf("\nH total:???? FPorch:%d Sync:?? BPorch:%d active:%d Pol:%bx",  
			//pVidTime->hTotal,
			(WORD)pVidTime->hFPorch,
			//(WORD)pVidTime->hSyncWidth,
			(WORD)pVidTime->hBPorch,
			pVidTime->hDeWidth,
			pVidTime->Pol >> 4);
	}
	else {
		dPrintf("\nH total:%4d FPorch:%d Sync:%d BPorch:%d active:%d Pol:%bx",  
			pVidTime->hTotal,
			(WORD)pVidTime->hFPorch,	//temp
			(WORD)pVidTime->hSyncWidth,	//done
			(WORD)pVidTime->hBPorch,
			pVidTime->hDeWidth,
			pVidTime->Pol >> 4);
	}
	dPrintf("\nV total:%4d FPorch:%d Sync:%d BPorch:%d active:%d Pol:%bx",  
		pVidTime->vTotal,
		(WORD)pVidTime->vPad + pVidTime->vSyncStart,
		(WORD)pVidTime->vSyncWidth,	//done
		(WORD)pVidTime->vBPorch,
		pVidTime->vDeWidth,
		pVidTime->Pol & 0x0F);		









	//search

	for(index=0; index < DTV_TABLE_MAX; index++) {
		if((vtotal-2) <= DTV_table[index].vTotal && (vtotal+2) >= DTV_table[index].vTotal) {
			table_hTotal = DTV_table[index].hTotal;
			if((htotal-2) <=  table_hTotal && (htotal+2) >= table_hTotal) {
				Printf("=>index[%bd] %dx%d", index, DTV_table[index].hDeWidth, DTV_table[index].vDeWidth);
				break;
			}
			if(DTV_table[index].fIorP==1) {
				table_hTotal >>= 1;
				if((htotal-2) <=  table_hTotal && (htotal+2) >= table_hTotal) {
					Printf("=>index[%bd] %dx%dI", index, DTV_table[index].hDeWidth, DTV_table[index].vDeWidth*2);
					break;
				}

			}
		}
	}
	if(index >= DTV_TABLE_MAX) {
		Printf("=>GiveUp");
		return;
	}	
		

	//===============
	// DTV09SetInputCrop
	//================
	DtvSetPolarity(DTV_table[index].Pol >> 4,DTV_table[index].Pol & 0x0F);

	hstart = DTV_table[index].hBPorch;
	if((DTV_table[index].Pol & 0x10) ==0)
		hstart += DTV_table[index].hSyncWidth;

	vstart = DTV_table[index].vBPorch;
	if((DTV_table[index].Pol & 0x01) ==0)
		vstart += DTV_table[index].vSyncWidth;

	hActive = DTV_table[index].hDeWidth;
	vActive = DTV_table[index].vDeWidth;
	//if(index == VID_720X480I_IDX || index == VID_720X576I_IDX) {
	//	hstart /= 2;
	//	hActive /= 2;
	//	vActive *= 2;
	//}
	InputSetCrop(hstart, vstart, hActive, vActive);
	Printf("\nCrop hstart:%d hActive:%d",hstart,hActive);
	Printf("\n     vstart:%d vActive:%d",vstart,vActive);

	//====================
	// DTV SetOutput
	//====================
	ScalerSetScaleRate(DTV_table[index].hDeWidth,DTV_table[index].vDeWidth,800,480);
	ScalerSetOutputTimeRegs(VID_800X480P_IDX, (WORD)vstart, DTV_table[index].vDeWidth);
}
#endif
#if 0
//--------------successed code------------------
void ScalerTest_DTV(void)
{
	WORD hstart,vstart;
	WORD hActive,vActive;
	WORD htotal,vtotal;


	BYTE offset, VPulse, HPulse;
	BYTE HPol, VPol;
	WORD Meas_HPulse,Meas_VPulse;


	BYTE index;
	BYTE vfreq;
	WORD table_hTotal;
//	BYTE bTemp;
//	BYTE speed;
	WORD wTemp;

	t_VIDEO_TIME *pVidTime;
	pVidTime = &VideoTime;

	if(MeasStartMeasure()) {
		//something wrong.
		dPrintf("==>FAIL!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		return;
	}			
			
	DtvSetPolarity(0,0);
	//we have a measured value.
	//read hTotal and vTotal then find out DTV mode.
	htotal = MeasGetVsyncRisePos();
	vtotal = MeasGetVPeriod();
	vfreq  = MeasGetVFreq();
	Printf("\nMeas %dx%d@%bd:", htotal,vtotal,vfreq);


	//===========================================
	// read measured value & adjust it.
	//===========================================

	pVidTime->hDeWidth = MeasGetHActive(&wTemp);
	pVidTime->hBPorch = wTemp;

	pVidTime->vDeWidth = MeasGetVActive(&wTemp);
	pVidTime->vBPorch = wTemp;

	//pVidTime->fIorP = 
	if(vfreq==59 || vfreq==60) 	pVidTime->vfreq = FREQ_60;
	else						pVidTime->vfreq = FREQ_50;	//assume
	//pVidTime->Pol = 

	pVidTime->fIorP = 0; //'P';
	if(htotal < pVidTime->hDeWidth) {
		pVidTime->fIorP = 1; //'I';
		htotal *= 2;
	}
	pVidTime->hTotal = htotal;
	pVidTime->hFPorch = htotal - MeasGetHSyncRiseToHActiveEnd();
	pVidTime->hSyncWidth = MeasGetHSyncRiseToFallWidth();
	
	pVidTime->vTotal = vtotal;
	pVidTime->vSyncWidth = MeasGetVSyncRiseToFallWidth();
	pVidTime->vPad = vtotal - pVidTime->vSyncWidth - pVidTime->vBPorch - pVidTime->vDeWidth;
	pVidTime->vSyncStart = 0;


	//search

	for(index=0; index < DTV_TABLE_MAX; index++) {
		if((vtotal-2) <= DTV_table[index].vTotal && (vtotal+2) >= DTV_table[index].vTotal) {
			table_hTotal = DTV_table[index].hTotal;
			if((htotal-2) <=  table_hTotal && (htotal+2) >= table_hTotal) {
				Printf("=>index[%bd] %dx%d", index, DTV_table[index].hDeWidth, DTV_table[index].vDeWidth);
				break;
			}
			if(DTV_table[index].fIorP==1) {
				table_hTotal >>= 1;
				if((htotal-2) <=  table_hTotal && (htotal+2) >= table_hTotal) {
					Printf("=>index[%bd] %dx%dI", index, DTV_table[index].hDeWidth, DTV_table[index].vDeWidth*2);
					break;
				}

			}
		}
	}
	if(index >= DTV_TABLE_MAX) {
		Printf("=>GiveUp");
		return;
	}	
		

	//===============
	// DTV09SetInputCrop
	//================
	DtvSetPolarity(DTV_table[index].Pol >> 4,DTV_table[index].Pol & 0x0F);

	hstart = DTV_table[index].hBPorch;
	if((DTV_table[index].Pol & 0x10) ==0)
		hstart += DTV_table[index].hSyncWidth;

	vstart = DTV_table[index].vBPorch;
	if((DTV_table[index].Pol & 0x01) ==0)
		vstart += DTV_table[index].vSyncWidth;

	hActive = DTV_table[index].hDeWidth;
	vActive = DTV_table[index].vDeWidth;
	//if(index == VID_720X480I_IDX || index == VID_720X576I_IDX) {
	//	hstart /= 2;
	//	hActive /= 2;
	//	vActive *= 2;
	//}
	InputSetCrop(hstart, vstart, hActive, vActive);
	Printf("\nCrop hstart:%d hActive:%d",hstart,hActive);
	Printf("\n     vstart:%d vActive:%d",vstart,vActive);

	//========================
	//debug area


#ifdef DEBUG_DTV
#endif

	dPrintf("\nMeasured");
	dPrintf("\nH total:%4d FPorch:%d Sync:%d BPorch:%d active:%d",  
		pVidTime->hTotal,
		(WORD)pVidTime->hFPorch,	   	//temp
		(WORD)pVidTime->hSyncWidth,	//temp
		(WORD)pVidTime->hBPorch,
		pVidTime->hDeWidth);
	dPrintf("\nV total:%4d FPorch:%d Sync:%d BPorch:%d active:%d",  
		pVidTime->vTotal,
		(WORD)pVidTime->vPad + pVidTime->vSyncStart,	   //temp
		(WORD)pVidTime->vSyncWidth,
		(WORD)pVidTime->vBPorch,
		pVidTime->vDeWidth);		

	if ( pVidTime->hSyncWidth > (pVidTime->hTotal/2) ) {
		pVidTime->hSyncWidth = pVidTime->hTotal - pVidTime->hSyncWidth;
		pVidTime->Pol = 0x10;	
	}
	else  {
		pVidTime->Pol = 0x00;
	}

	// v meas delay value:-1.
	if ( pVidTime->vSyncWidth > (pVidTime->vTotal/2) ) {
		pVidTime->vSyncWidth = pVidTime->vTotal - pVidTime->vSyncWidth;
		pVidTime->Pol |= 0x01;
	}
	else  {
	}
	pVidTime->vPad = pVidTime->vTotal - pVidTime->vSyncWidth - pVidTime->vBPorch - pVidTime->vDeWidth;

	dPrintf("\nAdjusted1");
	dPrintf("\nH total:%4d FPorch:%d Sync:%d BPorch:%d active:%d Pol:%bx",  
		pVidTime->hTotal,
		(WORD)pVidTime->hFPorch,	   	//temp
		(WORD)pVidTime->hSyncWidth,	//done
		(WORD)pVidTime->hBPorch,
		pVidTime->hDeWidth,
		pVidTime->Pol >> 4);
	dPrintf("\nV total:%4d FPorch:%d Sync:%d BPorch:%d active:%d Pol:%bx",  
		pVidTime->vTotal,
		(WORD)pVidTime->vPad + pVidTime->vSyncStart,
		(WORD)pVidTime->vSyncWidth,	//done
		(WORD)pVidTime->vBPorch,
		pVidTime->vDeWidth,
		pVidTime->Pol & 0x0F);		

	pVidTime->hBPorch += 4;
	pVidTime->hBPorch -= pVidTime->hSyncWidth;
	pVidTime->vBPorch -= 1;
	pVidTime->vBPorch -= pVidTime->vSyncWidth;
	pVidTime->vPad = pVidTime->vTotal - pVidTime->vSyncWidth - pVidTime->vBPorch - pVidTime->vDeWidth;

	dPrintf("\nAdjusted2");
	dPrintf("\nH total:%4d FPorch:%d Sync:%d BPorch:%d active:%d Pol:%bx",  
		pVidTime->hTotal,
		(WORD)pVidTime->hFPorch,	   	//temp
		(WORD)pVidTime->hSyncWidth,	//done
		(WORD)pVidTime->hBPorch,
		pVidTime->hDeWidth,
		pVidTime->Pol >> 4);
	dPrintf("\nV total:%4d FPorch:%d Sync:%d BPorch:%d active:%d Pol:%bx",  
		pVidTime->vTotal,
		(WORD)pVidTime->vPad + pVidTime->vSyncStart,
		(WORD)pVidTime->vSyncWidth,	//done
		(WORD)pVidTime->vBPorch,
		pVidTime->vDeWidth,
		pVidTime->Pol & 0x0F);		


	//====================
	// DTV SetOutput
	//====================
	ScalerSetScaleRate(DTV_table[index].hDeWidth,DTV_table[index].vDeWidth,800,480);
	ScalerSetOutputTimeRegs(VID_800X480P_IDX, (WORD)vstart, DTV_table[index].vDeWidth);
}
#endif

//mode
//	0:Original Scaler
//	1:New method
//	2:target 720x480p for BT656
void ScalerTest(BYTE mode)
{
	//scaler test
	if (mode)
	{
		switch (InputMain)
		{
		case INPUT_CVBS:
			ScalerTest_Decoder(mode);
			break;
		case INPUT_SVIDEO:
			ScalerTest_Decoder(mode);
			break;
		case INPUT_COMP:
			ScalerTest_Component(mode);
			break;
		case INPUT_PC:
			ScalerTest_PC(mode);
			break;
		case INPUT_DVI:
			ScalerTest_DTV(mode);
			break;
		case INPUT_HDMIPC:
		case INPUT_HDMITV:
			ScalerTest_DTV(mode);
			break;
		case INPUT_BT656:
			ScalerTest_DTV(mode);
			break;
		case INPUT_LVDS:
			ScalerTest_DTV(mode);
			break;
		}
	}
	else
	{
		//CheckAndSetInput will recover the test value.
		CheckAndSetInput();
	}
}

