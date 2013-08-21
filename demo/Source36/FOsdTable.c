/**
 * @file
 * FOsdTable.c 
 * @author Harry Han
 * @author YoungHwan Bae
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	FontOSD Window Initialization Table
*/
#include "config.h"
#include "typedefs.h"

#include "i2c.h"

#include "InputCtrl.h"		//BK120130

#include "FOsd.h"
#include "FOsdString.h"
#include "FOsdMenu.h"
#include "FOsdDisplay.h"
#include "FOsdTable.h"
#include "BT656.h"


#ifndef SUPPORT_FOSD_MENU
//==========================================
//----------------------------
//Trick for Bank Code Segment
//----------------------------
CODE BYTE DUMMY_FOSD_INITTABLE_CODE;
void Dummy_FosdInitTable_func(void)
{
	BYTE temp;
	temp = DUMMY_FOSD_INITTABLE_CODE;
}
//==========================================
#else //..SUPPORT_FOSD_MENU




//*****************************************************************************
//
//           OSD Window Initialization Table
//
//*****************************************************************************
//=============================================================================
//			Initialize Window for DisplayLogo "Techwell"
//=============================================================================
CODE BYTE Init_Osd_DisplayLogo[] = {
	16,
	OSD_Win_Num				( TECHWELLOGO_OSDWIN ),	// 1 byte Offset for windows
	OSD_Win_CONTROL_00		( WIN_MULTI_EN | WIN_V_EXT_EN0 | WIN_VZOOM(3) | WIN_HZOOM(3) ),
	OSD_Win_Blending_01		( 0 ),
	OSD_Win_Start_02_4		( 30, 0x80 ),	// 3 bytes
	OSD_Win_Height_05		( 2 ),
	OSD_Win_Width_06		( 21 ),
	OSD_Win_BorderColor_07	( FG_COLOR_WHITE ),
	OSD_Win_Border_08		( WIN_BD_EN0 | WIN_BD_WIDTH(0) ),
	OSD_Win_HBorder_09		( WIN_H_BD_W(0) ),
	OSD_Win_VBorder_0A		( WIN_V_BD_W(0) ),
	OSD_Win_3D_0B			( WIN_3D_EN0 | WIN_3D_TYPE0 | WIN_3D_LEVEL0 | WIN_CH_BDSH_SEL(0) | FG_COLOR_BLACK ),
 	OSD_Win_0C_0D			( WIN_SHADOW_EN0 | WIN_SHADOW_W(0), WIN_CH_HSPACE(0), WIN_CH_VSPACE(0) ),	 // 2bytes
 	OSD_Win_Attr_0E			( WIN_BG_COLOR_EX0 | BG_COLOR_BLACK | FG_COLOR_WHITE ),
	OSD_Win_SAddr_0F		( WIN_SADDR(0) ),

	16,
	OSD_Win_Num				( TECHWELLOGO_OSDWIN+1 ),	// 1 byte Offset for windows
	OSD_Win_CONTROL_00		( WIN_MULTI_EN | WIN_V_EXT_EN0 | WIN_VZOOM(3) | WIN_HZOOM(3) ),
	OSD_Win_Blending_01		( 0 ),
	OSD_Win_Start_02_4		( 264, 280 ),	// 3 bytes
	OSD_Win_Height_05		( 1 ),
	OSD_Win_Width_06		( 14 ),
	OSD_Win_BorderColor_07	( FG_COLOR_WHITE ),
	OSD_Win_Border_08		( WIN_BD_EN0 | WIN_BD_WIDTH(0) ),
	OSD_Win_HBorder_09		( WIN_H_BD_W(0) ),
	OSD_Win_VBorder_0A		( WIN_V_BD_W(0) ),
	OSD_Win_3D_0B			( WIN_3D_EN0 | WIN_3D_TYPE0 | WIN_3D_LEVEL0 | WIN_CH_BDSH_SEL(0) | FG_COLOR_BLACK ),
 	OSD_Win_0C_0D			( WIN_SHADOW_EN0 | WIN_SHADOW_W(0), WIN_CH_HSPACE(0), WIN_CH_VSPACE(0) ),	 // 2bytes
	OSD_Win_Attr_0E			( WIN_BG_COLOR_EX0 | BG_COLOR_BLACK | FG_COLOR_WHITE ),
	OSD_Win_SAddr_0F		( WIN_SADDR(42) ),

	16,
	OSD_Win_Num				( TECHWELLOGO_OSDWIN+2 ),	// 1 byte Offset for windows
	OSD_Win_CONTROL_00		( WIN_MULTI_EN0 | WIN_V_EXT_EN0 | WIN_VZOOM(4) | WIN_HZOOM(4) ),
	OSD_Win_Blending_01		( 0 ),
	OSD_Win_Start_02_4		( 0, 0 ),	// 3 bytes
	OSD_Win_Height_05		( 3 ),
	OSD_Win_Width_06		( 14 ),
	OSD_Win_BorderColor_07	( FG_COLOR_WHITE ),
	OSD_Win_Border_08		( WIN_BD_EN0 | WIN_BD_WIDTH(0) ),
	OSD_Win_HBorder_09		( WIN_H_BD_W(0) ),
	OSD_Win_VBorder_0A		( WIN_V_BD_W(0) ),
	OSD_Win_3D_0B			( WIN_3D_EN0 | WIN_3D_TYPE0 | WIN_3D_LEVEL0 | WIN_CH_BDSH_SEL(0) | FG_COLOR_BLACK ),
 	OSD_Win_0C_0D			( WIN_SHADOW_EN0 | WIN_SHADOW_W(0), WIN_CH_HSPACE(28), WIN_CH_VSPACE(15) ),	 // 2bytes
	OSD_Win_Attr_0E			( WIN_BG_COLOR_EX0 | BG_COLOR_BLACK | FG_COLOR_WHITE ),
	OSD_Win_SAddr_0F		( WIN_SADDR(0) ),

	0 // End of Data
};
//
#ifdef ADD_ANALOGPANEL
CODE BYTE Init_Osd_DisplayLogo_A[] = {

	16,
	OSD_Win_Num				( TECHWELLOGO_OSDWIN ),	// 1 byte Offset for windows
	OSD_Win_CONTROL_00		( WIN_MULTI_EN | WIN_V_EXT_EN0 | WIN_VZOOM(WINZOOMx2) | WIN_HZOOM(WINZOOMx3) ),
	OSD_Win_Blending_01		( 0x0f ),
	OSD_Win_Start_02_4		( ((480*2-12*FONT_W*WINZOOMx3)/2), ((234-FONT_H*2*WINZOOMx2)/2)),	// 3 bytes
	OSD_Win_Height_05		( 2 ),
	OSD_Win_Width_06		( 12 ),
	OSD_Win_BorderColor_07	( FG_COLOR_WHITE ),
	OSD_Win_Border_08		( WIN_BD_EN0 | WIN_BD_WIDTH(0) ),
	OSD_Win_HBorder_09		( WIN_H_BD_W(0) ),
	OSD_Win_VBorder_0A		( WIN_V_BD_W(0) ),
	OSD_Win_3D_0B			( WIN_3D_EN0 | WIN_3D_TYPE0 | WIN_3D_LEVEL0 | WIN_CH_BDSH_SEL(SEL_BORDER) | FG_COLOR_BLACK ),
 	OSD_Win_0C_0D			( WIN_SHADOW_EN0 | WIN_SHADOW_W(0), WIN_CH_HSPACE(0), WIN_CH_VSPACE(0) ),	 // 2bytes
	OSD_Win_Attr_0E			( WIN_BG_COLOR_EX0 | BG_COLOR_BLACK | FG_COLOR_WHITE ),
	OSD_Win_SAddr_0F		( WIN_SADDR(LOGO_ADDR) ),

	0 // End of Data
};
#endif
//=============================================================================
//			Initialize Window for Vchip Rathing
//=============================================================================
CODE BYTE Init_Osd_DisplayVchipWindow[] = {

	16,
	OSD_Win_Num				( RATINGINFO_OSDWIN ),	// 1 byte Offset for windows
	OSD_Win_CONTROL_00		( WIN_MULTI_EN | WIN_V_EXT_EN0 | WIN_VZOOM(LOGO_ZOOM) | WIN_HZOOM(LOGO_ZOOM) ),
	OSD_Win_Blending_01		( 0x0f ),
	OSD_Win_Start_02_4		( ((PANEL_H-FONT_W*18*LOGO_ZOOM-18)/2), ((PANEL_V-FONT_H*2*LOGO_ZOOM-8)/2)),	// 3 bytes
	OSD_Win_Height_05		( 2 ),
	OSD_Win_Width_06		( 18 ),
	OSD_Win_BorderColor_07	( FG_COLOR_WHITE ),
	OSD_Win_Border_08		( WIN_BD_EN | WIN_BD_WIDTH(1) ),
	OSD_Win_HBorder_09		( WIN_H_BD_W(0x10) ),
	OSD_Win_VBorder_0A		( WIN_V_BD_W(0x08) ),
	OSD_Win_3D_0B			( WIN_3D_EN0 | WIN_3D_TYPE0 | WIN_3D_LEVEL0 | WIN_CH_BDSH_SEL(SEL_BORDER) | FG_COLOR_BLACK ),
 	OSD_Win_0C_0D			( WIN_SHADOW_EN0 | WIN_SHADOW_W(0), WIN_CH_HSPACE(0), WIN_CH_VSPACE(0) ),	 // 2bytes
	OSD_Win_Attr_0E			( BACK_COLOR | FG_COLOR_WHITE ),
	OSD_Win_SAddr_0F		( WIN_SADDR(RATINGINFO_ADDR) ),

	0 // End of Data
};
//=============================================================================
//			Initialize Window for DisplayMuteInfo
//=============================================================================
CODE BYTE Init_Osd_DisplayMuteInfo[] = {

	16,
	OSD_Win_Num				( TVINFO_OSDWIN ),	// 1 byte Offset for windows
	OSD_Win_CONTROL_00		( WIN_MULTI_EN | WIN_V_EXT_EN0 | WIN_VZOOM(MUTE_ZOOM) | WIN_HZOOM(MUTE_ZOOM) ),
	OSD_Win_Blending_01		( 0x0f ),
	OSD_Win_Start_02_4		( PANEL_H-FONT_W*4*MUTE_ZOOM, 0 ),	// 3 bytes
	OSD_Win_Height_05		( 1 ),
	OSD_Win_Width_06		( 4 ),
	OSD_Win_BorderColor_07	( FG_COLOR_WHITE ),
	OSD_Win_Border_08		( WIN_BD_EN0 | WIN_BD_WIDTH(0) ),
	OSD_Win_HBorder_09		( WIN_H_BD_W(0x00) ),
	OSD_Win_VBorder_0A		( WIN_V_BD_W(0x00) ),
	OSD_Win_3D_0B			( WIN_3D_EN0 | WIN_3D_TYPE0 | WIN_3D_LEVEL0 | WIN_CH_BDSH_SEL(SEL_BORDER) | FG_COLOR_BLACK ),
 	OSD_Win_0C_0D			( WIN_SHADOW_EN0 | WIN_SHADOW_W(0), WIN_CH_HSPACE(0), WIN_CH_VSPACE(0) ),	 // 2bytes
	OSD_Win_Attr_0E			( WIN_BG_COLOR_EX0 | BG_COLOR_BLACK | FG_COLOR_WHITE ),
	OSD_Win_SAddr_0F		( WIN_SADDR(MUTEINFO_ADDR) ),

	0 // End of Data
};
//=============================================================================
//			Initialize Window for DisplayTvchannel
//=============================================================================
CODE BYTE Init_Osd_DisplayTVChannel[] = {

	16,
	OSD_Win_Num				( TVINFO_OSDWIN ),	// 1 byte Offset for windows
	OSD_Win_CONTROL_00		( WIN_MULTI_EN | WIN_V_EXT_EN0 | WIN_VZOOM(TVCHN_ZOOM) | WIN_HZOOM(TVCHN_ZOOM) ),
	OSD_Win_Blending_01		( 0x0f ),
	OSD_Win_Start_02_4		( PANEL_H-FONT_W*3*TVCHN_ZOOM, 0 ),	// 3 bytes
	OSD_Win_Height_05		( 1 ),
	OSD_Win_Width_06		( 3 ),
	OSD_Win_BorderColor_07	( FG_COLOR_WHITE ),
	OSD_Win_Border_08		( WIN_BD_EN0 | WIN_BD_WIDTH(0) ),
	OSD_Win_HBorder_09		( WIN_H_BD_W(0x00) ),
	OSD_Win_VBorder_0A		( WIN_V_BD_W(0x00) ),
	OSD_Win_3D_0B			( WIN_3D_EN0 | WIN_3D_TYPE0 | WIN_3D_LEVEL0 | WIN_CH_BDSH_SEL(SEL_BORDER) | FG_COLOR_BLACK ),
 	OSD_Win_0C_0D			( WIN_SHADOW_EN0 | WIN_SHADOW_W(0), WIN_CH_HSPACE(0), WIN_CH_VSPACE(0) ),	 // 2bytes
	OSD_Win_Attr_0E			( WIN_BG_COLOR_EX0 | BG_COLOR_BLACK | FG_COLOR_WHITE ),
	OSD_Win_SAddr_0F		( WIN_SADDR(TVCHINFO_ADDR) ),

	0 // End of Data
};

