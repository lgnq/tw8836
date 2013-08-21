/**
 * @file
 * DebugMsg.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	Debug Message
*/
#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "CPU.h"
#include "Printf.h"
#include "Monitor.h"

#include "I2C.h"

#include "Scaler.h"
#include "InputCtrl.h"
#include "eeprom.h"
#include "aRGB.h"
#include "dtv.h"
#include "measure.h"
#include "PC_modes.h"	//for DVI_PrepareInfoString

#ifdef SUPPORT_HDMI_EP9351
#include "HDMI_EP9351.h"
#include "EP9351_RegDef.h"
#endif
#ifdef SUPPORT_HDMI_EP907M
#include "EP907M_RegDef.h"
#endif
#ifdef SUPPORT_HDMI_EP9553
#include "HDMI_EP9553.h"
#include "EP9553_RegDef.h"
#endif


#if !defined(SUPPORT_HDMI_EP9351) && !defined(SUPPORT_HDMI_EP907M)

//==========================================
//----------------------------
//Trick for Bank Code Segment
//----------------------------
CODE BYTE DUMMY_DEBUGMSG_CODE;
void Dummy_DebugMsg_func(void)
{
	BYTE temp;
	temp = DUMMY_DEBUGMSG_CODE;
}
#else //..SUPPORT_HDMI_EP9351


void HDMI_DumpVideoCode(BYTE vcode)
{
//#if 1 //BK120214 I need memory
//	BYTE temp = vcode;
//#else
	switch(vcode) {
	case 1: dPuts("640x480p@60"); break;
	case 2:
	case 3: dPuts("720x480p@59.94/60"); break;
	case 4: dPuts("1280x720p @ 59.94/60"); break;
	case 5: dPuts("1920x1080i @ 59.94/60"); break;
	case 6: 
	case 7: dPuts("720(1440)x480i@59.94/60Hz Pixel sent 2 times"); break;
	case 8: 
	case 9: dPuts("720(1440)x240p@59.94/60Hz Pixel sent 2 times"); break;
	case 10:
	case 11: dPuts("2880x480i @ 59.94/60Hz Pixel sent 1 to 10 times"); break;
	case 12:
	case 13: dPuts("2880x240p @ 59.94/60Hz Pixel sent 1 to 10 times"); break;
	case 14: 
	case 15: dPuts("1440x480p @ 59.94/60Hz Pixel sent 1 to 2 times"); break;
	case 16: dPuts("1920x1080p @ 59.94/60"); break;
	case 17: 
	case 18: dPuts("720x576p @ 50"); break;
	case 19: dPuts("1280x720p @ 50"); break;
	case 20: dPuts("1920x1080i @ 50"); break;
	case 21: 
	case 22: dPuts("720(1440)x576i @ 50Hz Pixel sent 2 times"); break;
	case 23: 
	case 24: dPuts("720(1440)x288p @ 50Hz Pixel sent 2 times"); break;
	case 25: 
	case 26: dPuts("2880x576i @ 50Hz Pixel sent 1 to 10 times"); break;
	case 27: 
	case 28: dPuts("2880x288 @ 50Hz Pixel sent 1 to 10 times"); break;
	case 29: 
	case 30: dPuts("1440x576p @ 50Hz Pixel sent 1 to 2 times"); break;
	case 31: dPuts("1920x1080p @ 50"); break;
	case 32: dPuts("1920x1080p @ 23.97/24"); break;
	case 33: dPuts("1920x1080p @ 25"); break;
	case 34: dPuts("1920x1080p @ 29.97/30"); break;
	case 35: 
	case 36: dPuts("2880x480p @ 59.94/60Hz Pixel sent 1, 2 or 4 times"); break;
	case 37: 
	case 38: dPuts("2880x576p @ 50Hz Pixel sent 1, 2 or 4 times"); break;
	case 39: dPuts("1920x1080i (1250 total) @ 50Hz No Repetition"); break;
	case 40: dPuts("1920x1080i @ 100"); break;
	case 41: dPuts("1280x720p @ 100"); break;
	case 42: 
	case 43: dPuts("720x576p @ 100"); break;
	case 44: 
	case 45: dPuts("720(1440)x576i @ 100Hz Pixel sent 2 times"); break;
	case 46: dPuts("1920x1080i @ 119.88/120"); break;
	case 47: dPuts("1280x720p @ 119.88/120"); break;
	case 48: 
	case 49: dPuts("720x480p @ 119.88/120"); break;
	case 50: 
	case 51: dPuts("720(1440)x480i @ 119.88/120Hz Pixel sent 2 times"); break;
	case 52: 
	case 53: dPuts("720X576p @ 200"); break;
	case 54: 
	case 55: dPuts("720(1440)x576i @ 200Hz Pixel sent 2 times"); break;
	case 56: 
	case 57: dPuts("720x480p @ 239.76/240"); break;
	case 58: 
	case 59: dPuts("720(1440)x480i @ 239.76/240Hz Pixel sent 2 times"); break;
	default: dPrintf("Unknown:%bx",vcode); break;
	}
//#endif
}


