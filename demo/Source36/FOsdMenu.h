/*****************************************************************************/
/*																			 */
/*                           OSD menu                                        */
/*																			 */
/*****************************************************************************/
#ifndef __FOSDMENU_H__
#define __FOSDMENU_H__

//=============================================================================
// Define
//=============================================================================


//=============================================================================
// Declare
//=============================================================================
//typedef struct OsdBmpCommand{
//	WORD winno;
//	WORD sx;
//	WORD sy;
//	WORD w;
//	WORD h;
//	WORD dx;
//	WORD dy;
//	WORD c;
//};

//union OsdBmpCmdDefined{
//	WORD 	arg[8];
//	struct  OsdBmpCommand Cmd;
//};

//struct CmdOsd {
//	BYTE Cmd;
//	BYTE Winno;
//	WORD x;
//	WORD y;
//	WORD w;
//	WORD h;
//	BYTE color;
//	BYTE dat;  // Using Zoom infomation,.. so far
////	FAR CONST BYTE	*Ptr;
//};

// Operation Command

//--------------------------------

// Method of "DisplayedOSD" :  0000xxxx : Menu related,  xxxx0000 : Independent with Menu


enum bar_type{
	BAR_LFKEY,
	BAR_OPEN,
	BAR_EMPTY,
	BAR_CURSOR,
   	BAR_CLOSE,
	BAR_FULL,
	BAR_RTKEY
} ;

enum key_type{
	NOTHING,    // Noting happens	
	UPMENU,     // MENUKEY
	DNMENU,	    // SELKEY/LEFTKEY
	CURSORINC,  // UP
	CURSORDEC,  // DOWN
	VALUEINC,   // RIGHT
	VALUEDEC,   // LEFT
	EXITMENU,   // EXIT
	ACTITEM    // LEFT
}; 

enum KeyDef {
	Menu_Key,
	Left_Key,
	Right_Key,
	Up_Key,
	Down_Key,
	Sel_Key,
	Input_Key,
	Mute_Key
};

//---------------------------------------------------------------
struct _ConvColor {
	BYTE addr, color;	//EnConv[7]|EnOverwrite[6]|Addr[5:0], Conversion Color
};



//---------------------------------------------------------------
//typedef BYTE (*fpActFunc)(void);
//typedef WORD (*fpGetFunc)(void);
//typedef WORD (*fpSetFunc)(WORD val);



//----------------------------------------------------------------
struct MenuType {
	BYTE	width,	height;
	BYTE	TitleColor;	
	BYTE	CursorColor;
};

struct NumType {
	BYTE	PrePostCharLen;		// high nibble(pre) / low nibble(post)
	BYTE	PrePostChar[4];
};

struct NumberStringType {
	BYTE	x, y;
	BYTE    width;
	BYTE	BackColor;
	BYTE	Radix;
	BYTE	NumColor;
	BYTE    MaxLen;				// maximum length of string
	BYTE	PrePostCharLen;		// high nibble(pre) / low nibble(post)
	BYTE	PrePostChar[3];
};

struct TimeType {
	BYTE	x, y;
	BYTE    width;
	BYTE	BackColor;
	BYTE	Format;				//_24H_TIME
	BYTE	TimeColor;
	BYTE    CursorColor;
};

struct EnumType {
	BYTE    Sz;//size
	BYTE    ByLang;
	BYTE	Str[15];			// doesn't matter the size !!!
};

#define ICON_CH_SZ		2
//#define BAR_NUM_SZ		4
#define ICON_BUF_SZ		2
struct DefineMenu {
	BYTE						Type;				// MENU, CHOICEMENU, BAR, NUMBER, TIME, ACTION, ENUM_STRING

	BYTE				CODE	*Icon;
	BYTE						IconColor;			//NIL: not to display icon
	BYTE						**Desc;
	BYTE						DescColor;			//NIL: not to display desc. (IconColor==NIL && DescColor==NIL)->Last Item
					