//=============================================================================
//			Initialize Window for DisplayPCInfo
//=============================================================================
CODE BYTE Init_Osd_DisplayPCInfo[] = {

	16,
	OSD_Win_Num				( PCINFO_OSDWIN ),	// 1 byte Offset for windows
	OSD_Win_CONTROL_00		( WIN_MULTI_EN | WIN_V_EXT_EN0 | WIN_VZOOM(PCINFO_ZOOM) | WIN_HZOOM(PCINFO_ZOOM) ),
	OSD_Win_Blending_01		( 4 ),
	OSD_Win_Start_02_4		( 12, 13 ),	// 3 bytes
	OSD_Win_Height_05		( 1 ),
	OSD_Win_Width_06		( 10 ),
	OSD_Win_BorderColor_07	( FG_COLOR_WHITE ),
	OSD_Win_Border_08		( WIN_BD_EN | WIN_BD_WIDTH(1) ),
	OSD_Win_HBorder_09		( WIN_H_BD_W(4) ),
	OSD_Win_VBorder_0A		( WIN_V_BD_W(4) ),
	OSD_Win_3D_0B			( WIN_3D_EN0 | WIN_3D_TYPE0 | WIN_3D_LEVEL0 | WIN_CH_BDSH_SEL(SEL_BORDER) | FG_COLOR_BLACK ),
 	OSD_Win_0C_0D			( WIN_SHADOW_EN0 | WIN_SHADOW_W(0), WIN_CH_HSPACE(0), WIN_CH_VSPACE(0) ),	 // 2bytes
	OSD_Win_Attr_0E			( BACK_COLOR | FG_COLOR_WHITE ),
	OSD_Win_SAddr_0F		( WIN_SADDR(PCMEASUREINFO_ADDR) ),

	0 // End of Data
};
//=============================================================================
//			Initialize Window for DisplayInput
//=============================================================================
CODE BYTE Init_Osd_DisplayInput[] = {

	//----- Input Selection : Zoom 2
	16,
	OSD_Win_Num				( INPUTINFO_OSDWIN ),	// 1 byte Offset for windows
	OSD_Win_CONTROL_00		( WIN_MULTI_EN | WIN_V_EXT_EN0 | WIN_VZOOM(INPUTINFO_ZOOM) | WIN_HZOOM(WINZOOMx2) ),
	OSD_Win_Blending_01		( 4 ),
	OSD_Win_Start_02_4		( 12, 13 ),	// 3 bytes
	OSD_Win_Height_05		( 1 ),
	OSD_Win_Width_06		( 9 ),
	OSD_Win_BorderColor_07	( FG_COLOR_WHITE ),
	OSD_Win_Border_08		( WIN_BD_EN0 | WIN_BD_WIDTH(1) ),
	OSD_Win_HBorder_09		( WIN_H_BD_W(1) ),
	OSD_Win_VBorder_0A		( WIN_V_BD_W(0) ),
	OSD_Win_3D_0B			( WIN_3D_EN0 | WIN_3D_TYPE0 | WIN_3D_LEVEL0 | WIN_CH_BDSH_SEL(SEL_BORDER) | FG_COLOR_BLACK ),
 	OSD_Win_0C_0D			( WIN_SHADOW_EN0 | WIN_SHADOW_W(0), WIN_CH_HSPACE(0), WIN_CH_VSPACE(0) ),	 // 2bytes
	OSD_Win_Attr_0E			( BACK_COLOR | FG_COLOR_WHITE ),
	OSD_Win_SAddr_0F		( WIN_SADDR(INPUTINFO_ADDR) ),

	//----- Input Discription : Zoom 1
	16,
	OSD_Win_Num				( INPUTINFO_OSDWIN+1 ),	// 1 byte Offset for windows
	OSD_Win_CONTROL_00		( WIN_MULTI_EN | WIN_V_EXT_EN0 | WIN_VZOOM(WINZOOMx1) | WIN_HZOOM(WINZOOMx1) ),
	OSD_Win_Blending_01		( 4 ),
	OSD_Win_Start_02_4		( 11, 12 ),	// 3 bytes
#ifdef  WQVGA
	OSD_Win_Height_05		( 2 ),
#else
	OSD_Win_Height_05		( 3 ),
#endif
	OSD_Win_Width_06		( 0x11 ),
	OSD_Win_BorderColor_07	( FG_COLOR_WHITE ),
	OSD_Win_Border_08		( WIN_BD_EN | WIN_BD_WIDTH(1) ),
	OSD_Win_HBorder_09		( WIN_H_BD_W(8) ),
	OSD_Win_VBorder_0A		( WIN_V_BD_W(0) ),
	OSD_Win_3D_0B			( WIN_3D_EN0 | WIN_3D_TYPE0 | WIN_3D_LEVEL0 | WIN_CH_BDSH_SEL(SEL_BORDER) | FG_COLOR_BLACK ),
 	OSD_Win_0C_0D			( WIN_SHADOW_EN0 | WIN_SHADOW_W(0), WIN_CH_HSPACE(0), WIN_CH_VSPACE(0) ),	 // 2bytes
	OSD_Win_Attr_0E			( BACK_COLOR | FG_COLOR_WHITE ),
	OSD_Win_SAddr_0F		( WIN_SADDR(INPUTINFO_ADDR) ),

	0 // End of Data
};          

