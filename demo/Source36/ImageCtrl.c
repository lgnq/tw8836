/**
 * @file
 * ImageCtrl.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	Image Enhancement 
 *
 * The Image Control Block is located after UpSacler and before OSD mux.
 * But, If we use a CVBS or SVideo with Internal Decoder, we can also control 
 * the image on the internal decoder. Use a USE_FRONT_IMAGECTRL, if you want 
 * to use the image control block on the front internal decoder.(Removed..)
 * If you using USE_FRONT_IMAGECTRL, the image control for aRGB/YPbPr and DTV input 
 * will be serviced by BackEnd Imange Control Block.
 *
 * We are using 0..100 value for this image effect functions because the register valus
 * have a different value range. 
 *
 * see Image Block diagram
*/

/*
Image block
===========
   FrontEnd  <----|---->BackEnd
			  +-------+		  +----------+	    +--------+	    +-------+
Decoder ===>  |   	  |  	  |			 |	    |		 |	    |	    |
			  | Input |		  | Image	 |	    | RGB	 |	    |       |
   aRGB ===>  |		  | ===>  |			 | ===>	|		 | ===>	| Gamma | ==> Display
			  | Mux	  |		  | Enhancer | 	    | CSC 	 | 	    |       |
    DTV ===>  |  	  |		  |			 |	    |		 |	    |	    |
	          +-------+		  +----------+	    +--------+	    +-------+
							  <<YUV domain>>	<<RGB domain>>

*/


#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "Global.h"
#include "CPU.h"
#include "printf.h"
#include "util.h"
#include "monitor.h"

#include "I2C.h"
#include "spi.h"

#include "SOsd.h"
#include "InputCtrl.h"
#include "ImageCtrl.h"
#include "scaler.h"
#include "EEPROM.h"
#include "aRGB.h"
#include "PC_modes.h"
#include "measure.h"

						 // 0  1   2   3   4   5   6   7   8   9   a   b   c   d   e   f 
code BYTE Sharpness[16] = { 0, 6, 13, 20, 26, 33, 40, 47, 54, 61, 67, 74, 80, 87, 94, 100 };


WORD PcBaseClock;
BYTE PcBasePhase;
WORD PcBasePosH;
BYTE PcBasePosV;


//===============
// DOC

/*
Image Adjust block
*/

//============================== YCbCr  ================================================

//#define		ContrastR		0x81
//#define		BrightnessR		0x87
//#define		Sharpness		0x8B


//#define		ContrastY		0x84
//#define		BrightnessY		0x8A
//#define		HueY			0x80
//#define		SaturationY		0x85
//#define		SharpnessY		0x8B
//=============================================================================
//YCbCrContrast
//ImgAdjContrastY
//R284[7:0] Contrast Y
//		WORD ImgAdjGetContrastY(void)
//		WORD ImgAdjSetContrastY(WORD val)
//		BYTE ImgAdjChangeContrastY(void)
//=============================================================================

//-----------------------------------------------------------------------------
/**
* get ContrastY value
*/
BYTE ImgAdjGetContrastY(void)
{
    WORD	val;

	WriteTW88Page(PAGE2_IMAGE );
	val = ReadTW88(REG284_IA_CONTRAST_Y );
	val *= 100;
	val += 128;
	val /= 255;
	return (BYTE)(val);
}
//-----------------------------------------------------------------------------
/**
* set ContrastY value
*/
BYTE ImgAdjSetContrastY(BYTE dat)
{
	WORD val = dat;
	WriteTW88Page(PAGE2_IMAGE );
	val *= 255;
	val += 50;
	val /= 100;
	WriteTW88(REG284_IA_CONTRAST_Y, (BYTE)val );
	return ImgAdjGetContrastY();
}
//-----------------------------------------------------------------------------
/**
* save ContrastY value to EEPROM
*/
BYTE ImgAdjChangeContrastY(void)
{
    BYTE	val;

	val = ImgAdjGetContrastY();
	SaveVideoDatatoEE( EEP_IA_CONTRASE_Y, val );
	return (val);
}

//=============================================================================
//YCbCrBright
//ImgAdjBrightnessY
//		WORD ImgAdjGetBrightnessY(void)
//		WORD ImgAdjSetBrightnessY(WORD val)
//		BYTE ImgAdjChangeBrightnessY(void)
//=============================================================================

//-----------------------------------------------------------------------------
/**
* set BrightnessY value
*/
BYTE ImgAdjGetBrightnessY(void)
{
    WORD	val;

	WriteTW88Page(PAGE2_IMAGE );
	val = ReadTW88(REG28A_IA_BRIGHTNESS_Y );
	val *= 100;
	val += 128;
	val /= 255;
	return (BYTE)(val);
}
//-----------------------------------------------------------------------------
/**
* get BrightnessY value
*/
BYTE ImgAdjSetBrightnessY(BYTE dat)
{
	WORD val=dat;
	WriteTW88Page(PAGE2_IMAGE );
	val *= 255;
	val += 50;
	val /= 100;
	WriteTW88(REG28A_IA_BRIGHTNESS_Y, (BYTE)val );
	return ImgAdjGetBrightnessY();
}
//-----------------------------------------------------------------------------
/**
* save BrightnessY value to EEPROM
*/
BYTE ImgAdjChangeBrightnessY(void)
{
    BYTE	val;

	val = ImgAdjGetBrightnessY();
	SaveVideoDatatoEE( EEP_IA_BRIGHTNESS_Y, val );
	return (val);
}
//=============================================================================
//ImagAdjSaturation
//R285[7:0] Contrast Cb
//R286[7:0] Contrast Cr
//	old name was YCbCrSaturation
//	real meaning ImgAdjContrastCb & ImgAdjContrastCr
//	HW can support seperate Cb/Cr.
//	Cb => U, Cr => V
//	and U means Hue, V menas saturation. 
//	But, HW have a seperate Cb,Cr,HUE registers,
//		Cb & Cr register used for Saturateion and HUE register use only Hue.
//
//  SaturationY is a incorrect definition.
//
//		WORD ImgAdjGetSaturation(void)
//		WORD ImgAdjSetSaturation(WORD val)
//		BYTE ImgAdjChangeSaturation(void)
//=============================================================================

