#ifndef __EEP_H__
#define __EEP_H__

//========== EEPROM Emulation ==========
//
// size = SPI_SECTOR_SIZE * EE_SPI_BANKS * EE_BLOCKS
//	    = 4K              * 4             * 8
//		= 128K = 0x20000.
#ifdef USE_SFLASH_EEPROM
#include "e3prom.h"

#define SPI_SECTOR_SIZE		0x1000		// 4K
#define E3P_SPI_SECTOR0		0x080000L	// EE Start Address
#define E3P_SPI_BANKS		4
#define E3P_INDEX_PER_BLOCK	64			// 32 or 64
#define E3P_BLOCKS			8
#define E3P_MAX_INDEX		(E3P_INDEX_PER_BLOCK*E3P_BLOCKS)

#define EE_BUF_SIZE			SPI_BUFFER_SIZE		//64 or 128

#define EE_Read(index)			E3P_Read(index)
#define EE_Write(index, value) 	E3P_Write(index,value)

void E3P_Configure(void);

#elif defined(NO_EEPROM)  //..USE_SFLASH_EEPROM
BYTE EE_Read                (WORD index);
void EE_Write               (WORD index, BYTE dat);
#else
#define EE_Read(index)		ReadI2CByte(ADDRESS_EEPROM + ((index >> 8) << 1),(BYTE)index)
#define EE_Write(index,dat)	WriteI2CByte(ADDRESS_EEPROM + ((index >> 8) << 1),(BYTE)index,dat)
#endif //..USE_SFLASH_EEPROM

////eeprom.h

//=========================================================================
//	EEPROM structure
//	0x00	BYTE	4	'T835'	 means TW8835
#define	EEP_FWREV_MAJOR			0x04	//BYTE	1	F/W Rev.-major(Hex)	
#define	EEP_FWREV_MINOR			0x05	//BYTE	1	F/W Rev.-minor(Hex)
#define	EEP_DEBUGLEVEL			0x06	//BYTE	1	DebugLevel
//
#define EEP_AUTODETECT			0x07	//BYTE	1	Flag for Input Auto Detect	-0:Auto, 1:NTSC,....
#define EEP_AUTODETECTTYTE		0x08	//BYTE	1	Type of Auto-detect(will be value of register 0x1d)
//			---------------------------------------------------------------
#define EEP_WIDEMODE			0x09	//BYTE	1   Wide Mode

#define EEP_INPUTBT656SELECTION	0x0A	//BYTE  1   Input for BT656 module

#define EEP_VIDEOMODE			0x0b	//BYTE  1   Video Mode
#define EEP_OSDLANG				0x0c	//BYTE  1   OSDLang						// 0

#define EEP_OSDPOSITIONMODE 	0x0d	//BYTE	1	OSD Position Mode
#define EEP_CCD					0x0e	//BYTE	1	Closed Caption-	0: off, 1:on

#define EEP_INPUTSELECTION		0x0f	//BYTE	1	InputSelection


//----------------------------------
//0x12::0x2F
// Video Image
/*
--CVBS(YUV)
	CONTRAST
	BRIGHT
	SATURATE
	HUE
	SHARPNESS
--SVIDEO(YUV)
	CONTRAST
	BRIGHT
	SATURATE
	HUE
	SHARPNESS
--COMPONENT(YUV)
	CONTRAST
	BRIGHT
	SATURATE
	HUE
	SHARPNESS
--PC(RGB)
	contrastY
	brightY
	contrast RGB
--DVI(RGB)
	contrastY
	brightY
	contrast RGB
--HDMI PC
	contrastY
	brightY
	contrast RGB
--HDMI TV (YUV)
	CONTRAST
	BRIGHT
	SATURATE
	HUE
	SHARPNESS
--ExtCVBS (YUV)
	CONTRAST
	BRIGHT
	SATURATE
	HUE
	SHARPNESS
*/
//----------------------------------
#define EEP_IA_START			0x10


