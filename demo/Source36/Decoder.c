/**
 * @file
 * DECODER.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	Internal Decoder module 
 ******************************************************************************
 */
#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"
#include "Global.h"

#include "main.h"
#include "Printf.h"
#include "Monitor.h"
#include "I2C.h"
#include "CPU.h"
#include "Scaler.h"
#include "InputCtrl.h"
#include "util.h"

#include "EEPROM.h"
#include "Decoder.h"

#include "FOsd.h"

//-----------------------------------------------------------------------------
/*
	Decoder Signal.


NTSC
ITU-R BT.470-7

			Total  	Active	Blank		ScanLine
			Samples
NTSC		858		720		138			525@60Hz
PAL			864		720		144			625@50Hz


525




PAL
525




*/
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
/**
* check Video Loss
*
* register
*	R101[0]
*
* oldname: CheckDecoderVDLOSS().
*
* @param n: wait counter
* @return
*	0:Video detected
*	1:Video not present. Video Loss
*/ 
BYTE DecoderCheckVDLOSS( BYTE n )
{
	volatile BYTE	mode;
	BYTE start;

#ifdef DEBUG_DEC
	dPrintf("\nDecoderCheckVDLOSS(%d) start",(WORD)n);
#endif
	start = n;

	WriteTW88Page(PAGE1_DECODER );
	while (n--) {
		mode = ReadTW88(REG101);		//read Chip Status
		if (( mode & 0x80 ) == 0 ) {
#ifdef DEBUG_DEC
			dPrintf("->end%bd",start - n);
#endif
			return ( 0 );				//check video detect flag
		}
		delay1ms(10);
	}
#ifdef DEBUG_DEC
	ePrintf("\nDecoderCheckVDLOSS->fail");
#endif
	return ( 1 );						//fail. We loss the Video
}


//---------------------------------------------
/**
* description
*	input data format selection
*	if input is PC(aRGB),DVI,HDMI, you have to set.  ??
* @param
*	0:YCbCr 1:RGB
*
* CVBS:0x40
* SVIDEO:0x54. IFSET:SVIDEO, YSEL:YIN1
*/
#if 0
//void DecoderSetPath(BYTE path)
//{
//	WriteTW88Page(PAGE1_DECODER );	
//	WriteTW88(REG102, path );   		
//}
#endif

//R104 HSYNC Delay Control


//
//parameter
//	input_mode	0:RGB mode, 1:decoder mode
//register
//	moved from R105 to REG1E8
#if 0
//void DecoderSetAFE(BYTE input_mode)
//{
//	WriteTW88Page(PAGE1_DECODER );	
//	if(input_mode==0) {
//		WriteTW88(REG105, (ReadTW88(REG105) & 0xF0) | 0x04);	//? C is for decoder, not RGB	
//	}
//	else {
//		WriteTW88(REG105, (ReadTW88(REG105) | 0x0F));	
//	}
//}
#endif

#ifdef UNCALLED_SEGMENT
//-----------------------------------------------------------------------------
//register
//	R106[2]
//	R106[1]
//	R106[0]
//void DecoderPowerDown(BYTE fOn)
//{
//	WriteTW88Page(PAGE1_DECODER)
//	if(fOn) WriteTW88(REG106, ReadTW88(REG106) | 0x07);
//	else    WriteTW88(REG106, ReadTW88(REG106) & ~0x07);
//}
#endif

/**
* desc 
* 	set input crop for Decoder
*
*
* register
*	vDelay		R107[7:6]R108[7:0]
*	vActive		R107[5:4]R109[7:0]
*	hDelay		R107[3:2]R10A[7:0]
*	hActive		R107[1:0]R10B[7:0]
*
*		hDelay hActive	vDelay vActive
* NTSC	8		720		21		240
* PAL	6		720		23		288	
*
*/
void DecoderSetInputCrop(WORD hDelay, WORD hActive, WORD vDelay, WORD vActive)
{
	BYTE bTemp;
	WriteTW88Page(PAGE1_DECODER);
	bTemp  = vDelay >> 8;	bTemp <<= 2;
	bTemp |= vActive >> 8;	bTemp <<= 2;
	bTemp |= hDelay >> 8;	bTemp <<= 2;
	bTemp |= hActive >> 8;
	WriteTW88(REG107, bTemp);
	WriteTW88(REG108, (BYTE)vDelay);
	WriteTW88(REG109, (BYTE)vActive);
	WriteTW88(REG10A, (BYTE)hDelay);
	WriteTW88(REG10B, (BYTE)hActive);
}
//void DecoderSetVInputCrop(WORD vDelay, WORD vActive)
//{
//	BYTE bTemp;
//	WriteTW88Page(PAGE1_DECODER);
//	bTemp  = vDelay >> 8;	bTemp <<= 2;
//	bTemp |= vActive >> 8;	bTemp <<= 4;
//
//	WriteTW88(REG107, (ReadTW88(REG107) & 0x0F) | bTemp);
//	WriteTW88(REG108, (BYTE)vDelay);
//	WriteTW88(REG109, (BYTE)vActive);
//}