//-----------------------------------------------------------------------------
/**
* get Saturation value
*/
BYTE ImgAdjGetSaturation(void)
{
    WORD	val;

	WriteTW88Page(PAGE2_IMAGE );
	val = ReadTW88(REG285_IA_CONTRAST_CB );
	val *= 100;
	val += 128;
	val /= 255;
	return (BYTE)(val);
}
//-----------------------------------------------------------------------------
/**
* set Saturation value
*/
BYTE ImgAdjSetSaturation(BYTE dat)
{
	WORD val = dat;
	WriteTW88Page(PAGE2_IMAGE );
	val *= 255;
	val += 50;
	val /= 100;
	WriteTW88(REG285_IA_CONTRAST_CB, (BYTE)val );	  //update Contrast_Cb(U)
	WriteTW88(REG285_IA_CONTRAST_CB+1, (BYTE)val );  //update Contrast_Cr(V)
	return ImgAdjGetSaturation();
}
//-----------------------------------------------------------------------------
/**
* save Saturation value to EEPROM
*/
BYTE ImgAdjChangeSaturation(void)
{
    BYTE	val;

	val = ImgAdjGetSaturation();
	SaveVideoDatatoEE( EEP_IA_SATURATION, val );
	return (val);
}

//=============================================================================
//YCbCrHUE
//ImgAdjHue
//R280[5:0]. +45~-45. degree:1.4 default:0 degree(xx10 0000)
//		WORD ImgAdjGetHUE(void)
//		WORD ImgAdjSetHUE(WORD val)
//		BYTE ImgAdjChangeHUE(void)
//=============================================================================

//-----------------------------------------------------------------------------
/**
* get HUE value
*
*	reigster description
*	HW	logical		SW(0..100)
*	--	---			----
*	1F	3F			100
*	..
*	1	21			52
*	0	20			50
*	20	1F			49
*	3F	1E			47
*	...
*	21	0			0 
*/
BYTE ImgAdjGetHUE(void)
{
    BYTE reg;
	WORD val;

	WriteTW88Page(PAGE2_IMAGE );
	reg = ReadTW88(REG280_IA_HUE_REG );  //0..63
	reg &= 0x3F;
	val = reg;
	
	if(val==0x20) 			val = 0x1F;
	else if(val > 0x20)		val = val - 0x21;
	else					val = 0x20 + val;

	val *= 100;
	val += 32;
	val /= 63;
	//dPrintf("Get:HW:%bx,SW:%d",reg,val);
	return (BYTE)(val);
}
//-----------------------------------------------------------------------------
/**
* set HUE value
*
* @see ImgAdjGetHUE
*/
BYTE ImgAdjSetHUE(BYTE dat)
{
	WORD value;
	BYTE reserved;
	BYTE reg_hw, reg_sw;

	WriteTW88Page(PAGE2_IMAGE );
	reserved = ReadTW88(REG280_IA_HUE_REG ) & 0xC0;  //0..63

	value = dat;
	value *= 63;
	value += 50; //roundup
	value /= 100;
	reg_sw = value;

	if(reg_sw==0x1F) 		reg_hw = 0x20;
	else if(reg_sw >= 0x20)	reg_hw = reg_sw - 0x20;
	else					reg_hw = reg_sw + 0x21;

	//dPrintf("Set:HW:%bx,SW:%d",value,dat);
	WriteTW88(REG280_IA_HUE_REG, reserved | reg_hw);

	return ImgAdjGetHUE();
}
//-----------------------------------------------------------------------------
/**
* increase/decrease HUE value
*
* @see ImgAdjGetHUE
*/
BYTE ImgAdjVaryHUE(BYTE dat)
{
	BYTE old;
	BYTE reg_hw,reg_sw;
	BYTE reserved;

	old = ImgAdjGetHUE();	//0..100

	WriteTW88Page(PAGE2_IMAGE );
	reg_hw = ReadTW88(REG280_IA_HUE_REG );  //0..63
	reserved = reg_hw & 0xC0;
	reg_hw &= 0x3F;

	if(reg_hw==0x20) 		reg_sw = 0x1F;
	else if(reg_hw > 0x20)	reg_sw = reg_hw - 0x21;
	else					reg_sw = 0x20 + reg_hw;
	
	if(old < dat) {			//inc
		if(reg_sw < 0x3F)
			reg_sw++;
	}
	else if(old > dat) {	//dec
		if(reg_sw)
			reg_sw--;
	}

	if(reg_sw==0x1F) 		reg_hw = 0x20;
	else if(reg_sw >= 0x20)	reg_hw = reg_sw - 0x20;
	else					reg_hw = reg_sw + 0x21;


	WriteTW88(REG280_IA_HUE_REG, reserved | reg_hw);

	return ImgAdjGetHUE();
}
//-----------------------------------------------------------------------------
/**
* save HUE value to EEPROM
*
* @see ImgAdjGetHUE
*/
BYTE ImgAdjChangeHUE(void)
{
    BYTE	val;

	val = ImgAdjGetHUE();
	SaveVideoDatatoEE( EEP_IA_HUE, val );
	return (val);
}

