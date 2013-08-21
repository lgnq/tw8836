/**
 * @file
 * FOsdDisplay.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	FontOSD display routines
*/
#include "Config.h"
#include "typedefs.h"
#include "TW8836.h"

#include "Global.h"
#include "cpu.h"
#include "printf.h"
#include "util.h"
#include "Debug.h"

#include "i2c.h"

#include "main.h"
#include "EEPROM.h"

#include "SOsd.h"
#include "FOsd.h"

#include "FOsdMenu.h"
#include "FOsdDisplay.h"
//#include "RegMap.h"

#include "InputCtrl.h"
#include "decoder.h"
#include "FOsdTable.h"
#include "FOsdString.h"

#ifndef SUPPORT_FOSD_MENU
//==========================================
//----------------------------
//Trick for Bank Code Segment
//----------------------------
CODE BYTE DUMMY_FOSD_DISPINFO_CODE;
void Dummy_FosdDispInfo_func(void)
{
	BYTE temp;
	temp = DUMMY_FOSD_DISPINFO_CODE;
}
//==========================================
//==========================================
#else //..SUPPORT_FOSD_MENU
//==========================================
	    WORD  OSDLastKeyInTime;		// in ms

//----------------------------------------------------------------------

DATA 	WORD  OSDDisplayedTime;		//in ms
DATA 	BYTE  DisplayedOSD = 0;
//	    DATA BYTE  DisplayedOSD = 0;
//extern	IDATA struct BarInfo BarInfo;
extern  bit   TVChManual;
extern  bit	DisplayInputHold;

extern	BYTE	DummyChannel;
#ifdef SUPPORT_COMPONENT
extern  BYTE  ComponentMode;
#endif




CODE BYTE COMPONENT_STR[][15] = {
    "YPbPr 480i",	// 0
    "YPbPr 576i",	// 1
    "YPbPr 480p",	// 2
    "YPbPr 576p",	// 3
    "YPbPr 1080i",	// 4
    "YPbPr 720p",	// 5
	"YPbPr 1080p",	
    "YPbPr NONE",
};


//================================================================
//					Display information
//					Display Win0 :Intersil - from 0 ~ 3bit color, size 44
//					Display Win1 :Techwell - from 0x84 ~ 3bit color, size 15
//================================================================
#if 0
//code WORD LOGO_COLOR_8[16] = {
//     0xFFFF,0x0020,0xDEDB,0xA534,0x6B6D,0xC826,0x4A49,0xDCD5,
//     0xFFFF,0xC806,0xC98C,0xCB31,0xFE79,0xFCD9,0xCCD4,0xE71C
//};
//-------------------------------------------------------------------
// Display/Clear FOSD LOGO
//-------------------------------------------------------------------
void DisplayLogo(void)
{
	BYTE i;

	ReadTW88Page(page);

 	InitFOsdMenuWindow(Init_Osd_DisplayLogo);
	#ifdef ADD_ANALOGPANEL
	if(IsAnalogOn())
		InitFOsdMenuWindow(Init_Osd_DisplayLogo_A);
	#endif
	
	InitFontRamByNum(FONT_NUM_LOGO, 0);	//FOsdDownloadFont(2);

	FOsdSetPaletteColorArray(0, LOGO_COLOR_8, 16, 1);

	WriteTW88Page(PAGE4_CLOCK);
	dPrintf("\nDisplayLogo-Current MCU SPI Clock select : [0x4E0]=0x%02bx, [0x4E1]=0x%02bx", ReadTW88(REG4E0), ReadTW88(REG4E1));

	WriteTW88Page(PAGE3_FOSD );
	WriteTW88(REG304, ReadTW88(REG304)&0xFE); // OSD RAM Auto Access Enable
	WriteTW88(REG304, (ReadTW88(REG304)&0xF3)); // Normal

	for ( i=0; i<70; i++ ) {
		WriteTW88(REG306, i );
		WriteTW88(REG307, i*3 );
		WriteTW88(REG308, (i / 42)*2 );
	}

	WriteTW88(REG30B, 0 );						// 2bit multi color start = 0
	WriteTW88(REG_FOSD_MADD3, 0 );				// 3bit multi color start = 0
	WriteTW88(REG_FOSD_MADD4, 0xff );			// 4bit multi color start = 0

	FOsdWinEnable(TECHWELLOGO_OSDWIN,TRUE);
	FOsdWinEnable(TECHWELLOGO_OSDWIN+1,FALSE);
	FOsdWinEnable(TECHWELLOGO_OSDWIN+2,TRUE);
}
void ClearLogo(void)
{
	DECLARE_LOCAL_page
	BYTE i, j;

	ReadTW88Page(page);
	WriteTW88Page(PAGE3_FOSD );
	//=============================== Fade OUT ======================================
	for ( i=0; i<9; i++ ) {
		delay1ms(30);
		for ( j=0; j<16; j++ ) {
			WriteTW88(REG_FOSD_ALPHA_SEL, j );
			WriteTW88(FOSDWinBase[TECHWELLOGO_OSDWIN]  +1, i );
			WriteTW88(FOSDWinBase[TECHWELLOGO_OSDWIN+2]+1, i );
		}
	}
	//============ Disable window and recover Trans value ============================
	FOsdOnOff(OFF, 1);	//with vdelay 1
	FOsdWinEnable(TECHWELLOGO_OSDWIN,FALSE);
	FOsdWinEnable(TECHWELLOGO_OSDWIN+2,FALSE);
	//assume page3
	for ( j=0; j<16; j++ ) {
		WriteTW88(REG_FOSD_ALPHA_SEL, j );
		WriteTW88(FOSDWinBase[TECHWELLOGO_OSDWIN]  +1, 0 );
		WriteTW88(FOSDWinBase[TECHWELLOGO_OSDWIN+2]+1, 0 );
	}

	WriteTW88Page(page );
}
#endif