//-----------------------------------------------------------------------------
/**
* desc: set/get vertical delay
* @param	
* output
*
* register
*	R107[7:6]R108[7:0]
*/
//#if 1//def UNCALLED_SEGMENT
//void DecoderSetVDelay(WORD delay)
//{
//	WriteTW88Page(PAGE1_DECODER );		// get VDelay from Decoder
//	WriteTW88(REG107, (ReadTW88(REG107 ) & 0x3F) | ( (delay & 0x0300) >> 2)); 
//	WriteTW88(REG108, (BYTE)delay );
//}
//#endif
//-----------------------------------------------------------------------------
/**
* get decoder vertical delay value
*/
WORD DecoderGetVDelay(void)
{
	WORD vDelay;

	WriteTW88Page(PAGE1_DECODER );		// get VDelay from Decoder
	vDelay = ReadTW88(REG107 ) & 0xC0; 
	vDelay <<= 2;
	vDelay |= ReadTW88(REG108 );

	return vDelay;
}

//-----------------------------------------------------------------------------
/**
* set decoder vertical active length
*
*	register
*	R107[5:4]R109[7:0]
*/
//void DecoderSetVActive(WORD length)
//{
//	WriteTW88Page(PAGE1_DECODER );		
//	WriteTW88(REG107, (ReadTW88(REG107) & 0xCF) | ( (length & 0x0300) >> 4)); 
//	WriteTW88(REG109, (BYTE)length );
//}
//#ifdef UNCALLED_SEGMENT
//-----------------------------------------------------------------------------
WORD DecoderGetVActive(void)
{
	WORD vActive;

	WriteTW88Page(PAGE1_DECODER );
	vActive = ReadTW88(REG107 ) & 0x30; 
	vActive <<= 4;
	vActive |= ReadTW88(REG109 );

	return vActive;
}
//#endif

//-----------------------------------------------------------------------------
/**
* desc:set/get Horizontal delay
* register
*	R107[3:2]R10A[7:0]
*/
#if 0
//void DecoderSetHDelay(WORD delay)
//{
//	WriteTW88Page(PAGE1_DECODER );		// get VDelay from Decoder
//	WriteTW88(REG107, (ReadTW88(REG107 ) & 0xF3) | ( (delay & 0x0300) >> 6)); 
//	WriteTW88(REG10A, (BYTE)delay );
//}
#endif
//#ifdef UNCALLED_SEGMENT
WORD DecoderGetHDelay(void)
{
	WORD hDelay;

	WriteTW88Page(PAGE1_DECODER );		// get VDelay from Decoder
	hDelay = ReadTW88(REG107 ) & 0x0C; 
	hDelay <<= 6;
	hDelay |= ReadTW88(REG10A );

	return hDelay;
}
//#endif

//-----------------------------------------------------------------------------
/**
* desc: set/get Horizontal active
* register
*	R107[1:0]R10B[7:0]
*/
#if 0 //def UNCALLED_SEGMENT
//void DecoderSetHActive(WORD length)
//{
//	WriteTW88Page(PAGE1_DECODER );		// get VDelay from Decoder
//	WriteTW88(REG107, (ReadTW88(REG107 ) & 0xFC) | ( (length & 0x0300) >> 8)); 
//	WriteTW88(REG10B, (BYTE)length );
//}
#endif
//#ifdef UNCALLED_SEGMENT
WORD DecoderGetHActive(void)
{
	WORD hActive;

	WriteTW88Page(PAGE1_DECODER );		// get VDelay from Decoder
	hActive = ReadTW88(REG107 ) & 0x03; 
	hActive <<= 8;
	hActive |= ReadTW88(REG10B );

	return hActive;
}
//#endif