void HDMI_DumpAviInfoFrame(BYTE *cVal)
{
	BYTE i;

	Puts("\nAVI Info Frame reg $2A:");
	for ( i=0; i<15; i++ ) {
		Printf( "%02bx ", cVal[i] );
	}
	//S [2][1:0]
	i = cVal[2] & 3;
	dPuts("\n\tScan Information: ");
	if ( i==0 ) dPuts("No Data");
	else if ( i==1 ) dPuts("overscan");
	else if ( i==2 ) dPuts("underscan");
	else dPuts("Future");
	//B	[2][3:2]
	i = (cVal[2] & 0x0c)>>2;
	dPuts("\n\tBar Information: ");
	if ( i==0 ) dPuts("Bar data not Valid");
	else if ( i==1 ) dPuts("Vertical Data info Valid - [11]~[14]");
	else if ( i==2 ) dPuts("Horizontal Data info Valid - [7]~[10]");
	else dPuts("Vert[11]~[14] & Horiz[7]~[10] Data info Valid");
	//A [2][4]
	dPuts("\n\tActive Info:");
	if ( cVal[2] & 0x10 ) {
		// R [3][3:0]
		switch(cVal[3]&0x0f) {
		case 8:		dPuts("Same as picture aspect ratio"); break;
		case 9:		dPuts("4:3(center)"); break;
		case 10:	dPuts("16:9(center)"); break;
		default:	dPrintf("0x%bX",cVal[3]&0x0f); break;
		}
	}
	else dPuts("No");

	// Y [2][6:5]
	dPuts("\n\tInput HDMI format:");
	i = (cVal[2] & 0x60) >> 5;
	switch(i) {
	case 0: dPuts("RGB"); break;
	case 1:	dPuts("YUV(422)"); break;
	case 2:	dPuts("YUV(444)"); break;
	default: dPrintf("%bx",i);
	}
	// M [3][5:4]
	i = (cVal[3] & 0x30)>>4;
	Puts("\n\tPicture Aspect Ratio: ");
	if ( i==0 ) Puts("No Data");
	else if ( i==1 ) Puts("4:3");
	else if ( i==2 ) Puts("16:9");
	else Puts("Future");
	// 3[7:6]
	i = (cVal[3] & 0xc0)>>6;
	Puts("\n\tColorimetry: ");
	if ( i==0 ) Puts("No Data");
	else if ( i==1 ) Puts("SMPTE 170M, ITU601");
	else if ( i==2 ) Puts("ITU709");
	else {
		Printf("Extended Colorimetry Info - %bd", (cVal[4]&0x70)>>4);
		Puts("\n    0: xvYCC601");
		Puts("\n    1: xvYCC709");
	}
	// 4[1:0]
	i = cVal[4] & 3;
	Puts("\n\tNon-Uniform Picture Scaling: ");
	if ( i==0 ) Puts("No known");
	else if ( i==1 ) Puts("Horizontally");
	else if ( i==2 ) Puts("Vertically");
	else Puts("Horizontally and Vertically");

	Printf("\n\tVideo Identification Code:%bd:", cVal[5] & 0x7f );
	HDMI_DumpVideoCode(cVal[5] & 0x7f);


	Printf("\n\tPixel Repetition Factor: %bd", cVal[6] & 0x0f);
}

