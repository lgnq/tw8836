/**
 * @file
 * VADC.c 	==>aRGB
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	Video HighSpeed ADC module for aRGB 
 *
 *  VADC means Video ADC. we also call it as aRGB.
 *  VADC consist of "SYNC Processor" + "LLPLL" + "ADC".
 *  Component & PC inputs use VADC module. 	
 ******************************************************************************
 */

//-------------------------------------------------------------------
// global function
//	CheckAndSetComponent
//	CheckAndSetPC
//	aRGB_SetDefault
//-------------------------------------------------------------------
#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "Printf.h"
#include "Monitor.h"
#include "I2C.h"
#include "CPU.h"
#include "global.h"
#include "Scaler.h"

#include "InputCtrl.h"

#include "measure.h"
#include "PC_modes.h"
	
#include "aRGB.h"
#include "eeprom.h"
#include "settings.h"

//#ifdef SUPPORT_COMPONENT
//#include "data\DataComponent.inc"
//#endif

#include "util.h"
#include "FOsd.h"


#ifdef DEBUG_PC_COLOR
#define pcPrintf	dPrintf
#else 
#define pcPrintf	nullFn
#endif

XDATA	BYTE	Input_aRGBMode;

/*		

Component PATH
==============
									   +=>vSync=> VSYNCO => 		   Scaler
									   |
 SOG  => SOG Slicer  =>	SyncSeperator =+   
									   |
									   +=>hSync=> LLPLL  => HSO =>	   Scaler


PC PATH
=======

 HSYNC => LLPLL =>	HSO	  =>	 Scaler
								 Scaler
 VSYNC          =>


Module Polarity
===============					                                  
								Fixed
                module			output
				======		   polarity
			+----------+         +-+
			|	SOG	   |   ==>   | |
			|	Slicer |        -+ +-
			+----------+


   Request						Fixed
   INPUT        module          output
   polarity						polarity
   =====		=======			======
    +-+ 	  +---------+		-+ +-
    | |	  =>  |  Sync   |  =>    | |
   -+ +-      |Seperator|		 +-+
  active      +---------+      active 
    High					    Low
                             V: VSYNCO
							 H: CS_PAS


   Request						Fixed
   INPUT        module          output
   polarity						polarity
   =====		=======			======
   -+ +-	  +---------+		 +-+	 
    | |	  =>  |  LLPLL  |  =>    | |
	+-+		  +---------+		-+ +-
  active					   active 
    low						    high


   Prefer
   INPUT        module    
   =====	   =======	
    +-+   	  +---------+
    | |   =>  | measure | 
   -+ +-	  +---------+
   active  				
    high   				

*/

/**
* Description
*	Set aRGB signal path.
* @param input
*	0:component, 1:PC.
* registers
*	REG1C0[3]
*	REG1C0[4]
*	REG1CC[4]
*	REG1CC[3:2] - always HSO.
*	REG1CC[0] - if HSO, always 0.(ByPass)
*/
void aRGB_setSignalPath(BYTE fInputPC)
{
	BYTE bTemp;

	bTemp = ReadTW88(REG1C0);
	bTemp &= ~0x18;
	if(fInputPC)  bTemp |=  0x08;
	else		  bTemp |=  0x10;
	WriteTW88(REG1C0, bTemp);

	bTemp = ReadTW88(REG1CC);
	bTemp &= ~0x1D;
	if(fInputPC) bTemp |= 0x10;
	WriteTW88(REG1CC,bTemp);
}



/**
* Description
*	Select LLPLL input polarity.
*   This function needs a correct Path.
*	Execute aRGB_setSignalPath first.
*
@param fInputPC
	0:component
	1:PC
@param fActiveHighPol.Positive
	if input is component, select value will be bypass,REG1C0[2]=1.
		because, video path will be SOG_Slicer=>SyncSeperator. 
		and hPol is active low that LLPLL requests. 
	if input is PC, use fPol value that comes from REG1C1[6].
		If hSync is active high (REG1C1[6] is 1), invert it. 
		LLPLL requests an Active Low.
*/
void LLPLL_setInputPolarity(BYTE fInputPC, BYTE fActiveHighPol)
{
	BYTE bTemp;

	bTemp = ReadTW88(REG1C0);
	if(fInputPC) {
		if(fActiveHighPol)	bTemp &= ~0x04;	 //invert.
		else				bTemp |=  0x04;  //bypass.			
	}
	else {
		bTemp |=  0x04; //bypass
	}
	WriteTW88(REG1C0, bTemp);
}

/**
* Description
* 	select vSync output polarity.
*
*	scaler & measure like a positive polarity.
*	if input is component, SyncSeperator output is active low.
*		scaler need a inverted sync input.
*	if input is PC, make the scaler input as an active high.
*/
void aRGB_Set_vSyncOutPolarity(BYTE fInputPC, BYTE fActiveHighPol)
{
	BYTE bTemp;

	bTemp = ReadTW88(REG1CC);
	if(fInputPC) {
		if(fActiveHighPol) bTemp &= ~0x02;	//bypass
		else               bTemp |=  0x02;	//invert
	}
	else
		bTemp |= 0x02;		//invert
	WriteTW88(REG1CC,bTemp);
}

#define HPOL_P		0x40	//Horizontal Polarity Positive
#define HPOL_N		0x00	//Horizontal Polarity Negative

#define VPOL_P		0x80
#define VPOL_N		0x00

#define HPOL_X		0		//unknown
#define VPOL_X		0		//unknown

#define INTERLACED	0x01
#define UNKNOWN_0	0


#define HN_VN_P		HPOL_N | VPOL_N
#define HP_VP_P		HPOL_P | VPOL_P	
#define HN_VP_P		HPOL_N | VPOL_P
#define HP_VN_P		HPOL_P | VPOL_N
#define HN_VN_I		HPOL_N | VPOL_N | INTERLACED
#define HP_VP_I		HPOL_P | VPOL_P | INTERLACED	
#define HN_VP_I		HPOL_N | VPOL_P | INTERLACED
#define HP_VN_I		HPOL_P | VPOL_N | INTERLACED

//-----------------------------------------------------------------------------
// component video table.
// Overscanned value.
//-----------------------------------------------------------------------------

#define MEAS_COMP_MODE_480I			0
#define MEAS_COMP_MODE_576I			1
#define MEAS_COMP_MODE_480P			2
#define MEAS_COMP_MODE_576P			3
#define MEAS_COMP_MODE_1080I25		4
#define MEAS_COMP_MODE_1080I30		5
#define MEAS_COMP_MODE_720P50		6
#define MEAS_COMP_MODE_720P60		7
#define MEAS_COMP_MODE_1080P50		8
#define MEAS_COMP_MODE_1080P60		9

//OLD compoment table for 800x400.
//Now, TW8836 need to support several resolution panels.
//     I will use a video timing table 
							//       1      2       3      4      5      6      7      8      9     10
							//   	480i,  576i,   480p, 576p,1080i50,1080i60,720p50,720p60,1080p5,1080p6
//scaled
//code	WORD	YUVDividerPLL[] = { 858,   864,   858,   864,   2460,  2200,  1980,  1980,  2640,  2200 };		//total horizontal pixels
//code	WORD	YUVVtotal[]     = { 262,   312,   525,   625,   562,   562,   750,   750,   1124,  1124 };		//total vertical scan line
//code	BYTE	YUVClampPos[]   = { 128,   128,   64,    58,    40,    32,    38,    38,    14,    14 };		//clamp position offset. R1D7. 
//code	WORD	YUVCropH[]      = { 720,   720,   720,   720,   1920,  1920,  1280,  1280,  1920,  1920 };		// horizontal resolution
//code	WORD	YUVCropV[]      = { 240,   288,   480,   576,   540,   540,   720,   720,   1080,  1080 };		// vertical resolution
//code	WORD	YUVDisplayH[]   = { 700,   700,   700,   700,   1880,  1880,  1260,  1260,  1880,  1880 };		// reduced. R042[3:0]R046[7:0] for overscan
//code	WORD	YUVDisplayV[]   = { 230,   278,   460,   556,   520,   520,   696,   696,   1040,  1040 };		// reduced R042[6:4]R044[7:0] for overscan
//code	WORD	YUVStartH[]     = { 112,   126,   114,   123,   230,   233,   293,   293,   233,   233 };		// 0x040[7:6],0x045 InputCrop
//code	WORD	YUVStartV[]     = { 1,     1,     2,     2,     2,     2,     2,     2,     2,     2 };			// 0x043 InputCrop
//code	BYTE	YUVOffsetH[]    = { 5,     4,     10,    6,     40,    40,    20,    20,    30,    30 };
//code	BYTE	YUVOffsetV[]    = { 48,    48,    48,    48,    28,    26,    24,    25,    26,    26 };		// use as V-DE 0x215	
//code	BYTE	YUVScaleVoff[]  = { 128,   128,   0,     0,     128,   128,   0,     0,     0,     0 };
//code	WORD	MYStartH[]      = { 121,   131,   121,   131,235-44, 235-44,299-40,	299-40,	235-44,235-44 };		// 0x040[7:6],0x045 InputCrop
//code	WORD	MYStartV[]      = { 19,    21,    38,   44, 	20,		20,   25, 	25,   	41, 	41 };	



#if /*defined( SUPPORT_COMPONENT ) || */ defined( SUPPORT_PC ) || defined (SUPPORT_DVI)	|| defined(SUPPORT_BT656)
//CONST struct _PCMODEDATA PCMDATA[] = {
code struct _PCMODEDATA PCMDATA[] = {
//===========================================================================================================
//                       PC mode Table for XGA Panel    17-July, 2002
//===========================================================================================================
// Support
//	0:NotSupport. 1:PC 2:Component 3:DTV
// han		horizontal addressable size, resolution
// van		vertical addressable size, resolution
// vfreq	vertical frequency
// htotal	horizontal total pixels. horizontal period use it to set PLL with (htotal-1)
// vtotal	vertical total lines. vertical period
// hsyncpol we don't need it. 1:Negative,ActiveLow.
// vsyncpol we don't need it.
// hstart	horizontal addressable start(not active start). It will be HsyncWidth+HBackPorch+HLeftBorder.
// vstart	vertical addressable start(not active start). It will be VsyncWidth+VBackPorch+VTopBorder.
// offseth	Offset from hstart for scaler hstart input
// offsetv  Offset from vstart for scaler vstart input
// dummy0 	- for VAN adjust. removed
// dummy1	- for VDE. removed
// IHF      - input HFreq based 100Hz
// IPF		- input pixel clock based 10KHz


// HS = HSYNC Polarity, VS = VSYNC Polarity, 0 = Negative, 1 = Positive
//
//																					Dummy1=VScaleOffset		
// 	                                                            Offset 		Dummy0 	Dummy1 	Dummy2 	Dummy3

// 	SUPPORT  HAN,VAN,VFREQ  	H&V TOTAL  HSP &VSP	Hst,Vst		OffsetH V 	Dummy0 	Dummy1 	Dummy2 	Dummy3
//  	             IVF     	CLOCK+1             HST,VST     				


/* 00*/	{  0,	0,	0,	0, 		0,	0,		0x00,	0,	0/*,		0,	0,		0,		0,		0,		0*/ },		// unknown

/* 01*/	{  0,	720,400,70,		900,449,	HN_VP_P,	108+53,2+34, 	0/*,	10,	2,		0,		0,		0,		0*/ },		//  0: DOS mode
             
/* 02*/	{  0,	640,350,85,		832,445,	HP_VN_P,	64+96,	3+60, 	0/*,		0,	0,		0,		0,		0,		0*/ },		// 640x350x85
/* 03*/	{  0,	640,400,85,		832,445,	HN_VP_P,	64+96,	3+41, 	0/*,		0,	0,		0,		0,		0,		0*/ },		// 640x400x85
/* 04*/	{  0,	720,400,85,		936,446,	HN_VP_P,	72+108,3+42, 	0/*,		0,	0,		0,		0,		0,		0*/ },		// 720x400x85

/*>05*/	{  1,	640,480,60,		800,525,	HN_VN_P,	96+40, 2+25, 	-1 /*,		5,-3,		1,		2,		315,	0*/ },		// 640x480x60
/* 06*/	{  1,	640,480,72,		832,520,	HN_VN_P,	40+120,3+28, 	-1/*,		5,1,		1,		2,		377,	0*/ },		// 640x480x72
/* 07*/	{  1,	640,480,75,		840,500,	HN_VN_P,	60+120,3+16, 	0/*,		5,-8,		4,		6,		375,	0*/ },		// 640x480x75
/* 08*/	{  0,	640,480,85,		832,509,	HN_VN_P,	56+80,3+25, 	0 /*,		5,-6,		3,		4,		433,	0*/ },		// 640x480x85

/* 09*/	{  1,	800,600,56,		1024,625,	HP_VP_P,	72+128,2+22, 	0/*,		5,	-2,		1,		1,		351,	0*/ },		// 800x600x56
/*>10*/	{  1,	800,600,60,		1056,628,	HP_VP_P,	128+88,4+23, 	0/*,		5,	-4,		1,		2,		0,		0*/ },		// 800x600x60
/* 11*/	{  1,	800,600,70,		1040,625,	HP_VN_P,	80+111,3+20, 	0/*,		5,	-5,		1,		3,		0,		0*/ },		// 800x600x70
/* 12*/	{  1,	800,600,72,		1040,666,	HP_VP_P,	120+64,6+23, 	0/*,		5,	-6,		1,		2,		481,	0*/ },		// 800x600x72
/* 13*/	{  1,	800,600,75,		1056,625,	HP_VP_P,	80+160,3+21, 	0/*,		5,	-3,		1,		1,		469,	0*/ },		// 800x600x75
/* 14*/	{  0,	800,600,85,		1048,631,	HP_VP_P,	64+152,3+27, 	0/*,		5,	-3,		1,		1,		537,	0*/ },		// 800x600x85
/* 15*/	{  0,	800,600,120,	960,636,	HP_VN_P,	32+80,4+29, 	0/*,		0,	0,		0,		0,		0,		0*/ },		// 800x600x120	REDUCED

/* 16*/	{  1,	848,480,60,		1088,517,	HP_VP_P,	112+112,8+23, 	0/*,		0,	0,		0,		0,		0,		0*/ },		// 848x480x60

/* 17*/	{  1,	960,600,60,		1232,622,	HP_VN_P,	0+120,0+23, 	0/*,		5,	-6,		0,		0,		0,		0*/ },		// 800x600x60

/*>18*/	{  1,	1024,768,60,	1344,806,	HN_VN_P,	136+160,6+29, 	0/*,		4,	-6,		2,		1,		0,		0*/ },		// 1024x768x60
/*19*/	{  1,	1024,768,70,	1328,806,	HN_VN_P,	136+144,6+29, 	0},		// 75
/*20*/	{  1,	1024,768,75,	1312,800,	HP_VP_P,	96+176,3+28, 	0/*,		5,	-7,		4,		3,		600,	0*/ },		// 1024x768x75
/*21*/	{  0,	1024,768,85,	1376,808,	HN_VN_P,	96+208,3+36, 	0/*,		0,	0,		0,		0,		683,	0*/ },		// 1024x768x85
/*22*/	{  0,	1024,768,120,	1184,813,	HP_VN_P,	32+80, 4+38, 	0/*,		0,	0,		0,		0,		0,		0*/ },		// 1024x768x120 REDUCED
                                             
/*23*/	{  1,	1152,864,75,	1600,900,	HP_VP_P,	128+256,3+32, 	0/*,		5,	-6,		0,		0,		0,		0*/ },		// 1152x864x75

/*24*/	{  1,	1280,720,60, 	1664,746,	HP_VN_P,	0+192,0+26, 	0/*,		5,	-7,  	0,		0,		0,		0*/ },		// 1280x720x60		 // 

/*25*/	{  1,	1280,768,60, 	1440,790,	HP_VN_P,	32+80, 7+12, 	0/*,		5,	-8,  	2,		2,		0,		0*/ },		// 1280x768x60	REDUCED
/*26*/	{  1,	1280,768,60, 	1664,798,	HP_VN_P,	128+192,7+20, 	0/*,		5,	-10,  	0,		0,		0,		0*/ },		// 1280x768x60		 // It cannot display !!!!
/*27*/	{  1,	1280,768,60, 	1688,802,	HP_VN_P,	0+232,0+34, 	0/*,		5,	-7,  	0,		0,		0,		0*/ },		// 1280x768x60		 // Windows7 mode
/*28*/	{  1,	1280,768,75, 	1696,805,	HP_VN_P,	128+208,7+27, 	0/*,		5,	-10, 	0,		0,		0,		0*/ },		// 1280x768x75
/*29*/	{  0,	1280,768,85, 	1712,809,	HP_VN_P,	136+216,307+31, 0/*,		0,	0,  	0,		0,		0,		0*/ },		// 1280x768x85
/*30*/	{  0,	1280,768,120, 	1440,813,	HP_VN_P,	32+80, 7+35, 	0/*,		0,	0,  	0,		0,		0,		0*/ },		// 1280x768x120	REDUCED

/*31*/	{  1,	1280,800,60, 	1440,823,	HP_VN_P,	32+80, 6+14, 	0/*,		5,	-8,		0,		0,		0,		0*/ },		// 1280x800x60	REDUCED
/*32*/	{  1,	1280,800,60, 	1680,831,	HP_VN_P,	128+200,6+22, 	0/*,		5,	-7,		0,		0,		0,		0*/ },		// 1280x800x60
/*33*/	{  1,	1280,800,75, 	1696,838,	HP_VN_P,	128+208,6+29, 	0/*,		5,	-7,		0,		0,		0,		0*/ },		// 1280x800x75
/*34*/	{  0,	1280,800,85, 	1712,843,	HP_VN_P,	136+216,6+34, 	0/*,		0,	0,		0,		0,		0,		0*/ },		// 1280x800x85
/*35*/	{  0,	1280,800,120, 	1440,813,	HP_VN_P,	32+80, 6+38, 	0/*,		0,	0,		0,		0,		0,		0*/ },		// 1280x800x120	REDUCED

/*36*/	{  1,	1280,960,60, 	1800,1000,	HN_VN_P,	112+312,3+36, 	0/*,		5,	-7,		0,		0,		0,		0*/ },		// 1280x960x60
/*37*/	{  0,	1280,960,85, 	1728,1011,	HN_VN_P,	160+224,3+47, 	0/*,		0,	0,		0,		0,		0,		0*/ },		// 1280x960x85
/*38*/	{  0,	1280,960,120, 	1440,813,	HN_VP_P,	32+80, 4+50, 	0/*,		0,	0,		0,		0,		0,		0*/ },		// 1280x960x120	REDUCED

/*39*/	{  1,	1280,1024,60, 	1688,1066,	HN_VN_P,	112+248,3+38, 	0/*,		5,	-6,		0,		0,		640,	0*/ },		// 1280x1024x60
/*40*/	{  0,	1280,1024,75, 	1688,1066,	HN_VN_P,	144+248,3+38, 	0/*,		0,	0,		0,		0,		800,	0*/ },		// 1280x1024x75
/*41*/	{  0,	1280,1024,85, 	1728,1072,	HN_VN_P,	160+224,3+44, 	0/*,		0,	0,		0,		0,		911,	0*/ },		// 1280x1024x85

/*42*/	{  1,	1360,768,60, 	1792,795,	HN_VN_P,	112+256,6+18, 	0/*,		5,	-8,		0,		0,		0,		0*/ },		// 1360x768x60
														  
/*43*/	{  1,	1400,1050,60, 	1560,1080,	HP_VN_P,	32+80, 4+23, 	0/*,		5,	-7,		0,		0,		0,		0*/ },		// 1400x1050x60	REDUCED
/*44*/	{  1,	1400,1050,60, 	1864,1089,	HP_VN_P,	144+232,4+32, 	0/*,		5,	-6,		0,		0,		0,		0*/ },		// 1400x1050x60			// It cannot display

/*45*/	{  1,	1440,900,60, 	1600,926,	HP_VN_P,	32+80, 6+17, 	0/*,		5,	-8,		0,		0,		0,		0*/ },		// 1400x900x60	REDUCED
/*46*/	{  1,	1440,900,60, 	1904,934,	HP_VN_P,	152+232,6+25, 	0/*,		5,	-8,		0,		0,		0,		0*/ },		// 1400x900x60
/*47*/	{  0,	1440,900,75, 	1936,942,	HP_VN_P,	152+248,6+33, 	0/*,		0,	0,		0,		0,		0,		0*/ },		// 1400x900x75

/*48*/	{  1,	1680,1050,60, 	1840,1080,	HP_VN_P,	32+80, 6+21, 	0/*,		0,	0,		0,		0,		0,		0*/ },		// 1680x1440x60	REDUCED
/*49*/	{  1,	1920,1080,60,	2200,1124,  HP_VP_P,	44+148,5+36, 	0/*,		30,	10,		0,		0,		0,		0*/ },		// EE_RGB_1080P

#if 0
///============
	//if 0x1CC[0] is 0
// 	   SUPPORT  HAN,VAN,VFREQ  	H&V TOTAL  HSP &VSP	Hst,Vst		OffsetH,V	Dummy0 	Dummy1 	Dummy2 	Dummy3
/*49*/	{  3,	720,240+1,60,	858,262,	0x00,	112,17/*,		0,	0,		0,		0,		0,		0*/ },		// 34: EE_RGB_480I        	 0x359,0x359,   0x359,   120, 10,  123, 324, 21
/*50*/	{  3,	720,288,50,		864,312,	0x00,	109,22/*,		0,	0,		0,		0,		0,		0*/ },		// 35: EE_RGB_576I        	 0x35f,0x35f,   0x35f,   134, 10,  131, 324, 16
/*51*/	{  3,	720,480,60,		858,525,	0x00,	112,41/*,		0,	0,		0,		0,		0,		0*/ },		// 28: EE_RGB_480P        	 0x359,0x359,   0x359,   123, 28,  251, 378, 17
/*52*/	{  3,	720,576,50,		864,625,    0x00,	107,48/*,		0,	0,		0,		0,		0,		0*/ },		// 36: EE_RGB_576P			 0x35f,0x35f,   0x35f,   133, 36,  250, 324, 21
/*53*/	{  3,	1920,540,60,	2200,562,   0x00,	217,24/*,		0,	0,		0,		0,		0,		0*/ },		// 31: EE_RGB_1080I       	 0x897,0x897,   0x897,   300, 42,  740, 324, 32
/*54*/	{  3,	1280,720,60,	1650,750,   0x00,	277,30/*,		0,	0,		0,		0,		0,		0*/ },		// 29: EE_RGB_720P        	 0x671,0x671,   0x671,   282, 12,  742, 410, 3 
/*55*/	{  3,	1920,1080,60,	2200,1125,  0x00,	217,48/*,		0,	0,		0,		0,		0,		0*/ },		// EE_RGB_1080P
//	{  3,	1280,720,60,	1980,	    },		// 30: EE_RGB_720P50 ???  	 0x7bb,0x7bb,   0x7bb,   346, 25,  742, 378, 45
//	{  0,	1920,540-40,50,	2640,	    },		// 32: EE_RGB_1080I50A    	 0xa4f,0xa4f,   0xa4f,   300, 41,  740, 324, 32
//	{  0,	1920,540-40,60,	2200,	    },		// 33: EE_RGB_1080I50B ???	 0x897,0x897,   0x897,   300, 42,  740, 324, 32
/*54*/	{  3,	1280,720,50,	1000,100,   0x00,	277,30/*,		0,	0,		0,		0,		0,		0*/ },		// 720P@50
/*53*/	{  3,	1920,526,50,	1000,100,   0x00,	356,87/*,		0,	0,		0,		0,		0,		0*/ },		// 1080i@50
/*55*/	{  3,	1920,1080,50,	1000,100,   0x00,	217,48/*,		0,	0,		0,		0,		0,		0*/ },		// 1080P@50
//--------------
#endif
};
#endif  //.. defined( SUPPORT_PC ) || defined (SUPPORT_DVI)