//-----------------------------------------------------------------------------
/**
* read detected decoder mode
*
*	register
*	R11C[7]		0:idle, 1:detection in progress
*	R11C[6:4]	000: NTSC
*				001: PAL
*				...
*				111:N/A
*/
BYTE DecoderReadDetectedMode(void)
{
	BYTE mode;
	WriteTW88Page(PAGE1_DECODER);
	mode = ReadTW88(REG11C);
	mode >>= 4;
	return mode;
}



//-----------------------------------------------------------------------------
/**
* check detected decoder video input standard
*
*	To get a stable the correct REG11C[6:4] value,
*		read REG101[6] and REG130[7:5] also.
*	I saw the following values(BK110303)
* 		E7 E7 67 67 87 87 87 87 ..... 87 87 87 87 87 87 87 87 87 07 07 07 .... 
* 		B7 B7 B7 37 37 87 87 87 ..... 87 87 87 87 87 87 87 87 87 07 07 07 07 07 07 07
*
* oldname: CheckDecoderSTD
*
* register
*	R11C[6:4].
* 	R101[6].
*	R130[7:5].
* @return
*	0x80: filed.
*	other: detected standard value.
*/
BYTE DecoderCheckSTD( BYTE n )
{
	volatile BYTE	r11c,r101,r130;
	BYTE start=n;
	BYTE count;
#ifdef DEBUG_DEC
	ePrintf("\nDecoderCheckSTD(%d) start",(WORD)n);
#endif
	WriteTW88Page(PAGE1_DECODER );		// set Decoder page
	
	count=0;
	while (n--) {
		r11c = ReadTW88(REG11C);
		if (( r11c & 0x80 ) == 0 ) {
			r101 = ReadTW88(REG101);
			r130 = ReadTW88(REG130);
#ifdef DEBUG_DEC
			dPrintf("\n%02bx:%02bx-%02bx-%02bx ",start-n, r11c, r101,r130);
#endif
			if((r101 & 0x40) && ((r130 & 0xE0)==0)) {
#ifdef DEBUG_DEC
				ePrintf("->success:%d",(WORD)start-n);
#endif
				if(count > 4)
					return (r11c);
				count++;
			}
 		}
		delay1ms(5);
	}
#ifdef DEBUG_DEC
	ePrintf("->fail");
#endif
	return ( 0x80 );
}

//-----------------------------------------------------------------------------
/**
* set decoder freerun mode
*
* example
*   DecoderFreerun(DECODER_FREERUN_60HZ);
*
* R133[7:6]
* @param
*	mode	0:AutoMode
*			1:AutoMode
*			2:60Hz
*			3:50Hz
*/
void DecoderFreerun(BYTE mode)
{
	WriteTW88Page(PAGE1_DECODER );
	WriteTW88(REG133, (ReadTW88(REG133) & 0x3F) | (mode<<6));
}

//=============================================================================
// Change to DECODER. (CVBS & SVIDEO)
//=============================================================================

