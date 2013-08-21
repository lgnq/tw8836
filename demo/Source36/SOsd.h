#ifndef __OSD_H__
#define __OSD_H__


//=============================================================================
//		FontOSD Definition
//=============================================================================


//=============================================================================
//		RLE Type Definition
//=============================================================================

struct _RLE_HEADER {
    BYTE id[2];     			// id='TW' 
    BYTE c;  					// LUT | bits/pixel
    BYTE cp; 					// compression info = data bits | count bits      
	WORD w;                		// width (little endian)
    WORD h;				   		// height (little endian)
    DWORD size;					// size (little endian)
    CODE_P BYTE *bmp;   		//
} ;
//=============================================================================
//		Animation Structure
//=============================================================================

#define MAX_ANIMATION	4

struct st_animation {
	BYTE active;
	BYTE current;
	BYTE speed;					// fixable
	BYTE vcnt;
	WORD x, y;					// fixable
	WORD w, h;					// fixable
	BYTE max;					// fixable
	WORD srcx[10], srcy[10];	// fixable
};


extern BYTE string_zoom_x;
extern BYTE string_zoom_y;


//=============================================================================
//		OSD Register Definition
//=============================================================================
//#define OSD_PAGE			0x07	  pls remove

#define OSD_ST				0x00
#define WIN0_ST				0x80
#define WIN1_ST				0xa0
#define WIN4_ST				0xc0

#define OSD_MODE			0x00
#define OSD_START			0x01
#define OSD_DATAPORT		0x02
#define OSD_SPECIAL			0x03
#define OSD_RLECTRL			0x04
#define OSD_RLEDATACNT		0x05
#define OSD_BITBLTCOLOR		0x0e

#define	OSD_SOURCECOLOR		0x40
#define	OSD_TARGETCOLOR		0x48
#define	OSD_NOWRITECOLOR	0x50

#define OSD_BITBLTSOURCE	0x60
#define OSD_BITBLTSOURCEW64	0x63
#define OSD_BITBLTSOURCEXY	0x64	
#define OSD_BITBLTSIZE		0x68
#define OSD_BITBLTDEST		0x70
#define OSD_BITBLTDESTW64	0x73
#define OSD_BITBLTDESTXY	0x74

#define OSD_LUT_WINNO		0x7a
#define OSD_LUT_INDEX		0x7b
#define OSD_LUT_ATTR		0x7c		
#define OSD_LUT_R			0x7d
#define OSD_LUT_G			0x7e
#define OSD_LUT_B			0x7f

//#define SPIOSD_PAGE			0x04  	//use CPU.h

#define	SPI_OSDWIN_ENABLE		0x00
#define SPI_OSDWIN_HZOOM		0x00

#define	SPI_OSD_ST				REG400
#define SPI_WIN0_ST				REG420
#define SPI_WIN1_ST				REG440
#define SPI_WIN2_ST				REG450
#define SPI_WIN3_ST				REG460
#define SPI_WIN4_ST				REG470
#define SPI_WIN5_ST				REG480
#define SPI_WIN6_ST				REG490
#define SPI_WIN7_ST				REG4A0
#define SPI_WIN8_ST				REG4B0


#define SPI_OSDWIN_SCREEN				0x01
#define	SPI_OSDWIN_BUFFERSTART			0x07
#define SPI_OSDWIN_BUFFERSTART_BIT		0x0A
#define SPI_OSDWIN_DISPSIZE				0x0A
#define SPI_OSDWIN_DISPOFFSET			0x0D
#define SPI_OSDWIN_ALPHA				0x0C	//430(42C+4), 44C
#define SPI_OSDWIN_LUT_PTR				0x0D	//431(42D+4), 44D
#define SPI_OSDWIN_FILLCOLOR			0x0E	//436(42E+8), 44E
#define	SPI_OSDWIN_ANIMATION			0x12    //432
//---------------------------------------------






//---------- Window Definition -------------
#define OSDWIN_ENABLE		0x00
#define OSDWIN_BUFFERSTART	0x09
#define OSDWIN_DISPOFFSET	0x0e
#define OSDWIN_SCREEN		0x01

//---------- OSD Output Path Selection -------------
#define OSD_OUT_SEL			0xf8
//=============================================================================
//		Bit Definition in OSD Registers
//=============================================================================

//=== [7:6] Write Mode
#define	CPUWRITE			0x00	// CPU
#define BLOCKTRANS			0x40	// Block Transfer
#define BLOCKFILL			0x80	// Block Fill
#define BLOCKTRANSL			0xc0	// Block Transfer with Linear