	BYTE						PreFnId, 
								PostFnId;
//	void		(*CalltoFunction)(void);				    // Un/Dn/Sel Function

	struct DefineMenu	CODE	*Child;
	BYTE						ActiveWithCursor;
	BYTE						Id;
	BYTE				CODE	*TypeData;	//MenuType, 
											//BarType, 
											//NumType,,,
} ;

struct PictureInfo {

	BYTE Register_Addr;
	BYTE EEPROM_Addr;
	struct RegisterInfo 	CODE	*Control_Range;
};



extern DATA	BYTE	OSDLang;
extern DATA char	AudioVol, AudioBalance, AudioBass, AudioTreble, AudioEffect;



//----------------------------------------------------------------

void	InitLogo( void );
void 	EnterMenu( void );
void 	ExitMenu( void );
void 	MoveCursorNext( void );
void 	MoveCursorPrev( void );
void 	MoveCursor( BYTE key );
BYTE 	SystemDemo( void );
BYTE	InputChangeCVBS( void );
BYTE	InputChangeSVIDEO( void );
BYTE	InputChangeYUV( void );
BYTE	InputChangeRGB( void );
BYTE	InputChange656( void );
BYTE	InputChangeCVBS2( void );




BYTE GetVInputStdInx(void);

//=======================================================================
// 
//BYTE GetOSDDuration(void);

//#if defined( SUPPORT_PC ) || defined( SUPPORT_DTV )
//void ResetPCValue(void);
//#endif
BYTE GetOSDMenuLevel( void );

void FOsdMenuDrawList(BYTE);
void Display_EnterPin(void);
//void FOsdHighMenu_sub(void);
void ResetOSDValue(void);
//BYTE OnChangingValue(void);
//Not used yet *******************BYTE OnChangingTime(void);

//WORD GetOSDclock(void);

void FOsdMenuOpen(void);
void FOsdMenuInit(void);
void FOsdMenuClose(void);
void FOsdHighMenu(void);
void FOsdMenuProcSelectKey(void);
void FOsdMenuMoveCursor(BYTE flag );
//void OSDLeftRight(BYTE flag );
void FOsdMenuValueUpDn(BYTE flag);
WORD FOsdMenuSetItemValue( BYTE id, BYTE flag );
BYTE FOsdMenuDoAction(BYTE id);

//BYTE SaveValue1(void);
//void ClearValueWin(void);

void OSDNumber( BYTE num );

void Set4WideScreen(BYTE id);

BYTE SetPCColorTemp(BYTE flag);
void ResetPanelAttributeValue(void);
WORD FOsdMenuGetItemValue( BYTE);
void DisplayViewBar( WORD,BYTE);
void DrawNum(WORD, CODE_P struct NumType *, WORD);
void DrawStringByLang( WORD, CODE_P struct EnumType *, WORD);

void DisplayVolumebar(BYTE val);
void InitOSDMenuXY(void);

void Display_TVGuide_Window(void);
void Display_TVGuide_Status(void);

void ChangeVchip_LevelCursor(BYTE);
void ChangeVchip_ContentCursor(BYTE);
void Select_VchipLevel(void);
void SetVideoMode(BYTE mode);
BYTE GetVideoDatawithOffset(BYTE offset);
void SetVideoContrast(BYTE newv);
void SetVideoBrightness(BYTE newv);
void SetVideoHue(BYTE newv) ;
void SetVideoSaturation(BYTE svtype, BYTE newv) ;
BYTE SetVideoSharpness(BYTE flag) ;
//void SetVideoBacklight(BYTE newv) ;

void DisplayLineOSD(void);	// OSD Line display Demo

//-----------------
// audio
//-----------------
void ToggleAudioMute(void);
void CheckAndClearMuteOn(void);
BYTE IsAudioMuteOn(void);
BYTE ChangeVol(char amount);
BYTE ChangeBalance(char amount);
BYTE ChangeBass(char amount);
BYTE ChangeTreble(char amount);


