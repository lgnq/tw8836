/**
 * @file
 * TouchKey.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	Touch and Keypad
*/
//*****************************************************************************
//
//  ADC(or AUX)
//	KeyPad
//	Touch

#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "Global.h"
#include "CPU.h"
#include "Printf.h"
#include "util.h"
#include "Monitor.h"

#include "I2C.h"
#include "spi.h"

#include "SOsd.h"
#include "FOsd.h"
#include "TouchKey.h"
#include "EEPROM.h"
//#include "SOsdMenu.h"

#ifdef DEBUG_TOUCH_SW
	#define TscPrintf	Printf
	#define TscPuts		Puts	
	#define dTscPrintf	dPrintf
	#define dTscPuts	dPuts	
#else
	#define TscPrintf	nullFn
	#define TscPuts		nullFn	
	#define dTscPrintf	nullFn
	#define dTscPuts	nullFn	
#endif

#define ADC_MODE_X		0
#define ADC_MODE_Z1		1
#define ADC_MODE_Z2		2
#define ADC_MODE_Y		3
#define ADC_MODE_AUX0	4
#define ADC_MODE_AUX1	5
#define ADC_MODE_AUX2	6
#define ADC_MODE_AUX3	7


WORD 	OldPosX, OldPosY;
BYTE	TouchStatus, LastTouchStatus;
DWORD	veloX, veloY;

DWORD TscTimeStart, TscTimeEnd, TscTimeLastEnd;
BYTE TscHwReady=0;


//#define MOVE_MIN	20
#define TSC_MOVE_MIN_THRESHOLD		10		//5:NG
#define TSC_LONGCLICK_THRESHOLD		1000	//10sec
#define TSC_DCLICK_THRESHOLD		100		//1sec
#define TSC_PRESS_RECOVER_THRESHOLD	100		//1sec
#define TSC_Z_THRESHOLD				220
#define TSC_MOVED_THRESHOLD 		50		//500msec


#define		RES_X	800
#define 	RES_Y	480

BYTE	TouchStatus;
WORD	TouchX, TouchY, TouchZ1, TouchZ2;
WORD 	PosX, PosY, StartX, StartY;
WORD	TouchLeft = 200;   TouchRight = 3900; 
WORD	TouchTop = 3600; TouchBottom = 340;  
WORD	TouchCenterX = 2100; TouchCenterY = 2000;
bit		TouchAuto = 0, TraceAuto = 0;
bit		tsc_debug = 0;

bit 	TouchPressedOld=0;
BYTE	TouchChangedOld=0;


//=============================================================================
//		AUX 
//=============================================================================

//internal
#ifdef SUPPORT_DIPSW
WORD GetAUX( BYTE channel )
{
	WORD	ADC;
	BYTE	page;

	ReadTW88Page(page);
	WriteTW88Page(PAGE0_TOUCH );
	WriteTW88(REG0B4, 0x03 );						// clock sel div16
#if 0
	WriteTW88(REG0B0, 0x0c + channel );				// write Start, erase Ready,
	delay1ms(1);
	WriteTW88(REG0B0, 0x24 + channel );				// write Start, erase Ready,
	while (( ReadTW88(REG0B0 ) & 8 ) != 8 ) ;
	ADC = ( ReadTW88(REG0B2 ) << 4 ) + ReadTW88(REG0B3 ) ;
	//dPrintf( "\nAUX Value: %d", ADC );
#else
	ADC = _TscGetAdcDataOutput(0x40+ channel);
#endif
	WriteTW88Page(page );
	return ( ADC );
}
#endif

//internal
//#ifdef SUPPORT_ANALOG_SENSOR_NEED_VERIFY
//BYTE GetAUXBYTE( BYTE channel )
//{
//	BYTE	ADC;
//	WORD	wADC;
//	BYTE	page;
//
//	ReadTW88Page(page);
//	WriteTW88Page(PAGE0_TOUCH );
//	WriteTW88(REG0B4, 0x03 );						// clock sel div16
//	wADC = _TscGetAdcDataOutput(0x40+ channel);
//	ADC = (wADC >> 4);	//only MSB 8 bits[11:4]
//	WriteTW88Page(page );
//	return ( ADC );
//}
//#endif

#if 0
WORD GetAuxWord(BYTE mode)
{...}
#endif