//backend image adjustment
//for analog (YUV)
#define EEP_IA_CONTRASE_Y		0
#define EEP_IA_BRIGHTNESS_Y		1
#define EEP_IA_SATURATION		2	//for contrast_Cb,contrast_Cr
#define EEP_IA_HUE				3
#define EEP_IA_SHARPNESS		4
//for digital (RGB)
//#define EEP_IA_CONTRASE_Y		0
//#define EEP_IA_BRIGHTNESS_Y	1
#define EEP_IA_CONTRAST_R		2
#define EEP_IA_CONTRAST_G		3
#define EEP_IA_CONTRAST_B		4

#define IA_TOT_VIDEO			5

#define EEP_CVBS				EEP_IA_START					//0x10						
#define EEP_SVIDEO				(EEP_IA_START+IA_TOT_VIDEO)		//0x15
#define EEP_YPBPR				(EEP_IA_START+IA_TOT_VIDEO*2)	//0x1A
#define EEP_PC					(EEP_IA_START+IA_TOT_VIDEO*3)	//0x1F
#define EEP_DVI					(EEP_IA_START+IA_TOT_VIDEO*4)	//0x24
#define EEP_HDMI_PC				(EEP_IA_START+IA_TOT_VIDEO*5)	//0x29
#define EEP_HDMI_TV				(EEP_IA_START+IA_TOT_VIDEO*6)	//0x2E
#define EEP_BT656				(EEP_IA_START+IA_TOT_VIDEO*7)	//0x33
#define EEP_LVDS				(EEP_IA_START+IA_TOT_VIDEO*8)	//0x38
#define EEP_INPUT_IMAGE_END		(EEP_IA_START+IA_TOT_VIDEO*8)	//0x3D 

#define EEP_ASPECT_MODE			0x40	//0:Normal,1:Zoom,2:full,3:Panorama
#define EEP_OSD_TRANSPARENCY	0x41
#define EEP_OSD_TIMEOUT			0x42
#define EEP_FLIP				0x43	//0:default,1:flip
#define EEP_BACKLIGHT			0x44
//0x3D
#define EEP_HDMI_MODE			0x45
#define EEP_DVI_MODE			0x46

#define EEP_IMAGE_END			0x50



/*
--PC
	position(X,Y)
--DVI-24bit
	position (X,Y)
	pclk
	phase
--DVI-12bit
	position (X,Y)
	pclk
	phase
--HDMI PC
	position (X,Y)
	pclk
	phase
*/
#if 0
#define EEP_PC_POSITION_X		6
#define EEP_PC_POSITION_Y		7

#define EEP_DTV_PCLOCK			8
#define EEP_DTV_PHASE			9
#define TOT_DTV_VIDEO			10	//contrast(R/G/B),bright(R/G/B), position(X/Y), PCLOCK & PHASE
#endif



//	    	---------------------------------------------------------------
#define	EEP_PIPMODE				EEP_IMAGE_END+0		//0x50
#define	EEP_PIP1SELECTION		EEP_IMAGE_END+1		//0x51
#define	EEP_PIP2SELECTION		EEP_IMAGE_END+2		//0x52
//	    	---------------------------------------------------------------
#define EEP_AUDIOVOL			EEP_IMAGE_END+3		//0x53	//BYTE	1   AudioVol
#define EEP_AUDIOBALANCE		EEP_IMAGE_END+4		//0x54	//BYTE	1   AudioBalance
#define EEP_AUDIOBASS			EEP_IMAGE_END+5		//0x55    //BYTE  1   AudioBass
#define EEP_AUDIOTREBLE			EEP_IMAGE_END+6		//0x56	//BYTE  1   AudioTreble
#define EEP_AUDIOEFFECT			EEP_IMAGE_END+7		//0x57	//BYTE  1
//
//	    	---------------------------------------------------------------

