// FOSDString.h
#ifndef __FOSD_STRING_H__
#define __FOSD_STRING_H__

#define		MAX_DESC			12 //14	

//==================================================================================
//
//   Choice Menu String define in osd menu.
//
//==================================================================================
// Form
// 
//struct EnumType {
//	BYTE    Sz;
//	BYTE    ByLang;
//	BYTE	Str[15];			// doesn't matter size of the string !!!
//};


#define OSDLANG_EN		0
#define OSDLANG_FR		1
#define OSDLANG_DE		2
#define OSDLANG_SP		3	
#define OSDLANG_KR		4
#define MAX_LANG		5	


extern CODE BYTE OSDLangString[];
extern CODE BYTE ModeChoice[];
extern CODE BYTE VInputStdChoice[];

#ifdef WIDE_SCREEN
extern CODE BYTE WideScreenChoice[];
#endif

#ifdef SUPPORT_OSDPOSITIONMOVE
extern CODE BYTE OsdPositionModeChoice[];
#endif


extern CODE BYTE CaptionChoice[];

//--------------------------------------------------------------------------------

extern CODE BYTE 	*Enter_Password_Str[];				   
extern CODE BYTE 	*CheckPCCableStr[];
extern CODE BYTE 	*CheckDTVCableStr[];
extern CODE BYTE 	*OutOfRangeStr[];
extern CODE BYTE 	*StrVolume[];
extern CODE BYTE 	*MuteOnStr[];
//--------------------------------------------------------------------------------
extern CODE BYTE 	*Str_Input[];
extern CODE BYTE 	*Str_Picture[];
extern CODE BYTE 	*Str_Sound[];
extern CODE BYTE 	*Str_Utility[];
extern CODE BYTE 	*Str_ColorTemp[];

extern CODE BYTE 	*Str_Comments[];
//============================================================================================
//  OSD Menu String Table
extern CODE BYTE 	*Str_Composite[];

#ifdef SUPPORT_CVBS2
extern CODE BYTE 	*Str_Composite2[];
#endif
#ifdef SUPPORT_SVIDEO
extern CODE BYTE 	*Str_Svideo[];
#endif
#ifdef SUPPORT_COMPONENT
extern CODE BYTE 	*Str_Component[];
#endif
#if defined(SUPPORT_BT656_LOOP) || defined(SUPPORT_DTV656) || defined(SUPPORT_BT656_OUTPUT)
extern CODE BYTE 	*Str_656[];
extern CODE BYTE 	*Str_Panel[];
extern CODE BYTE 	*Str_Off[];
#endif
#if defined(SUPPORT_LVDSRX)
extern CODE BYTE 	*Str_LVDS[];
#endif
#ifdef SUPPORT_PC
extern CODE BYTE 	*Str_Pc[];
#endif
#ifdef SUPPORT_DVI
extern CODE BYTE 	*Str_DVI[];
#endif
#if defined(SUPPORT_HDMI)
extern CODE BYTE	*Str_HDMI[];
#endif

#ifdef SUPPORT_PC
extern CODE BYTE 	*Str_VPosition[];
extern CODE BYTE 	*Str_HPosition[];
#endif

#ifdef SUPPORT_OSDPOSITIONMOVE
extern CODE BYTE 	*Str_OSDPosition[];
#endif
extern CODE BYTE 	*Str_OSDLang[];
extern CODE BYTE 	*Str_SleepTimer[];
extern CODE BYTE 	*Str_VInputStd[];
extern CODE BYTE 	*Str_Information[];
extern CODE BYTE 	*Str_Volume[];
extern CODE BYTE 	*Str_Treble[];
extern CODE BYTE 	*Str_Bass[];
extern CODE BYTE 	*Str_Balance[];
extern CODE BYTE 	*Str_Red[];
extern CODE BYTE 	*Str_Blue[];
extern CODE BYTE 	*Str_Green[];
extern CODE BYTE 	*Str_Hue[];
extern CODE BYTE 	*Str_Backlight[];
extern CODE BYTE 	*Str_Sharpness[];
extern CODE BYTE 	*Str_Contrast[];
extern CODE BYTE 	*Str_Brightness[];
extern CODE BYTE 	*Str_Saturation[];
extern CODE BYTE 	*Str_Reset[];

#ifdef SUPPORT_PC
extern CODE BYTE 	*Str_Phase[];
extern CODE BYTE 	*Str_Clock[];
extern CODE BYTE 	*Str_AutoColor[];
extern CODE BYTE 	*Str_AutoAdjust[];
#endif

#ifdef WIDE_SCREEN
extern CODE BYTE 	*Str_WideScreen[];
#endif

#endif //..__FOSD_STRING_H__