void DBG_PrintAviInfoFrame(void)
{
	BYTE TempByte[15];

#ifdef SUPPORT_HDMI_EP9351
	ReadI2C_B0(I2CID_EP9351, EP9351_AVI_InfoFrame, TempByte, 15);
#endif
#ifdef SUPPORT_HDMI_EP907M
	TempByte[0] = ReadI2CI16Byte(I2CID_EP907M, EP907M_AVI_Information);
	TempByte[1] = 0xFF;  //dummy
	ReadI2CI16(I2CID_EP907M, EP907M_AVI_Information+1, &TempByte[2], 5);
#endif
	HDMI_DumpAviInfoFrame(TempByte);
}
//-------------------------------------
#if 0
//BKFYI121121. 
//This function has a bank issue.
//Please do not use it. I will update it later.
void HDMI_DumpTimingRegister(BYTE *TempByte, BYTE Status)
{
	//WORD temp16;
	//BYTE i;
	//DECLARE_LOCAL_page
	//WORD HActiveStart;
//	WORD VDEStart;
	//BYTE ret;

	WORD HSync,HBPorch,HActive,HFPorch;
	WORD VActive, VFPorch;
	WORD VSync,VBPorch;

	BYTE i;
	//volatile BYTE vTemp;
	//BYTE TempByte[15];
	//BYTE bTemp;

	//BYTE Status;
	BYTE HPol,VPol;		
	WORD HTotal,VTotal;


#ifdef SUPPORT_HDMI_EP9351
	HActive = TempByte[1]; 	HActive <<= 8;		HActive += TempByte[0];
	HFPorch = TempByte[3]; 	HFPorch <<= 8;		HFPorch += TempByte[2];
	HBPorch = TempByte[5]; 	HBPorch <<= 8;		HBPorch += TempByte[4];
	HSync   = TempByte[7]; 	HSync <<= 8;		HSync   += TempByte[6];
	VActive = TempByte[9]; 	VActive <<= 8;		VActive += TempByte[8];
	VFPorch =  TempByte[10];
	VBPorch =  TempByte[11];
	VSync =  TempByte[12]&0x7F;

	HTotal = HSync + HBPorch + HActive + HFPorch;
	VTotal = VSync + VBPorch + VActive + VFPorch;

	HPol = Status & 0x40 ? 1:0;	//ActiveLow
	VPol = Status & 0x80 ? 1:0;	//ActiveLow
#endif
#ifdef SUPPORT_HDMI_EP907M
	HActive = TempByte[0]; 	HActive <<= 8;		HActive += TempByte[1];
	HFPorch = TempByte[2]; 	HFPorch <<= 8;		HFPorch += TempByte[3];
	HBPorch = TempByte[4]; 	HBPorch <<= 8;		HBPorch += TempByte[5];
	HSync   = TempByte[6]; 	HSync <<= 8;		HSync   += TempByte[7];
	VActive = TempByte[8]; 	VActive <<= 8;		VActive += TempByte[9];
	VFPorch =  TempByte[10];
	VBPorch =  TempByte[11];
	VSync =  TempByte[12]&0x7F;
	HTotal = HSync + HBPorch + HActive + HFPorch;
	VTotal = VSync + VBPorch + VActive + VFPorch;
	HPol = Status & EP907M_Polarity_Control__HS_POL_Low ? 1:0;	//ActiveLow
	VPol = Status & EP907M_Polarity_Control__VS_POL_Low ? 1:0;	//ActiveLow
#endif


#ifdef SUPPORT_HDMI_EP9351
	Puts("\nTimeReg:$41:");
#endif
#ifdef SUPPORT_HDMI_EP907M
	Puts("\nTimeReg:$3B:");
#endif
	for(i=0; i < 13; i++)
		Printf("%02bx ",TempByte[i]);
	dPrintf(" $41:%02bx",Status);

	dPrintf("\n\tH Total:%4d Pulse:%4d BPorch:%4d Active:%4d FPorch:%3d Pol:%bd",
		HTotal,HSync,HBPorch,HActive,HFPorch,HPol);
	dPrintf("\n\tV Total:%4d Pulse:%4d BPorch:%4d Active:%4d FPorch:%3d Pol:%bd",
		VTotal,VSync,VBPorch,VActive,VFPorch,VPol);
	dPrintf("\n\t%s", TempByte[12] & 0x80 ? "Interlace" : "Progressive");
}
#endif


