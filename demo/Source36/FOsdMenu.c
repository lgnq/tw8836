/**
 * @file
 * FOsdMenu.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	FontOSD MENU
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
#include "SPI.h"

#include "main.h"
#include "EEPROM.h"
#include "Decoder.h"
#include "InputCtrl.h"
#include "ImageCtrl.h"

#include "SOsd.h"
#include "FOsd.h"

#include "FOsdString.h"
#include "FOsdTable.h"
#include "FOsdDisplay.h"
#include "FOsdMenu.h"

#include "SOsdMenu.h"
//#include "RegMap.h"
#include "BT656.h"


//==========================================
#ifndef SUPPORT_FOSD_MENU
//----------------------------
//Trick for Bank Code Segment
//----------------------------
CODE BYTE DUMMY_FOSD_MENU_CODE;
void Dummy_FosdMenu_func(void)
{
	BYTE temp;
	temp = DUMMY_FOSD_MENU_CODE;
}
#else	//..SUPPORT_FOSD_MENU
//==========================================
/*
*/
//extern DATA	BYTE  DisplayedOSD;

DATA	BYTE	OSDMenuX, OSDMenuY;
DATA	BYTE	OSDLang=0;
DATA	BYTE    OSDMenuLevel;
		bit     OnChangingValue = 0;
DATA	BYTE    OSDNumberString[5];
		bit   	DisplayInputHold = 0;

DATA	WORD	OSDItemValue;
		bit   	I2CAutoIncFlagOn = 0;

BYTE	title_item, cursor_item;
BYTE    cur_osdwin,  val_osdwin=0xff,  val_index=0;


struct MenuType   	CODE *MenuFormat;
struct RegisterInfo CODE *BarRange;

struct DefineMenu 	CODE *MenuTitle;
struct DefineMenu 	CODE *MenuChild;
struct DefineMenu	CODE *MenuStack[4];

BYTE MenuSP=0, MenuIndexStack[4];


CODE struct RegisterInfo UserRange={0,100,50};

CODE struct RegisterInfo OSDMenuYRange= { OSDMENUY_MIN,	OSDMENUY_MAX,	OSDMENUY_50		};				
CODE struct RegisterInfo OSDMenuXRange= { OSDMENUX_MIN, OSDMENUX_MAX,	OSDMENUX_50		};				

CODE struct RegisterInfo PCUserColorTempRange = 	{ 0, 55, 28 };			

CODE struct RegisterInfo PanelContrastRange=		{0,255,128};
//CODE struct RegisterInfo PanelBrightnessRange=		{0,255,128};
//CODE struct RegisterInfo PanelSharpnessRange=		{0,7,3};
//CODE struct RegisterInfo PanelHueRange=			{0,0x3f,0x20};
//CODE struct RegisterInfo PanelHueRange=				{-31,31,0};
//CODE struct RegisterInfo PanelSaturationRange=		{0,255,128};
//CODE struct RegisterInfo PanelBacklightRange=		{0,255,128};

//CODE struct RegisterInfo VideoContrastRange=		{0,255,128};
//CODE struct RegisterInfo VideoBrightnessRange=		{-128,127,0};
//CODE struct RegisterInfo VideoSaturationRange=		{0,255,128};
//CODE struct RegisterInfo VideoHueRange=				{-128,127,0};	//-90~90
//CODE struct RegisterInfo VideoSharpnessRange=		{0,15,7};
//CODE struct RegisterInfo DigitalVideoSaturationRange={0, 0x7f, 0x40};


#define OFFSET_contrast 		0
#define OFFSET_bright	 		1
#define OFFSET_staturation	 	2
#define OFFSET_hue		 		3
#define OFFSET_sharpness 		4
#define OFFSET_backlight 		5

#define PC_COLOR_DATA	0x80	// StartAddress of EEPROM for PC Color Temperature
#define PC_USERCOLOR	0x8A


//CODE BYTE Contast_Reg_Addr[]={0x11, // Composite
//							  0x11, // Svideo
//	                          0x3b, // Component
//	                          0x71, // DTV
//	                          0x11, // TV
//	                          0x3b  // Scart
//	};

#ifdef WIDE_SCREEN
extern BYTE WideScreenMode;
#define MAXWIDESCREEN 4
#endif
#ifdef SUPPORT_COMPONENT
extern  BYTE  ComponentMode;
#endif

#define HOUR			1
#define MIN				2

/////////////////////////////////////////////////////////////////////////////
// Mapping( int fromValue, CRegInfo fromRange,
//                                           int * toValue, CRegInfo toRange )
// Purpose: Map a value in certain range to a value in another range
// Input:   int fromValue - value to be mapped from
//          CRegInfo fromRange - range of value mapping from
//          CRegInfo toRange   - range of value mapping to
// Output:  int * toValue - mapped value
// Return:  Fail if error in parameter, else Success
// Comment: No range checking is performed here. Assume parameters are in
//          valid ranges.
//          The mapping function does not assume default is always the mid
//          point of the whole range. It only assumes default values of the
//          two ranges correspond to each other.
//          
//          The mapping formula is:
//
//            For fromRange.Min() <= fromValue <= fromRange.Default():
//
//				(fromValue -fromRange.Min())* (toRange.Default() - toRange.Min())
//				-------------------------------------------------------------------- + toRange.Min()
//					fromRange.Default() - fromRange.Min()
//
//			  For fromRange.Default() < fromValue <= fromRange.Max():
//
//				(fromValue - fromRange.Default()) * (toRange.Max() - toRange.Default())
//				--------------------------------------------------------------------- + toRange.Default()
//		             fromRange.Max() - fromRange.Default()
////
////////////////////////////////////////////////////////////////////////////
BYTE Mapping1( int fromValue, CODE_P struct RegisterInfo *fromRange,
                                 int * toValue, CODE_P struct RegisterInfo *toRange ){

	// calculate intermediate values
	int a;
	int b;

	// perform mapping
	if ( fromValue <= fromRange->Default ) {
		a = toRange->Default - toRange->Min;
		b = fromRange->Default - fromRange->Min;
		// prevent divide by zero
		if( b==0 )		return (FALSE);
		*toValue = (int) ( (DWORD)fromValue- (DWORD)fromRange->Min ) * a / b 
						+(DWORD)toRange->Min;
	}
	else {
		a = toRange->Max - toRange->Default;
		b = fromRange->Max - fromRange->Default;
		// prevent divide by zero
		if( b==0 )		return (FALSE);
        *toValue = (int) ( (DWORD)fromValue - (DWORD)fromRange->Default ) * a / b
                       + (DWORD)toRange->Default;
	}

	#ifdef DEBUG_OSD
	dPrintf("\r\n++(Mapping1)%d(%d-%d-%d)", (WORD)fromValue, (WORD)fromRange->Min, (WORD)fromRange->Default, (WORD)fromRange->Max );
	dPrintf("->%d(%d-%d)", (WORD)*toValue, (WORD)toRange->Min, (WORD)toRange->Max);
	#endif
	
	return ( TRUE );
   
}

//=============================================================================
//		BYTE GetVInputStdInx(void)		                                               
//=============================================================================
BYTE GetVInputStdInx(void)
{
	BYTE i, std;

	std = DecoderReadVInputSTD();

#if 0 //BKFYI:I am not using IVF,IHF
	switch( std ) {
	case NTSC4:
	case PALM:
	case PAL60:
	case NTSC:		IVF = 60;		IHF = 15723;	break;	// 15734

	case SECAM:
	case PALN:
	case PAL:		IVF = 50;		IHF = 15723;	break;  // 15625
	default:		IVF = 0;		IHF = 0;		break;
	}
#endif

	for(i=0; ; i++) {
		if( struct_VInputStd[i].Id ==std )
			return i;
		if( struct_VInputStd[i].Id ==0 )
			break;
	}

	return 0;
}



//==============================================================
//
//  OSD Menu Operation Function
//
//==============================================================
#define MAX_OSDPOSITIONMODE		5
CODE BYTE OSDPOSITIONMODE_TABLE[][2] = { 
	{OSDMENUX_50, OSDMENUY_50},
	{OSDMENUX_MIN,OSDMENUY_MIN},
	{OSDMENUX_MIN,OSDMENUY_MAX},
	{OSDMENUX_MAX,OSDMENUY_MAX},
	{OSDMENUX_MAX,OSDMENUY_MIN},
};

static BYTE GetOSDPositionMode(void)
{
	BYTE val;
	val = GetOSDPositionModeEE();
	dPrintf("OSD Position=%d\r\n",(BYTE)val);
	
	return val;
}
//
//global
//	OSDMenuX
//	OSDMenuY	
static BYTE SetOSDPositionMode( BYTE flag )
{
	int val, addx, addy;
	BYTE i,inc, winno;
	WORD x_start,y_start;

#ifdef ADD_ANALOGPANEL
	if(IsAnalogOn()) return 0;
#endif

	// Get OSD Position Mode
	i = GetOSDPositionMode();
	inc = ( flag==FOSD_UP  ? 1  :  -1  );
	i = i + MAX_OSDPOSITIONMODE + inc;
	i %= MAX_OSDPOSITIONMODE;

	addx = OSDPOSITIONMODE_TABLE[i][0];
	addx -= OSDMenuX;
	OSDMenuX = OSDPOSITIONMODE_TABLE[i][0];

	addy = OSDPOSITIONMODE_TABLE[i][1];
	addy -= OSDMenuY;
	OSDMenuY = OSDPOSITIONMODE_TABLE[i][1];

	//get HStart, VStart for window
	//set HStart, Vstart for window
	for( winno=FOSDMENU_CURSORWIN; winno<=FOSDMENU_TITLEWIN; winno++) {
		//FOsdWinGetX() return the real coordinator value
		//If OSDMenuX use 8 boundary value, we have to add FOsdWinGetX() value later.     
		val = addx;
		val = (BYTE)val<<3;
		x_start = FOsdWinGetX(winno) + val;

		val = addy;
		val = (BYTE)val<<2;
		y_start = FOsdWinGetY(winno) + val;

		FOsdWinScreenXY(winno, x_start, y_start);
	}
	SaveOSDPositionModeEE(i);

	return i;
}


