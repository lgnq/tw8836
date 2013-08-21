/**
 * @file
 * remo.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	remote controller
 *  Default function for Key Input and Remocon Input
*/
#include "config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "global.h"
#include "CPU.h"
#include "printf.h"

#include "I2C.h"
#include "Remo.h"
#include "TouchKey.h"


#define REMO_PAGE	0xbad



//=============================================================================
// REMOCON
//=============================================================================


extern BYTE	InputMode;
extern bit		OnPIP;
extern bit		CaptureMode;
extern BYTE	SavePIP;		// PIP_CVBS - start mode
extern BYTE	SaveInput;	// 0(CVBS) or 1(SVGA60) for demo
extern BYTE	SaveNum;
extern BYTE	LoadNum;
//#define CAPTURE_MAX		10



//=============================================================================
//=============================================================================





//=============================================================================
//		CheckRemo
//=============================================================================

//#define		REMO_CUSTOM			0

#ifdef DEBUG_REMO
extern BYTE RemoCapture0[];
extern BYTE RemoCapture1[];
extern BYTE RemoCapture2[];
#endif

#if defined(REMO_RC5) && defined(DEBUG_REMO)
void PrintRemoCapture(void)
{
	BYTE i,j;

	dPrintf(" RemoDataReady:%bd RemoCaptureDisable:%bd",RemoDataReady,RemoCaptureDisable);
	//dPrintf("\nSample:");
	//if(RemoCaptureDisable > 3)
	//	RemoCaptureDisable = 3;
	if(RemoCaptureDisable >= 1) {
		dPrintf("\nRemo0 Sys:%02bx Data:%02bx Auto:%02bx ", RemoSystemCode0, RemoDataCode0);
		for(i=0; i < 14;i++) {
			if(i==0) dPuts("\nstart:");
			if(i==2) dPuts(" toggle:");
			if(i==3) dPuts("\nADDR:");
			if(i==8) dPuts("\nCMD:");
			for(j=0; j < 8; j++) {
				if(RemoCapture0[i] & (1 << (j))) dPrintf("1");
				else                          dPrintf("0");
			}
			RemoCapture0[i]=0;
			dPrintf(" ");
		}
	}

	if(RemoCaptureDisable >= 2) {
		dPrintf("\nRemo1 Sys:%02bx Data:%02bx Auto:%02bx ", RemoSystemCode1, RemoDataCode1);
		//dPuts("\n");
		for(i=0; i < 14;i++) {
			if(i==0) dPuts("\nstart:");
			if(i==2) dPuts(" toggle:");
			if(i==3) dPuts("\nADDR:");
			if(i==8) dPuts("\nCMD:");
			for(j=0; j < 8; j++) {
				if(RemoCapture1[i] & (1 << (j))) dPrintf("1");
				else                          dPrintf("0");
			}
			RemoCapture1[i]=0;
			dPrintf(" ");
		}
	}

	if(RemoCaptureDisable >= 3) {
		dPrintf("\nRemo2 Sys:%02bx Data:%02bx Auto:%02bx ", RemoSystemCode2, RemoDataCode2);
		//dPuts("\n");
		for(i=0; i < 14;i++) {
			if(i==0) dPuts("\nstart:");
			if(i==2) dPuts(" toggle:");
			if(i==3) dPuts("\nADDR:");
			if(i==8) dPuts("\nCMD:");
			for(j=0; j < 8; j++) {
				if(RemoCapture2[i] & (1 << (j))) dPrintf("1");
				else                          dPrintf("0");
			}
			RemoCapture2[i]=0;
			dPrintf(" ");
		}
	}
	dPuts("\n");

	RemoCaptureDisable = 0;
}

#endif


#ifdef REMO_RC5
/**
* check remocon ready
* 
* NOTE: Use Call by Reference. USE only ON the Current BANK.
*/
BYTE IsRemoDataReady(BYTE *DataCode, BYTE *AutoKey)
{
	BYTE	togglebit;
	static BYTE	LastToggle=0;
	static BYTE AutoWait=0;
	static DWORD	LastTime = 0;

	BYTE _RemoSystemCode;
	BYTE _RemoDataCode;

	if( !RemoDataReady ) return 0;

	_RemoSystemCode = RemoSystemCode;
	_RemoDataCode = RemoDataCode;

	if ( ( RemoReceivedTime - LastTime ) > 100 ) LastToggle = 0;		// time out means newkey
	LastTime = RemoReceivedTime;

	ClearRemoTimer();			
	EnableRemoInt();
	//
	//BKFYI: DO NOT USE RemoDataReady,RemoDataCode,RemoSystemCode after here.
	//

#ifdef DEBUG_KEYREMO
	dPrintf("\r\n(CheckRemo)R-S:%02bx, R-D:%02bx", _RemoSystemCode, _RemoDataCode);
#endif

	//check start1 & start2 and 5BIT ADDRESS
	if( (_RemoSystemCode & 0xc0) != 0xc0 )			return 0;
	if( (_RemoSystemCode & 0x1f) != REMO_CUSTOM )	return 0;

	//check toggle
	togglebit = LastToggle;
	LastToggle = _RemoSystemCode & 0xE0;
	/*
	if( _RemoSystemCode & 0x20 ) LastToggle = 1;
	else LastToggle = 0;
	*/

	*AutoKey = 0;
	if( LastToggle != togglebit ) {			// new key
		AutoWait =  0;
	}
	else {								// auto key
		if(AutoWait < 4) {				// wait 4 auto key
			AutoWait++;
#ifdef DEBUG_KEYREMO
			dPrintf("  autowait:%bd",AutoWait);
#endif
			return 0;
		}
		//max AutoWait will be 4.
		*AutoKey = 1;
	}	

	//6BIT COMMAND.
	*DataCode = _RemoDataCode;

#ifdef DEBUG_KEYREMO
	dPrintf("\n':%02bx Data:%02bx Auto:%02bx ", _RemoSystemCode, _RemoDataCode, *AutoKey);
#endif
#ifdef DEBUG_REMO
	PrintRemoCapture();
#endif

	return 1;
}
#endif //..REMO_RC5

