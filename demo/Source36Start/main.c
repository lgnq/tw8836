/**
 * @file
 * main.c 
 * @author Harry Han
 * @author YoungHwan Bae
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	main file
 * @section DESCRIPTION
 *	- CPU : DP80390
 *	- Language: Keil C
 *  - See 'Release.txt' for firmware revision history 
 ******************************************************************************
 */

#include <intrins.h>
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
#include "SPI.h"

#include "main.h"
#include "misc.h"
#include "Remo.h"
#include "TouchKey.h"
#include "eeprom.h"
#include "e3prom.h"

#include "Settings.h"
#include "InputCtrl.h"
#include "decoder.h"
#include "Scaler.h"
#include "ImageCtrl.h"
#include "OutputCtrl.h"
#include "aRGB.h"
#include "DTV.h"
#include "measure.h"

#ifdef SUPPORT_HDMI_EP907M
#include "hdmi_ep907m.h"
#endif

#include "SOsd.h"
#include "FOsd.h"
#include "SpiFlashMap.h"
#include "SOsdMenu.h"
#include "Demo.h"
#include "Debug.H"
#include "BT656.h"

CODE BYTE TW8835_DefaultRegTable[] = {
0x00, 0x00,		//START

0xFF, 0x00,  //; Page 0
0x02, 0xC8,
0x03, 0xE8,
0x04, 0x00,
0x06, 0x06,
0x07, 0x08,
0x08, 0x86,
0x0F, 0x00,
0x1F, 0x00,
0x40, 0x00,
0x41, 0x0C,
0x42, 0x02,
0x43, 0x20,
0x44, 0xF0,
0x45, 0x20,
0x46, 0xD0,
0x47, 0x00,
0x48, 0x00,
0x4B, 0x00,
0x50, 0x00,
0x51, 0x00,
0x52, 0x00,
0x53, 0x03,
0x54, 0x00,
0x56, 0x00,
0x57, 0x00,
0x5F, 0x00,
0x60, 0x00,
0x66, 0x00,
0x67, 0x00,
0x68, 0x00,
0x69, 0x02,
0x6A, 0x20,
0x6B, 0xF0,
0x6C, 0x20,
0x6D, 0xD0,
0x6E, 0x10,
0x6F, 0x10,
0xD4, 0x00,
0xF6, 0x00,
0xF7, 0x16,
0xF8, 0x02,
0xF9, 0x00,
0xFA, 0x00,
0xFB, 0x40,
0xFC, 0x23,
0xFD, 0x34,

0xFF, 0x01,  //; Page 1
0x01, 0x68,
0x02, 0x40,
0x04, 0x00,
0x05, 0x00,
0x06, 0x03,
0x07, 0x02,
0x08, 0x15,
0x09, 0xF0,
0x0A, 0x06,
0x0B, 0xCA,
0x0C, 0xCC,
0x0D, 0x00,
0x10, 0x00,
0x11, 0x5C,
0x12, 0x11,
0x13, 0x80,
0x14, 0x80,
0x15, 0x00,
0x17, 0x30,
0x18, 0x44,
0x1A, 0x10,
0x1B, 0x00,
0x1C, 0x07,
0x1D, 0x7F,
0x1E, 0x00,
0x1F, 0x00,
0x20, 0x50,
0x21, 0x22,
0x22, 0xF0,
0x23, 0xD8,
0x24, 0xBC,
0x25, 0xB8,
0x26, 0x44,
0x27, 0x38,
0x28, 0x00,
0x29, 0x00,
0x2A, 0x78,
0x2B, 0x44,
0x2C, 0x30,
0x2D, 0x14,
0x2E, 0xA5,
0x2F, 0xE0,
0x30, 0x00,
0x31, 0x10,
0x32, 0x00,
0x33, 0x05,
0x34, 0x1A,
0x35, 0x00,
0x36, 0x03,
0x37, 0x28,
0x38, 0xAF,
0x40, 0x00,
0x41, 0x80,
0x42, 0x00,
0xC0, 0x01,
0xC1, 0xC7,
0xC2, 0xD2,
0xC3, 0x03,
0xC4, 0x5A,
0xC5, 0x00,
0xC6, 0x20,
0xC7, 0x04,
0xC8, 0x00,
0xC9, 0x06,
0xCA, 0x06,
0xCB, 0x16,
0xCC, 0x00,
0xCD, 0x54,
0xD0, 0x00,
0xD1, 0xF0,
0xD2, 0xF0,
0xD3, 0xF0,
0xD4, 0x20,
0xD5, 0x00,
0xD6, 0x10,
0xD7, 0x00,
0xD8, 0x00,
0xD9, 0x02,
0xDA, 0x80,
0xDB, 0x80,
0xDC, 0x10,
0xE0, 0x00,
0xE1, 0x05,
0xE2, 0x59,
0xE3, 0x07,
0xE4, 0x33,
0xE5, 0x33,
0xE6, 0x00,
0xE7, 0x2A,
0xE8, 0x2F,
0xE9, 0x00,
0xEA, 0x03,
0xF6, 0x30,
0xF7, 0x00,
0xF8, 0x00,
0xF9, 0x00,
0xFA, 0x38,

0xFF, 0x02,  //; Page 2
0x01, 0x00,
0x02, 0x20,
0x03, 0x8F,
0x04, 0x1C,
0x05, 0x33,
0x06, 0x0F,
0x07, 0x40,
0x08, 0x20,
0x09, 0x00,
0x0A, 0x04,
0x0B, 0x10,
0x0C, 0xCA,
0x0D, 0x92,
0x0E, 0x20,
0x0F, 0x02,
0x10, 0x30,
0x11, 0x21,
0x12, 0x03,
0x13, 0x00,
0x14, 0x0A,
0x15, 0x39,
0x16, 0xE0,
0x17, 0x01,
0x18, 0x00,
0x19, 0x27,
0x1A, 0x00,
0x1B, 0x00,
0x1C, 0x42,
0x1D, 0x3E,
0x1E, 0x02,
0x20, 0x00,
0x21, 0x00,
0x40, 0x10,
0x41, 0x00,
0x42, 0x05,
0x43, 0x01,
0x44, 0x64,
0x45, 0xF4,
0x46, 0x00,
0x47, 0x0A,
0x48, 0x36,
0x49, 0x10,
0x4A, 0x00,
0x4B, 0x00,
0x4C, 0x00,
0x4D, 0x44,
0x4E, 0x04,
0xE4, 0x21,
0xF8, 0x00,
0xF9, 0x80,

0xFF, 0x03,  //; Page 3

0xFF, 0x04,  //; Page 4

0xFF, 0x05,  //; Page 5

0xFF, 0x06,  //; Page 6
0x40, 0x00,
0x41, 0x00,
0x42, 0x00,
0x48, 0x00,
0x49, 0x17,
0x4A, 0x00,
0x4B, 0x00,
0x4C, 0x00,
0x4D, 0x00,
0x4E, 0x00,

0xFF, 0x00,

0xff, 0xff
};