void ResetOSDValue(void)
{
	OSDMenuX = OSDPOSITIONMODE_TABLE[0][0];  // Default
	OSDMenuY = OSDPOSITIONMODE_TABLE[0][1];
	#ifdef SUPPORT_OSDPOSITIONMOVE
	SaveOSDPositionModeEE(0);
	#endif

	SaveOSDLangEE(0);	// Save default Language1
	OSDLang = 0;

	FOsdMenuInit();	
}

BYTE SetLanguage(BYTE flag)
{
	char inc;
	inc = ( flag==FOSD_UP  ? 1  :  -1  );
	OSDLang = OSDLang + MAX_LANG + inc;
	OSDLang %= MAX_LANG ; 

	SaveOSDLangEE(OSDLang);

	//BKFYI: If (OSDLang==OSDLANG_KR) you have to reload FontSet.

	return OSDLang;
}


#define INC_SLEEPTIMER		30
#define	MAX_SLEEPTIMER		150
//extern
//	OSDItemValue
BYTE ChangeSleepTimer( BYTE flag )
{
	int t;

	t = OSDItemValue / INC_SLEEPTIMER * INC_SLEEPTIMER;		//cut edge of remained sleep timer.

	if( flag==FOSD_UP )		t+= INC_SLEEPTIMER;
	else					t-= INC_SLEEPTIMER;

	if( t<0 )				t = 0;
	if( t> MAX_SLEEPTIMER)	t = MAX_SLEEPTIMER;

	OSDItemValue = t;
	return OSDItemValue;
}

//BKTODO:See ImgAdjGetContrastRGB()
//	temp
//	val
static void OSDSetRGBContrast( BYTE temp, BYTE val )
{
	WORD index;
	BYTE i;
	int newval, toValue;

	index = PC_COLOR_DATA + 1 + temp*3;
	for( i=0; i<3; i++)	{
		newval = EE_Read( index+i );
		Mapping1( newval, &UserRange , &toValue, &PCUserColorTempRange);
		WriteTW88Page(PAGE2_RGBLEVEL);
		WriteTW88(REG281+i,   toValue + val );
	}
}

#define MAX_COLORTEMP	4
CODE BYTE PCColorTempRGB[][3]={
	75,		50,		50,				// Mode 1: Warm
	50,		50,		50,				// Mode 2: plain -- COLOR_MODE2
	50,		50,		75,				// Mode 3: Cold
	50,		50,		50				// User mode
};

#define COLOR_MODE2		1
#define COLOR_USER		3

BYTE SetPCColorTemp(BYTE flag)
{
	BYTE val;
	int  regv;
	char inc;

	//OSDItemValue = EE_Read( PC_COLOR_DATA );

	inc = ( flag==FOSD_UP  ? 1  :  -1  );
	OSDItemValue = OSDItemValue + MAX_COLORTEMP + inc;
	OSDItemValue %= MAX_COLORTEMP ; 

	EE_Write( PC_COLOR_DATA, OSDItemValue );
	val = EE_Read(0x10);
	Mapping1( val, &UserRange ,&regv, &PanelContrastRange);
	OSDSetRGBContrast( OSDItemValue, (BYTE)regv );

	if( EE_Read( PC_COLOR_DATA ) != 3 )  // User
			FOsdRamMemsetAttr(FOSDMENU_SUBADDR+MENU_WIDTH*2, BACK_COLOR, (MENU_HEIGHT-1)*MENU_WIDTH );	
	else
			FOsdRamMemsetAttr(FOSDMENU_SUBADDR+MENU_WIDTH*2, DEFAULT_COLOR, (MENU_HEIGHT-1)*MENU_WIDTH );	

	return OSDItemValue;
}


BYTE SetPCColor(BYTE color, BYTE flag) 
{
	int newval, toValue, regv;

//	GetPCColorTemp(color);
	newval = ( flag==FOSD_UP  ? OSDItemValue+1  :  OSDItemValue-1  );
	if( newval< UserRange.Min || newval> UserRange.Max )
		return OSDItemValue;

	OSDItemValue = newval;
	Mapping1( newval, &UserRange , &toValue, &PCUserColorTempRange);
	EE_Write( PC_USERCOLOR+color, newval );

	newval = EE_Read(0x10) ;
	Mapping1( newval, &UserRange ,&regv, &PanelContrastRange);

	WriteTW88Page(2);
	WriteTW88(REG281+color,(BYTE)(toValue + regv));

	return  OSDItemValue;
}


//#define		BACKLIGHT	0x07

CODE BYTE VideoModeTable_value[][6] ={
	//  Contrast		Brightness	Saturation	Hue		Sharpness		Backlight
	{	0x83,          0x80,        0x80,       0x00,     0x60,			0x00/*0xF0*/  },	// Mode 1: 
	{	0x88,	       0x78,	    0x90,	    0x00,     0x60,			0x00/*0xE0*/  },	// Mode 2: 
	{	0x78,	       0x78,	    0xa0,	    0x00,     0x60,			0x00/*0xD0*/  },	// Mode 3: 
};


#define USER_VIDEOMODE	3
void SetVideoMode(BYTE mode)
{
	//BYTE	i;
	//int		regv;

	dPrintf("\nSetVideoMode - %bd", mode);
	EE_Write( EEP_VIDEOMODE, mode );
	
	if( mode != USER_VIDEOMODE ) {
		WriteTW88Page(PAGE2_IMAGE );
		WriteTW88(REG284_IA_CONTRAST_Y, 	VideoModeTable_value[mode][0]);
		WriteTW88(REG28A_IA_BRIGHTNESS_Y,	VideoModeTable_value[mode][1]);
		WriteTW88(REG285_IA_CONTRAST_CB, 	VideoModeTable_value[mode][2]);
		WriteTW88(REG285_IA_CONTRAST_CB+1, 	VideoModeTable_value[mode][2]);
		WriteTW88(REG280_IA_HUE_REG, 		VideoModeTable_value[mode][3] | (ReadTW88(REG280_IA_HUE_REG) & 0xC0));
		WriteTW88(REG28B_IA_SHARPNESS, 		VideoModeTable_value[mode][4] | (ReadTW88(REG28B_IA_SHARPNESS ) & 0xF0) );
		WriteTW88Page(PAGE0_LEDC);
		WriteTW88(REG0E5, 					VideoModeTable_value[mode][5] | (ReadTW88(REG0E5) & 0x80));
	}
	else { // USERMODE
		SetImage(InputMain);
	}
}

BYTE GetVideoDatawithOffset(BYTE offset)
{
	if(offset==OFFSET_backlight) {	 //backright
		OSDItemValue = EE_Read(EEP_BACKLIGHT);
	}
	else {
		OSDItemValue = GetVideoDatafromEE(offset);
	}
	return OSDItemValue;
}

BYTE ChangeVideoData(BYTE offset, BYTE flag)
{
	int newv, inc;

	inc = (flag == FOSD_UP ? 1 : -1);

	newv = OSDItemValue + inc;
	if (newv < UserRange.Min || newv > UserRange.Max)
	{
		return OSDItemValue;
	}

	switch (offset)
	{
	case OFFSET_contrast:
		ImgAdjSetContrastY(newv);
		ImgAdjChangeContrastY();
		break;
	case OFFSET_bright:  
		ImgAdjSetBrightnessY(newv);
		ImgAdjChangeBrightnessY();
		break;
	case OFFSET_staturation: 
		ImgAdjSetSaturation(newv);
		ImgAdjChangeSaturation();
		 break;
	case OFFSET_hue:
		ImgAdjSetHUE(newv);
		ImgAdjChangeHUE();
		break;
	case OFFSET_sharpness:
		ImgAdjSetSharpness(newv);
		ImgAdjChangeSharpness();
		break;
	case OFFSET_backlight:	  
		BackLightSetRate(newv);
		BackLightChangeRate();
		break;
	}

	OSDItemValue = newv;

	return newv;	
}

#ifdef WIDE_SCREEN

#include "\data\WideModeData.txt"
void SetScale4_3Y(void) // 16:9 --> 4:3  // Normal
{
	extern  CODE struct _PCMDATA  PCMDATA[];
	extern	IDATA BYTE PcMode;
	DWORD outv, scale;

	// Vertical
	scale = (DWORD)PCMDATA[PcMode].vActive	;
	outv  = (DWORD)PANEL_V; 
	scale = 0x10000L * scale / outv;

	YScale2( scale );

	// Horizontal
	if (PcMode >= EE_XGA_60 && PcMode >= EE_XGA_75)
		scale = (DWORD)PCMDATA[PcMode].hActive-4; //just 1024	for focus issue(1:1).
	else
		scale = (DWORD)PCMDATA[PcMode].hActive;	

	scale = scale * 0x10000L * 3 / PANEL_V /4;

	XScale2( scale );
}