#ifdef REMO_NEC
extern DATA		BYTE	 RemoData[4];

#ifdef DEBUG_REMO_NEC
extern DATA BYTE	DebugRemoStep;
extern 	DATA BYTE	DebugRemoHcnt;
extern 	DATA BYTE   DebugRemoLcnt;
extern 	DATA BYTE   DebugRemoNum;
extern 	DATA BYTE   DebugRemoBit;
extern 	DATA BYTE   DebugRemoPhase;
extern 	DATA BYTE   DebugRemoDataReady;
#endif

/**
* check remocon is ready
*
* extern 
*	RemoData[4]
*	RemoDataReady
* @return
*	-1:success, return valid DataCode and AutoKey
*	-0:fail. invalid
*/
BYTE IsRemoDataReady(BYTE *DataCode, BYTE *AutoKey)
{
	static BYTE AutoWait=0;
	static IDATA BYTE LastDataCode=0xff;
	static IDATA BYTE repeatcnt=0;

#ifdef DEBUG_REMO_NEC
	if(DebugRemoStep) { 
		dPrintf("\r\nNEC Remo Debug Step:%02bx Hcnt:%02bx Lcnt:%02bx Num:%02bx Bit:%02bx Phase:%02bx", 
			DebugRemoStep, DebugRemoHcnt, DebugRemoLcnt, DebugRemoNum,DebugRemoBit, DebugRemoPhase);
		DebugRemoStep = 0;
	}
#endif


	if(	!RemoDataReady )	{
		return 0;
	}

	if(	RemoDataReady == 1) {
		#ifdef DEBUG_KEYREMO
		dPrintf("\r\nNEC RemoData = %02bx %02bx %02bx %02bx", RemoData[0], RemoData[1], RemoData[2], RemoData[3] );
		#endif

		if((RemoData[0] != REMO_CUSTOM1)
		|| (RemoData[1] != REMO_CUSTOM2)
		|| ((RemoData[2]+RemoData[3]) != 0xff) ) {
			//unknown address or invalid invert.

			LastDataCode = 0xff;	//reset LastDataCode

			EnableRemoInt();
			return 0;
		}
		//we have a valid NEC code.

		LastDataCode = RemoData[2];	//assume no 0xFF on RemoData[2].
		AutoWait = 0;
		repeatcnt = 0;

		*AutoKey = 0;
		*DataCode = LastDataCode;

	}
	else if( RemoDataReady == 2) {
		// we have a repeat key.
		
		if( LastDataCode==0xff ) {
			wPrintf("\r\nWARNING:NEC Auto without LastDataCode");
			EnableRemoInt();
			return 0;
		}

		AutoWait++;

		if(AutoWait < 6) {
			#ifdef DEBUG_REMO_NEC
			wPrintf("\r\nNEC AutoWait:%02bd", AutoWait);
			#endif
			EnableRemoInt();		//
			return 0;
		}

		//Q:Why we need AutoWait and repeatcnt ?. One of them can be removed.
		//1~5 : ignore.
		//6~10: pass every second time.
		//more then 10: pass every time.

		repeatcnt++;
		if( repeatcnt>=5 )
			AutoWait = 5;
		else
			AutoWait = 4;

		*AutoKey = 1;
		*DataCode = LastDataCode;
	
	} else {
		wPrintf("BUG:invalid RemoDataReady:%bd", RemoDataReady);
	}
	

	EnableRemoInt();


#ifdef DEBUG_KEYREMO
	dPrintf("\nRemo Data:%02bx Auto:%02bx ", *DataCode, *AutoKey);
	if(*AutoKey) {
		dPrintf(" AutoWait:%bd repeatcnt:%bd ", AutoWait, repeatcnt);
	}
#endif

	return 1;
}

#endif //..REMO_NEC

/**
* check remocon input
*/
BYTE CheckRemo(void)
{
	BYTE AutoKey,  _RemoDataCode;
	BYTE ret=0;

	if( IsRemoDataReady(&_RemoDataCode, &AutoKey) )	{
		ret = ActionRemo( _RemoDataCode, (BYTE)AutoKey) ;

#ifdef DEBUG_KEYREMO
		//dPrintf("\n**** Remo: %02bx, %02bx ", _RemoDataCode, (BYTE)AutoKey);
		//dPrintf("\n%02bx,%02bx ", _RemoDataCode, AutoKey);
#endif
	}

	return ret;
}