//VEST TABLE
//===========

struct VIDEO_TIME_TABLE_s {
	BYTE id;		//DMT id. if bit7 is high, we donot support.
	WORD hActive;	//horizontal resoultion
	WORD vActive;	//vertical resoultion
	BYTE vFreq;		//vertical frequency

	BYTE pol;		//H & V Sync polarity.

	WORD hTotal;	//horizontal period
	BYTE hFPorch;	//horizontal Front Porch
	BYTE hSync;		//horizontal Synch width
	WORD hBPorch;	//horizontal Back Porch

	WORD vTotal;	//vertical period
	BYTE vFPorch;	//vertical Front Porch
	BYTE vSync;		//vertical Synch Width
	BYTE vBPorch;	//vertical Back Porch

	//hFreq:kHz
	//PixelFreq:MHz
	BYTE hOverScan;
	BYTE vOverScan;
};


#if 0
code struct VIDEO_TIME_TABLE_s MY_VESA_TABLE[] = {									/* HFreq:kHz Pixel:MHz */

/*   */ {  0x80, 720,400,70,    HN_VP_P,	900,18,108,53,		449,13,2,34 },			/*31.472	 28.325*/

/* 02*/	{  0x81, 640,350,85,	HP_VN_P,	832,23,64,96,   	445,32,3,60 }, 
    
/* 03*/	{  0x82, 640,400,85,	HN_VP_P,	832,32,64,96,		445,1,3,41 },
/* 04*/	{  0x83, 720,400,85,	HN_VP_P,	936,36,72,108,		446,1,3,42 },

////	{  0x04,640,480,60,		HN_VN_P,	800,(8+8),96,(40/*+8*/),	525,(8+2),2,(25/*+8*/) /*31.5 25.175*/},
/*>05*/	{  0x04,640,480,60,		HN_VN_P,	800,8+8,96,40,		525,8+2,2,25,	0,-1 /*31.5 25.175*/},

/* 06*/	{  0x05,640,480,72,		HN_VN_P,	832,8+24,40,120,	520,(8+1),3,(20+8) }, 
/* 07*/	{  0x06,640,480,75,		HN_VN_P,	840,16,64,120,		500,1,3,16 },
/* 08*/	{  0x87,640,480,85,		HN_VN_P,	832,56,56,80,		509,1,3,25 }, 

/* 09*/	{  0x08,800,600,56,		HP_VP_P,	1024,24,72,128,		625,1,2,22 },
/*>10*/	{  0x09,800,600,60,		HP_VP_P,	1056,40,128,88,		628,1,4,23 },
/* 11*/	{0x50+0,800,600,70,		HP_VN_P,	1040,49,80,111,		625,2,3,20 },	//...@PC
/* 12*/	{  0x0A,800,600,72,		HP_VP_P,	1040,56,120,64,		666,37,6,23},
/* 13*/	{  0x0B,800,600,75,		HP_VP_P,	1056,16,80,160,		625,1,3,21 },
/* 14*/	{  0x8C,800,600,85,		HP_VP_P, 	1048,32,64,152,		631,1,3,27 },
/* 15*/	{  0x8D,800,600,120,	HP_VN_P,	960,48,32,80,		636,3,4,29 }, //REDUCED Blanking

/* 16*/	{  0x0E,848,480,60,		HP_VP_P,	1088,16,112,112,	517,6,8,23 },
/* 17*/	{0x50+1,960,600,60,		HN_VP_P,	1232,UNKNOWN_0,UNKNOWN_0,120,	622,UNKNOWN_0,UNKNOWN_0	,23},	//unknown.... 

//		{  0x0F,1024,768,43,  HP_VP_P..
/*>18*/	{  0x10,1024,768,60,	HN_VN_P,	1344,24,136,160,	806,3,6,29 },	//292(16) 36(6)
/*19*/	{  0x11,1024,768,70,	HN_VN_P,	1328,24,136,144,	806,3,6,29 },
/*20*/	{  0x12,1024,768,75,	HP_VP_P,	1312,16,96,176,		800,1,3,28 },	
/*21*/	{  0x93,1024,768,85,	HP_VP_P,	1376,48,96,208,		808,1,3,36 },	
/*22*/	{  0x94,1024,768,120,	HP_VN_P,	1184,48,32,80,		813,3,4,38 },	
                                             
/*23*/	{  0x15,1152,864,75,	HP_VP_P,	1600,64,128,256,	900,1,3,32 },

/*24*/	{0x51+2,1280,720,60, 	HP_VP_P,	1650,110,40,220,	750,5,5,20},		// 720P

/*25*/	{  0x16,1280,768,60, 	HP_VN_P,	1440,48,32,80,		790,3,7,12},	
/*26*/	{  0x17,1280,768,60, 	HN_VP_P,	1664,64,128,192,	798,3,7,20},	
/*27*/	{  0x80,1280,768,60, 	HN_VP_P,	1688,UNKNOWN_0,UNKNOWN_0,232,	802,UNKNOWN_0,UNKNOWN_0,34},	
/*28*/	{  0x18,1280,768,75, 	HN_VP_P,	1696,80,128,208,	805,3,7,27},	
/*29*/	{  0x99,1280,768,85, 	HN_VP_P,	1712,80,136,216,	809,307,31 },	
/*30*/	{  0x9A,1280,768,120, 	HP_VN_P,	1440,48,32,80,		813,3,7,35 },	

/*31*/	{  0x1B,1280,800,60, 	HP_VN_P,	1440,48,32,80,		823,3,6,14},	
/*32*/	{  0x1C,1280,800,60, 	HN_VP_P,	1680,72,128,200,	831,3,6,22},	
/*33*/	{  0x1D,1280,800,75, 	HN_VP_P,	1696,80,128,208,	838,3,6,29 },	
/*34*/	{  0x9E,1280,800,85, 	HN_VP_P,	1712,80,136,216,	843,3,6,34 },	
/*35*/	{  0x9F,1280,800,120, 	HP_VN_P,	1440,48,32,80,		813,3,6,38 },	

/*36*/	{  0x20,1280,960,60, 	HP_VP_P,	1800,96,112,312,	1000,1,3,36 },
/*37*/	{  0xA1,1280,960,85, 	HP_VP_P,	1728,64,160,224,	1011,1,3,47 },		
/*38*/	{  0xA2,1280,960,120, 	HP_VN_P,	1440,48,32,80,		1017,3,4,50 },		

/*39*/	{  0x23,1280,1024,60, 	HP_VP_P,	1688,48,112,248,	1066,1,3,38 },		
/*40*/	{  0xA4,1280,1024,75, 	HP_VP_P,	1688,16,144,248,	1066,1,3,38 },		
/*41*/	{  0xA5,1280,1024,85, 	HP_VP_P,	1728,64,160,224,	1072,1,3,44	},		
//			0x26,1280,1024,120, HP_VN_P,	1440,48,32,80,	1084,3,7,50};

/*42*/	{  0x27,1360,768,60, 	HP_VP_P,	1792,64,112,256,	795,3,6,18},		
//			0x28,1360,768,120	HP_VN_P,	1520,48,32,80,		813,3,5,37},
														  
/*43*/	{  0x29,1400,1050,60, 	HP_VN_P,	1560,48,32,80,		1080,3,4,23	 },		
/*44*/	{  0x2A,1400,1050,60, 	HN_VP_P,	1864,88,144,232,	1089,3,4,32	 },		
//			0x2B,1400,1050,75,	HN_VP_P,	1896,104,144,248,	1099,3,4,42 },
//			0x2C,1400,1050,85
//			0x2D,1400,1050,120
/*45*/	{  0x2E,1440,900,60, 	HP_VN_P,	1600,48,32,80,		926,3,6,17	 },		
/*46*/	{  0x2F,1440,900,60, 	HN_VP_P,	1904,80,152,232,	934,3,6,25 },		
/*47*/	{  0xB0,1440,900,75, 	HN_VP_P,	1936,96,152,248,	942,3,6,33 },		
//				0x31,1440,900,85	HN_VP_P
//				0x32,1440,900,120,	0x01
//				0x33,1600,1200,60
//				0x34,1600,1200,65
//				0x35,1600,1200,70
//				0x36,1600,1200,75
//				0x37,1600,1200,85
//				0x38,1600,1200,120
/*48*/	{  0x39,1680,1050,60,	HP_VN_P, 	1840,48,32,80,		1080,3,6,21	 },		// 1680x1440x60	REDUCED
//				0x3A,1680,1050,60
//				0x3B,1680,1050,75
//				0x3C,1680,1050,85
//				0x3D,1680,1050,120
//				0x3E,1792,1344,60
//				0x3F,1792,1344,75,
//				0x40,1792,1344,120
//				0x41,1856,1392,60
//				0x42,1856,1392,75,
//				0x43,1856,1392,120
//				0x44,1920,1200,60
//				0x45,1920,1200,60
//				0x46,1920,1200,75
//				0x47,1920,1200,85
//				0x48,1920,1200,120
//				0x49,1920,1440,60
//				0x4A,1920,1440,75
//				0x4B,1920,1440,120
//				0x4C,2560,1600,60
//				0x4D,2560,1600,60
//				0x4E,2560,1600,75
//				0x4F,2560,1600,85
//				0x50,2560,1600,120
/*49*/	{0x50+3,1920,1080,60,	HP_VP_P,	2200,88,44,148, 	1124,4,5,36 },		// 1080P
};

#endif


#if 0
code struct VIDEO_TIME_TABLE_s MY_EIA_TABLE[] = {									/* HFreq:kHz Pixel:MHz */
{1,		640,480,50,		HN_VN_P,	800,16,96,48,		525,10,2,33},
{2,		720,480,60,		HN_VN_P,	858,16,62,60+44,	525,9,6,30,		10,10},		//480p
{3,		720,480,60,		HN_VN_P,	858,16,62,60,		525,9,6,30},
{4,		1280,720,60,	HP_VP_P,	1650,110,40,220+54,	750,5,5,20,		10,5},		//720p@60
{5,		1920,540,60,	HP_VP_I,	2200,88,44,148+52,	562,2,5,15,		20,10}, 	//1080i@60
{6,		1440,480,60,	HN_VN_I,	1716,38,124,114,	262,4,3,15},				//480i
{7,		720,240,60,		HN_VN_I,	858,19,62,57+48,	262,4,3,15+1,	10,5},		//480i for FW
{8,		1440,240,60,	HN_VN_P,	1716,38,124,114,	262,4,3,15},			
{9,		1440,240,60,	HN_VN_P,	1716,38,124,114,	262,4,3,15},	
{10,	2880,240,60,	HN_VN_I,	3432,76,248,228,	262,4,3,15},	
{11,	2880,240,60,	HN_VN_I,	3432,76,248,228,	262,4,3,15},	
{12,	2880,240,60,	HN_VN_P,	3432,6,248,228,		262,4,3,15},	
{13,	2880,240,60,	HN_VN_P,	3432,6,248,228,		262,4,3,15},	
{14,	1440,480,60,	HN_VN_P,	1716,32,276,120,	525,9,6,30},
{15,	1440,480,60,	HN_VN_P,	1716,32,276,120,	525,9,6,30},
{16,	1920,1080,60,	HP_VP_P,	2200,88,44,148+50,	1125,4,5,36,	20,20},		//1080p@60
{17,	720,576,50,		HN_VN_P,	864,12,64,68,		625,5,5,39},
{18,	720,576,50,		HN_VN_P,	864,12,64,68+47,	625,5,5,39,		10,10},		//576p
{19, 	1280,720,50,	HP_VP_P,	1980,440,40,220,	750,5,5,20,		10,12},		//720p@50
{20,	1920,1080,50,	HP_VP_I,	2640,528,44,148,	562,2,5,15,		20,10},		//1080i@50
{21,	1440,288,50,	HN_VN_I,	1728,24,126,138,	312,2,3,19}, 				//576i
{22,	720,288,50,		HN_VN_I,	864,12,63,69+48,	312,2,3,19,		10,5},		//576i for FW 
{23,	1440,288,50,	HN_VN_P,	1728,24,126,138,	313,3,3,19}, 
{24,	1440,288,50,	HN_VN_P,	1728,24,126,138,	313,3,3,19}, 
{25,	2880,288,50,	HN_VN_I,	3456,48,252,276,	312,2,3,19},
{26,	2880,288,50,	HN_VN_I,	3456,48,252,276,	312,2,3,19},
{27,	2880,288,50,	HN_VN_P,	3456,48,252,276,	313,3,3,19},
{28,	2880,288,50,	HN_VN_P,	3456,48,252,276,	313,3,3,19},
{29,	1440,576,50,	HN_VP_P,	1728,24,128,136,	625,5,5,39},
{30,	1440,576,50,	HN_VP_P,	1728,24,128,136,	625,5,5,39},
{31,	1920,1080,50,	HP_VP_P,	2640,528,44,148,	1125,4,5,36,	20,20},	//1080p@50
{32,	1920,1080,24,	HP_VP_P,	2750,638,44,148,	1125,4,5,36},
{33,	1920,1080,25,	HP_VP_P,	2640,528,44,148,	1125,4,5,36},
{34,	1920,1080,30,	HP_VP_P,	2640,528,44,148,	1125,4,5,36},
};
BYTE ConvertCompIdToEiaId(BYTE mode)
{
	BYTE index;
	switch(mode) {
	case 0:	index=6;	break;	//480i
	case 1:	index=21;	break;	//576i
	case 2:	index=1;	break;	//480p
	case 3:	index=17;	break;	//576p
	case 4:	index=19;	break;	//1080i50
	case 5:	index=4;	break;	//1080i60
	case 6:	index=18;	break;	//720p50
	case 7:	index=3;	break;	//720p60
	case 8:	index=19;	break;	//1080p50
	case 9:	index=15;	break;	//1080p60
	default: index=0;	break;	//giveup..
	}
	return index;
}
#endif