//=============================================================================
//DIP
//=============================================================================
#ifdef SUPPORT_DIPSW
// code WORD DIP_MEASURE[] = { 0, 282, 525, 745, 891, 1056, 1223, 1369, 1454, 1574, 1680, 1781, 1851, 1936, 2014, 2094 }; real measure
code WORD DIP_MEASURE[] = { 141, 403, 635, 818, 973, 1138, 1302, 1414, 1514, 1630, 1731, 1815, 1980, 2050 };

BYTE	DipSW( void )
{
	WORD	i;
	BYTE	j;

	i = GetAUX( 1 );

	j = 0;
	for ( j=0; j<15; j++ ) {
		if ( i < DIP_MEASURE[j] ) return (j);
	}
	return ( 15 );
}
#endif

//===========================================
// KeyPad
//===========================================
bit			RepeatKey, KeyReady;
DATA BYTE 	Key;

DATA BYTE	keytic;

#define KEYPAD_INPUT	3

//#ifdef SUPPORT_ANALOG_SENSOR


//#define KEY_RIGHT			1
//#define KEY_UP			2
//#define KEY_MENU			3
//#define KEY_DOWN			4
//#define KEY_LEFT			5
//#define	KEY_INPUT		6

// key sampleing values
//  key name    average     range
//  ----------	-------	-----------
//	input key:	0x0200	0x100~0x2FF
//	menu key:	0x0400	0x300~0x4FF
//	up key:		0x0600	0x500~0x6FF
//	down key:	0x0800	0x700~0x9FF
//	right key:	0x0A00  0x900~0xAFF
//  left key:   0x0C00  0xB00~0xCFF
//------------------------------
BYTE	KeyPadMap[] = { 0, KEY_INPUT, KEY_MENU, KEY_UP, KEY_DOWN, KEY_RIGHT, KEY_LEFT, 0 };
/**
* read keypad value
*/
BYTE ReadKeyPad(void) 
{
	WORD i, j;
	WORD min, max;
	BYTE temp;
	WORD key_in;

	i = CpuAUX3;
	temp = CpuAUX3_Changed;
	if (i < 100)
		return (0);

	key_in = i;

	min = i; max = i;
	for (j=0; j<9; j++)
	{
		while ( temp == CpuAUX3_Changed ) ;				// wait new measurement
		temp = CpuAUX3_Changed;
		i = CpuAUX3;
		if (i < 100)
			return (0);
		key_in += i;
		if (i < min)
			min = i;
		else if (i > max)
			max = i;
	}
	key_in -= min;
	key_in -= max;

	key_in /= 8;			// divided by 8 for averaging
	temp = (key_in >> 8);

//	dPrintf("\r\nKeyIn ADC Value is: 0x%4x, last: 0x%04x", (WORD)key_in, i );
	temp++;
	temp /= 2;

	return (KeyPadMap[temp]);
}
//#endif

/**
* get keypad input
*
* extern 
*	Key: keep pressed key value.
*	keytic: key tic counter
*	KeyReady
*	RepeatKey
*/
BYTE GetKey(BYTE repeat)
{
	BYTE i;

//#ifdef SUPPORT_ANALOG_SENSOR
	i = repeat;
	i = ReadKeyPad();

	if (i == 0)
	{
		RepeatKey = 0;
		Key       = 0;
		KeyReady  = 0;
		keytic    = 0;
		return (0);
	}

	if (Key == 0)			// first pressed?
	{
		Key = i;
		keytic++;
		return (i);
	}
	else if (i != Key)		//diff key value when Key has a value. 		
	{
		RepeatKey = 0;
		KeyReady  = 0;
		keytic    = 0;
		return (0);
	}
	else						// same key pressed check for repeat function
	{
		keytic++;
		if (keytic < 20)
		{
			return (0);
		}
		else
		{
			RepeatKey = 1;
			KeyReady  = 1;
			keytic    = 18;
			return (i);
		}
	}
//#else
//	i = repeat;
//	keytic = 0;
//	return 0;
//#endif
}

//===========================================
// TOUCH
//===========================================


#ifdef SUPPORT_TOUCH
   //extern
//	TraceAuto
//	TouchAuto
/**
* set TouchAuto flag. only for test
*/
void SetTouchAuto( BYTE on )
{
	TraceAuto = 0;
	if ( on )
		TouchAuto = 1;
	else
		TouchAuto = 0;
}
/**
* set traceauto flag. only for test
*/
void SetTraceAuto( BYTE on )
{
	TouchAuto = 0;
	if ( on )
		TraceAuto = 1;
	else
		TraceAuto = 0;
}
#endif