void DBG_PrintTimingRegister(void)
{
	BYTE TempByte[15];
	BYTE Status;
	WORD HSync,HBPorch,HActive,HFPorch;
	WORD VActive, VFPorch;
	WORD VSync,VBPorch;

	BYTE i;
	BYTE HPol,VPol;		
	WORD HTotal,VTotal;

#ifdef SUPPORT_HDMI_EP9351
	ReadI2C_B0(I2CID_EP9351, EP9351_Timing_Registers, TempByte, 13);
	Status = ReadI2CByte(I2CID_EP9351,EP9351_General_Control_1);
#endif
#ifdef SUPPORT_HDMI_EP907M
	ReadI2CI16(I2CID_EP907M, EP907M_Hsync_Timing, TempByte, 8);
	ReadI2CI16(I2CID_EP907M, EP907M_Vsync_Timing, &TempByte[8], 5);
	Status = ReadI2CI16Byte(I2CID_EP907M,EP907M_Polarity_Control);
#endif

	//HDMI_DumpTimingRegister(TempByte,Status);




#ifdef SUPPORT_HDMI_EP9351
	HActive = TempByte[1]; 	HActive <<= 8;		HActive += TempByte[0];
	HFPorch = TempByte[3]; 	HFPorch <<= 8;		HFPorch += TempByte[2];
	HBPorch = TempByte[5]; 	HBPorch <<= 8;		HBPorch += TempByte[4];
	HSync   = TempByte[7]; 	HSync <<= 8;		HSync   += TempByte[6];
	VActive = TempByte[9]; 	VActive <<= 8;		VActive += TempByte[8];
	VFPorch =  TempByte[10];
	VBPorch =  TempByte[11];
	VSync =  TempByte[12]&0x7F;

	HTotal = HSync + HBPorch + HActive + HFPorch;
	VTotal = VSync + VBPorch + VActive + VFPorch;

	HPol = Status & 0x40 ? 1:0;	//ActiveLow
	VPol = Status & 0x80 ? 1:0;	//ActiveLow
#endif
#ifdef SUPPORT_HDMI_EP907M
	HActive = TempByte[0]; 	HActive <<= 8;		HActive += TempByte[1];
	HFPorch = TempByte[2]; 	HFPorch <<= 8;		HFPorch += TempByte[3];
	HBPorch = TempByte[4]; 	HBPorch <<= 8;		HBPorch += TempByte[5];
	HSync   = TempByte[6]; 	HSync <<= 8;		HSync   += TempByte[7];
	VActive = TempByte[8]; 	VActive <<= 8;		VActive += TempByte[9];
	VFPorch =  TempByte[10];
	VBPorch =  TempByte[11];
	VSync =  TempByte[12]&0x7F;
	HTotal = HSync + HBPorch + HActive + HFPorch;
	VTotal = VSync + VBPorch + VActive + VFPorch;
	HPol = Status & EP907M_Polarity_Control__HS_POL_Low ? 1:0;	//ActiveLow
	VPol = Status & EP907M_Polarity_Control__VS_POL_Low ? 1:0;	//ActiveLow
#endif


#ifdef SUPPORT_HDMI_EP9351
	Puts("\nTimeReg:$41:");
#endif
#ifdef SUPPORT_HDMI_EP907M
	Puts("\nTimeReg:$3B:");
#endif
	for(i=0; i < 13; i++)
		Printf("%02bx ",TempByte[i]);
	dPrintf(" $41:%02bx",Status);

	dPrintf("\n\tH Total:%4d Pulse:%4d BPorch:%4d Active:%4d FPorch:%3d Pol:%bd",
		HTotal,HSync,HBPorch,HActive,HFPorch,HPol);
	dPrintf("\n\tV Total:%4d Pulse:%4d BPorch:%4d Active:%4d FPorch:%3d Pol:%bd",
		VTotal,VSync,VBPorch,VActive,VFPorch,VPol);
	dPrintf("\n\t%s", TempByte[12] & 0x80 ? "Interlace" : "Progressive");

}


#if defined(SUPPORT_HDMI_EP9351)
void DBG_DumpControlRegister(void)
{
	BYTE TempByte[9];

	TempByte[0] = ReadI2CByte(I2CID_EP9351, EP9351_General_Control_0);
	Printf("EP9351_General_Control_0 : %02bX\n",TempByte[0]);
	TempByte[0] = ReadI2CByte(I2CID_EP9351, EP9351_General_Control_8);
	Printf("EP9351_General_Control_8 : %02bX\n",TempByte[0]);

	//read M0
	ReadI2C_B0(I2CID_EP9351, EP9351_M0_Register, &TempByte, 9);
	Printf("M0 : %02bX %02bX %02bX %02bX %02bX %02bX %02bX %02bX %02bX\n",
		TempByte[0],TempByte[1],TempByte[2],TempByte[3],TempByte[4],TempByte[5],TempByte[6],TempByte[7],TempByte[8]);
}
#endif