//=============================================================================
//YCbCrSharp
//ImgAdjSharpness
//		WORD ImgAdjGetSharpness(void)
//		WORD ImgAdjSetSharpness(WORD val)
//		BYTE ImgAdjChangeSharpness(void)
//=============================================================================

//-----------------------------------------------------------------------------
/**
* get Sharpness value
*/
BYTE ImgAdjGetSharpness(void)
{
    BYTE	val;

	WriteTW88Page(PAGE2_IMAGE );
	val = Sharpness[(ReadTW88(REG28B_IA_SHARPNESS ) & 0x0f)];
	return (val);
}
//-----------------------------------------------------------------------------
/**
* increase/decrease Sharpness value
*/
BYTE ImgAdjSetSharpness(BYTE val)  //variation
{
	BYTE	i;

	WriteTW88Page(PAGE2_IMAGE );
	for (i=0; i<16; i++) {
		if ( val == (Sharpness[i]+1) ){
			val = i+1;
			break;
		}
		if ( Sharpness[i] == (val+1) ){
			val = i-1;
			break;
		}
	}

	if(i==16) {
		if(val > 100)
			val = 100;
		for (i=0; i<16; i++) {
			if(val == Sharpness[i]) {
				val = i;
				break;
			}
			if(val < Sharpness[i+1]) {
				val = i+1;
				break;
			}
		}
		if(i==16)
			val = i;
	}

	WriteTW88(REG28B_IA_SHARPNESS, ((ReadTW88(REG28B_IA_SHARPNESS ) & 0xf0) + val) );	  
	val = Sharpness[val]; 	//BK101028 bugfix	//val = Sharpness[i];
	return (val);
}
//-----------------------------------------------------------------------------
/**
* save Sharpness value to EEPROM
*/
BYTE ImgAdjChangeSharpness(void)
{
    BYTE	val;

	val = ImgAdjGetSharpness();
	SaveVideoDatatoEE( EEP_IA_SHARPNESS, val );
	return (val);
}



//==============================================================================

//=============================================================================
//RGBContrast
//ImgAdjContrastR
//R281[7:0] Contrast R
//R282[7:0] Contrast G
//R283[7:0] Contrast B
//		WORD ImgAdjGetContrastRGB(void)
//		WORD ImgAdjSetContrastRGB(WORD val)
//		BYTE ImgAdjChangeContrastRGB(void)
//=============================================================================

//-----------------------------------------------------------------------------
/**
* get ContrastRGB value
*/
BYTE ImgAdjGetContrastRGB(BYTE index)
{
    WORD	val;

	WriteTW88Page(PAGE2_IMAGE );
	val = ReadTW88(REG281_IA_CONTRAST_R + index);	 					// check GREEN chanel
	val *= 100;
	val += 128;
	val /= 255;
	return (BYTE)(val);
}
//-----------------------------------------------------------------------------
/**
* set ContrastRGB value
*/
BYTE ImgAdjSetContrastRGB(BYTE index, BYTE dat)
{
	WORD val = dat;
	WriteTW88Page(PAGE2_IMAGE );
	val *= 255;
	val += 50;
	val /= 100;
	WriteTW88(REG281_IA_CONTRAST_R+index,   (BYTE)val );
	return ImgAdjGetContrastRGB(index);
}
//-----------------------------------------------------------------------------
/**
* save ContrastRGB value to EEPROM
*/
BYTE ImgAdjChangeContrastRGB(BYTE index)
{
    BYTE	val;

	val = ImgAdjGetContrastRGB(index);
	SaveVideoDatatoEE( EEP_IA_CONTRAST_R+index, val );
	return (val);
}
//=============================================================================
//RGBBright
//R287[7:0] Brightness R
//R288[7:0] Brightness G
//R289[7:0] Brightness B
//ImgAdjBrightnessR
//		WORD ImgAdjGetBrightnessRGB(void)
//		WORD ImgAdjSetBrightnessRGB(WORD val)
//		BYTE ImgAdjChangeBrightnessRGB(void)
//=============================================================================

#if 0
#define DITHER_FORMAT_888	0
#define DITHER_FORMAT_666	1
#define		DITHER_666_OPTION_3		1
#define		DITHER_666_OPTION_32	2
#define		DITHER_666_OPTION_321	3
#define		DITHER_666_OPTION_3210	4
#define DITHER_FORMAT_565	2
#define DITHER_FORMAT_555	3
#define DITHER_FORMAT_444	4
#define DITHER_FORMAT_333	5
#define DITHER_FORMAT_332	6

