/**
 * @file
 * FOsdString.c 
 * @author Harry Han
 * @author YoungHwan Bae
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	FontOSD String for Multi-Lenguage
*/
#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "Global.h"

#include "SOsd.h"
#include "FOsd.h"
#include "FosdString.h"

#ifndef SUPPORT_FOSD_MENU
//==========================================
//----------------------------
//Trick for Bank Code Segment
//----------------------------
CODE BYTE DUMMY_FOSD_STRING_CODE;
void Dummy_FosdString_func(void)
{
	BYTE temp;
	temp = DUMMY_FOSD_STRING_CODE;
}
//==========================================
#else //..SUPPORT_FOSD_MENU

// OSDString.c
//==============================================================================
//
// String by Language
//
// Lang1 = English
// Lang2 = Frensh
// Lanf3 = Deutch
// Lang4 = Spanish
// Lang5 = Korean
//==============================================================================
//
//  Display String by language in information
//
//--------------------------------------------------------------------------------
CODE BYTE Enter_Password_Str_E[] = {' ',' ',' ',' ','E','n','t','e','r',' ','P','I','N',' ',':',' ','-','-','-','-',0};
CODE BYTE Enter_Password_Str_F[] = {' ',' ',' ','E','n','t','r','e','r',' ','P','I','N',' ',':',' ','-','-','-','-',0};
CODE BYTE Enter_Password_Str_D[] = {' ',' ','E','i','n','g','a','b','e',' ','P','I','N',' ',':',' ','-','-','-','-',0};
CODE BYTE Enter_Password_Str_S[] = {' ',' ',' ','E','n','t','r','e','r',' ','P','I','N',' ',':',' ','-','-','-','-',0};
CODE BYTE Enter_Password_Str_K[] = {' ',' ',FONT_RAM,CH_KR_BI,CH_KR_MIL,CH_KR_BEON,CH_KR_HO,CH_KR_REUL,CH_KR_SPACE,CH_KR_IP,CH_KR_RYEOK,CH_KR_SPACE,CH_KR_HA,CH_KR_SE,CH_KR_YO,FONT_ROM,':',' ','-','-','-','-',0};							   

//--------------------------------------------------------------------------------
CODE BYTE CheckPCCableStr_E[] = {'C','h','e','c','k',' ','P','C',' ','C','a','b','l','e',0};
CODE BYTE CheckPCCableStr_K[] = {FONT_RAM,CH_KR_IP,CH_KR_RYEOK,CH_KR_SIN,CH_KR_HO,CH_KR_REUL,CH_KR_SPACE,CH_KR_HWAK,CH_KR_IN,CH_KR_HA,CH_KR_SE,CH_KR_YO,FONT_ROM,0};
//--------------------------------------------------------------------------------

CODE BYTE CheckDTVCableStr_E[] = {'C','h','e','c','k',' ','D','T','V',' ','C','a','b','l','e',0};
CODE BYTE CheckDTVCableStr_K[] = {FONT_RAM,CH_KR_IP,CH_KR_RYEOK,CH_KR_SIN,CH_KR_HO,CH_KR_REUL,CH_KR_SPACE,CH_KR_HWAK,CH_KR_IN,CH_KR_HA,CH_KR_SE,CH_KR_YO,FONT_ROM,0};

//--------------------------------------------------------------------------------
CODE BYTE OutOfRangeStr_E[] = {'O','u','t',' ','O','f',' ','R','a','n','g','e',0};
CODE BYTE OutOfRangeStr_K[] = {FONT_RAM,CH_KR_JI,CH_KR_WON,CH_KR_HA,CH_KR_JI,CH_KR_SPACE,CH_KR_ANH,CH_KR_NEUN,CH_KR_SPACE,CH_KR_MO,CH_KR_DEU,FONT_ROM,0};

//--------------------------------------------------------------------------------

CODE BYTE StrVolume_E[] = {CH_SPK_VOL,CH_SPK_VOL+1,'V','o','l','u','m','e',0};								
CODE BYTE StrVolume_F[] = {CH_SPK_VOL,CH_SPK_VOL+1,'V','o','l','u','m','e',0};								
CODE BYTE StrVolume_D[] = {CH_SPK_VOL,CH_SPK_VOL+1,'L', 'a', 'u', 't', 's', 't', 'a', 'r', 'k', 'e',0};								
CODE BYTE StrVolume_S[] = {CH_SPK_VOL,CH_SPK_VOL+1,'V','o','l','u','m','e','n',0};								
CODE BYTE StrVolume_K[] = {CH_SPK_VOL,CH_SPK_VOL+1,FONT_RAM,CH_KR_EUM,CH_KR_RYANG,FONT_ROM,0};

//--------------------------------------------------------------------------------
CODE BYTE MuteOnStr_E[] = {'M','u','t','e',0};
CODE BYTE MuteOnStr_K[] = {FONT_RAM,CH_KR_SPACE,CH_KR_JO,CH_KR_YONG,CH_KR_HI,FONT_ROM,0};

//--------------------------------------------------------------------------------
CODE BYTE AutoAdjustInfo[]=	{"   Auto Adjusting ....   "};
							//012345678901234