//-------------------------------------------------------------------
// OSD Window Information
//-------------------------------------------------------------------
// Window Number
#define FOSDMENU_CURSORWIN		0
#define FOSDMENU_ITEMWIN		FOSDMENU_CURSORWIN+1
#define FOSDMENU_TOPWIN			FOSDMENU_CURSORWIN+2
#define FOSDMENU_TITLEWIN		FOSDMENU_CURSORWIN+3
#define FOSDMENU_BARWIN			FOSDMENU_CURSORWIN

#define TECHWELLOGO_OSDWIN		0	//1	
#define TVINFO_OSDWIN			0	//1	
#define FONTTEST_OSDWIN			0	//1	
#define INPUTINFO_OSDWIN		1	//2  
#define RATINGINFO_OSDWIN		0	//1	
#define PCINFO_OSDWIN			2	//3	

//-------------------------------------------------------------------
// Window Start address
#define FOSDMENU_MAINADDR	0
#define FOSDMENU_SUBADDR     FOSDMENU_MAINADDR+10    //25*9:196
#define FOSDMENU_BARADDR		175 //FOSDMENU_SUBADDR+225    //256   //25x2:50

#define INPUTINFO_ADDR		160	// 9x1:9
#ifdef WQVGA
#define INPUTINFO1_ADDR		INPUTINFO_ADDR+17	//17x3:51
#else
#define INPUTINFO1_ADDR		INPUTINFO_ADDR+34	//17x2:51
#endif

#define MUTEINFO_ADDR	    INPUTINFO1_ADDR+51	// 4x1:4
#define TVCHINFO_ADDR		MUTEINFO_ADDR//324	// 3X1:3
//??#define LOGO_ADDR			0
#define RATINGINFO_ADDR		0	//addr:210(8x1:8)
#define PCMEASUREINFO_ADDR  MUTEINFO_ADDR //10x1

//#ifdef BIG_FONT
//#define	FONT_W	16
//#define	FONT_H	26
//#else
#define	FONT_W	12
#define	FONT_H	18
//#endif

//--------------------------------------------------------------------
// OSD Menu Position Range
#if ((defined WQVGA) && (defined ANALOG_7INCH)) || ((defined QVGA) && (defined SHARP_LQ050Q5DR01))
	#define OSD_PHR_ PANEL_H/2
#else
	#define OSD_PHR_ PANEL_H
#endif

#define OSDHSTART_MAX (OSD_PHR_-(FONT_W+1)*25 - 56*2 -4)  //PANEL_H - (pixelsize +Hspace)*MenuWidth - Hborder*2 - outlinewidth*2
#define OSDVSTART_MAX (PANEL_V-(FONT_H+11)*9 - 5*2 ) //  PANEL_V - (Linesize+Vspace)*MENU_HEIGHT - Vborder*2  - outlinewidth*2

#define OSDMENUX_MIN 1//0
#define OSDMENUX_MAX (OSDHSTART_MAX>>3)
#define OSDMENUX_50  (OSDHSTART_MAX>>4)

#define OSDMENUY_MIN 2//0
#define OSDMENUY_MAX (OSDVSTART_MAX>>2)
#define OSDMENUY_50  (OSDVSTART_MAX>>3)

//--------------------------------------------------------------------
// OSD Menu Width, Height 

#define TOTAL_MAX_HIGHT			9

#define TOPMENU_WIDTH			2
#define TOTAL_TOP_HEIGHT		5	// Include blank line!

#define TOPMENU_HEIGHT			4	

#if (defined QVGA) && (defined SHARP_LQ050Q5DR01)
#define MENU_WIDTH				17 	//23//25 
#define	TITLE_MAX_DESC			17	//25	//
#define BAR_LENGTH				0
#else
#define MENU_WIDTH				23	//25 
#define	TITLE_MAX_DESC			23	//25	//
#define BAR_LENGTH				8
#endif

#define MENU_HEIGHT				7	//6 

#define BARNUM_LENGTH 11