//-----------------------------------------------------------------------------
BYTE ImgAdjDither(BYTE value)
{
	BYTE old;
	WriteTW88Page(PAGE2_IMAGE);
	old = ReadTW88(REG2E4);
	WriteTW88(REG2E4,value);
	return old;
}
#endif

//=============================================================================
//RGBSharp
//		void ResetRGBSharp( void )
//=============================================================================
#ifdef UNCALLED_SEGMENT
void ResetRGBSharp( void )
{...}
#endif

//=============================================================================
//		void RGBImageReset(void)
//=============================================================================
#ifdef UNCALLED_SEGMENT_CODE
//void RGBImageReset( void )
//{
//	ImgAdjSetContrastRGB( 50 );
//	ImgAdjSetBrightnessRGB( 50 );
//}
#endif

//=============================================================================
//		void RGBAutoAdj(void)
//=============================================================================
#if 0
void RGBAutoAdj( void )
{
	AutoColorAdjust();	//aRGB_ part
	//BKTODO: Add Phase & Clock detection.
}
#endif

//=============================================================================
//RGBPosH - use InputHStart
//		WORD GetRGBPosH(void)
//		WORD SetRGBPosH(WORD val)
//		BYTE ChangeRGBPosH(void)
//=============================================================================
//return 0..100. default 50
// if return value > 50, HW value will be decreased and the screen will be moved to right.
//extern BYTE	RGB_HDE;

//use InputHStart
//if HW is decreased, return value will be increased.
//extern WORD RGB_HSTART;
//extern BYTE RGB_VDE;
#ifdef SUPPORT_PC
//-----------------------------------------------------------------------------
/**
* get Horizontal position 
*/
BYTE GetRGBPosH(void)
{
	return 50+RGB_HSTART-InputGetHStart();
}
//-----------------------------------------------------------------------------
/**
* set Horizontal position 
*/
BYTE SetRGBPosH(BYTE val)
{
	WORD hw_value;
	if((RGB_HSTART+50) > val)
		hw_value = RGB_HSTART + 50 - val;
	else
		hw_value = 1;

	InputSetHStart(hw_value);
	return GetRGBPosH();
}
//-----------------------------------------------------------------------------
/**
* get Horizontal position to EEPROM
*/
BYTE ChangeRGBPosH()
{
	BYTE value;

	value = GetRGBPosH();
	SaveHActiveEE(Input_aRGBMode,value);
	return value;
}
#endif

//=============================================================================
//RGBPosV - use Scaler Output VDE
//		WORD GetRGBPosV(void)
//		WORD SetRGBPosV(WORD val)
//		BYTE ChangeRGBPosH(void)
//=============================================================================
//return 0..100 base 50.
// if return value > 50, move up HW screen
//if HW is decreased, return value will be decrease.
#if defined(SUPPORT_PC)
//-----------------------------------------------------------------------------
/**
* get Vertical position 
*/
BYTE GetRGBPosV(void)
{
	return 50 + ScalerReadVDEReg() - RGB_VDE;
}
//-----------------------------------------------------------------------------
/**
* set Vertical position 
*/
BYTE SetRGBPosV(BYTE val)
{
	WORD hw_value;
	if((RGB_VDE + val) > 50) 
		hw_value = RGB_VDE + val - 50;
	else 
		hw_value = 1;
	ScalerWriteVDEReg(hw_value);
	return GetRGBPosV();
}
//-----------------------------------------------------------------------------
/**
* save Vertical position to EEPROM
*/
BYTE ChangeRGBPosV(void)
{
	BYTE value;

	value = GetRGBPosV();
	SaveVBackPorchEE(Input_aRGBMode,value);
	return value; 
}
#endif

//=============================================================================
//See AutoTunePhase() on TW8816
//		WORD GetaRGB_Phase(void)
//		WORD SetaRGB_Phase(WORD val)
//		BYTE ChangeaRGB_Phase(void)
//=============================================================================
//interface between MENU and aRGB_
#if defined(SUPPORT_PC)
//-----------------------------------------------------------------------------
/**
* get aRGB Phase value
*
* @return 0..100
*/
BYTE GetaRGB_Phase(void)
{
	WORD value;

	value = aRGB_GetPhase();
	value *= 100;
	value += 15;	//roundup
	value /= 31;
	return (BYTE)value;	  //0..100
}

//-----------------------------------------------------------------------------
/**
* increase/decrease aRGB Phase value
*
* @param 0..100
*	compare val with old value, and then, do variation. 
*/
BYTE VaryaRGB_Phase(BYTE val)
{
	BYTE old,value;

	old = GetaRGB_Phase();	//0..100
	value = aRGB_GetPhase();	//0..0x1F

	if(old < val) {
		if(value < 0x1F)
			value++;
	}
	else if(old > val) {
		if(value > 0)
			value--;
	}
	aRGB_SetPhase(value,ON);	//WithInit

	return GetaRGB_Phase();
}
//-----------------------------------------------------------------------------
/**
* set aRGB Phase value
*
* @param 0..100
*/
BYTE SetaRGB_Phase(BYTE val)
{
	WORD value;
	value = val * 31;
	value += 50;	//roundup
	value /= 100;
	
	//value = PcBasePhase + val - 50;
	aRGB_SetPhase((BYTE)value, ON); //WithInit
	return GetaRGB_Phase();
}