/*================================================================================*/
/*
/* Title String in OSD menu                                                         
/*
/*=================================================================================*/
//#define		TITLEMAXDEC			25	
///* Title String Length :
/*================================================================================*/
CODE BYTE Str_Input_E[] = {'I','n','p','u','t',0};
CODE BYTE Str_Input_F[] = {'S','o','u','r','c','e',' ','d',CH_A_ACUTE, 'e', 'n', 't', 'r', CH_U_ACUTE, 'e',0};
CODE BYTE Str_Input_D[] = {'E', 'i', 'n', 'g', 'a', 'n', 'g', 's', 'q', 'u', 'e', 'l', 'l', 'e',0};
CODE BYTE Str_Input_S[] = {'F', 'u', 'e', 'n', 't', 'e', ' ', 'E', 'n', 't', 'r', 'a', 'd', 'e', 0};
CODE BYTE Str_Input_K[] = {FONT_RAM,CH_KR_IP,CH_KR_RYEOK,CH_KR_SPACE,CH_KR_SEON,CH_KR_TAEK,FONT_ROM,0};

//--------------------------------------------------------------------------------
CODE BYTE Str_Picture_E[] = {'P','i','c','t','u','r','e',0};
CODE BYTE Str_Picture_F[] = {'E', 'c', 'r', 'a', 'n', 0};
CODE BYTE Str_Picture_D[] = {'B', 'i', 'l', 'd',0};
CODE BYTE Str_Picture_S[] = {'P', 'a', 'n', 't', 'a', 'l', 'l', 'a',0};
CODE BYTE Str_Picture_K[] = {FONT_RAM,CH_KR_HWA,CH_KR_MYEON,CH_KR_SPACE,CH_KR_JO,CH_KR_JEONG,FONT_ROM,0};

//--------------------------------------------------------------------------------
CODE BYTE Str_Sound_E[] = {'A','u','d','i','o',0};
CODE BYTE Str_Sound_S[] = {'S','o','n','i','d','o',0};
CODE BYTE Str_Sound_K[] = {FONT_RAM,CH_KR_EUM,CH_KR_HYANG,CH_KR_SPACE,CH_KR_JO,CH_KR_JEONG,FONT_ROM,0};

//--------------------------------------------------------------------------------
CODE BYTE Str_Utility_E[] = {'U','t','i','l','i','t','y',0};
CODE BYTE Str_Utility_F[] = {'U', 't', 'i', 'l', 'i', 't', 'a', 'i', 'r', 'e',0};
CODE BYTE Str_Utility_D[] = {'F', 'u', 'n', 'k', 't', 'i', 'o', 'n',0};
CODE BYTE Str_Utility_S[] = {'U', 't', 'i', 'l', 'i', 'd', 'a', 'd',0};
CODE BYTE Str_Utility_K[] = {FONT_RAM,CH_KR_GI,CH_KR_NEUNG,CH_KR_SPACE,CH_KR_SEON,CH_KR_TAEK,FONT_ROM,0};

//--------------------------------------------------------------------------------
CODE BYTE Str_ColorTemp_E[] = {'C','o','l','o','r',' ','T','e','m','p',0};
CODE BYTE Str_ColorTemp_F[] = {'T', 'e', 'm', 'p', '.', ' ', 'C', 'o', 'u', 'l', 'e', 'u', 'r',0};
CODE BYTE Str_ColorTemp_D[] = {'F', 'a', 'r', 'b', 't', 'e', 'm', 'p', 'e', 'r', 'a', 't', 'u', 'r',0};
CODE BYTE Str_ColorTemp_S[] = {'T', 'e', 'm', 'p', '.', ' ',  'd', 'e', ' ', 'C', 'o', 'l', 'o', 'r',0};
CODE BYTE Str_ColorTemp_K[] = {FONT_RAM,CH_KR_SAEK,CH_KR_ON,CH_KR_DO,CH_KR_SPACE,CH_KR_JO,CH_KR_JEONG,FONT_ROM,0};


//--------------------------------------------------------------------------------
CODE BYTE Str_Comments_E[] = {'E','x','i','t',':','M','E','N','U',' ','M','o','v','e',':',CH_ARROW_UP,CH_ARROW_DOWN,' ','S','e','l',':',CH_ARROW_RIGHT,0};
CODE BYTE Str_Comments_K[] = {FONT_RAM,CH_KR_NA,CH_KR_GAM,FONT_ROM,':','M','E','N','U',' ',' ',' ',
								FONT_RAM,CH_KR_I,CH_KR_DONG,FONT_ROM,':',CH_ARROW_UP,CH_ARROW_DOWN,	' ',' ',' ',' ',
								FONT_RAM,CH_KR_SEON,CH_KR_TAEK,FONT_ROM,':',CH_ARROW_RIGHT,0};
								 
