#ifndef  _MEASURE_H_
#define _MEASURE_H_

#define EE_DOS				0

#define EE_SP1				1
#define EE_SP2				2
#define EE_SP3				3

#define EE_VGA_60			4
#define EE_VGA_66			5
#define EE_VGA_70			6
#define EE_VGA_72			7
#define EE_VGA_75			8
#define EE_VGA_85			9

#define EE_SVGA_56			10
#define EE_SVGA_60			11
#define EE_SVGA_70			12
#define EE_SVGA_72			13
#define EE_SVGA_75			14
#define EE_SVGA_85			15

#define EE_832_75			16

#define EE_XGA_60			17
#define EE_XGA_70			18
#define EE_XGA_72			19
#define EE_XGA_75			20
#define EE_XGA_85			21

#define EE_WXGA_60			47
#define EE_WXGAR_60			48				// reduced
#define EE_WXGAP_60			49
#define EE_WXGAPR_60		50				// reduced

#define EE_1440_60			51
#define EE_1400_60			52

#define EE_1152_60 			22
#define EE_1152_70 			23
#define EE_1152_75 			24

#define EE_SXGA_60 			25
#define EE_SXGA_70 			26
#define EE_SXGA_75 			27

#define EE_RGB_480P			28
#define EE_RGB_720P			29
#define EE_RGB_720P50		30
#define EE_RGB_1080I		31
#define EE_RGB_1080I50A		32
#define EE_RGB_1080I50B		33
#define EE_RGB_480I			34
#define EE_RGB_576I			35
#define EE_RGB_576P			36

#define EE_YPbPr_480P		37
#define EE_YPbPr_720P		38
#define EE_YPbPr_720P50		39
#define EE_YPbPr_1080I		40
#define EE_YPbPr_1080I50A	41
#define EE_YPbPr_1080I50B	42
#define EE_YPbPr_480I		43
#define EE_YPbPr_576I		44
#define EE_YPbPr_576P		45

#define EE_PC_MAX			46

#define EE_PC_NO_SIGNAL		0xfe
#define EE_PC_UNKNOWN		0xff

extern WORD	MeasVLen, MeasVStart; 
extern WORD MeasHLen, MeasHStart;
extern WORD	RGB_HSTART; //, RGB_VSTART;
extern BYTE	/*RGB_HDE,*/ RGB_VDE;

//=====================================
// Level 0
//=====================================

void MeasSetWindow( WORD x, WORD y, WORD w, WORD h );
void MeasSetField(BYTE field);
void MeasLockData(BYTE fLock);
BYTE MeasStartMeasure(void);
BYTE MeasureAndWait(BYTE field);
BYTE MeasSetInputClkForHPeriodReg(BYTE fOn);
void MeasSetNoiseMask(BYTE mask);
void MeasSetErrTolerance(BYTE value);
void MeasEnableChangedDetection(BYTE on);
void MeasEnableDeMeasure(BYTE fOn);
void MeasSetThreshold(BYTE thold);

WORD MeasGetVPeriod( void );
WORD MeasGetHTotal(BYTE hPol);
WORD MeasGetHPeriod( void );
BYTE MeasGetHFreq(void);
WORD MeasGetHSyncRiseToFallWidth(void);
WORD MeasGetHSyncRiseToHActiveEnd(void);
WORD MeasGetVSyncRiseToFallWidth(void);
WORD MeasGetVsyncRisePos(void);
WORD MeasGetHActive( WORD *start );
WORD MeasGetHActive2( void );
WORD MeasGetDviDivider(void);
BYTE MesaGetHPol(void);

WORD MeasGetVActive( WORD *start );
WORD MeasGetVActive2(void);
DWORD	MeasGetVPeriod27( void );
BYTE MeasGetVFreq(void);
BYTE MeasRoundDownVFreqValue(BYTE vfreq);


BYTE MeasCheckVPulse(void);
//=====================================
// Level 1
//=====================================

void AutoTunePhase(void);
void AutoTuneColor(void);

void CheckMeasure(void);

#endif
