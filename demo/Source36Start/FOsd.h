/*
	FOSD.H
*/
#ifndef __FOSD_H__
#define __FOSD_H__

//---------------------------------
// manifast
//---------------------------------
#define FOSD_MAX_OSDRAM_SIZE	512
#define FOSD_MAX_OSDWIN		8
#define	FOSD_MAX_LUT			64
#define FOSD_MAX_FONTRAMSIZE	(10*1024)	//bytes

//---------------------------------
//
//---------------------------------
//typedef struct FONT_INFO_s {
//	BYTE name[4+2];
//	BYTE width,height;
//	WORD max;
//	WORD bpp2;		//start of 2BPP
//	WORD bpp3;		//start of 3BPP
//	WORD bpp4;		//start of 4BPP
//	BYTE lut2, lut3, lut4;	//(LUT offset*4)
//	BYTE dummy;
//} FONT_INFO_t;

typedef struct FONT_SPI_INFO_s {
	DWORD loc;
	WORD size;				//curr max 10*1024 = 2800h
	BYTE width,height;
	WORD bpp2;
	WORD bpp3;
	WORD bpp4;
	WORD max;

	BYTE *palette_bpp2;
	BYTE *palette_bpp3;
	BYTE *palette_bpp4;
} FONT_SPI_INFO_t;

//typedef struct FONT_WIN_INFO_s {
//	BYTE win;
//	BYTE osdram;			//osdram start offset
//} FONT_WIN_INFO_t;


//---------------------------------
// FontOsd Window Base table
//---------------------------------
extern code	WORD	FOSDWinBase[];

#define FOSD_WIN0		0
#define FOSD_WIN1		1
#define FOSD_WIN2		2
#define FOSD_WIN3		3
#define FOSD_WIN4		4
#define FOSD_WIN5		5
#define FOSD_WIN6		6
#define FOSD_WIN7		7

#define FOSD_MSG_WIN	0

#define FOSD_ASCII_BLANK	0x00

//FOSDWinBase[]+0x0B
#define WINDOW_ENABLE		0x01	//BUG
#define WINDOW_3D_ENABLE    0x04
#define WINDOW_3D_BOTTOM    0x08
#define WINDOW_3D_TOP	    0x00


//---------------------------------
// redefined registers
//---------------------------------
#define TW88_ENABLERAM		REG304	//0x04	// TW8804C
#define TW88_WINSADDRLO		0x0F	// starting address of OSD ram for Windows 1


//---------------------------------
//
//---------------------------------
//=============================================================================
// Palette(LookUp Table)
//=============================================================================
//=========================================================
//Palette map
//=========================================================
//=========================================================
//     normal          with SpiOSD MENU	 TEST
//	+<64>---------------+---------------+---------------+
//	|60	4BPP			| 4BPP		    | 4BPP		    | 
//	|56					|				|				|
//	|52	  original		|				|				|
//	|48	  16 palette	|				|				|
//	+-------------------|---------------+---------------+
//	|44	3BPP original	| 3BPP			| 3BPP			|
//	|40		 8 palette	|				|				|
//	+-------------------+---------------+---------------+
//	|36	2BPP original	| 2BPP			| 2BPP			|
// 	+-------------------+---------------+---------------+
//	|32	2BPP test		| 2BPP test		|32	2BPP test	|
// 	+--<<alpha limit>>--+---------------+---------------+
//	|28	3BPP alpha 	    | 3BPP alpha	| 3BPP alpha	|
//	|24	Green(def)    	| Green(def)	| Green(def)	|
// 	+-------------------+---------------+---------------+
//	|20 2BPP alpha B    | 3BPP alpha	| 2BPP alpha B	|
// 	+-------------------| BLUE			+---------------+
//	|16	2BPP alpha A	|				| 2BPP alpha A	|
// 	+-------------------+---------------+---------------+
//	|1BPP Area.			| 3BPP alpha	|           	|
//	|15	White			| RED			|SILVER			|
//	|14	Yellow			|				|DYELLOW		|
//	|13	Magenta			|				|DMAGENTA		|
//	|12	Red				|				|DRED			|
//	|11	Cyan			|				|DCyan			|
//	|10	Lime			|				|Green			|
//	|9	Blue			|				|DBlue			|
//	|8	Silver			| 				|Gray			|
//  |              		|---------------|---------------|			
//	|7	Gray			|White			|White          |
//	|6	DK Yellow		|Yellow			|Yellow      	|
//	|5	DK Magenta		|Magenta		|Magenta       	|			
//	|4	DK Red			|Red			|Red   			|	
//	|3	DK Cyan			|Cyan			|Cyan    		|	
//	|2	Green			|Lime			|Lime    		|	
//	|1	DK Blue(Alpha)	|Blue	        |Blue           |
//	|0	Black			|Black			|Black     		|
// 	+-------------------+---------------+---------------+
//Note:
//	SpiOSD MENU (Need 3 Bpp3 alpha)
//=========================================================


