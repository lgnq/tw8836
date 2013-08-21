#ifndef __SCALER_H__
#define __SCALER_H__

#define VIDEO_ASPECT_NORMAL		0
#define VIDEO_ASPECT_ZOOM		1
#define VIDEO_ASPECT_FILL		2
#define VIDEO_ASPECT_PANO		3
extern BYTE VideoAspect;


//======================================================================
// CEA 861 Video Table
//======================================================================
#define SYNC_POL_ACTIVE_LOW		1
#define FREQ_60		60
#define FREQ_50		50
#define FREQ_30		30
#define FREQ_25		25
#define FREQ_24		24

typedef struct s_DTV_table {
	BYTE vid;
	WORD hDeWidth,vDeWidth;		
	BYTE fIorP; //'I' has 1, 'P' has 0.
	BYTE vfreq;
	BYTE Pol;	/* hPol | vPol */
	WORD hTotal; 	BYTE hFPorch, hSyncWidth, hBPorch;  
	WORD vTotal; 	BYTE vPad, vSyncStart, vSyncWidth, vBPorch;
	DWORD pixelfreq;
} t_DTV_table;
typedef struct s_VESA_table {
	BYTE vid;
	WORD hDeWidth,vDeWidth;		
	BYTE vfreq;
	BYTE Pol;	/* hPol | vPol */
	WORD hTotal; 	BYTE hFPorch, hSyncWidth, hBPorch;  
	WORD vTotal; 	BYTE vFPorch, vSyncWidth, vBPorch;
	DWORD pixelfreq;
} t_VESA_table;

//18bytes
typedef struct s_VIDEO_TIME {
	WORD hDeWidth,vDeWidth;
	BYTE fIorP;	//'I' has 1, 'P' has 0.
	BYTE vfreq;	//59 or 60, 50,25,24,30.
	BYTE Pol;	//[4]:hPol, [0]:vPol. if high->low->high, it has 1.	So, 480p will have 0x11.
	WORD hTotal;	BYTE hFPorch, hSync, hBPorch;  
	WORD vTotal; 	BYTE vFPorch, /*vPad, vSyncStart,*/ vSync, vBPorch;
} t_VIDEO_TIME;