void SetScale(void)
{
	DWORD scale, outv;

	// Vertical
	scale = (DWORD)PCMDATA[PcMode].vActive;
	outv  = (DWORD)PANEL_V ; 
	scale = 0x10000L * scale / outv;
	YScale2( scale );
	#ifdef DEBUG_PC
	Printf("\n PCMode[%d] %ld YScale: %ld", (WORD)PcMode, outv, scale);
	#endif
	// Horizontal
	scale = PCMDATA[PcMode].hActive;
	outv  = PANEL_H ; 
//	scale = scale  * 0x100L / outv;
	scale = (scale*0x10000L+outv-1) / outv;
	XScale2( scale );
}

void SetScale4_3X(void)	
{
	DWORD scale, outv;

	// Horizontal
	scale = (DWORD)PCMDATA[PcMode].hActive;
	outv  = (DWORD)PANEL_H ; 
	scale = scale  * 0x10000L / outv;
	XScale2( scale );

	// Vertical
	scale = (DWORD)PCMDATA[PcMode].vActive; 
	outv  = outv * 3 / 4;			// 4:3
	scale = 0x10000L * scale / outv;
	YScale2( scale);				// *********** plus 2
}
void SetLRBlank(BYTE bl)
{
	WriteTW88(0x66, bl);
}

void Set4WideScreen( BYTE id )
{
	if( id > WIDESCREEN_FULL ){
		Set4WideScreen(WIDESCREEN_WIDE);
		id = WIDESCREEN_WIDE;
	}

	WideScreenMode = id;

	{
		int i;
		CODE_P BYTE *WideData;
		
		#ifdef SUPPORT_COMPONENT
		if (InputMain== COMPONENT )
		{
			switch (ComponentMode)
			{
				case YPBPR_576i:	WideData = WIDEDATA_576i[id];	break;
				#ifndef WQVGA
				case YPBPR_480p:	WideData = WIDEDATA_480p[id];	break;
				case YPBPR_576p:	WideData = WIDEDATA_576p[id];	break;
				case YPBPR_720p:	WideData = WIDEDATA_720p[id];	break;
				case YPBPR_1080i:	WideData = WIDEDATA_1080i[id];	break;
				#endif
				default:			WideData = WIDEDATA_480i[id];	break;
			}

		}
		else 
		#endif
		{
				//if( DecoderReadVInputSTD()== NTSC || DecoderReadVInputSTD()== NTSC4 ) 
				if( GetVInputStd() == NTSC || GetVInputStd()== NTSC4 || GetVInputStd() == 0) {
					#ifdef ADD_ANALOGPANEL
					if(IsAnalogOn()) 
					WideData = WIDEDATA_NA[id];
					else
					#endif
					WideData = WIDEDATA_N[id];
				}
				else{
					#ifdef ADD_ANALOGPANEL
					if(IsAnalogOn()) 
					WideData = WIDEDATA_PA[id];
					else
					#endif
					WideData = WIDEDATA_P[id];
				}
		}

		#ifdef DEBUG
		//Printf( "\r\nWrite Wide Mode Type: %d, Data: %d, VInputStd: %d", (WORD)ComponentMode, (WORD)id, (WORD)GetVInputStd());
		#endif
		WaitVBlank	(1);				// Wait Vblank()
		for(i=0; i<WIDE_DATA_MAX; i++){
					WriteTW88($$$WIDE_ADDRESS[i], *WideData);
					//#ifdef DEBUG
					//Printf( " %2x", (WORD)*WideData );
					//#endif
					WideData++;
		}
	}
	SaveWideModeEE(WideScreenMode);
}
#endif // WIDE_SCREEN




//---------------------------------------------------------------------
// Push MenuTile, title_item in stack 
//      MenuTile[title_item][]...  
//
BYTE pushMenu(void)
{
	#ifdef DEBUG_OSD
	dPrintf("\n(pushMenu)-MenuSP:%d", (WORD)MenuSP);
	#endif

	if( MenuSP < sizeof( MenuIndexStack ) ) {
		MenuStack[MenuSP] = MenuTitle;
		MenuIndexStack[MenuSP++] = title_item;
		return 1;
	}
	return 0;
}
BYTE popMenu(void)
{
	#ifdef DEBUG_OSD
	dPrintf("\n(popMenu)-MenuSP:%d", (WORD)MenuSP);
	#endif

	if( MenuSP ) {
		MenuTitle = MenuStack[--MenuSP];
		title_item = MenuIndexStack[MenuSP];
		return 1;
	}
	return 0;
}		

static BYTE FOsdMenuGetLowerItem(CODE_P struct DefineMenu *DMp, BYTE itemno)
{
	if( itemno==NIL )
		return NIL;
	itemno++;
	if(DMp[itemno].Type==0)
		return NIL;
	return itemno;

}
static BYTE FOsdMenuGetUpperItem(BYTE itemno)
{
	if(	itemno==NIL )
		return NIL;
	itemno--;
	return itemno;
}

BYTE GetTargetChild(BYTE tid)
{
	BYTE i;

	#ifdef DEBUG_OSD
	dPrintf("\r\nTarget id:0x%x", (WORD)tid);
	#endif

	if( MenuChild )		i = 0;
	else				i = NIL;

	while( i != NIL ) {
		
		#ifdef DEBUG_OSD
		dPrintf("__ForActionId[%d]:0x%x", (WORD)i, (WORD)MenuChild[i].Id);
		#endif
		
		if ( ( MenuChild[i].Id & 0x0f ) == tid )
			return i;//break;
		i = FOsdMenuGetLowerItem(MenuChild, i );
	}

	return i;
}


//********************************************************************
//
//	Functions for BAR.
//
//********************************************************************
#define VOLBAR_LENGTH 21+3
//extern
//	FOSDMENU_BARADDR
void DisplayVolumebar(BYTE val)
{
	BYTE i;
	WORD addr;
	CODE BYTE BAR_ROM_TABLE[] = {
		CH_BAR_000, 			// ...
		CH_BAR_100,CH_BAR_100, 	// |..
		CH_BAR_110,CH_BAR_110, 	// ||.
		CH_BAR_1, 				// |
		CH_BAR_111, 			// |||
	};
	BYTE Bar_str[VOLBAR_LENGTH]={ 
		CH_BAR_111, CH_BAR_111, CH_BAR_111, CH_BAR_111, CH_BAR_111,	CH_BAR_111, CH_BAR_111, CH_BAR_111, CH_BAR_111, CH_BAR_111,
		CH_BAR_111, CH_BAR_111, CH_BAR_111, CH_BAR_111, CH_BAR_111,	CH_BAR_111, CH_BAR_111, CH_BAR_111, CH_BAR_111, CH_BAR_111, 
		CH_BAR_1,
		' ',' ',' ' 	//number area
	};
	
	//convert value to Bar_str[]
	if(val < 100) {
		Bar_str[val/5] = BAR_ROM_TABLE[val%5];
		for(i = val/5 + 1; i < 20; i++)
			Bar_str[i] = BAR_ROM_TABLE[0];
	}
	if( Bar_str[0] == CH_BAR_000 ) Bar_str[0] = CH_BAR_100;	// ... => |..

	// Number
    Bar_str[23] = val % 10 + '0';    val /= 10;
    Bar_str[22] = val % 10 + '0';    val /= 10;
    Bar_str[21] = val + '0';
	if( Bar_str[21] == '0' ) Bar_str[21] = ' ';

	addr = FOSDMENU_BARADDR + 1 + VOLBAR_LENGTH;	//Pls, use parameter. 
	WriteStringToAddr(addr, Bar_str, VOLBAR_LENGTH ); 
}

BYTE FOsdMenuGetCursorItem(BYTE id)
{
	BYTE i=NIL, tid;

#ifdef DEBUG_OSD
	dPrintf("\r\nFOsdMenuGetCursorItem(id:%d)", (WORD)id);
#endif

	switch ( id ) {
	case FMID_INPUTSELECTION:	
		tid = InputMain; 
		break;
	case FMID_BT656_SRC_SELECTION:
		tid = InputBT656;
	default:				
		return 0;// The first one
	}

	i = GetTargetChild( tid );

	if( i==NIL ) {	//couldn't find
#ifdef DEBUG_OSD
		dPuts("___Couldn't find cursor!!");
#endif
		i=0;
	}

#ifdef DEBUG_OSD
	dPrintf("___Cursor:%d\r\n__(GetDefaultForChoiceMenu)", (WORD)i);
#endif

	return i;
}