//=== [5:4] BitBlt Mode
#define BLT_SELECTIVE		0x00	// Selective No Write
#define BLT_SOURCE			0x10	// Mask from Source
#define BLT_REG				0x20	// Mask from register
#define BLT_NO				0x30	// No

//=== [3] Enable Color Conversion
#define CONV_EN				0x08	// Enable Color Conversion
#define CONV_DIS			0x00	// Disable

//=== [2] Window Pixel Unit
#define PIXEL16				0x00	// 16 bit
#define PIXEL8				0x04	// 8 bit

//=== [1:0] Source Data bit/pixel
#define SOURCE0				0x00	// Source Data = same as display
#define SOURCE1BIT			0x01	// 1 bit
#define SOURCE2BIT			0x02	// 2 bit
#define SOURCE4BIT			0x03	// 4 bit
#define SOURCE_				0xfc	// mask source data

//=============================================================================
//		OSD Window Setup
//=============================================================================
#define OSD16_YCBCR422		0x00
#define OSD16_RGB565		0x02
#define OSD16_RGB4444		0x04
#define OSD16_RGB1555		0x06



void OsdWinDisplay			(BYTE mode);

BYTE OsdWinBase 			(BYTE winno);
void OsdWinEnable 			(BYTE winno, BYTE en);
void OsdWinBufferMem 		(BYTE winno, DWORD start);
void OsdWinBufferWH 		(BYTE winno, BYTE w, BYTE h);
void OsdWinBuffer 			(BYTE winno, DWORD start, BYTE w, BYTE h);
void OsdWinBufferOffsetXY 	(BYTE winno, WORD x, WORD y);
void OsdWinScreen 			(BYTE winno, WORD x, WORD y, WORD w, WORD h);
void Osd16Format			(BYTE format);
void OsdWriteMode			(BYTE mode);
void OsdSpecialExpansion	( BYTE mode );
//=============================================================================
//
//=============================================================================
void OsdLoadDefaultLUT (BYTE winno);
//=============================================================================
//		OSD BitBlit Memory Setup
//=============================================================================
void OsdBltSourceMemoryStart(DWORD start);
void OsdBltSourceMemory   	(DWORD start, BYTE w64);
void OsdBltSourceXY       	(WORD x, WORD y);
void OsdBltSourceMemoryW64	(BYTE w64);

void OsdBltSize           	(WORD w, WORD h);

void OsdBltDestMemoryStart	(DWORD start);
void OsdBltDestMemoryW64  	(BYTE w64);
void OsdBltDestXY         	(WORD x, WORD y);

void OsdBltColor          	(WORD color);
void OsdSourceColor			( BYTE index, WORD color);
void OsdTargetColor			( BYTE index, WORD color);
void OsdNoWriteColor		( BYTE index, WORD color);

#define OSD_WMODE_CPU				0x00
#define OSD_WMODE_BLOCKTRANSFER		0x40
#define OSD_WMODE_BLOCKFILL			0x80
#define OSD_WMODE_BLOCKTRANSFERL	0xc0









//void OsdWriteMode(BYTE mode);
void OsdStart(BYTE en);
BYTE OsdBusy(void);
	
//=============================================================================
//
//=============================================================================
void OsdLoadBmpXY            ( struct _RLE_HEADER *p, WORD sx, sy );
void OsdLoadBmpXYFromSPI     ( struct _RLE_HEADER *p, DWORD spiaddr, WORD sx, WORD sy );
void OsdLoadBmpLinear        ( struct _RLE_HEADER *p, DWORD ddraddr );
//void OsdLoadBmpLinearFromSPI0( BYTE wbits, struct _RLE_HEADER *p, DWORD spiaddr, DWORD ddraddr );
void OsdLoadBmpLinearFromSPI ( struct _RLE_HEADER *p, DWORD spiaddr, DWORD ddraddr );
void OsdLoadTransBmpXYFromSPI ( struct _RLE_HEADER *p, DWORD spiaddr, WORD sx, WORD sy  );
void OsdLoadFont             ( BYTE wbits );
void OsdLoadFontMV           ( BYTE wbits );
void OsdLoadFontTransparent	 ( BYTE wbits ) ;
void Osd16bitTransparent	 ( BYTE wbits );
void OsdDisplayString        ( BYTE kind, WORD x, WORD y, char *str, BYTE fore, BYTE back );
void OsdDisplayStringT       ( BYTE kind, WORD x, WORD y, char *str, BYTE fore );
void OsdDisplayStringLinear  ( WORD x, WORD y, char *str, BYTE fore, BYTE back );
void OsdDisplayStringTLinear ( WORD x, WORD y, char *str, BYTE fore );

