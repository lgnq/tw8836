/**
 * @file
 * eeprom.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	eeprom
*/
#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "Global.h"
#include "CPU.h"
#include "printf.h"

#include "I2C.h"
#include "spi.h"	//USE_SFLASH_EEPROM	

#include "Main.h"
#include "EEPROM.h"
#include "e3prom.h"
#include "InputCtrl.h"
#include "TouchKey.h"
#include "debug.h"
#include "BT656.h"

#include "DebugMsg.h"

#ifdef USE_SFLASH_EEPROM

/* wrap functions for E3P */
void E3P_Read2XMem(BYTE * dest_loc, DWORD src_loc, WORD size)
{
	SpiFlashDmaRead2XMem(dest_loc, src_loc, size);
}

void E3P_SectorErase(DWORD spiaddr)
{
	SPI_SectorErase(spiaddr);
}

void E3P_PageProgram(DWORD spiaddr, BYTE * xaddr, WORD cnt)
{
	SPI_PageProgram(spiaddr, (WORD)xaddr, cnt);
}

/* configure and init E3P */
void E3P_Configure(void)
{
/*
	E3P_GetVersion();
	E3P_SetStartAddr(E3P_SPI_SECTOR0);				//start from 0x080000
	E3P_SetSize(E3P_INDEX_PER_BLOCK, E3P_BLOCKS);	//512 bytes 64 * 8.
	E3P_SetBuffer(SPI_Buffer, SPI_BUFFER_SIZE);		//128
	
	if (E3P_Init())
		E3P_Repair();
*/		
}

#elif defined(NO_EEPROM)	//.. USE_SFLASH_EEPROM		//=========================================

CODE BYTE EE_dummy_DATA[0x1C0] = {
/* 000 */ 0x54, 0x38, 0x33, 0x35, 0x00, 0x09, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 010 */ 0x32, 0x32, 0x3E, 0x32, 0x14, 0x32, 0x32, 0x3E, 0x32, 0x14, 0x32, 0x32, 0x3E, 0x32, 0x14, 0x32,
/* 020 */ 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 
/* 030 */ 0x3E, 0x32, 0x14, 0x32, 0x32, 0x3E, 0x32, 0x14, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 040 */ 0x00, 0x00, 0x00, 0x32, 0x32, 0x32, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 050 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 060 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 070 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 080 */ 0x01, 0xDE, 0x0D, 0xFF, 0x0D, 0xFC, 0x01, 0xF0, 0x07, 0xED, 0x0D, 0x68, 0x0D, 0x5D, 0x02, 0x51, 
/* 090 */ 0x02, 0x5C, 0x07, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0A0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0B0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x32, 0xFF, 
/* 0C0 */ 0x32, 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0xFF, 0x32, 
/* 0D0 */ 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 
/* 0E0 */ 0x32, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 0F0 */ 0x32, 0xFF, 0x32, 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x32, 
/* 100 */ 0xFF, 0x32, 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 110 */ 0x00, 0x00, 0x00, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x32, 0xFF, 0x32, 
/* 120 */ 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 
/* 130 */ 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0xFF, 0x32, 0x32, 0x32, 
/* 140 */ 0x32, 0xFF, 0x32, 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 150 */ 0x00, 0x00, 0x00, 0x00, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 160 */ 0x00, 0x00, 0x00, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 170 */ 0x00, 0x00, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 
/* 180 */ 0x32, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 
/* 190 */ 0x32, 0xFF, 0x32, 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x32, 
/* 1A0 */ 0xFF, 0x32, 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x32, 0xFF, 
/* 1B0 */ 0x32, 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x32, 0xFF, 0x32, 0x32, 0x32, 0x00, 0x00, 0x00 
};
BYTE EE_Read(WORD index)
{
	if(index >= 0x1C0) return 0x00;
	return EE_dummy_DATA[index];	
}
void EE_Write(WORD index, BYTE dat)
{
	Printf("\nEE_Write(%x,%bx) SKIP",index,dat);	
} 
#endif