#define EEP_BLOCKMOVIE 			EEP_IMAGE_END+8		//0x58	BYTE	1	BlockedMovie:Blocked rating for Movie	
#define EEP_BLOCKTV				EEP_IMAGE_END+9		//0x59 	BYTE	1	BlockedTV:Blocked rating for TV			
#define EEP_FVSLD				EEP_IMAGE_END+10	//0x5A	BYTE  6
//						                7    6       4    3    2    1    0
//  FVSLD Level                        ALL   FV(V)   S    L    D    
//  0x5A    BYTE    1   TV-Y            X  
//  0x5B	BYTE    1   TV-Y7           X    X 
//  0x5C	BYTE    1   TV-G            X 
//  0x5D	BYTE    1   TV-PG           X       X    X    X    X
//  0x5E	BYTE    1   TV-14           X       X    X    X    X 
//  0x5F	BYTE    1   TV-MA           X       X    X    X
//
#define	EEP_VCHIPPASSWORD		EEP_IMAGE_END+0x10 	//0x60	 BYTE	4   OSDPassword					//Defualt:3366
//			---------------------------------------------------------------
//
//
//	0x70	WORD	2   PanelXRes
//	0x72	WORD	2	PanelYRes
//	0x74	BYTE	1	PanelHsyncMinPulseWidth
//	0x75	BYTE	1	PanelVsyncMinPulseWidth
//	0x76	WORD	2	PanelHminBackPorch
//	0x78	BYTE	1	PanelHsyncPolarity
//	0x79	BYTE	1	PanelVsyncPolarity
//	0x7a	WORD	2	PanelDotClock
//	0x7c	BYTE	1	PanelPixsPerClock
//	0x7d	BYTE	1	PanelDEonly
//			---------------------------------------------------------------
//
//	0x80	PC Data

#define EEP_TOUCH_CALIB_X		0x80
#define EEP_TOUCH_CALIB_Y		0x80+10
#define EEP_TOUCH_CALIB_END		0x80+20	  //..0x94



#define EEP_ADC_GAIN_START		0x94	//0x94..
#define EEP_PC_MODE_START		0xA0
										//50*5
										//
										//end at 0x19A..


//
//			---------------------------------------------------------------
//
//	0x300	TV Data
//
//	--- NTSC_TV -------------------------------------------------------------
//	CNT_SAVEDAIR			BYTE	1   Total count of saved Air TV Channel.
//	IDX_CURAIR				BYTE	1   Index of Current Air TV Channel
//	CHN_CURAIR				BYTE    1   Current Air TV Channel.
//
//	FIRSTSAVED_AIRCHN		BYTE    1	First saved Air TV channel no	(maximum 100)
//	....
//
//	CNT_SAVEDCABLE			BYTE	1   Total count of saved Cable TV Channel.
//	IDX_CURCABLE			BYTE	1   Index of Current Cable TV Channel
//	CHN_CURCABLE			BYTE    1   Current Cable TV Channel.
//
//	FIRSTSAVED_CABLECHN		BYTE	1	First saved Cable TV channel no	(maximum 100)
//
//	--- PAL_TV --------------------------------------------------------------
//	PR_CUR					BYTE	1   Current PR no.
//	FIRST_SAVEDPR			DWORD	4   Freq of PR0.	(TOTAL_PR)
//	FIRST_SAVEDPR+4			DWORD   4   Freq of PR1.
//	.....
//

//removed
//
//#define CCCOLOR		0x52
//#define VOLZOOM		0x53
//
//#define PANELINFO	0x61

/*
// PAL_TV
#define  PR_CUR					0x301	
#define  FIRST_SAVEDPR			0x308	
 #define TVFREQ_HIGH 0 
 #define TVFREQ_LOW  1
#ifdef PAL_TV
 #define TVFINETUNE  2
		// NOT Finetune: 0 , Range: -32 ~ +32
 #define TVPRSYSTEM  3	
		// bit 7: Add:1 Ereased :0
		// 
 #define TVCHNAME    4

 #define BYTEPERCHANNEL 9 
#endif
// NTSC_TV
#define  EEP_TVInputSel			0x302
#define  CHN_CURAIR				0x303	
#define  FIRSTSAVED_AIRCHN		0x308	
#define  CHN_CURCABLE			0x403	
#define  FIRSTSAVED_CABLECHN	0x408	
#ifdef NTSC_TV
 #define TVFINETUNE  0
		// NOT Finetune: 0 , Range: -32 ~ +32
 #define TVPRSYSTEM  1	
		// bit 7: Add:1 Ereased :0
		// 
 #define BYTEPERCHANNEL 2
#endif

*/