//-------------------------------------------------------------------
// Display/Clear FOSD Info
//-------------------------------------------------------------------
//desc
//	set DisplayedOSD global flag.
void SetDisplayedOSD( BYTE newd )
{
	DisplayedOSD |= newd;
}

void ClearOSDInfo(void)
{
	BYTE dt;

	dt = DisplayedOSD; 
	#ifdef DEBUG_OSD
	dPrintf("\r\nClearOSDInfo: DisplayedOSD:%2x",(WORD) DisplayedOSD);
	#endif

	if( (dt & FOSD_TVVOL) || ( dt & FOSD_TVCHN) )	ClearTVInfo();
	if( dt & FOSD_MUTE )							ClearMuteInfo();
	if( dt & FOSD_INPUTINFO )						FOsdClearInput();
	if( dt & FOSD_MENU )							FOsdMenuClose();
	if( dt & FOSD_PCINFO )							ClearPCInfo();
	
}

//-------------------------------------------------------------------
// Display/Clear DisplayInput
//-------------------------------------------------------------------
void FOsdDisplayInput(void)
{
	BYTE	inputs, len1, len2,i, SystemAddr;
	#ifdef SUPPORT_COMPONENT
	BYTE CODE 	*Str;
	#endif

	SystemAddr=INPUTINFO1_ADDR;

	if( (DisplayedOSD & FOSD_INPUTINFO ) && DisplayInputHold ) return;

	ClearOSDInfo();


	inputs = InputMain;

	#ifdef DEBUG_OSD
 	dPrintf("\r\n++(DisplayInput) : %d", (WORD)inputs);
	#endif

	#if 0
	#if defined( SUPPORT_PC ) || defined( SUPPORT_BT656_LOOP )
	if( inputs==PC || inputs==DTV )
		len2 = TWstrlen( GetPCInputSourceName() );
	else
	#endif
	#endif

	#ifdef SUPPORT_COMPONENT
	if( inputs==INPUT_COMP ){
		WriteTW88Page(1 );
		i = ReadTW88(REG1C1) & 7;
		Str = COMPONENT_STR[i];
		len2 = TWstrlen(Str);
	}
	else
	#endif
		len2 = TWstrlen( struct_VInputStd[GetVInputStdInx()].Name );


	for (i=1; ;i++)
		if( struct_InputSelection[i].Id==inputs )  break;

	len1 = TWstrlen(struct_InputSelection[i].Name);

	InitFOsdMenuWindow(Init_Osd_DisplayInput);
	#ifdef ADD_ANALOGPANEL
	if(IsAnalogOn())
		InitFOsdMenuWindow(Init_Osd_DisplayInput_A);
	#endif
	FOsdRamMemset(INPUTINFO_ADDR, 0x020, DEFAULT_COLOR, 51 );	// Total 42 Char.	
	WriteStringToAddr(INPUTINFO_ADDR, struct_InputSelection[i].Name, len1); 

	#if 0
	#if defined( SUPPORT_PC ) || defined( SUPPORT_BT656_LOOP )
	if( inputs==PC || inputs==DTV )
		WriteStringToAddr(SystemAddr, GetPCInputSourceName(), len2); 
	else
	#endif
	#endif
	#ifdef SUPPORT_COMPONENT
	if( inputs==INPUT_COMP ){
		//i = DecoderIsNoInput();
		//Printf("\r\n Display Info==> Component Noinutcheck: %d ", (WORD)i );
		//if( DecoderIsNoInput() == 0 ) WriteStringToAddr(SystemAddr, Str, len2); 
		WriteStringToAddr(SystemAddr, Str, len2); 
	}
	else
	#endif
	{
		if( DecoderIsNoInput() == 0 ) 
		WriteStringToAddr(SystemAddr, struct_VInputStd[GetVInputStdInx()].Name, len2); 
	}

	FOsdWinEnable(OSD_Win_Num(INPUTINFO_OSDWIN),TRUE);
	FOsdWinEnable(OSD_Win_Num(INPUTINFO_OSDWIN+1),TRUE);
	FOsdOnOff(ON, 1); //with vdelay 1

	DisplayedOSD |= FOSD_INPUTINFO;
	OSDDisplayedTime = GetTime_ms();	
}