//--------------------------------------------------------------------------------
#ifdef PAL_TV
CODE BYTE Str_Comments1[] = {'M','o','v','e',':',CH_SYMBOL_07F,CH_SYMBOL_07F,' ','S','k','i','p','/','A','d','d',':',CH_SYMBOL_07F,' ','E','d','i','t',':',CH_SYMBOL_07F,0};
//CODE BYTE Color_Comments1[] = {FG_COLOR_RED|TITLE_BACK_COLOR,FG_COLOR_GREEN|TITLE_BACK_COLOR,
// 							   COMMENT_COLOR,COMMENT_COLOR,COMMENT_COLOR,COMMENT_COLOR,COMMENT_COLOR,COMMENT_COLOR,COMMENT_COLOR,COMMENT_COLOR,
//							   COMMENT_COLOR,COMMENT_COLOR,
//							   FG_COLOR_YELLOW|TITLE_BACK_COLOR,
//							   COMMENT_COLOR,COMMENT_COLOR,COMMENT_COLOR,COMMENT_COLOR,COMMENT_COLOR,COMMENT_COLOR,
//							   FG_COLOR_CYAN|TITLE_BACK_COLOR
//								};			
#endif

/*================================================================================*/
/*                                                                                */
/* Decsription String in OSD menu                                                 */
/*                                                                                */
//==================================================================================
//MenuStr
///* Menu String Max Length :
/*================================================================================*/
CODE BYTE Str_Composite_E[] = {'C','o','m','p','o','s','i','t','e',0};
CODE BYTE Str_Composite2_E[] = {'C','o','m','p','o','s','i','t','e','2',0};

#ifdef SUPPORT_SVIDEO
CODE BYTE Str_Svideo_E[] = {'S','-','V','i','d','e','o',0};
#endif

	#ifdef SUPPORT_COMPONENT
CODE BYTE Str_Component_E[] = {'C','o','m','p','o','n','e','n','t',0};
	#endif

	#ifdef SUPPORT_BT656_LOOP
CODE BYTE Str_Dtv_E[] = {'6','5','6',0};
CODE BYTE Str_Panel_E[] = {'P','a','n','e','l',0};
CODE BYTE Str_Off_E[] = {'O','f','f',0};
	#endif
	#if defined(SUPPORT_LVDSRX)
CODE BYTE Str_LVDS_E[] = { 'L','V','D','S',0};
	#endif

	#ifdef SUPPORT_PC
CODE BYTE Str_Pc_E[] = {'P','C',0};
	#endif

	#ifdef SUPPORT_DVI
CODE BYTE Str_DVI_E[] = {'D','V','I',0};
	#endif
	#if defined(SUPPORT_HDMI)
CODE BYTE Str_HDMI_E[] = {'H','D','M','I',0};
	#endif


//--------------------------------------------------------------------------------
#ifdef SUPPORT_PC
CODE BYTE Str_VPosition_E[] = {'V',' ','P','o','s','i','t','i','o','n',0};
CODE BYTE Str_VPosition_F[] = {'P','o','s','i','t','i','o','n',' ','V','.',0};
CODE BYTE Str_VPosition_D[] = {'V',' ','P','o','s','i','t','i','o','n',0};
CODE BYTE Str_VPosition_S[] = {'P','o','s','i','c','i',CH_O_ACUTE,'n',' ','V','.',0};
CODE BYTE Str_VPosition_K[] = {FONT_RAM,CH_KR_HWA,CH_KR_MYEON,CH_KR_SPACE,CH_KR_SU,CH_KR_JIK,CH_KR_SPACE,CH_KR_WI,CH_KR_CHI,CH_KR_SPACE,CH_KR_JO,CH_KR_JEONG,FONT_ROM,0};

//--------------------------------------------------------------------------------
CODE BYTE Str_HPosition_E[] = {'H',' ','P','o','s','i','t','i','o','n',0};
CODE BYTE Str_HPosition_F[] = {'P','o','s','i','t','i','o','n',' ','H','.',0};
CODE BYTE Str_HPosition_D[] = {'H',' ','P','o','s','i','t','i','o','n',0};
CODE BYTE Str_HPosition_S[] = {'P','o','s','i','c','i',CH_O_ACUTE,'n',' ','H','.',0};
CODE BYTE Str_HPosition_K[] = {FONT_RAM,CH_KR_HWA,CH_KR_MYEON,CH_KR_SPACE,CH_KR_SU,CH_KR_PYEONG,CH_KR_SPACE,CH_KR_WI,CH_KR_CHI,CH_KR_SPACE,CH_KR_JO,CH_KR_JEONG,FONT_ROM,0};
#endif
//--------------------------------------------------------------------------------
#ifdef SUPPORT_OSDPOSITIONMOVE
CODE BYTE Str_OSDPosition_E[] =	{'O','S','D',' ','P','o','s','i','t','i','o','n',0};
CODE BYTE Str_OSDPosition_S[] =	{'O','S','D',' ','P','o','s','i','c','i',CH_O_ACUTE,'n',0};
CODE BYTE Str_OSDPosition_K[] =	{'O','S','D',' ',FONT_RAM,CH_KR_WI,CH_KR_CHI,CH_KR_SPACE,CH_KR_SEON,CH_KR_TAEK,FONT_ROM,0};
#endif

//--------------------------------------------------------------------------------
CODE BYTE Str_OSDLang_E[] = {'L','a','n','g','u','a','g','e',0};
CODE BYTE Str_OSDLang_F[] = {'L', 'a', 'n', 'g', 'u', 'e',0};
CODE BYTE Str_OSDLang_D[] = {'S', 'p', 'r', 'a', 'c', 'h', 'e',0};
CODE BYTE Str_OSDLang_S[] =	{'I', 'd', 'i', 'o', 'm', 'a',0};
CODE BYTE Str_OSDLang_K[] = {FONT_RAM,CH_KR_EON,CH_KR_EO,CH_KR_SPACE,CH_KR_SEON,CH_KR_TAEK,FONT_ROM,0};