struct COMP_VIDEO_TIME_TABLE_s {
	BYTE id;		//DMT id. if bit7 is high, we donot support.
	WORD hActive;	//horizontal resoultion
	WORD vActive;	//vertical resoultion
	BYTE vFreq;		//vertical frequency

	BYTE pol;		//H & V Sync polarity + interlaced flag.

	WORD hTotal;	//horizontal period
	WORD hStart;	//horizontal Sync width + Back Porch.

	WORD vTotal;	//vertical period
	BYTE vFPorch;	//vertical Front Porch
	BYTE vSync;		//vertical Synch Width
	BYTE vBPorch;	//vertical Back Porch

	BYTE ClampPos;	//Clamp position value for REG1D7.

	BYTE hOverScan;	//overscan pixels
	BYTE vOverScan;	//overscan lines
};

code struct COMP_VIDEO_TIME_TABLE_s TW8836_COMP_TABLE[] = {
/*COMP_MODE_480I		0*/ {7,		720,240,60,		HN_VN_I,	858, 57+48,		262,4,3,15+1,	128,	10,5},		//480i for FW
/*COMP_MODE_576I		1*/ {22,	720,288,50,		HN_VN_I,	864, 69+48,		312,2,3,19,		128,	10,5},		//576i for FW 
/*COMP_MODE_480P		2*/	{2,		720,480,60,		HN_VN_P,	858, 60+44,		525,9,6,30,		64,		10,10},		//480p
/*COMP_MODE_576P		3*/ {18,	720,576,50,		HN_VN_P,	864, 68+47,		625,5,5,39,		58,		10,10},		//576p
/*COMP_MODE_1080I25		4*/	{20,	1920,1080,50,	HP_VP_I,	2640,148+52,	562,2,5,15,		40,		20,10},		//1080i@50. need asjust value.
/*COMP_MODE_1080I30		5*/	{5,		1920,540,60,	HP_VP_I,	2200,148+52,	562,2,5,15,		32,		20,10}, 	//1080i@60
/*COMP_MODE_720P50		6*/ {19, 	1280,720,50,	HP_VP_P,	1980,220+54,	750,5,5,20,		38,		10,12},		//720p@50. need asjust value.
/*COMP_MODE_720P60		7*/	{4,		1280,720,60,	HP_VP_P,	1650,220+54,	750,5,5,20,		38,		10,5},		//720p@60
/*COMP_MODE_1080P50		8*/ {31,	1920,1080,50,	HP_VP_P,	2640,148+50,	1125,4,5,36,	14,		20,20},		//1080p@50. need asjust value.
/*COMP_MODE_1080P60		9*/	{16,	1920,1080,60,	HP_VP_P,	2200,148+50,	1125,4,5,36,	14,		20,20},		//1080p@60
};

/*
LINUX TABLE
# 640x400 @ 85Hz (VESA) hsync: 37.9kHz
ModeLine "640x400"    31.5    640  672  736  832  400  401  404  445 -hsync +vsync
# 720x400 @ 85Hz (VESA) hsync: 37.9kHz
ModeLine "720x400"    35.5    720  756  828  936  400  401  404  446 -hsync +vsync

# 640x480 @ 60Hz (Industry standard) hsync: 31.5kHz
ModeLine "640x480"    25.2    640  656  752  800  480  490  492  525 -hsync -vsync
									16  112             10   12
# 640x480 @ 100Hz hsync: 50.9kHz
Modeline "640x480"    43.163  640  680  744  848  480  481  484  509 +hsync +vsync
# 640x480 @ 72Hz (VESA) hsync: 37.9kHz
ModeLine "640x480"    31.5    640  664  704  832  480  489  491  520 -hsync -vsync
# 640x480 @ 75Hz (VESA) hsync: 37.5kHz
ModeLine "640x480"    31.5    640  656  720  840  480  481  484  500 -hsync -vsync
# 640x480 @ 85Hz (VESA) hsync: 43.3kHz
ModeLine "640x480"    36.0    640  696  752  832  480  481  484  509 -hsync -vsync
# 768x576 @ 60 Hz (GTF) hsync: 35.82 kHz; pclk: 34.96 MHz
ModeLine "768x576"    34.96   768  792  872  976  576  577  580  597 -hsync +vsync
# 768x576 @ 72 Hz (GTF) hsync: 43.27 kHz; pclk: 42.93 MHz
ModeLine "768x576"    42.93   768  800  880  992  576  577  580  601 -hsync +vsync
# 768x576 @ 75 Hz (GTF) hsync: 45.15 kHz; pclk: 45.51 MHz
ModeLine "768x576"    45.51   768  808  888  1008 576  577  580  602 -hsync +vsync
# 768x576 @ 85 Hz (GTF) hsync: 51.42 kHz; pclk: 51.84 MHz
ModeLine "768x576"    51.84   768  808  888  1008 576  577  580  605 -hsync +vsync
# 768x576 @ 100 Hz (GTF) hsync: 61.10 kHz; pclk: 62.57 MHz
ModeLine "768x576"    62.57   768  816  896  1024  576 577  580  611 -hsync +vsync
# 800x600 @ 56Hz (VESA) hsync: 35.2kHz
ModeLine "800x600"    36.0    800  824  896 1024  600  601  603  625 +hsync +vsync
# 800x600 @ 60Hz (VESA) hsync: 37.9kHz
ModeLine "800x600"    40.0    800  840  968 1056  600  601  605  628 +hsync +vsync
# 800x600 @ 72Hz (VESA) hsync: 48.1kHz
ModeLine "800x600"    50.0    800  856  976 1040  600  637  643  666 +hsync +vsync
# 800x600 @ 75Hz (VESA) hsync: 46.9kHz
ModeLine "800x600"    49.5    800  816  896 1056  600  601  604  625 +hsync +vsync
# 800x600 @ 85Hz (VESA) hsync: 53.7kHz
ModeLine "800x600"    56.3    800  832  896 1048  600  601  604  631 +hsync +vsync
# 800x600 @ 100Hz hsync: 63.6kHz
Modeline "800x600"    68.179  800  848  936 1072  600  601  604  636 +hsync +vsync
# 1024x600 @ 60 Hz (GTF) hsync: 37.32 kHz; pclk: 48.96 MHz
ModeLine "1024x600"   48.96   1024 1064 1168 1312  600 601  604  622 -hsync +vsync
# 1024x768i @ 43Hz (industry standard) hsync: 35.5kHz
ModeLine "1024x768"   44.9   1024 1032 1208 1264  768  768  776  817 +hsync +vsync Interlace
# 1024x768 @ 60Hz (VESA) hsync: 48.4kHz
ModeLine "1024x768"   65.0   1024 1048 1184 1344  768  771  777  806 -hsync -vsync
# 1024x768 @ 70Hz (VESA) hsync: 56.5kHz
ModeLine "1024x768"   75.0   1024 1048 1184 1328  768  771  777  806 -hsync -vsync
# 1024x768 @ 75Hz (VESA) hsync: 60.0kHz
ModeLine "1024x768"   78.8   1024 1040 1136 1312  768  769  772  800 +hsync +vsync
# 1024x768 @ 85Hz (VESA) hsync: 68.7kHz
ModeLine "1024x768"   94.5   1024 1072 1168 1376  768  769  772  808 +hsync +vsync
# 1024x768 @ 100Hz hsync: 81.4kHz
Modeline "1024x768" 113.309  1024 1096 1208 1392  768  769  772  814 +hsync +vsync
# 1024x768 @ 120Hz hsync: 98.8kHz
Modeline "1024x768" 139.054  1024 1104 1216 1408  768  769  772  823 +hsync +vsync
# 1152x864 @ 60Hz hsync: 53.7kHz
Modeline "1152x864"  81.642  1152 1216 1336 1520  864  865  868  895 +hsync +vsync
# 1152x864 @ 75Hz (VESA) hsync: 67.5kHz
ModeLine "1152x864" 108.0    1152 1216 1344 1600  864  865  868  900 +hsync +vsync
# 1152x864 @ 85Hz hsync: 77kHz
Modeline "1152x864" 119.651  1152 1224 1352 1552  864  865  868  907 +hsync +vsync
# 1152x864 @ 100Hz hsync: 91.5kHz
Modeline "1152x864" 143.472  1152 1232 1360 1568  864  865  868  915 +hsync +vsync
# 1280x768 @ 60 Hz (GTF) hsync: 47.70 kHz; pclk: 80.14 MHz
ModeLine "1280x768"  80.14   1280 1344 1480 1680  768  769  772  795 -hsync +vsync
# 1280x800 @ 60 Hz (GTF) hsync: 49.68 kHz; pclk: 83.46 MHz
ModeLine "1280x800"  83.46   1280 1344 1480 1680  800  801  804  828 -hsync +vsync
# 1280x960 @ 60Hz (VESA) hsync: 60.0kHz
ModeLine "1280x960"  108.0   1280 1376 1488 1800  960  961  964 1000 +hsync +vsync
# 1280x960 @ 75Hz hsync: 75.1kHz
Modeline "1280x960" 129.859  1280 1368 1504 1728  960  961  964 1002 +hsync +vsync
# 1280x960 @ 85Hz (VESA) hsync: 85.9kHz
ModeLine "1280x960"  148.5   1280 1344 1504 1728  960  961  964 1011 +hsync +vsync
# 1280x960 @ 100Hz hsync: 101.7kHz
Modeline "1280x960" 178.992  1280 1376 1520 1760  960  961  964 1017 +hsync +vsync
# 1280x1024 @ 60Hz (VESA) hsync: 64.0kHz
ModeLine "1280x1024" 108.0   1280 1328 1440 1688 1024 1025 1028 1066 +hsync +vsync
# 1280x1024 @ 75Hz (VESA) hsync: 80.0kHz
ModeLine "1280x1024" 135.0   1280 1296 1440 1688 1024 1025 1028 1066 +hsync +vsync
# 1280x1024 @ 85Hz (VESA) hsync: 91.1kHz
ModeLine "1280x1024" 157.5   1280 1344 1504 1728 1024 1025 1028 1072 +hsync +vsync
# 1280x1024 @ 100Hz hsync: 108.5kHz
Modeline "1280x1024" 190.96  1280 1376 1520 1760 1024 1025 1028 1085 +hsync +vsync
# 1280x1024 @ 120Hz hsync: 131.6kHz
Modeline "1280x1024" 233.793 1280 1384 1528 1776 1024 1025 1028 1097 +hsync +vsync
# 1368x768 @ 60 Hz (GTF) hsync: 47.70 kHz; pclk: 85.86 MHz
ModeLine "1368x768"  85.86   1368 1440 1584 1800 768  769  772  795  -hsync +vsync
# 1400x1050 @ 60Hz M9 Laptop mode, hsync: 65kHz
ModeLine "1400x1050" 122.00  1400 1488 1640 1880 1050 1052 1064 1082 +hsync +vsync
# 1400x1050 @ 72 Hz (GTF) hsync: 78.77 kHz; pclk: 149.34 MHz
ModeLine "1400x1050" 149.34  1400 1496 1648 1896 1050 1051 1054 1094 -hsync +vsync
# 1400x1050 @ 75 Hz (GTF) hsync: 82.20 kHz; pclk: 155.85 MHz
ModeLine "1400x1050" 155.85  1400 1496 1648 1896 1050 1051 1054 1096 -hsync +vsync
# 1400x1050 @ 85 Hz (GTF) hsync: 93.76 kHz; pclk: 179.26 MHz
ModeLine "1400x1050" 179.26  1400 1504 1656 1912 1050 1051 1054 1103 -hsync +vsync
# 1400x1050 @ 100 Hz (GTF) hsync: 111.20 kHz; pclk: 214.39 MHz
ModeLine "1400x1050" 214.39  1400 1512 1664 1928 1050 1051 1054 1112 -hsync +vsync
# 1440x900 @ 60 Hz  (GTF) hsync: 55.92 kHz; pclk: 106.47 MHz
ModeLine "1440x900"  106.47  1440 1520 1672 1904 900  901  904  932  -hsync +vsync
# 1440x1050 @ 60 Hz (GTF) hsync: 65.22 kHz; pclk: 126.27 MHz
ModeLine "1440x1050" 126.27  1440 1536 1688 1936 1050 1051 1054 1087 -hsync +vsync
# 1600x1000 @ 60Hz hsync: 62.1kHz
Modeline "1600x1000" 133.142 1600 1704 1872 2144 1000 1001 1004 1035 +hsync +vsync
# 1600x1000 @ 75Hz hsync: 78.3kHz
Modeline "1600x1000" 169.128 1600 1704 1880 2160 1000 1001 1004 1044 +hsync +vsync
# 1600x1000 @ 85Hz hsync: 89.2kHz
Modeline "1600x1000" 194.202 1600 1712 1888 2176 1000 1001 1004 1050 +hsync +vsync
# 1600x1000 @ 100Hz hsync: 105.9kHz
Modeline "1600x1000" 232.133 1600 1720 1896 2192 1000 1001 1004 1059 +hsync +vsync
# 1600x1024 @ 60Hz hsync: 63.6kHz
Modeline "1600x1024" 136.385 1600 1704 1872 2144 1024 1027 1030 1060 +hsync +vsync
# 1600x1024 @ 75Hz hsync: 80.2kHz
Modeline "1600x1024" 174.416 1600 1712 1888 2176 1024 1025 1028 1069 +hsync +vsync
# 1600x1024 @ 76Hz hsync: 81.3kHz
Modeline "1600x1024" 170.450 1600 1632 1792 2096 1024 1027 1030 1070 +hsync +vsync
# 1600x1024 @ 85Hz hsync: 91.4kHz
Modeline "1600x1024" 198.832 1600 1712 1888 2176 1024 1027 1030 1075 +hsync +vsync
# 1600x1200 @ 60Hz (VESA) hsync: 75.0kHz
ModeLine "1600x1200" 162.0   1600 1664 1856 2160 1200 1201 1204 1250 +hsync +vsync
# 1600x1200 @ 65Hz (VESA) hsync: 81.3kHz
ModeLine "1600x1200" 175.5   1600 1664 1856 2160 1200 1201 1204 1250 +hsync +vsync
# 1600x1200 @ 70Hz (VESA) hsync: 87.5kHz
ModeLine "1600x1200" 189.0   1600 1664 1856 2160 1200 1201 1204 1250 +hsync +vsync
# 1600x1200 @ 75Hz (VESA) hsync: 93.8kHz
ModeLine "1600x1200" 202.5   1600 1664 1856 2160 1200 1201 1204 1250 +hsync +vsync
# 1600x1200 @ 85Hz (VESA) hsync: 106.3kHz
ModeLine "1600x1200" 229.5   1600 1664 1856 2160 1200 1201 1204 1250 +hsync +vsync
# 1600x1200 @ 100 Hz (GTF) hsync: 127.10 kHz; pclk: 280.64 MHz
ModeLine "1600x1200" 280.64  1600 1728 1904 2208 1200 1201 1204 1271 -hsync +vsync
# 1680x1050 @ 60 Hz  (GTF) hsync: 65.22 kHz; pclk: 147.14 MHz
ModeLine "1680x1050" 147.14  1680 1784 1968 2256 1050 1051 1054 1087 -hsync +vsync
# 1792x1344 @ 60Hz (VESA) hsync: 83.6kHz
ModeLine "1792x1344" 204.8   1792 1920 2120 2448 1344 1345 1348 1394 -hsync +vsync
# 1792x1344 @ 75Hz (VESA) hsync: 106.3kHz
ModeLine "1792x1344" 261.0   1792 1888 2104 2456 1344 1345 1348 1417 -hsync +vsync
# 1800x1440 (unchecked)
ModeLine "1800x1440" 230     1800 1896 2088 2392 1440 1441 1444 1490 +hsync +vsync
# 1856x1392 @ 60Hz (VESA) hsync: 86.3kHz
ModeLine "1856x1392" 218.3   1856 1952 2176 2528 1392 1393 1396 1439 -hsync +vsync
# 1856x1392 @ 75Hz (VESA) hsync: 112.5kHz
ModeLine "1856x1392" 288.0   1856 1984 2208 2560 1392 1393 1396 1500 -hsync +vsync
# 1920x1080 @ 60Hz hsync: 67.1kHz
Modeline "1920x1080" 172.798 1920 2040 2248 2576 1080 1081 1084 1118 -hsync -vsync
# 1920x1080 @ 75Hz hsync: 81.1kHz
Modeline "1920x1080" 211.436 1920 2056 2264 2608 1080 1081 1084 1126 +hsync +vsync
# 1920x1200 @ 60Hz hsync: 74.5kHz
Modeline "1920x1200" 193.156 1920 2048 2256 2592 1200 1201 1203 1242 +hsync +vsync
# 1920x1200 @ 75Hz hsync: 94kHz
Modeline "1920x1200" 246.590 1920 2064 2272 2624 1200 1201 1203 1253 +hsync +vsync
# 1920x1440 @ 60Hz (VESA) hsync: 90.0kHz
ModeLine "1920x1440" 234.0   1920 2048 2256 2600 1440 1441 1444 1500 -hsync +vsync
# 1920x1440 @ 75Hz (VESA) hsync: 112.5kHz
ModeLine "1920x1440" 297.0   1920 2064 2288 2640 1440 1441 1444 1500 -hsync +vsync
# 1920x2400 @ 25Hz for IBM T221, VS VP2290 and compatible, hsync: 60.8kHz
Modeline "1920x2400" 124.62  1920 1928 1980 2048 2400 2401 2403 2434 +hsync +vsync
# 1920x2400 @ 30Hz for IBM T221, VS VP2290 and compatible, hsync: 73.0kHz
Modeline "1920x2400" 149.25  1920 1928 1982 2044 2400 2402 2404 2434 +hsync +vsync
# 2048x1536 @ 60Hz hsync: 95.3kHz
Modeline "2048x1536" 266.952 2048 2200 2424 2800 1536 1537 1540 1589 +hsync +vsync
*/