#ifdef INCLUDE_DTV_TABLE
code t_DTV_table DTV_table[] = {
/* 0 */	{1, 640,480,0,FREQ_60,	0x11,	800, 16,96,48,		525, 10,0,2,33,		251750000L },	/* #1		640x480p @59.94*/
/* 1 */	{2, 720,480,0,FREQ_60,	0x11,	858, 16,62, 60, 	525, 3,6,6, 30,		27000000L },	/* #2,#3	720x480p	@59.940 */			
/* 2 */	{4, 1280,720,0,FREQ_60,	0x00,	1650, 110,40,220, 	750, 5,0,5, 20,		44000000L  },	/* #4 		1280x720p	@59.939 pclk:74175000L */
/* 3*/	{5, 1920,540,1,FREQ_60,	0x00,	2200, 88,44, 148,  	562, 2,0,5, 15,		47500000L },	/* #5 1920x1080i	@59.939 pclk:74175000L NOTE: "1920x540I"->"1080I"*/ 
/* 4*/	{6, 1440,240,1,FREQ_60,	0x11,	1716,38,124,114,  	262, 1,3,3, 15,		27000000L },  	/* #6,#7 720(1440)x480i @59.940 */
/*FYI   {6, 720, 480,1,FREQ_60,	0x11,	858, 19, 62,57,  	524, 2,6,6, 30,		27000000L }, */
/*FYI	{6, 720, 480,1,FREQ_60,	0x11,	858, 19, 62,57,  	525, 2,6,6, 30,		27000000L }, */
/* 5*/	{8, 1440,240,0,FREQ_60,	0x11,	1716,38,124,114,	262, 1,3,3,15,		27000000L},		/* #8,#9 720(1440)x240p @59.94 */
/* 6*/ 	{10, 2880,240,1,FREQ_60,0x11,	3432,76,248,228,	262,1,3,3,15,		54000000L},		/* #10,#11 (2880)x480i @59.94 */
/* 7 */	{12, 2880,240,0,FREQ_60,0x11,	3432,76,248,228,	262,1,3,3,15,		54000000L},		/* #12,#13 (2880)x240p@59.94 */
/* 8 */	{14, 1440,480,0,FREQ_60,0x11,	1716,32,124,120,	525,3,6,6,30,		54000000L},	   	/* #14,#15 1440x480p@59.94 */
/* 9 */	{16, 1920,1080,0,FREQ_60,0x00,	2200,88,44, 148,  	1125,4,0,5, 36,		44000000L  },	/* #16 1920x1080p @59.939 pclk:148350000L */	   
/*10 */	{17, 720,576,0,FREQ_50,	0x11,	864,12,64,68,  	 	625,5,0,5, 39, 		27000000L }, 	/* #17,#18 720x576p	@50Hz */
/*11 */	{19, 1280,720,0,FREQ_50,0x11,	1980, 440,40,220,	750, 5,0,5,20,		74250000L },	/* #19 1280x720p @50Hz  */
/*12 */	{20, 1920,540,1,FREQ_50,0x11,	2640,528,44,148,	566,2,0,5,15,   	74250000L},		/* #20 1920x1080i@50Hz */
/*13 */	{21, 1440,288,1,FREQ_50,0x11,	1728,24,126,138,  	312,2,0,3,19,  		27000000L },  	/* #21,#22 720(1440)x576i@50Hz */
/*14 */	{23, 1440,288,0,FREQ_50,0x11,	1728,24,126,138,	312,2,0,3,19,		27000000L}, 	/* #23,#24 720(1440)x288p @50 */
/*15 */	{25, 2880,288,1,FREQ_50,0x11,	3456,48,252,276,	312,2,0,3,19, 		54000000L},	 	/* #25,#26 (2880)x576i @50 */
/*16 */	{27, 2880,288,0,FREQ_25,0x11,	3456,48,252,276,	312,2,0,3,19, 		54000000L},	 	/* #27,#28 (2880)x288p @25 */						
/*17 */	{29, 1440,576,0,FREQ_50,0x11,	1728,24,128,136,  	625,5,0,5, 39,  	54000000L }, 	/* #29,#30 1440x576p	@50Hz  */
/*18 */	{31, 1920,1080,0,FREQ_50, 0x00,	2640,528,44,148,	1125,4,0,5,36, 		148500000L},  	/* #31 1920x1080p @50 */
/*19 */	{32, 1920,1080,0,FREQ_24, 0x00,	2750,638,44,148,	1125,4,0,5,36, 		74175000L},	  	/* #32 1920x1080p @23.97 */
/*20 */	{33, 1920,1080,0,FREQ_25, 0x00,	2640,528,44,148,	1125,4,0,5,36, 		74250000L},		/* #33 1920x1080p @25Hz */
/*21 */	{34, 1920,1080,0,FREQ_30, 0x00,	2640,528,44,148,	1125,4,0,5,36, 		74250000L},	 	/* #34 1920x1080p @29.97 */

//-----------------------------------
// convert table for 480i,576i,1080i
/*22 */	{35, 720,480,1,FREQ_60,	0x11,	858,19,62,57,  		525, 3,6,6, 30,		27000000L },  	/* 480i */
/*23 */	{36, 720,576,1,FREQ_50,	0x11,	864,12,63,69,  		624, 4,0,6, 38,  	27000000L },  	/* 576i */
/*24 */	{37, 1920,1080,1,FREQ_60,0x00,	2200, 88,44, 148,  	1124, 4,0,10, 30,	44000000L },	 /* 1080i pclk:74175000L */


/* for test */
/*25 */	{35,800,600,0,FREQ_60,	0x00,	1056,40,128,88,		628, 1,0,4,23, 		40000000L},     /* 800x600@60 */ 
/*26 */	{36,800,480,0,FREQ_60,	0x01,	1056,40,128,88,		525, 3,6,6,30,		33264000L}      /* 800x480@60 */

};