//-----------------------------------------------------------------------------
/**
* check and set the decoder input
*
* @return
*	0: success
*	1: VDLOSS
*	2: No Standard
*	3: Not Support Mode
*
* extern
*	InputSubMode
*
* measure result
*
* NTSC									PAL		
*	vTotal:262 vFreq:59						vTotal:313 vFreq:49.99
*	hSync:1 vSync:0							hSync:1 vSync:0
*
*	hActive	 		vActive
*	2,73			1,1	   					2,65		1,1
*	1713,1711		261,262					1725,1086	311,312
*	---------		-------					----		----
*	1712			261						1724		311
*
*	H Rise to Act End:1						1								   	
*	VS Rise Pos. in on e H:1317				1327 	
*	FIFO Read Start :0						0
*
*   1440x480i	1716,38,124,114				1440x576i	1728,24,126,138
*				262,4,3,15								312,2,3,19
*
*	DecInputCrop
*		8,720,21,240						6,720,23,288
*/
BYTE CheckAndSetDecoderScaler( void )
{
	BYTE	mode;
	DWORD	vPeriod, vDelay;
	BYTE vDelayAdd;
	DWORD x_ratio, y_ratio;
	DWORD dTemp;

#ifdef DEBUG_DEC
	dPrintf("\nCheckAndSetDecoderScaler start.");
#endif	
	if ( DecoderCheckVDLOSS(100) ) {
#ifdef DEBUG_DEC
		ePuts("\nCheckAndSetDecoderScaler VDLOSS");
#endif
		DecoderFreerun(DECODER_FREERUN_60HZ);
		ScalerSetFreerunManual( ON );
		return( 1 );
	}
	//get standard
	mode = DecoderCheckSTD(100);
	if ( mode == 0x80 ) {
	    ePrintf("\nCheckAndSetDecoderScaler NoSTD");
		return( 2 );
	}
	mode >>= 4;
	InputSubMode = mode;

	VideoAspect = GetAspectModeEE();

	//read VSynch Time+VBackPorch value
	vDelay = DecoderGetVDelay();

	//reduce VPeriod to scale up.
	//and adjust V-DE start.

	//720x240 => 800x480
	x_ratio = PANEL_H;
	x_ratio *=100;
	x_ratio /= 720;
	y_ratio = PANEL_V;
	y_ratio *=100;
	y_ratio /= 480;
#ifdef DEBUG_DEC
	dPrintf("\nXYRatio X:%ld Y:%ld",x_ratio,y_ratio);
#endif

	if(VideoAspect==VIDEO_ASPECT_ZOOM) {
		if(x_ratio > y_ratio) {
			dPrintf(" use x. adjust Y");
			y_ratio = 0;
		}
		else {
			dPrintf(" use y. adjust X");	
			x_ratio = 0;
		}
	}
	else if(VideoAspect==VIDEO_ASPECT_NORMAL) {
		if(x_ratio > y_ratio) {
			dPrintf(" use y. adjust X");
			x_ratio = 0;
		}
		else {
			dPrintf(" use x. adjust Y");
			y_ratio = 0;
		}
	}
	else {
		x_ratio = 0;
		y_ratio = 0;
	}
	//720x288 => 800x480

	if ( mode == 0 ) {				// NTSC(M)
		vPeriod = 228;				// NTSC line number.(reduced from 240 to 228)
		//vDelay += 12; 			// (6 = ( 240-228 ) / 2) times 2 because. 240->480
		//vDelay += 27;				// for V Back Porch & V top border
		vDelayAdd = 39;

		if(VideoAspect==VIDEO_ASPECT_ZOOM) {
			vDelayAdd += 30;
			vDelayAdd += 5; //???
		}

		//crop value
		//	---normal---
		//	hStart:8   hActive:720
		//	vStart:21  hActive:240				ScalerOutputVDE = 27*480/228 =42
		//	---overscan---
		//	hStart:8  	  hActive:720
		//	vStart:21+6   vActive:228(240*0.95)  ScalerOutputVDE = 27*480/228 = 56.8


		//DecoderSetVActive(240);		//set:240 0x0F0
		////BK130123 When it change from PAL to NTSC, we need Vdelay 18.	REG108[]
		//DecoderSetVDelay(18);
		//DecoderSetInputCrop(8+3,720-6,21,240);
		DecoderSetInputCrop(3+3,720-6,21,240);
		vDelay=21;
//#if defined(PANEL_AUO_B133EW01)
//		vDelayAdd = 74;
//#else
//		vDelayAdd = 36;
//#endif
		//if PANEL_V is 800, vDelayAdd is 74.
		//if PANEL_V is 480, vDelayAdd is 36.
		dTemp = (DWORD)(vDelay+6) * PANEL_V / 228;
		dTemp += 1;
		vDelayAdd = dTemp - vDelay;


		WriteTW88(REG040, ReadTW88(REG040) & ~0x10);	//recover. pal using "1".

		//prepare info
		FOsdSetInputMainString2FOsdMsgBuff();	//GetInputMainString(FOsdMsgBuff);									 	
		TWstrcat(FOsdMsgBuff," NTSC");			//BK110811. call FOsdCopyMsgBuff2Osdram(OFF); to draw
	}
	else if ( mode == 1 ) {			 //PAL(B,D,G,H,I)
		vPeriod = 275;				// PAL line number,(Reduced from 288 to 275)
#if 0
		//vDelay += 7; 				// 6.7 = ( 288-275 ) / 2
		//vDelay += 2;				// add some more for V Back Porch & V Top border
		vDelayAdd = 25;
#else
		//vDelay += 14; 			// (6.7 = ( 288-275 ) / 2  ) * 2
		//vDelay += 25;				// add some more for V Back Porch & V Top border
		vDelayAdd = 39;
#endif
		if(VideoAspect==VIDEO_ASPECT_ZOOM)
			vDelayAdd += 33;

		//crop value
		//	---normal---
		//	hStart:6  hActive:720
		//	vStart:23 vActive:288			ScalerOutputVDE = 23*480/288=38.3
		//	---overscan---
		//	hStart:6  hActive:720	
		//	vStart:23+(13/2)  vActive:275(288*0.95)	 ScalerOutputVDE = 29.5*480/275=51.4

		//DecoderSetVActive(288);		//set:288. 0x120
		////BK1211129 Vdelay need 22.	REG108[]
		//DecoderSetVDelay(22);
//		DecoderSetInputCrop(6+3,720-6,23,288);
		DecoderSetInputCrop(2+3,720-6,23,288);
		vDelay = 23;	//real vDelay is a 23.9.
//#if defined(PANEL_AUO_B133EW01)
//		vDelayAdd = 53;
//#else
//		vDelayAdd = 30;
//#endif	

		dTemp = (DWORD)(20+1+6) * PANEL_V / 275;
		dTemp += 1;
		vDelayAdd = dTemp - 20;

	
		WriteTW88(REG040, ReadTW88(REG040) | 0x10);	//clk pol invert. clear some noise.
		 
		//prepare info
		FOsdSetInputMainString2FOsdMsgBuff();	//GetInputMainString(FOsdMsgBuff);									 	
		TWstrcat(FOsdMsgBuff," PAL");			//BK110811. call FOsdCopyMsgBuff2Osdram(OFF); to draw
	}
	//BKTODO: Support more mode
	//0 = NTSC(M)          
	//1 = PAL (B,D,G,H,I)          
	//2 = SECAM          
	//3 = NTSC4.43
	//4 = PAL (M)            
	//5 = PAL (CN)                     
	//6 = PAL 60  
	else if ( mode == 3 //MTSC4
		   || mode == 4 //PAL-M
	       || mode == 6 //PAL-60			 
	) {				
		vPeriod = 228;
		vDelayAdd = 39;
 		if(VideoAspect==VIDEO_ASPECT_ZOOM) {
			vDelayAdd += 30;
			vDelayAdd += 5; //???
		}
 		//DecoderSetVActive(240);		//set:240 0x0F0
		DecoderSetInputCrop(8+3,720-6,21,240);
		vDelay=21;
#if defined(PANEL_AUO_B133EW01)
		vDelayAdd = 74;
#else
		vDelayAdd = 36;
#endif

		WriteTW88(REG040, ReadTW88(REG040) & ~0x10);	//recover. pal using "1".

		//prepare info
		FOsdSetInputMainString2FOsdMsgBuff();									 	
		if(mode==3) TWstrcat(FOsdMsgBuff," NTSC4");		
		if(mode==4) TWstrcat(FOsdMsgBuff," PAL-M");		
		if(mode==6) TWstrcat(FOsdMsgBuff," PAL-60");		
   }     
	else if ( mode == 2 //SECAM
		  ||  mode == 5 //PAL-CN
	) {	
		vPeriod = 275;
		vDelayAdd = 39;			
		if(VideoAspect==VIDEO_ASPECT_ZOOM)
			vDelayAdd += 33;

		//DecoderSetVActive(288);		//set:288. 0x120
		DecoderSetInputCrop(6+3,720-6,23,288);
		vDelay = 23;
#if defined(PANEL_AUO_B133EW01)
		vDelayAdd = 53;
#else
		vDelayAdd = 28;
		vDelayAdd += 3;	//??
#endif		
		 
		WriteTW88(REG040, ReadTW88(REG040) & ~0x10);	//recover. pal using "1".

		//prepare info
		FOsdSetInputMainString2FOsdMsgBuff();									 	
		if(mode==2) TWstrcat(FOsdMsgBuff," SECAM");
		if(mode==4) TWstrcat(FOsdMsgBuff," PAL-CN");
	}
	else {
#ifdef DEBUG_DEC
		ePrintf( "\nCheckAndSetDecoderScaler Input Mode %bd does not support now", mode );
#endif
		return(3);
	}
	
	ScalerSetLineBufferSize(720-6);	//BK120116	- temp location. pls remove

	if(y_ratio) ScalerSetHScaleWithRatio(720-6, (WORD)y_ratio);
	else		ScalerSetHScale(720-6);					//PC->CVBS need it.
	if(x_ratio)	ScalerSetVScaleWithRatio(vPeriod, (WORD)x_ratio);
	else 		ScalerSetVScale(vPeriod);				//R206[7:0]R205[7:0]	= vPeriod
	
	ScalerWriteVDEReg(vDelay+vDelayAdd);			//R215[7:0]=vDelay, R217[3:0]R216[7:0]=PANEL_V

#ifdef DEBUG_DEC
 	//dPrintf( "\nInput_Mode:%02bx VDE_width:%ld, vBackPorch:%ld", mode, vPeriod, vDelay );
	ePrintf( "\nInput_Mode:%s VDE_width_for_scaler:%ld, V-DE:%ld+%bd", mode ? "PAL":"NTSC", vPeriod, vDelay,vDelayAdd );
#endif	
	return(0);
}