//default color index
#define FOSD_COLOR_IDX_BLACK		0
#define FOSD_COLOR_IDX_DBLUE		1
#define FOSD_COLOR_IDX_LIME			2	//??who is a default
#define FOSD_COLOR_IDX_CYAN			3	//??
#define FOSD_COLOR_IDX_RED			4	//??
#define FOSD_COLOR_IDX_MAGENTA		5	//??
#define FOSD_COLOR_IDX_YELLOW		6	//??
#define FOSD_COLOR_IDX_SILVER		7	//??
#define FOSD_COLOR_IDX_GRAY			8	//??
#define FOSD_COLOR_IDX_BLUE			9
#define FOSD_COLOR_IDX_DLIME		10	//??
#define FOSD_COLOR_IDX_DCYAN		11	//??
#define FOSD_COLOR_IDX_DRED			12	//??
#define FOSD_COLOR_IDX_DMAGENTA		13	//??
#define FOSD_COLOR_IDX_DYELLOW		14	//??
#define FOSD_COLOR_IDX_WHITE		15
#define FOSD_COLOR_IDX_BLANK		FOSD_COLOR_IDX_DBLUE

#define FOSD_COLOR_VALUE_BLACK		0x0000	/*0:Black*/		
#define FOSD_COLOR_VALUE_DBLUE		0x0010	/*1:DarkBlue*/	
#define FOSD_COLOR_VALUE_GREEN		0x0400	/*2:Green*/		
#define FOSD_COLOR_VALUE_DCYAN		0x0410	/*3:DarkCyan*/	
#define FOSD_COLOR_VALUE_DRED		0x8000	/*4:DarkRed*/	
#define FOSD_COLOR_VALUE_DMAGENTA	0x8010	/*5:DarkMagenta*/
#define FOSD_COLOR_VALUE_DYELLOW	0x8400	/*6:DarkYellow*/
#define FOSD_COLOR_VALUE_GRAY		0x8410	/*7:Gray*/		
#define FOSD_COLOR_VALUE_SILVER		0xC618	/*8:Silver*/	
#define FOSD_COLOR_VALUE_BLUE		0x001F	/*9:Blue*/		
#define FOSD_COLOR_VALUE_LIME		0x07E0	/*A:Lime*/		
#define FOSD_COLOR_VALUE_CYAN		0x07FF	/*B:Cyan*/		
#define FOSD_COLOR_VALUE_RED		0xF800	/*V:Red*/		
#define FOSD_COLOR_VALUE_MAGENTA	0xF81F	/*D:Magenta*/	
#define FOSD_COLOR_VALUE_YELLOW		0xFFE0	/*E:Yellow*/	
#define FOSD_COLOR_VALUE_WHITE		0xFFFF 	/*F:White*/		

#define FOSD_COLOR_VALUE_OLIVE		FOSD_COLOR_VALUE_DYELLOW
#define FOSD_COLOR_VALUE_ORANGE		0xFC00

#define FOSD_COLOR_USER0		FOSD_COLOR_VALUE_OLIVE
#define FOSD_COLOR_USER1		FOSD_COLOR_VALUE_ORANGE
#define FOSD_COLOR_USER(num)	FOSD_COLOR_USER##num

#define EXTENDED_COLOR		0x80
#define BG_COLOR_BLACK 		0x00	//For background color of window, border, and char. 
#define BG_COLOR_RED		0x40	
#define BG_COLOR_GREEN		0x20
#define BG_COLOR_YELLOW		0x60
#define BG_COLOR_BLUE		0x10
#define BG_COLOR_MAGENTA	0x50
#define BG_COLOR_CYAN		0x30
#define BG_COLOR_WHITE		0x70
#define BG_COLOR_TRANS		0x80	
#define BG_COLOR_MASK		0xf0	

#define FG_COLOR_BLACK 		0x00	//For color of char.
#define FG_COLOR_BLUE		0x01
#define FG_COLOR_GREEN		0x02
#define FG_COLOR_CYAN		0x03
#define FG_COLOR_RED		0x04	
#define FG_COLOR_MAGENTA	0x05
#define FG_COLOR_YELLOW		0x06
#define FG_COLOR_WHITE		0x07
#define FG_COLOR_MASK		0x0f	