/*
	  +-----------------+---------------------+
      | 0(80x48)        |        1(720x48)	  |
	  |					|					  |
	  +--------------4(400x240)---------------+
	  |					|					  |
      |  3(80x432)      |        2(720x432)	  |
      +-----------------+---------------------+
*/

#define TSC_ZONE_TOPLEFT		0
#define TSC_ZONE_TOPRIGHT		1
#define TSC_ZONE_BOTTOMRIGHT	2
#define TSC_ZONE_BOTTOMLEFT		3
#define TSC_ZONE_CENTER			4

	 WORD	CalibDataX[]= 		{ 80,   720,  720,  80,  400  };
	 WORD	CalibDataY[]= 		{ 48,   48,   432,  432, 240  };

code WORD	Def_TouchCalibX[] = { 478,  3583, 3580, 496, 2029 };
code WORD	Def_TouchCalibY[] = { 3432, 3421, 593,  604, 2008 };
	 WORD	TouchCalibX[] = 	{ 478,  3583, 3580, 496, 2029 };
	 WORD	TouchCalibY[] = 	{ 3432, 3421, 593,  604, 2008 };
	 WORD	Temp_TouchCalibX[]= { 478,  3583, 3580, 496, 2029 };
	 WORD	Temp_TouchCalibY[]= { 3432, 3421, 593,  604, 2008 };

//=============================================================================
//		_TscGetScreenPos 
//		ScrrenPosX = (TouchX - CalibDataX[4]) / (CalibData[n] - CalibDataX[4]) / (TouchCalibX[n] - TouchCalibX[4])
//=============================================================================
#ifdef SUPPORT_TOUCH
/**
* update PosX,PosY.
*
* input
*	TouchX,TouchY
* output
*	PosX,PosY
*/
void _TscGetScreenPos( void )
{
	DWORD calcX, calcY;

	if ( TouchX < TouchCalibX[4] ) {				// center of 
		if ( TouchY > TouchCalibY[4] ) {
			// GetZone0;
			//calc = (TouchCalibX[4] - TouchX) / (TouchCalibX[4] - TouchCalibX[0]) / (CalibDataX[4] - CalibDataX[0]);
			calcX = TouchCalibX[4] - TouchX;
			calcX *= (DWORD)( CalibDataX[4] - CalibDataX[0] );
			calcX /= (DWORD)( TouchCalibX[4] - TouchCalibX[0] );
			if ( calcX >= CalibDataX[4] )	PosX = 0;
			else							PosX = CalibDataX[4] - calcX;

			calcY = TouchY - TouchCalibY[4];
			calcY *= ( CalibDataY[4] - CalibDataY[0] );
			calcY /= ( TouchCalibY[0] - TouchCalibY[4] );
			if ( calcY >= CalibDataY[4] )	PosY = 0;
			else							PosY = CalibDataY[4] - calcY;
		}
		else {
			//GetZone3
			//calc = (TouchCalibX[4] - TouchX) / (TouchCalibX[4] - TouchCalibX[3]) / (CalibDataX[4] - CalibDataX[3]);
			calcX = TouchCalibX[4] - TouchX;
			calcX *= (DWORD)( CalibDataX[4] - CalibDataX[3] );
			calcX /= (DWORD)( TouchCalibX[4] - TouchCalibX[3] );
			if ( calcX >= CalibDataX[4] )	PosX = 0;
			else							PosX = CalibDataX[4] - calcX;

			calcY = TouchCalibY[4] - TouchY;
			calcY *= (DWORD)( CalibDataY[3] - CalibDataY[4] );
			calcY /= (DWORD)( TouchCalibY[4] - TouchCalibY[3] );
			if ( calcY >= CalibDataY[4] )	PosY = RES_Y - 1;
			else							PosY = CalibDataY[4] + calcY;
		}
	}
	else {   
		if ( TouchY > TouchCalibY[4] )  {
			//GetZone1
			calcX = TouchX - TouchCalibX[4];
			calcX *= ( CalibDataX[1] - CalibDataX[4] );
			calcX /= ( TouchCalibX[1] - TouchCalibX[4] );
			if ( calcX >= CalibDataX[4] )	PosX = RES_X - 1;
			else							PosX = CalibDataX[4] + calcX;

			calcY = TouchY - TouchCalibY[4];
			calcY *= ( CalibDataY[4] - CalibDataY[1] );
			calcY /= ( TouchCalibY[1] - TouchCalibY[4] );
			if ( calcY >= CalibDataY[4] )	PosY = 0;
			else							PosY = CalibDataY[4] - calcY;
		}
		else  {
			//GetZone2
			calcX = TouchX - TouchCalibX[4];
			calcX *= ( CalibDataX[2] - CalibDataX[4] );
			calcX /= ( TouchCalibX[2] - TouchCalibX[4] );
			if ( calcX >= CalibDataX[4] )	PosX = RES_X - 1;
			else							PosX = CalibDataX[4] + calcX;

			calcY = TouchCalibY[4] - TouchY;
			calcY *= ( CalibDataY[2] - CalibDataY[4] );
			calcY /= ( TouchCalibY[4] - TouchCalibY[2] );
			if ( calcY >= CalibDataY[4] )	PosY = RES_Y - 1;
			else							PosY = CalibDataY[4] + calcY;
		}
	}
	//dPrintf("\n_TscGetScreenPos XY:%dx%d from Touch XY:%dx%d",PosX,PosY, TouchX, TouchY);	//WORD
}
#endif