//==================================================================================================
// Func: FOsdMenuDisplayOneItem
//
//
void FOsdMenuDisplayOneItem(BYTE winno, CODE_P struct DefineMenu *DMp, BYTE x, BYTE y, BYTE NOTViewflag )
{
	BYTE i,j,k,IconColor;
	WORD addr;
	CODE_P BYTE *Str;
	CODE BYTE RightSign_str[]=	{ CH_ARROW_RIGHT,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0 };// RIGHT mark
	CODE_P BYTE *ptr;

	#ifdef DEBUG_OSD
	dPrintf("\nFOsdMenuDisplayOneItem(%bx,ID:%bx,%bd,%bd,) ",winno,DMp->Id, x,y);
	#else
	i = winno;	
	#endif

	if( cur_osdwin == FOSDMENU_TOPWIN) 	addr = FOSDMENU_MAINADDR;
	else 								addr = FOSDMENU_SUBADDR;
	addr = addr + x + y*MenuFormat->width;

	#ifdef DEBUG_OSD
	dPrintf("\tStartaddr:%d", addr);
	#endif

	if( DMp->IconColor !=NIL ) {		// Draw Icon
		//if(OnChangingValue) IconColor = BG_COLOR_BLACK|CH_COLOR_WHITE|EXTENDED_COLOR; 
		//else
			IconColor =  DMp->IconColor;

		Str = DMp->Icon;
		j=TWstrlen(Str);
		WriteStringToAddr(addr, Str, j);

		x += ICON_CH_SZ;
	}

	if( DMp->DescColor !=NIL ) {		// Draw Desc.
		Str = DMp->Desc[OSDLang];
		j=TWstrlen(Str);

		for( i=0,k=0; i<j; i++ )
			if( *(Str+i) == FONT_ROM || *(Str+i) == FONT_RAM ) k++;

		//#ifdef DEBUG_OSD
		//dPrintf("\r\naddr:%d x:%d y:%d MenuFormat->width:%d", addr, (WORD)x, (WORD)y, (WORD)MenuFormat->width );
		//#endif

		WriteStringToAddr(addr, Str, j);

		if( (j-k ) < MAX_DESC ) 
			FOsdRamMemsetData(addr+j-k, 0x020, MAX_DESC-j+k);
	
		addr+=MAX_DESC;
	}

	if( OSDMenuLevel < 3) 		// Main Menu Item.
		if ( NOTViewflag==1 )	// In case of not need diplay contents. ex, BAR windows, top menu display.
			return;

	// Menu contents view

	switch (DMp->Type ) {
	case MIT_BAR:	// Draw Bar and Number value
		DisplayViewBar( addr, FOsdMenuGetItemValue(DMp->Id));
		break;

	case MIT_NUM: // Draw Number value
		ptr = DMp->TypeData;
		switch ( DMp->Id ) {
			#if defined( PAL_TV )
			case MANUAL_TUNE_CHANNEL:
				WriteStringToAddr(addr, RightSign_str, BARNUM_LENGTH); 
				break;
			#endif
			default:
				DrawNum( addr, (CODE_P struct NumType *)ptr, FOsdMenuGetItemValue(DMp->Id) );
				break;
		}
		break;

	case MIT_E_STR:		
		ptr = DMp->TypeData;
		DrawStringByLang( addr, (CODE_P struct EnumType *)ptr, FOsdMenuGetItemValue(DMp->Id) );
		break;

	case NUMBER_STRING:
	case MIT_SELMENU:
	case MIT_MENU:
	case MIT_ACTION:
			WriteStringToAddr(addr, RightSign_str, BARNUM_LENGTH); 
			break;

	}
  
}

//-------------------------------------------------------------------
// Cursor
//
//	ClearCursor
//	DisplayCursor
//-------------------------------------------------------------------
void FOsdMenuClearCursor(void)
{
	BYTE cursor_y;
	WORD addr;

	if( cursor_item==NIL )
		return;

	cursor_y = cursor_item + ( MenuFormat->TitleColor ==NIL ? 0 : 1 );
	if( cur_osdwin == 4 ) 	addr = 0;
	else 					addr = 10;
	addr += cursor_y*MenuFormat->width;
	FOsdRamMemsetAttr(addr, DEFAULT_COLOR, MenuFormat->width );	
}

//extern
//	cursor_item
//	MenuFormat
//	OnChangingValue
void FOsdMenuDisplayCursor(void)
{
	BYTE cursor_y=0 ;
	WORD addr;

#ifdef DEBUG_FOSD
	dPrintf("\nFOsdMenuDisplayCursor() cursor_item:%bx",cursor_item);
#endif
	
	if( cursor_item==NIL )
		return;
	
	cursor_y = cursor_item + ( MenuFormat->TitleColor ==NIL ? 0 : 1 );

	if( OSDMenuLevel == 1 ) addr = FOSDMENU_MAINADDR;
	else 					addr = FOSDMENU_SUBADDR;

	addr += cursor_y*MenuFormat->width;

	if( OSDMenuLevel == 1)
		FOsdRamMemsetAttr(addr, MenuFormat->CursorColor, MenuFormat->width );	
	else {
		if(OnChangingValue)	{
			FOsdMenuGetItemValue(FOsdMenuGetItemValue(MenuChild[cursor_item].Id));	//BUGBUG
			FOsdRamMemsetAttr(addr, MenuFormat->CursorColor, MenuFormat->width );	
		}
		else  {
			FOsdRamMemsetAttr(addr, MenuFormat->CursorColor, MenuFormat->width-BARNUM_LENGTH );	
			FOsdRamMemsetAttr(addr+MAX_DESC, DEFAULT_COLOR, BARNUM_LENGTH );	
		}
	}
}


//===============================================================================
//  OSD Menu Operation funtion moved to "OSDoper.c" by RYU.
// 
//===============================================================================


//-------------------------------------------------------------------
// GetItemValue
// SetItemValue
//-------------------------------------------------------------------

WORD FOsdMenuGetItemValue(BYTE id)
{
	WORD val=0;

	#ifdef DEBUG_OSD
	dPrintf("\n\tFOsdMenuGetItemValue(id:0x%x)", (WORD)id);
	#endif

	switch( id ) {
	//case SYSTEM_CLOCK:	val = GetSystemClock_HM();			break;
	//case WAKEUP_TIME:		val = OSDItemValue = GetWakeupTime();	break;
	//case OFF_TIME:		val = OSDItemValue = GetOffTime();		break;
	case FMID_SLEEP_TIMER:		val = OSDItemValue = GetSleepTimer();	break;

#ifdef SUPPORT_OSDPOSITIONMOVE
	case FMID_OSDPOSITIONMODE:   val = GetOSDPositionMode();				break;
	//case FMID_OSD_VPOS:			val = GetOSDMenuYValue();				break;
	//case FMID_OSD_HPOS:			val = GetOSDMenuXValue();				break;
#endif
	//case ID_OSD_DURATION:	val = GetOSDDurationValue();			break;
	case FMID_OSDLANG:		val = OSDLang & 0x0f;					break;


	case FMID_VIDEO_MODE:		val = EE_Read( EEP_VIDEOMODE );			break;

	case FMID_VIDEO_CONTRAST:	
	case FMID_VIDEO_BRIGHTNESS:	
	case FMID_VIDEO_SATURATION:	
	case FMID_VIDEO_HUE:			
	case FMID_VIDEO_SHARPNESS:	
	case FMID_VIDEO_BACKLIGHT:	
		val = GetVideoDatawithOffset(id-FMID_VIDEO_CONTRAST);	
		break;


	case FMID_AUDIO_VOLUME:		val = AudioVol;					break;
	case FMID_AUDIO_BALANCE:	val = AudioBalance;				break;
	case FMID_AUDIO_BASS:		val = AudioBass;				break;
	case FMID_AUDIO_TREBLE:		val = AudioTreble;				break;
	//case FMID_AUDIO_EFFECT:	val = GetAudioEffect();			break;
	case FMID_AUDIO_MUTE:		val = IsAudioMuteOn();			break;

	case FMID_INPUTSTD:			
		val = OSDItemValue = InputMain;			
		break;

#ifdef WIDE_SCREEN		//.................................................
	case FMID_WIDESCREEN:
		val = WideScreenMode;					
		break;
#endif					//.................................................

	}
#ifdef DEBUG_OSD
	dPrintf(" val:%x", val);
#endif

	return val;
}

#define MAX_VIDEOMODE	4
#define MAXVCHIPMOVIE	7 

//#define	RED		0
//#define	GREEN	1
//#define	BLUE	2
//#define WHITE	3