#if 0
///============
	//if 0x1CC[0] is 0
// 	   SUPPORT  HAN,VAN,VFREQ  	H&V TOTAL  HSP &VSP	Hst,Vst		OffsetH,V	Dummy0 	Dummy1 	Dummy2 	Dummy3
/*49*/	{  3,	720,240+1,60,	858,262,	0x00,	112,17/*,		0,	0,		0,		0,		0,		0*/ },		// 34: EE_RGB_480I        	 0x359,0x359,   0x359,   120, 10,  123, 324, 21
/*50*/	{  3,	720,288,50,		864,312,	0x00,	109,22/*,		0,	0,		0,		0,		0,		0*/ },		// 35: EE_RGB_576I        	 0x35f,0x35f,   0x35f,   134, 10,  131, 324, 16
/*51*/	{  3,	720,480,60,		858,525,	0x00,	112,41/*,		0,	0,		0,		0,		0,		0*/ },		// 28: EE_RGB_480P        	 0x359,0x359,   0x359,   123, 28,  251, 378, 17
/*52*/	{  3,	720,576,50,		864,625,    0x00,	107,48/*,		0,	0,		0,		0,		0,		0*/ },		// 36: EE_RGB_576P			 0x35f,0x35f,   0x35f,   133, 36,  250, 324, 21
/*53*/	{  3,	1920,540,60,	2200,562,   0x00,	217,24/*,		0,	0,		0,		0,		0,		0*/ },		// 31: EE_RGB_1080I       	 0x897,0x897,   0x897,   300, 42,  740, 324, 32
/*54*/	{  3,	1280,720,60,	1650,750,   0x00,	277,30/*,		0,	0,		0,		0,		0,		0*/ },		// 29: EE_RGB_720P        	 0x671,0x671,   0x671,   282, 12,  742, 410, 3 
/*55*/	{  3,	1920,1080,60,	2200,1125,  0x00,	217,48/*,		0,	0,		0,		0,		0,		0*/ },		// EE_RGB_1080P
//	{  3,	1280,720,60,	1980,	    },		// 30: EE_RGB_720P50 ???  	 0x7bb,0x7bb,   0x7bb,   346, 25,  742, 378, 45
//	{  0,	1920,540-40,50,	2640,	    },		// 32: EE_RGB_1080I50A    	 0xa4f,0xa4f,   0xa4f,   300, 41,  740, 324, 32
//	{  0,	1920,540-40,60,	2200,	    },		// 33: EE_RGB_1080I50B ???	 0x897,0x897,   0x897,   300, 42,  740, 324, 32
/*54*/	{  3,	1280,720,50,	1000,100,   0x00,	277,30/*,		0,	0,		0,		0,		0,		0*/ },		// 720P@50
/*53*/	{  3,	1920,526,50,	1000,100,   0x00,	356,87/*,		0,	0,		0,		0,		0,		0*/ },		// 1080i@50
/*55*/	{  3,	1920,1080,50,	1000,100,   0x00,	217,48/*,		0,	0,		0,		0,		0,		0*/ },		// 1080P@50
//--------------
#endif
//};








//enum EE_PC_OFFSET {
//	EE_PC_UNKNOWN=0,
//	EE_PC_DOS,	
//	EE_PC_640x350_85,
//};
#if defined(SUPPORT_PC)
/**
* get sizeof PCMDATA table.(PC Mode Data Table)
*/
DWORD sizeof_PCMDATA(void)
{
	return sizeof(PCMDATA);
}
#endif
#if 0
DWORD sizeof_DVIMDATA(void)
{
	return sizeof(DVIMDATA);
}
#endif


//---------------------------------------
//R1C2[5:4] - VCO range
//				0 = 5  ~ 27MHz
//				1 = 10 ~ 54MHz
//				2 = 20 ~ 108MHz
//				3 = 40 ~ 216MHz
//R1C2[2:0] - Charge pump
//				0 = 1.5uA
//				1 = 2.5uA
//				2 = 5.0uA
//				3 = 10uA
//				4 = 20uA
//				5 = 40uA
//				6 = 80uA
//				7 = 160uA
//----------------------------------------

#ifdef UNCALLED_SEGMENT
//-----------------------------------------------------------------------------
//parmeter
//	_IPF: Input PixelClock Frequency= Htotal*Vtotal*Hz
//
//
//BYTE SetVCORange(DWORD _IPF)
//
//need pixel clock & POST divider value.
//
//pixel clock = Htotal * Vtotal * Freq
//HFreq = Vtotal * Freq
//pixel clock = Htotal * HFreq
//
//-----------------------------------------------------------------------------
//==>LLPLLSetVcoRange
BYTE aRGB_SetVcoRange(DWORD _IPF)
{
	BYTE VCO_CURR, value, chged=0;
	WORD val;
	
	val = _IPF / 1000000L;

	dPrintf("\naRGB_SetVcoRange _IPF:%lx val:%dMHz",_IPF,val);  
												//   +------BUG
												//    ??pump value		
	if     ( val < 15 )		VCO_CURR = 0x01;	// 00 000
	else if( val < 34 )		VCO_CURR = 0x01;	// 00 000
	else if( val < 45 )		VCO_CURR = 0x11;	// 01 000
	else if( val < 63 )		VCO_CURR = 0x11;	// 01 000
	else if( val < 70 )		VCO_CURR = 0x21;	// 10 000
	else if( val < 80 )		VCO_CURR = 0x21;	// 10 000
	else if( val <100 )		VCO_CURR = 0x21;	// 10 000
	else if( val <110 )		VCO_CURR = 0x21;	// 10 000
	else					VCO_CURR = 0x31;	// 11 000
	VCO_CURR |= 0xC0;	//POST div 1
	
	WriteTW88Page(PAGE1_VADC);
	value = ReadTW88(REG1C2);
	if( VCO_CURR != value) {
		chged = 1;
		dPrintf(" R1C2:%bx->%bx", value, VCO_CURR );
		WriteTW88(REG1C2, VCO_CURR);			// VADC_VCOCURR
		delay1ms(1);					// time to stabilize
	}


//	#ifdef DEBUG_PC_MEAS
//	dPrintf("\r\nSetVCO=%02bx, changed=%bd", VCO_CURR, chged );
//	#endif
	return chged;
}
#endif


#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC) || defined(SUPPORT_BT656)
//-----------------------------------------------------------------------------
/**
* Read aRGB(VAdc) InputStauts
*
*	register
*	R1C1	LLPLL Input Detection Register
*	R1C1[7] - VSync input polarity. 1:Active High. 
*	R1C1[6]	- HSync input polarity. 1:Active High.
*	R1C1[5]	- VSYNC pulse detection status. 1=detected
*	R1C1[4]	- HSYNC pulse detection status. 1=detected
*	R1C1[3]	- Composite Sync detection status	
*	R1C1[2:0] Input source format detection in case of composite sync.
*				0:480i	1:576i	3:480p	3:576p
*				4:1080i	5:720p	6:1080p	7:fail
*/
BYTE aRGB_GetInputStatus(void)
{
	BYTE value;
	WriteTW88Page(PAGE1_VADC);
	value = ReadTW88(REG1C1);
	return value;
}
#endif

#if defined(SUPPORT_PC)
//-----------------------------------------------------------------------------
/**
* Read HSync&VSync input polarity status register.
* And, Set LLPLL input polarity & VSYNC output polarity.
*
*	Read R1C1[6] and set R1C0[2]   hPol
*	Read R1C1[7] and set R1CC[1]   vPol
*
*	PC uses aRGB_SetPolarity(0) and Component uses aRGB_SetPolarity(1).
*
* register
*	R1C0[2]	- LLPLL input polarity. Need Negative. CA_PAS need a normal
*	R1C1[6]	- HSync input polarity
*	R1C1[7] - VSync input polarity 
*	R1CC[1] - VSYNC output polarity. Need Positive

* ==>ARGBSetPolarity
* othername PolarityAdjust
*
* @param
*	fUseCAPAS.	If "1", R1C0[2] always use 0.
*				component use fUseCAPAS=1. 
*
*/
#if 0
void aRGB_SetPolarity(BYTE fUseCAPAS)
{
	volatile BYTE r1c1;
	int i;

	WriteTW88Page(PAGE1_VADC );
	for(i=0; i < 5; i++) {
		r1c1 = ReadTW88(REG1C1);
		Printf("\naRGB_SetPolarity hPol:%bx vPol:%bx", r1c1 & 0x40 ? 1:0, r1c1 & 0x80 ? 1:0);
		delay1ms(10);
	}
	if(fUseCAPAS) {
		//CA_PAS need a normal
		WriteTW88(REG1C0, ReadTW88(REG1C0) & ~0x04);
		WriteTW88(REG1CC, ReadTW88(REG1CC) & ~0x02);	//if active high, no inv.
	}
	else {
		//check HS_POL.		Make LLPLL input polarity Negative.
		//LLPLL request an active low.
		//Meas prefers an active high.
		if(r1c1 & 0x40) WriteTW88(REG1C0, ReadTW88(REG1C0) | 0x04);		//if active high, ((invert)). make active low.
		else			WriteTW88(REG1C0, ReadTW88(REG1C0) & ~0x04);	//if active low, ((normal)). keep active low.
		//check VS_POL.		Make VS output polarity Positive.
		//Meas prefers an active high.
		if(r1c1 & 0x80) WriteTW88(REG1CC, ReadTW88(REG1CC) & ~0x02);	//if active high, normal.
		else			WriteTW88(REG1CC, ReadTW88(REG1CC) | 0x02);		//if active low, invert.
	}
}
#endif
#endif

#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC)
#define LLPLL_POST_8		0x00
#define LLPLL_POST_4		0x40
#define LLPLL_POST_2		0x80
#define LLPLL_POST_1		0xC0 //*
#define LLPLL_VCO_40TO216	0x30 //*
#define LLPLL_PUMP_5		0x02 //*

/**
* Set LLPLL Control
*
*	register
*	R1C2[7:6]	PLL post divider
*	R1C2[5:4]	VCO range select
*	R1C2[2:0]	Charge pump current
*/
void aRGB_SetLLPLLControl(BYTE value)
{
	WriteTW88Page(PAGE1_VADC);
	WriteTW88(REG1C2, value);
}
#endif

//-----------------------------------------------------------------------------
// LLPLL Divider
//-----------------------------------------------------------------------------
#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC)
//-----------------------------------------------------------------------------
/**
* Write LLPLL divider
*
* other name: SetCoarse(WORD i)
*
*	register
*		R1C3[3:0]R1C4[7:0] - LLPLL Divider. PLL feedback divider. A 12-bit register 
* @param	value: PLL value. Use (Htotal-1)
* @param	fInit:	init flag
*/
void aRGB_LLPLLSetDivider(WORD value, BYTE fInit)
{
	volatile BYTE mode;

	WriteTW88Page(PAGE1_VADC );
	Write2TW88(REG1C3,REG1C4, value);
	if(fInit) {	
		WriteTW88(REG1CD, ReadTW88(REG1CD) | 0x01);		// PLL init
		//wait
		do {
			mode = TW8835_R1CD;
		} while(mode & 0x01);
	}
}
#endif

#if defined(SUPPORT_PC)
//-----------------------------------------------------------------------------
/**
* Read LLPLL divider value
*
* other name: GetCoarse(void)
*/
WORD aRGB_LLPLLGetDivider(void)
{
	WORD value;

	WriteTW88Page(PAGE1_VADC);
	Read2TW88(REG1C3,REG1C4,value);
	return value & 0x0FFF;
}
#endif

//-----------------------------------------------------------------------------
// LLPLL Clock PHASE
//-----------------------------------------------------------------------------
#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC)
//-----------------------------------------------------------------------------
/**
* set Phase value
*
*	register
*	R1C5[4:0]
* @param value: Phase value
* @param fInit:	init flag
*/
void aRGB_SetPhase(BYTE value, BYTE fInit)
{
	volatile BYTE mode;

	WriteTW88Page(PAGE1_VADC);
	WriteTW88(REG1C5, value&0x1f);
	if(fInit) {
		WriteTW88(REG1CD, ReadTW88(REG1CD) | 0x01);	// PLL init
		//wait
		do {
			mode = TW8835_R1CD;
		} while(mode & 0x01);
	}
}
#endif
#if defined(SUPPORT_PC)
//-----------------------------------------------------------------------------
/**
* get Phase value
*/
//-----------------------------------------------------------------------------
BYTE aRGB_GetPhase(void)
{
	WriteTW88Page(PAGE1_VADC);
	return ReadTW88(REG1C5) & 0x1f;		//VADC_PHASE
}
#endif



//-----------------------------------------------------------------------------
// LLPLL Filter BandWidth
//---------------------------
//register
//	R1C6[2:0]	R1C6 default: 0x20.
//-----------------------------------------------------------------------------
/**
* set filter bankwidth
*/
void aRGB_SetFilterBandwidth(BYTE value, WORD delay)
{
	if(delay)
		delay1ms(delay);
	WriteTW88Page(PAGE1_VADC);
	WriteTW88(REG1C6, (ReadTW88(REG1C6) & 0xF8) | value);
}


//-----------------------------------------------------------------------------
//desc: check input
//@param
//	type	0:YPbPr, 1:PC
//		YPbPr	Use CompositeSync(CSYNC) with clamping & Slicing
//		PC		Use a seperate HSYNC & VSYNC PIN
//
//for YPbPr
//	0x1C0[7:6]	= 0		default
//	0x1C0[3] = 0		Select Clamping output(not HSYNC)
//	0x1C0[4] = 1		Select CS_PAS
//
//for PC(aRGB)
//preprocess
//	0x1C0[3] = 1	Select HSYNC
//	0x1C0[4] = 0	Select HSYNC(or Slice, Not a CS_PAS)
//	
//detect
//	0x1C1[6]	Detected HSYNC polarity
//	0x1C1[4]	HSYNC detect status
//postprocess
//	0x1C0[2]	PLL reference input polarity	
//
//return
//	0: fail
//	else: R1C1 value		
#ifdef UNCALLED_SEGMENT
BYTE aRGB_CheckInput(BYTE type)
{
//	BYTE value;
	volatile BYTE rvalue;
	BYTE check;
	BYTE i;

	ePrintf("\naRGB_CheckInput(%bx) %s",type,type ? "PC": "YPbPr" );

	WriteTW88Page(PAGE1_VADC );

//	//power up PLL, SOG,....
//	value = 0x40;										// powerup PLL
//	if(type==0)	value |= 0x80;							// powerup SOG
//	WriteTW88(REG1CB, (ReadTW88(REG1CB) & 0x1F) | value );	// keep SOG Slicer threshold & coast
		
	if(type==0) check = 0x08;	//check CompositeSynch
	else 		check = 0x30;	//check HSynch & VSynch

	//(YPbPr need more then 370ms, PC need 200ms). max 500ms wait
	for(i=0; i < 50; i++) {
		rvalue = TW8835_R1C1;
		dPrintf(" %02bx",rvalue);

		if((rvalue & check) == check) {
			ePrintf("->success:%bd",i);
			return rvalue;	
		}
		delay1ms(10);
	}
	ePrintf("->fail");

//	WriteTW88(REG1CB, ReadTW88(REG1CB) & 0x1F);	//PowerDown

	return 0;	//No detect		
}
#endif


//-----------------------------------------------------------------------------
//only for test
//-----------------------------------------------------------------------------
#ifdef UNCALLED_SEGMENT
BYTE aRGB_DoubleCheckInput(BYTE detected)
{
	BYTE i, count;
	BYTE rvalue;
	BYTE old = detected;

	ePrintf("\naRGB_DoubleCheckInput");
	count=0;

	WriteTW88Page(PAGE1_VADC );
	for(i=0; i < 200; i++) {
		rvalue = ReadTW88(REG1C1 );
		if(rvalue == old) count++;
		else {
			dPrintf(" %02bx@%bd", rvalue, i);
			old = rvalue;
			count=0;
		}
		if(count >= 30) {
			ePrintf("->success");
			return rvalue;
		}
		delay1ms(10);
	}
	ePrintf("->fail");
	return 0;
}
#endif


//0x1C1 - LLPLL Input Detection Register 