//-----------------------------------------------------------------------------
/**
* Change to Decoder
*
* extern
*	InputMain
*	InputSubMode
* @param
*	fSVIDEO		0:CVBS, 1:SVIDEO
* @return
*	- 0: success
*	- 1: No Update happen
*	- 2: No Signal or unknown video sidnal.
*	- 3: NO STD
* @see InitInputAsDefault
* @see CheckAndSetDecoderScaler
* @see VInput_enableOutput
* @see VInput_gotoFreerun
*/
BYTE ChangeDecoder(BYTE fSVIDEO)
{
	BYTE ret;

	if(fSVIDEO) {
		if ( InputMain == INPUT_SVIDEO ) {
#ifdef DEBUG_DEC
			dPrintf("\nSkip ChangeSVIDEO");
#endif
			return(1);
		}
		InputMain = INPUT_SVIDEO;
	}
	else {
		if ( InputMain == INPUT_CVBS ) {
#ifdef DEBUG_DEC
			dPrintf("\nSkip ChangeCVBS");
#endif
			return(1);
		}
		InputMain = INPUT_CVBS;
	}
	InputSubMode = 7; //N/A

	if(GetInputMainEE() != InputMain) 	
		SaveInputMainEE( InputMain );

	//----------------
	// initialize video input
	InitInputAsDefault();


	//BKFYI: We need a delay before call DecoderCheckVDLOSS() on CheckAndSetDecoderScaler()
	//But, if fRCDMode, InputMode comes from others, not CVBS, not SVIDEO. We don't need a delay 
	delay1ms(350);

	//
	// Check and Set 
	//
	ret = CheckAndSetDecoderScaler();	//same as CheckAndSetInput()
	if(ret==ERR_SUCCESS) {
		//success
		VInput_enableOutput(0);
		return 0;
	}
	//------------------
	// NO SIGNAL
	//------------------
	VInput_gotoFreerun(ret-1);	//1->0:NoSignal 2->1:NO STD
	return (ret+1);	 //2:NoSignal 3:NO STD
}