WORD FOsdMenuSetItemValue(BYTE id, BYTE flag )
{
	WORD val=0;

#ifdef DEBUG_OSD
	dPrintf("\n\tFOsdMenuSetItemValue(%bx,%bx)", id, flag);
#endif

	switch( id ) {
	case FMID_SLEEP_TIMER:		val = ChangeSleepTimer( flag );			break;

//#ifdef SUPPORT_OSDPOSITIONMOVE
	case FMID_OSDPOSITIONMODE:    val = SetOSDPositionMode( flag );		break;
	//case FMID_ID_OSD_VPOS:			val = SetOSDMenuYValue(flag);			break;
	//case FMID_ID_OSD_HPOS:			val = SetOSDMenuXValue(flag);			break;
//#endif

	//case FMID_OSD_DURATION:		val = SetOSDDurationValue(flag);		break;
	case FMID_OSDLANG:		val = SetLanguage (flag);				break;

	#if 0
	#if defined(SUPPORT_PC) || defined(SUPPORT_BT656_LOOP)
	case FMID_PANEL_CONTRAST:	val = SetPanelContrast(flag);			break;
	case FMID_PANEL_BRIGHTNESS:	val = SetPanelBrightness(flag);			break;
	#endif

	#ifdef SUPPORT_PC
	case FMID_PC_VPOS:			val = SetVPosition(flag);				break;
	case FMID_PC_HPOS:			val = SetHPosition(flag);				break;
	case FMID_PC_CLOCK:			val = SetPCCoarseValue(flag);			break;
	case FMID_PC_PHASE:			val = SetPhaseValue(flag);				break;
	#endif
	#endif

	case FMID_PC_COLOR_TEMP:	val = SetPCColorTemp(flag);				break;

	case FMID_PC_COLOR_R:		val = SetPCColor(0/*RED*/,   flag);		break;	
	case FMID_PC_COLOR_G:		val = SetPCColor(1/*GREEN*/, flag);		break;	
	case FMID_PC_COLOR_B:		val = SetPCColor(2/*BLUE*/,  flag);		break;	

	case FMID_VIDEO_MODE:		
		val = EE_Read( EEP_VIDEOMODE );
		if(flag==FOSD_UP) val ++;
		else val = val + MAX_VIDEOMODE -1;
		val %= MAX_VIDEOMODE ; 
		
		SetVideoMode(val);		
		
		if( EE_Read( EEP_VIDEOMODE ) != 3 )  // if not user mode, mask the control.
			FOsdRamMemsetAttr(FOSDMENU_SUBADDR+MENU_WIDTH*2, BACK_COLOR, (MENU_HEIGHT-1)*MENU_WIDTH );	
		else
			FOsdRamMemsetAttr(FOSDMENU_SUBADDR+MENU_WIDTH*2, DEFAULT_COLOR, (MENU_HEIGHT-1)*MENU_WIDTH );	
		break;

	case FMID_VIDEO_CONTRAST:	
	case FMID_VIDEO_BRIGHTNESS:	
	case FMID_VIDEO_HUE:			
	case FMID_VIDEO_SATURATION:	
	case FMID_VIDEO_SHARPNESS:	
	case FMID_VIDEO_BACKLIGHT:
		val = ChangeVideoData(id-FMID_VIDEO_CONTRAST,flag);			
		break;

 	case FMID_AUDIO_VOLUME:		val = ChangeVol( (flag==FOSD_UP ? 1 : -1) );		break;
	case FMID_AUDIO_BALANCE:	val = ChangeBalance( (flag==FOSD_UP ? 1 : -1) );	break;
	case FMID_AUDIO_BASS:		val = ChangeBass( (flag==FOSD_UP ? 1 : -1) );		break;
	case FMID_AUDIO_TREBLE:		val = ChangeTreble( (flag==FOSD_UP ? 1 : -1) );		break;


	#ifdef WIDE_SCREEN
	case FMID_WIDESCREEN:			
		val = WideScreenMode;
		if(flag==UP) val++;
		else val = ( val+MAXWIDESCREEN -1 );	
		val %= MAXWIDESCREEN;
		Set4WideScreen( val );
		break;
	#endif	//WIDE_SCREEN

	}
#ifdef DEBUG_OSD
	dPrintf(" val:%x", val);
#endif

	return val;
}

//extern 
//	BAR_LENGTH
//	BARNUM_LENGTH
void DisplayViewBar(WORD addr, BYTE val)
{
	BYTE i=0;
	CODE BYTE BAR_ROM_TABLE[] = {
		CH_BAR_000,CH_BAR_000,CH_BAR_000,CH_BAR_000, 				// ...
		CH_BAR_100,CH_BAR_100,CH_BAR_100,CH_BAR_100,CH_BAR_100, 	// |..
		CH_BAR_110,CH_BAR_110,CH_BAR_110,CH_BAR_110,CH_BAR_110,		// ||.
		CH_BAR_1, 													// |
		CH_BAR_111, 												// |||
	};
	BYTE Bar_str[]={ CH_BAR_111, CH_BAR_111, CH_BAR_111, CH_BAR_111, CH_BAR_111, CH_BAR_111, CH_BAR_111, CH_BAR_1, ' ', ' ', ' ',0 };

	// Bar
	if( BAR_LENGTH != 0 ) {
		if(val < 98) {
			Bar_str[val/14] = BAR_ROM_TABLE[val%14];
			for(i = val/14 + 1; i < 7; i++)
				Bar_str[i] = BAR_ROM_TABLE[0];
		}
		if( Bar_str[0] == CH_BAR_000 ) Bar_str[0] = CH_BAR_100;	// ...=>|..
	}

	i = 7;
	// Number
    Bar_str[i+2] = val % 10 + '0';    val /= 10;
    Bar_str[i+1] = val % 10 + '0';    val /= 10;
    Bar_str[i] 	 = val + '0';
	if( Bar_str[i] == '0' ) Bar_str[i] = ' ';

	for(;i<7;i++)
		Bar_str[i+3] = ' ';

	WriteStringToAddr(addr, Bar_str, BARNUM_LENGTH); 
}
//
//extern
//	MAX_DESC
//	NUM_RADIX
//	BARNUM_LENGTH
void DrawNum(WORD addr, CODE_P struct NumType *nptr, WORD val)
{
	BYTE x,i, j,k, len, mstr[5];//,color;
	BYTE Num_str[]={ ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0 };
	CODE_P BYTE *str;
	
	x=  MAX_DESC;//+ICON_CH_SZ;
	i =  nptr->PrePostCharLen >> 4 ;
	j =  nptr->PrePostCharLen & 0x0f ;
	str = nptr->PrePostChar;

	// Draw char on front of number. 
	if( i ) 
		for(k=0; k<i; k++)
			Num_str[k] = *str++;
	// Draw number.
	TWutoa(val,mstr,NUM_RADIX);
	len = TWstrlen(mstr);
	str=mstr;
	for(k=0; k<len; k++)
		Num_str[k+i] = *str++;

	// Draw char on back of number
	if( j ) 
		for(k=0; k<j; k++)
			Num_str[k+i+len] = *str++;
	WriteStringToAddr(addr, Num_str, BARNUM_LENGTH); 	
}

void DrawStringByLang( WORD addr, CODE_P struct EnumType *yptr, WORD val )
{
	BYTE j,l,Size_val;
	CODE_P BYTE *Str;

	//clear
	FOsdRamMemsetData(addr, ' ', BARNUM_LENGTH); 

	Size_val = yptr->Sz;
	Str = yptr->Str;

	if(yptr->ByLang) {
		for(j=0; j < OSDLang*Size_val; j++)			
			Str += ( TWstrlen(Str) + 1 );
	}

	if(val){ 
		for(j=0; j<val; j++)			
			Str += ( TWstrlen(Str) + 1 );	
	}

	l = TWstrlen(Str);
	WriteStringToAddr(addr, Str, l); 

}

//========================================================
//
//========================================================
//extern
//	MenuFormat
//	MenuTitle[]
//	MenuChild
//	cur_osdwin
//	OSDLang
//	title_item
void FOsdMenuDrawList(BYTE NOTViewflag)
{
	BYTE  i,j,x=0, y=0;
	CODE_P struct DefineMenu *DMp;
	CODE_P BYTE *Desc;
	BYTE  Max_y;
	WORD addr;

#ifdef DEBUG_FOSD
	dPrintf("\nFOsdMenuDrawList");
#endif

	// Draw Title line
	if( MenuFormat->TitleColor != NIL ) {
		//#ifdef DEBUG_OSD
		//dPrintf("\r\nTitle color:0x%x", (WORD)MenuFormat->TitleColor);
		//#endif
 
		DMp = &MenuTitle[title_item];
		Desc = DMp->Desc[OSDLang];
		j=TWstrlen(Desc);
		FOsdRamMemset(FOSDMENU_SUBADDR, 0x020, MenuFormat->TitleColor,TITLE_MAX_DESC);
		WriteStringToAddr(FOSDMENU_SUBADDR, Desc, j);			
		y++;
	}
	
	if( MenuChild )	i = 0;
	else			i = NIL;

	// Draw 
	Max_y = (cur_osdwin == FOSDMENU_TOPWIN ? TOTAL_TOP_HEIGHT: MENU_HEIGHT+1);
	addr = (cur_osdwin == FOSDMENU_TOPWIN ? FOSDMENU_MAINADDR: FOSDMENU_SUBADDR);
	addr += y*MenuFormat->width;

//	if( NOTViewflag==0 ) {
//		I2CAutoIncFlagOn = 1;	// speedy submenu drawing 
//		WriteTW88Page(PAGE3_FOSD );
//		WriteTW88(TW88_ENABLERAM, 0x04); // OSD RAM Auto Access Enable
//	}
	dPrintf("	Line:%bd",Max_y);
	for(  ; y < Max_y; y++) {
		if(i != NIL) {
			FOsdMenuDisplayOneItem(cur_osdwin, &MenuChild[i], x, y, NOTViewflag);
			i = FOsdMenuGetLowerItem( MenuChild, i );
		}
		else {
			// Clear the counter of OSD RAM Data from addr with space(' ').
			FOsdRamMemsetData(addr, 0x020, MenuFormat->width);	
		}
		addr += MenuFormat->width;	
	}

 //	 if( NOTViewflag==0 ) {
//		 I2CAutoIncFlagOn = 0;	// Recover  
//		 WriteTW88Page(PAGE3_FOSD );
//		 WriteTW88(TW88_ENABLERAM, 0x00); // OSD RAM Auto Access Disable
//	 }
}