//---------------------------------
//
//---------------------------------
#define FOSD_LUT_MAP_BPP2_ALPHA_A_START		16
#define FOSD_LUT_MAP_BPP2_ALPHA_B_START		20
#define FOSD_LUT_MAP_BPP2_START				36
#define FOSD_LUT_MAP_BPP3_START				40
#define FOSD_LUT_MAP_BPP4_START				48


#define FOSD_LUT_MAP_BPP3_ALPHA_R_START		8 	//
#define FOSD_LUT_MAP_BPP3_ALPHA_B_START		16
#define FOSD_LUT_MAP_BPP3_ALPHA_G_START		24

#define FOSD_LUT_MAP_BPP3_ALPHABLACK_START	40

extern code WORD FOsdHwDefPaletteBpp1[16];
extern code WORD FOsdSwDefPaletteBpp1[16];
extern code WORD FOsdDefPaletteBpp2[4]; 
extern code WORD FOsdDefPaletteBpp3[8];
extern code BYTE FOsdDefPaletteBpp3Alpha[8];
extern code WORD FOsdDefPaletteBpp4[16];

extern BYTE BPP3_alpha_lut_offset[8];

//=============================================================================
// FONT
//=============================================================================
//---------------------------------
// default font. 12x18
//---------------------------------
#define FONT_DEF_WIDTH			12
#define FONT_DEF_HEIGHT			18
#define	FONT_DEF_BYTES			27
#define FONT_PIXS_PER_XSTEP		8
#define FONT_LINES_PER_YSTEP	4

//---------------------------------
// ESCAPE CODE
//---------------------------------
//escape control characters
#define FONT_ROM	0xFF
#define FONT_RAM	0xFE	//from 0x0D0 to 0x12C
#define FONT_2BPP	0xFD	//from 0x100 to 0x11F
#define FONT_3BPP	0xFC	//from 0x120 to 0x15E
#define FONT_4BPP	0xFB	//from 0x15F to 0x17A..max at 12x18 font.
//depend on current downloaded font.
#define FONT_RAM_START	0x0A0
#define FONT_2BPP_START	0x100		//0x12E
#define FONT_3BPP_START	0x120		//0x130
#define FONT_4BPP_START	0x15F		//0x160