//=============================================================================
//	 
//=============================================================================
/**
* init Touch
*/
void InitAuxADC( void )
{
	BYTE val;
	dPuts("\nInitTouch");
#ifdef SUPPORT_TOUCH
	TscTimeStart = TscTimeEnd = TscTimeLastEnd = 0;;

	//read CalibDataX[] and CalibDataY[] from EEPROM.
	ReadCalibDataFromEE();


	WriteTW88Page(PAGE0_TOUCH );
	CpuTouchStep = 0;
	WriteTW88(REG0B0, 0x02 );				//power up. with Z2 measure
	WriteTW88(REG0B1, 0xF8 );				//TODO:Touch Ready & Touch Pen is not implemented yet.
											// 

	//--------------------------------------------
	//Check connector
	WriteTW88(REG0B4, 0x02 ); 
	WriteTW88(REG0B0, 0x20 | ReadTW88(REG0B0) );	//need a start command
	delay1ms(1);	//need more then 1CLK cycle.
	val = ReadTW88(REG0B2);
	if( val < 0x80  ) {
		dPrintf("--FAIL.%bx",val);
		// do not power down. Still you need a keypad.
		TscHwReady = 0;
	}
	else {
		dPrintf("--OK.%bx",val);
		TscHwReady = 1;
	}
	WriteTW88(REG0B0, ~0x20 & ReadTW88(REG0B0) );



#if 1	//120323 Touch Spec. can support from div8~64 : 4MHz~320K.
	WriteTW88(REG0B4, 0x0A );				// div 8, rsel=10k, continuous sensing mode
#else
	WriteTW88(REG0B4, 0x0C );				// div 32, rsel=10k, continuous sensing mode
#endif


	SFR_EINT6 = 0;
	CpuTouchPressed = 0;

	SFR_TL1 = SFR_TH1;
	CpuTouchSkipCount = 0;
	SFR_ET1 = 1;


	SetTouchStatus(TOUCHEND);
	SetLastTouchStatus(TOUCHEND);
#else
	dPuts("-removed");
#endif
}