//-----------------------------------------------------------------------------
/**
* "system no initialize mode" global variable.
*
* If P1_5 is connected at the PowerUpBoot, 
*   it is a system no init mode (SYS_MODE_NOINIT).
* If the system is a SYS_MODE_NOINIT, 
*  FW will skips the initialize routine, 
*  and supports only the Monitor function.
*  and SYS_MODE_NOINIT can not support a RCDMode and a PowerSaveMode.
* But, if the system bootup with normal, 
*  the P1_5 will be worked as a PowerSave ON/OFF switch.
*/
//-----------------------------------------------------------------------------
BYTE CurrSystemMode;

//-----------------------------------------------------------------------------
// Interrupt Handling Routine Variables			                                               
//-----------------------------------------------------------------------------
BYTE SW_Video_Status;
BYTE SW_INTR_cmd;
#define SW_INTR_VIDEO_CHANGED	1

//-----------------------------------------------------------------------------
// Task NoSignal
//-----------------------------------------------------------------------------
#define TASK_FOSD_WIN	0
#define NOSIGNAL_TIME_INTERVAL	(10*100)

XDATA BYTE Task_NoSignal_cmd;		//DONE,WAIT_VIDEO,WAIT,RUN,RUN_FORCE
XDATA BYTE Task_NoSignal_count;		//for dPuts("\nTask NoSignal TASK_CMD_WAIT_VIDEO");

//-----------------------------------------------------------------------------
// MovingGrid TASK ROUTINES				                                               
//-----------------------------------------------------------------------------
XDATA BYTE Task_Grid_on;
XDATA BYTE Task_Grid_cmd;

