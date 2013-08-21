//=============================================================================
// TOUCH Status
// bit[7] - press
// bit[6] - move
// bit[5] -
// bit[4] -
// 00xxNNNN
// 	bit[0] - click
// 	bit[1] - doubleclick
// 	bit[2] - longclick
// 	bit[3] - moved.(move done)
//
// 1xxxNNNN
//	bit[3]- Y
//	bit[2] - Y LEFT/RIGHT
//	bit[1] - X
//	bit[0] - X DN/UP
//=============================================================================

//status
#define	TOUCHUNKNOWN		0x00
#define	TOUCHCLICK			0x01
#define	TOUCHDOUBLECLICK	0x02
#define	TOUCHLONGCLICK		0x04
#define	TOUCHMOVED			0x40
#define TOUCHEND			0x00

#define	TOUCHPRESS			0x80
#define	TOUCHMOVE			0xC0	//0000
#define	TOUCHMOVEUP			0xC2    //xx10   
#define	TOUCHMOVEDOWN		0xC3	//xx11
#define TOUCHMOVELEFT		0xC8	//10xx
#define TOUCHMOVERIGHT		0xCC	//11xx
//press+move bit
#define TSC_PRESS			0x80
#define TSC_MOVE			0x40
#define TSC_MOVEX			0x08
#define TSC_MOVEXPLUS		0x0C
#define TSC_MOVEY			0x02
#define TSC_MOVEYPLUS		0x03			



#define	TOUCH_DOWN			0x01
#define	TOUCH_BUTTON		0x02
#define	TOUCH_CLICK			0x04
#define	TOUCH_DRAG			0x08


//TEMP
#define KEY_RIGHT			1
#define KEY_UP				2
#define KEY_MENU			3
#define KEY_DOWN			4
#define KEY_LEFT			5
#define	KEY_INPUT			6


extern BYTE	TouchStatus;


void InitAuxADC( void );	//old name was InitTouch
BYTE GetTouch2(void);
void TraceTouch( void );

void PrintTouchStatusParam(BYTE TscStatus);
void PrintTouchStatus(BYTE new, BYTE NewTscStatus);

void WaitTouchButtonUp( void );

void SetTouchStatus( BYTE ts );
void SetLastTouchStatus( BYTE ts );
void TscUpdateStatus(BYTE TscStat);


void SaveCalibDataToEE(BYTE flag);
void ReadCalibDataFromEE(void);
void PrintCalibData(void);
void CalibTouch( BYTE n );


//void UpdateTouchCalibXY(BYTE index,WORD x, WORD y);
WORD TouchGetCalibedX(BYTE index);					//friend 
WORD TouchGetCalibedY(BYTE index);					//friend 
void TouchSetCalibedXY(BYTE index, WORD x, WORD y);	//friend 

//void PrintTouchStatus(BYTE TouchStatus, WORD xpos, WORD ypos,BYTE newline);
BYTE CheckTouchStatus( WORD *xpos, WORD *ypos);

void SetTouchAuto( BYTE on );
void CheckTouch( void );
void GetTouch( void );


void SetTraceAuto( BYTE on );
BYTE SenseTouch( WORD *xpos, WORD *ypos);
void TouchDump( void );

WORD GetVeloX( void );
WORD GetVeloY( void );

void InitTscAdc(BYTE mode);
void TscDebug(void);
void TscPseudoISR(void);