//===================================================================

#if 0
#define PI_I		   0	//Interlaced
#define PI_P		   1	//Progressive
#define ASPECT_43	0x10	//4x3
#define ASPECT_169	0x20	//16x9
#define ASPECT_BOTH 0x00	//4x3,16x9
struct DVITimeData_s {
	BYTE format;
	WORD hActive,vActive;
	BYTE aspect_p;
	//WORD hTotal; = hActive+hBlank
	//WORD vTotal; = vActive+vBlank
	BYTE hPol_vPol;
	WORD hSync,hBPorch,hBlank;	//hFPorch=hBlank-hSync-hBPorch;
	WORD vSync,vBPorch,vBlank;
	BYTE hFreq_hi;
	WORD hFreq_lo;
	BYTE vFreq_hi;
	WORD vFreq_lo;	
	//DWORD pFreq;	//Pixel Freq. = hTotal * hFreq
};
CODE struct DVITimeData_s DviTable[] = {
//Format V active Int/Prog Total Lines V blanking1 V Freq H Freq Pixel Freq H total H active H blanking Aspect2 Display3 Reference
//  Active    		Total		Blank		   Freq		 		Pixel
//	H    V	  		H     V		H		V		H		V		Freq
//#if 0 //Output~59.94Hz
{1,	 640, 480,PI_P|ASPECT_43, 	/*800, 525, */ 0x00,	96,48,160,	2,33,45,	31,469, 59,940	},	//	25.175,  CRT/Dig MTS [10]
{3,	 720, 480,PI_P|ASPECT_BOTH, /*858, 525, */ 0x00,	62,60,138,	6,30,45,	31,469, 59,940	},	//	27.000,  CRT/Dig 770.2 [19]
{4,	1280, 720,PI_P|ASPECT_169,	/*1650, 750,*/ 0x11,	40,220,370,	5,20,30,	44,955, 59,939	},	//	74.175,  Dig 770.3 [20]
{5,	1920,1080,PI_I|ASPECT_169,	/*2200,1125,*/ 0x11,	44,148,280,	5,15,22,	33,716, 59,939	},	//	74.175,  CRT 770.3 [20]
{7,	1440, 480,PI_I|ASPECT_BOTH,	/*1716, 525,*/ 0x00,	124,114,276,3,15,22,	15,734, 59,940	},	//	27.000, CRT 770.2 [19]
{9,	1440, 240,PI_P|ASPECT_BOTH,	/*1716, 262,*/ 0x00,	124,114,276,3,15,22, 	15,734, 60, 54	},	//	27.000, CRT 770.2 [19]
{9,	1440, 240,PI_P|ASPECT_BOTH,	/*1716, 263,*/ 0x00,	124,114,276,3,15,23, 	15,734, 59,826	},	//	27.000, CRT 770.2 [19]
{11,2880, 480,PI_I|ASPECT_BOTH,	/*3432, 525,*/ 0x00,	248,228,552,3,15,22, 	15,734, 59,940	},	//	54.000, CRT 770.2 [19]
{13,2880, 240,PI_P|ASPECT_BOTH,	/*3432, 262,*/ 0x00,	248,228,552,3,15,22, 	15,734, 60, 54	},	//	54.000, CRT 770.2 [19]
{13,2880, 240,PI_P|ASPECT_BOTH,	/*3432, 263,*/ 0x00,	248,228,552,3,15,23, 	15,734, 59,826	},	//	54.000, CRT 770.2 [19]
{15,1440, 480,PI_P|ASPECT_BOTH,	/*1716, 525,*/ 0x00,	124,120,276,6,30,45, 	31,469, 59,940	},	//	54.000, CRT/Dig 770.2 [19]
{16,1920,1080,PI_P|ASPECT_169,	/*2200,1125,*/ 0x11,	44,148,280,	5,36,45, 	67,432, 59,939	},	// 148.350, Dig
//#endif

{1,  640, 480,PI_P|ASPECT_43, 	/*800, 525,	*/	0x00,	96,48,160,	2,33,45,	31,500, 60,  0},	//	25.200,	 	CRT/Dig MTS [10]
{3,	 720, 480,PI_P|ASPECT_BOTH, /*858, 525,	*/	0x00,	62,60,138,	6,30,45,	31,500, 60,  0},	//	27.027,	 	 CRT/Dig 770.2 [19]
{4, 1280, 720,PI_P|ASPECT_169,	/*1650, 750,*/	0x11,	40,220,370,	5,20,30,	45,  0, 60,  0},	//	74.250,	 	 Dig 770.3 [20]
{5, 1920,1080,PI_I|ASPECT_169,	/*2200,1125,*/	0x11,	44,148,280,	5,15,22,	33,750, 60,  0},	//	74.250,	 	 CRT 770.3 [20]
{7, 1440, 480,PI_I|ASPECT_BOTH,	/*1716, 525,*/	0x00,	124,114,276,3,15,22,	15,750, 60,  0},	//	27.027,	  	 CRT 770.2 [19]
{9, 1440, 240,PI_P|ASPECT_BOTH,	/*1716, 262,*/	0x00,	124,114,276,3,15,22, 	15,750, 60,115},	//	27.027,	  	 CRT 770.2 [19]
{9, 1440, 240,PI_P|ASPECT_BOTH,	/*1716, 263,*/	0x00,	124,114,276,3,15,23, 	15,750, 59,886},	//	27.027,	  	 CRT 770.2 [19]
{11,2880, 480,PI_I|ASPECT_BOTH,	/*3432, 525,*/	0x00,	248,228,552,3,15,22, 	15,750, 60,  0},	//	54.054,	  	 CRT 770.2 [19]
{13,2880, 240,PI_P|ASPECT_BOTH,	/*3432, 262,*/	0x00,	248,228,552,3,15,22, 	15,750, 60,115},	//	54.054,	  	 CRT 770.2 [19]
{13,2880, 240,PI_P|ASPECT_BOTH,	/*3432, 263,*/	0x00,	248,228,552,3,15,23, 	15,750, 59,886},	//	54.054,	  	 CRT 770.2 [19]
{15,1440, 480,PI_P|ASPECT_BOTH,	/*1716, 525,*/	0x00,	124,120,276,6,30,45, 	31,500, 60,  0},	//	54.054,	 	 CRT/Dig 770.2 [19]
{16,1920,1080,PI_P|ASPECT_169,	/*2200,1125,*/	0x11,	44,148,280,	5,36,45, 	67,500, 60,  0},	// 148.500,	 	 Dig

{18, 720, 576,PI_P|ASPECT_BOTH, /*864, 625,	*/	0x00,	64,68,144, 	5,39,49,	31,250, 50,  0},	//   27.000 	  	CRT/Dig BT1358[31]
{19,1280, 720,PI_P|ASPECT_169,	/*1980, 750,*/	0x11,	40,220,700, 5,20,30,	37,500, 50,  0},	//   74.250 	  	Dig 296M[14]
{20,1920,1080,PI_I|ASPECT_169,	/*2640,1125,*/	0x11,	44,148,720, 5,15,22, 	28,125, 50,  0},	//   74.250 	  	CRT 274M[12]
{22,1440, 576,PI_I|ASPECT_BOTH,	/*1728, 625,*/	0x00,	126,138,288,3,19,24, 	15,625, 50,  0},	//   27.000 	  	CRT BT656-4[33]
{24,1440, 288,PI_P|ASPECT_BOTH,	/*1728, 312,*/	0x00,	126,138,276,3,19,24, 	15,625, 50, 80},	//   27.000 	  	CRT BT656-4[33]
{24,1440, 288,PI_P|ASPECT_BOTH,	/*1728, 313,*/	0x00,	126,138,276,3,19,25, 	15,625, 49,920},	//   27.000 	  	CRT BT656-4[33]
{24,1440, 288,PI_P|ASPECT_BOTH,	/*1728, 314,*/	0x00,	126,138,276,3,19,26, 	15,625, 49,761},	//   27.000 	  	CRT BT656-4[33]
{26,2880, 576,PI_I|ASPECT_BOTH,	/*3456, 625,*/	0x00,	252,276,552,3,19,24, 	15,625, 50,  0},	//   54.000 	  	CRT BT656-4[33]
{28,2880, 288,PI_P|ASPECT_BOTH,	/*3456, 312,*/	0x00,	252,276,552,3,19,24, 	15,625, 50, 80},	//   54.000 	  	CRT BT656-4[33]
{28,2880, 288,PI_P|ASPECT_BOTH,	/*3456, 313,*/	0x00,	252,276,552,3,19,25, 	15,625, 49,920},	//   54.000 	  	CRT BT656-4[33]
{28,2880, 288,PI_P|ASPECT_BOTH,	/*3456, 314,*/	0x00,	252,276,552,3,19,26, 	15,625, 49,761},	//   54.000 	  	CRT BT656-4[33]
{30,1440, 576,PI_P|ASPECT_BOTH,	/*1728, 625,*/	0x01,	128,136,288,5,19,49,	31,250, 50,  0},	//   54.000 	  	CRT/Dig BT1358[31]
{31,1920,1080,PI_P|ASPECT_169,	/*2640,1125,*/	0x11,	44,148,720, 5,36,45,	56,250, 50,  0},	//   148.500 	Dig 274M[12]

{32,1920,1080,PI_P|ASPECT_169,	/*2750,1125,*/	0x11,	44,148,830, 5,36,45, 	26,973, 23,976},	//  74.175, 	 CRT/Dig 274M[12]
{32,1920,1080,PI_P|ASPECT_169,	/*2750,1125,*/	0x11,	44,148,830, 5,36,45, 	27,  0, 24,  0},	//  74.250, 	 CRT/Dig 274M[12]
{33,1920,1080,PI_P|ASPECT_169,	/*2640,1125,*/	0x11,	44,148,720, 5,36,45, 	28,125, 25,  0},	//  74.250, 	 CRT/Dig 274M[12]
{34,1920,1080,PI_P|ASPECT_169,	/*2200,1125,*/	0x11,	44,148,280, 5,36,45, 	33,716, 29,970},	//  74.175, 	 CRT/Dig 274M[12]
{34,1920,1080,PI_P|ASPECT_169,	/*2200,1125,*/	0x11,	44,148,280, 5,36,45, 	33,750, 30,  0}, 	//  74.250, 	 CRT/Dig 274M[12]
{0,0,0}
};											
#endif						