BYTE FOsdClearInput(void)
{
	//FOsdOnOff(OFF, 1);	//with vdelay 1
	FOsdWinEnable(INPUTINFO_OSDWIN,		FALSE);
	FOsdWinEnable(INPUTINFO_OSDWIN+1,  	FALSE);

	DisplayedOSD &= (~(FOSD_INPUTINFO));
	//#ifdef DEBUG_OSD
	//dPrintf("\r\n++(ClearInput)");
	//#endif

	return 1;
//	ClearBlending();
}

//-------------------------------------------------------------------
// DisplayVol / ClearTVInfo
//-------------------------------------------------------------------

void DisplayVol(void)
{
	BYTE CODE *Str ;
	BYTE len;

	//#ifdef DEBUG_OSD
	//dPuts("\r\n++(DisplayVol)");
	//#endif
	
	if(( DisplayedOSD & FOSD_TVVOL ) == 0 ) {
		ClearOSDInfo();
		FOsdDefaultLUT();
		delay1ms(100);
		
		InitFOsdMenuWindow(Init_Osd_BarWindow);
	
		FOsdRamMemset(FOSDMENU_BARADDR,    0x020, BG_COLOR_CYAN | FG_COLOR_WHITE, 25 );		// Total 25*2 Char. 
		FOsdRamMemset(FOSDMENU_BARADDR+25, 0x020, BG_COLOR_WHITE | FG_COLOR_CYAN, 25 );
	
		Str = StrVolume[OSDLang];
		len=TWstrlen(Str);
		WriteStringToAddr(FOSDMENU_BARADDR, Str, len);  
	
		FOsdWinEnable(FOSDMENU_BARWIN,TRUE);
		FOsdOnOff(ON, 1);	//with vdelay 1
	}
	DisplayVolumebar(AudioVol);

	DisplayedOSD |= FOSD_TVVOL;
	OSDDisplayedTime = GetTime_ms();
}
BYTE ClearTVInfo(void)
{
	
	if( DisplayedOSD & FOSD_TVVOL ) {
		FOsdWinEnable(FOSDMENU_BARWIN, FALSE);
		DisplayedOSD &= (~(FOSD_TVVOL));
	}
	else  {
		FOsdWinEnable(TVINFO_OSDWIN, FALSE);
		DisplayedOSD &= (~(FOSD_TVCHN));
	}
	return 1;
}

//-------------------------------------------------------------------
// Display/Clear Mute Info
//-------------------------------------------------------------------
void DisplayMuteInfo(void)
{
	BYTE CODE	*Str ;
	BYTE len;

	ClearOSDInfo();
	InitFOsdMenuWindow(Init_Osd_DisplayMuteInfo);

	Str = MuteOnStr[OSDLang];
	len = TWstrlen(Str);
	WriteStringToAddr(MUTEINFO_ADDR, Str, len);  
	FOsdRamMemsetAttr(MUTEINFO_ADDR, BACK_COLOR|FG_COLOR_RED, len );	 

	FOsdWinEnable(TVINFO_OSDWIN,TRUE);
	FOsdOnOff(ON, 1);	//with vdelay 1
	DisplayedOSD |= FOSD_MUTE;
}
void ClearMuteInfo(void)
{
	FOsdWinEnable(TVINFO_OSDWIN,FALSE);
	DisplayedOSD &= (~(FOSD_MUTE));
}