#ifdef ADD_ANALOGPANEL
CODE BYTE Init_Osd_DisplayInput_A[] = {
	//----- Input Selection : Zoom 1
	16,
	OSD_Win_Num				( INPUTINFO_OSDWIN ),	// 1 byte Offset for windows
	OSD_Win_CONTROL_00		( WIN_MULTI_EN | WIN_V_EXT_EN0 | WIN_VZOOM(INPUTINFO_ZOOM) | WIN_HZOOM(WINZOOMx2) ),
	OSD_Win_Blending_01		( 4 ),
	OSD_Win_Start_02_4		( 12, 13 ),	// 3 bytes
	OSD_Win_Height_05		( 1 ),
	OSD_Win_Width_06		( 9 ),
	OSD_Win_BorderColor_07	( FG_COLOR_WHITE ),
	OSD_Win_Border_08		( WIN_BD_EN0 | WIN_BD_WIDTH(1) ),
	OSD_Win_HBorder_09		( WIN_H_BD_W(1) ),
	OSD_Win_VBorder_0A		( WIN_V_BD_W(0) ),
	OSD_Win_3D_0B			( WIN_3D_EN0 | WIN_3D_TYPE0 | WIN_3D_LEVEL0 | WIN_CH_BDSH_SEL(SEL_BORDER) | FG_COLOR_BLACK ),
 	OSD_Win_0C_0D			( WIN_SHADOW_EN0 | WIN_SHADOW_W(0), WIN_CH_HSPACE(0), WIN_CH_VSPACE(0) ),	 // 2bytes
	OSD_Win_Attr_0E			( BACK_COLOR | FG_COLOR_WHITE ),
	OSD_Win_SAddr_0F		( WIN_SADDR(INPUTINFO_ADDR) ),

	16,
	OSD_Win_Num				( INPUTINFO_OSDWIN+1 ),	// 1 byte Offset for windows
	OSD_Win_CONTROL_00		( WIN_MULTI_EN | WIN_V_EXT_EN0 | WIN_VZOOM(INPUTINFO_ZOOM) | WIN_HZOOM(WINZOOMx2) ),
	OSD_Win_Blending_01		( 4 ),
	OSD_Win_Start_02_4		( 11, 12 ),	// 3 bytes
	OSD_Win_Height_05		( 2 ),
	OSD_Win_Width_06		( 11 ),
	OSD_Win_BorderColor_07	( FG_COLOR_WHITE ),
	OSD_Win_Border_08		( WIN_BD_EN0 | WIN_BD_WIDTH(1) ),
	OSD_Win_HBorder_09		( WIN_H_BD_W(1) ),
	OSD_Win_VBorder_0A		( WIN_V_BD_W(0) ),
	OSD_Win_3D_0B			( WIN_3D_EN0 | WIN_3D_TYPE0 | WIN_3D_LEVEL0 | WIN_CH_BDSH_SEL(SEL_BORDER) | FG_COLOR_BLACK ),
 	OSD_Win_0C_0D			( WIN_SHADOW_EN0 | WIN_SHADOW_W(0), WIN_CH_HSPACE(0), WIN_CH_VSPACE(0) ),	 // 2bytes
	OSD_Win_Attr_0E			( BACK_COLOR | FG_COLOR_WHITE ),
	OSD_Win_SAddr_0F		( WIN_SADDR(INPUTINFO_ADDR) ),

	0
};
#endif
//=============================================================================
//			Initialize Window for MainMenu
//=============================================================================
CODE BYTE Init_Osd_MainMenu[] = {
/*===========================================================================*/
// Normal Panel Menu Window tabel
/*===========================================================================*/
	//----- Top(Left) Window
	16,
	OSD_Win_Num				( FOSDMENU_TOPWIN ),	// 1 byte Offset for windows
	OSD_Win_CONTROL_00		( WIN_EN | WIN_MULTI_EN | WIN_V_EXT_EN | WIN_HZOOM(WINZOOMx2) | WIN_VZOOM(WINZOOMx2) ),
	OSD_Win_Blending_01		( 3 ),
	OSD_Win_Start_02_4		( (OSDHSTART_MAX/2+14), ((OSDVSTART_MAX/2)+FONT_H+10) ),	// 3 bytes
	OSD_Win_Height_05		( 5 ),
	OSD_Win_Width_06		( 2 ),
	OSD_Win_BorderColor_07	( FG_COLOR_WHITE ),
	OSD_Win_Border_08		( WIN_BD_EN | WIN_BD_WIDTH(1) ),
	OSD_Win_HBorder_09		( WIN_H_BD_W(4) ),
	OSD_Win_VBorder_0A		( WIN_V_BD_W(3) ),
	OSD_Win_3D_0B			( WIN_3D_EN0 | WIN_3D_TYPE0 | WIN_3D_LEVEL0 | WIN_CH_BDSH_SEL(SEL_BORDER) | FG_COLOR_BLACK ),
 	OSD_Win_0C_0D			( WIN_SHADOW_EN0 | WIN_SHADOW_W(4), WIN_CH_HSPACE(0), WIN_CH_VSPACE(2) ),	 // 2bytes
	OSD_Win_Attr_0E			( BACK_COLOR | FG_COLOR_WHITE ),
	OSD_Win_SAddr_0F		( WIN_SADDR(FOSDMENU_MAINADDR) ),

	//----- Title Window
	16,
	OSD_Win_Num				( FOSDMENU_TITLEWIN ),	// 1 byte Offset for windows
	OSD_Win_CONTROL_00		( WIN_MULTI_EN | WIN_V_EXT_EN0 | WIN_VZOOM(WINZOOMx1) | WIN_HZOOM(WINZOOMx1) ),
	OSD_Win_Blending_01		( 3 ),
	OSD_Win_Start_02_4		( (OSDHSTART_MAX/2), (OSDVSTART_MAX/2) ),	// 3 bytes
	OSD_Win_Height_05		( 0x09 ),
	OSD_Win_Width_06		( TITLE_MAX_DESC ),
	OSD_Win_BorderColor_07	( WIN_BD_COLOR_EX | FG_COLOR_WHITE ),
	OSD_Win_Border_08		( WIN_BD_EN | WIN_BD_WIDTH(2) ),
	OSD_Win_HBorder_09		( WIN_H_BD_W(FONT_W*4+8) ),
	OSD_Win_VBorder_0A		( WIN_V_BD_W(0x05) ),
	OSD_Win_3D_0B			( WIN_3D_EN0 | WIN_3D_TYPE0 | WIN_3D_LEVEL0 | WIN_CH_BDSH_SEL(SEL_BORDER) | FG_COLOR_BLACK ),
 	OSD_Win_0C_0D			( WIN_SHADOW_EN0 | WIN_SHADOW_W(8), WIN_CH_HSPACE(1), WIN_CH_VSPACE(0x0b) ),	 // 2bytes
	OSD_Win_Attr_0E			( BG_COLOR_BLUE | FG_COLOR_WHITE ),
	OSD_Win_SAddr_0F		( WIN_SADDR(FOSDMENU_SUBADDR) ),

	//----- Sub(Right) Window
	16,
	OSD_Win_Num				( FOSDMENU_ITEMWIN ),	// 1 byte Offset for windows
	OSD_Win_CONTROL_00		( WIN_MULTI_EN | WIN_V_EXT_EN0 | WIN_VZOOM(WINZOOMx1) | WIN_HZOOM(WINZOOMx1) ),
	OSD_Win_Blending_01		( 3 ),
	OSD_Win_Start_02_4		( ((OSDHSTART_MAX/2)+FONT_W*4+18+14), (OSDVSTART_MAX/2+FONT_H+10) ),	// 3 bytes
	OSD_Win_Height_05		( MENU_HEIGHT ),
	OSD_Win_Width_06		( MENU_WIDTH ),
	OSD_Win_BorderColor_07	( FG_COLOR_WHITE ),
	OSD_Win_Border_08		( WIN_BD_EN | WIN_BD_WIDTH(1) ),
	OSD_Win_HBorder_09		( WIN_H_BD_W(FONT_W*2-4) ),
	OSD_Win_VBorder_0A		( WIN_V_BD_W(0x16+1) ),
	OSD_Win_3D_0B			( WIN_3D_EN0 | WIN_3D_TYPE0 | WIN_3D_LEVEL0 | WIN_CH_BDSH_SEL(SEL_BORDER) | FG_COLOR_BLACK ),
 	OSD_Win_0C_0D			( WIN_SHADOW_EN0 | WIN_SHADOW_W(0), WIN_CH_HSPACE(0), WIN_CH_VSPACE(0x06) ),	 // 2bytes
	OSD_Win_Attr_0E			( BACK_COLOR | FG_COLOR_WHITE ),
	OSD_Win_SAddr_0F		( WIN_SADDR(FOSDMENU_SUBADDR+MENU_WIDTH) ),

	//----- 3D Cursor Window
	16,
	OSD_Win_Num				( FOSDMENU_CURSORWIN ),	// 1 byte Offset for windows
	OSD_Win_CONTROL_00		( WIN_MULTI_EN | WIN_V_EXT_EN0 | WIN_VZOOM(WINZOOMx2) | WIN_HZOOM(WINZOOMx2) ),
	OSD_Win_Blending_01		( 3 ),
	OSD_Win_Start_02_4		( ((OSDHSTART_MAX/2)+7+14+1), ((OSDVSTART_MAX/2)+4+FONT_H+10+1) ),	// 3 bytes
	OSD_Win_Height_05		( 1 ),
	OSD_Win_Width_06		( 2 ),
	OSD_Win_BorderColor_07	( FG_COLOR_WHITE ),
	OSD_Win_Border_08		( WIN_BD_EN | WIN_BD_WIDTH(2) ),
	OSD_Win_HBorder_09		( WIN_H_BD_W(0) ),
	OSD_Win_VBorder_0A		( WIN_V_BD_W(0) ),
	OSD_Win_3D_0B			( WIN_3D_EN | WIN_3D_TYPE0 | WIN_3D_LEVEL0 | WIN_CH_BDSH_SEL(SEL_BORDER) | FG_COLOR_BLACK ),
 	OSD_Win_0C_0D			( WIN_SHADOW_EN0 | WIN_SHADOW_W(0), WIN_CH_HSPACE(0), WIN_CH_VSPACE(0) ),	 // 2bytes
	OSD_Win_Attr_0E			( BACK_COLOR | FG_COLOR_WHITE ),
	OSD_Win_SAddr_0F		( WIN_SADDR(FOSDMENU_MAINADDR) ),

	0 // End of Data
/*===========================================================================*/
};