#ifdef UNCALLED_SEGMENT
#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC)
//-----------------------------------------------------------------------------
//power down SOG,PLL,Coast
//register
//	R1CB[7]	SOG power down.	1=Powerup
//	R1CB[6]	PLL power down.	1=Powerup
//	R1CB[5]	PLL coast function. 1=Enable
//-----------------------------------------------------------------------------
void aRGB_SetPowerDown(void)
{
	WriteTW88Page(PAGE1_VADC );
	WriteTW88(REG1CB, (ReadTW88(REG1CB) & 0x1F));	
}
#endif
#endif

//-----------------------------------------------------------------------------
//register
//	R1CB[4:0]  SOG Slicer Threshold Register
//-----------------------------------------------------------------------------
#ifdef UNCALLED_SEGMENT
void aRGB_SetSOGThreshold(BYTE value)
{
	WriteTW88Page(PAGE1_VADC );
	WriteTW88(REG1CB, ReadTW88(REG1CB) & ~0x1F | value);	
}
#endif

//-----------------------------------------------------------------------------
//gain control
//R1D0[2]R1D1[7:0]	Y/G channel gain
//R1D0[1]R1D2[7:0]	C/B channel gain
//R1D0[0]R1D3[7:0]	V/R channel gain
//-----------------------------------------------------------------------------
#ifdef SUPPORT_PC
void aRGB_SetChannelGainReg(WORD GainG,WORD GainB,WORD GainR)
{
	WriteTW88Page(PAGE1_VADC );
	WriteTW88(REG1D1, GainG );
	WriteTW88(REG1D2, GainB );
	WriteTW88(REG1D3, GainR );
	WriteTW88(REG1D0, (GainR >> 8)+ ((GainB >> 7) & 2) + ((GainG >> 6) & 4 ));
}
WORD aRGB_ReadGChannelGainReg(void)
{
	WORD wTemp;
	WriteTW88Page(PAGE1_VADC );
	wTemp = ReadTW88(REG1D0) & 0x04;
	wTemp <<= 6;
	wTemp |= ReadTW88(REG1D1);
	return wTemp;
}
WORD aRGB_ReadBChannelGainReg(void)
{
	WORD wTemp;
	WriteTW88Page(PAGE1_VADC );
	wTemp = ReadTW88(REG1D0) & 0x02;
	wTemp <<= 7;
	wTemp |= ReadTW88(REG1D2);
	return wTemp;
}
WORD aRGB_ReadRChannelGainReg(void)
{
	WORD wTemp;
	WriteTW88Page(PAGE1_VADC );
	wTemp = ReadTW88(REG1D0) & 0x01;
	wTemp <<= 8;
	wTemp |= ReadTW88(REG1D3);
	return wTemp;
}
#endif

//-----------------------------------------------------------------------------
//register
//	R1D4[7:0]
//-----------------------------------------------------------------------------
#if 0
void aRGB_SetClampMode(BYTE value)
{
	WriteTW88Page(PAGE1_VADC );
	WriteTW88(REG1D4, value );
}
#endif

//-----------------------------------------------------------------------------
//register
//	R1D4[5]
//-----------------------------------------------------------------------------
#ifdef SUPPORT_COMPONENT
/**
* set clamp mode and HSync Edge
*/
void aRGB_SetClampModeHSyncEdge(BYTE fOn)
{
	BYTE bTemp;
	WriteTW88Page(PAGE1_VADC );
	bTemp =  ReadTW88(REG1D4);

	if(fOn)	bTemp |=  0x20;
	else	bTemp &= ~0x20;

	WriteTW88(REG1D4, bTemp);
}
#endif

//-----------------------------------------------------------------------------
//register
//	R1D7[7:0]
//-----------------------------------------------------------------------------
#ifdef SUPPORT_COMPONENT
/**
* set clamp position
*/
void aRGB_SetClampPosition(BYTE value)
{
	WriteTW88Page(PAGE1_VADC );
	WriteTW88(REG1D7, value );	// ADC clamp position from HSync edge by TABLE ClampPos[]
}
#endif

//-----------------------------------------------------------------------------
//register
//	R1E6[5]	PGA control	0=low speed operation. 1=high speed operation
//-----------------------------------------------------------------------------
#ifdef UNCALLED_SEGMENT
/**
* set PGA control
*/
void aRGB_SetPGAControl(BYTE fHigh)
{
	WriteTW88Page(PAGE1_VADC );
	if(fHigh)	WriteTW88(REG1E6, ReadTW88(REG1E6) | 0x20);		//HighSpeed
	else		WriteTW88(REG1E6, ReadTW88(REG1E6) & ~0x20);	//LowSpeed
}
#endif

//===================================================================
//
//===================================================================
//-----------------------------------------------------------------------------
/**
* set default VAdc for PC & Component.
*
* If input is not PC or Component, powerdown VAdc.
*	R1C0[]	10
*	R1C2[]	d2
*	* R1C6	20
*	R1CB[]
*	R1CC[]
*	R1D4[]	00	20
*	R1D6[]	10	10
*	R1D7[]		00
*	R1DA[]	80	01
*	R1DB[]	80	01
*	R1E6[]	00  20		PGA high
* external
*  InputMain
* @todo pls, remove or refind. it is too big.
*/

//BKTODO. make it as table and move to InitReg.c
void aRGB_SetDefaultFor(void)
{
#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC)
	BYTE rvalue;
#endif
	//dPrintf("\naRGB_SetDefaultFor()");

	WriteTW88Page(PAGE1_VADC );
	if ( InputMain == INPUT_COMP ) {
#ifdef SUPPORT_COMPONENT
		WriteTW88(REG1C0,0x10);	// mode for SOG slicer
		WriteTW88(REG1C2,0xD2);	// ==> VCO Charge pump		POST:1. VCO:10~54MHz Pump:5uA
		WriteTW88(REG1C6,0x20);	// PLL loop control
		WriteTW88(REG1C9,0x00);	// Pre-coast = 0
		WriteTW88(REG1CA,0x00);	// Post-coast = 0
		WriteTW88(REG1CB,0xD6);	// Power up PLL, SOG
		WriteTW88(REG1CC,0x00);	// ==> Sync selection

		WriteTW88(REG1D0,0x00);	// ADC gain
		WriteTW88(REG1D1,0xF0);	// 
		WriteTW88(REG1D2,0xF0);	// 
		WriteTW88(REG1D3,0xF0);	// 

		WriteTW88(REG1D4,0x20);	// clamp mode
		WriteTW88(REG1D5,0x00);	// clamp start
		WriteTW88(REG1D6,0x10);	// clamp stop
		WriteTW88(REG1D7,0x00);	// clamp pos.
		WriteTW88(REG1D9,0x02);	// clamp Y level
		WriteTW88(REG1DA,0x80);	// clamp U level
		WriteTW88(REG1DB,0x80);	// clamp V level
		WriteTW88(REG1DC,0x10);	// HS width

		WriteTW88(REG1E2,0x59);	//***	0x59
		WriteTW88(REG1E3,0x17);	//***	0x37
		WriteTW88(REG1E4,0x34);	//***	0x55
		WriteTW88(REG1E5,0x33);	//***	0x55

		WriteTW88(REG1E6,0x20);	// PGA high speed

		//set default divider(856-1. for 480i or 480p) & phase. 
		aRGB_LLPLLSetDivider(0x035A, 1);
		//rvalue=GetPhaseEE(EE_YUVDATA_START+0);
		//if(rvalue==0xff)
			rvalue=0;
		aRGB_SetPhase(rvalue, 0);


		aRGB_setSignalPath(0); //add 130227

#endif
	}
	else if ( InputMain == INPUT_PC ) {
#ifdef SUPPORT_PC
		WriteTW88(REG1C0,0x08);	// mode for HV sync
		WriteTW88(REG1C2,0xD2);	// ==> VCO Charge pump		POST:1. VCO:10~54MHz Pump:5uA
		WriteTW88(REG1C6,0x20);	// PLL loop control
		WriteTW88(REG1C9,0x00);	// Pre-coast = 0
		WriteTW88(REG1CA,0x00);	// Post-coast = 0
		WriteTW88(REG1CB,0x56);	// Power up PLL
		WriteTW88(REG1CC,0x12);	// ==> Sync selection

		WriteTW88(REG1D0,0x00);	// ADC gain
		WriteTW88(REG1D1,0xF0);	// 
		WriteTW88(REG1D2,0xF0);	// 
		WriteTW88(REG1D3,0xF0);	// 

		WriteTW88(REG1D4,0x20);	// clamp mode
		WriteTW88(REG1D5,0x00);	// clamp start
		WriteTW88(REG1D6,0x10);	// clamp stop
		WriteTW88(REG1D7,0x00);	// clamp pos.
		WriteTW88(REG1D9,0x02);	// clamp G/Y level
		WriteTW88(REG1DA,0x01);	// clamp B/U level
		WriteTW88(REG1DB,0x01);	// clamp R/V level
		WriteTW88(REG1DC,0x10);	// HS width

		WriteTW88(REG1E2,0x59);	//***  0x59
		WriteTW88(REG1E3,0x17);	//***  0x37
		WriteTW88(REG1E4,0x34);	//***  0x55
		WriteTW88(REG1E5,0x33);	//***  0x55

		WriteTW88(REG1E6,0x20);	// PGA high speed

		//set default divider(1056, for SVGA) & phase. 
		aRGB_LLPLLSetDivider(0x0420, 1);	
		rvalue=GetPhaseEE(5);	//SVGA.
		if(rvalue==0xff)
			rvalue=0;
		aRGB_SetPhase(rvalue, 0); //VGA

		aRGB_setSignalPath(1); //add 130207
#endif
	}
	else
	{
		//power down SOG,PLL,Coast
		//same as aRGB_SetPowerDown();	
		WriteTW88(REG1CB, (ReadTW88(REG1CB) & 0x1F));
		LLPLLSetClockSource(1);			//select 27MHz. R1C0[0]
	}	
}




//-----------------------------------------------------------------------------
//R1D0
//R1D1 Y channel gain
//R1D2 C channel gain
//R1D3 V channel gain
//read RGB max value from meas and adjust color gain value on VAdc.
//-----------------------------------------------------------------------------

#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC)
//-----------------------------------------------------------------------------
//extern BYTE WaitStableLLPLL(WORD delay);
//BYTE aRGB_SetupLLPLL(WORD divider, /*BYTE ctrl,*/ BYTE fInit, BYTE delay)
//-----------------------------------------------------------------------------
/**
* update LLPLL divider
*
*/
BYTE aRGB_LLPLLUpdateDivider(WORD divider, /*BYTE ctrl,*/ BYTE fInit, BYTE delay)
{
	BYTE ret;
	
	ret = ERR_SUCCESS;
	
	aRGB_SetFilterBandwidth(0, 0);

//	//LLPLL Control.
//	aRGB_SetLLPLLControl(ctrl);

	aRGB_LLPLLSetDivider(divider, fInit);
	if (fInit)
	{
		if (WaitStableLLPLL(delay))
			ret = ERR_FAIL;
	}
	aRGB_SetFilterBandwidth(7, 0);	//restore
	
	return ret;
}
#endif

#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC)
//-----------------------------------------------------------------------------
/**
* wait stable LLPLL input.
*
* @return
*	0:success. ERR_SUCCESS.
*	1:fail. ERR_FAIL
*/
BYTE WaitStableLLPLL(WORD delay)
{
	DECLARE_LOCAL_page
	BYTE	i;
	WORD	HActive, HActiveOld, HStart;
	BYTE 	PolOld;
	volatile BYTE	Pol;

	ReadTW88Page(page);

	if(delay)
		delay1ms(delay);

#if defined(DEBUG_COMP) || defined(DEBUG_PC)
	dPrintf("\nWaitStableLLPLL: ");
#endif
	for(i=0; i < 128; i++) {	//max loop
		if(MeasStartMeasure()) {
#if defined(DEBUG_COMP) || defined(DEBUG_PC)
			dPrintf("fail measure");
#endif
			WriteTW88Page(page );
			return ERR_FAIL;
		}
		HActive = MeasGetHActive( &HStart );	//h_active_start h_active_perios
		Pol = aRGB_GetInputStatus();
		if(i==0) {
			//skip.
		}
		else if((HActive==HActiveOld) && (Pol == PolOld)) {
#if defined(DEBUG_COMP) || defined(DEBUG_PC)
			dPrintf("%bd times, HS: %d, HActive: %d InputStatus:0x%bx", i, HStart, HActive, Pol);
#endif
			WriteTW88Page(page );
			return ERR_SUCCESS;
		}
		HActiveOld = HActive;
		PolOld = Pol;
	}
#if defined(DEBUG_COMP) || defined(DEBUG_PC)
	dPrintf("fail max loop");
#endif
	WriteTW88Page(page );
	return ERR_FAIL;
}
#endif


#if defined(SUPPORT_PC)
//-----------------------------------------------------------------------------
/**
* find PC input mode
*
* @return
*	0: fail
*	else: success.
*		  index number of PC Mode Data Table.
*/
BYTE FindInputModePC(WORD *vt)
{
	WORD	vtotal;
	WORD 	wTemp;
	BYTE	vFreq, i;

	//
	// get a vertical frequency and  a vertical total scan lines.
	//
	//BKFYI. We donot have a PLL value yet that depend on the mode.
	//so, we are using 27MHz register.

	vtotal = MeasGetVPeriod();	//Vertical Period Registers
	//if video signal starts, HW needs a time.
	//normally ?? mS.  I saw, Compment needs 30mS.
	for(i=0; i < 10; i++) {
		MeasStartMeasure();
		wTemp = MeasGetVPeriod();	//Vertical Period Registers	
		if(wTemp == vtotal)
			break;
		vtotal = wTemp;
		delay1ms(5);
	}
	*vt = vtotal;

	vFreq = MeasGetVFreq();
	vFreq = MeasRoundDownVFreqValue(vFreq);

	//
	//Search PC mode.
	//0 is unknown.
	for ( i=1; i<(sizeof_PCMDATA() / sizeof(struct _PCMODEDATA)); i++ ) {
		if ( PCMDATA[i].support == 0 ) continue;
		if ( PCMDATA[i].vFreq == vFreq ) {			//check vfreq
			if(( PCMDATA[i].vTotal == vtotal )		//check vtotal 
			|| ( PCMDATA[i].vTotal == (vtotal+1) ) 
			|| ( PCMDATA[i].vTotal == (vtotal-1) )
			|| ( PCMDATA[i].vTotal == (vtotal+2) ) 
			|| ( PCMDATA[i].vTotal == (vtotal-2) ) 
			){

				//dPrintf("\nFindInputModePC FOUND mode=%bd vtotal:%d, %dx%d@%bdHz", 
				//	i, vtotal, PCMDATA[i].han, PCMDATA[i].van, vfreq);
				dPrintf("\nFindInputModePC mode:%bd %dx%d@%bd vTotal:%d",
					i,PCMDATA[i].hActive, PCMDATA[i].vActive,PCMDATA[i].vFreq, vtotal);
				return (i);
			}
		}
	}

	ePuts( "\nCurrent Input resolution IS Not Supported." );
	ePrintf(" V total: %d, V freq: %bd", vtotal, vFreq );
	return (0);							// not support
}
#endif



#if defined(SUPPORT_COMPONENT)
//-----------------------------------------------------------------------------
/**
* find component input mode
*
* @return
*	0xFF: fail.
*	other:success. component mode value.
*
*	0:480i
*	1:576i@50
*	2:480p
*	3:576p@50
*	4:1080i@50
*	5:1080i
*	6:720p@50
*	7:720p
*	8:1080p@50
*   9:1080p
*
*/
BYTE FindInputModeCOMP( void )
{
	WORD	vtotal;
	BYTE	vfreq, i;
	WORD wTemp;

	//
	// get a vertical frequency and  a vertical total scan lines.
	//
	//BKFYI. We donot have a PLL value yet that depend on the mode.
	//so, we better use 27MHz register.

	vtotal = MeasGetVPeriod();	//Vertical Period Registers

	//if video signal starts, HW needs a time.
	//normally 30mS.
	for(i=0; i < 10; i++) {
		MeasStartMeasure();
		wTemp = MeasGetVPeriod();	//Vertical Period Registers	
		if(wTemp == vtotal)
			break;
		vtotal = wTemp;
		delay1ms(5);
	}
	vfreq = MeasGetVFreq();
	if ( vfreq < 55 ) vfreq = 50;
	else  vfreq = 60;

	if ( vfreq == 50 ) {
		if ( vtotal < 200)			i = 0xFF;
		else if ( vtotal < 320 )	i = MEAS_COMP_MODE_576I;	// 576i	 = 625 for 2, 312.5
		else if ( vtotal < 590 )	i = MEAS_COMP_MODE_1080I25;	// 1080i50A
		else if ( vtotal < 630 )	i = MEAS_COMP_MODE_576P;	// 576P=625	or 1080i50B = sync=5	  
																// vblank length different 576P=45, 1080i=21
																// can check with even/odd measure
		else if ( vtotal < 800 )	i = MEAS_COMP_MODE_720P50;	// 720P = 750
		else if ( vtotal < 1300 )	i = MEAS_COMP_MODE_1080P50;	// 1080P = 1250 total from set-top box
		else 						i = 0xFF;	
	}
	else {
		if ( vtotal < 200)			i = 0xFF;
		else if ( vtotal < 300 )	i = MEAS_COMP_MODE_480I;	// 480i = 525 for 2, 262.5
		else if ( vtotal < 540 )	i = MEAS_COMP_MODE_480P;	// 480P	= 525
		else if ( vtotal < 600 )	i = MEAS_COMP_MODE_1080I30;	// 1080i
		else if ( vtotal < 800 )	i = MEAS_COMP_MODE_720P60;	// 720P = 750
		else if ( vtotal < 1300 )	i = MEAS_COMP_MODE_1080P60;	// 1080P
		else 						i = 0xFF;	
	}

	dPrintf( "\nCOMP %bd: vFreq:%bdHz, vTotal:%d", i, vfreq, vtotal );
	return (i);							// if 0xff, not support
}
#endif