//--------------------------------------------------------------------------------
CODE BYTE Str_SleepTimerE[] = {'S','l','e','e','p',' ','T','i','m','e','r',0};
CODE BYTE Str_SleepTimerK[] = {FONT_RAM,CH_KR_KKEO,CH_KR_JIM,CH_KR_SPACE,CH_KR_SI,CH_KR_GAN,CH_KR_SPACE,CH_KR_SEOL,CH_KR_JEONG,FONT_ROM,0};

//--------------------------------------------------------------------------------
CODE BYTE Str_VInputStd_E[] = {'I','n','p','u','t',' ','S','t','d','.',0};

//--------------------------------------------------------------------------------
CODE BYTE Str_Information_E[] =	{'I','n','f','o','r','m','a','t','i','o','n',0};

//--------------------------------------------------------------------------------
CODE BYTE Str_Volume_E[] = {'V','o','l','u','m','e',0};
CODE BYTE Str_Volume_D[] = {'V','o','l','u','m','e','n',0};
CODE BYTE Str_Volume_S[] = {'L', 'a', 'u', 't', 's', 't', 'a', 'r', 'k', 'e',0};
CODE BYTE Str_Volume_K[] = {FONT_RAM,CH_KR_EUM,CH_KR_RYANG,CH_KR_SPACE,CH_KR_JO,CH_KR_JEONG,FONT_ROM,0};
//--------------------------------------------------------------------------------

CODE BYTE Str_Treble_E[] = {'T','r','e','b','l','e',0};
CODE BYTE Str_Treble_F[] = {'A','i','g','u','e','s',0};
CODE BYTE Str_Treble_D[] = {'H','o','h','e','n',0};
CODE BYTE Str_Treble_S[] = {'A','g','u','d','o','s',0};
CODE BYTE Str_Treble_K[] = {FONT_RAM,CH_KR_GO,CH_KR_EUM,CH_KR_SPACE,CH_KR_JO,CH_KR_JEONG,FONT_ROM,0};
//--------------------------------------------------------------------------------

CODE BYTE Str_Bass_E[] = {'B','a','s','s',0};
CODE BYTE Str_Bass_F[] = {'B','a','s','s','e','s',0};
CODE BYTE Str_Bass_D[] = {'T','i','e','f','e','n',0};
CODE BYTE Str_Bass_S[] = {'G','r','a','v','e','s',0};
CODE BYTE Str_Bass_K[] = {FONT_RAM,CH_KR_JEO,CH_KR_EUM,CH_KR_SPACE,CH_KR_JO,CH_KR_JEONG,FONT_ROM,0};
//--------------------------------------------------------------------------------
CODE BYTE Str_Balance_E[] = {'B','a','l','a','n','c','e',0};
CODE BYTE Str_Balance_F[] = {'B','a','l','a','n','c','e',0};
CODE BYTE Str_Balance_D[] =	{'B','a','l','a','n','c','e',0};
CODE BYTE Str_Balance_S[] =	{'B','a','l','a','n','c','e',0};
CODE BYTE Str_Balance_K[] = {FONT_RAM,CH_KR_EUM,CH_KR_GYUN,CH_KR_HYEONG,CH_KR_SPACE,CH_KR_JO,CH_KR_JEONG,FONT_ROM,0};

//--------------------------------------------------------------------------------
CODE BYTE Str_Red_E[] = {'R','e','d',0};
CODE BYTE Str_Red_F[] =	{'R','o','u','g','e',0};
CODE BYTE Str_Red_D[] = {'R','o','t',0};
CODE BYTE Str_Red_S[] = {'R','o','j','o',0};
CODE BYTE Str_Red_K[] = {FONT_RAM,CH_KR_JEOK,CH_KR_SAEK,CH_KR_SPACE,CH_KR_JO,CH_KR_JEONG,FONT_ROM,0};

//--------------------------------------------------------------------------------
CODE BYTE Str_Green_E[] = {'G','r','e','e','n',0};
CODE BYTE Str_Green_F[] = {'V','e','r','t',0};
CODE BYTE Str_Green_D[] = {'G','r','e','n',0};
CODE BYTE Str_Green_S[] = {'V', 'e', 'r', 'd', 'e',0};
CODE BYTE Str_Green_K[] = {FONT_RAM,CH_KR_NOK,CH_KR_SAEK,CH_KR_SPACE,CH_KR_JO,CH_KR_JEONG,FONT_ROM,0};

//--------------------------------------------------------------------------------
CODE BYTE Str_Blue_E[] = {'B','l','u','e',0};
CODE BYTE Str_Blue_F[] = {'B','l','e','u',0};
CODE BYTE Str_Blue_D[] = {'B','l','a','u',0};
CODE BYTE Str_Blue_S[] = {'A','z','u','l',0};
CODE BYTE Str_Blue_K[] = {FONT_RAM,CH_KR_CHEONG,CH_KR_SAEK,CH_KR_SPACE,CH_KR_JO,CH_KR_JEONG,FONT_ROM,0};