//
// Here, Main Menu will display Only Icon at leftside.
//
void FOsdMenuDisplayMenu(void)
{
	CODE_P BYTE *Str;
	BYTE i;

#ifdef DEBUG_FOSD
	dPrintf("\nFOsdMenuDisplayMenu title_item:%bx cursor_item:%bx", title_item, cursor_item );
#endif

	//Clear
	if((MenuChild[ cursor_item ].Id == FMID_PC_COLOR_MODE && EE_Read( PC_COLOR_DATA ) != 3 )  // User
	|| (MenuChild[ cursor_item ].Id == FMID_VIDEOPICTURE && EE_Read( EEP_VIDEOMODE ) != 3 ) ) // User
	{
		FOsdRamMemsetAttr(FOSDMENU_SUBADDR+MENU_WIDTH, DEFAULT_COLOR, MENU_WIDTH );	
		FOsdRamMemsetAttr(FOSDMENU_SUBADDR+MENU_WIDTH*2, BACK_COLOR, (MENU_HEIGHT-1)*MENU_WIDTH );	
	}
	else
		FOsdRamMemsetAttr(FOSDMENU_SUBADDR+TITLE_MAX_DESC, DEFAULT_COLOR, MENU_HEIGHT*MENU_WIDTH );	
	FOsdRamMemsetAttr(FOSDMENU_SUBADDR+TITLE_MAX_DESC*8, COMMENT_COLOR, TITLE_MAX_DESC );	 // All Line Attr clear by default color.

	//Draw Current Menu
	FOsdMenuDrawList(1);

	// Draw Child
	if( OSDMenuLevel == 1 ) {
		// Store ManuTile, tile_item.
		pushMenu();
		
		//	
		MenuTitle = MenuChild;
		title_item = cursor_item;
		
		MenuFormat = ( CODE_P struct MenuType *) MenuTitle[title_item].TypeData;
		MenuChild = MenuTitle[title_item].Child;
		cur_osdwin--; 

		// draw total Submemu.
		FOsdMenuDrawList(0);	
		
		// Restore ManuTile, tile_item.
		popMenu(); 
		MenuFormat = ( CODE_P struct MenuType *) MenuTitle[title_item].TypeData;
		MenuChild = MenuTitle[title_item].Child;
		cur_osdwin++;
	}
	//Draw Comment
	Str = Str_Comments[OSDLang];
	i=TWstrlen(Str);
	WriteStringToAddr(FOSDMENU_SUBADDR+TITLE_MAX_DESC*8, Str, i);

	//Draw Cursor
	FOsdMenuDisplayCursor();
}

static void FOsdHighMenu_sub(void)
{
	BYTE temp_cursor;

#ifdef DEBUG_OSD
	dPuts("\nFOsdHighMenu_sub()");
#endif
	temp_cursor = cursor_item ;
	MenuChild = MenuTitle;
	cursor_item = title_item;

	if( !popMenu() ) {
		FOsdMenuClose();

		MenuTitle = MenuChild;	//  Restore the Menu status.
		title_item = cursor_item;
		MenuChild = MenuTitle[title_item].Child;
		cursor_item = temp_cursor;
	
		//FOsdMenuDoAction(EXITMENU);
		
		if( DisplayInputHold ) FOsdDisplayInput();
		//else if( IsAudioMuteOn() ) DisplayMuteInfo();
		return;
	}

	#ifdef DEBUG_OSD
	dPrintf("  OSDMenuLevel:%d", (WORD)OSDMenuLevel );
	#endif
	
	MenuFormat = ( CODE_P struct MenuType *) MenuTitle[title_item].TypeData;

	OSDMenuLevel--; // Up Menu tree

	if(OSDMenuLevel == 1){	// Top Menu Level
		cur_osdwin = FOSDMENU_TOPWIN;//Window #4
		FOsdMenuDisplayMenu();
	}
	else {
		FOsdMenuDrawList(0); // Redraw List and contents.
		FOsdMenuDisplayCursor();
	}
	
	if(OSDMenuLevel == 1) {
		FOsdWinSet3DControl(FOSDMENU_CURSORWIN, WINDOW_3D_TOP|BG_COLOR_BLACK|EXTENDED_COLOR|WINDOW_3D_ENABLE|WINDOW_ENABLE );
	}
	return;
}


BYTE FOsdMenuDoAction(BYTE id)
{
	BYTE ret = 1;

#ifdef DEBUG_OSD
	dPrintf("\nFOsdMenuDoAction(%bx)", id );
#endif
	switch( id ) {

#ifdef SUPPORT_CVBS
	case FMID_CHANGE_TO_COMPOSITE:  ChangeInput(FMID_CHANGE_TO_COMPOSITE & 0x0F);	break; 	//ChangeCVBS();
#endif
#ifdef SUPPORT_SVIDEO
	case FMID_CHANGE_TO_SVIDEO:  	ChangeInput(FMID_CHANGE_TO_SVIDEO & 0x0F); 		break;	//ChangeSVIDEO();
#endif
#ifdef SUPPORT_COMPONENT
	case FMID_CHANGE_TO_COMPONENT:  ChangeInput(FMID_CHANGE_TO_COMPONENT & 0x0F); 	break;	//ChangeCOMPONENT();
#endif
#ifdef SUPPORT_PC
	case FMID_CHANGE_TO_PC:  		ChangeInput(FMID_CHANGE_TO_PC & 0x0F); 			break;	//ChangePC();
#endif
#ifdef SUPPORT_DVI
	case FMID_CHANGE_TO_DVI:  		ChangeInput(FMID_CHANGE_TO_DVI & 0x0F); 		break;	//ChangeDVI();
#endif
#if defined(SUPPORT_HDMI)
	case FMID_CHANGE_TO_HDMI:		ChangeInput(FMID_CHANGE_TO_HDMI & 0x0F); 		break;	//ChangeHDMI();
#endif	
#ifdef SUPPORT_BT656_LOOP
	case FMID_CHANGE_TO_656:		ChangeInput(FMID_CHANGE_TO_656 & 0x0F); 		break;	//ChangeBT656();
#endif
#ifdef SUPPORT_BT656_LOOP
	case FMID_CHANGE_TO_LVDS:		ChangeInput(FMID_CHANGE_TO_LVDS & 0x0F); 		break;	//ChangeLVDS();
#endif
						

#ifdef SUPPORT_BT656_LOOP  //it is not LOOP, use the other define..
	case FMID_BT656_FROM_DEC:	SetBT656Output(BT656_SRC_DEC);	break;
	case FMID_BT656_FROM_PC:	SetBT656Output(BT656_SRC_ARGB);	break;
	case FMID_BT656_FROM_HDMI:	SetBT656Output(BT656_SRC_DTV);	break;
	case FMID_BT656_FROM_LVDS:	SetBT656Output(BT656_SRC_LVDS);	break;
	case FMID_BT656_FROM_PANEL:	SetBT656Output(BT656_SRC_PANEL);	break;
	case FMID_BT656_FROM_OFF:	SetBT656Output(BT656_SRC_OFF);	break;
#endif
	
	
	case FMID_RESET_VIDEOVALUE:		
		ClearOSDInfo();
		ResetVideoValue();
		ResetOSDValue();
		ResetAudioValue();
		break;

//----
	case FMID_TOHIGHMENU:			
		FOsdHighMenu_sub();						
		break;

	case FMID_EXITMENU:				
		FOsdMenuClose();	
		if( DisplayInputHold ) 		FOsdDisplayInput();
		else if( IsAudioMuteOn() ) 	DisplayMuteInfo();
		break;
	}
	return ret;
}


BYTE FOsdMenuGetNextCursorItem( CODE_P struct DefineMenu *DMp, BYTE itemno, BYTE flag)
{
	BYTE new_cursor;

	switch( flag ) {
	case FOSD_UP:	new_cursor = FOsdMenuGetUpperItem(  itemno );		break;
	case FOSD_DN:	new_cursor = FOsdMenuGetLowerItem( DMp, itemno );	break;
	}
	return new_cursor;
}

//============== Public functions =================================================
//CODE BYTE TryAgainMsg[]={ "Try again."};

static BYTE SaveValue1(void)
{
	switch( MenuChild[cursor_item].Type )	{
		case MIT_NUM:
	       	switch ( MenuChild[cursor_item].Id ) {
				case FMID_SLEEP_TIMER:		
					SetSleepTimer( OSDItemValue );	
					return 1;	//break;
			}
			//ret = 1;
			break;
	
		default:	break;
	}

	return 0;	
}

static void ClearValueWin(void)
{
	SaveValue1();
	//FOsdWinEnable(FOSDMENU_BARWIN, FALSE);
	val_osdwin = 0xff;
	OnChangingValue = 0;
}

void FOsdMenuMoveCursor(BYTE key_flag )
{
	BYTE new_cursor, old_cursor;
	WORD dat;
	BYTE i;

	#ifdef DEBUG_OSD
	dPrintf("\nFOsdMenuMoveCursor(%bx) cursor_item:%bx", key_flag, cursor_item);
	#endif

	if( MenuChild[ cursor_item ].Id == FMID_PC_COLOR_TEMP && EE_Read( PC_COLOR_DATA ) != 3 ) return; // User
	if( MenuChild[ cursor_item ].Id == FMID_VIDEO_MODE && EE_Read( EEP_VIDEOMODE ) != 3 ) return; // User

	new_cursor = FOsdMenuGetNextCursorItem( MenuChild, cursor_item, key_flag );	// Get item to move
		
	if( new_cursor ==NIL ) {		
		if( key_flag == FOSD_UP ) {
			for(i=0; i<8; ) {
				new_cursor=i;
				if( (i=FOsdMenuGetLowerItem(MenuChild, i) ) == NIL) 
					break;
			}
		}
		else
			new_cursor = 0;
	}

	if( OnChangingValue )
		ClearValueWin();

	if( OSDMenuLevel != 1) 
		FOsdMenuClearCursor();

	old_cursor = cursor_item;
	cursor_item = new_cursor;
	dPrintf(" OldItemNo:%bd, NewItemNumber:%bd", old_cursor, new_cursor );

	if(OSDMenuLevel == 1) { 
		WaitVBlank(1);
		dat = FOsdWinGetY(FOSDMENU_CURSORWIN);
		dat += (cursor_item-old_cursor)*((FONT_H+2)*2);
		FOsdWinSetY(FOSDMENU_CURSORWIN,dat);
		FOsdWinSetOsdRamStartAddr(FOSDMENU_CURSORWIN,cursor_item*2);

		FOsdMenuDisplayMenu();
	}
	else {
		if( MenuChild[ old_cursor ].Id == FMID_OSDLANG)
			FOsdMenuDrawList(0); //		Recover Attr. and menu Data list
		FOsdMenuDisplayCursor();
	}
}