//return
//	success: if need an Action
#ifdef SUPPORT_TOUCH
/**
* get Touch2
*/
BYTE GetTouch2(void)
{
	BYTE ret;
	short movX,movY;
	BYTE TC;		 //Touch change counter
	bit	 TP;		 //pressed status
	DWORD dt_start,dt_end;
	DWORD TscTimeCurr;
	BYTE new_status;

	//update value 
	SFR_EA = 0;
	TC = CpuTouchChanged;
	TP = CpuTouchPressed;
	TouchX = CpuTouchX;
	TouchY = CpuTouchY;
	SFR_EA = 1;

#if 0
	if(CpuTouchSkipCount)
		Printf("\nCpuTouchSkipCount:%d", CpuTouchSkipCount);
	//Printf("\nGetTouch2() TC:%bx TP:%bx step:%bx ",TC, TP ? 1:0, CpuTouchStep);
	if(TP) {
		Printf("\nTP:%dx%d", TouchX,TouchY);
	}
	//else {
	//	WriteTW88Page(0);
	//	Printf(" %02bx%01bx", ReadTW88(REG0B2), ReadTW88(REG0B3) & 0x0F);
	//	//#define ReadTscData(TscData) TscData = ReadTW88(REG0B2); TscData <<= 4; TscData += ReadTW88(REG0B3) 
	//
	//}
#endif

	if ( TouchChangedOld == TC ) {
		if(TouchStatus==TOUCHMOVED && ((TscTimeEnd + TSC_MOVED_THRESHOLD) < SystemClock)) {			//500msec delay
			dTscPrintf("\nTouch TOUCHMOVED Action dt:%ld", SystemClock - TscTimeEnd);
			return 1;			
		}
		return 0;			// no measurement
	}

	ret = 0;
	TscTimeCurr = SystemClock;

	if(TouchPressedOld) {
		if(TP) {
//-----------
LABEL_TP11: /* Pressed=>Pressed */
//-----------
			new_status = TSC_PRESS; 
			_TscGetScreenPos();
			//--------------------
			//_TscGetDirection();
			//--------------------
			movX = PosX - OldPosX;
			movY = PosY - OldPosY;
			if(TWabsShort(movX) <= TSC_MOVE_MIN_THRESHOLD) {
				if(TouchStatus >= TOUCHMOVE)
					new_status |= TSC_MOVE;	
			}
			else {
				new_status |= TSC_MOVE;
				if(movX > 0) new_status |= TSC_MOVEXPLUS;	//right
				else         new_status |= TSC_MOVEX;		//left
			}
			if(TWabsShort(movY) <= TSC_MOVE_MIN_THRESHOLD) {
				if(TouchStatus >= TOUCHMOVE)
					new_status |= TSC_MOVE;	
			}
			else {
				new_status |= TSC_MOVE;
				if(movY > 0) new_status |= TSC_MOVEYPLUS;	//down
				else         new_status |= TSC_MOVEY;		//up
			}
			if(TouchStatus == new_status) {
				dTscPrintf("\nTSC keep Stat:%bx", TouchStatus);
			}
			else {
				PrintTouchStatusParam(new_status);
				PrintTouchStatus(0,new_status);
				TscUpdateStatus(new_status);
			}
			dTscPrintf(" xypos=%dx%d",PosX, PosY);
			dTscPrintf(" z:%d(0x%x-0x%x)",CpuZ2-CpuZ1,CpuZ2,CpuZ1);

			if(new_status & (TSC_MOVEX | TSC_MOVEY)) { //move X or Y
				OldPosX = PosX;
				OldPosY = PosY;
			}
			//...
			ret = 1;
		}
		else {
//-----------
//LABEL_TP10: /*Press=>Detached */
//-----------
			//NOTE:update TscTimeEnd

			dt_start = TscTimeCurr - TscTimeStart;	//pressed position(previous)
			dt_end   = TscTimeCurr - TscTimeEnd;	//detached position(previous)


			if(LastTouchStatus>=TOUCHMOVE) {
				PrintTouchStatus(1, TOUCHMOVED);
				TscUpdateStatus(TOUCHMOVED);
				ret=0;
			}
			else if(dt_start > TSC_LONGCLICK_THRESHOLD) {	//more then 10sec
				PrintTouchStatus(1, TOUCHLONGCLICK);
				TscUpdateStatus(TOUCHLONGCLICK);
				ret = 1;
			}
			else if(dt_end < TSC_DCLICK_THRESHOLD) {
				if(TouchStatus >= TOUCHMOVE) {
					PrintTouchStatus(1, TOUCHMOVED);
					TscUpdateStatus(TOUCHMOVED);	 dTscPuts("(DCLK->MOVED)");
					ret = 0;
				}
				else { 
					PrintTouchStatus(1, TOUCHDOUBLECLICK);
					TscUpdateStatus(TOUCHDOUBLECLICK);
					ret = 1;
				}
			}
			else {
				if(TouchStatus >= TOUCHMOVE) {
					PrintTouchStatus(1, TOUCHMOVED);
					TscUpdateStatus(TOUCHMOVED);	 dTscPuts("(CLK->MOVED)");
					ret = 0;
				}
				else {
					PrintTouchStatus(1, TOUCHCLICK); 
					TscUpdateStatus(TOUCHCLICK);
					ret = 1;
				}
			}
			dTscPrintf(" xypos=%dx%d",PosX, PosY);
			dTscPrintf(" z:%d(0x%x-0x%x)",CpuZ2-CpuZ1,CpuZ2);
			dTscPrintf(" dt_start:%ld dt_end:%ld",dt_start,dt_end);

			if(TouchStatus == TOUCHMOVED) {
				movX = OldPosX - StartX;
				movX = TWabsShort(movX);
				movY = OldPosY - StartY;
				movY = TWabsShort(movY);

				veloX = 1000;
				veloX *= movX;
				veloX /= dt_start;
				veloY = 1000;
				veloY *= movY;
				veloY /= dt_start;
				dTscPrintf(" Velocity X:%ld Y:%ld", veloX,veloY );
			}
			TscTimeEnd = TscTimeCurr;
		}
	}
	else {
		if(TP) {
//-----------
//LABEL_TP01: /*Detached=>Pressed.*/
//-----------
			//NOTE:update TscTimeStart. StartX,StartY
			dt_end   = TscTimeCurr - TscTimeEnd;	//detached position(previous)

			if(TouchStatus==TOUCHMOVED) {
				//if TP0_MOVED, TouchStatus=TOUCHMOVE and then goto LABEL_TP11.
				//Touch is pressed before TSC_MOVED_THRESHOLD. It will ignore the previous unpress state.
				PrintTouchStatus(1,TOUCHMOVE); dTscPuts(" RECOVER1 ");
				TscUpdateStatus(TOUCHMOVE);
				goto LABEL_TP11;
			}
			else if(LastTouchStatus==TOUCHMOVED && dt_end < TSC_PRESS_RECOVER_THRESHOLD) {	 //assume TouchStatus==TOUCHEND
				PrintTouchStatus(1,TOUCHMOVE); dTscPuts(" RECOVER2 ");
				TscUpdateStatus(TOUCHMOVE);
				goto LABEL_TP11;
			}
			else {
				PrintTouchStatus(1,TOUCHPRESS);
				TscUpdateStatus(TOUCHPRESS);
				_TscGetScreenPos();
				OldPosX = StartX = PosX;
				OldPosY = StartY = PosY;
				TscTimeLastEnd = TscTimeEnd; //NOTE
				TscTimeStart = TscTimeCurr;
				dTscPrintf(" xypos=%dx%d",PosX, PosY);
				dTscPrintf(" z:%d(0x%x-0x%x)",CpuZ2-CpuZ1,CpuZ2);	
			}
			ret = 1;
		}
		else {
//-----------
//LABEL_TP00: /*Deteched=>Deteched */
//-----------
			//something happen.But, ignore.
			//dTscPrintf("\nTouch TC:%bd dt_start:%ld dt_end:%ld",
			//	TC > TouchChangedOld ? TC-TouchChangedOld : 0x100 - TC + TouchChangedOld), dt_start,dt_end);
		}
	}
	TouchChangedOld = TC;
	TouchPressedOld = TP;

	return ret;
}
#endif