#if 0
void SetaRGB_Phase100ToReg(BYTE val)
{
...
}
#endif
//-----------------------------------------------------------------------------
/**
* save aRGB Phase value to EEPROM
*/
BYTE ChangeaRGB_Phase(void)
{
	BYTE val;

	val = aRGB_GetPhase(); //0..0x1F

	SavePhaseEE(Input_aRGBMode, val);

	val = GetaRGB_Phase(); //0..100
	return val;
}
#endif

//=============================================================================
//aRGB_Clock
//See AutoTuenClock() on TW8816
//		WORD GetaRGB_Clock(void)
//		WORD SetaRGB_Clock(WORD val)
//		BYTE ChangeaRGB_Clock(void)
//=============================================================================
//interface between MENU and aRGB_

#if defined(SUPPORT_PC)
//-----------------------------------------------------------------------------
/**
* Get PC Pixel clock value
*
* @return 0..100
*/
BYTE GetaRGB_Clock(void) //GetaRGB_Clock
{
	BYTE bTemp;
	WORD wTemp;
	if(Input_aRGBMode==0)
		return 50;

	//return (BYTE)(50+PcBaseClock-aRGB_LLPLLGetDivider());

	wTemp =	aRGB_LLPLLGetDivider();
	PcBaseClock = PCMDATA[ Input_aRGBMode ].hTotal-1;
	bTemp = 50+aRGB_LLPLLGetDivider()-PcBaseClock;
	dPrintf("\nGetaRGB_Clock PcBaseClock:%d LLPLL:%d ret:%bd",PcBaseClock,wTemp,bTemp);
	return bTemp;
}
//-----------------------------------------------------------------------------
/**
* set PC Pixel clock value
*
* @param 0..100
*/
BYTE SetaRGB_Clock(BYTE val)
{
	WORD clock;
	if(Input_aRGBMode==0)
		return 50;

	PcBaseClock = PCMDATA[ Input_aRGBMode ].hTotal-1;
	clock = PcBaseClock + val - 50;
	aRGB_LLPLLSetDivider(clock, 0);	//without INIT

	return GetaRGB_Clock();
}
//-----------------------------------------------------------------------------
/**
* save PC pixel clock value to EEPROM
*/
BYTE ChangeaRGB_Clock(void)
{
	BYTE u_val;
	if(Input_aRGBMode==0)
		return 50;

	u_val = GetaRGB_Clock(); //0..100
	SavePixelClkEE(Input_aRGBMode, u_val);
	return u_val;	
}
#endif

#if 0
BYTE GetHdmiPhase(void) { return 0; }	//BKTODO110511
BYTE SetHdmiPhase(BYTE val) 
{ 
BYTE temp = val;
return 0; 
}
BYTE ChangeHdmiPhase(void) { return 0; }
#endif

//we assume, we already check the input signal.
//external
//	InputMain
//#define IMAGE_TYPE_YUV		0
//#define IMAGE_TYPE_RGB		1
//#define IMAGE_TYPE_DTV		3

//@param
//	type
//-----------------------------------------------------------------------------
/**
* set image effect
*/
void SetImage(BYTE type)
{
	if(type==INPUT_CVBS || type==INPUT_SVIDEO) { //CVBS & SVIDEO
		//reset default internal decoder value.
		//FW uses a default front value, and control the backend image control.
		WriteTW88Page(PAGE1_DECODER );
		WriteTW88(REG110, 0x00);
		WriteTW88(REG111, 0x5C);
		WriteTW88(REG112, 0x11);
		WriteTW88(REG113, 0x80);
		WriteTW88(REG114, 0x80);
		WriteTW88(REG115, 0x00);
		WriteTW88(REG117, 0x30);
	}

	if(type == INPUT_PC ||type == INPUT_DVI || type == INPUT_HDMIPC || type == INPUT_LVDS) {
		//PC, DVI24,DVI16,HDMI_PC

		ImgAdjSetContrastY( 		GetVideoDatafromEE( EEP_IA_CONTRASE_Y ) );
		ImgAdjSetBrightnessY( 		GetVideoDatafromEE( EEP_IA_BRIGHTNESS_Y ) );

		ImgAdjSetContrastRGB(0, 	GetVideoDatafromEE( EEP_IA_CONTRAST_R ) );
		ImgAdjSetContrastRGB(1, 	GetVideoDatafromEE( EEP_IA_CONTRAST_G ) );
		ImgAdjSetContrastRGB(2, 	GetVideoDatafromEE( EEP_IA_CONTRAST_B ) );

		WriteTW88Page(PAGE2_IMAGE );
		WriteTW88(REG280_IA_HUE_REG,      (ReadTW88(REG280_IA_HUE_REG ) & 0xC0) | IA_HUE_DEFAULT);
		WriteTW88(REG285_IA_CONTRAST_CB,  IA_CONTRAST_CB_DEFAULT);
		WriteTW88(REG286_IA_CONTRAST_CR,  IA_CONTRAST_CR_DEFAULT);
		WriteTW88(REG287_IA_BRIGHTNESS_R, IA_BRIGHTNESS_R_DEFAULT);
		WriteTW88(REG288_IA_BRIGHTNESS_G, IA_BRIGHTNESS_G_DEFAULT);
		WriteTW88(REG289_IA_BRIGHTNESS_B, IA_BRIGHTNESS_B_DEFAULT);
		WriteTW88(REG28B_IA_SHARPNESS,    (ReadTW88(REG28B_IA_SHARPNESS ) & 0xf0) | IA_SHARP_DEFAULT);
	}
	else {
		ImgAdjSetContrastY( 		GetVideoDatafromEE( EEP_IA_CONTRASE_Y ) );
		ImgAdjSetBrightnessY( 		GetVideoDatafromEE( EEP_IA_BRIGHTNESS_Y ) );
		ImgAdjSetSaturation( 		GetVideoDatafromEE( EEP_IA_SATURATION ) );
		ImgAdjSetHUE( 				GetVideoDatafromEE( EEP_IA_HUE ) );
		ImgAdjSetSharpness( 		GetVideoDatafromEE( EEP_IA_SHARPNESS ) );

		WriteTW88Page(PAGE2_IMAGE );
		WriteTW88(REG281_IA_CONTRAST_R,   IA_CONTRAST_R_DEFAULT);
		WriteTW88(REG282_IA_CONTRAST_G,   IA_CONTRAST_G_DEFAULT);
		WriteTW88(REG283_IA_CONTRAST_B,   IA_CONTRAST_B_DEFAULT);
		WriteTW88(REG287_IA_BRIGHTNESS_R, IA_BRIGHTNESS_R_DEFAULT);
		WriteTW88(REG288_IA_BRIGHTNESS_G, IA_BRIGHTNESS_G_DEFAULT);
		WriteTW88(REG289_IA_BRIGHTNESS_B, IA_BRIGHTNESS_B_DEFAULT);
	}
}