//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
CODE BYTE Str_Hue_E[] = {'H','u','e',0};
CODE BYTE Str_Hue_F[] = {'T', 'o', 'n', 'a', 'l', 'i', 't', CH_E_ACUTE,0};
CODE BYTE Str_Hue_D[] = {'F', 'a', 'r', 'b', 'e',0};
CODE BYTE Str_Hue_S[] = {'T', 'o', 'n', 'a', 'l', 'i', 'd', 'a', 'd',0};
CODE BYTE Str_Hue_K[] =	{FONT_RAM,CH_KR_SAEK,CH_KR_WI,CH_KR_SANG,CH_KR_SPACE,CH_KR_JO,CH_KR_JEONG,FONT_ROM,0};

//--------------------------------------------------------------------------------
CODE BYTE Str_Sharpness_E[] = {'S','h','a','r','p','n','e','s','s',0};
CODE BYTE Str_Sharpness_F[] = {'A', 'c', 'u', 'i', 't', CH_E_ACUTE,0};
CODE BYTE Str_Sharpness_D[] = {'S', 'c', 'h', 'e', 'r','f', 'e',0};
CODE BYTE Str_Sharpness_S[] = {'A', 'g', 'u', 'd', 'e', 'z', 'a',0};
CODE BYTE Str_Sharpness_K[] = {FONT_RAM,CH_KR_SEON,CH_KR_MYEONG,CH_KR_DO,CH_KR_SPACE,CH_KR_JO,CH_KR_JEONG,FONT_ROM,0};

//--------------------------------------------------------------------------------
CODE BYTE Str_Contrast_E[] = {'C','o','n','t','r','a','s','t',0};
CODE BYTE Str_Contrast_F[] = {'C','o','n','t','r','a','s','t','e',0};
CODE BYTE Str_Contrast_D[] = {'K','o','n','t','r','a','s','t',0};
CODE BYTE Str_Contrast_S[] = {'C','o','n','t','r','a','s','t','e',0};
CODE BYTE Str_Contrast_K[] = {FONT_RAM,CH_KR_MYEONG,CH_KR_AM,CH_KR_SPACE,CH_KR_JO,CH_KR_JEONG,FONT_ROM,0};

//--------------------------------------------------------------------------------
CODE BYTE Str_Brightness_E[] = {'B','r','i','g','h','t','n','e','s','s',0};
CODE BYTE Str_Brightness_F[] = {'L', 'u', 'm', 'i', 'n', 'o', 's', 'i', 't', CH_O_ACUTE,0};
CODE BYTE Str_Brightness_D[] = {'H', 'e', 'l', 'l', 'i', 'g', 'k', 'e', 'i', 't',0};
CODE BYTE Str_Brightness_S[] = {'B', 'r', 'i', 'l', 'l', 'o',0};
CODE BYTE Str_Brightness_K[] = {FONT_RAM,CH_KR_BALK,CH_KR_GI,CH_KR_SPACE,CH_KR_JO,CH_KR_JEONG,FONT_ROM,0};

//--------------------------------------------------------------------------------
CODE BYTE Str_Backlight_E[] = {'B','a','c','k','l','i','g','h','t',0};
//CODE BYTE Str_Backlight_F[] = {'B','a','c','k','l','i','g','h','t',0};
//CODE BYTE Str_Backlight_D[] = {'B','a','c','k','l','i','g','h','t',0};
//CODE BYTE Str_Backlight_S[] = {'B','a','c','k','l','i','g','h','t',0};
//CODE BYTE Str_Backlight_K[] = {'B','a','c','k','l','i','g','h','t',0};

//--------------------------------------------------------------------------------
CODE BYTE Str_Saturation_E[] = {'S','a','t','u','r','a','t','i','o','n',0};
CODE BYTE Str_Saturation_D[] = {'S', 'e', 't', 't', 'i', 'g', 'u', 'n', 'g',0};
CODE BYTE Str_Saturation_S[] = {'S', 'a', 't', 'u', 'r', 'a', 'c', 'i', CH_O_ACUTE, 'n',0};
CODE BYTE Str_Saturation_K[] = {FONT_RAM,CH_KR_SAEK,CH_KR_NONG,CH_KR_DO,CH_KR_SPACE,CH_KR_JO,CH_KR_JEONG,FONT_ROM,0};

//--------------------------------------------------------------------------------
CODE BYTE Str_Reset_E[] = {'R','e','s','e','t',0};
CODE BYTE Str_Reset_F[] = {'D', CH_E_ACUTE, 'f', 'a', 'u', 't',0};
CODE BYTE Str_Reset_D[] = {'G', 'r', 'u', 'n', 'd', 'e', 'i', 'n', 's', 't', 'e', 'l', 'u', 'g',0};
CODE BYTE Str_Reset_S[] = {'P', 'r', 'e', 'd', 'e', 't', 'e', 'r', 'm', 'i', 'n', 'a', 'c', 'i',CH_O_ACUTE, 'n',0};
CODE BYTE Str_Reset_K[] = {FONT_RAM,CH_KR_CHO,CH_KR_GI,CH_KR_HWA,FONT_ROM,0};

//--------------------------------------------------------------------------------