#ifdef SUPPORT_PC
//-----------------------------------------------------------------------------
/**
* set inputcrop for PC
*
* output
*	RGB_HSTART
*/
#if 0
static void PCSetInputCrop( BYTE mode )
{
	BYTE hPol, vPol;
	WORD hStart, vStart;
	BYTE bTemp;
	WORD SyncWidth;
	WORD Meas_hActive,Meas_vActive;
	WORD Meas_Start;

	bTemp = ReadTW88(REG041);
	if(bTemp & 0x08) 	vPol = 1;	//0:rising edge, 1:falling edge
	else				vPol = 0;
	if(bTemp & 0x04) 	hPol = 1;	//0:rising edge, 1:falling edge
	else				hPol = 0;

	dPrintf("\nPCSetInputCrop offset:%bd, HPol: %bd, VPol: %bd", hPol, vPol );

	//read sync width.
	//HSyncWidth comes from .., not real value.
	//Meas_HPulse = MeasGetHSyncRiseToFallWidth();
	//Meas_VPulse = MeasGetVSyncRiseToFallWidth();
	//dPrintf("\n\tHPulse: %d, VPulse: %d", Meas_HPulse, Meas_VPulse );



	//meas uses active high sync, and use a rising edge.
	//if you give a active low sync to meas, we will have a wrong result.
	//meas has an internal delay.
	//PC has 5 offset on horizontal.
	//       1 offset on vertical.
	//if you use a falling edge in inputcrop, you have to remove SyncWidth.
	Meas_hActive = MeasGetHActive( &Meas_Start );
	hStart = Meas_Start +5;
	if(hPol) {
		SyncWidth = MeasGetHSyncRiseToFallWidth();
		hStart -= SyncWidth;	
	}

	Meas_vActive = MeasGetVActive( &Meas_Start );
	vStart = Meas_Start +1;
	if(vPol) {
		SyncWidth = MeasGetVSyncRiseToFallWidth();
		vStart -= SyncWidth;
	}

	RGB_HSTART = hStart;	//save for IE.

	//adjust EEPROM. 0..100. base 50. reversed value.
	bTemp = GetHActiveEE(mode); //PcBasePosH;
	if(bTemp != 50) {
		hStart += 50;
		hStart -= bTemp;
		dPrintf("\n\tModified HS:%d->%d, VS:%d", RGB_HSTART, hStart, vStart );
	}
	dPrintf("\n\tHLen: %d, VLen: %d", Meas_hActive, Meas_vActive );

//#ifdef MODEL_TW8835
//	InputSetCrop(hstart, 1, Meas_hActive, 0x7FE);
//#else
	InputSetCrop(hStart, vStart, Meas_hActive, Meas_vActive);
//#endif
}
#endif
#endif //#ifdef SUPPORT_PC

#ifdef SUPPORT_PC
//-----------------------------------------------------------------------------
/**
* set PC output value
*
* extern
*	MeasHLen
*	MeasVLen
*
* Set V first. 
*/
#if 0
static void PCSetOutput( BYTE mode )
{
	WORD wTemp;
	BYTE bTemp;
	WORD Meas_Active,Meas_Start;

	dPuts("\nPCSetOutput");

	Meas_Active = MeasGetVActive( &Meas_Start );
	ScalerSetVScale(Meas_Active);

	Meas_Active = MeasGetHActive( &Meas_Start );
	ScalerSetHScale(Meas_Active);

	//=============VDE=====================
	wTemp = ScalerCalcVDE();
	dPrintf("\n\tV-DE start = %d", wTemp);
	RGB_VDE = wTemp;
	//use EEPROM
	bTemp = GetVBackPorchEE(mode);
	if(bTemp != 50) {
		wTemp += bTemp;
		wTemp -= 50;
		dPrintf("=> %d", wTemp);
	}
	//if 640x460, -1.
	//if 1280x1024, need -1.  40*480/1024=18.75. It was 19. Correct 18.


	ScalerWriteVDEReg(wTemp);

	//=============HDE=====================
	wTemp = ScalerCalcHDE();
	dPrintf("\n\tH-DE start = %d", wTemp);
	if(ReadTW88(REG045) & 1) {
		//If Input crop is odd, minus 1. BK130122
		wTemp--;
		dPrintf("=> %d", wTemp);
	}
	ScalerWriteHDEReg(wTemp);	//BKFYI. Scaler ouput width : 801

	//================= H Free Run settings ===================================
	wTemp=ScalerCalcFreerunHtotal();
	dPrintf("\n\tFree Run Htotal: 0x%x", wTemp);
	ScalerWriteFreerunHtotal(wTemp);

	//================= V Free Run settings ===================================
	wTemp=ScalerCalcFreerunVtotal();
	dPrintf("\n\tFree Run Vtotal: 0x%x", wTemp);
	ScalerWriteFreerunVtotal(wTemp);

	//================= FreerunAutoManual, MuteAutoManual =====================
	ScalerSetFreerunAutoManual(ON,OFF);
	ScalerSetMuteAutoManual(ON,0x02);	//use skip on Manual.
}
#endif


static void PC_SetScaler(BYTE mode)
{
	BYTE hPol, vPol;
	WORD hStart, vStart;
	BYTE bTemp;
	WORD SyncWidth;
	WORD Meas_hActive, Meas_vActive;
	WORD Meas_hStart, Meas_vStart;
//	WORD vSync;
	WORD wTemp;

	//read measured value. 
	Meas_vActive = MeasGetVActive( &Meas_vStart );				//v_active_start v_active_perios
	Meas_hActive = MeasGetHActive( &Meas_hStart );				//h_active_start h_active_perios
	dPrintf("\nMeasure %dx%d hStart:%d+4, vStart:%d-1", Meas_hActive,Meas_vActive, Meas_hStart,Meas_vStart );
	Meas_hStart += 4;	//add meas delay.
	Meas_vStart -= 1;   //minus meas delay.

	//vSync = MeasGetVSyncRiseToFallWidth();
	//if(vSync > Meas_vActive) {
	//	Printf("\nBUG vSync:%d",vSync);
	//}

	//if input video is a special, meas can be incorrect.
	//In this case, use a Table value. It will hurt InputCrop and SetOuput functions.=>So I merged it.
	if ( Meas_vActive < PCMDATA[ mode ].vActive ) {			// use table
		Meas_vStart = PCMDATA[mode].vstart;
		Meas_vActive = PCMDATA[mode].vActive;
		dPrintf("\nUse table vStart:%d vActive:%d",Meas_vStart,Meas_vActive);
	}
	if ( Meas_hActive < PCMDATA[ mode ].hActive ) {			// use table
		Meas_hStart = PCMDATA[mode].hstart;
		Meas_hActive = PCMDATA[mode].hActive;
		dPrintf("\nUse table hStart:%d hActive:%d",Meas_hStart,Meas_hActive);
	}

	//read sync width.
	//HSyncWidth comes from .., not real value.
	//Meas_HPulse = MeasGetHSyncRiseToFallWidth();
	//Meas_VPulse = MeasGetVSyncRiseToFallWidth();
	//dPrintf("\n\tHPulse: %d, VPulse: %d", Meas_HPulse, Meas_VPulse );

	//read Scaler input polarity.
	//0:rising edge, 1:falling edge.
	bTemp = ReadTW88(REG041);
	if (bTemp & 0x08)
		vPol = 1;	
	else
		vPol = 0;
	if (bTemp & 0x04)
		hPol = 1;	//0:rising edge, 1:falling edge
	else
		hPol = 0;
	dPrintf("\nPCSetInputCrop hPol:%bd vPol:%bd", hPol, vPol );

	//meas uses active high sync, and use a rising edge.
	//if you give a active low sync to meas, we will have a wrong result.
	//meas has an internal delay.
	//
	//PC inpurcrop has 1 offset on horizontal.
	//                 2 offset on vertical.
	//
	//if you use a falling edge in inputcrop, you have to remove SyncWidth.
	//
	hStart = Meas_hStart +1;
	if (hPol) { //if it uses fall_edge, subtract SynchWidth.
		SyncWidth = MeasGetHSyncRiseToFallWidth();
		hStart -= SyncWidth;	
	}
	vStart = Meas_vStart +2;
	if (vPol) {	//if it uses fall_edge, subtract SynchWidth.
		SyncWidth = MeasGetVSyncRiseToFallWidth();
		vStart -= SyncWidth;
	}

	RGB_HSTART = hStart;	//save for IE.

	//adjust EEPROM. 0..100. base 50. reversed value.
	bTemp = GetHActiveEE(mode); //PcBasePosH;
	if (bTemp != 50)
	{
		hStart += 50;
		hStart -= bTemp;
		dPrintf("\n\tModified HS:%d->%d, VS:%d", RGB_HSTART, hStart, vStart );
	}
	dPrintf("\n\tHLen: %d, VLen: %d", Meas_hActive, Meas_vActive );

	InputSetCrop(hStart, vStart, Meas_hActive, Meas_vActive);

	//==================================
	//
	//
	dPuts("\nPCSetOutput");

	ScalerSetVScale(Meas_vActive);
	ScalerSetHScale(Meas_hActive);

	//=============VDE=====================
	//if we are using the table, ScalerCalcVDE need a base value.
	wTemp = ScalerCalcVDE2(vStart, PCMDATA[ mode ].vDeOffset);	 
	dPrintf("\n\tV-DE start = %d", wTemp);
	RGB_VDE = wTemp;
	
	//use EEPROM
	bTemp = GetVBackPorchEE(mode);
	if(bTemp != 50) {
		wTemp += bTemp;
		wTemp -= 50;
		dPrintf("=> %d", wTemp);
	}
	ScalerWriteVDEReg(wTemp);

	//=============HDE=====================
	wTemp = ScalerCalcHDE();
	dPrintf("\n\tH-DE start = %d", wTemp);
	if(ReadTW88(REG045) & 1) {
		//If Input crop is odd, minus 1. BK130122
		wTemp--;
		dPrintf("=> %d", wTemp);
	}
	ScalerWriteHDEReg(wTemp);

	//================= H Free Run settings ===================================
	wTemp=ScalerCalcFreerunHtotal();
	dPrintf("\n\tFree Run Htotal: 0x%x", wTemp);
	ScalerWriteFreerunHtotal(wTemp);

	//================= V Free Run settings ===================================
	wTemp=ScalerCalcFreerunVtotal();
	dPrintf("\n\tFree Run Vtotal: 0x%x", wTemp);
	ScalerWriteFreerunVtotal(wTemp);

	//================= FreerunAutoManual, MuteAutoManual =====================
	ScalerSetFreerunAutoManual(ON,OFF);
	ScalerSetMuteAutoManual(ON,0x02);	//use skip on Manual.
}



#endif


#if defined(SUPPORT_COMPONENT)
//-----------------------------------------------------------------------------
/**
* set component output value
*
*/
#if 0
static void YUVSetOutput(BYTE mode)
{
	BYTE HDE;
	WORD temp16;
#if (PANEL_V==800)
	WORD vStart;
	WORD vActive;
	WORD vDE;
#endif

	BYTE mode_eia;
	struct VIDEO_TIME_TABLE_s *p_EIA;

	mode_eia = ConvertCompIdToEiaId(mode);
	p_EIA = &MY_EIA_TABLE[mode_eia];
	//ScalerWriteOutputWidth(PANEL_H+1);
	//ScalerWriteOutputHeight(PANEL_V);

	ScalerSetHScale(YUVDisplayH[mode]);
	ScalerSetVScale(YUVDisplayV[mode]);

	HDE = ScalerReadLineBufferDelay() + 32;
	dPrintf("\nH-DE start = %bd", HDE);
	ScalerWriteHDEReg(HDE);

#if (PANEL_V==800)
	//BKTODO 130225:
	//vStart = YUVOffsetV[mode];
//	vStart = MYStartV[mode];
//	temp16 = (YUVCropV[mode] - YUVDisplayV[mode]) / 2;
//	vStart += temp16;
//	vStart = vStart * PANEL_V / YUVDisplayV[mode];
//	ScalerWriteVDEReg(vStart);

	vStart = p_EIA->vSync + p_EIA->vBPorch + p_EIA->vOverScan;
	vActive = p_EIA->vActive - p_EIA->vOverScan*2;
	vDE = (DWORD)vStart * PANEL_V / vActive;
	ScalerWriteVDEReg(vDE);
#else
	ScalerWriteVDEReg(YUVOffsetV[mode]);
#endif

	//===== Free Run settings ==========
	temp16=ScalerCalcFreerunHtotal();
	ScalerWriteFreerunHtotal(temp16);

	temp16 = ScalerCalcFreerunVtotal();
	ScalerWriteFreerunVtotal(temp16);

	ScalerSetFreerunAutoManual(ON,OFF);
	ScalerSetMuteAutoManual(ON,OFF);
}
#endif
#endif


#ifdef SUPPORT_PC
//-----------------------------------------------------------------------------
/** 
* check PC mode
*
* @return
*	0:fail
*	other: pc mode value
*/
#if 0
static BYTE PCCheckMode(void)
{
	BYTE i, mode;
	WORD VTotal;	//dummy

	for(i=0; i < 10; i++) {
		if(MeasStartMeasure())
			return 0;

 		mode = FindInputModePC(&VTotal);	// find input mode from Vfreq and VPeriod
		if(mode) {
			dPrintf("\nPCCheckMode ret:%bd",mode);
			return mode;
		}
	}

	return 0;
}
#endif
#endif

#if defined(SUPPORT_PC) || defined(SUPPORT_DVI) || defined(SUPPORT_HDMI)
//-----------------------------------------------------------------------------
/** 
* adjust the pixel clock
*
* oldname: void	PCLKAdjust( BYTE mode )
*
* INPUT_PC
*	use mode.
*
* INPUT_DVI
*	skip divider & mode.
*
* INPUT_HDMI
*	use divider.
*/
void AdjustPixelClk(WORD digital_divider, BYTE mode )
{
#ifdef PANEL_AUO_B133EW01
	//do nothing
	WORD wTemp;
	wTemp = digital_divider;
	wTemp = mode;
#else
	DWORD	PCLK, PCLK1, PCLK2;
	BYTE	i, PCLKO;
	WORD	HDown, HPeriod, Divider, VPN, VScale, HActive, H_DE;
	DWORD	VPeriod, VFreq;


	PCLK = SspllGetPPF();
	//	FPCLK1 calculation
	//	FREQ = REG(0x0f8[3:0],0x0f9[7:0],0x0fa[7:0])															   	
	//	POST = REG(0x0fd[7:6])
	//	Hperiod = REG(0x524[7:0],0x525[7:0])
	//	Divider = REG(0x1c3[3:0],0x1c4[7:0]) ;;InFreq = (Divider+1) * (27000000 * FREQ / ((2^15)*(2^POST))) / Hperiod
	//	Hdown = REG(0x20a[3:0],0x209[7:0])
	//	PCLKO = REG(0x20d[1:0]) {1,1,2,3}
	//	PCLKx = REG(0x20d[1:0]) {1,2,3,4}
	//	result = ((Divider+1) * (27000000 * FREQ / ((2^15)*(2^POST))) / Hperiod) * (1024 / Hdown) * (PCLKx / PCLKO)
	//	result = ((Divider+1) * FPCLK / Hperiod) * (1024 / Hdown) * (PCLKx / PCLKO)

	HDown=ScalerReadXDownReg();
	HPeriod = MeasGetHPeriod();
	VPeriod = MeasGetVPeriod27();
	VFreq = 27000000L / VPeriod;
#if defined(SUPPORT_PC)
	if(InputMain==INPUT_PC /*|| InputMain==INPUT_COMP*/) {
		Divider = aRGB_LLPLLGetDivider() + 1;
		//Divider = PCMDATA[ mode ].htotal - 1 +1;
	}
	else 
#endif
	{
		//DTV input(DVI,HDMI)
#ifdef SUPPORT_DVI
		//if DVI ??. No Component
		if(InputMain==INPUT_DVI) {
			Divider = MeasGetDviDivider();
		}
		else 
#endif
		{
			//HDMI
			Divider = digital_divider; //DVI_Divider;
		}
	}

	VPN = MeasGetVPeriod();
	VScale = ScalerReadVScaleReg();

	H_DE = ScalerReadHDEReg();
	HActive = ScalerReadOutputWidth();
	//	FPCLK2 calculation
	//	PCLKx = REG(0x20d[1:0]) {1,2,3,4}
	//	VPN    = REG(0x522[7:0],0x523[7:0])
	//	Vscale = REG(0x206[7:0],0x205[7:0]) ;;Vtotal = VPN / (Vscale / 8192)
	//	H_DE   = REG(0x210[7:0])
	//	Hactive= REG(0x212[3:0],0x211[7:0]) ;;Htotal = H_DE + Hactive + 10
	//	Vperiod = REG(0x543[7:0],0x544[7:0],0x545[7:0]) ;;Vfreq = 27000000 / Vperiod
	//	result = (H_DE + Hactive + 1) * (VPN / (Vscale / 8192)) * (27000000 / Vperiod) * PCLKx

	dPrintf("\nPCLK:%ld, Divider: %d, HPeriod: %d, HDown: %d", PCLK, Divider, HPeriod, HDown);
	if(InputMain==INPUT_PC) {
		for ( i=2; i<=4; i++ ) {
			//PCLK1 = (DWORD)(((Divider+1) * PCLK / HPeriod) * (1024 / HDown) * i ) / (i-1);
			/*
			PCLK1 = PCLK / HPeriod;
			dPrintf("\n PCLK1 = PCLK / HPeriod :: %ld", PCLK1 );
			PCLK1 *= (Divider+1);
			dPrintf("\n PCLK1 *= (Divider+1) :: %ld", PCLK1 );
			PCLK1 /= HDown;
			dPrintf("\n PCLK1 /= HDown :: %ld", PCLK1 );
			PCLK1 *= 1024;
			dPrintf("\n PCLK1 *= 1024 :: %ld", PCLK1 );
			PCLK1 = (PCLK1 * i) / (i-1);
			*/
			PCLK1 = ((((Divider+1) * VFreq * VPN ) / HDown) * 1024 * i ) / (i-1);
			PCLK2 = (DWORD)( H_DE + HActive + 1 ) * ( VPN * 8192L* VFreq * i / VScale ) ;
			dPrintf("\n[%bd] - PCLK1: %ld, PCLK2: %ld", i, PCLK1, PCLK2);
			if ( i == 2 ) {
				PCLKO = 2;
				if ( PCLK1 > PCLK2 ) {
					PCLK = PCLK1;
				}
				else {
					PCLK = PCLK2;
				}
			}
			else {
				if ( PCLK1 > PCLK2 ) {
					if ( PCLK > PCLK1 )	{
						PCLK = PCLK1;
						PCLKO = i;
					}
				}
				else {
					if ( PCLK > PCLK2 )	{
						PCLK = PCLK2;
						PCLKO = i;
					}
				}
			}
		}
		PclkoSetDiv(PCLKO-1);
		if(mode>=5 && mode <= 8)	//640x480@60 
			PclkSetPolarity(0);	//normal
		else
			PclkSetPolarity(1);	//invert


		dPrintf("\nMinimum PCLK is %ld at PCLKO: %bd", PCLK, PCLKO );
		PCLK = PCLK + 4000000L;
		dPrintf("\nAdd 2MHz to PCLK is %ld", PCLK );
	
		i = SspllGetPost();
		PCLK = SspllFREQ2FPLL(PCLK, i);

	}
	else {
		//DVI & HDMI
		i = 3;
		{
			PCLK1 = ((((Divider+1) * VFreq * VPN ) / HDown) * 1024 * i ) / (i-1);
			PCLK2 = (DWORD)( H_DE + HActive + 1 ) * ( VPN * 8192L* VFreq * i / VScale ) ;
			dPrintf("\n[%bd] - PCLK1: %ld, PCLK2: %ld", i, PCLK1, PCLK2);
			if ( PCLK1 > PCLK2 ) {
				PCLK = PCLK1;
			}
			else {
				PCLK = PCLK2;
			}
			PCLK += 5000000L;
			if ( PCLK < 108000000L )	
				PCLK = 108000000L;
			else if ( PCLK > 120000000L )
				PCLK = 120000000L;
		}
		dPrintf("\nFound PCLK is %ld", PCLK, PCLKO );
		WriteTW88Page(0 );
		if ( PCLK == 108000000L )
			PCLK = 0x20000L;
		else {
			i = SspllGetPost();
			PCLK = SspllFREQ2FPLL(PCLK, i);
		}
	}


	SspllSetFreqReg(PCLK); 	
	//WriteTW88Page(page );
#endif //..PANEL_AUO_B133EW01
}
#endif