//=============================================================================
//			Initialize Window for 
//=============================================================================
CODE BYTE Init_Osd_BarWindow[] = {

	//----- Bar/Message Window
	16,
	OSD_Win_Num				( FOSDMENU_BARWIN ),	// 1 byte Offset for windows
	OSD_Win_CONTROL_00		( WIN_MULTI_EN | WIN_V_EXT_EN0 | WIN_VZOOM(BAR_ZOOM) | WIN_HZOOM(BAR_ZOOM) ),
	OSD_Win_Blending_01		( 5 ),
	OSD_Win_Start_02_4		( ((PANEL_H-FONT_W*25*BAR_ZOOM)/2), (PANEL_V-FONT_H*2*BAR_ZOOM-76) ),	// 3 bytes
	OSD_Win_Height_05		( 2 ),
	OSD_Win_Width_06		( 25 ),
	OSD_Win_BorderColor_07	( FG_COLOR_WHITE ),
	OSD_Win_Border_08		( WIN_BD_EN | WIN_BD_WIDTH(1) ),
	OSD_Win_HBorder_09		( WIN_H_BD_W(2) ),
	OSD_Win_VBorder_0A		( WIN_V_BD_W(4) ),
	OSD_Win_3D_0B			( WIN_3D_EN | WIN_3D_TYPE | WIN_3D_LEVEL0 | WIN_CH_BDSH_SEL(SEL_BORDER) | FG_COLOR_BLUE ),
 	OSD_Win_0C_0D			( WIN_SHADOW_EN0 | WIN_SHADOW_W(0), WIN_CH_HSPACE(0), WIN_CH_VSPACE(0) ),	 // 2bytes
	OSD_Win_Attr_0E			( BACK_COLOR | FG_COLOR_WHITE ),
	OSD_Win_SAddr_0F		( WIN_SADDR(FOSDMENU_BARADDR) ),			 //OSDMENU_MAINADDR

	0 // End of Data
};