//-----------------------------------------------------------------------------
// MAIN LOOP				                                               
//-----------------------------------------------------------------------------
#define RET_MAIN_LOOP_PSM_BY_REMO	1
#define RET_MAIN_LOOP_PSM_BY_PORT	2



//=============================================================================
// Prototype				                                               
//=============================================================================
static void InitVariables(void);
static void PrintModelVersionInfo(void);
void InitCore(BYTE fPowerUpBoot);
BYTE InitSystem(BYTE _fPowerUpBoot);
BYTE main_loop(void);

void SystemPowerSave(void);
void SystemPowerResume(void);
BYTE WaitPowerOn(void);

void InterruptPollingHandlerRoutine(void);
void CheckAndClearOSD(void);


//-----------------------------------------------------------------------------
// Task NoSignal
//-----------------------------------------------------------------------------
void NoSignalTask( void );
void NoSignalTaskOnWaitMode(void);

void TaskNoSignal_setCmd(BYTE cmd); 
BYTE TaskNoSignal_getCmd(void); 


//-----------------------------------------------------------------------------
// MovingGrid TASK ROUTINES				                                               
//-----------------------------------------------------------------------------
extern void MovingGridTask( void );
void TaskSetGrid(BYTE onoff);
BYTE TaskGetGrid(void);
void TaskSetGridCmd(BYTE cmd);
BYTE TaskGetGridCmd(void);



//=============================================================================
// MAIN
//=============================================================================
/**
 * main
 *
 * main
 * ====
 *
 *	InitCPU
 *	InitCore
 *	InitSystem
 *	while(1)
 *		main_loop
 *
 * @param void
 * @return NONE
 *
 * @see InitCPU
 * @see InitCore
 * @see InitSystem
 * @see main_loop
*/
void main(void)
{
	BYTE ret;

	InitVariables();			//init glbal variable
	InitCPU();					//init 8051 MCU
	PrintModelVersionInfo();	//print MODEL VERSION
	InitCore(1);				//init SPIFlash QuadIO mode	

	SetMonAddress(TW88I2CAddress);
	Prompt(); //first prompt

	//==================================================
	// Init System
	//==================================================
	//
	// init system with a powerup mode
	//
	WriteTW88(REG008,  ReadTW88(REG008) | 0x10);	//Enable Output
	FP_GpioDefault();

	DCDC_StartUP();
		LedBackLight(ON);				//TurnOn Display
	LedPowerUp();

	I2CDeviceInitialize( TW8835_DefaultRegTable, 0 );	 //Pls, do not use this ....

	PrintSystemClockMsg("start loop");
	Prompt(); //second prompt
	WriteTW88Page(PAGE0_GENERAL);

	while(1) 
	{
		//==================================================
		// MAIN LOOP
		//==================================================
		ret=main_loop();		
		Printf("\nmain_loop() ret %bd",ret);		
		//	DO NOTHING
	}	
	//you can not be here...
}


//=============================================================================
// MAIN LOOP				                                               
//=============================================================================
//external
//	INT_STATUS
//	EX0
//	P1_3
//extern BYTE	TouchStatus;
//
/**
 * main_loop
 *
 * main_loop
 * =========
 *
 * @param void
 * @return 
 *	0: 
 *	1:PowerSaveMode by Remo
 *	2:PowerSaveMode by Port
 *
 * @see ext0_int
 * @see	InterruptPollingRoutine
 * @see	InterruptHandlerRoutine
 * @see	I2CCMD_exec_main
 * @see Monitor
 * @see	CheckKeyIn
 * @see	CheckRemo
 * @see	GetTouch2
 * @see	ActionTouch
 * @see CheckAndClearOSD
 * @see	NoSignalTask
 * @see NoSignalTaskOnWaitMode
*/
BYTE main_loop(void)
{
	BYTE ret;

	//---------------------------------------------------------------
	//			             Main Loop 
	//---------------------------------------------------------------
	while(1) {
		//-------------- Check Serial Port ---------------------
		Monitor();				// for new monitor functions

		//-------------- Check Watchdog ------------------------

		//-------------- block access routines -----------------
		if ( access == 0 ) continue;		
		/* NOTE: If you donot have an access, You can not pass */

 		//-------------- Check Keypad input --------------------
 		//-------------- Check Remote Controller ---------------
		//-------------- Check Touch ---------------
 		//-------------- Check special port for Power Save------
 		//-------------- Check special port for RCD mode------		
		//============== Task Section ==========================
		//-------------- Check TW8835 Chip Interrupt -------------
		//if(INT_STATUS || VH_Loss_Changed ) {
		//	InterruptPollingHandlerRoutine();
		//}
		//-------------- Check OSD timer -----------------------
		//============== HDMI Section ==========================
		//--------------
		//Task
		//-------------- I2CCMD -------------------------------
		//-------------- ext_i2c_timer ------------------------
	} //..while(1)

	return ret;
}