//desc:increase/decrease a value
void FOsdMenuValueUpDn(BYTE flag)
{
	CODE_P BYTE *ptr;
	WORD val;
	WORD addr;
//	BYTE len1;

	addr = FOSDMENU_SUBADDR;
	addr = addr + MAX_DESC + (cursor_item+1)*MenuFormat->width;

	dPrintf("\nFOsdMenuValueUpDn(%bd) type:%bx",flag,MenuChild[cursor_item].Type);

	switch ( MenuChild[cursor_item].Type ) {
	case MIT_BAR:	
		val = FOsdMenuSetItemValue(MenuChild[cursor_item].Id, flag );
		DisplayViewBar( addr, val);
		break;
	
	case MIT_NUM:
		ptr = MenuChild[cursor_item].TypeData;
		switch ( MenuChild[cursor_item].Id ) {

#ifdef PAL_TV	//-----------------------------------------------
		case FMID_MANUAL_TUNE_CHANNEL:
				ManualTune(flag);
				break;
#endif			//-----------------------------------------------
		default:
				val = FOsdMenuSetItemValue(MenuChild[cursor_item].Id, flag);
				DrawNum( addr, (CODE_P struct NumType *)ptr, val );
				break;
		}		
		break;
	/* case TIME:
				//tptr = (CODE_P struct TimeType *)MenuChild[cursor_item].TypeData;
				//val = FOsdMenuSetItemValue( (val_index==HOUR ? MenuChild[cursor_item].Id + 1 : MenuChild[cursor_item].Id ) , flag);
				//if( val_index == HOUR )
				//	DrawTime( val, tptr->CursorColor, tptr->TimeColor);
				//else
				//	DrawTime( val, tptr->TimeColor,   tptr->CursorColor);

				
				ptr = MenuChild[cursor_item].TypeData;
				val = FOsdMenuSetItemValue( (val_index==HOUR ? MenuChild[cursor_item].Id + 1 : MenuChild[cursor_item].Id ) , flag);
				if( val_index == HOUR )
					DrawTime( val, 
					((CODE_P struct TimeType *)ptr)->CursorColor, 
					((CODE_P struct TimeType *)ptr)->TimeColor);
				else
					DrawTime( val, 
					((CODE_P struct TimeType *)ptr)->TimeColor,   
					((CODE_P struct TimeType *)ptr)->CursorColor);
				break;
	*/
	case MIT_E_STR:
		dPuts("\nENUM value change");
		ptr = MenuChild[cursor_item].TypeData;
		val = FOsdMenuSetItemValue( MenuChild[cursor_item].Id , flag);
		DrawStringByLang( addr, (CODE_P struct EnumType *)ptr, val );
		break;
	
	default:	
		break;
	}
}

//desc:exit FOsd Menu
void FOsdMenuClose(void)
{
	BYTE i;
	extern IDATA BYTE PcMode;

	WaitVBlank(1);
	FOsdOnOff(OFF, 1);  // with vdelay 1

	for (i=FOSDMENU_CURSORWIN; i<=FOSDMENU_TITLEWIN; i++ )
		FOsdWinEnable( i, OFF );

	DisplayedOSD &= (~(FOSD_MENU));

	//-------------
	//	????
	MenuEnd();

	return;
}
/*
#ifdef SUPPORT_OSDPOSITIONMOVE
void InitOSDMenuXY(void)
{
	int val, addx, addy;
	BYTE winno;

	addx = OSDMenuX - OSDMenuXRange.Default;
	addy = OSDMenuY - OSDMenuYRange.Default;

	#ifdef DEBUG_OSD
	dPrintf("\r\nOSDMenuX:%d, OSDMenuY:%d", (WORD)OSDMenuX, (WORD)OSDMenuY);
	dPrintf("\r\nOSDMenuYRange.Min=%d", (WORD)OSDMenuXRange.Min);
	dPrintf("\r\nOSDMenuYRange.Max=%d", (WORD)OSDMenuXRange.Max);
	dPrintf("\r\nOSDMenuYRange.Default=%d", (WORD)OSDMenuXRange.Default);
	dPrintf("\r\nOSDMenuX - OSDMenuXRange.Default=%d", (WORD)addx);
	#endif

	for( winno=FOSDMENU_CURSORWIN; winno<=FOSDMENU_TITLEWIN; winno++) {

		val = FOsdWinGetX( winno );
		SetOSDWindowX(winno,val+addx);

		val = FOsdWinGetY( winno );
		SetOSDWindowY(winno,val+addy);
	}
}
#endif
*/


//desc:start FOsd MENU
void FOsdMenuOpen(void)
{
	BYTE i;
	DECLARE_LOCAL_page;

	//--------------------------
	//
	if(TaskNoSignal_getCmd() != TASK_CMD_DONE)
		TaskNoSignal_setCmd(TASK_CMD_DONE); //turn off NoSignal Task
	if(getNoSignalLogoStatus()/*NoSignalLogoShow*/)
		RemoveLogo(); //and InitLogo2						

	ReadTW88Page(page);

	ClearOSDInfo();
	
	FOsdMenuInit();		// reset OSD window and prepare menu
	
	//--------------------
	//enable FOSDWIN
	for(i=FOSDMENU_CURSORWIN; i<=FOSDMENU_TITLEWIN; i++)
		FOsdWinEnable(i, ON);

	SetDisplayedOSD( FOSD_MENU );
	FOsdOnOff(ON, 1);	//with vdelay 1

	WriteTW88Page(page);
}


//======================================================================
//		   void InitOSDMenu()
//======================================================================
//extern
//	InputMain
//	OSDLang
//	MenuTitle
void FOsdMenuInit(void)
{
	BYTE i=0;

	OSDLang = GetOSDLangEE();

	switch( InputMain ) {
	//#ifdef SUPPORT_DTV
	//case DTV:	MenuTitle = DTVMenu;		break;
	//#endif 

	default:	MenuTitle = DefaultMenu;	break;
	}

	MenuSP = 0;								// Clear stack of menu.
	title_item = 0;
	MenuFormat = ( CODE_P struct MenuType *) MenuTitle[title_item].TypeData;

	OSDMenuLevel = 1; // OSD menu tree level
	cur_osdwin = FOSDMENU_TOPWIN;

	MenuChild = MenuTitle[title_item].Child;
	cursor_item = 0;
	
	val_osdwin = 0xff;
	val_index = 0;
	OnChangingValue = 0;

	//--- OSD Window Initialization
	#ifdef ADD_ANALOGPANEL
	if(IsAnalogOn())
		InitFOsdMenuWindow(Init_Osd_MainMenu_A);
	else	
	#endif
	InitFOsdMenuWindow(Init_Osd_MainMenu);  // Define Window Top(4), Sub(3), Bar(2), Cursor(1) 

	//--- Positioning 
	#ifdef SUPPORT_OSDPOSITIONMOVE
	i = GetOSDPositionModeEE();

	OSDMenuX = OSDPOSITIONMODE_TABLE[i][0];
	OSDMenuY = OSDPOSITIONMODE_TABLE[i][1];

	InitOSDMenuXY();	 // Default + Saved OSD Position
	#endif

	//--- Display OSD Menu
	FOsdRamMemsetAttr(FOSDMENU_MAINADDR, DEFAULT_COLOR, 5*2 );	 // All Line Attr clear by default color.
	FOsdMenuDisplayMenu(); 
}


void FOsdHighMenu(void)
{
	//Printf("\nFOsdHighMenu cursor_item:%bx  MenuChild[cursor_item].Id:%bx",cursor_item, MenuChild[cursor_item].Id);


	if( OnChangingValue ) {

		if(MenuChild[cursor_item].Id == FMID_OSDLANG) {
			CODE_P BYTE *Str;
			BYTE i;
			if(OSDLang==OSDLANG_KR) {
				//reload font
				InitFontRamByNum(FONT_NUM_PLUS_RAMFONT, 0);
			}
			else /*if()*/ {
				//reload font
				InitFontRamByNum(FONT_NUM_DEF12X18, 0);
			}
			//ReDraw Comment
			Str = Str_Comments[OSDLang];
			i=TWstrlen(Str);
			WriteStringToAddr(FOSDMENU_SUBADDR+TITLE_MAX_DESC*8, Str, i);

		}

		ClearValueWin();
		FOsdMenuDrawList(0); //		Recover Attr. and menu Data list
		FOsdMenuDisplayCursor();
		return;
	}

	FOsdMenuDoAction(MenuChild[cursor_item].PostFnId); // When exiting from the previous menu, Use to need some of action.

	if( OSDMenuLevel != 1) 
		FOsdMenuClearCursor();


	FOsdHighMenu_sub();
}