//#define BACK_COLOR				BG_COLOR_CYAN
#define BACK_COLOR				BG_COLOR_CYAN	| FG_COLOR_BLUE
#define DEFAULT_COLOR			BG_COLOR_CYAN   | FG_COLOR_WHITE
//#define ACTIVE_COLOR			BG_COLOR_YELLOW | FG_COLOR_WHITE
#define TITLE_BACK_COLOR		BG_COLOR_MAGENTA
#define TITLE_COLOR				TITLE_BACK_COLOR | FG_COLOR_YELLOW
#define COMMENT_COLOR			TITLE_BACK_COLOR | FG_COLOR_WHITE
#define CURSOR_COLOR			BG_COLOR_YELLOW | FG_COLOR_BLUE
#define OSD_INFO_COLOR			DEFAULT_COLOR

#define BAR_COLOR				DEFAULT_COLOR	
#define BAR_NUM_COLOR			DEFAULT_COLOR
#define NUM_COLOR				DEFAULT_COLOR

#define OSDMENU_OUTLINE_COLOR	BG_COLOR_BLUE


//DisplayedOSD
//		new	  |	MENU	TVINFO		CC		INPUTINFO		PCINFO		RATINGINFO
//	current   |
//============|=========================================================================
//	MENU	  |			TVINFO		--		INPUTINFO		PCINFO		MENU + RATINGINFO	
//			  |						 
//------------|-------------------------------------------------------------------------
//	TVINFO	  |	MENU	TVINFO		--		INPUTINFO		PCINFO		TVINFO+RATING
//------------|-------------------------------------------------------------------------
//	CC		  |	MENU->	TVINFO->			INPUTINFO->		PCINFO		RATINGINFO
//			  |	 CC		 CC					 CC				 		 
//------------|-------------------------------------------------------------------------
//	INPUTINFO |	MENU	TVINFO		--		INPUTINFO		PCINFO		--
//------------|-------------------------------------------------------------------------
//	PCINFO	  |	MENU	x			x		INPUTINFO		PCINFO		x
//------------|-------------------------------------------------------------------------
//	RATINGINFO|	MENU	TVINFO	   RATING   INPUTINFO		x			RATINGINFO
//									

//=======================================================================================
// OSD Menu Constant define.
//=======================================================================================
//Time type
#define _24H_TIME		1

//for previous, next, parent or child
#define YES				0xfe	
#define NO				0


//#define OVERWRITE		0
//#define POPUP			1

//CursorMove
#define VERT			0
#define HORI			1

//---------------------------
// MenuItem Type
//---------------------------
#define MIT_MENU		1		//linked TypeData:MenuType
#define MIT_SELMENU		2		//CHOICEMENU  linked TypeData:MenuType  ..MenuFormat
#define MIT_ACTION		3		//no TypeData
#define MIT_BAR			4
#define MIT_NUM			5
#define MIT_CHOICE		6
#define MIT_E_STR		7		//#define ENUM_STRING		7 //YESNO			7
#define MIT_CHOICE1		8
#define MIT_TIME		9
#define NUMBER_STRING	10

//Bar type
#define LEFT_FILLED		1
#define RIGHT_FILLED	2
#define NO_FILLED		3

//OSDmenu up and down key
#define FOSD_UP					1
#define FOSD_DN					2
#define FOSD_RIGHT				3
#define FOSD_LEFT				4

// Action Id
#define	FMID_DISP_INFO			0x16

#define	FMID_INPUTSELECTION				0x2f