#ifdef SUPPORT_HDMI_EP9351
void DumpDviTable(WORD hActive,WORD vActive)
{
#if 1
	WORD temp;
	temp = hActive;
	temp = vActive;
#else
	BYTE i;

	for(i=0; DviTable[i].format ;i++) {
		//if(DivTable[i]format==0)
		//	break;
		if(DviTable[i].hActive==hActive && DviTable[i].vActive==vActive) {
			Printf("\ni:%bd id:%bd ", i,DviTable[i].format);
			Printf("%dx%d",DviTable[i].hActive,DviTable[i].vActive);
			Printf("%s ", DviTable[i].aspect_p & PI_P ? "p" :"i"); 
			if(DviTable[i].aspect_p & ASPECT_43)  Printf("4:3 "); 
			if(DviTable[i].aspect_p & ASPECT_169) Printf("16:9 ");

			Printf("%dx%d ",
				DviTable[i].hActive+DviTable[i].hBlank,
				DviTable[i].vActive+DviTable[i].vBlank);	


			Printf("H:%bd",DviTable[i].hPol_vPol >> 4);
			Printf(" FPorch:%d Sync:%d BPorch:%d",DviTable[i].hBlank-DviTable[i].hSync-DviTable[i].hBPorch,DviTable[i].hSync,DviTable[i].hBPorch); 
			//Printf(" %bd.%03dKhz",DviTable[i].hFreq_hi,DviTable[i].hFreq_lo);
			Printf(" V:%bd",DviTable[i].hPol_vPol & 0x0F);
			Printf(" FPorch:%d Sync:%d BPorch:%d",DviTable[i].vBlank-DviTable[i].vSync-DviTable[i].vBPorch,DviTable[i].vSync,DviTable[i].vBPorch); 
			//Printf(" %bd.%03dHz",DviTable[i].vFreq_hi,DviTable[i].vFreq_lo);
			Printf(" %bdHz",DviTable[i].vFreq_hi);
		}
	}
#endif
}
#endif						

//===================================================================


#endif //..SUPPORT_HDMI_EP9351

void DbgMsg_EEP_Corruptted(void)
{
	Printf("\nSPI EEPROM is Corruptted.");
	Printf("\nPlease execute following steps.");
	Printf("\n	mcu ws 9b 0 ;turnoff cache");
	Printf("\n	ee format	;format EEPROM");
	Printf("\n	ee find		;find bank& write position");
	Printf("\n	ee init		;assign default video values");
	Printf("\n	ee w %bx 3	;set DEBUG_INFO", EEP_DEBUGLEVEL);
	Printf("\n--and restart system");
}