#ifdef SUPPORT_PC
//--------------------------------------------------------------------------------
CODE BYTE Str_Phase_E[] = {'P','h','a','s','e',0};
CODE BYTE Str_Phase_S[] = {'F','a','s','e',0};

//--------------------------------------------------------------------------------
CODE BYTE Str_Clock_E[] = {'C','l','o','c','k',0};
CODE BYTE Str_Clock_F[] = {'H', 'o', 'r', 'l', 'o', 'g', 'e',0};
CODE BYTE Str_Clock_D[] = {'T', 'a', 'k', 't',0};
CODE BYTE Str_Clock_S[] = {'R', 'e', 'l', 'o', 'j',0};

//--------------------------------------------------------------------------------

CODE BYTE Str_AutoColor_E[] = {'A','u','t','o',' ','C','o','l','o','r',0};
CODE BYTE Str_AutoColor_K[] = {FONT_RAM,CH_KR_JA,CH_KR_DONG,CH_KR_SPACE,CH_KR_SAEK,CH_KR_SANG,CH_KR_SPACE,CH_KR_JO,CH_KR_JEONG,FONT_ROM,0};

CODE BYTE Str_AutoAdjust_E[] = {'A','u','t','o',' ','A','d','j','u','s','t',0};
CODE BYTE Str_AutoAdjust_K[] = {FONT_RAM,CH_KR_JA,CH_KR_DONG,CH_KR_SPACE,CH_KR_HWA,CH_KR_MYEON,CH_KR_SPACE,CH_KR_JO,CH_KR_JEONG,FONT_ROM,0};
#endif

//--------------------------------------------------------------------------------
#ifdef WIDE_SCREEN
CODE BYTE Str_WideScreen_E[] = {'W','i','d','e',' ','S','c','r','e','e','n',0};
CODE BYTE Str_WideScreen_K[] = {'W','I','D','E',' ',FONT_RAM,CH_KR_HWA,CH_KR_MYEON,CH_KR_SPACE,CH_KR_SEON,CH_KR_TAEK,FONT_ROM,0};
#endif

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


CODE BYTE OSDLangString[] = 
{
		0x02, // Size 
		0, // Independence by language:0, Dependenced by language:1 
//					 Language I			Language II			Language III....
		'E','n','g','l','i','s','h',0,	
        'F', 'r', 'a', 'n', CH_C_CEDIL, 'a', 'i', 's',0,
        'D', 'e', 'u', 't', 's', 'c', 'h',0,
        'E', 's', 'p', 'a', CH_N_TILDE, 'o', 'l',0,
		'K', 'o', 'r', 'e', 'a', 'n', 0 //FONT_RAM,CH_KR_HAN,CH_KR_GUK,CH_KR_EO,FONT_ROM,' ',0
};

CODE BYTE ModeChoice[] =		{
		0x04, // Size 
		1,
		'M','o','d','e','1',0,	'M','o','d','e','2',0, 'M','o','d','e','3',0,	'U','s','e','r',0,
		'M','o','d','e','1',0,	'M','o','d','e','2',0, 'M','o','d','e','3',0,	'U','s','a','g','e','r',0,
		'M','o','d','u','s','1',0, 'M','o','d','u','s','2',0, 'M','o','d','u','s','3',0, 'B','e','n','u','t','z','e','r',0,
		'M','o','d','o','1',0,	'M','o','d','o','2',0,	'M','o','d','o','3',0,	'U','s','u','a','r','i','o',0,
		FONT_RAM,CH_KR_MO,CH_KR_DEU,FONT_ROM,'1',0, 
		FONT_RAM,CH_KR_MO,CH_KR_DEU,FONT_ROM,'2',0,	
		FONT_RAM,CH_KR_MO,CH_KR_DEU,FONT_ROM,'3',0, 
		FONT_RAM,CH_KR_SA,CH_KR_YONG,CH_KR_JA,CH_KR_SPACE,FONT_ROM,' ',0  // Caution!: Do not end to osd control command "FONT_ROM" or "FONT_RAM". 
};


CODE BYTE VInputStdChoice[] = {
			0x08,//size of supported system.(Attention!!)
			0,// None by language.
			'A','u','t','o',0,
			'N','T','S','C',0,
#ifdef SUPPORT_PAL
			'P','A','L',0,
#endif
#ifdef SUPPORT_SECAM
			'S','E','C','A','M',0,	
#endif
#ifdef SUPPORT_NTSC4
			'N','T','S','C','4','.','4','3',0,	
#endif
#ifdef SUPPORT_PALM
			'P','A','L','M',0,	
#endif
#ifdef SUPPORT_PALN
			'P','A','L','N',0,	
#endif
#ifdef SUPPORT_PAL60
			'P','A','L','6','0',0,	
#endif
};

#ifdef WIDE_SCREEN
CODE BYTE WideScreenChoice[] = {
			5,//size 
			0,// by language.
			'N','o','r','m','a','l',0, 			
			'W','i','d','e',0, 			
			'P','a','n','o','r','a','m','a',0,	
			'F','u','l','l',0,//			'Z','o','o','m',0,
};
#endif