//=============================================================================
//	struct_IdName		
//=============================================================================

CODE struct struct_IdName	struct_InputSelection[]={
	{ FOSD_UNKNOWN,		"" },				//don't remove or change this.
	{ INPUT_CVBS,		"CVBS" },			//don't remove or change this.
											// you can change the order of the followings
	{ INPUT_SVIDEO,		"S-Video" },
	{ INPUT_COMP,		"Component" },
	{ INPUT_PC,			"PC-HV"},
	{ INPUT_DVI,		"DVI" },
	{ INPUT_HDMIPC,		"HDMI" },
	{ INPUT_HDMITV,		"HDMI" },
	{ INPUT_BT656,		"656" },

	{0,					""},				//don't remove or change this.
};


CODE struct struct_IdName struct_VInputStd[]={
	{ FOSD_UNKNOWN		,	"" },				//don't remove or change this.
	{ NTSC,					"NTSC"},			//don't remove or change this.

#ifdef SUPPORT_PAL
	{ PAL,					"PAL"},
#endif

#ifdef SUPPORT_SECAM
	{ SECAM,				"SECAM"},
#endif

#ifdef SUPPORT_PALM
	{ PALM,					"PALM"},
#endif

#ifdef SUPPORT_PALN
	{ PALN,					"PALN"},
#endif

#ifdef SUPPORT_PAL60
	{ PAL60,				"PAL60"},
#endif

#ifdef SUPPORT_NTSC4
	{ NTSC4,				"NTSC4.43"},
#endif

	{0,						""},				//don't remove or change this.
};



//=============================================================================
// TABLES -START
//=============================================================================
//#define DEGITALDEVICE // OSD Type
//Techwell menu
//#define OSDMENU_X_BORDER		0x8//16
//#define OSDMENU_Y_BORDER		0x6//4
//#define OUTLINE_WIDTH			2
//#define NO_TITLE_COLOR			NIL

// Color style IV ---------------------------------------------
/*
#define BACK_COLOR				BG_COLOR_BLACK | EXTENDED_COLOR
#define DEFAULT_COLOR			BG_COLOR_BLACK   | FG_COLOR_WHITE | EXTENDED_COLOR
#define ACTIVE_COLOR			BG_COLOR_WHITE   | FG_COLOR_BLUE | EXTENDED_COLOR	

#define TITLE_COLOR				BG_COLOR_BLACK    | FG_COLOR_YELLOW | EXTENDED_COLOR   
#define CURSOR_COLOR			BG_COLOR_YELLOW | FG_COLOR_BLACK | EXTENDED_COLOR

#define OSD_INFO_COLOR			DEFAULT_COLOR

#define BAR_COLOR				DEFAULT_COLOR	
#define BAR_NUM_COLOR			BAR_COLOR
#define NUM_COLOR				DEFAULT_COLOR
*/
/*
#define BACK_COLOR				BG_COLOR_CYAN
#define DEFAULT_COLOR			BG_COLOR_CYAN   | FG_COLOR_WHITE
//#define ACTIVE_COLOR			BG_COLOR_YELLOW | FG_COLOR_WHITE
#define TITLE_BACK_COLOR		BG_COLOR_BLUE
#define TITLE_COLOR				TITLE_BACK_COLOR | FG_COLOR_WHITE
#define CURSOR_COLOR			BG_COLOR_YELLOW | FG_COLOR_BLUE
#define OSD_INFO_COLOR			DEFAULT_COLOR

#define BAR_COLOR				DEFAULT_COLOR	
#define BAR_NUM_COLOR			DEFAULT_COLOR
#define NUM_COLOR				DEFAULT_COLOR

#define OSDMENU_OUTLINE_COLOR	BG_COLOR_WHITE
*///#define SPACES_BETWEEN_ICON_DESC		0			

#define ICON_X					0
#define ICON_COLOR				DEFAULT_COLOR
#define TOPICON_COLOR			DEFAULT_COLOR
#define NO_ICON_COLOR			NIL

#define DESC_X					2
#define DESC_X_NO_ICON			0
#define DESC_COLOR				DEFAULT_COLOR

//#define CURSOR_START			0

#define ITEMSTARTX				0
#define TITLESTARTX				0



//===================================================================
// OSD Menu Icon define.
//
//#define COLORFONT				COLORFONT_START+0x100

/*
CODE BYTE UTIL_ICON[]={	FONT_RAM, COLORFONT+0x18,	COLORFONT+0x1b, 0}; 
CODE BYTE INPUT_ICON[]={	FONT_RAM, COLORFONT+0x12,	COLORFONT+0x15, 0};
CODE BYTE TV_ICON[]={		FONT_RAM, COLORFONT+0x0c,	COLORFONT+0x0f, 0};
CODE BYTE VIDEO_ICON[]={	FONT_RAM, COLORFONT+0x00,	COLORFONT+0x03, 0};
CODE BYTE AUDIO_ICON[]={	FONT_RAM, COLORFONT+0x06,	COLORFONT+0x09, 0};
CODE BYTE PC_ADJ_ICON[]={	FONT_RAM, 0x118,	0x119,						0};
*/
CODE BYTE VIDEO_ICON[]={	FONT_3BPP, 0x00, 0x03, FONT_ROM, 0};	//NOTE: we can use 0 between Escape to FONT_ROM
CODE BYTE AUDIO_ICON[]={	FONT_3BPP, 0x06, 0x09, FONT_ROM, 0};
CODE BYTE TV_ICON[]={		FONT_3BPP, 0x0c, 0x0f, FONT_ROM, 0};
CODE BYTE INPUT_ICON[]={	FONT_3BPP, 0x12, 0x15, FONT_ROM, 0};
CODE BYTE PC_ADJ_ICON[]={	0xB8,	0xB9,	0};					   //BUGBUG
CODE BYTE UTIL_ICON[]={	 	FONT_3BPP, 0x18, 0x1b, FONT_ROM, 0}; 
/*
#define UTIL_ICON				COLORFONT+0x18,	COLORFONT+0x1b 
#define INPUT_ICON				COLORFONT+0x12,	COLORFONT+0x15
#define TV_ICON					COLORFONT+0x0c,	COLORFONT+0x0f
#define VIDEO_ICON				COLORFONT+0x00,	COLORFONT+0x03
#define AUDIO_ICON				COLORFONT+0x06,	COLORFONT+0x09
#define PC_ADJ_ICON				0x118,	0x119


#define INFO_ICON				0x21,	0x20
#define WIDE_ICON				CODE2FONT+0x02,	CODE2FONT+0x03
#define SRCSEL_ICON				CODE2FONT+0x04,	CODE2FONT+0x05
#define SATU_ICON				CODE2FONT+0x14,	CODE2FONT+0x15
#define HUE_ICON				CODE2FONT+0x10,	CODE2FONT+0x11
#define SHARP_ICON				CODE2FONT+0x12,	CODE2FONT+0x13
#define RESET_ICON				CODE2FONT+0x0c,CODE2FONT+0x0d
#define BRIGHTNESS_ICON			0x10,	0x11
#define CONTRAST_ICON			0x0e,	0x0f
#define PC_COLOR_ICON			CODE2FONT+0x0e,	CODE2FONT+0x0f
#define PC_CLOCK_ICON			0x17,	0x18
#define PC_PHASE_ICON			0x19,	0x1a
#define PC_VPOS_ICON			0x0a,	0x0b
#define PC_HPOS_ICON			0x0c,	0x0d
*/