//desc: Process "SELECT" Key. It means "ENTER" key.
//extern
//	cursor_item
//	MenuChild[]
void FOsdMenuProcSelectKey(void)
{
	BYTE ret;
	WORD addr;

	if( cursor_item==NIL )			
		return;

	if( MenuChild[ cursor_item ].Id == FMID_VIDEO_HUE 
	&& DecoderReadVInputSTD()!= NTSC 
	&& DecoderReadVInputSTD()!= NTSC4 )  		
		return;

	ret = FOsdMenuDoAction(MenuChild[cursor_item].PreFnId);
	if(!ret) 
		return;

	switch ( MenuChild[cursor_item].Type ) {
	case MIT_ACTION:
		FOsdMenuDoAction(MenuChild[cursor_item].Id);
		return;

	case MIT_E_STR:		
	case MIT_BAR:
		OnChangingValue = 1; 		// Value changable label on 				
		FOsdMenuDisplayCursor();	//Display Setect Item cursor
		FOsdMenuGetItemValue(MenuChild[cursor_item].Id);
		return;
						
	case MIT_NUM:	
		OnChangingValue = 1;
		FOsdMenuDisplayCursor();
		return;

	case MIT_MENU:
	case MIT_SELMENU:
		if(OSDMenuLevel == 1) {
			addr = FOSDMENU_MAINADDR + cursor_item*MenuFormat->width + 0;
			// 3D Effect Cursor Bottom
			FOsdWinSet3DControl(FOSDMENU_CURSORWIN,(WINDOW_3D_BOTTOM|BG_COLOR_BLACK|EXTENDED_COLOR|WINDOW_3D_ENABLE|WINDOW_ENABLE));
		}

		OSDMenuLevel++;	// Down Menu tree.

		pushMenu();	// Push MenuTitle, title_item in stack.
		MenuTitle = MenuChild;
		title_item = cursor_item;

		MenuFormat = ( CODE_P struct MenuType *) MenuTitle[title_item].TypeData;				
		MenuChild = MenuTitle[title_item].Child;
		cursor_item = FOsdMenuGetCursorItem( MenuTitle[title_item].Id );

		if( cur_osdwin == FOSDMENU_TOPWIN ) // Window #4
			cur_osdwin = FOSDMENU_ITEMWIN;  // Window #3
		else
			FOsdMenuDisplayMenu();

		FOsdMenuDisplayCursor();
		break;
	}	//..switch
}

//===============================================================================
// AUDIO
//===============================================================================
DATA char 	AudioVol = 50, 
			AudioBalance = 50, 
			AudioBass = 50,
			AudioTreble = 46,
			AudioEffect = 0;
bit   AudioMuteOn=0;

#if 0	//if FOSD is linked with key, we do not support audio mute.
void ToggleAudioMute(void)
{
	if( AudioMuteOn ) 		AudioMuteOn = 0;
	else 					AudioMuteOn = 1;
}
#endif

void CheckAndClearMuteOn(void)
{
	if( AudioMuteOn ) {
		AudioMuteOn = 0;
		ClearMuteInfo();
	}
}

BYTE IsAudioMuteOn(void)
{
	return (BYTE)AudioMuteOn;
}

BYTE ChangeVol(char amount)
{
	BYTE chg=0;

	CheckAndClearMuteOn();

	if( amount>0 ) {
		if( ((AudioVol+amount) <= 100) )	AudioVol += amount;
		else								AudioVol = 100;
	}
	else {
		if( (AudioVol+amount) > 0  ) 		AudioVol += amount;
		else								AudioVol = 0;
	}

	EE_Write( EEP_AUDIOVOL, AudioVol );

	return AudioVol;
}
BYTE ChangeBalance(char amount)
{
	BYTE chg=0;

	if( amount>0 ) {
		if( ((AudioBalance+amount) <= 100) )	AudioBalance += amount;
		else									AudioBalance = 100;
	}
	else {
		if( (AudioBalance+amount) > 0  ) 		AudioBalance += amount;
		else									AudioBalance = 0;
	}

	EE_Write( EEP_AUDIOBALANCE, AudioBalance );

	return AudioBalance;
}

BYTE ChangeBass(char amount)
{
	BYTE chg=0;

	if( amount>0 ) {
		if( ((AudioBass+amount) <= 100) ) 		AudioBass += amount;
		else									AudioBass = 100;
	}
	else {
		if( (AudioBass+amount) > 0  ) 			AudioBass += amount;
		else									AudioBass = 0;
	}

	EE_Write( EEP_AUDIOBASS, AudioBass );

	return AudioBass;
}

BYTE ChangeTreble(char amount)
{
	BYTE chg=0;

	if( amount>0 ) {
		if( ((AudioTreble+amount) <= 100) ) 	AudioTreble += amount;
		else									AudioTreble = 100;
	}
	else {
		if( (AudioTreble+amount) > 0  ) 		AudioTreble += amount;
		else									AudioTreble = 0;
	}

	EE_Write( EEP_AUDIOTREBLE, AudioTreble );

	return AudioTreble;
}



/*		   for 	   Philip 07062010

//===============================================================================
//
//  Display Information of the menu
//
//===============================================================================
CODE BYTE OSDInfoStr[][26]={
	{ "   Input  :              " },
	{ "   System :              " },
	{ "   H Freq :      kHz     " },	
	{ "   V Freq :       Hz     " },
	{ "   F/W Rev:              " }, 	
	{ "" }	
};
#define OSD_INFO_SUB_X	12//9
/*
void DisplayInfo_OSD(void)
{
	BYTE i, title=1, len, inputs;
	BYTE mstr[5];

	pushMenu();
	MenuTitle = MenuChild;
	title_item = cursor_item;

	OSDMenuLevel++; 
	
	MenuChild = 0;
	cursor_item = NIL;

	FOsdMenuDrawList(1); // Display Title

	FOsdRamMemsetAttr(FOSDMENU_SUBADDR+MENU_WIDTH, OSD_INFO_COLOR, 5*MENU_WIDTH );	 // Line 1 ~ Line 5
	for(i=0; OSDInfoStr[i][0]; i++) 
		WriteStringToAddr(FOSDMENU_SUBADDR+MENU_WIDTH*(i+title), OSDInfoStr[i], MENU_WIDTH ); 

	// Display Input
	inputs = InputMain;
	for (i=1; ;i++)
		if( struct_InputSelection[i].Id==inputs )  break;
	len=TWstrlen(struct_InputSelection[i].Name);
	WriteStringToAddr(FOSDMENU_SUBADDR+MENU_WIDTH*(0+title)+OSD_INFO_SUB_X, struct_InputSelection[i].Name, len ); 

	// Display ColorSystem
	#if defined( SUPPORT_PC ) || defined( SUPPORT_DTV )
	i = InputMain;
	if( i==PC || i==DTV ){
		len=TWstrlen(GetPCInputSourceName());
		WriteStringToAddr(FOSDMENU_SUBADDR+MENU_WIDTH*(1+title)+OSD_INFO_SUB_X, GetPCInputSourceName(), len ); 
	}
	else
	#endif
	{
		len=TWstrlen(struct_VInputStd[GetVInputStdInx()].Name);
		WriteStringToAddr(FOSDMENU_SUBADDR+MENU_WIDTH*(1+title)+OSD_INFO_SUB_X, struct_VInputStd[GetVInputStdInx()].Name, len ); 
	}
	//IHF
	MeasureAndWait(3);

	ltoa_K(GetIHF(), mstr, 1);
	len=TWstrlen(mstr);
	WriteStringToAddr(FOSDMENU_SUBADDR+MENU_WIDTH*(2+title)+OSD_INFO_SUB_X, mstr, len ); 

	TWutoa(GetIVF(),mstr,NUM_RADIX);
	len=TWstrlen(mstr);
	WriteStringToAddr(FOSDMENU_SUBADDR+MENU_WIDTH*(3+title)+OSD_INFO_SUB_X, mstr, len ); 

	//FWRev
	i = ReadEEP(EEP_FWREV_MAJOR);
	mstr[0] = i + '0';
	mstr[1] = '.';
	i = ReadEEP(EEP_FWREV_MINOR);
	mstr[2] = (i/0x10) + '0';
	mstr[3] = (i%0x10) + '0';
	mstr[4] = '\0';
	
	WriteStringToAddr(FOSDMENU_SUBADDR+MENU_WIDTH*(4+title)+OSD_INFO_SUB_X, mstr, 4 ); 

}
*/

/*
#ifdef PAL_TV
void DrawFraction( CODE_P struct NumType *nptr, DWORD val, BYTE fp )
{
	BYTE mstr[6], len;

	ltoa_K( val, mstr, fp );
	len = TWstrlen(mstr);
	CopyStrToOSDWindow(cur_osdwin, 0, 0, len, nptr->NumColor, mstr);
	ClearOSDLine(cur_osdwin, len, 0, nptr->width-len, nptr->NumColor);
}
#endif

BYTE OnChangingValue(void)
{
	return ( (val_osdwin==0xff  ) ? 0 : 1 );	//OSDItemValue==0xffff: password error
}
// Not used yet *******************************************
BYTE OnChangingTime(void)
{
	if ( val_osdwin==0xff )						return 0 ;
	if ( MenuChild[cursor_item].Type==TIME )	return 1;
	else										return 0;

}
*/

//==========================================
#endif //.. SUPPORT_FOSD_MENU
//==========================================