code t_VESA_table VESA_table[] = {	   
/**/	//{0x01,640,350,85 	},
/**/	//{0x02,640,400,85 	},
/**/	//{0x03,720,400,85 	},
 		 	  
/**/	{0x04,640,480,60,	0x00,	800,16/*8*/,96,48,	525,10/*2*/,2,33/*25*/, 25175	 	},
/**/ //	{0x05,640,480,72 	},
/**/ //	{0x06,640,480,75 	},
/**/	//{0x07,640,480,85 	},
 		 	  
/**/ //	{0x08,800,600,56 	},
/**/	{0x09,800,600,60, 	0x00,	1056,40,129,88,	628,1,4,23, 40000},
/**/ //	{0x0A,800,600,72 	},
/**/ //	{0x0B,800,600,75 	},
/**/	//{0x0C,800,600,85 	},
/**/	//{0x0D,800,600,120	},
 		 	  
/**/ //	{0x0E,848,480,60 	},
 		 	  
/**/ //	{0x0F,1024,768,43	},
/**/	{0x10,1024,768,60,	0x00,	1344,24,136,160,	806,3,6,29},
/**/ //	{0x11,1024,768,70	},
/**/ //	{0x12,1024,768,75	},
/**/	//{0x13,1024,768,85	},
/**/	//{0x14,1024,768,120 },
 		 	  
/**/ //	{0x15,1152,864, 75},
 		 	  
/**/ //	{0x16,1280,768,60	},
/**/ //	{0x17,1280,768,60	},
/**/ //	{0x18,1280,768,75	},
/**/	//{0x19,1280,768,85	},
/**/	//{0x1A,1280,768,120},
 		 	  
/**/ //	{0x1B,1280,800,60 	},
/**/ //	{0x1C,1280,800,60 	},
/**/ //	{0x1D,1280,800,75 	},
/**/ //	{0x1E,1280,800,85 	},
/**/ //	{0x1F,1280,800,120	},
 		 	  
/**/ //	{0x20,1280,960,60	},
/**/	//{0x21,1280,960,85	},
/**/	//{0x22,1280,960,120},
 		 	  
/**/ //	{0x23,1280,1024,60 	},
/**/	//{0x24,1280,1024,75 	},
/**/	//{0x25,1280,1024,85 	},
/**/	//{0x26,1280,1024,120	},
 		 	  
/**/ //	{0x27,1360,768,60	},
/**/	//{0x28,1360,768,120	},
 		 	  
/**/ //	{0x29,1400,1050, 60 },
/**/ //	{0x2A,1400,1050, 60 },
/**/	//{0x2B,1400,1050, 75 },
/**/	//{0x2C,1400,1050, 85 },
/**/	//{0x2D,1400,1050, 120},
 		 	  
/**/ //	{0x2E,1440,900, 60 },
/**/ //	{0x2F,1440,900, 60 },
/**/	//{0x30,1440,900, 75 },
/**/	//{0x31,1440,900, 85 },
/**/	//{0x32,1440,900, 120},
 		 	  
/**/ //	{0x33,1600,1200,60 },
/**/	//{0x34,1600,1200,65 },
/**/	//{0x35,1600,1200,70 },
/**/	//{0x36,1600,1200,75 },
/**/	//{0x37,1600,1200,85 },
/**/	//{0x38,1600,1200,120},
 		 	  
/**/ //	{0x39,1680,1050,60 },
/**/	//{0x3A,1680,1050,60 },
/**/	//{0x3B,1680,1050,75 },
/**/	//{0x3C,1680,1050,85 },
/**/	//{0x3D,1680,1050,120},
 		 	  
/**/	//{0x3E,1792,1344,60 },
/**/	//{0x3F,1792,1344,75 },
/**/	//{0x40,1792,1344,120 },
 		 	  
/**/	//{0x41,1856,1392,60},
/**/	//{0x42,1856,1392,75},
/**/	//{0x43,1856,1392,120},
 		 	  
/**/	//{0x44,1920,1200,60},
/**/	//{0x45,1920,1200,60},
/**/	//{0x46,1920,1200,75},
/**/	//{0x47,1920,1200,85},
/**/	//{0x48,1920,1200,120},
 		 	  
/**/	//{0x49,1920,1440,60},
/**/	//{0x4A,1920,1440,75},
/**/	//{0x4B,1920,1440,120},
 		 	  
/**/	//{0x4C,2560,1600, 60},
/**/	//{0x4D,2560,1600, 60},
/**/	//{0x4E,2560,1600, 75},
/**/	//{0x4F,2560,1600, 85},
/**/	//{0x50,2560,1600, 120},
	   	
/**/	{0xFF,0xFFFF,0xFFFF, 0xFF}
};
#else
//extern t_DTV_table DTV_table[];
#endif