//-----------------------------------------------------------------------------
//BKTODO:110518
BYTE SetAspectHW(BYTE mode)
{
	dPrintf("\nAspect %bd ",mode);
#if 0
	switch(mode) {
	case 0:	dPrintf("normal");	break;
	case 1:	dPrintf("zoom");	break;
	case 2:	dPrintf("full");	break;
	case 3:	dPrintf("panorama");	break;
	default: dPrintf("unknown");	break;
	}
#endif
	return 0;
}


BYTE OsdTime;
//-----------------------------------------------------------------------------
/**
* Get OsdChangeTime value
*/
BYTE OsdGetTime(void)
{
//	dPrintf("\nOsdGetTime:%bd",OsdTime);
	return OsdTime;
}
//-----------------------------------------------------------------------------
/**
* increase/decrease OsdChangeTime value
*/
BYTE OsdVaryTime(BYTE dat)
{
	if(dat > OsdTime) {
		if(dat < 10)
			OsdTime = 10;
		else if(dat < 100)
			OsdTime++;
	}
	else if(dat < OsdTime) {
		if(dat < 10)
			OsdTime = 0;
		else 
			OsdTime--;
	}

	dPrintf("\nOsdSetTime:%bd",OsdTime);
	return OsdTime;
}
//-----------------------------------------------------------------------------
/**
* set OsdChangeTime value
*/
BYTE OsdSetTime(BYTE dat)
{
	if(dat <= 100)
		OsdTime = dat;
	if(OsdTime < 5)
		OsdTime = 0;

	dPrintf("\nOsdSetTime:%bd",OsdTime);
	return OsdTime;
}
//-----------------------------------------------------------------------------
/**
* save OsdChangeTime value to EEPROM
*/
BYTE OsdChangeTime(void)
{
	BYTE val;
	val = OsdGetTime();
	EE_Write(EEP_OSD_TIMEOUT, val);
	return (BYTE)(val);
}

BYTE OsdTransRate;	//0..100
//-----------------------------------------------------------------------------
/**
* get Osd Transparent value
*/
BYTE OsdGetTransparent(void) //0..100
{
	return OsdTransRate;
}
//-----------------------------------------------------------------------------
/**
* convert OsdTransRate to HW value(0..127)
*/
BYTE OsdGetTrans(void)
{
	WORD val;

	val = OsdTransRate;	 //0..100
	val *= 127;
	val += 50;
	val /= 100;

	return (BYTE)val;	//0..127
}
//-----------------------------------------------------------------------------
/**
* set Osd Transparent value
*/
BYTE OsdSetTransparent(BYTE dat) //0..100
{
	if(dat <= 100)
		OsdTransRate = dat;

	return OsdTransRate;
}
//-----------------------------------------------------------------------------
/**
* save Osd Transparent value to EEPROM
*/
BYTE OsdChangeTransparent(void)
{
	//BYTE val;
	//val = OsdGetTransparent();
	EE_Write(EEP_OSD_TRANSPARENCY, OsdTransRate);
	return OsdTransRate;
}

BYTE BackLightRate;	//0..100
//-----------------------------------------------------------------------------
/**
* get BackLight Rate
*/
BYTE BackLightGetRate(void) //0..100
{
	return BackLightRate;
}
//-----------------------------------------------------------------------------
/**
* convert BackLight Rate to HW value(0..127)
*/
BYTE BackLightGet(void)
{
	WORD val;

	val = BackLightRate;	 //0..100
	val *= 127;
	val += 50;
	val /= 100;

	return (BYTE)val;	//0..127
}
//-----------------------------------------------------------------------------
/**
* set BackLight Rate
*/
BYTE BackLightSetRate(BYTE dat) //0..100
{
	BYTE val;
	if(dat <= 100)
		BackLightRate = dat;

	val = BackLightGet();
	//only for test
	WriteTW88Page(PAGE0_LEDC);
	WriteTW88(REG0E5, (ReadTW88(REG0E5) & 0x80) | val);

	return BackLightRate;
}

