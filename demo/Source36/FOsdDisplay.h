#ifndef __FOSD_DISPINFO__
#define __FOSD_DISPINFO__
//==================================================================
//				Display Information
//==================================================================
#if (defined WQVGA) && (defined ANALOG_7INCH)
#define		INPUTINFO_ZOOM		WINZOOMx1
#define		LOGO_ZOOM			WINZOOMx3

#else // (defined WQVGA) && (defined SHARP_LQ043T3DX02)
#define		INPUTINFO_ZOOM		WINZOOMx2
#define		LOGO_ZOOM			WINZOOMx2
#endif
//#define		VOL_ZOOM			WINZOOMx4
#define		MUTE_ZOOM			WINZOOMx3
#define		TVCHN_ZOOM			WINZOOMx4
#define		RATINGINFO_ZOOM		WINZOOMx3
#define     SCANTV_ZOOM			WINZOOMx2		//ljy012304
#define		PCINFO_ZOOM			WINZOOMx2
#define     BAR_ZOOM			WINZOOMx2


#define FOSD_MUTE				1	
#define FOSD_MENU				2	
#define FOSD_TVVOL				4	
#define FOSD_CC					8	
#define FOSD_INPUTINFO			0x10
#define FOSD_PCINFO				0x20
#define FOSD_RATINGINFO			0x40
#define FOSD_TVCHN				0x80
extern DATA 	BYTE  DisplayedOSD;
extern		bit   	DisplayInputHold;



void FOsdDisplayInput(void);
BYTE FOsdClearInput(void);

void DisplayLogo(void);
void ClearLogo(void);

void DisplayPCInfo(BYTE CODE *ptr);
void ClearPCInfo(void);
void DisplayAutoAdjust(void);
void ClearAutoAdjust(void);

void SetDisplayedOSD( BYTE newd );

void ClearOSDInfo(void);

void DisplayVol(void);

void DisplayMuteInfo(void);
void ClearMuteInfo(void);
//BYTE CheckAndClearOSD(void);

//==================================================================
//						TV Info
//==================================================================

BYTE ClearTVInfo(void);

#endif	// __FOSD_DISPINFO__