#define FMID_CHANGE_TO_COMPOSITE		(0x20|INPUT_CVBS)			// Low nibble need to start from 1 
#define FMID_CHANGE_TO_COMPOSITE2		(0x20|INPUT_CVBS2)			// Low nibble need to start from 1 
#define FMID_CHANGE_TO_SVIDEO			(0x20|INPUT_SVIDEO)				// so that it match with COMPOSITE...
#define FMID_CHANGE_TO_COMPONENT		(0x20|INPUT_COMP)		
#define FMID_CHANGE_TO_656				(0x20|INPUT_BT656)		
#define FMID_CHANGE_TO_LVDS				(0x20|INPUT_LVDS)		
//#define FMID_CHANGE_TO_TV				(0x20|TV)					
#define FMID_CHANGE_TO_PC				(0x20|INPUT_PC)					
#define FMID_CHANGE_TO_DVI				(0x20|INPUT_DVI)			
//#define FMID_CHANGE_TO_SCART			(0x20|SCART)				
#define FMID_CHANGE_TO_HDMI				(0x20|INPUT_HDMIPC)

#define FMID_BT656_SRC_SELECTION		0x2e

#define FMID_BT656_FROM_DEC				(0x20| BT656_SRC_DEC)
#define FMID_BT656_FROM_PC				(0x20| BT656_SRC_ARGB)
#define FMID_BT656_FROM_HDMI			(0x20| BT656_SRC_DTV)
#define FMID_BT656_FROM_LVDS			(0x20| BT656_SRC_LVDS)
#define FMID_BT656_FROM_PANEL			(0x20| BT656_SRC_PANEL)
#define FMID_BT656_FROM_OFF				(0x20| BT656_SRC_OFF)
#define FMID_BT656_FROM_AUTO			(0x20| BT656_SRC_AUTO)


#define FMID_OSDPOSITIONMODE     	0x10
#define FMID_OSD_VPOS				0x11
#define FMID_OSD_HPOS				0x12
#define FMID_OSD_HALFTONE			0x13
#define FMID_OSD_DURATION			0x14
#define FMID_RESET_OSDVALUE			0x15
#define FMID_OSDLANG				0x16

#define FMID_PANEL_CONTRAST		0x58
#define FMID_PANEL_BRIGHTNESS	0x59
#define FMID_PANEL_HUE			0x5a
#define FMID_PANEL_SHARPNESS		0x5b
#define FMID_PANEL_SATURATION	0x5c

#define FMID_PC_VPOS				0x43
#define FMID_PC_HPOS				0x44
#define FMID_PC_HSIZE			0x4d

#define FMID_PC_CLOCK			0x45
#define FMID_PC_PHASE			0x46

#define FMID_AUTOADJUST			0x47
#define FMID_AUTOADJUST2			0x48
//#define FMID_SAVE_AUTOADJUST		0x49
#define FMID_RESET_AUTOADJUST	0x4a

#define FMID_PCCOLORADJUST		0x4b
#define FMID_SAVE_COLORADJUST	0x4c
//#define FMID_RESET_COLORADJUST	0x4d

#define FMID_RESET_PCVALUE		0x4e

#define FMID_PC_COLOR_MODE		0x3f
#define FMID_PC_COLOR_TEMP		0x30
#define FMID_PC_COLOR_MODE1		0x31
#define FMID_PC_COLOR_MODE2		0x32
#define FMID_PC_COLOR_MODE3		0x33
#define FMID_PC_COLOR_USER		0x34
#define FMID_PC_COLOR_R			0x35
#define FMID_PC_COLOR_G			0x36
#define FMID_PC_COLOR_B			0x37

#define FMID_VIDEO_MODE			0x90
#define FMID_VIDEO_MODE1		0x91
#define FMID_VIDEO_MODE2		0x92
#define FMID_VIDEO_MODE3		0x93
#define FMID_VIDEO_USER			0x94
#define FMID_VIDEO_CONTRAST		0x95
#define FMID_VIDEO_BRIGHTNESS	0x96
#define FMID_VIDEO_SATURATION	0x97
#define FMID_VIDEO_HUE			0x98
#define FMID_VIDEO_SHARPNESS	0x99
#define FMID_VIDEO_BACKLIGHT	0x9a		// add new value 
#define FMID_RESET_VIDEOVALUE	0x9b
#define FMID_VIDEOPICTURE		0x9f