//==================================================================================
//Common
CODE BYTE NumberOnly[]=	{0};
CODE BYTE DefaultBar[]=	{0};

//==================================================================================
// calculate MAX_SUPPORT
//==================================================================================

#ifdef SUPPORT_SVIDEO
#define ADD_SVIDEO		1
#else
#define ADD_SVIDEO		0
#endif

#ifdef SUPPORT_COMPONENT
#define ADD_COMPONENT	1
#else
#define ADD_COMPONENT	0
#endif

#ifdef SUPPORT_PC
#define ADD_PC			1
#else
#define ADD_PC			0
#endif

#ifdef SUPPORT_DVI
#define ADD_DVI			1
#else
#define ADD_DVI			0
#endif

//#ifdef SUPPORT_HDMI
//#define ADD_HDMI		1
//#else
#define ADD_HDMI		0
//#endif

#ifdef SUPPORT_BT656_LOOP
#define ADD_656			1
#else
#define ADD_656			0
#endif

#ifdef SUPPORT_LVDS
#define ADD_LVDS			1
#else
#define ADD_LVDS			0
#endif

//==========================================================================
//  Menu Structure define
//
//struct MenuType {
//	BYTE	width,	height;
//	BYTE	TitleColor;	
//	BYTE	CursorColor;
//};
//struct MenuType
#define INPUTSELECT_H	3
#define MAX_SUPPORT		(2 + ADD_SVIDEO+ ADD_COMPONENT + ADD_PC + ADD_DVI + ADD_HDMI + ADD_656 + ADD_LVDS)	// +1 default CVBS
#define	INPUTCTRL_H		MAX_SUPPORT
#define	PICTURE_H		4
#define	UTILITY_H		4
#define	SOUND_H			4
#define PCCOLOR_H		4
#define	USER_PICTURE_H	5
#ifdef SUPPORT_PC
#define		PCUTILITY_H	3
#define		PCPICTURE_H	6
#endif

//----------------------------------------------------------------------------
// MenuType
//----------------------------------------------------------------------------
CODE BYTE InputSelectMenu[]= 	{	MENU_WIDTH,		INPUTSELECT_H,	TITLE_COLOR, 	CURSOR_COLOR,  };
CODE BYTE InputSourceMenu[]= 	{	MENU_WIDTH,		INPUTCTRL_H,   	TITLE_COLOR,   	CURSOR_COLOR,  };
CODE BYTE PictureMenu[]= 	 	{	MENU_WIDTH,		PICTURE_H, 		TITLE_COLOR, 	CURSOR_COLOR,	};
CODE BYTE UtilityMenu[]=	 	{ 	MENU_WIDTH,		UTILITY_H,	 	TITLE_COLOR,	CURSOR_COLOR,};
CODE BYTE SoundMenu[]=		 	{	MENU_WIDTH,		SOUND_H,	  	TITLE_COLOR,	CURSOR_COLOR,   };
CODE BYTE PCColorTempMenu[]= 	{	MENU_WIDTH,		PCCOLOR_H,		TITLE_COLOR,	CURSOR_COLOR,	};
CODE BYTE UserPictureMenuType[]= {	MENU_WIDTH,		USER_PICTURE_H,	TITLE_COLOR,	CURSOR_COLOR,	};
CODE BYTE TVMainMenu[]=			{	TOPMENU_WIDTH,	TOPMENU_HEIGHT,	NIL,		 	DEFAULT_COLOR,	};
CODE BYTE SystemMainMenu[]=		{	TOPMENU_WIDTH,	TOPMENU_HEIGHT,	NIL,/*DEFAULT_COLOR,*/	2,	/* means offset 8 */	};
#ifdef SUPPORT_PC
CODE BYTE PCUtilityMenu[]=		{	MENU_WIDTH,		PCUTILITY_H,  	TITLE_COLOR,	CURSOR_COLOR,	};
CODE BYTE PCPictureMenu[]=		{	MENU_WIDTH,		PCPICTURE_H,   	TITLE_COLOR,	CURSOR_COLOR,	};
CODE BYTE PCMainMenu[]=			{	TOPMENU_WIDTH,	TOPMENU_HEIGHT,	NIL,		   	DEFAULT_COLOR, };
#endif


//----------------------------------------------------------------------------------
// DefaultMenu=>DM_MainMenu=>DM_INPUTSOURCE=>DM_INPUTSELECT
//		composite
//		composite2
//		svideo
//		component
//		pc
//		dvi
//		...
CODE struct DefineMenu DM_INPUTSELECT[]={
{	MIT_ACTION, {0},NIL,	Str_Composite,DESC_COLOR,		NIL,NIL,		0, NO, FMID_CHANGE_TO_COMPOSITE, 0	},		//ACTION value
#ifdef SUPPORT_CVBS2
{	MIT_ACTION, {0},NIL,	Str_Composite2,DESC_COLOR,		NIL,NIL,		0, NO, FMID_CHANGE_TO_COMPOSITE2, 0	},		//ACTION value
#endif
#ifdef SUPPORT_SVIDEO
{	MIT_ACTION, {0},NIL,	Str_Svideo,DESC_COLOR,			NIL,NIL,		0, NO, FMID_CHANGE_TO_SVIDEO,0 		},
#endif
#ifdef SUPPORT_COMPONENT
{	MIT_ACTION, {0},NIL,	Str_Component,DESC_COLOR,		NIL,NIL,		0, NO, FMID_CHANGE_TO_COMPONENT,0 	},
#endif
#ifdef SUPPORT_PC
{	MIT_ACTION, {0},NIL, 	Str_Pc,	DESC_COLOR,				NIL,NIL,		0,NO,FMID_CHANGE_TO_PC,0 			},
#endif
#ifdef SUPPORT_DVI
{	MIT_ACTION, {0},NIL,	Str_DVI,DESC_COLOR,				NIL,NIL,		0,NO,FMID_CHANGE_TO_DVI,0			},
#endif
#if defined(SUPPORT_HDMI)
{	MIT_ACTION, {0},NIL, 	Str_HDMI,DESC_COLOR,			NIL,NIL,		0,NO,FMID_CHANGE_TO_HDMI,0			},
#endif
#ifdef SUPPORT_BT656_LOOP
{	MIT_ACTION, {0},NIL,	Str_656,DESC_COLOR,				NIL,NIL,		0,NO,FMID_CHANGE_TO_656,0			},
#endif
#ifdef SUPPORT_LVDSRX
{	MIT_ACTION, {0},NIL,	Str_LVDS,DESC_COLOR,			NIL,NIL,		0,NO,FMID_CHANGE_TO_LVDS,0			},
#endif
{	0, 		   {0},NIL,		0,NIL,							0,0,		0,0,	0,0}	 //Terminator
};