#ifdef SUPPORT_UART1
//-----------------------------------------------------------------------------
/**
* increase BackLight value
*/
BYTE BackLightUp(void)
{
	DECLARE_LOCAL_page
	BYTE reg;

	ReadTW88Page(page);
	WriteTW88Page(PAGE0_LEDC);
	reg = ReadTW88(REG0E5);
	if((reg & 0x7F) < 0x7F) {
		reg++;
		WriteTW88(REG0E5, reg);
	}
	WriteTW88Page(page);
	return (reg & 0x7F);
}
//-----------------------------------------------------------------------------
/**
* decrease BackLight value
*/
BYTE BackLightDown(void)
{
	DECLARE_LOCAL_page
	BYTE reg;

	ReadTW88Page(page);
	WriteTW88Page(PAGE0_LEDC);
	reg = ReadTW88(REG0E5);
	if((reg & 0x7F) != 0) {
		reg--;
		WriteTW88(REG0E5, reg);
	}
	WriteTW88Page(page);
	return (reg & 0x7F);
}
#endif

//-----------------------------------------------------------------------------
/**
* save BackLight Rate value to EEPROM
*/
BYTE BackLightChangeRate(void)
{
	EE_Write(EEP_BACKLIGHT, BackLightRate);
	return BackLightRate;
}
#if 0
BYTE GetBackLight(void)
{...}
BYTE SetBackLight(BYTE value)
{...}
BYTE ChangeBackLight(BYTE value)
{...}
#endif

#ifdef SUPPORT_GAMMA
//-----------------------------------------------------------------------------
//// RGB Gamma Text File 

CODE BYTE GammaRed[]={
0x00,0x01,0x03,0x04,0x06,0x07,0x09,0x0A,0x0C,0x0D,0x0F,0x11,0x12,0x14,0x15,0x16,
0x18,0x19,0x1B,0x1D,0x1E,0x1F,0x21,0x22,0x24,0x25,0x27,0x28,0x2A,0x2B,0x2C,0x2E,
0x2F,0x31,0x32,0x34,0x35,0x36,0x38,0x39,0x3B,0x3C,0x3D,0x3F,0x40,0x42,0x43,0x45,
0x46,0x48,0x49,0x4A,0x4B,0x4D,0x4E,0x50,0x51,0x52,0x54,0x55,0x57,0x58,0x59,0x5B,
0x5C,0x5D,0x5F,0x60,0x61,0x63,0x64,0x65,0x66,0x68,0x69,0x6A,0x6C,0x6D,0x6E,0x70,
0x71,0x72,0x73,0x75,0x76,0x77,0x78,0x7A,0x7B,0x7C,0x7D,0x7F,0x80,0x81,0x82,0x84,
0x85,0x87,0x88,0x89,0x8A,0x8B,0x8D,0x8E,0x8F,0x90,0x91,0x93,0x94,0x95,0x96,0x97,
0x98,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,0xA0,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,
0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,
0xBB,0xBC,0xBD,0xBE,0xBF,0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,
0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD8,
0xD9,0xDA,0xDB,0xDC,0xDD,0xDD,0xDE,0xDF,0xE0,0xE0,0xE1,0xE2,0xE3,0xE3,0xE4,0xE5,
0xE6,0xE6,0xE7,0xE8,0xE8,0xE9,0xEA,0xEA,0xEB,0xEC,0xEC,0xED,0xEE,0xEE,0xEF,0xEF,
0xF0,0xF1,0xF1,0xF2,0xF2,0xF3,0xF3,0xF4,0xF4,0xF5,0xF5,0xF6,0xF6,0xF7,0xF7,0xF8,
0xF8,0xF8,0xF9,0xF9,0xFA,0xFA,0xFA,0xFB,0xFB,0xFB,0xFC,0xFC,0xFC,0xFC,0xFD,0xFD,
0xFD,0xFD,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
};

CODE BYTE GammaBlue[]={
0x00,0x01,0x03,0x04,0x06,0x07,0x09,0x0A,0x0C,0x0D,0x0F,0x11,0x12,0x14,0x15,0x16,
0x18,0x19,0x1B,0x1D,0x1E,0x1F,0x21,0x22,0x24,0x25,0x27,0x28,0x2A,0x2B,0x2C,0x2E,
0x2F,0x31,0x32,0x34,0x35,0x36,0x38,0x39,0x3B,0x3C,0x3D,0x3F,0x40,0x42,0x43,0x45,
0x46,0x48,0x49,0x4A,0x4B,0x4D,0x4E,0x50,0x51,0x52,0x54,0x55,0x57,0x58,0x59,0x5B,
0x5C,0x5D,0x5F,0x60,0x61,0x63,0x64,0x65,0x66,0x68,0x69,0x6A,0x6C,0x6D,0x6E,0x70,
0x71,0x72,0x73,0x75,0x76,0x77,0x78,0x7A,0x7B,0x7C,0x7D,0x7F,0x80,0x81,0x82,0x84,
0x85,0x87,0x88,0x89,0x8A,0x8B,0x8D,0x8E,0x8F,0x90,0x91,0x93,0x94,0x95,0x96,0x97,
0x98,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,0xA0,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,
0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,
0xBB,0xBC,0xBD,0xBE,0xBF,0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,
0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD8,
0xD9,0xDA,0xDB,0xDC,0xDD,0xDD,0xDE,0xDF,0xE0,0xE0,0xE1,0xE2,0xE3,0xE3,0xE4,0xE5,
0xE6,0xE6,0xE7,0xE8,0xE8,0xE9,0xEA,0xEA,0xEB,0xEC,0xEC,0xED,0xEE,0xEE,0xEF,0xEF,
0xF0,0xF1,0xF1,0xF2,0xF2,0xF3,0xF3,0xF4,0xF4,0xF5,0xF5,0xF6,0xF6,0xF7,0xF7,0xF8,
0xF8,0xF8,0xF9,0xF9,0xFA,0xFA,0xFA,0xFB,0xFB,0xFB,0xFC,0xFC,0xFC,0xFC,0xFD,0xFD,
0xFD,0xFD,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
};