//=============================================================================
//		TraceTouch 
//=============================================================================
#ifdef SUPPORT_TOUCH
/**
* trace Touch. only for debug
*/
void TraceTouch( void )
{
	bit TouchPressedOld;

	SFR_EA = 0;
	TouchPressedOld = CpuTouchPressed;
	TouchX = CpuTouchX;
	TouchY = CpuTouchY;
	SFR_EA = 1;

	//CheckTouch();

	if ( TouchPressedOld ) {   		// before it pressed with start
		if ( CpuTouchPressed ) {
			_TscGetScreenPos();
			FOsdWinScreenXY( 0, PosX, PosY);
			FOsdWinScreenWH( 0, 1, 1 );
		}
		else {
			//Prompt();
		}
	}
	else {
		if ( CpuTouchPressed ) {
			_TscGetScreenPos();
			FOsdWinScreenXY( 0, PosX, PosY);
			FOsdWinScreenWH( 0, 1, 1 );
		}
		else {
		}
	}
}
#endif



//=============================================================================
//		Print Status 
//=============================================================================
#if defined(SUPPORT_TOUCH)
/**
* print TouchStatus
*/
void PrintTouchStatusParam(BYTE TscStatus)
{
#if !defined(DEBUG_TOUCH_SW)
	BYTE temp = TscStatus;
#else
 	dPuts("\nTSC ");
	if(TscStatus & 0x80) {
		dPuts("PRESS ");
		if(TscStatus & 0x40) {
			dPuts("MOVE ");
			if(TscStatus & 0x02) {
				if(	TscStatus & 0x01)	dPuts("DN ");
				else                    dPuts("UP ");
			} 
			if(TscStatus & 0x08) {
				if(	TscStatus & 0x04)	dPuts("RIGHT ");
				else                    dPuts("LEFT ");
			} 
		}
	}
	else {
		if(TscStatus & 0x40) {
			dPuts("MOVED ");
			if(TscStatus & 0x02) {
				if(	TscStatus & 0x01)	dPuts("DN ");
				else                    dPuts("UP ");
			} 
			if(TscStatus & 0x08) {
				if(	TscStatus & 0x04)	dPuts("RIGHT ");
				else                    dPuts("LEFT ");
			} 
		}	
		else {
			switch(TscStatus & 0x07) {
			case 0: dPuts("END "); break;
			case 1: dPuts("CLK "); break;
			case 2: dPuts("DCLK "); break;
			case 4: dPuts("LONGCLK "); break;
			default: dPrintf("unknown:%bx ",TscStatus); break;
			}
		}
	}
#endif
}
/**
* print touch status
*/
void PrintTouchStatus(BYTE new, BYTE NewTscStatus)
{
#if !defined(DEBUG_TOUCH_SW)
	BYTE temp;
	temp = new;
	temp = NewTscStatus;
#else
	if(new) dPuts("\nTSC stat:");
	dPrintf("%bx=>%bx=>%bx ",	LastTouchStatus,TouchStatus,NewTscStatus);
#endif
}
#endif