//=============================================================================
// 	EE[0]	T	T
// 	EE[1]	8	8
// 	EE[2]	3	3
// 	EE[3]	5	6
// 	EE[4]	F/W Rev.-major(Hex)	
//	EE[5]	F/W Rev.-minor(Hex)
//=============================================================================
/**
* get FW revision
*/
WORD GetFWRevEE(void)
{
	XDATA WORD rev;

	if (EE_Read(0) != 'T')
		return 0;		//TW
	if (EE_Read(1) != '8')
		return 0;		//88
	if (EE_Read(2) != '3')
		return 0;		//3
	if (EE_Read(3) != '6')
		return 0;		//6

	rev = (WORD)EE_Read(EEP_FWREV_MAJOR);

#ifdef DEBUG_EEP
	dPrintf("\nFW rev: %d", rev );
#endif

	rev <<= 8;
	rev |= (WORD)EE_Read(EEP_FWREV_MINOR);

#ifdef DEBUG_EEP
	dPrintf("\.%02d", (rev&0xff) );
#endif

	return rev;
}

/**
* save FW revision
*/
void SaveFWRevEE(WORD rev)
{
	EE_Write(0, 'T');	//TW
	EE_Write(1, '8');	//88
	EE_Write(2, '3');	//3
	EE_Write(3, '6');	//6

	EE_Write(EEP_FWREV_MAJOR, (BYTE)(rev>>8));
	EE_Write(EEP_FWREV_MINOR, (BYTE)rev );
//	dPrintf(" Save to new FW ver %bd.%02bd", (BYTE)(rev>>8), (BYTE)rev );
}

//=============================================================================
//	EE[6]	DebugLevel
//=============================================================================
/**
* get debug level
*/
BYTE GetDebugLevelEE(void)
{
	return EE_Read(EEP_DEBUGLEVEL);
}

/**
* save debug level
*/
void SaveDebugLevelEE(BYTE dl)
{
	EE_Write(EEP_DEBUGLEVEL, dl);
}

//=============================================================================
//	EE[7]	#define EEP_AUTODETECT			//BYTE	1	Flag for Input Auto Detect	-0:Auto, 1:NTSC,....
//	EE[8]	#define EEP_AUTODETECTTYTE		//BYTE	1	Type of Auto-detect(will be value of register 0x1d)
//=============================================================================


#ifdef WIDE_SCREEN
//=============================================================================
//	EE[9]	Wide Mode	
//=============================================================================
BYTE GetWideModeEE(void)
{
	return EE_Read(EEP_WIDEMODE);
}
void SaveWideModeEE(BYTE dl)
{
	EE_Write(EEP_WIDEMODE, dl);
}
#endif


#ifdef UNCALLED_SEGMENT_CODE
//=============================================================================
//	EE[0x0A]	Auto Recognition	EEP_AUTORECOGNITION		
//=============================================================================
BYTE GetPossibleAutoDetectStdEE(void)
{
	BYTE val;

	val = EE_Read(EEP_AUTORECOGNITION);

	#ifdef DEBUG_EEP
	dPrintf("\r\nGetPossibleAutoDetectStdEE:0x%02x", (WORD)val);
	#endif

	return val;
}
#endif

//=============================================================================
//	EE[0x0A]	Input for BT656Output	EEP_INPUTBT656SELECTION		
//=============================================================================
/**
* get & save InputBT656 mode
*/
BYTE GetInputBT656EE(void)
{
	return (EE_Read(EEP_INPUTBT656SELECTION));
}

void SaveInputBT656EE(BYTE mode)
{
	EE_Write(EEP_INPUTBT656SELECTION, mode);
}

//=============================================================================
// EE[0x0B]	 EEP_VIDEOMODE	 Video Mode
//=============================================================================

//=============================================================================
//	EE[0C]	EEP_OSDLANG
//=============================================================================
#ifdef SUPPORT_FOSD_MENU
/**
* get OSD Language vale
*/
BYTE GetOSDLangEE(void)
{
	BYTE val;

	val = EE_Read(EEP_OSDLANG);
	
	#ifdef DEBUG_EEP
	dPrintf("\r\nGetOSDLang:%02x", (WORD)val);
	#endif

	return val;
}
/**
* save OSD Language value
*/
void SaveOSDLangEE(BYTE val)
{
	EE_Write(EEP_OSDLANG, val);
}
#endif