#define FMID_INPUTSTD			0x6f
#define CHANGE_TO_AUTO			0x60
#define CHANGE_TO_NTSC		(0x60 | NTSC)
#define CHANGE_TO_PAL		(0x60 | PAL)
#define CHANGE_TO_SECAM		(0x60 | SECAM)
#define CHANGE_TO_NTSC4		(0x60 | NTSC4)
#define CHANGE_TO_PALM		(0x60 | PALM)
#define CHANGE_TO_PALN		(0x60 | PALN)
#define CHANGE_TO_PAL60		(0x60 | PAL60)

#define CLOSED_CAPTION		0x7f
#define CLOSEDCAP_OFF		0x70
#define CLOSEDCAP_CC1		0x71
#define CLOSEDCAP_CC2		0x72
#define CLOSEDCAP_CC3		0x73
#define CLOSEDCAP_CC4		0x74
#define CLOSEDCAP_T1		0x75
#define CLOSEDCAP_T2		0x76
#define CLOSEDCAP_T3		0x77
#define CLOSEDCAP_T4		0x78

#define TV_GUIDE			0x8f
#define NO_VCHIP_LOCK		0
#define VCHIP_TV_Y			1
#define VCHIP_TV_Y7			2
#define VCHIP_TV_G			3
#define VCHIP_TV_PG			4
#define VCHIP_TV_14			5
#define VCHIP_TV_MA			6
#define PASSWORD			0x87
#define NEW_PASSWORD		0x88
#define CHK_PASSWORD		0x89
#define RET_CHK_PASSWORD	0x8a

#define MPAA_RATING			0x9f
#define NO_MPAA_RATING		0
#define MPAA_G				1
#define MPAA_PG				2
#define MPAA_PG_13			3
#define MPAA_R				4
#define MPAA_NC_17			5
#define MPAA_X				6

#define TVINSTALL			0xa0
#define SCAN_CHANNEL		0xa1
#define AIR_CABLE_TV		0xa3
#define STORE_CLEAR_TVCHN	0xa4
#define MTS					0xa5
#define EDIT_CHANNEL		0xa6
	#define		MTS_STEREO			0//0x10
	#define		MTS_MONO			1//0x11

#define FMID_AUDIO_VOLUME		0xb8
#define FMID_AUDIO_BALANCE		0xb9
#define FMID_AUDIO_BASS			0xba
#define FMID_AUDIO_TREBLE		0xbb
#define FMID_AUDIO_LOUDNESS		0xbc
#define FMID_AUDIO_EFFECT		0xbd
#define FMID_AUDIO_MUTE			0xbe

#define MANUAL_TUNE_CHANNEL	0xab
#define FINE_TUNE_CHANNEL	0xac
#define STORE_PALCHANNEL	0xad

//#define CLEAR_TVCHN			0xb0
//#define STORE_TVCHN			0xb1

#define FMID_WIDESCREEN			0xc0
#define WIDESCREEN_NORMAL	0 
#define WIDESCREEN_WIDE		1 
#define WIDESCREEN_PANO		2
#define WIDESCREEN_FULL		3 
#define WIDESCREEN_ZOOM		4 

#define SYSTEM_CLOCK		0xd1
#define SYSTEM_CLOCK_H		0xd2
#define WAKEUP_TIME			0xd3
#define WAKEUP_TIME_H		0xd4
#define WAKEUP_PR			0xd5
#define OFF_TIME			0xd6
#define OFF_TIME_H			0xd7
#define FMID_SLEEP_TIMER			0xd8

#define OSDLANGI			0xe8
#define OSDLANGII			OSDLANGI+1
#define OSDLANGIII			OSDLANGI+2
#define OSDLANGIV			OSDLANGI+3

#define FMID_TOHIGHMENU			0xf1
#define MOVECURSOR2FIRST	0xf2
#define RESET_ALL			0xf3
#define FMID_EXITMENU			0xf4

#define CHOICE1_VAL			0xfc
#define CHOICE_VAL			0xfd
#define BAR_VAL				0xfe
#define NUM_VAL				0xff



#endif //__OSDMENU_H__