#define VID_720X480P_IDX	1
#define VID_1280X720P_IDX	2
#define VID_1920X1080I_IDX	3
#define	VID_720X480I_IDX	4
#define VID_720X576P_IDX	10
#define VID_720X576I_IDX	13

#define VID_800X600P_IDX	25
#define VID_800X480P_IDX	26

#define DTV_TABLE_MAX 22
#define DTV_TABLE_480P 1	//who is a default?

#define VESA_TABLE_MAX 22


void ScalerSetOutputFixedVline(BYTE onoff /*, WORD lines*/);


//-------------------------------------
// Scale ratio
void ScalerWriteXUpReg(WORD value);
void ScalerWriteXDownReg(WORD value);
WORD ScalerReadXDownReg(void);
void ScalerWriteVScaleReg(WORD value);
WORD ScalerReadVScaleReg(void);
void ScalerSetHScaleReg(WORD down, WORD up);

void ScalerSetHScale(WORD Length);
void ScalerSetHScaleWithRatio(WORD Length, WORD ratio);
void ScalerSetVScale(WORD Length);
void ScalerSetVScaleWithRatio(WORD Length, WORD ratio);

//-------------------------------------
// Panorama WaterGlass effect
void ScalerPanoramaOnOff(BYTE fOn);
void ScalerSetPanorama(WORD px_scale, short px_inc);

//-------------------------------------
// Line Buffer
void ScalerWriteLineBufferDelay(BYTE delay);
BYTE ScalerReadLineBufferDelay(void);
void ScalerSetLineBufferSize(WORD length);
void ScalerSetLineBuffer(BYTE delay, WORD length);
void ScalerSetFPHSOutputPolarity(BYTE fInvert);

void ScalerWriteOutputHBlank(WORD length);

//-------------------------------------
// HDE VDE
void ScalerWriteHDEReg(WORD pos);
WORD ScalerReadHDEReg(void);
WORD ScalerCalcHDE(void);

void ScalerWriteOutputWidth(WORD width);
WORD ScalerReadOutputWidth(void);

void ScalerSetOutputWidthAndHeight(WORD width, WORD height);

void ScalerWriteOutputHeight(WORD height);

void ScalerWriteVDEReg(WORD pos);
WORD ScalerReadVDEReg(void);
WORD ScalerCalcVDE(void);
WORD ScalerCalcVDE2(WORD vStart, char vde_offset);
void ScalerSetVDEPosHeight(BYTE pos, WORD len);

void ScalerSetVDEMask(BYTE top, BYTE bottom);

//-------------------------------------
//HSYNC VSYNC

void ScalerSetHSyncPosLen(WORD pos, BYTE len);
void ScalerSetVSyncPosLen(WORD pos, BYTE len);


//-------------------------------------
// Freerun
WORD ScalerCalcFreerunHtotal(void);
void ScalerWriteFreerunHtotal(WORD value);

WORD ScalerCalcFreerunVtotal(void);
void ScalerWriteFreerunVtotal(WORD value);


//-------------------------------------
// Scaler Freerun
void ScalerSetFreerunManual( BYTE on );
BYTE ScalerIsFreerunManual( void );
void ScalerSetFreerunAutoManual(BYTE fAuto, BYTE fManual);
void ScalerSetMuteAutoManual(BYTE fAuto, BYTE fManual);

void ScalerSetMuteManual( BYTE on );




void ScalerSetFreerunValue(BYTE fForce);
void ScalerCheckPanelFreerunValue(void);
void ScalerSetDeOnFreerun(void);

#define ScalerReadHActive()	ScalerReadOutputWidth()
#define ScalerSetWidthAndHeight(w,h)	ScalerSetOutputWidthAndHeight(w,h)	
#define ScalerSetBlackScreen(on)	ScalerSetMuteManual(!on)

void ScalerTest(BYTE mode);

#endif