//=============================================================================
// EE[0x0D]	EEP_OSDPOSITIONMODE		OSD Position Mode
//=============================================================================
#ifdef SUPPORT_FOSD_MENU
/**
* get OSDPosition Move value
*/
BYTE GetOSDPositionModeEE(void)
{
	BYTE val;
	val = EE_Read(EEP_OSDPOSITIONMODE);
	return val;
}
/**
* save OSDPosition Mode value
*/
void SaveOSDPositionModeEE(BYTE ndata)
{
	EE_Write(EEP_OSDPOSITIONMODE, ndata);
}
#endif

//=============================================================================
//	EE[0x0e]	EEP_CCD 	Closed Caption-	0: off, 1:on
//=============================================================================

//=============================================================================
//	EE[0x0f]	EEP_INPUTSELECTION 		InputSelection
//=============================================================================
/**
* get Input mode
*/
BYTE GetInputMainEE(void)
{
	return (EE_Read(EEP_INPUTSELECTION));
}

/**
* save Input mode
*/
void SaveInputMainEE( BYTE mode )
{
	EE_Write( EEP_INPUTSELECTION, mode );
}


//=============================================================================
// Video Color Effect
//=============================================================================
//   analog       digital	CVBS   SVIDEO	COMP	PC		DVI		HDMIPC	HDMITV	BT656
//-----------------------------------------------------------------------------------------
//      CONTRASE_Y			EE[10]	EE[15]	EE[1A]	EE[1F]	EE[24]	EE[29]	EE[2E]	EE[33]  
//      BRIGHTNESS_Y		EE[11]	EE[16]	EE[1B]	EE[20]	EE[25]	EE[2A]	EE[2F]	EE[34]
//SATURATION | CONTRAST_R	EE[12]	EE[17]	EE[1C]	EE[21]	EE[26]	EE[2B]	EE[30]	EE[35]
//HUE		 | CONTRAST_G	EE[13]	EE[18]	EE[1D]	EE[22]	EE[27]	EE[2C]	EE[31]	EE[36]
//SHARPNESS	 | CONTRAST_G	EE[14]	EE[19]	EE[1E]	EE[23]	EE[28]	EE[2D]	EE[32]	EE[37]
//--------------------------	
//
//	start	input
//	EE[10]	EEP_CVBS
//	EE[15]	EEP_SVIDEO
//	EE[1A]	EEP_YPBPR
//	EE[1F]	EEP_PC
//	EE[24]	EEP_DVI
//	EE[29]	EEP_HDMI_PC
//	EE[2E]	EEP_HDMI_TV
//	EE[33] 	EEP_BT656
//-----------------------------------------------------------------------------------------

//=============================================================================
//
//=============================================================================
/**
* get VideoData
*/
BYTE GetVideoDatafromEE(BYTE offset)
{
	XDATA	BYTE val;
	BYTE	index;

	index = EEP_IA_START+InputMain*IA_TOT_VIDEO;
	//BK121218 temp
	//if(InputMain==INPUT_LVDS) 
	//	index = EEP_IA_START+INPUT_DVI*IA_TOT_VIDEO;

	index += offset;
	val = EE_Read(index);
	return val;
}