#ifdef SUPPORT_OSDPOSITIONMOVE
CODE BYTE OsdPositionModeChoice[] = {
			5,//size
			0,// by language.
			CODE2FONT+0x1a,CODE2FONT+0x1b,' ',0,				CODE2FONT+0x1c,CODE2FONT+0x1d,' ',0,
			CODE2FONT+0x1e,CODE2FONT+0x1d,' ',0,				CODE2FONT+0x1f,CODE2FONT+0x20,' ',0,
			CODE2FONT+0x1f,CODE2FONT+0x21,' ',0,
};
#endif


CODE BYTE CaptionChoice[] = {
			0x09,//size
			0,// None by lang.
			'O','f','f',0,	'C','C','1',0,	'C','C','2',0,	'C','C','3',0,	'C','C','4',0,
			'T','1',0,		'T','2',0,		'T','3',0,		'T','4',0,

};


//--------------------------------------------------------------------------------

CODE BYTE 	*Enter_Password_Str[]=	{Enter_Password_Str_E,Enter_Password_Str_F,Enter_Password_Str_D,Enter_Password_Str_S,Enter_Password_Str_K};					   
CODE BYTE 	*CheckPCCableStr[] =	{CheckPCCableStr_E,CheckPCCableStr_E,CheckPCCableStr_E,CheckPCCableStr_E,CheckPCCableStr_K};
CODE BYTE 	*CheckDTVCableStr[] = 	{CheckDTVCableStr_E,CheckDTVCableStr_E,CheckDTVCableStr_E,CheckDTVCableStr_E,CheckDTVCableStr_K};
CODE BYTE 	*OutOfRangeStr[] =		{OutOfRangeStr_E,OutOfRangeStr_E,OutOfRangeStr_E,OutOfRangeStr_E,OutOfRangeStr_K};
CODE BYTE 	*StrVolume[]=			{StrVolume_E,StrVolume_F,StrVolume_D,StrVolume_S,StrVolume_K};
CODE BYTE 	*MuteOnStr[]=			{MuteOnStr_E,MuteOnStr_E,MuteOnStr_E,MuteOnStr_E,MuteOnStr_K};
//--------------------------------------------------------------------------------
CODE BYTE 	*Str_Input[]=			{Str_Input_E,	Str_Input_F,	Str_Input_D,	Str_Input_S,	Str_Input_K};
CODE BYTE 	*Str_Picture[]=			{Str_Picture_E,	Str_Picture_F,	Str_Picture_D,	Str_Picture_S,	Str_Picture_K};
CODE BYTE 	*Str_Sound[]=			{Str_Sound_E,Str_Sound_E,Str_Sound_E,Str_Sound_S,Str_Sound_K};
CODE BYTE 	*Str_Utility[]=			{Str_Utility_E,Str_Utility_F,Str_Utility_D,Str_Utility_S,Str_Utility_K};
CODE BYTE 	*Str_ColorTemp[]=		{Str_ColorTemp_E,Str_ColorTemp_F,Str_ColorTemp_D,Str_ColorTemp_S,Str_ColorTemp_K};

CODE BYTE 	*Str_Comments[]=		{Str_Comments_E,Str_Comments_E,Str_Comments_E,Str_Comments_E,Str_Comments_K};
//============================================================================================
//  OSD Menu String Table
CODE BYTE 	*Str_Composite[] =		{Str_Composite_E,Str_Composite_E,Str_Composite_E,Str_Composite_E,Str_Composite_E};

#ifdef SUPPORT_CVBS2
CODE BYTE 	*Str_Composite2[] =		{Str_Composite2_E,Str_Composite2_E,Str_Composite2_E,Str_Composite2_E,Str_Composite2_E};
#endif
#ifdef SUPPORT_SVIDEO
CODE BYTE 	*Str_Svideo[] =			{Str_Svideo_E,Str_Svideo_E,Str_Svideo_E,Str_Svideo_E,Str_Svideo_E};
#endif
#ifdef SUPPORT_COMPONENT
CODE BYTE 	*Str_Component[] =		{Str_Component_E,Str_Component_E,Str_Component_E,Str_Component_E,Str_Component_E};
#endif
#ifdef SUPPORT_BT656_LOOP
CODE BYTE 	*Str_656[] =			{Str_Dtv_E,Str_Dtv_E,Str_Dtv_E,Str_Dtv_E,Str_Dtv_E};
CODE BYTE   *Str_Panel[] = 			{Str_Panel_E,Str_Panel_E,Str_Panel_E,Str_Panel_E,Str_Panel_E};
CODE BYTE   *Str_Off[] = 			{Str_Off_E,Str_Off_E,Str_Off_E,Str_Off_E,Str_Off_E};
#endif
#ifdef SUPPORT_BT656_LOOP
CODE BYTE 	*Str_LVDS[] =			{Str_LVDS_E,Str_LVDS_E,Str_LVDS_E,Str_LVDS_E,Str_LVDS_E};
#endif
#ifdef SUPPORT_PC
CODE BYTE 	*Str_Pc[] = 			{Str_Pc_E,Str_Pc_E,Str_Pc_E,Str_Pc_E,Str_Pc_E};
#endif
#ifdef SUPPORT_DVI
CODE BYTE 	*Str_DVI[] =   			{Str_DVI_E,Str_DVI_E,Str_DVI_E,Str_DVI_E,Str_DVI_E};
#endif
#if defined(SUPPORT_HDMI)
CODE BYTE 	*Str_HDMI[] =   		{Str_HDMI_E,Str_HDMI_E,Str_HDMI_E,Str_HDMI_E,Str_HDMI_E};
#endif