//=============================================================================
// Init QuadIO SPI Flash			                                               
//=============================================================================

//-----------------------------------------------------------------------------
/**
* init core
*
* prepare SPIFLASH QuadIO.
* enable chip interrupt.
*
* If you are using NOINIT, you should type "init core" by manually..
*/
void InitCore(BYTE fPowerUpBoot)
{
	if(fPowerUpBoot) {
		//check port 1.5. if high, it is a skip(NoInit) mode.
		CurrSystemMode = SYS_MODE_NORMAL;
	}

	Puts("\nInitCore");	
	//----- Set SPI mode
	SpiFlashVendor = SPI_QUADInit();
	if(SpiFlashVendor==0)
		Puts("\nWarning:System can be corrupted");
	SPI_SetReadModeByRegister(SPI_READ_MODE);		// Match DMA READ mode with SPI-read
	if(SpiFlashVendor==SFLASH_VENDOR_EON_256) {
		SPI_Set4BytesAddress(ON);
		if(SpiFlash4ByteAddr) Puts(" EN4B");
	}
	if(SpiFlashVendor==SFLASH_VENDOR_MX_256) {
#if 1
		Printf("\n==Found MX_256 need EN4B");
#else
		SPI_Set4BytesAddress(ON);
		if(SpiFlash4ByteAddr) Puts(" EN4B");
#endif
	}

	//----- Enable Chip Interrupt
	WriteTW88Page(PAGE0_GENERAL );
	WriteTW88(REG002, 0xFF );	// Clear Pending Interrupts
	WriteTW88(REG003, 0xEE );	// enable SW. enable SW INTR7FF
}
//-----------------------------------------------------------------------------
/**
* initialize TW8836 System
*
*	CheckEEPROM
*	InitWithNTSC
*	Set default GPIO
*	SSPLL_PowerUp
*	Startup DCDC
*	download Font
*	Start Video input
*	InitLogo1
*	Powerup LED
*	Remove Logo
*	Init Touch
* 	enable remocon
*
* @param bool fPowerUpBoot
* 	if fPowerUpBoot is true, 
*		download default value,
*		turn on Panel
* @return 0:success
* @see InitWithNTSC
* @see FP_GpioDefault
* @see SSPLL_PowerUp
* @see DCDC_StartUP
* @see FontOsdInit
* @see StartVideoInput
* @see InitLogo1
* @see LedPowerUp
* @see RemoveLogoWithWait
* @see OsdSetTime
* @see OsdSetTransparent
* @see BackLightSetRate
* @see MeasSetErrTolerance
* @see InitTouch
* @see UpdateOsdTimerClock
*/



//=============================================================================
// Video TASK ROUTINES				                                               
//=============================================================================


//-----------------------------------------------------------------------------
/**
* set NoSignalTask status
*		
* @param  cmd
*	- TASK_CMD_DONE
*	- TASK_CMD_WAIT_VIDEO
*	- TASK_CMD_WAIT_MODE
*	- TASK_CMD_RUN
*	- TASK_CMD_RUN_FORCE
*/
//-----------------------------------------------------------------------------
/**
* get NoSignalTask status
*
* @return Task_NoSignal_cmd
*/
//=============================================================================
// MovingGrid TASK ROUTINES				                                               
//=============================================================================

//-----------------------------------------------------------------------------
/**
 * on/off Grid task
 *
 * @param onoff
*/
//-----------------------------------------------------------------------------
/**
 * get Grid task status
 *
 * @return Task_Grid_on
*/
//-----------------------------------------------------------------------------
/**
 * set Grid task command
 *
 * @param cmd
*/
//-----------------------------------------------------------------------------
/**
 * get Grid task command
 *
 * @return Task_Grid_cmd
*/