//----------------------------------------------------------------------------------
// DefaultMenu=>DM_MainMenu=>DM_INPUTSOURCE=>DM_BT656SELECT
//		decoder
//		aRGB
//		DTV
//		LVDS
//		PANEL
//		OFF
//		...
CODE struct DefineMenu DM_BT656SELECT[]={
{	MIT_ACTION, {0},NIL,	Str_Composite,DESC_COLOR,		NIL,NIL,		0,NO,FMID_BT656_FROM_DEC, 0	},		//ACTION value
#if defined(SUPPORT_PC)
{	MIT_ACTION, {0},NIL, 	Str_Pc,	DESC_COLOR,				NIL,NIL,		0,NO,FMID_BT656_FROM_PC,0 	},
#endif
{	MIT_ACTION, {0},NIL, 	Str_HDMI,DESC_COLOR,			NIL,NIL,		0,NO,FMID_BT656_FROM_HDMI,0	},
{	MIT_ACTION, {0},NIL,	Str_LVDS,DESC_COLOR,			NIL,NIL,		0,NO,FMID_BT656_FROM_LVDS,0	},
{	MIT_ACTION, {0},NIL,	Str_Panel,DESC_COLOR,			NIL,NIL,		0,NO,FMID_BT656_FROM_PANEL,0	},
{	MIT_ACTION, {0},NIL,	Str_Off,DESC_COLOR,				NIL,NIL,		0,NO,FMID_BT656_FROM_OFF,0	},
{	0, 		   {0},NIL,		0,NIL,							0,0,			0,0,	0,0}	 //Terminator
};


//----------------------------------------------------------------------------------
// DefaultMenu=>DM_MainMenu=>DM_INPUTSOURCE
//		input => DM_INPUTSELECT
//		information => 
//
CODE struct DefineMenu DM_INPUTSOURCE[]={
{	MIT_SELMENU,{0},NIL, 	Str_Input,DESC_COLOR,			NIL,NIL,		DM_INPUTSELECT,NO,		FMID_INPUTSELECTION,		InputSelectMenu	 },
{	MIT_ACTION, {0},NIL, 	Str_Information,DESC_COLOR,		NIL,NIL,		0,NO,					FMID_DISP_INFO, 0	},
{	MIT_SELMENU,{0},NIL, 	Str_656,DESC_COLOR,				NIL,NIL,		DM_BT656SELECT,NO,		FMID_BT656_SRC_SELECTION,	InputSelectMenu	 },
{	0, 		   {0},NIL,		0,NIL,							0,0,		0,0,	0,0}	 //Terminator
};
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------

CODE struct DefineMenu DM_Utility[]={
{	MIT_NUM,   {0},NIL,		Str_SleepTimer,DESC_COLOR,		NIL,NIL,	0,NO,	FMID_SLEEP_TIMER,		NumberOnly		},
#ifdef WIDE_SCREEN
{	MIT_E_STR, {0},NIL,		Str_WideScreen,DESC_COLOR,		NIL,NIL,	0,NO,	FMID_WIDESCREEN,			WideScreenChoice	},
#endif // WIDE_SCREEN
#ifdef SUPPORT_OSDPOSITIONMOVE
{	MIT_E_STR, {0},NIL,		Str_OSDPosition,DESC_COLOR,		NIL,NIL,	0,NO,	FMID_OSDPOSITIONMODE, 	OsdPositionModeChoice  	},
#endif
{	MIT_E_STR, {0},NIL,		Str_OSDLang,DESC_COLOR,			NIL,NIL,	0,NO, 	FMID_OSDLANG,			OSDLangString		},
{	MIT_ACTION,{0},NIL,		Str_Reset,DESC_COLOR,			NIL,NIL,	0,NO,	FMID_RESET_VIDEOVALUE,0						},
{	0, 		   {0},NIL,		0,NIL,							0,0,		0,0,	0,0}	 //Terminator
};

//----------------------------------------------------------------------------------

CODE struct DefineMenu DM_Sound[]={
{	MIT_BAR, 	{0},NIL,	Str_Volume,DESC_COLOR,			NIL,NIL,	0,NO,	FMID_AUDIO_VOLUME,		DefaultBar		  },
{	MIT_BAR, 	{0},NIL,	Str_Treble,DESC_COLOR,			NIL,NIL,	0,NO,	FMID_AUDIO_TREBLE,		DefaultBar		  },
{	MIT_BAR, 	{0},NIL,	Str_Bass,DESC_COLOR,			NIL,NIL,	0,NO,	FMID_AUDIO_BASS,			DefaultBar		  },
{	MIT_BAR, 	{0},NIL,	Str_Balance,DESC_COLOR,			NIL,NIL,	0,NO,	FMID_AUDIO_BALANCE,		DefaultBar		  },
{	0, 		   {0},NIL,		0,NIL,							0,0,		0,0,	0,0}	 //Terminator
};


//----------------------------------------------------------------------------------

CODE struct DefineMenu DM_PCColor[]={
{	MIT_E_STR,	{0},NIL,	Str_ColorTemp,DESC_COLOR,		NIL,NIL,	0,NO,	FMID_PC_COLOR_TEMP,		  ModeChoice,	},
{	MIT_BAR, 	{0},NIL,	Str_Red,DESC_COLOR,				NIL,NIL,	0,NO,	FMID_PC_COLOR_R,		  DefaultBar	},
{	MIT_BAR, 	{0},NIL,	Str_Green,DESC_COLOR,			NIL,NIL,	0,NO,	FMID_PC_COLOR_G,		  DefaultBar	},
{	MIT_BAR, 	{0},NIL,	Str_Blue,DESC_COLOR,			NIL,NIL,	0,NO,	FMID_PC_COLOR_B,		  DefaultBar	},
{	0, 		   {0},NIL,		0,NIL,							0,0,		0,0,	0,0}	 //Terminator
};

//============================================================================================

//============================================================================================
//Picture

CODE struct DefineMenu DM_UserPicture[]={
{	0, 		   {0},NIL,		0,NIL,							0,0,		0,0,	0,0}	 //Terminator
};


//----------------------------------------------------------------------------------
// DefaultMenu=>DM_MainMenu=>DM_Picture
//
//Picture
//	Contrast
//	Brightness
//	Saturation
//	Hue
//	Sharpness
//	BackLight
CODE struct DefineMenu DM_Picture[]={
{	MIT_E_STR,	{0},NIL,	Str_Picture,DESC_COLOR,		  	NIL,NIL,	0,NO,	FMID_VIDEO_MODE,		 ModeChoice,   },
{	MIT_BAR,  	{0},NIL,	Str_Contrast,DESC_COLOR,		NIL,NIL,	0,NO,	FMID_VIDEO_CONTRAST,	 DefaultBar	   },
{	MIT_BAR,  	{0},NIL,	Str_Brightness,DESC_COLOR,		NIL,NIL,	0,NO,	FMID_VIDEO_BRIGHTNESS,DefaultBar	  },				   
{	MIT_BAR,  	{0},NIL,	Str_Saturation,	DESC_COLOR,		NIL,NIL,	0,NO,	FMID_VIDEO_SATURATION,DefaultBar	  },
{	MIT_BAR,  	{0},NIL,	Str_Hue,DESC_COLOR,				NIL,NIL,	0,NO,	FMID_VIDEO_HUE,		 DefaultBar	  },
{	MIT_BAR,  	{0},NIL,	Str_Sharpness,DESC_COLOR,		NIL,NIL,	0,NO,	FMID_VIDEO_SHARPNESS,  DefaultBar   	},
{	MIT_BAR,  	{0},NIL,	Str_Backlight,	DESC_COLOR,		NIL,NIL,	0,NO,	FMID_VIDEO_BACKLIGHT,  DefaultBar   	},
{	0, 		   {0},NIL,		0,NIL,							0,0,		0,0,	0,0}	 //Terminator
};