#ifdef SUPPORT_COMPONENT
//-----------------------------------------------------------------------------
/**
* convert the component mode to HW mode.
*
* SW and HW use a different mode value.
* ISR will check the HW mode value to check the SYNC change.
*
* Or, just read REG1C1[2:0].
*/
static BYTE ConvertComponentMode2HW(BYTE mode)
{
	BYTE new_mode;
	switch(mode) {
	case 0: new_mode = mode;	break;	//480i
	case 1:	new_mode = mode;	break;	//576i
	case 2:	new_mode = mode;	break;	//480p
	case 3:	new_mode = mode;	break;	//576p
	case 4:	new_mode = 4;		break;	//1080i25->1080i
	case 5:	new_mode = 4;		break;	//1080i30->1080i
	case 6:	new_mode = 5;		break;	//720p50->720p
	case 7:	new_mode = 5;		break;	//720p60->720p
	case 8:	new_mode = 6;  		break;	//1080p50->1080p
	case 9:	new_mode = 6;		break;	//1080p60->1080p
	default: new_mode = 7;		break;	//UNKNOWN->non of above
	}
	return new_mode;
}
#endif




//-----------------------------------------------------------------------------
/**
* check and set the componnent
*
* oldname: BYTE CheckAndSetYPBPR( void )
* @return
*	success	:ERR_SUCCESS
*	fail	:ERR_FAIL
*/

#if defined(SUPPORT_COMPONENT)
#define COMP_OVER_SCAN
BYTE CheckAndSetComponent( void )
{
	BYTE	i,j;
	BYTE	mode, modeNew;
	BYTE ret;

	BYTE bTemp;
	WORD wTemp;
	WORD hStart,vStart;
	WORD hActive,vActive;

	struct COMP_VIDEO_TIME_TABLE_s *pTimeTable;


	DECLARE_LOCAL_page
	ReadTW88Page(page);

	Input_aRGBMode = 0;		//BK111012

	for(i=0; i < 10; i++) {
		for(j=0; j < 10; j++) {
			if(MeasStartMeasure()) {
				WriteTW88Page(page );
				return ERR_FAIL;
			}
			// find input mode from Vfreq and VPeriod
			mode = FindInputModeCOMP();	
			if(mode != 0xFF)
				break;
		}	
		if(mode==0xFF) {
			WriteTW88Page(page );
			return ERR_FAIL;
		}

		pTimeTable = &TW8836_COMP_TABLE[mode];

		aRGB_SetLLPLLControl(0xF2);	// POST[7:6]= 3 -> div 1, VCO: 40~216, Charge Pump: 5uA
		ret = aRGB_LLPLLUpdateDivider(pTimeTable->hTotal - 1, 1, 0 );
		if(ret==ERR_FAIL) {
			WriteTW88Page(page );
			return ERR_FAIL;
		}		

		// find input mode and compare it is same or not
		modeNew = FindInputModeCOMP();	
		if(mode==modeNew)
			break;
		//retry..
	}

	dPrintf("\nFind COMP  %dx",pTimeTable->hActive);
	if(pTimeTable->pol & INTERLACED)	
		dPrintf("%d %s", pTimeTable->vActive*2,"I");
	else
		dPrintf("%d %s", pTimeTable->vActive,"P");
	dPrintf("@%bd",pTimeTable->vFreq);


	Input_aRGBMode = mode + EE_YUVDATA_START;
	InputSubMode = ConvertComponentMode2HW(mode);

	InitComponentReg(mode);

	aRGB_SetClampModeHSyncEdge(ON);
	aRGB_SetClampPosition(pTimeTable->ClampPos);

	MeasSetErrTolerance(4);							//tolerance set to 32
	MeasEnableChangedDetection(ON);					// set EN. Changed Detection

	hStart  = pTimeTable->hStart;
	hActive = pTimeTable->hActive;
	vStart  = pTimeTable->vBPorch + pTimeTable->vSync;
	vActive = pTimeTable->vActive;
#ifdef COMP_OVER_SCAN
	hStart  += pTimeTable->hOverScan;
	hActive -= pTimeTable->hOverScan*2;
	vStart  += pTimeTable->vOverScan;
	vActive -= pTimeTable->vOverScan*2;
#endif
	InputSetCrop(hStart, vStart, hActive, vActive);	
	if(mode==MEAS_COMP_MODE_720P60)
		InputSetFieldPolarity(0);
	else
		InputSetFieldPolarity(1);
	if(pTimeTable->pol & INTERLACED)
		WriteTW88(REG041, ReadTW88(REG041) & ~0x20);

	ScalerSetHScale(hActive);
	ScalerSetVScale(vActive);

	//hDE
	bTemp = ScalerReadLineBufferDelay() + 32;
	ScalerWriteHDEReg(bTemp);
	//vDE
	wTemp = (DWORD)vStart * PANEL_V / vActive;
	ScalerWriteVDEReg(wTemp);

	//===== Free Run settings ==========
	wTemp=ScalerCalcFreerunHtotal();
	ScalerWriteFreerunHtotal(wTemp);

	wTemp = ScalerCalcFreerunVtotal();
	ScalerWriteFreerunVtotal(wTemp);

	ScalerSetFreerunAutoManual(ON,OFF);
	ScalerSetMuteAutoManual(ON,OFF);

#if defined(PANEL_AUO_B133EW01)
#else
	SspllSetFreqReg(0x20000);	//108MHz. Where is a POST value ?
#endif
	YUV_PrepareInfoString(mode);


	WriteTW88Page(page );
	return ERR_SUCCESS;
}
#endif

#if 0 //defined(SUPPORT_COMP) ...not yet working..
//BYTE CheckAndSetComponent( void )
//{
//	BYTE	i;
//	BYTE	mode, new_mode;
//	BYTE ret;
//	WORD temp16;
//	DWORD temp32;
//	volatile BYTE InputStatus;
//
//	DECLARE_LOCAL_page
//	ReadTW88Page(page);
//
//	Input_aRGBMode = 0;		//BK111012
//
//	//wait..
//	for(i=0; i < 2; i++) {
//		ret=MeasStartMeasure();
//		if(ret==ERR_SUCCESS)
//			break;
//		delay1ms(10);
//	}	
//	if(ret)	{
//#ifdef DEBUG_COMP
//		Printf("\nCheckAndSetCOMP fail 1");
//#endif
//		WriteTW88Page(page);
//		return ERR_FAIL;
//	}
//
//	//Init LLPLL.
//	//	if input changes from 720P to low resulution, vtotal has a small value.
//	WriteTW88(REG1CD, ReadTW88(REG1CD) | 0x01);
//
//
//	//Find input mode
//	//	use vFreq and vPeriod.
//	mode = FindInputModeCOMP();	
//	if(mode==0) {
//#ifdef DEBUG_COMP
//		Printf("\nCheckAndSetCOMP fail 2. No proper mode");
//#endif
//		WriteTW88Page(page );
//		return ERR_FAIL;
//	}
//
//	//
//	//set LLPLL	& wait
//	//
//	//BKFYI. aRGB_LLPLLUpdateDivider(, 1,) has a MeasStartMeasure().
//	//
//	aRGB_SetLLPLLControl(0xF2);	// POST[7:6]= 3 -> div 1, VCO: 40~216, Charge Pump: 5uA
//	ret = aRGB_LLPLLUpdateDivider(YUVDividerPLL[ mode-1 ] - 1, 1, 0 );
//	if(ret==ERR_FAIL) {
//		WriteTW88Page(page );
//		return ERR_FAIL;
//	}		
//	//LLPLL needs a time until it becomes a stable state.
//	//TW8836 needs 110ms delay to get the correct vPol on PC.	How about COMP ?
//	//wait a detection flag.
//	for(i=0; i < 50; i++) {
//		InputStatus = ReadTW88(REG1C1);
//		if(InputStatus & 0x08) {
//			if((InputStatus & 0x07) != 0x07)
//				break;
//		}
//		delay1ms(10);
//	}
//#ifdef DEBUG_COMP
//	Printf("\nstep %bd mode:%bx",i, InputStatus & 0x0F); //
//#endif
//
//
//	//find input mode and compare it is same or not.
//	//if mode!=new_mode, just use mode..
//	new_mode = FindInputModeCOMP();	
//	if(mode!=new_mode) {
//		Printf("\nWARNING mode curr:%bd new:%bd",mode, new_mode);
//	}
//
//	//now adjust mode.
//	mode--;
//	dPrintf("\nFind YUV mode: %bd %dx%d@", mode, YUVCropH[mode], YUVCropV[mode] );
//
//	Input_aRGBMode = mode + EE_YUVDATA_START;
//	InputSubMode = ConvertComponentMode2HW(mode);
//
//	InitComponentReg(mode);
//
//	aRGB_SetClampModeHSyncEdge(ON);					//R1D4[5]
//	aRGB_SetClampPosition(YUVClampPos[mode]);
//
//	MeasSetErrTolerance(4);							//tolerance set to 32
//	MeasEnableChangedDetection(ON);					// set EN. Changed Detection
//
//
//	//BKTODO130208: assign correct aRGB_output_polarity and remove this routine.
//	//check VPulse & adjust polarity
//	temp16 = MeasGetVSyncRiseToFallWidth();
//	if(temp16 > YUVDisplayV[mode]) {
//		dPrintf("\nVSyncWidth:%d", temp16);
//		WriteTW88Page(PAGE1_VADC );		
//		WriteTW88(REG1CC, ReadTW88(REG1CC) | 0x02);	
//
//		MeasStartMeasure();
//		temp16 = MeasGetVSyncRiseToFallWidth();
//		dPrintf("=>%d", temp16);		
//	}
//
//	//TW8836 needs input crop value on 480i & 576i.
//	InputSetCrop(YUVStartH[mode], MYStartV[mode], YUVDisplayH[mode], YUVCropV[mode]);
//
//	YUVSetOutput(mode);
//
//	SspllSetFreqReg(0x20000);	//108MHz. Where is a POST value ?
//	YUV_PrepareInfoString(mode);
//
//	//BKTODO130208. It is a debug routine. Please remove..
//	//
//	//check HStart
//	MeasStartMeasure();
//	temp16 = MeasGetHActive2();
//	dPrintf("\n**measure:%d",temp16);
//	if(mode < 4) { //SDTV or EDTV
//	}
//	else {			//HDTV
//		temp16 = MYStartH[mode] + temp16;
//	}
//	temp16 -=16;	//HWidth
//	temp16 += 3;
//	temp16 += ((YUVCropH[mode] - YUVDisplayH[mode]) / 2);
//	dPrintf("\n**HStart:%d suggest:%d",YUVStartH[mode],temp16);	
//
//	//
//	//check VDE
//	if(mode < 4) { //SDTV or EDTV
//		temp16 = MeasGetVActive2();
//		dPrintf("\n**measure:%d",temp16);
//		//temp16 += 0.5;
//		temp16 -= MeasGetVSyncRiseToFallWidth();  //if use faling.
//		temp16 += ((YUVCropV[mode] - YUVDisplayV[mode]) / 2);
//		temp32 = temp16;
//		temp32 *= PANEL_V;
//		temp32 /= YUVDisplayV[mode];
//		dPrintf("\n**VDE:%bd suggest:%d",YUVOffsetV[mode],(WORD)temp32);
//			
//		temp16 = MeasGetVActive2();
//		temp16 += 1;	//NOTE
//		temp16 -= MeasGetVSyncRiseToFallWidth(); //if use faling.
//		temp16 += ((YUVCropV[mode] - YUVDisplayV[mode]) / 2);
//		temp32 = temp16;
//		temp32 *= PANEL_V;
//		temp32 /= YUVDisplayV[mode];
//		dPrintf("~%d",(WORD)temp32);
//	}
//	else {			//HDTV
//		temp16 = MYStartV[mode];
//		//temp16 += 0.5;
//		temp16 -= MeasGetVSyncRiseToFallWidth();
//		temp16 += ((YUVCropV[mode] - YUVDisplayV[mode]) / 2);
//		temp32 = temp16;
//		temp32 *= PANEL_V;
//		temp32 /= YUVDisplayV[mode];
//		dPrintf("\n**VDE:%bd suggest:%d",YUVOffsetV[mode],(WORD)temp32);	
//		temp16 = MYStartV[mode];
//		temp16 += 1;   	//NOTE
//		temp16 -= MeasGetVSyncRiseToFallWidth();
//		temp16 += ((YUVCropV[mode] - YUVDisplayV[mode]) / 2);
//		temp32 = temp16;
//		temp32 *= PANEL_V;
//		temp32 /= YUVDisplayV[mode];
//		dPrintf("~%d",(WORD)temp32);	
//	}
//	
//
//	//TW8836 uses "1" on 480i & 576i also.
//	InputSetFieldPolarity(1);
//
//	WriteTW88Page(page );
//	return ERR_SUCCESS;
//}
#endif