/**
* save VideoData
*/
void SaveVideoDatatoEE(BYTE offset, BYTE ndata)
{
	BYTE	index;

	index = EEP_IA_START+InputMain*IA_TOT_VIDEO;
	//BK121218 temp
	//if(InputMain==INPUT_LVDS) 
	//	index = EEP_IA_START+INPUT_DVI*IA_TOT_VIDEO;
	index += offset;

//	dPrintf("\nWrite EE Video Data %02bx: %bd",index, ndata);
	EE_Write(index, ndata);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CODE BYTE Init_YCbCr_Video_Data[] = {
	50, 50, 62, 50, 20 }; // Contrast,Bright,Staturation_UV,Hue,Sharpness
CODE BYTE Init_RGB_Video_Data[] = {
	50, 50, 50, 50, 50 }; // Contrast_Y, BrightY. Contrast_R Contrast_G Contrast_B

/**
* reset VideoData value
*/
void ResetVideoValue(void)
{
	XDATA	BYTE i, CurInput;
	BYTE	*pTable;
	
	CurInput = InputMain;
	for (InputMain=INPUT_CVBS; InputMain < INPUT_TOTAL ; InputMain++ ) {
		if(InputMain == INPUT_PC || InputMain == INPUT_DVI || InputMain == INPUT_HDMIPC || InputMain == INPUT_LVDS)
			pTable = Init_RGB_Video_Data;
		else
			pTable = Init_YCbCr_Video_Data;

		for( i=0; i<IA_TOT_VIDEO; i++){
			SaveVideoDatatoEE(i, pTable[i]);	
		}
	}
	InputMain = CurInput;
 	//SetVideoMode(USER_VIDEOMODE);
}


//=============================================================================
// EE[0x38]	EEP_ASPECT_MODE		0:Normal,1:Zoom,2:full,3:Panorama		
//=============================================================================
/**
* get Aspect Mode value
*/
BYTE GetAspectModeEE(void)
{
	return ( EE_Read( EEP_ASPECT_MODE ) );	
}
/**
* save Aspect Mode value
*/
void SaveAspectModeEE(BYTE mode)
{
	EE_Write( EEP_ASPECT_MODE, mode );
}

//=============================================================================
// EE[0x39]	EEP_OSD_TRANSPARENCY
//=============================================================================

//=============================================================================
// EE[0x3A]	EEP_OSD_TIMEOUT
//=============================================================================

//=============================================================================
// EE[0x3B]	EEP_FLIP	//0:default,1:flip
//=============================================================================

//=============================================================================
// EE[0x3C]	EEP_BACKLIGHT
//=============================================================================

//=============================================================================
// EE[0x3D]	N/A
//=============================================================================

//=============================================================================
// EE[0x3E]	EEP_HDMI_MODE
//=============================================================================
/**
* get Hdmi Mode value
*/
BYTE GetHdmiModeEE(void)
{
	return ( EE_Read( EEP_HDMI_MODE ) );	
}
/**
* save Hdmi mode value
*/

void SaveHdmiModeEE(BYTE mode)
{
	EE_Write( EEP_HDMI_MODE, mode );
}

//=============================================================================
// EE[0x3F]	EEP_DVI_MODE
//=============================================================================
#if 0
/**
* Get DviMode value
*/
BYTE GetDviModeEE(void)
{
	return ( EE_Read( EEP_DVI_MODE ) );	
}
#endif

/**
* save Dvi mode value
*/
void SaveDviModeEE(BYTE mode)
{
	EE_Write( EEP_DVI_MODE, mode );
}


//=============================================================================
// Audio Part
//=============================================================================
//	EE[0x43]	EEP_AUDIOVOL			 AudioVol
//	EE[0x44]	EEP_AUDIOBALANCE		 AudioBalance
//	EE[0x45]	EEP_AUDIOBASS			 AudioBass
//	EE[0x46]	EEP_AUDIOTREBLE			 AudioTreble
//	EE[0x47]	EEP_AUDIOEFFECT			
//=============================================================================

/**
* reset Audio value
*/

void ResetAudioValue(void)
{
	EE_Write( EEP_AUDIOVOL, 50 );
	EE_Write( EEP_AUDIOBALANCE, 50 );
	EE_Write( EEP_AUDIOBASS, 50 );
	EE_Write( EEP_AUDIOTREBLE, 50 );
	EE_Write( EEP_AUDIOEFFECT, 0 );
}


//=============================================================================
//	EE[0x48]		EEP_BLOCKMOVIE	BlockedMovie:Blocked rating for Movie
//	EE[0x49]		EEP_BLOCKTV		BlockedTV:Blocked rating for TV	
//	EE[0x4A:0x4F]	EEP_FVSLD
//=============================================================================

//=============================================================================
//	EE[0x50:0x53]	EEP_VCHIPPASSWORD	//Defualt:3366
//=============================================================================

//=============================================================================
// Touch
//	EE[0x80]	EEP_TOUCH_CALIB_X
//	EE[0x80+10]	EEP_TOUCH_CALIB_Y
//	EE[0x80+20]	EEP_TOUCH_CALIB_END
//=============================================================================

//=============================================================================
//	EE[0x94]	EEP_ADC_GAIN_START
//=============================================================================

//=============================================================================
//	EE[0xA0]	EEP_PC_MODE_START
//=============================================================================

//=============================================================================
//	EE[]
//=============================================================================


//=============================================================================
//
//=============================================================================
/**
* set default value
*/
void ClearBasicEE(void)
{
	dPuts("\nClearBasicEE........");
	EE_Write(EEP_INPUTSELECTION, 0);	// AUTO
	EE_Write(EEP_INPUTBT656SELECTION, BT656_SRC_AUTO ); //default? DEC?

	EE_Write(EEP_CCD, 0);	// CC = OFF

	EE_Write(EEP_OSDLANG,0);	// Save default Language1

	ResetAudioValue();

	#ifdef SUPPORT_CCD_VCHIP
	//Vchip Password
	EE_Write(EEP_VCHIPPASSWORD,   3);
	EE_Write(EEP_VCHIPPASSWORD+1, 3);
	EE_Write(EEP_VCHIPPASSWORD+2, 6);
	EE_Write(EEP_VCHIPPASSWORD+3, 6);
	#endif

	EE_Write(EEP_DVI_MODE, 0);
	//EE_Write( EEP_HDMI_MODE, 0 );

	EE_Write(EEP_VIDEOMODE, 0);			
	ResetVideoValue();
	SaveAspectModeEE(2);				//0:Normal,1:Zoom,2:full,3:Panorama
	EE_Write(EEP_OSD_TRANSPARENCY, 0);
	EE_Write(EEP_OSD_TIMEOUT, 0);		//default. 30->0 sec.
	EE_Write(EEP_FLIP, 0);			
	EE_Write(EEP_BACKLIGHT, 0);
	SaveHdmiModeEE(0);
	SaveDviModeEE(0);

	#ifdef SUPPORT_CCD_VCHIP
	ResetCCEDS();
	#endif //SUPPORT_CCD_VCHIP

	#ifdef SUPPORT_TV
	EE_Write(EEP_TVInputSel, 0);  //AIRTV
	TVChannel = DummyChannel = 2; 

	#ifdef NTSC_TV
 	EE_Write(CHN_CURAIR, TVChannel);
	TVInputSel = AIRTV; SetFineTuneEE(TVChannel, 0); //E2(55.25 Mhz)
	EE_Write(CHN_CURCABLE, TVChannel);
	TVInputSel = CABLETV;	SetFineTuneEE(TVChannel, 0); //E2(55.25 Mhz)
	#else
	EE_Write(PR_CUR, TVChannel);
	SetTVFreqEE(TVChannel, 0x05e2); //E3(55.25 Mhz)
	#endif
	#endif // SUPPORT_TV

	#ifdef WIDE_SCREEN
	Set4WideScreen(WIDESCREEN_WIDE);
	#endif

#ifdef SUPPORT_TOUCH
	//save default CalibDataX[] and CalibDataY[]
	SaveCalibDataToEE(0);
#endif

//	EE_PrintCurrInfo();

#ifdef SUPPORT_PC
	//init pcdata on EE
	InitPCDataEE();
#endif
}

/**
* init EEPROM
* 
* @see ClearBasicEE
* @see SaveFWRevEE
*/
void InitializeEE(void)
{
	dPuts("\nEE initialize........");
	ClearBasicEE();
	SaveFWRevEE(FWVER);
}

/**
* check EEPROM
* 
* If invalid FW revision, init EEPROM
* @return
*	-0: success. Start with saved input...
*	-1: false.	need a repair routine
*/
BYTE CheckEEPROM(void)
{
#ifdef USE_SFLASH_EEPROM
	BYTE ret;

	// Link EEPROM(SFlash)
	ret = EE_FindCurrInfo();	//<--first EE function.
	if (ret)
		E3P_Repair();
#endif

	//eeprom
	if (GetFWRevEE() == FWVER)
		return 0;

	DbgMsg_EEP_Corruptted();

	//we have a trouble.
	return 1;
}

//===================================================================
//===================================================================
// PC EEPROM
//===================================================================
//===================================================================
#if defined( SUPPORT_PC ) /*|| defined( SUPPORT_COMPONENT ) */ // || defined (SUPPORT_DVI)
//	0x80	----------- PC Color Temp Data --------------------------------
//			0	PC Color Temp Mode
//			1	R G B for Mode 1
//			4	R G B for Mode 2
//			7   R G B for Mode 3
//			A	R G B for User Mode
//
//	0x90	----------- PC Input ------------------------------------------
//
//			0	ADC Coarse
//			1	ADC Phase
//			2	Vertical Active Start
//			3	Horizontal Active Start
//


extern CONST struct _PCMODEDATA PCMDATA[];

extern  IDATA BYTE  PcMode, PcModeLast;
extern	IDATA WORD  IVF;
extern	IDATA DWORD	IHF;

#define WriteEEP(index,dat)	EE_Write(index,dat)
#define ReadEEP(index)		EE_Read(index)

//-------------------------
//desc:
//@param:
//	mode - PC Mode
//	offset - EE_PCDATA_xxx offset
//			0:PClock,1:Phase,2:VActive,3:VBackporch,4:HActive
//return:
/**
* get PC Data
*/
BYTE GetPCDataEE(BYTE mode, BYTE offset)
{
	WORD index;
	index = EE_PCDATA + (WORD)mode*LEN_PCDATA + offset;
	return ReadEEP(index);
}
/**
* save PC Data
*/
void SavePCDataEE(BYTE mode, BYTE offset, BYTE val)
{
	WORD index;
	index = EE_PCDATA + mode*LEN_PCDATA + offset;
	WriteEEP(index,val);
}
//-------------------------
//desc:get Pixel Clock
//@param
//	mode - PC Mode
//return
//	value -128..127
/**
*  get Pixel Clock value
*/
char GetPixelClkEE(BYTE mode)
{
	BYTE val;
	val=GetPCDataEE(mode,EE_PCDATA_CLOCK);
	return (char)val;
}
/**
* save Pixel clock value
*/
void SavePixelClkEE(BYTE mode, char val)
{
	SavePCDataEE(mode,EE_PCDATA_CLOCK,(BYTE)val);
}

//-------------------------
//desc:get Pixel Clock
//@param
//	mode - pc mode.
//return
//	0..31
/**
* get Phase value
*/
BYTE GetPhaseEE(BYTE mode)
{
	BYTE val;
	val=GetPCDataEE(mode,EE_PCDATA_PHASE);
	return (char)val;
}
/**
* save Phase value
*/
void SavePhaseEE(BYTE mode, BYTE val)
{
	SavePCDataEE(mode,EE_PCDATA_PHASE,(BYTE)val);
}



#ifdef UNCALLED_SEGMENT
WORD GetVActiveStartEE(BYTE mode)
{... }
void SaveVActiveEE(BYTE mode)
{...}
#endif

//-------------------------
//desc:
//@param:
//	mode - PC Mode
//return:
#if 0
char GetVActiveEE(BYTE mode)
{
	WORD index;
	index = EE_PCDATA + mode*LEN_PCDATA + EE_PCDATA_VACTIVE;
	return (char)ReadEEP(index);
}

void SaveVActiveEE(BYTE mode, char value)
{
	WORD index;
	index = EE_PCDATA + mode*LEN_PCDATA + EE_PCDATA_VACTIVE;
	WriteEEP(index, (BYTE)value) ;
}
#endif
/**
* get Vertical BackPorch value
*/
char GetVBackPorchEE(BYTE mode)
{
	WORD index;
	index = EE_PCDATA + mode*LEN_PCDATA + EE_PCDATA_VBACKPORCH;
	return (char)ReadEEP(index);
}

/**
* save Vertical BackPorch value
*/
void SaveVBackPorchEE(BYTE mode, char value)
{
	WORD index;
	index = EE_PCDATA + mode*LEN_PCDATA + EE_PCDATA_VBACKPORCH;
	WriteEEP(index, (BYTE)value) ;
}


//-------------------------
//desc:
//@param:
//	mode - PC Mode
//return:
/**
* get Horizontal Active value
*/
char GetHActiveEE(BYTE mode)
{
	WORD index;
	index = EE_PCDATA + mode*LEN_PCDATA + EE_PCDATA_HACTIVE;
	return (char)ReadEEP(index);
}
//-------------------------
//desc:
//@param:
//	mode - PC Mode
//	value - -128..127
//return:
/**
* save Horizontal Active value
*/
void SaveHActiveEE(BYTE mode, char value)
{
	WORD index;
	index = EE_PCDATA + mode*LEN_PCDATA + EE_PCDATA_HACTIVE;
	WriteEEP(index, (BYTE)value) ;
}

//-----------------------------------------------------------------------------
//			Get PC Function Data from EEPROM
//-----------------------------------------------------------------------------
#ifdef UNCALLED_SEGMENT
void GetPCDataEE(BYTE mode)
{...}
#endif
//=============================================================================
//			Save PC Function Data to EEPROM
//=============================================================================
#ifdef UNCALLED_SEGMENT
void SavePCDataEE(BYTE mode)
{...}
#endif

//=============================================================================
#ifdef UNCALLED_SEGMENT
void SaveDefaultPCDataEE(void)
{...}
#endif

#ifdef UNCALLED_SEGMENT
void SaveDefaultPCDataAllEE(void)
{...}
#endif



//=============================================================================



#ifdef EXTERNAL_ADC

#ifdef SUPPORT_SVDO
void SaveFVSDelay(BYTE val)
{
	WriteEEP(EE_VDELAY, val);
}

void SaveTRSP_Start(BYTE val)
{
	WriteEEP(EE_TRSP_START, val);
}

BYTE GetFVSDelayEE(void)
{
	return (ReadEEP(EE_VDELAY));
}

BYTE GetTRSP_StartEE(void)
{
	return (ReadEEP(EE_TRSP_START));
}

#endif
//-----------------------------------------------------------------------------
//		Save ADC Gain & Offset	mod=0 default value, mod=1 current value
//-----------------------------------------------------------------------------
/**
* save ADC Gain Offset
*/
void SaveADCGainOffsetEE(BYTE mod)
{
	BYTE i, dat;
	WORD index;

	index = EE_ADC_GO;

	for(i=0; i<6; i++) {
		if( mod==0 ) dat = 0x80;
		else		 dat = ReadADC_EX(i+0x08);
		WriteEEP(index+i, dat);
	}
}

/**
* get ADC Gain Offset
*/
void GetADCGainOffsetEE(void)
{
	BYTE i, dat;
	WORD index;

	index = EE_ADC_GO;

	for(i=0; i<6; i++) {
		dat = ReadEEP(index+i);
		WriteADC_EX(i+0x08, dat);
	}
}

#ifdef SUPPORT_DTV
//-----------------------------------------------------------------------------
//		Save ADC Gain & Offset for DTV	mod=0 default value, mod=1 current value
//-----------------------------------------------------------------------------
/**
* save ADC Gain Offset
*/
void SaveADCGainOffsetForDTVEE(BYTE mod)
{
	BYTE i, dat;

	for(i=0; i<6; i++) {
		if( mod==0 ) dat = 0x80;
		else		 dat = ReadADC_EX(0x08+i);
		WriteEEP(EE_ADC_GO_DTV + i, dat);
	}
}

/**
* get ADC gain offset
*/
void GetADCGainOffsetForDTVEE(void)
{
	BYTE i, dat;

	for(i=0; i<6; i++) {
		dat = ReadEEP(EE_ADC_GO_DTV + i);
		WriteADC_EX(0x08+i, dat);
	}
}
#endif // SUPPORT_DTV

#endif //EXTERNAL_ADC
#endif // SUPPORT_PC

//-------------------------
//desc:
//@param:
//return:
#ifdef SUPPORT_PC
void InitPCDataEE(void)
{
	BYTE mode; //,offset;
	WORD index;

	for(mode=0; mode < (EE_EOF_PCDATA); mode++) {
		if(PCMDATA[mode].support) {
			SavePCDataEE(mode,EE_PCDATA_CLOCK,50);		//0..100
			SavePCDataEE(mode,EE_PCDATA_PHASE,0xFF);	//0x00..0x1F. 0xFF measn unused.
			SavePCDataEE(mode,EE_PCDATA_VACTIVE,50);	//0..100
			SavePCDataEE(mode,EE_PCDATA_VBACKPORCH,50);	//0..100
			SavePCDataEE(mode,EE_PCDATA_HACTIVE,50);	//0..100
		}
	}
	index = EE_PCDATA + mode*LEN_PCDATA;
#ifdef DEBUG_EEP
	dPrintf("\nInitPCDataEE start:%lx end+1:%x", EE_PCDATA, index);
#endif
}
#endif