void OsdLoadLUT              ( BYTE winno, BYTE *ptr );
void OsdLoadLUT16            ( BYTE winno, BYTE *ptr );
void OsdLoadTranparentLUT	 (BYTE winno, BYTE red, BYTE green, BYTE blue);
void OsdLoadTranparentLUT2	 (BYTE winno, BYTE red, BYTE green, BYTE blue, BYTE cred, BYTE cgreen, BYTE cblue);
void OsdWaitDisplayBlank     ( BYTE cnt );
void OsdWaitOSDBlank		 (BYTE cnt);
void OsdWaitWindowBlank		 (BYTE winno, BYTE cnt);
void OsdBlockFill            ( WORD dx, WORD dy, WORD w, WORD h, WORD color );
void OsdBlockTransfer        ( WORD sx, WORD sy, WORD dx, WORD dy, WORD w, WORD h );
void OsdBlockTransferLinear  ( DWORD addr, WORD dx, WORD dy, WORD w, WORD h );

void OsdMirror( BYTE on );
void OsdFlip( BYTE on );

void Animation(void);

//*****************************************************************************
//				Initialize OSD
//*****************************************************************************
void OsdInit(BYTE winno);



//============================
//SpiOSD
//============================
void SOsdWinBuffClean(BYTE hw);
void SOsdWinBuffWrite2Hw(BYTE start, BYTE end);


void SpiOsdSetDeValue(void);
void SpiOsdEnable(BYTE en);
BYTE SpiOsdIsOn(void);


void SpiOsdWinHWEnable				(BYTE winno, BYTE en);
BYTE SpiOsdWinIsOn(BYTE winno);
void SpiOsdWinBuffEnable			(BYTE winno, BYTE en);
void SpiOsdWinHWOffAll(BYTE wait);
void SpiOsdWinBuffSynchEnable(void);
void SpiOsdWinBuffOffAll(void);
void SpiOsdWinHWOff(BYTE start, BYTE end);


void SpiOsdWinImageLoc		(BYTE winno, DWORD start);
void SpiOsdWinImageLocBit	(BYTE winno, BYTE start);
//void SpiOsdWinImageSizeW	(BYTE winno, WORD w);
void SpiOsdWinImageSizeWH 	(BYTE winno, WORD x, WORD y);
void SpiOsdWinScreen		(BYTE winno, WORD x, WORD y, WORD w, WORD h);
void SpiOsdWinLutOffset		(BYTE winno, WORD table_offset );
void SpiOsdWinPixelWidth	(BYTE winno, BYTE bpp);
void SpiOsdWinFillColor		(BYTE winno, BYTE color );
void SpiOsdWinGlobalAlpha	(BYTE winno, BYTE alpha );
void SpiOsdWinPixelAlpha	(BYTE winno, BYTE alpha );

void SpiOsdWin0SetPriority	(BYTE high);
void SpiOsdWin0Animation	(BYTE mode, BYTE FrameH, BYTE FrameV, BYTE Duration);
void SpiOsdWin0ImageOffsetXY(WORD x, WORD y);

#define LUTTYPE_BYTE	1	//LUTS
#define LUTTYPE_ADDR	0
//void SpiOsdIoLoadLUT(BYTE winno, BYTE type, WORD LutOffset, WORD size, DWORD spiaddr);
void SpiOsdLoadLUT(BYTE winno, BYTE type, WORD LutOffset, WORD size, DWORD address, BYTE alpha);
//void SpiOsdPixelAlphaAttr(BYTE winno, WORD lutloc, BYTE value);

void SpiOsdRlcReg(BYTE winno,BYTE dcnt, BYTE ccnt);
void SpiOsdDisableRlcReg(BYTE winno);


//call from main.
void InitLogo(void);

void SOsdHwBuffClean(void);
void SOsdHwBuffSetLut(BYTE win, /*BYTE type,*/  WORD LutOffset, WORD size, DWORD address);
void SOsdHwBuffSetRle(BYTE win, BYTE bpp, BYTE count);
void SOsdHwBuffSetAlpha(BYTE win, WORD alpha);
void SOsdHwBuffWrite2Hw(void);


#endif // __OSD_H__