//=====================================
// FONT SYMBOL NAME
//=====================================
#define CH_BAR_X0X				0x00	// .   removed
#define CH_BAR_000				0x01	//...
#define CH_BAR_100				0x02	//|..
#define CH_BAR_1				0x03	//|
#define CH_BAR_111				0x04	//|||
#define CH_BAR_110				0x05	//||.
#define CH_SCR_INC_VERTI		0x06
#define CH_SCR_INC_HORIZ		0x08
#define CH_SCR_MOVE_VERTI		0x0A
#define CH_SCR_MOVE_HORIZ		0x0C
#define CH_CONTRAST				0x0E
#define CH_BRIGHTNESS			0x10
#define CH_SPK_VOL				0x12
#define CH_SPK_MUTE				0x14
#define CH_UNKNOWN_015			0x15
#define CH_TOUCH				0x16
#define CH_HORIZ_FREQ			0x17
#define CH_VERTI_FREQ			0x19
#define CH_ARROW_LEFT			0x1B
#define CH_ARROW_RIGHT			0x1C
#define CH_ARROW_UP				0x1D
#define CH_ARROW_DOWN			0x1E
#define CH_CURRENCY_YEN			0x1F
//----------------------------------
// ASCII AREA from 0x20~0c7F
//----------------------------------
#define CH_SYMBOL_07F			0x7F	//BUG
#define CH_IEXCL				0x80	//invert exclamation point
#define CH_CURRENCY_CENT		0x81
#define CH_CURRENCY_POUND		0x82
#define CH_IQUEST				0x83	//invert question mark
#define CH_UNKNOWN_084			0x84
#define CH_A_ACUTE				0x85
#define CH_E_ACUTE				0x86
#define CH_I_ACUTE				0x87
#define CH_O_ACUTE				0x88
#define CH_U_ACUTE				0x89
#define CH_C_CEDIL				0x8A
#define CH_N_TILDE				0x8B
#define CH_SCR_ASPECT			0x8C
#define CH_TV					0x8E
#define CH_INPUT				0x90
#define CH_OSD					0x92
#define CH_EXIT					0x94
#define CH_PROGRESS_LEFT		0x96
#define CH_PROGRESS_11			0x97
#define CH_PROGRESS_10			0x98
#define CH_PROGRESS_00			0x99
#define CH_PROGRESS_RIGHT		0x9A
#define CH_UNKNOWN_09B			0x9B
#define CH_UNKNOWN_09C			0x9C
#define CH_UNKNOWN_09D			0x9D
#define CH_UNKNOWN_09E			0x9E
#define CH_UNKNOWN_09F			0x9F
#define CH_SYMBOL_0AA			0xAA	//BUG
//==================================
// CH 0xA0~0xFE reload area(for Multi-Language)
//==================================
#define CH_KR_GAN				0x00
#define CH_KR_GUK				0x01
#define CH_KR_GI				0x02
#define CH_KR_GYUN				0x03
#define CH_KR_GAM				0x04
#define CH_KR_JIM				0x05
#define CH_KR_GYEONG			0x06
#define CH_KR_GO				0x07
#define CH_KR_NA				0x08
#define CH_KR_NOK				0x09
#define CH_KR_NONG				0x0A
#define CH_KR_NEOL				0x0B
#define CH_KR_NEUNG				0x0C
#define CH_KR_NEUN				0x0D
#define CH_KR_KKEO				0x0E
#define CH_KR_GEOM				0x0F
#define CH_KR_DA				0x10
#define CH_KR_DO				0x11
#define CH_KR_DEU				0x12
#define CH_KR_DONG				0x13
#define CH_KR_RYEOK				0x14
#define CH_KR_RYANG				0x15
#define CH_KR_REUL				0x16
#define CH_KR_MYEONG			0x17
#define CH_KR_MO				0x18
#define CH_KR_MYEON				0x19
#define CH_KR_MI				0x1A
#define CH_KR_MIL				0x1B
#define CH_KR_BEON				0x1C
#define CH_KR_BYEON				0x1D
#define CH_KR_BALK				0x1E
#define CH_KR_BI				0x1F
#define CH_KR_SI				0x20
#define CH_KR_SIN				0x21
#define CH_KR_SEOL				0x22
#define CH_KR_SU				0x23
#define CH_KR_SEON				0x24
#define CH_KR_SA				0x25
#define CH_KR_SANG				0x26
#define CH_KR_SE				0x27
#define CH_KR_SAEK				0x28
#define CH_KR_SANG2				0x29
#define CH_KR_ANH				0x2A
#define CH_KR_I					0x2B
#define CH_KR_IP				0x2C
#define CH_KR_AM				0x2D
#define CH_KR_IN				0x2E
#define CH_KR_YONG				0x2F
#define CH_KR_YO				0x30
#define CH_KR_EUM				0x31
#define CH_KR_WI				0x32
#define CH_KR_ON				0x33
#define CH_KR_EO				0x34
#define CH_KR_EON				0x35
#define CH_KR_EOK				0x36
#define CH_KR_WON				0x37
#define CH_KR_UM				0x38
#define CH_KR_JA				0x39
#define CH_KR_JEOK				0x3A
#define CH_KR_CHAE				0x3B
#define CH_KR_JEO				0x3C
#define CH_KR_JO				0x3D
#define CH_KR_JEONG				0x3E
#define CH_KR_JUNG				0x3F
#define CH_KR_JI				0x40
#define CH_KR_JIK				0x41
#define CH_KR_CHEONG			0x42
#define CH_KR_CHI				0x43
#define CH_KR_CHO				0x44
#define CH_KR_TAEK				0x45
#define CH_KR_PYEONG			0x46
#define CH_KR_HI				0x47
#define CH_KR_HAN				0x48
#define CH_KR_HA				0x49
#define CH_KR_HWAK				0x4A
#define CH_KR_HWA				0x4B
#define CH_KR_HO				0x4C
#define CH_KR_HYEONG			0x4D
#define CH_KR_HYANG				0x4E
#define CH_KR_SPACE				0x4F					
										

#define CH_2BPP_BAR				0x100
#define CH_3BPP_BAR				0x13E
#define CH_4BPP_BAR				0x15F


//=============================================================================
// Prototype
//=============================================================================
//---------------------------------
// OSDFont.C header
//---------------------------------

void FontOsdInit(void);

BYTE FOsdOnOff(BYTE fOnOff, BYTE vdelay);

void FOsdSetFontWidthHeight(BYTE width, BYTE height);
void FOsdSetDeValue(void);
void FOsdBlinkOnOff(BYTE fOn);
	#define FOSD_ACCESS_OSDRAM		0	//default
	#define FOSD_ACCESS_FONTRAM		1
void FOsdSetAccessMode(BYTE fType);