//=============================================================================
// CheckAndSet LINK ROUTINES				                                               
//=============================================================================

//-----------------------------------------------------------------------------
/**
 * function pointer for CheckAndSetInput
 *
*/
BYTE (*CheckAndSetInput)(void);

//-----------------------------------------------------------------------------
/**
 * dummy CheckAndSet function
 *
*/
//-----------------------------------------------------------------------------
/**
 * link CheckAndSetInput Routine
 *
 * @see CheckAndSetDecoderScaler
 * @see CheckAndSetComponent
 * @see CheckAndSetPC
 * @see CheckAndSetDVI
 * @see CheckAndSetHDMI
 * @see CheckAndSetBT656Loop
 * @see CheckAndSetUnknown
*/



//-----------------------------------------------------------------------------
/**
 * Update OSD Timer
 *
*/
//-----------------------------------------------------------------------------
/**
 *	Check OSD Timer and clear OSD if timer is expired.
*/


//-----------------------------------------------------------------------------


//=============================================================================
// RearCameraDisplayMode				                                               
//=============================================================================



//-----------------------------------------------------------------------------
/**
* print model, version, compile date
*
* example:
*	********************************************************
*	 TW8835 EVB 3.1 - 18:52:43 (May 14 2012) SLAVE
*	********************************************************
*/
static void PrintModelVersionInfo(void)
{
	//-------------------------------------
	Printf("\n********************************************************");
#if defined(MODEL_TW8836)
	Puts("\n TW8836 ");
	#ifdef MODEL_TW8835
		Puts("on TW8835 ");
	#endif
	#if defined(CHIP_MANUAL_TEST) 
		Puts(" CHIPTEST ");
	#endif
#else
	Puts("\n TW88XX ");
#endif

#if defined(MODEL_TW88HAPS)
	Puts("HAPS - ");
#else
	Puts("EVB ");
#endif
	//board revision
#ifdef EVB_10
	Puts("1.0 - ");
#else
	Puts("0.0 - ");
#endif
	//FW revision
	Printf(" FW %bx.%bx -", (BYTE)(FWVER >> 8), (BYTE)FWVER);

	//compiled data
	Printf("%s (%s)", __TIME__, __DATE__);

	//server mode
#ifdef MODEL_TW8835_SLAVE
	Puts(" Server");
#endif
	Printf("\n********************************************************");
}




//=============================================================================
// INIT ROUTINES
//=============================================================================
//-----------------------------------------------------------------------------
extern BYTE	 OsdTime;
/**
* init global variables
*/
static void InitVariables(void)
{
	CurrSystemMode = SYS_MODE_NOINIT;
	DebugLevel=0;
	access = 1;
	SW_key = 0;

	//--task variables

	SpiFlashVendor = 0;	//see spi.h

	SpiFlash4ByteAddr = 0;	//32bit address mode.

}


//=============================================================================
//			                                               
//=============================================================================



//--------------------------------------------------
// Description  : Video initialize
// Input Value  : None
// Output Value : None
//--------------------------------------------------

//=============================================================================
// Video Signal Task				                                               
//=============================================================================

//-----------------------------------------------------------------------------
/**
* do Video Signal Task Routine
*
*
* @see Interrupt_enableVideoDetect
* @see CheckAndSetInput
* @see VInput_enableOutput
*/

//-----------------------------------------------------------------------------
/**
*  Check each input status
*
*  recover routine for unstable video input status.
*  only need it when user connect/disconnect the connector 
*  or, the QA toggles the video mode on the pattern generator.
*/

//=============================================================================
// Interrupt Handling Routine			                                               
//=============================================================================
//-----------------------------------------------------------------------------
/**
* enable VideoDetect interrupt
*
* Turn off the SYNC Change(R003[2]) mask,
*               the Video Loss(R003[1]) mask,
*               the WirteReg0x00F(R003[0] mask.
* 
* Turn On  the Video Loss(R003[1]) mask,
*               the WirteReg0x00F(R003[0] mask. 
*
* I do not turn on the SYNC Change.
* if you want to turn on SYNC, You have to call Interrupt_enableSyncDetect(ON).
*
* @param bool fOn
* @see Interrupt_enableSyncDetect
*/

//-----------------------------------------------------------------------------
/**
* Turn off/on SYNC Interrupt mask.
*
* @see Interrupt_enableVideoDetect
*/