//----------------------------------
// Display/Clear PCInfo
//
//	win: PCINFO_OSDWIN
//----------------------------------
void DisplayPCInfo(BYTE CODE *ptr)
{
	WORD xstart, ystart;
	BYTE len;

	//#ifdef DEBUG_PC
	//Printf("\r\n==== DisplayPCInfo (%s) ====", ptr);
	//#endif

	len  = TWstrlen(ptr);

	xstart = ( PANEL_H - len*PCINFO_ZOOM*FONT_DEF_WIDTH ) / 2;
	ystart =  PANEL_V / 3;
	
	InitFOsdMenuWindow(Init_Osd_DisplayPCInfo);
	FOsdWinScreenXY(PCINFO_OSDWIN, xstart,ystart);
	FOsdWinSetW(PCINFO_OSDWIN, len);

	FOsdRamMemset(PCMEASUREINFO_ADDR,0x020, FG_COLOR_RED, len);	 

	WriteStringToAddr(PCMEASUREINFO_ADDR, (BYTE CODE *)ptr, len);
	FOsdWinEnable(PCINFO_OSDWIN,TRUE);
	FOsdOnOff(ON, 1);	//with vdelay 1
	
	DisplayedOSD |=FOSD_PCINFO;

}
void ClearPCInfo(void)
{
	if( !(DisplayedOSD & FOSD_PCINFO) )		
		return;

	DisplayedOSD &= (~(FOSD_PCINFO));
	FOsdWinEnable(PCINFO_OSDWIN,FALSE);
	//dPuts("\r\n== ClearPCInfo ==");
}

/*
//----------------------------------
// AutoAdjust
//----------------------------------

void DisplayAutoAdjust(void)
{
	InitFOsdMenuWindow(Init_Osd_BarWindow);
	FOsdRamMemset(FOSDMENU_BARADDR, 0x020, BG_COLOR_CYAN | FG_COLOR_WHITE, 50 );	// Total 25*2 Char.	 
	WriteStringToAddr(FOSDMENU_BARADDR, AutoAdjustInfo, 25);

	FOsdWinEnable(FOSDMENU_BARWIN,TRUE);
	FOsdOnOff(ON, 1);	//with vdelay 1
}
void ClearAutoAdjust(void)
{
	FOsdWinEnable(FOSDMENU_BARWIN,FALSE);
}
*/


BYTE CheckAndClearFOsd(void)
{
	WORD	ctime,  stime, diff;
	BYTE	dt;
	BYTE	Result=0;

	dt = DisplayedOSD;
	if ( dt & ( FOSD_MENU | FOSD_INPUTINFO | FOSD_TVCHN | FOSD_TVVOL  ) ) {

		if( dt & FOSD_MENU )	{ 
			stime = OSDLastKeyInTime;	
			diff =  20 * 1000; //GetOSDDuration()
		}
		else { 
			stime = OSDDisplayedTime;	
			diff = 2 * 1000;	
		}
	
		ctime = GetTime_ms();			

		if( DiffTime_ms( stime, ctime ) >= diff ) {

			#ifdef DEBUG_OSD
			if( !(dt & FOSD_MENU) ) {
				dPrintf("\r\n(CheckAndClearOSD)DisplayedOSD:0x%x, ", (WORD)dt);
				dPrintf("starttime:%d, currtime:%d", stime, ctime);
			}
			#endif

			//if( dt & LOGO )			ClearLogo();
			if( !DisplayInputHold )
				if( dt & FOSD_INPUTINFO )	
					Result = FOsdClearInput();
			if( dt & FOSD_TVVOL )		
				Result = ClearTVInfo();

			if( dt & FOSD_MENU ) {			
				Result = 1;
				FOsdMenuClose();  
			}

			if( Result == 1){
				if( DisplayInputHold ) 
					FOsdDisplayInput();
				else {
					if( IsAudioMuteOn() )	
						DisplayMuteInfo();
				}
			}
			return 1;
		}
	}

	return 0;
}

//==========================================
#endif //.. SUPPORT_FOSD_MENU
//==========================================