#ifdef SUPPORT_PC
CODE BYTE 	*Str_VPosition[]=		{Str_VPosition_E,Str_VPosition_F,Str_VPosition_D,Str_VPosition_S,Str_VPosition_K};
CODE BYTE 	*Str_HPosition[]=		{Str_HPosition_E,Str_HPosition_F,Str_HPosition_D,Str_HPosition_S,Str_HPosition_K};
#endif

#ifdef SUPPORT_OSDPOSITIONMOVE
CODE BYTE 	*Str_OSDPosition[]=		{Str_OSDPosition_E,Str_OSDPosition_E,Str_OSDPosition_E,Str_OSDPosition_S,Str_OSDPosition_K};
#endif
CODE BYTE 	*Str_OSDLang[]=			{Str_OSDLang_E,Str_OSDLang_F,Str_OSDLang_D,Str_OSDLang_S,Str_OSDLang_K};
CODE BYTE 	*Str_SleepTimer[]=		{Str_SleepTimerE,Str_SleepTimerE,Str_SleepTimerE,Str_SleepTimerE,Str_SleepTimerK};
CODE BYTE 	*Str_VInputStd[]=		{Str_VInputStd_E,Str_VInputStd_E,Str_VInputStd_E,Str_VInputStd_E,Str_VInputStd_E};
CODE BYTE 	*Str_Information[]=		{Str_Information_E,Str_Information_E,Str_Information_E,Str_Information_E,Str_Information_E};
CODE BYTE 	*Str_Volume[]=			{Str_Volume_E,Str_Volume_E,Str_Volume_D,Str_Volume_S,Str_Volume_K};
CODE BYTE 	*Str_Treble[]=			{Str_Treble_E,Str_Treble_F,Str_Treble_D,Str_Treble_S,Str_Treble_K};
CODE BYTE 	*Str_Bass[]=			{Str_Bass_E,Str_Bass_F,Str_Bass_D,Str_Bass_S,Str_Bass_K};
CODE BYTE 	*Str_Balance[]=			{Str_Balance_E,Str_Balance_F,Str_Balance_D,Str_Balance_S,Str_Balance_K};
CODE BYTE 	*Str_Red[]=				{Str_Red_E,Str_Red_F,Str_Red_D,Str_Red_S,Str_Red_K};
CODE BYTE 	*Str_Blue[]=			{Str_Blue_E,Str_Blue_F,Str_Blue_D,Str_Blue_S,Str_Blue_K};
CODE BYTE 	*Str_Green[]=			{Str_Green_E,Str_Green_F,Str_Green_D,Str_Green_S,Str_Green_K};
CODE BYTE 	*Str_Hue[]=				{Str_Hue_E,Str_Hue_F,Str_Hue_D,Str_Hue_S,Str_Hue_K};
CODE BYTE 	*Str_Backlight[]=		{Str_Backlight_E,Str_Backlight_E,Str_Backlight_E,Str_Backlight_E,Str_Backlight_E};
CODE BYTE 	*Str_Sharpness[]=		{Str_Sharpness_E,Str_Sharpness_F,Str_Sharpness_D,Str_Sharpness_S,Str_Sharpness_K};
CODE BYTE 	*Str_Contrast[]=		{Str_Contrast_E,Str_Contrast_F,Str_Contrast_D,Str_Contrast_S,Str_Contrast_K};
CODE BYTE 	*Str_Brightness[]=		{Str_Brightness_E,Str_Brightness_F,Str_Brightness_D,Str_Brightness_S,Str_Brightness_K};
CODE BYTE 	*Str_Saturation[]=		{Str_Saturation_E,Str_Saturation_E,Str_Saturation_D,Str_Saturation_S,Str_Saturation_K};
CODE BYTE 	*Str_Reset[]=			{Str_Reset_E,Str_Reset_F,Str_Reset_D,Str_Reset_S,Str_Reset_K};

#ifdef SUPPORT_PC
CODE BYTE 	*Str_Phase[]=			{Str_Phase_E,Str_Phase_E,Str_Phase_E,Str_Phase_S,Str_Phase_E};
CODE BYTE 	*Str_Clock[]=			{Str_Clock_E,Str_Clock_F,Str_Clock_D,Str_Clock_S,Str_Clock_E};
CODE BYTE 	*Str_AutoColor[]=		{Str_AutoColor_E,Str_AutoColor_E,Str_AutoColor_E,Str_AutoColor_E,Str_AutoColor_K};
CODE BYTE 	*Str_AutoAdjust[]=		{Str_AutoAdjust_E,Str_AutoAdjust_E,Str_AutoAdjust_E,Str_AutoAdjust_E,Str_AutoAdjust_K};
#endif

#ifdef WIDE_SCREEN
CODE BYTE 	*Str_WideScreen[] =		{Str_WideScreen_E,Str_WideScreen_E,Str_WideScreen_E,Str_WideScreen_E,Str_WideScreen_K};
#endif

#endif //.. SUPPORT_FOSD_MENU