//---------------------------------
// FontOSD OsdRam
//---------------------------------
void FOsdRamSetFifo(BYTE fOn, BYTE vdelay);
	#define FOSD_OSDRAM_WRITE_NORMAL	0
	#define FOSD_OSDRAM_WRITE_AUTO		1	//Font Data or Attribute Address auto mode
	#define FOSD_OSDRAM_WRITE_DATA		3	//Font Data Auto Mode
void FOsdRamSetWriteMode(BYTE fMode);
BYTE FOsdRamGetWriteMode(void);
void FOsdRamSetAddress(WORD addr);
void FOsdRamSetAttr(BYTE attr);
void FOsdRamMemset(WORD addr, WORD dat, BYTE attr, BYTE size);
void FOsdRamMemsetAttr(WORD addr, BYTE color, BYTE cnt);
void FOsdRamMemsetData(WORD addr, WORD dat, BYTE len);
void FOsdRamClearAll(WORD dat, BYTE attr);
void FOsdRamSetAddrAttr(WORD OsdRamAddr, BYTE attr/*, BYTE mode*/);
void FOsdRamWriteByteStr(BYTE *str, BYTE len);
void WriteStringToAddr(WORD addr, BYTE *str, BYTE cnt);

//---------------------------------
// FontOsd Palette
//---------------------------------
void FOsdSetPaletteColor(BYTE start, WORD color, BYTE size, BYTE vdelay);
void FOsdSetPaletteColorArray(BYTE index, WORD *array, BYTE size, BYTE vdelay);
void FOsdSetDefPaletteColor(BYTE mode);
void FOsdInitBpp3AlphaTable(BYTE fCalculate);		//??local
void FOsdDefaultLUT( void );
void FOsdDumpPalette(BYTE winno);

//---------------------------------
// FontOsd Window
//---------------------------------
void FOsdWinEnable(BYTE winno, BYTE en);
void FOsdWinToggleEnable(BYTE winno);
void FOsdWinOffAll(void);
void FOsdWinMulticolor(BYTE winno, BYTE en);
void FOsdWinAlphaPixel(BYTE winno, BYTE color, BYTE alpha);
void FOsdWinAlphaPixelArray(BYTE winno, BYTE *array, BYTE size);
void FOsdWinSetBpp3Alpha(BYTE winno, BYTE base_lut);
void FOsdWinScreenXY(BYTE winno, WORD x, WORD y);
void FOsdWinScreenWH(BYTE winno, BYTE w, BYTE h);
void FOsdWinZoom(BYTE winno, BYTE zoomH, BYTE zoomV);
WORD FOsdWinGetX(BYTE winno);
void FOsdWinSetX(BYTE winno, WORD x);
WORD FOsdWinGetY(BYTE winno);
void FOsdWinSetY(BYTE winno, WORD y);
void FOsdWinSetW(BYTE winno, WORD w);
void FOsdWinSetOsdRamStartAddr(BYTE winno, WORD addr);
void FOsdWinSet3DControl(BYTE win, BYTE value);
void FOsdWinInit(BYTE winno);
void InitFOsdMenuWindow(BYTE *ptr);

//---------------------------------
// FontOsd Font
//---------------------------------
void FOsdDownloadFontByDMA(WORD dest_loc, DWORD src_loc, WORD size);
void FOsdDownloadFontBySerial(WORD dest_font_index, BYTE *dat, BYTE unit_size, BYTE unit_num);
void FOsdDownloadUDFontBySerial(void);
void InitFontRam(WORD dest_font_index,FONT_SPI_INFO_t *font, char *sName);
	#define FONT_NUM_DEF12X18		0
	#define FONT_NUM_CON16X28		1
	#define FONT_NUM_GRAYNUM16X28	2
	#define FONT_NUM_CONGRAY16X28	3
	#define FONT_NUM_LOGO			4
	#define FONT_NUM_PLUS_RAMFONT	4
void InitFontRamByNum( BYTE FontMode, WORD start );

//---------------------------------
// FontOsd Message
//---------------------------------
void FontOsdPrintIndexedString(BYTE winno, WORD x, WORD y, BYTE str_n);
	#define FOSD_STR0_GOOD			0
	#define FOSD_STR1_TW8835		1
	#define FOSD_STR2_NOSIGNAL		2
	#define FOSD_STR3_OUTRANGE		3
	#define FOSD_STR4_INIT_EEPROM	4
	#define FOSD_STR5_INPUTMAIN		5
void FOsdCopyMsgBuff2Osdram(BYTE fOn);
void FOsdIndexMsgPrint(BYTE index);

char *GetInputMainString(char *p_itoa_buff);
void FOsdSetInputMainString2FOsdMsgBuff(void);


#endif //..__FOSD_H__