//=============================================================================
//		WaitTouchButtonUp 
//=============================================================================
#ifdef SUPPORT_TOUCH
/**
* wait until touch is detached
*/
void WaitTouchButtonUp( void )
{
	dPuts("\nWaitTouchButtonUp start");
	do {
		GetTouch2();
		delay1ms(10);
	} while (( TouchStatus != TOUCHCLICK ) 
	      && ( TouchStatus != TOUCHDOUBLECLICK ) 
		  && ( TouchStatus != TOUCHLONGCLICK )
		  && ( TouchStatus != TOUCHMOVED )
		  && ( TouchStatus != TOUCHEND ));
	dPuts("===>END");
}
#endif
//=============================================================================
//		SetTouchStatus 
//		SetTouchStatus 
//=============================================================================
#ifdef SUPPORT_TOUCH
/**
* set TouchStatus
*/
void SetTouchStatus( BYTE ts )
{
	dTscPrintf("\nSetTouchStatus(%bx)",ts);
	if(TouchStatus != ts) {
		LastTouchStatus = TouchStatus;
		TouchStatus = ts;
	}
}
/**
* update Touch Status
*/
void TscUpdateStatus(BYTE TscStat)
{
	//dTscPrintf("\nTouch Stat:%bx->%bx->%bx",LastTouchStatus,TouchStatus,TscStat);  
	if(TouchStatus != TscStat) {
		LastTouchStatus = TouchStatus;
		TouchStatus = TscStat;	
	}
}
/**
* set Last Touch Status
*/
void SetLastTouchStatus( BYTE ts )
{
	dTscPrintf("\nSetLastTouchStatus(%bd)",ts);
	LastTouchStatus = ts;
}
#endif
//=============================================================================
//		CalibTouch 
//=============================================================================