#undef CHECK_USEDTIME
#ifdef SUPPORT_PC
//-----------------------------------------------------------------------------
/**
* check and set the PC
*
* calls from ChangePC and Interrupt Handler
* @return
*	0:ERR_SUCCESS
*	1:ERR_FAIL
* @see ChangePC
* @see CheckAndSetInput
* @see NoSignalTask
*/
#if 0
//BYTE CheckAndSetPC_________________OLD(void)
//{
//	BYTE mode,new_mode;
//  
//#ifdef CHECK_USEDTIME
//	DWORD UsedTime;
//#endif
//	BYTE value;
//	BYTE value1;
//	WORD new_VTotal;
//	WORD wTemp;
//	BYTE ret;
//	DECLARE_LOCAL_page
//
//
//	ReadTW88Page(page);
//#ifdef CHECK_USEDTIME
//	UsedTime = SystemClock;
//#endif
//	Input_aRGBMode = 0;
//	InputSubMode = Input_aRGBMode;
//
//	do {
//		mode = PCCheckMode();
//		if(mode==0) {
//			WriteTW88Page(page );
//			return ERR_FAIL;
//		}
//
//		//
//		//set LLPLL	& wait
//		//
//		//BK110927
//		//BK110928 assume LoopGain:2.
//		//LLPLL divider:PCMDATA[ mode ].htotal - 1
//		//ControlValue, 0xF2.  POST[7:6]= 3 -> div 1, VCO: 40~216, Charge Pump: 5uA
//		//LLPLL init: ON
//		//Wait delay for WaitStableLLPLL: 40ms
//		//LLPLL Control.
//		aRGB_SetLLPLLControl(0xF2);	// POST[7:6]= 3 -> div 1, VCO: 40~216, Charge Pump: 5uA
//		ret = aRGB_LLPLLUpdateDivider(PCMDATA[ mode ].htotal - 1, 1, 40 );
//		if(ret==ERR_FAIL) {
//			WriteTW88Page(page );
//			return ERR_FAIL;
//		}
//		//TW8836 needs 110ms delay to get the correct vPol.
//		delay1ms(120);
//		//WaitStableLLPLL(120);
// 		aRGB_SetPolarity(0);
//
//		//check Phase EEPROM.
//		value = GetPhaseEE(mode);
//		if(value!=0xFF) {
//			dPrintf("\nuse Phase 0x%bx",value);
//			//we read first. because update routine can make a shaking.
//			value1=aRGB_GetPhase();
//			if(value != value1) {
//				dPrintf("  update from 0x%bx",value1);
//				aRGB_SetPhase(value, 0);	//BKTODO? Why it does not have a init ?
//				if(WaitStableLLPLL(0)) {
//					WriteTW88Page(page );
//					return ERR_FAIL;
//				}
//			}
//			else {
//				WaitStableLLPLL(0); //BK110830
//				MeasCheckVPulse();
//			}
//		}
//		else 
//		{
//			AutoTunePhase();
//			value=aRGB_GetPhase();
//			dPrintf("\ncalculate Phase %bx",value);
//			SavePhaseEE(mode,value);
//
//			if(WaitStableLLPLL(0)) {
//				WriteTW88Page(page );
//				return ERR_FAIL;
//			}
//		}
//		//adjust polarity again and update all measured value
//		aRGB_SetPolarity(0);
//		MeasStartMeasure();
//
//		MeasCheckVPulse(); //BK130122
//
//
//		//use measured value.  
//		MeasVLen = MeasGetVActive( &MeasVStart );				//v_active_start v_active_perios
//		MeasHLen = MeasGetHActive( &MeasHStart );				//h_active_start h_active_perios
//
//		dPrintf("\nMeasure Value Start %dx%d Len %dx%d", MeasHStart,MeasVStart, MeasHLen,MeasVLen);
//
//		if ( MeasVLen < PCMDATA[ mode ].van ) {			// use table
//			MeasVStart = PCMDATA[mode].vstart;
//			MeasVLen = PCMDATA[mode].van;
//			dPrintf("->VS:%d VLen:%d",MeasVStart,MeasVLen);
//		}
//		if ( MeasHLen < PCMDATA[ mode ].han ) {			// use table
//			MeasHStart = PCMDATA[mode].hstart;
//			MeasHLen = PCMDATA[mode].han;
//			dPrintf("->HS:%d HLen:%d",MeasHStart,MeasHLen);
//		}
//
//		PCSetInputCrop(mode);
//		PCSetOutput(mode);
//
//		new_mode = FindInputModePC(&new_VTotal);
//	} while(mode != new_mode);
//
//	Input_aRGBMode = mode;
//	InputSubMode = Input_aRGBMode;
//
//	//EE
//	//PCLKAdjust();
//	AdjustPixelClk(0, mode); //BK120117 need a divider value
//
//	//adjust PCPixelClock here.
//	//If R1C4[], measure block use a wrong value.
//	wTemp = PCMDATA[ mode ].htotal - 1;
//	dPrintf("\nPixelClk %d",wTemp);
//	wTemp += GetPixelClkEE(mode);	//0..100. default:50
//	wTemp -= 50;
//	dPrintf("->%d EEPROM:%bd",wTemp,GetPixelClkEE(mode));
//
//	aRGB_LLPLLUpdateDivider(wTemp, OFF, 0);	//without init. Do you need aRGB_SetLLPLLControl(0xF2) ?
//	
//	MeasSetErrTolerance(4);						//tolerance set to 32
//	MeasEnableChangedDetection(ON);				//set EN. Changed Detection
//	
//	PC_PrepareInfoString(mode);
//
//#ifdef CHECK_USEDTIME
//	UsedTime = SystemClock - UsedTime;
//	Printf("\nUsedTime:%ld.%ldsec", UsedTime/100, UsedTime%100 );
//#endif
//			
//	WriteTW88Page(page );
//  
//	return ERR_SUCCESS;
//}
#endif
//new 130206
BYTE CheckAndSetPC(void)
{
	BYTE mode, old_mode;
	BYTE i;
	BYTE bTemp;
//	BYTE pol;

#ifdef CHECK_USEDTIME
	DWORD UsedTime;
#endif

	BYTE value;
	BYTE value1;
//	WORD new_VTotal;
	WORD wTemp;
	BYTE ret;
	DECLARE_LOCAL_page
	volatile BYTE InputStatus; //REG1C1

	ReadTW88Page(page);

#ifdef CHECK_USEDTIME
	UsedTime = SystemClock;
#endif

	Input_aRGBMode = 0;
	InputSubMode = Input_aRGBMode;

	//check signal. if fail, give up.
	for (i = 0; i < 2; i++)
	{
		ret = MeasStartMeasure();
		if (ret == ERR_SUCCESS)
			break;
		delay1ms(10);
	}	
	
	if (ret)
	{
#ifdef DEBUG_PC
		Printf("\nCheckAndSetPC fail 1");
#endif
		WriteTW88Page(page);
		return 1; //ERR_FAIL;
	}

	//find PC mode
	old_mode = 0;
	while (1)
	{
		mode = FindInputModePC(&wTemp/*&vTotal*/);
		if (mode == 0)
		{
#ifdef DEBUG_PC
			Printf("\nCheckAndSetPC fail 2. No proper mode");
#endif
			WriteTW88Page(page);
			return 2; //ERR_FAIL;
		}
		if (old_mode == mode)
			break;
		old_mode = mode;

		//
		//set LLPLL	& wait
		//
		aRGB_SetLLPLLControl(0xF2);	// POST[7:6]= 3 -> div 1, VCO: 40~216, Charge Pump: 5uA
		ret = aRGB_LLPLLUpdateDivider(PCMDATA[mode].hTotal - 1, 1, 40 );
		if (ret == ERR_FAIL)
		{
#ifdef DEBUG_PC
			Printf("\nCheckAndSetPC fail 3. No stable LLPLL");
#endif
			WriteTW88Page(page );
			return 3; //ERR_FAIL;
		}
		//LLPLL needs a time until it becomes a stable state.
		//TW8836 needs 110ms delay to get the correct vPol.
		delay1ms(120);

		//wait a detection flag.
		for (i = 0; i < 50; i++)
		{
			InputStatus = ReadTW88(REG1C1);
			if ((InputStatus & 0x30) == 0x30)
				break;
			delay1ms(10);
		}
		//note. The detected polarity can be incorrect.
		aRGB_Set_vSyncOutPolarity(1 /*1 means PC */, InputStatus & 0x80);
		LLPLL_setInputPolarity(1 /*1 means PC */, InputStatus & 0x40);

		//update Phase.
		value = GetPhaseEE(mode);
		if (value == 0xFF)
		{
			//No previous data. We need a AutoTunePhase.
			AutoTunePhase();
			value=aRGB_GetPhase();
#ifdef DEBUG_PC
			dPrintf("\nAutoTune Phase 0x%bx",value);
#endif
			SavePhaseEE(mode,value);
		}
		else
		{
#ifdef DEBUG_PC
			dPrintf("\nuse EE Phase 0x%bx",value);
#endif
			//we read first, because update routine can make a shaking.
			value1=aRGB_GetPhase();
			if (value != value1)
			{
#ifdef DEBUG_PC
				dPrintf("  update from 0x%bx",value1);
#endif
				aRGB_SetPhase(value, 0);	//BKTODO? Why it does not have a init ?
			}
		}
		ret = WaitStableLLPLL(0);
		if (ret)
		{
			ePrintf("\nWARNING WaitStableLLPLL faile at %d",__LINE__);
		}

		//adjust polarity again
		for (i = 0; i < 50; i++)
		{
			InputStatus = ReadTW88(REG1C1);
			if ((InputStatus & 0x30) == 0x30)
				break;
			delay1ms(10);
		}
		aRGB_Set_vSyncOutPolarity(1 /*1 means PC */, InputStatus & 0x80);
		LLPLL_setInputPolarity(1 /*1 means PC */, InputStatus & 0x40);

		//reflash measure value
		MeasStartMeasure();
	}

	//check polarity.
	InputStatus = ReadTW88(REG1C1);
	if ((PCMDATA[mode].syncpol & 0xC0) != (InputStatus & 0xC0))
	{
		//incorrect polarity.
		Printf("\nmode:%bd hPol %s->%s vPol %s->%s",mode,
			PCMDATA[ mode ].syncpol & HPOL_P ? "P" : "N",
			InputStatus & HPOL_P 			 ? "P" : "N",	 
			PCMDATA[ mode ].syncpol & VPOL_P ? "P" : "N",
			InputStatus & VPOL_P 			 ? "P" : "N");	 
	}

	//final check.
#if 0
	old_mode = mode;
	mode = FindInputModePC(&new_VTotal);
	if(old_mode != mode) {
		Printf("\nWARNING mode curr:%bd new:%bd",old_mode, mode);
		mode = old_mode;
	}
#endif

	PC_SetScaler(mode);

	Input_aRGBMode = mode;
	InputSubMode = Input_aRGBMode;

	AdjustPixelClk(0, mode); //BK120117 need a divider value

	//update EEPROM pixel clock.
	//Note: It is not a pixel clock. It is a LLPLL divider.
	//      
	bTemp = GetPixelClkEE(mode);  //value 0..100
	if (bTemp != 50)
	{
		wTemp = PCMDATA[ mode ].hTotal - 1;
#ifdef DEBUG_PC
		Printf("\nEEPROM has a PixelClock value %bd",bTemp);
		Printf("\tchange LLPLL %d",wTemp);
#endif
		wTemp += bTemp;
		wTemp -= 50;
#ifdef DEBUG_PC
		Printf("->%d",wTemp);
#endif
		aRGB_LLPLLUpdateDivider(wTemp, OFF, 0);	//without init.
	}

	MeasSetErrTolerance(4);						//tolerance set to 32
	MeasEnableChangedDetection(ON);				//set EN. Changed Detection
	
	PC_PrepareInfoString(mode);

#ifdef CHECK_USEDTIME
	UsedTime = SystemClock - UsedTime;
	Printf("\nUsedTime:%ld.%ldsec", UsedTime/100, UsedTime%100 );
#endif
			
	WriteTW88Page(page );

	return ERR_SUCCESS;
}
#endif

//=============================================================================
//setup menu interface
//=============================================================================
#ifdef SUPPORT_PC
//-----------------------------------------------------------------------------
/**
*
* extern
*	RGB_HSTART
*/
void PCRestoreH(void)
{
	WORD hstart;
	hstart = RGB_HSTART;

	if(Input_aRGBMode==0) {
		//?Freerun mode
		return;
	}
	//adjust EEPROM. 0..100. base 50. reversed value.
	hstart += 50;
	hstart -= GetHActiveEE(Input_aRGBMode); //PcBasePosH;
	InputSetHStart(hstart);
}
//-----------------------------------------------------------------------------
/**
*
* extern
*	RGB_VDE
*/
void PCRestoreV(void)
{
	WORD temp16;
	temp16 = RGB_VDE;
	dPrintf("\n\tV-DE start = %d", temp16);

	if(Input_aRGBMode==0) {
		//?Freerun mode
		return;
	}

	temp16 += GetVBackPorchEE(Input_aRGBMode);
	temp16 -= 50;
	dPrintf("=> %d", temp16);
	ScalerWriteVDEReg((BYTE)temp16);
}
//-----------------------------------------------------------------------------
/**
*
* extern
*	Input_aRGBMode
*/
void PCResetCurrEEPROMMode(void)
{
	BYTE temp;
	temp = GetPixelClkEE(Input_aRGBMode);
	if(temp!=50)
		SavePixelClkEE(Input_aRGBMode,50);
	temp = GetPhaseEE(Input_aRGBMode);
	if(temp != 0xFF)
		SavePhaseEE(Input_aRGBMode,0xFF);	
	temp = GetHActiveEE(Input_aRGBMode);
	if(temp!=50)
		SaveHActiveEE(Input_aRGBMode,50);
	temp = GetVBackPorchEE(Input_aRGBMode);
	if(temp!=50)
		SaveVBackPorchEE(Input_aRGBMode,50);	
}
#endif

#ifdef SUPPORT_PC
/**
* prepare Info String for PC
*/
void PC_PrepareInfoString(BYTE mode)
{
	BYTE itoa_buff[5];					

	//prepare info. ex: "PC 1024x768 60Hz"
	FOsdSetInputMainString2FOsdMsgBuff();	//GetInputMainString(FOsdMsgBuff);										 	
	TWstrcat(FOsdMsgBuff," ");
	TWitoa(PCMDATA[mode].hActive, itoa_buff);
	TWstrcat(FOsdMsgBuff,itoa_buff);
	TWstrcat(FOsdMsgBuff,"x");
	TWitoa(PCMDATA[mode].vActive, itoa_buff);
	TWstrcat(FOsdMsgBuff,itoa_buff);
	TWstrcat(FOsdMsgBuff," ");
	TWitoa(PCMDATA[mode].vFreq, itoa_buff);
	TWstrcat(FOsdMsgBuff,itoa_buff);
	TWstrcat(FOsdMsgBuff,"Hz");

//BK110811	FOsdCopyMsgBuff2Osdram(OFF);
}
#endif

//related with YUVCropH[] array.
#ifdef SUPPORT_COMPONENT
/**
* prepare Info String for Component
*/
void YUV_PrepareInfoString(BYTE mode)
{
	//prepare info. ex: "Component 1080p 60Hz"
	FOsdSetInputMainString2FOsdMsgBuff();	//GetInputMainString(FOsdMsgBuff);										 	
	TWstrcat(FOsdMsgBuff," ");
	switch(mode) {
	case 0:	TWstrcat(FOsdMsgBuff,"480i");	break;
	case 1:	TWstrcat(FOsdMsgBuff,"576i");	break;
	case 2:	TWstrcat(FOsdMsgBuff,"480p");	break;
	case 3:	TWstrcat(FOsdMsgBuff,"576p");	break;
	case 4:	TWstrcat(FOsdMsgBuff,"1080i 50Hz");	break;
	case 5:	TWstrcat(FOsdMsgBuff,"1080i 60Hz");	break;
	case 6:	TWstrcat(FOsdMsgBuff,"720p 50Hz");	break;
	case 7:	TWstrcat(FOsdMsgBuff,"720p 60Hz");	break;
	case 8:	TWstrcat(FOsdMsgBuff,"1080p 50Hz");	break;
	case 9:	TWstrcat(FOsdMsgBuff,"1080p 60Hz");	break;
	default:TWstrcat(FOsdMsgBuff,"Unknown");	break;
	}
//BK110811	FOsdCopyMsgBuff2Osdram(OFF);
}
#endif


//=============================================================================
// Change to COMPONENT (YPBPR)
//=============================================================================


#ifdef SUPPORT_COMPONENT
//-----------------------------------------------------------------------------
/**
* Change to Component
*
* @return
*	- 0: success
*	- 1: No Update happen
*	- 2: No Signal or unknown video sidnal.
*/
BYTE ChangeComponent( void )
{
	BYTE ret;

	if ( InputMain == INPUT_COMP ) {
		dPrintf("\nSkip ChangeComponent");
		return(1);
	}
		
	InputMain = INPUT_COMP;
	InputSubMode = 7; //N/A. Note:7 is a correct value.

 	if(GetInputMainEE() != InputMain)
		SaveInputMainEE( InputMain );

	//----------------
	// initialize video input
	InitInputAsDefault();

	//
	// Check and Set aRGB,mesaure,Scaler for Component input
	//
	ret = CheckAndSetComponent();
	if(ret==ERR_SUCCESS) {
		//success
		VInput_enableOutput(0);
		return 0;
	}
	//------------------
	// NO SIGNAL

	//start recover & force some test image.
	VInput_gotoFreerun(0);

	return(2);  //fail
}
#endif
//=============================================================================
// Change to PC
//=============================================================================


//-----------------------------------------------------------------------------
//BYTE last_position_h;
//BYTE last_position_v;
//BYTE temp_position_h;
//BYTE temp_position_v;
#ifdef SUPPORT_PC
//-----------------------------------------------------------------------------
/**
* Change to PC
*
* @return
*	- 0: success
*	- 1: No Update happen
*	- 2: No Signal or unknown video sidnal.
*/
BYTE ChangePC( void )
{
	BYTE ret;

	if ( InputMain == INPUT_PC ) {
		dPrintf("\nSkip ChangePC");
		return(1);
	}

	InputMain = INPUT_PC;
	InputSubMode = 0; //N/A....maybe it same as Input_aRGBMode ? why use diff variable ?

	if(GetInputMainEE() != InputMain)
		SaveInputMainEE( InputMain );

	//----------------
	// initialize video input
	InitInputAsDefault();

	//
	// Check and Set aRGB,mesaure,Scaler for Analog PC input
	//
	ret = CheckAndSetPC();
	if(ret==ERR_SUCCESS) {
		//success
		VInput_enableOutput(0);
		return 0;
	}

	//------------------
	// NO SIGNAL
	// Prepare NoSignal Task...

	//free run		
	//start recover & force some test image.
	VInput_gotoFreerun(0);

	return 2;	//fail..
}
#endif


//check routine
#if defined(SUPPORT_COMP) || defined(SUPPORT_PC)
#if 0
BYTE Check_aRGB(BYTE mode)
{
	//check AnaliogMux

	//check Video Status

	//check aRGB input status
	InputStatus = ReadTW88(REG1C1);
	if(InputMain == INPUT_COMP) {
		if((InputStatus & 0x04) == 0) {
			Printf("CSync");
			return ERR_ERROR;
		}
		if(mode != (InputStatus & 0x07)) {
			Printf("mode");
			return ERR_ERROR;
		}

	}
	else {
		if((InputStatus & 0x30) != 0x30) {
			Printf("Sync");
			return ERR_ERROR;
		}
	}
	
	//check aRGB path

	//check aRGB polarity
	if(InputMain == INPUT_COMP) {
	}
	else {
		pol = 0;
		if(InputStatus & 0x80) pol |= 0x01;	//vPol
		if(InputStatus & 0x40) pol |= 0x10;	//hPol
		if(PCMData[mode].pol != pol) {
			Puts("Input Pol");
			return ERR_ERROR;
		}
		if(hPol != LLPLL_InputPol) {
			Puts("LLPLL_ipol");
			return ERR_ERROR	
		}
		if(vPol != Output_vPol) {
			Puts("vPol");
			return ERR_ERROR	
		}
	}
	
	//check meased polarity
	
	
	
		 
	return ERR_SUCCESS;
}
#endif
#endif