WORD GetFWRevEE(void);
void SaveFWRevEE(WORD);

BYTE GetDebugLevelEE(void);
void SaveDebugLevelEE(BYTE);

//BYTE GetWideModeEE(void);
//void SaveWideModeEE(BYTE dl);

//BYTE GetPossibleAutoDetectStdEE(void);

BYTE GetOSDLangEE(void);
void SaveOSDLangEE(BYTE val);

BYTE GetOSDPositionModeEE(void);
void SaveOSDPositionModeEE(BYTE ndata);

BYTE GetInputMainEE( void );
void SaveInputMainEE( BYTE mode );
BYTE GetInputBT656EE( void );
void SaveInputBT656EE( BYTE mode );



BYTE GetVideoDatafromEE(BYTE offset);
void SaveVideoDatatoEE(BYTE offset, BYTE ndata);
void ResetVideoValue(void);

BYTE GetAspectModeEE(void);
void SaveAspectModeEE(BYTE mode);

BYTE GetHdmiModeEE(void);
void SaveHdmiModeEE(BYTE mode);
BYTE GetDviModeEE(void);
void SaveDviModeEE(BYTE mode);

void ResetAudioValue(void);

//----------------------------
//
//----------------------------
void CheckSystemVersion(void);
BYTE CheckEEPROM(void);

void InitializeEE( void );
void ClearBasicEE(void);

//void SavePIP1EE( BYTE mode );
//void SavePIP2EE( BYTE mode );


//=============================
// PC EEPROM
//=============================
#define EE_ADC_GO		EEP_ADC_GAIN_START		// ADC Gain Offset for PC
#define EE_ADC_GO_DTV	EEP_ADC_GAIN_START+6	// ADC Gain Offset for DTV

#define	EE_PCDATA		EEP_PC_MODE_START	// StartAddress of EEPROM for PCDATA

#if defined( SUPPORT_PC ) || defined (SUPPORT_DVI)
#define EE_YUVDATA_START	50
#else
#define EE_YUVDATA_START	0
#endif
#define EE_EOF_PCDATA		EE_YUVDATA_START+7		

#define EE_PCDATA_CLOCK			0
#define EE_PCDATA_PHASE			1
#define EE_PCDATA_VACTIVE		2
#define EE_PCDATA_VBACKPORCH	3
#define EE_PCDATA_HACTIVE		4
#define	LEN_PCDATA				5					// Length of PCDATA

//void SavePixelClkEE(BYTE mode);
//void SavePhaseEE(BYTE mode);
//void SaveVBackPorchEE(BYTE mode);

//WORD GetVActiveStartEE(BYTE mode);
//WORD GetHActiveStartEE(BYTE mode);
char GetHActiveEE(BYTE mode);
char GetVActiveEE(BYTE mode);
void SaveHActiveEE(BYTE mode, char value);
void SaveVActiveEE(BYTE mode, char value);
char GetVBackPorchEE(BYTE mode);
void SaveVBackPorchEE(BYTE mode, char value);


//BYTE GetPixelClkEE(BYTE fRGB);
//void MY_SavePixelClkEE(BYTE fRGB, BYTE value);
//BYTE GetPhaseEE(BYTE fRGB);
//void MY_SavePhaseEE(BYTE fRGB, BYTE value);
char GetPixelClkEE(BYTE mode);
void SavePixelClkEE(BYTE mode, char val);
BYTE GetPhaseEE(BYTE mode);
void SavePhaseEE(BYTE mode, BYTE val);

void InitPCDataEE(void);


void SaveADCGainOffsetEE(BYTE mod);
void GetADCGainOffsetEE(void);




#endif	// __ETC_EEP__
