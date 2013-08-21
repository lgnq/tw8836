//-------------------------------------------------------------
//	HDMI_EPEP907M.h
//
//-------------------------------------------------------------
#ifndef __EP907M_H__
#define __EP907M_H__

#if 0
struct struct_HDINFO {
	WORD	HACTIVE;
	WORD	HFRONT;
	WORD	HBACK;
	WORD	HPULSE;
	WORD	VACTIVE;
	BYTE	VFRONT;
	BYTE	VBACK;
	BYTE 	VPULSE;
	BYTE	INTERLACE;
};
#endif


void EP9351_Reset(BYTE mode, WORD wait);
void Dummy_HDMI_EP907M_func(void);

void HDMIPowerDown(void);
BYTE HdmiDownloadEdid(DWORD addr); //test
BYTE HdmiDownloadDhcp(DWORD addr); //test

void Hdmi_SystemInit_EP907M(void);
void HdmiInitEp907MChip(void);
//BYTE CheckAndSet_EP9351(void);


void HDMI_Task(void);


void EP9x53Controller_Timer(void);


void EP9x53Controller_Task(void);


void  ResetHDMIStatus(void);


void Reset_Signal_Valid_Info(void);


void Reset_HDMI_Info(void);


void Reset_HDMI_Core(void);


void AudioMuteEnable(void) ;

void AudioMuteDisable(void) ;



void VideoMuteEnable(void) ;


void VideoMuteDisable(void) ;



void OutputToAnalogueYPbPr(void) ;// Analogue YPbPr


void OutputToAnalogueRGB(void); 



unsigned char DecodeTiming(short Timing_APPL, short Timing_HFP, short Timing_ALPF, short Timing_LFP, bit INTL);


// Called by DecodeTiming() only
unsigned char DecodeTiming_Conventional_6(unsigned char Code_4_3_0, unsigned char Code_16_9_1, unsigned char Code_4_3_2, unsigned char Code_16_9_3, unsigned char Code_4_3_4, unsigned char Code_16_9_5); 


// Called by DecodeTiming() only
unsigned char DecodeTiming_Conventional_2(unsigned char Code_4_3, unsigned char Code_16_9) ;

void ReadInterruptFlags() ;


#endif	//__EP907M_H__