//----------------------------------------------------------------------------------
// DefaultMenu=>DM_MainMenu
//		Input		=>DM_INPUTSOURCE
//		Picture		=>DM_Picture
//		Audio		=>DM_Sound
//		Utility		=>DM_Utility
//----------------------------------------------------------------------------------
CODE struct DefineMenu DM_MainMenu[]={
{	MIT_MENU, INPUT_ICON,TOPICON_COLOR,Str_Input,NIL,		NIL,NIL,	DM_INPUTSOURCE,NO,		0,		InputSourceMenu		},
{	MIT_MENU, VIDEO_ICON,TOPICON_COLOR,Str_Picture,NIL,	   	NIL,NIL,	DM_Picture,NO,			FMID_VIDEOPICTURE,		PictureMenu	},
{	MIT_MENU, AUDIO_ICON,TOPICON_COLOR,Str_Sound,NIL,		NIL,NIL,	DM_Sound,NO,		   	0,	   	SoundMenu		},
{	MIT_MENU, UTIL_ICON,TOPICON_COLOR,Str_Utility,NIL,		NIL,NIL,	DM_Utility,NO,			0,		UtilityMenu		},
{	0, 		   {0},NIL,		0,NIL,							0,0,		0,0,	0,0}	 //Terminator
};


//----------------------------------------------------------------------------------
// Menu Start Position.
// DefaultMenu
// 		=>DM_MainMenu
//
//----------------------------------------------------------------------------------
CODE struct DefineMenu DefaultMenu[]={
{	MIT_MENU,  0,ICON_COLOR,	0,NIL,						NIL,NIL,	DM_MainMenu,NO,			 0,	SystemMainMenu   },
{	0, 		   {0},NIL,		0,NIL,							0,0,		0,0,	0,0}	 //Terminator
};


//#endif


//============================================================================================
#ifdef SUPPORT_PC

//----------------------------------------------------------------------------------

CODE struct DefineMenu DM_PCUtility[]={
{	MIT_ACTION, {0},NIL,	Str_AutoAdjust,	DESC_COLOR,	  	NIL,NIL,	0,NO,	 FMID_AUTOADJUST		   },
{	MIT_ACTION, {0},NIL,	Str_AutoColor,DESC_COLOR,		NIL,NIL,	0,NO,	 FMID_PCCOLORADJUST	   },
#ifdef WIDE_SCREEN
{	MIT_E_STR,	{0},NIL,	Str_WideScreen,	DESC_COLOR,		NIL,NIL,	0,NO,	 FMID_WIDESCREEN,	   WideScreenChoice	  },
#endif // WIDE_SCREEN
#ifdef SUPPORT_OSDPOSITIONMOVE
{	MIT_E_STR, 	{0},NIL, 	Str_OSDPosition,DESC_COLOR,		NIL,NIL,	0,NO,	 FMID_OSDPOSITIONMODE,  OsdPositionModeChoice  },
#endif
{	MIT_E_STR, 	{0},NIL,	Str_OSDLang,DESC_COLOR,			NIL,NIL,	0,NO,	 FMID_OSDLANG,	  OSDLangString,   },
{	MIT_ACTION, {0},NIL,	Str_Reset,DESC_COLOR,			NIL,NIL,	0,NO,	 FMID_RESET_PCVALUE,0	  	},
{	0, 		   {0},NIL,		0,NIL,							0,0,		0,0,	0,0}	 //Terminator
};


//----------------------------------------------------------------------------------

//	Brightness

CODE struct DefineMenu DM_PCPicture[]={
{	MIT_BAR,	{0},NIL, 	Str_Brightness,DESC_COLOR,		NIL,NIL,	0,NO,	   FMID_PANEL_BRIGHTNESS,	DefaultBar	 	},
{	MIT_BAR,	{0},NIL,	Str_Contrast,DESC_COLOR,		NIL,NIL,	0,NO,	   FMID_PANEL_CONTRAST,	DefaultBar	   	},
{	MIT_BAR,	{0},NIL,	Str_HPosition,DESC_COLOR,		NIL,NIL,	0,NO,	  FMID_PC_HPOS,			   	DefaultBar		},
{	MIT_BAR,	{0},NIL,	Str_VPosition,DESC_COLOR,		NIL,NIL,	0,NO,	  FMID_PC_VPOS,			   	DefaultBar	   },
{	MIT_NUM,	{0},NIL,	Str_Phase,DESC_COLOR,			NIL,NIL,	0,NO,	  FMID_PC_PHASE,				NumberOnly	   },
{	MIT_NUM,	{0},NIL,	Str_Clock,DESC_COLOR,			NIL,NIL,	0,NO,	  FMID_PC_CLOCK,				NumberOnly	   },
{	0, 		   {0},NIL,		0,NIL,							0,0,		0,0,	0,0}	 //Terminator
};

//----------------------------------------------------------------------------------

CODE struct DefineMenu DM_PCMenuList[]={
{	MIT_MENU,	{INPUT_ICON},TOPICON_COLOR, Str_Input,NIL,		NIL,NIL,	DM_INPUTSOURCE,NO,	 0,		InputSourceMenu		},
{	MIT_MENU,	{PC_ADJ_ICON},TOPICON_COLOR,Str_Picture,NIL,	NIL,NIL,	DM_PCPicture,NO,	 0,		PCPictureMenu		},
{	MIT_MENU,	{VIDEO_ICON},TOPICON_COLOR,Str_ColorTemp,NIL,	NIL,NIL,	DM_PCColor,NO,		 FMID_PC_COLOR_MODE,	   PCColorTempMenu	},
{	MIT_MENU,	{AUDIO_ICON},TOPICON_COLOR,Str_Sound,NIL,		NIL,NIL,	DM_Sound,NO,	   	0,	SoundMenu	},
{	MIT_MENU,	{UTIL_ICON},TOPICON_COLOR,Str_Utility,NIL,		NIL,NIL,	DM_PCUtility,NO,	0,	PCUtilityMenu	},
{	0, 		   {0},NIL,		0,NIL,								0,0,		0,0,	0,0}	 //Terminator
};


//----------------------------------------------------------------------------------
CODE struct DefineMenu PCMenu[]={
{	MIT_MENU,	0,ICON_COLOR,	0,NIL,						NIL,NIL,	DM_PCMenuList,NO,	0,		PCMainMenu	}, 				//TypeData:MenuType
{	0, 		   {0},NIL,		0,NIL,							0,0,		0,0,	0,0}	 //Terminator
};

#endif

//=============================================================================
// TABLES -END
//=============================================================================
#undef ADD_SVIDEO
#undef ADD_COMPONENT
#undef ADD_PC
#undef ADD_DVI
#undef ADD_HDMI
#undef ADD_656


#endif //..SUPPORT_FOSD_MENU