CODE BYTE GammaGreen[]={
0x00,0x01,0x03,0x04,0x06,0x07,0x09,0x0A,0x0C,0x0D,0x0F,0x11,0x12,0x14,0x15,0x16,
0x18,0x19,0x1B,0x1D,0x1E,0x1F,0x21,0x22,0x24,0x25,0x27,0x28,0x2A,0x2B,0x2C,0x2E,
0x2F,0x31,0x32,0x34,0x35,0x36,0x38,0x39,0x3B,0x3C,0x3D,0x3F,0x40,0x42,0x43,0x45,
0x46,0x48,0x49,0x4A,0x4B,0x4D,0x4E,0x50,0x51,0x52,0x54,0x55,0x57,0x58,0x59,0x5B,
0x5C,0x5D,0x5F,0x60,0x61,0x63,0x64,0x65,0x66,0x68,0x69,0x6A,0x6C,0x6D,0x6E,0x70,
0x71,0x72,0x73,0x75,0x76,0x77,0x78,0x7A,0x7B,0x7C,0x7D,0x7F,0x80,0x81,0x82,0x84,
0x85,0x87,0x88,0x89,0x8A,0x8B,0x8D,0x8E,0x8F,0x90,0x91,0x93,0x94,0x95,0x96,0x97,
0x98,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,0xA0,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,
0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,
0xBB,0xBC,0xBD,0xBE,0xBF,0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,
0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD8,
0xD9,0xDA,0xDB,0xDC,0xDD,0xDD,0xDE,0xDF,0xE0,0xE0,0xE1,0xE2,0xE3,0xE3,0xE4,0xE5,
0xE6,0xE6,0xE7,0xE8,0xE8,0xE9,0xEA,0xEA,0xEB,0xEC,0xEC,0xED,0xEE,0xEE,0xEF,0xEF,
0xF0,0xF1,0xF1,0xF2,0xF2,0xF3,0xF3,0xF4,0xF4,0xF5,0xF5,0xF6,0xF6,0xF7,0xF7,0xF8,
0xF8,0xF8,0xF9,0xF9,0xFA,0xFA,0xFA,0xFB,0xFB,0xFB,0xFC,0xFC,0xFC,0xFC,0xFD,0xFD,
0xFD,0xFD,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
};
void DownLoadGamma(void)
{
#if 0
	// Red
	WriteI2CByte(TW88I2CAddress, 0xf0, 0xe8|1 ); 
	WriteI2CByte(TW88I2CAddress, 0xf1, 0x00); //  Start Address
	WriteI2CByte(TW88I2CAddress, 0xf2, GammaRed[0]); //  
	WriteI2CByten(TW88I2CAddress, 0xf2, &GammaRed[1], 255);
	// Blue
	WriteI2CByte(TW88I2CAddress, 0xf0, 0xe8|2 ); 
	WriteI2CByte(TW88I2CAddress, 0xf1, 0x00); //  Start Address
	WriteI2CByte(TW88I2CAddress, 0xf2, GammaGreen[0]); //  
	WriteI2CByten(TW88I2CAddress, 0xf2, &GammaGreen[1], 255);
	// Blue
	WriteI2CByte(TW88I2CAddress, 0xf0, 0xe8|3 ); 
	WriteI2CByte(TW88I2CAddress, 0xf1, 0x00); //  Start Address
	WriteI2CByte(TW88I2CAddress, 0xf2, GammaBlue[0]); //  
	WriteI2CByten(TW88I2CAddress, 0xf2, &GammaBlue[1], 255);
#else
	BYTE i;
	
	WriteTW88Page(2);
	WriteTW88(REG2E0, 0xe0|1 );			//RED 
	WriteTW88(REG2E1, 0x00); 				//Start Address
	for(i=0; i <= 255; i++)
		WriteTW88(REG2E2, GammaRed[i]); 
		
	WriteTW88(REG2E0, 0xe0|2 );			//Blue 
	WriteTW88(REG2E1, 0x00); 				// Start Address
	for(i=0; i <= 255; i++)
		WriteTW88(REG2E2, GammaGreen[i]);  

	WriteTW88(REG2E0, 0xe0|3 ); 			//Blue
	WriteTW88(REG2E1, 0x00); 				//Start Address
	for(i=0; i <= 255; i++)
		WriteTW88(REG2E2, GammaBlue[i]);  
#endif
}
#endif