//@param
// flag		
//	0:update TouchCalibX/TouchCalibY from Default
//	1:update TouchCalibX/TouchCalibY from temp
//	other:normal.  
#ifdef SUPPORT_TOUCH
/**
* save the calibrated touch data to EEPROM
*/
void SaveCalibDataToEE(BYTE flag)
{
	BYTE i;
	WORD value;

	if(flag==0) {
		for(i=0; i < 5; i++) {
			TouchCalibX[i] = Def_TouchCalibX[i];
			TouchCalibY[i] = Def_TouchCalibY[i];
		}
	}
	else if(flag==1) {
		for(i=0; i < 5; i++) {
			TouchCalibX[i] = Temp_TouchCalibX[i];
			TouchCalibY[i] = Temp_TouchCalibY[i];
		}
	}

	for(i=0; i < 5; i++) {
		value = TouchCalibX[i];
		EE_Write( EEP_TOUCH_CALIB_X + i*2, value >> 8);
		EE_Write( EEP_TOUCH_CALIB_X + i*2+1, (BYTE)value);
	}
	for(i=0; i < 5; i++) {
		value = TouchCalibY[i];
		EE_Write( EEP_TOUCH_CALIB_Y + i*2, value >> 8);
		EE_Write( EEP_TOUCH_CALIB_Y + i*2+1, (BYTE)value);
	}
}
/**
* read calibrated touch data from EEPROM
*/
void ReadCalibDataFromEE(void)
{
	BYTE i;
	WORD value;

	for(i=0; i < 5; i++) {
		value = (BYTE)EE_Read( EEP_TOUCH_CALIB_X + i*2);
		value <<= 8;
		value |= (BYTE)EE_Read( EEP_TOUCH_CALIB_X + i*2+1);
		TouchCalibX[i] = value;
	}
	for(i=0; i < 5; i++) {
		value = (BYTE)EE_Read( EEP_TOUCH_CALIB_Y + i*2);
		value <<= 8;
		value |= (BYTE)EE_Read( EEP_TOUCH_CALIB_Y + i*2+1);
		TouchCalibY[i] = value;
	}
}
#endif

#ifdef SUPPORT_TOUCH
/**
* print Calibrated data
*/
void PrintCalibData(void)
{
	BYTE i;
	WORD value;
	Puts("\nPrintCalibData");
	Puts("\ndefault:");
	for(i=0; i < 5; i++) {
		Printf("%dx%d\t",Def_TouchCalibX[i],Def_TouchCalibY[i]);
	}
	Puts("\ncurrent:");
	for(i=0; i < 5; i++) {
		Printf("%dx%d\t",TouchCalibX[i],TouchCalibY[i]);
	}
	Puts("\neeprom :");
	for(i=0; i < 5; i++) {
		//Printf("%dx%d\t",TouchCalibX[i],TouchCalibY[i]);
		value = (BYTE)EE_Read( EEP_TOUCH_CALIB_X + i*2);
		value <<= 8;
		value |= (BYTE)EE_Read( EEP_TOUCH_CALIB_X + i*2+1);
		Printf("%dx",value);
		value = (BYTE)EE_Read( EEP_TOUCH_CALIB_Y + i*2);
		value <<= 8;
		value |= (BYTE)EE_Read( EEP_TOUCH_CALIB_Y + i*2+1);
		Printf("%d\t",value);
	}
}
#endif



#ifdef SUPPORT_TOUCH
/**
* calibrate Touch
*/
void CalibTouch( BYTE n )
{
	bit TouchPressedOld;
	DWORD	CalibX, CalibY;
	WORD	count;

	dTscPrintf("\nCalibTouch(%bd)",n);

	GetTouch2();

	TouchPressedOld = CpuTouchPressed;
	CalibX = 0;
	CalibY = 0;
	count = 0;
	
	while (count < 100 /*32768*/) {
		if ( TouchChangedOld == CpuTouchChanged ) continue;
		TouchPressedOld = CpuTouchPressed;
		GetTouch2();

		if ( TouchPressedOld ) {   		// before it pressed with start
			if ( CpuTouchPressed ) {
				CalibX += TouchX;
				CalibY += TouchY;
				count++;
			}
			else {
				TouchPressedOld = CpuTouchPressed;
				dTscPrintf("\nTouch End: %d, %d", TouchX, TouchY );
				break;
			}
		}
		else {
			if ( CpuTouchPressed ) {
				dTscPrintf("\nTouch Start with: %d, %d", TouchX, TouchY );
				TouchPressedOld = CpuTouchPressed;
			}
			else {
			}
		}
	}
	CalibX /= (DWORD)count;
	CalibY /= (DWORD)count;
	dTscPrintf("\nCalibration data %bd: (%d, %d) --> (%ld, %ld)", n, TouchCalibX[n], TouchCalibY[n], CalibX, CalibY );
	Temp_TouchCalibX[n] = CalibX;
	Temp_TouchCalibY[n] = CalibY;

	dTscPrintf("====>END");

	//BKTODO: Save CalibDataX[n] and CalibDataY[n] to EEPROM
}
#endif

//=============================================================================
//		TouchDump : for checking Touch ADC character 
//=============================================================================
#ifdef SUPPORT_TOUCH
void TouchDump( void )
{
}
#endif