//-----------------------------------------------------------------------------
/**
* Change to CVBS
*
* @return
*	- 0: success
*	- 1: No Update happen
*	- 2: No Signal or unknown video sidnal.
*	- 3: NO STD
* @see ChangeDecoder
*/
BYTE ChangeCVBS( void )
{
	return ChangeDecoder(0);
}

//-----------------------------------------------------------------------------
/**
* Change to SVIDEO
*
* @return
*	- 0: success
*	- 1: No Update happen
*	- 2: No Signal or unknown video sidnal.
*	- 3: NO STD
* @see ChangeDecoder
*/
BYTE	ChangeSVIDEO( void )
{
	return ChangeDecoder(1);
}


//=============================================================================
// for FontOSD MENU
//=============================================================================
#ifdef SUPPORT_FOSD_MENU
//-----------------------------------------------------------------------------
/**
* Is it a video Loss State
*
* @return
*	- 1:If no Input
*	- 0:Found Input
*/
BYTE DecoderIsNoInput(void)
{
	DECLARE_LOCAL_page
	BYTE ret;
	
	ReadTW88Page(page);
	WriteTW88Page(PAGE1_DECODER);
	ret = TW8835_R101;	
	WriteTW88Page(page);
	
	if(ret & 0x80)
		return 1;	//No Input
	return 0;		//found Input
}
#endif

#ifdef SUPPORT_FOSD_MENU
//-----------------------------------------------------------------------------
/**
* read video input standard
*
* BKTODO120201 Pls, remove this
*/
BYTE DecoderReadVInputSTD(void)
{
	DECLARE_LOCAL_page
	BYTE std, ret;

	ReadTW88Page(page);
	
	if( DecoderIsNoInput() ) ret = 1; // Noinput!!	BUGBUG


	std = DecoderReadDetectedMode();
	if(std & 0x08) 
		ret = 0xff;	// Detection in progress..
	else
		ret = std + 1;

	WriteTW88Page(page );
	return (ret);
}
#endif


