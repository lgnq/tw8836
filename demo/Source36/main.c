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

	//
	// if "access" becomes 0 in the InitCore(),
	// FW will be a no_init mode.
	// In this no_init mode, FW will support only the monitor.
	//  
	if (access == 0)
	{
		Puts("\n***SKIP_MODE_ON***");
		DebugLevel = 3;
		//skip...do nothing
		Puts("\nneed **init core***ee find***init ntsc***init panel***");
	}

	SetMonAddress(TW88I2CAddress);
	Prompt(); //first prompt

	//==================================================
	// Init System
	//==================================================
#ifdef SUPPORT_RCD
	//
	//If it is a RCD mode, FW will init only minimum routines for RCD.
	//RCD - Rear Camera Display
	//
	if ((PORT_BACKDRIVE_MODE == 0) && (CurrSystemMode == SYS_MODE_NORMAL))
	{
		CurrSystemMode = SYS_MODE_RCD; 
		InputMain      = 0;	//dummy
		InputBT656     = BT656INPUT_PANEL;
		InitRCDMode(1);
	}
	else
#endif

	//
	// init system with a powerup mode
	//
	InitSystem(1);

	PrintSystemClockMsg("start loop");
	Prompt(); //second prompt
	WriteTW88Page(PAGE0_GENERAL);

	while (1) 
	{
		//==================================================
		// MAIN LOOP
		//==================================================
		ret = main_loop();		
		Printf("\nmain_loop() ret %bd", ret);		
		if (ret == RET_MAIN_LOOP_PSM_BY_REMO || ret == RET_MAIN_LOOP_PSM_BY_PORT)
		{
			SystemPowerSave();		//move to PowerSaveMode	
			WaitPowerOn();			//wait PowerOn input from keypad, Remo. not Touch
			SystemPowerResume();	//resume
		}
		//else 
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
	while (1)
	{
		//-------------- Check Serial Port ---------------------
		Monitor();				// for new monitor functions

#ifdef SUPPORT_UART1
		Monitor1();				// for UART1
#endif

		//-------------- Check Watchdog ------------------------
#ifdef DEBUG_WATCHDOG
		if (F_watch)
		{
			static DWORD wdt_last=0;
			DWORD wdt_diff;
			DWORD curr_clock;
			F_watch = 0;

			RestartWatchdog();	//refresh watchdog counter.

			SFR_EA = 0;
			curr_clock = SystemClock;
			SFR_EA = 1;

			wdt_diff = curr_clock - wdt_last;
			wdt_last = curr_clock;
			ePrintf("\nWatchdog Interrupt !!! %02bx  %ld.%ldsec", SFR_WDCON, curr_clock/100, curr_clock%100);
			ePrintf(" diff:%ld.%02ldsec",wdt_diff/100, wdt_diff%100);

			RestartWatchdog();	//refresh watchdog counter. again
								//because, you are using a printf.
		}
#elif defined(SUPPORT_WATCHDOG)
		RestartWatchdog();		//refresh watchdog counter.
#endif

		//-------------- block access routines -----------------
		if (access == 0)
			continue;		
		/* NOTE: If you donot have an access, You can not pass */

 		//-------------- Check Keypad input --------------------
		CheckKeyIn();

 		//-------------- Check Remote Controller ---------------
		ret = CheckRemo();
		if (ret == REQUEST_POWER_OFF && CurrSystemMode == SYS_MODE_NORMAL)
		{
			ePrintf("\n===POWER SAVE===by Remo");
			ret = RET_MAIN_LOOP_PSM_BY_REMO;
			break;
		}

		//-------------- Check Touch ---------------
#ifdef SUPPORT_TOUCH
		if (TraceAuto)
			TraceTouch();
		ret = GetTouch2();
		if (ret)
		{
			ActionTouch();		
		}
#endif

 		//-------------- Check special port for Power Save------
#if 0
		//BK130114. disable TCREV(use GPIO) and test POWER_SAVE
		//TW8836 uses P1_5 for TCREV. We can not use it.
		if(PORT_POWER_SAVE==1 && CurrSystemMode==SYS_MODE_NORMAL) {
			ePrintf("\n===POWER SAVE===by PORT_POWER_SAVE");
			ret = RET_MAIN_LOOP_PSM_BY_PORT;
			break;
		}
#endif

 		//-------------- Check special port for RCD mode------
#ifdef SUPPORT_RCD
		if (IsBackDrivePortOn())
		{
			if (CurrSystemMode == SYS_MODE_NORMAL)
			{
				//move to RCD mode.
				CurrSystemMode = SYS_MODE_RCD;
				InputMain = 0;	//dummy
				InitRCDMode(0);
			}
		}
		else
		{
			if (CurrSystemMode == SYS_MODE_RCD)
			{
				DWORD BootTime;
				
				BootTime = SystemClock;
				//move to normal mode.
				CurrSystemMode = SYS_MODE_NORMAL;
				//turn off a parkgrid task first
				TaskSetGrid(OFF);
				SpiOsdWinHWEnable(0, OFF);
				SpiOsdWinHWEnable(1, OFF);
				SpiOsdEnable(OFF);

				InitSystem(0);

				BootTime = SystemClock - BootTime;
				ePrintf("\nBootTime(RCD2Normal):%ld.%ldsec", BootTime/100, BootTime%100 );
			}
		}
#endif	
		
		//============== Task Section ==========================
		if (Task_Grid_on)
			MovingGridTask();

		//skip VideoISR on RCD.
		if (CurrSystemMode == SYS_MODE_RCD)
			continue;					

		//-------------LVDS--------------------------------
#if 0 //BK130103
		if (InputMain==INPUT_LVDS)
		{
			if (LVDS_timer < SystemClock)
			{
				ret = ReadI2CI16Byte(I2CID_EP907M, 0x201);
				Printf("\nLVDS_Video_Status:%bx $201:%bx", LVDS_Video_Status, ret);
				if ((ret & 0xC0) == 0xC0)
				{
					//LinkON. Valid DE
					if (LVDS_Video_Status == 0)
					{
						Printf("\nLVDS Video OFF=>ON..I will call CheckAndSetInput");
						//TaskNoSignal_setCmd(TASK_CMD_RUN_FORCE);
						CheckAndSetInput();
					}
				}
				else if ((ret & 0xC0) == 0)
				{
					//LinkOFF, InvalidDE.
					if (LVDS_Video_Status)
					{
						Printf("\nLVDS Video ON=>OFF");
						LVDS_Video_Status = 0;
					}
				}

				//It does n't care overflow. Please do it later.
				LVDS_timer =  SystemClock + 200;
				if (LVDS_timer < SystemClock)
					LVDS_timer = 200;
			}
		}
#endif

		//-------------- Check TW8835 Chip Interrupt -------------
		if (INT_STATUS || VH_Loss_Changed)
		{
			InterruptPollingHandlerRoutine();
		}

#ifdef MODEL_TW8835
#else
		if (INT_STATUS3)
		{
			extern DWORD ExtIntCount;
			Printf("\nINT_STATUS3:%bx  count: %ld",INT_STATUS3, ExtIntCount );
			INT_STATUS3 = 0;
		}
#endif

		//-------------- Check OSD timer -----------------------
		CheckAndClearOSD();

		//============== HDMI Section ==========================

		//--------------
		//Task
		NoSignalTask();
		NoSignalTaskOnWaitMode(); //Check each input status when WAIT_MODE

		//-------------- I2CCMD -------------------------------
#if defined(SUPPORT_I2CCMD_SLAVE_V1)
		if (F_i2ccmd_exec)
		{
			F_i2ccmd_exec = 0;
			I2CCMD_exec_main();
		}
#endif

		//-------------- ext_i2c_timer ------------------------
#if defined(SUPPORT_I2CCMD_TEST_SLAVE)
		if (ext_i2c_cmd)
		{
			if (ext_i2c_timer)
				continue;

			//if timeover
			WriteTW88(REG009, 0);	// ? clear
			
			ext_i2c_cmd = 0;		//clear
			InitISR(0);	
			Printf("\next_i2c_timer expire");
		}
#endif
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
	if (fPowerUpBoot)
	{
		//check port 1.5. if high, it is a skip(NoInit) mode.
#if 0 //BK130116   def MODEL_TW8836FPGA
//BK121220. Not working.
//		if(PORT_NOINIT_MODE == 1)	
		if(1)	
		{
			CurrSystemMode = SYS_MODE_NOINIT;
			//turn on the SKIP_MODE.
			access = 0;
					
			McuSpiClkSelect(MCUSPI_CLK_27M);
			I2C_delay_base = 1;
			return;
		}
#endif
		CurrSystemMode = SYS_MODE_NORMAL;
	}

	Puts("\nInitCore");	
	//----- Set SPI mode
	SpiFlashVendor = SPI_QUADInit();
	if (SpiFlashVendor == 0)
		Puts("\nWarning:System can be corrupted");
	SPI_SetReadModeByRegister(SPI_READ_MODE);		// Match DMA READ mode with SPI-read
	if (SpiFlashVendor == SFLASH_VENDOR_EON_256)
	{
		SPI_Set4BytesAddress(ON);
		if (SpiFlash4ByteAddr)
			Puts(" EN4B");
	}
	if (SpiFlashVendor == SFLASH_VENDOR_MX_256)
	{
#if 1
		Printf("\n==Found MX_256 need EN4B");
#else
		SPI_Set4BytesAddress(ON);
		if(SpiFlash4ByteAddr) Puts(" EN4B");
#endif
	}

	//----- Enable Chip Interrupt
	WriteTW88Page(PAGE0_GENERAL);
	WriteTW88(REG002, 0xFF);	// Clear Pending Interrupts
	WriteTW88(REG003, 0xEE);	// enable SW. enable SW INTR7FF
}

/**
* InitLVDS Tx
*/
#ifdef PANEL_AUO_B133EW01
void InitLVDSTx(void)
{
	WriteTW88(REG006, (ReadTW88(REG006) & 0xE0) | 0x06);

	WriteTW88(REG640, 0x0C);
	WriteTW88(REG641, 0x00);
	WriteTW88(REG642, 0x41);
	WriteTW88(REG644, 0xBF);
	WriteTW88(REG645, 0xBF);
	WriteTW88(REG646, 0xBF);
	WriteTW88(REG647, 0xBF);
	WriteTW88(REG648, 0xBF);
	WriteTW88(REG649, 0x17);
	WriteTW88(REG64A, 0x00);
	WriteTW88(REG64B, 0x00);
}
#endif

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
BYTE InitSystem(BYTE _fPowerUpBoot)
{
	BYTE ee_mode;
	BYTE value;
	BYTE FirstInitDone;
	BYTE fPowerUpBoot;	//to solve the compiler bug, use a variable.

	if (access == 0)
		//do nothing.
		return 0;

	fPowerUpBoot = _fPowerUpBoot;

	//check EEPROM
#ifdef USE_SFLASH_EEPROM
//	E3P_Configure();
#endif

	ee_mode = CheckEEPROM();
	if (ee_mode == 1) //CheckEEPROM() failed
	{
		//---------- if FW version is not matched, initialize EEPROM data -----------
		Init8836AsDefault(0, 1);	//input:INPUT_CVBS,fPowerUp:1
		
		DebugLevel = 3;

		#ifdef USE_SFLASH_EEPROM
		E3P_Format();
		EE_FindCurrInfo();
		#endif
	
		InputMain = 0xff;	// start with saved input
		InitializeEE();	 	//save all default EE values.
	
		DebugLevel = 0;
		SaveDebugLevelEE(DebugLevel);

		ee_mode = 0;
	}

	//read debug level
	DebugLevel = GetDebugLevelEE();
	if ((DebugLevel == 0) && (fPowerUpBoot))
		Printf("\n===> Debugging was OFF (%02bx)", DebugLevel);
	else 
		ePrintf("\n===> Debugging is ON (%02bx)", DebugLevel);

	ePrintf("\nInitSystem(%bd)", fPowerUpBoot);

	InputMain  = GetInputMainEE();
	InputBT656 = GetInputBT656EE();

	//
	//set default setting.
	//
	if (fPowerUpBoot)
	{
		//Init HW with default
		Init8836AsDefault(InputMain, 1);
		I2C_delay_base = 3;
		
#if 1
	   /*
		*	Check I2C GPIO Expendor.
		*	If I2C device holds SCL, it can hangup the system.
		*	0: success
		*	1: NAK
		*	2: I2C dead
		*/
		value = CheckI2C_B0(I2CID_SX1504);
		switch (value)
		{
			case 0:		
				Puts("\nI2CID_SX1504:Pass");
				break;
			case 1:
				Puts("\nI2CID_SX1504:NAK");
				break;
			case 2:
				Puts("\nI2CID_SX1504:Dead");
				break;
			default:
				Printf("\nI2CID_SX1504:%bx", value);
				break;
		}
		
		if (value == 0)
			Printf("\nI2CID_SX1504 0:%02bx 1:%bx", ReadI2CByte(I2CID_SX1504, 0), ReadI2CByte(I2CID_SX1504, 1));
#endif

		//------------------
		//first GPIO position.
		//GPIO needs TRI_EN=0.
		FP_GpioDefault();

		SSPLL_PowerUp(ON);
		//If you use DCDC data out, it needs 200ms.
		//PrintSystemClockMsg("SSPLL_PowerUp");

#if 0 	//If you want to use PLLCLK, select MCUSPI_CLK_PCLKPLL 
		//default is a PLL108M.
		McuSpiClkSelect(MCUSPI_CLK_PCLKPLL);
#endif

#ifdef PANEL_AUO_B133EW01
		SspllSetFreqReg(0x20000);						//start from SSPLL 108MHz.
		WriteTW88(REG4E1, ReadTW88(REG4E1) & 0xCF);		//select 27Mhz
		WriteTW88(REG4E0, ReadTW88(REG4E0) | 0x01);		//select PLL108
		WriteTW88(REG4E1, 0xE0);						//select PLLCLK with divider 1 + clk_edge+1cycle_delay.
		//SspllSetFreqReg(0x2C71C);						//increase SSPLL as 150MHz. => FOSD max is 139Mhz 
		SspllSetFreqReg(0x2097B);						//increase SSPLL as 110Mhz
		PclkoSetDiv(6);									//divider 1.5		
		I2C_delay_base = 4;

		InitLVDSTx();
#endif

		DumpClock();
	}

	//------------------------
	// init HDMI chip for EDID & HDCP.
#if defined(SUPPORT_HDMI_EP9351) || defined(SUPPORT_HDMI_EP9553)
	//Init_HdmiSystem();
#endif

	FirstInitDone = 0;

	//---------------------
	// turn on DCDC
	//---------------------
	if (fPowerUpBoot)
		DCDC_StartUP();

	//---------------
	//init FontOSD 
	FontOsdInit();
	FOsdSetDeValue();
	FOsdIndexMsgPrint(FOSD_STR1_TW8835);
	FOsdWinEnable(0, OFF);	//win0, disable..
	
	//------------------------
	//start with saved input
	//------------------------
	StartVideoInput();
	PrintSystemClockMsg("Finish StartVideoInput");

	//
	//draw Logo
	//
	if (FirstInitDone == 0)
	{
		if (InputMain == INPUT_HDMIPC || InputMain == INPUT_HDMITV || InputMain == INPUT_LVDS) 
		{
			Printf("\nSkip InitLogo1()");
		}
		else
		{
			InitLogo1();
			FirstInitDone = 1;
		}
	}
	//
	// Power Up FP LED.
	// Now you can see somthing on your panel.
	LedPowerUp();

	//enable human input. (remocon,Touch, and Keypad).
	EnableRemoInt();
	InitAuxADC();	

	//
	//remove Logo
	//
//#ifdef NOSIGNAL_LOGO 
//	if(FirstInitDone ==1) {
//		FirstInitDone = 2;
//		if(Task_NoSignal_cmd == TASK_CMD_DONE) {
//			RemoveLogoWithWait(fPowerUpBoot);
//			FOsdWinEnable(0, OFF);	//win0, disable..
//		}
//	}	
//	//BK120803. If VDLoss, set FreerunManual.
//	else {
//		WriteTW88Page(PAGE0_GENERAL);
//		if(ReadTW88(REG004) & 0x01)
//			ScalerSetFreerunManual(ON);
//	}
//#else
	if (FirstInitDone == 1)
	{
		FirstInitDone = 2;
		RemoveLogoWithWait(1);
		if (Task_NoSignal_cmd == TASK_CMD_DONE)
			FOsdWinEnable(0, OFF);	//win0, disable..
	}	
//#endif

	//------------------------
	// setup eeprom effect
	//------------------------
	SetAspectHW(GetAspectModeEE());
	value = EE_Read(EEP_FLIP);	//mirror
	if (value)
	{
		WriteTW88Page(PAGE2_SCALER);
	    WriteTW88(REG201, ReadTW88(REG201) | 0x80);
	}

	OsdSetTime(EE_Read(EEP_OSD_TIMEOUT));
	OsdSetTransparent(EE_Read(EEP_OSD_TRANSPARENCY));
	BackLightSetRate(EE_Read(EEP_BACKLIGHT));

	//set the Error Tolerance value for "En Changed Detection"
	MeasSetErrTolerance(0x04);		//tolerance set to 32. 

#ifdef USE_SFLASH_EEPROM
	//to cleanup E3PROM
	//call EE_CleanBlocks();
#endif

	UpdateOsdTimerClock();

	// init I2CCMD server.
	// 
#if defined(MODEL_TW8835_SLAVE)
	//Init_I2CCMD_Slave();
#endif

	// re calculate FOSD DE
	FOsdSetDeValue();

	//enable watchdog.
#ifdef SUPPORT_WATCHDOG
	EnableWatchdog(1);
#ifdef DEBUG_WATCHDOG
	F_watch = 1; //for first  wdt_last value
#endif
#endif

	return 0;
}

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
void TaskNoSignal_setCmd(BYTE cmd) 
{ 	
	if (cmd == TASK_CMD_WAIT_VIDEO && MenuGetLevel())	
		Task_NoSignal_cmd = TASK_CMD_DONE;	
	else
		Task_NoSignal_cmd = cmd;

	if (cmd == TASK_CMD_RUN_FORCE)
		tic_task = NOSIGNAL_TIME_INTERVAL;	//right now

	Task_NoSignal_count = 0;
}							

//-----------------------------------------------------------------------------
/**
* get NoSignalTask status
*
* @return Task_NoSignal_cmd
*/
BYTE TaskNoSignal_getCmd(void) 
{ 	
	return Task_NoSignal_cmd;	
}

//=============================================================================
// MovingGrid TASK ROUTINES				                                               
//=============================================================================

//-----------------------------------------------------------------------------
/**
 * on/off Grid task
 *
 * @param onoff
*/
void TaskSetGrid(BYTE onoff)  {	Task_Grid_on = onoff;	}	
//-----------------------------------------------------------------------------
/**
 * get Grid task status
 *
 * @return Task_Grid_on
*/
BYTE TaskGetGrid(void)		  {	return Task_Grid_on;    }
//-----------------------------------------------------------------------------
/**
 * set Grid task command
 *
 * @param cmd
*/
void TaskSetGridCmd(BYTE cmd) { Task_Grid_cmd = cmd;	}	
//-----------------------------------------------------------------------------
/**
 * get Grid task command
 *
 * @return Task_Grid_cmd
*/
BYTE TaskGetGridCmd(void)	  { return Task_Grid_cmd;   } 


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
BYTE CheckAndSetUnknown(void)
{
	return ERR_FAIL;
}

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
void LinkCheckAndSetInput(void)
{
	switch(InputMain) {
#if defined(SUPPORT_CVBS) || defined(SUPPORT_SVIDEO)
	case INPUT_CVBS:
	case INPUT_SVIDEO:
		CheckAndSetInput = &CheckAndSetDecoderScaler;
		break;
#endif
#ifdef SUPPORT_COMPONENT
	case INPUT_COMP:
		CheckAndSetInput = &CheckAndSetComponent;
		break;
#endif
#ifdef SUPPORT_PC
	case INPUT_PC:
		CheckAndSetInput = &CheckAndSetPC;
		break;
#endif
#ifdef SUPPORT_DVI
	case INPUT_DVI:
		CheckAndSetInput = &CheckAndSetDVI;
		break;
#endif
#if defined(SUPPORT_HDMI)
	case INPUT_HDMIPC:
	case INPUT_HDMITV:
		CheckAndSetInput = &CheckAndSetHDMI;
		break;
#endif
#ifdef SUPPORT_BT656_LOOP
	case INPUT_BT656:
		CheckAndSetInput = &CheckAndSetBT656Loop;
		break;
#endif
#ifdef SUPPORT_LVDSRX
	case INPUT_LVDS:
		CheckAndSetInput = &CheckAndSetLVDSRx;
		break;
#endif
	default:
		CheckAndSetInput = &CheckAndSetUnknown;
		break;
	}
	SW_Video_Status = 0;					//clear
	FOsdWinEnable(TASK_FOSD_WIN,OFF);		//WIN0, Disable
	TaskNoSignal_setCmd(TASK_CMD_DONE);		//turn off NoSignal Task
}



#ifdef SUPPORT_RCD
//-----------------------------------------------------------------------------
/**
 * check RCD(Rear Camera Display) port
 *
 * @return 0:No, 1:Yes
*/
BYTE IsBackDrivePortOn(void)
{
	//BT656_LOOP uses P1_6. FW can not support RCD on BT656 mode.
	if(InputMain >= INPUT_DVI) 
		return 0;
	return (PORT_BACKDRIVE_MODE==0 ? 1 : 0);
}
#endif

/**
 * Update OSD Timer
*/
void UpdateOsdTimerClock(void)
{
	DECLARE_LOCAL_page

	OsdTimerClock = (DWORD)OsdGetTime() * 100;
	
	ReadTW88Page(page);

	//Turn On FontOSD.
	FOsdOnOff(ON, 0);	//with vdelay 0

	WriteTW88Page(page);		
}

/**
 *	Check OSD Timer and clear OSD if timer is expired.
*/
void CheckAndClearOSD(void)
{
	DECLARE_LOCAL_page

	if (OsdGetTime() == 0)
		return;

	if (OsdTimerClock == 0)
	{
		ReadTW88Page(page);

		if (MenuGetLevel())	
			MenuEnd();	
		
		//Turn OFF Font OSD
		if (FOsdOnOff(OFF, 0))	//with vdelay 0
			dPuts("\nCheckAndClearOSD disable FOSD");
			
		if (getNoSignalLogoStatus())
			RemoveLogo();
					
		WriteTW88Page(page);
	}
}

//================================
// Power Save & Resume
//================================
BYTE Buf_r003;
BYTE Buf_0B0;	// Touch
BYTE Buf_106;	// ADC
BYTE Buf_1E6;	// AFE mode
BYTE Buf_1CB;	// LLPLL, SOG
BYTE Buf_1E1;	// LLPLL GPLL
BYTE Buf_4E1;	// Clock selection

//-----------------------------------------------------------------------------
/**
* Go into Power Save Mode
*
* System PowerSave procedure
* ==========================
*
*	set all GPIOs as input mode
*	switch MCU clock to 27MKz
*	Powerdown all analog blocks
*	Power up RC oscillator
*	Switch MCU/SPI clock to RC oscillator
*	Power	down crysital oscillator
*	Now, it is a PowerSave Mode
*
* @see WaitPowerOn
* @see SystemPowerResume
*/
void SystemPowerSave(void)
{
	BYTE i;

	Printf("\n----- SystemPowerSave -----");
	delay1ms(10);

	//set all GPIOs as input mode	
	//WriteTW88Page(PAGE0_GPIO);
	//WriteTW88(REG08C, 0x00);

	FP_BiasOnOff(OFF);

	WriteTW88Page(PAGE0_GENERAL);
	Buf_r003 = ReadTW88(REG003);
	Interrupt_enableVideoDetect(OFF);

	WriteTW88Page(PAGE0_OUTPUT); 		
	WriteTW88(REG008, ReadTW88(REG008) | 0x30);	// Tri-State All outputs & FPdata 

	FP_PWC_OnOff(OFF);

	//switch MCU clock to 27MKz
	WriteTW88Page(PAGE4_CLOCK);
	Buf_4E1 = ReadTW88(REG4E1);
	WriteTW88(REG4E1, 0x00);	 				// SPI clock Source=27MHz

	//----- Powerdown all analog blocks
	WriteTW88Page(PAGE0_LEDC );
#ifdef MODEL_TW8835
	WriteTW88(REG0E0, 0xF2 );					// LEDC
#else
//	WriteTW88(REG0E0, 0xF2 );					// LEDC	  BK120914
#endif
	WriteTW88(REG0E8, 0xFE );					// DCDC, VCOM-DC, VCOM-AMP

	SFR_ET1 = 0;								// Disable Touch Timer
	WriteTW88Page(PAGE0_TOUCH );
	Buf_0B0 = ReadTW88(REG0B0);
	WriteTW88(REG0B0, Buf_0B0 | 0x80 );			// TSC_ADC			  	*** 0.2uA

	WriteTW88Page(PAGE1_DECODER );
	Buf_106 = ReadTW88(REG106);
	WriteTW88(REG106, Buf_106 | 0x0F );			// ADC
	Buf_1E6 = ReadTW88(REG1E6);
	WriteTW88(REG1E6, 0x00 );					// AFE Mode=low speed	*** 0.6uA

	//WriteTW88Page(PAGE1_VADC );
	Buf_1CB = ReadTW88(REG1CB);
	WriteTW88(REG1CB, Buf_1CB & 0x1F );			// SOG, LLPLL
	Buf_1E1 = ReadTW88(REG1E1);
	WriteTW88(REG1E1, Buf_1E1 | 0x20 );			// LLPLL GPLL

	//----- SSPLL power down
	SSPLL_PowerUp(OFF);							// SSPLL

	//----- Switch MCU/SPI clock to RC oscillator
	WriteTW88Page(PAGE4_CLOCK);
	WriteTW88(REG4E1, 0x10);	 				// SPI clock Source=32KHz, ...

	//----- Power down crysital oscillator
	WriteTW88Page(PAGE0_LOPOR);
	WriteTW88(REG0D4, ReadTW88(REG0D4) | (0x80));	// Enable Xtal PD Control
	PORT_CRYSTAL_OSC = 0;						// Power down Xtal

	while( PORT_POWER_SAVE==1 );

	//----- Wait ~30msec to remove key bouncing
	for(i=0; i<100; i++);

	//
	//Now, it is a PowerSave Mode
	//
}

//-----------------------------------------------------------------------------
/**
* Resume from Power Save Mode				                                               
*
* System Resume procedure
* ========================
*
*	Power up crystal oscillator
*	wait until crystal oscillator stable
*	switch MCU/SPI clock to 27MHz
*	Power up all analog blocks
*	Set MCU clock mode back
*	Set GPIO mode back
*	Now, Normal Operation mode
*
* @see WaitPowerOn
* @see SystemPowerSave
*/
void SystemPowerResume(void)
{
	BYTE i;

	SFR_EA = 0;

	//----- Power up Xtal Oscillator
	WriteTW88Page(PAGE0_LOPOR);
	PORT_CRYSTAL_OSC = 1;							// Power up Xtal
	WriteTW88(REG0D4, ReadTW88(REG0D4) & ~(0x80)); 	// Disable Xtal PD Control

	//----- Wait until Xtal stable (~30msec)
	for(i=0; i<100; i++);

	//----- switch MCU/SPI clock to 27MHz
	WriteTW88Page(PAGE4_CLOCK);
	WriteTW88(REG4E1, 0x00);	 					// SPI clock Source=27MHz, ...

	//----- Power up SSPLL
	SSPLL_PowerUp(ON);								// SSPLL
	//DCDC data out needs 200ms.
	//GlobalBootTime = SystemClock;
	//PrintSystemClockMsg("SSPLL_PowerUp");

	//----- Wait until SSPLL stable (~100usec)
	for(i=0; i<200; i++);
	for(i=0; i<200; i++);

	//----- Power up all analog blocks
	WriteTW88Page(PAGE1_VADC );
	WriteTW88(REG1E1, Buf_1E1);					// LLPLL GPLL
	WriteTW88(REG1CB, Buf_1CB);					// LLPLL, SOG
	
	WriteTW88Page(PAGE1_DECODER );
	WriteTW88(REG106, Buf_106);					// ADC
	WriteTW88(REG1E6, Buf_1E6);					// AFE mode
	
	WriteTW88Page(PAGE0_TOUCH );
	WriteTW88(REG0B0, Buf_0B0);					// Touch

	//----- Set MCU clock mode back
	WriteTW88Page(PAGE4_CLOCK);
	WriteTW88(REG4E1, Buf_4E1);	 				// Clock selection

	//----- Set GPIO mode back
	//WriteTW88Page(PAGE0_GPIO);
	//WriteTW88(REG08C, 0x0C);

	SFR_EA = 1;

	//
	// In case aRGB input, set Filter=0 and wait until stable and Filter=7
	//

#ifdef SUPPORT_TOUCH
	SFR_ET1 = 1;									// Enable Touch Timer
#endif
	//
	//Now, Normal Operation mode
	//

	while( PORT_POWER_SAVE==1 );				// Wait untill button is released
	delay1ms(100);								// To remove key bouncing

	//----- Power up Panel, Backlight
	//SSPLL_PowerUp needs 100ms before FW turns on the DataOut
	DCDC_StartUP();								// DCDC. it has WaitVBlank.
	LedPowerUp();								// LEDC


	Puts("\n----- SystemPowerResume -------");
	Interrupt_enableVideoDetect(ON);
	WriteTW88Page(PAGE0_GPIO);
	WriteTW88(REG003, Buf_r003);	//recover ISR mask
	if(DebugLevel)
		Prompt();
}

//-----------------------------------------------------------------------------
/**
* wait powerup condition on the power save state
*
* @return 1:by button, 2:by Touch
* @see SystemPowerSave
* @see SystemPowerResume
*/
BYTE WaitPowerOn(void)
{
//	BYTE i;
	WORD ii;

	while(1) {

#if 1
		//PORT_POWER_SAVE has a problem, temporary, use a poll to check a suspend & resume.
		for(ii=0; ii < 0x100; ii++) ;
		return 1;
#endif

#if 0
		//----- Check Power Button
#if 1
		if(PORT_POWER_SAVE==1) {
			for(i=0; i < 100; i++);
			if(PORT_POWER_SAVE==1) return 1;
		}
#else //BK130103. 
		//only for TW8836 Test.
		//Select CBVS.
		//turn off BT656 before you start.
		//Press POWER KEY on remocon.
		//to resume, move SW1 pin 2.
		if(P1_7==0) {
			for(i=0; i < 100; i++);
			if(P1_7==0) return 1;
		}
#endif
		//----- Check Touch
		if( P2_4==0 ) return 2;

		//----- Check Remote Control
		//if( P1_2==0 ) return 3;	// Need to confirm if it is by Power Button
#endif
	}
}


//=============================================================================
// RearCameraDisplayMode				                                               
//=============================================================================
#ifdef SUPPORT_RCD
//-----------------------------------------------------------------------------
/**
*	Turn On/Off Back drive grid SPIOSD image
*/
static void BackDriveGrid(BYTE on)
{
	if(on) {
		//draw parkgrid
		SOsdWinBuffClean(0);

		//init DE
		SpiOsdSetDeValue();

		//init SOSD
		WaitVBlank(1);
		SpiOsdEnable(ON);
		SpiOsdDisableRlcReg(0);
		SpiOsdWinImageLocBit(1,0);
		SpiOsdWinLutOffset( 1, 0 /*SOSD_WIN_BG,  WINBG_LUTLOC*/ );  //old: SpiOsdLoadLUT_ptr
		SpiOsdWinFillColor( 1, 0 );

		MovingGridInit();
		//MovingGridDemo(0 /*Task_Grid_n*/);
		MovingGridTask_init();
		MovingGridLUT(3);	//I like it.

	}
	else {
		SpiOsdWinHWOffAll(0);	//without wait
		StartVideoInput();
	}
}

//-----------------------------------------------------------------------------
/**
* init RCD mode
*
* goto RCDMode (RearCameraDisplay Mode)
* and, prepare ParkingGrid.
* RCDMode does not support a video ISR.
* @return
*	0:success
*	other:error code
*/
BYTE InitRCDMode(BYTE fPowerUpBoot)
{
	BYTE ret;

	Printf("\nInitRCDMode(%bd)", fPowerUpBoot);
	if (fPowerUpBoot == 0)
	{
		if (MenuGetLevel())
		{
			MenuQuitMenu();
			SpiOsdWinHWOffAll(1);	//with WaitVBlank
		}
		//FYI. I don't care demo page.
	}

	//skip CheckEEPROM() and manually assign DevegLevel
	DebugLevel = 1;

	//set default setting.
	Init8836AsDefault(0/*InputMain*/, 1);

	FP_GpioDefault();

	SSPLL_PowerUp(ON);
	//PrintSystemClockMsg("SSPLL_PowerUp");
	//DCDC needs 100ms, but we have enough delay on...

	WriteTW88Page(PAGE0_GENERAL);
	WriteTW88(REG040, ReadTW88(REG040) & ~0x10);

	DCDC_StartUP();

	PrintSystemClockMsg("before DecoderCheck");

	//FW add a check routine because the customer wants a stable video..
	//Current code only check the NTSC. 
	//If you want to PAL, change REG11D value.
	//If we assign only NTSC, it uses a 300ms.
	//If we add all standard, it uses a 500ms.
	WriteTW88Page(1);
	WriteTW88(REG11D, 0x01);

	//wait until we have a stable signal
	ret = DecoderCheckVDLOSS(100);
	if (ret)
	{
		ePuts("\nCheckAndSetDecoderScaler VDLOSS");
	}
	else
	{
		//get standard
		ret = DecoderCheckSTD(100);
		if (ret == 0x80)
		{
		    ePrintf("\nCheckAndSetDecoderScaler NoSTD");
			//return( 2 );
		}
		else
		{
			ret >>= 4;
			//InputSubMode = mode;
			ePrintf("\nMode:%bx",ret);
		}
	}
	PrintSystemClockMsg("after DecoderCheck");

	//disable interrupt.
	WriteTW88Page(PAGE0_GENERAL);
	WriteTW88(REG003, 0xFE);	// enable only SW interrupt

	LedBackLight(ON);
	ScalerSetMuteManual(OFF);

	//draw parkgrid
	BackDriveGrid(ON);

	LedPowerUp();

	return ret;
}
#endif

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
	Task_Grid_on = 0;
	Task_Grid_cmd = 0;
	Task_NoSignal_cmd = TASK_CMD_DONE;
	SW_INTR_cmd = 0;

	SpiFlashVendor = 0;	//see spi.h

	OsdTime	=	0;
	SpiFlash4ByteAddr = 0;	//32bit address mode.

#if defined(SUPPORT_I2CCMD_TEST_SLAVE)
	ext_i2c_cmd=0;
	ext_i2c_timer=0;
#endif

#ifdef DEBUG_REMO_NEC
	DebugRemoStep = 0;
#endif

#ifdef DEBUG_UART
	UART0_OVERFLOW_counter = 0;  //clear
	UART0_MAX_counter = 0; 		//clear
#endif
}


//=============================================================================
//			                                               
//=============================================================================

//-----------------------------------------------------------------------------
/**
* start video with a saved input.
*
* @see ChangeInput
*/
void StartVideoInput(void)
{
	BYTE InputMainEE;
				
	ePrintf("\nStart with Saved Input: ");
	InputMainEE = GetInputMainEE();
	PrintfInput(InputMainEE, 1);

	InputMain = 0xff;			// start with saved input						
	ChangeInput(InputMainEE);	
}

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
void NoSignalTask(void)
{	
	DECLARE_LOCAL_page
	BYTE ret;
	BYTE r004;

	if (Task_Grid_on)
		// MovingGridTask uses tic_task. It can not coexist with NoSignalTask.
		return;

	if (Task_NoSignal_cmd == TASK_CMD_DONE)
		return;

	if (tic_task < NOSIGNAL_TIME_INTERVAL) 
		return;

	if (Task_NoSignal_cmd == TASK_CMD_WAIT_VIDEO)
	{
		ReadTW88Page(page);

		FOsdWinToggleEnable(TASK_FOSD_WIN); //WIN0-toggle
		if (Task_NoSignal_count < 3)
		{
			dPuts("\nTask NoSignal TASK_CMD_WAIT_VIDEO");
			Task_NoSignal_count++;
		}
		tic_task = 0;

		WriteTW88Page(page);
		return;
	}
	
	if (Task_NoSignal_cmd == TASK_CMD_WAIT_MODE)
		return;
 
	//--------------------------------------------
	//
	//--------------------------------------------

	dPuts("\n***Task NoSignal TASK_CMD_RUN");
	if (Task_NoSignal_cmd == TASK_CMD_RUN_FORCE)
		dPuts("_FORCE");

	ReadTW88Page(page);
 	WriteTW88Page(PAGE0_GENERAL);
	r004 = ReadTW88(REG004);
	if (r004 & 0x01)
	{						
		ePrintf("..Wait...Video");

		tic_task = 0;
		WriteTW88Page(page);
		return;
	}

	//turn off Interrupt.
	Interrupt_enableVideoDetect(OFF);

	//start negotition
	ret = CheckAndSetInput();

	//turn on Interrupt. 
	//if success, VInput_enableOutput() will be executed.
	Interrupt_enableVideoDetect(ON);

	if (ret == ERR_SUCCESS)
	{
		dPuts("\n***Task NoSignal***SUCCESS");
		VInput_enableOutput(VH_Loss_Changed);
		FOsdWinEnable(TASK_FOSD_WIN, OFF); 	//WIN0, Disable

		if (getNoSignalLogoStatus())
		{
			ScalerSetFreerunManual(OFF);	//BK120803
			RemoveLogo();
		}

		//need SetBT656Output() with proper mode.....
		//SetBT656Output(GetInputBT656EE());
	}
#ifdef SUPPORT_PC
	else
	{
		//fail 
		if (InputMain == INPUT_PC)
		{
			WriteTW88Page(PAGE0_GENERAL);
			if (ReadTW88(REG004) & 0x01)
				FOsdIndexMsgPrint(FOSD_STR2_NOSIGNAL);	//over write
			else
				FOsdIndexMsgPrint(FOSD_STR3_OUTRANGE);	//replace 
		}
	}
#endif

	//update tic_task.
	tic_task = 0;

	WriteTW88Page(page);
}

//-----------------------------------------------------------------------------
/**
*  Check each input status
*
*  recover routine for unstable video input status.
*  only need it when user connect/disconnect the connector 
*  or, the QA toggles the video mode on the pattern generator.
*/
void NoSignalTaskOnWaitMode(void)
{
	BYTE ret;
	
	DECLARE_LOCAL_page

	if ((Task_NoSignal_cmd != TASK_CMD_WAIT_MODE))
		return;
	
	ReadTW88Page(page); 
	if (InputMain==INPUT_CVBS || InputMain==INPUT_SVIDEO)
	{
		ret = DecoderReadDetectedMode();
		//only consider NTSC & PAL with an idle mode.
		if (ret == 0 || ret == 1)
		{
			if (InputSubMode != ret)
			{
				ScalerSetMuteManual(ON);

				SW_INTR_cmd = SW_INTR_VIDEO_CHANGED;
				dPrintf("\nRequest SW Interrupt cmd:%bd InputSubMode:%bd->%bd", SW_INTR_cmd, InputSubMode, ret);
				InputSubMode = ret;
				WriteTW88Page(PAGE0_GENERAL);
				WriteTW88(REG00F, SW_INTR_VIDEO);	//SW interrupt.		
			} 
		}
	}
#ifdef SUPPORT_COMPONENT
	else if (InputMain == INPUT_COMP)
	{
		ret = aRGB_GetInputStatus();	//detected input.
		if (ret & 0x08) 			//check the compoiste detect status first.
		{
			ret &= 0x07;
			if ((ret!=7) && (InputSubMode != ret))
			{
				ScalerSetMuteManual(ON);

				SW_INTR_cmd = SW_INTR_VIDEO_CHANGED;
				dPrintf("\nRequest SW Interrupt cmd:%bd InputSubMode:%bd->%bd", SW_INTR_cmd, InputSubMode, ret);
				InputSubMode = ret;
				WriteTW88Page(PAGE0_GENERAL);
				WriteTW88(REG00F, SW_INTR_VIDEO);	//SW interrupt.		
			} 
		}
	}
#endif
	WriteTW88Page(page);
}

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
void Interrupt_enableVideoDetect(BYTE fOn)
{
#ifdef DEBUG_ISR
	WORD temp_VH_Loss_Changed;
	BYTE temp_INT_STATUS, temp_INT_STATUS2;
#endif

	DECLARE_LOCAL_page

	ReadTW88Page(page);
	WriteTW88Page(PAGE0_GENERAL);
	if (fOn)
	{
		WriteTW88(REG002, 0xFF);	//clear
		WriteTW88(REG004, 0xFF);	//clear
		WriteTW88(REG003, 0xEC);	//release Video, but still block SYNC
	}
	else
	{
		WriteTW88(REG003, 0xEE);	//block.
		WriteTW88(REG002, 0xFF);	//clear
		WriteTW88(REG004, 0xFF);	//clear

#ifdef DEBUG_ISR
		//copy
		temp_INT_STATUS = INT_STATUS;
		temp_VH_Loss_Changed = VH_Loss_Changed;
		temp_INT_STATUS2 = INT_STATUS2;
#endif
		//clear
		INT_STATUS = 0;
		VH_Loss_Changed = 0;
		INT_STATUS2 = 0;
#ifdef DEBUG_ISR
		if(temp_INT_STATUS+temp_VH_Loss_Changed+temp_INT_STATUS2)
			dPrintf("\nclear INT_STATUS:%bx INT_STATUS2:%bx VH_Loss_Changed:%d",temp_INT_STATUS,temp_INT_STATUS2,temp_VH_Loss_Changed);
#endif
	}

	WriteTW88Page(page);
}

//-----------------------------------------------------------------------------
/**
* Turn off/on SYNC Interrupt mask.
*
* @see Interrupt_enableVideoDetect
*/
void Interrupt_enableSyncDetect(BYTE fOn)
{
	DECLARE_LOCAL_page
#ifdef DEBUG_ISR
	BYTE temp_INT_STATUS, temp_INT_STATUS2;
#endif

	ReadTW88Page(page);
	WriteTW88Page(PAGE0_GENERAL);
	if(fOn) {
		WriteTW88(REG002, 0x04);	//clear
		WriteTW88(REG004, 0x06);	//clear
		WriteTW88(REG003, ReadTW88(REG003) & ~0x04);	//release

		SW_Video_Status = 1;
		//dPrintf("\nSW_Video_Status:%bd",SW_Video_Status);
	}
	else {
		WriteTW88(REG003, ReadTW88(REG003) | 0x04);	//block
		WriteTW88(REG002, 0x04);	//clear
		WriteTW88(REG004, 0x06);	//clear

#ifdef DEBUG_ISR
		//copy
		temp_INT_STATUS = INT_STATUS;
		temp_INT_STATUS2 = INT_STATUS2;
#endif
		//clear
		INT_STATUS &= ~0x04;
		INT_STATUS2 &= ~0x06;
#ifdef DEBUG_ISR
		if( (temp_INT_STATUS != INT_STATUS) || (temp_INT_STATUS2 != INT_STATUS2))
			dPrintf("\nclear SYNC at INT_STATUS:%bx INT_STATUS2:%bx",temp_INT_STATUS,temp_INT_STATUS2);
#endif
	}
	WriteTW88Page(page);
}

//-----------------------------------------------------------------------------
/**
* do interrupt polling
*
* Read interrupt global value that changed on interrupt service routine,
* and print status.
*
* @see ext0_int
* @see InterruptHandlerRoutine
*
*
* Interrupt Handler Routine 
* ==========================
* use InterruptPollingRoutine first
* @see ext0_int
* @see InterruptPollingRoutine
*
*/
void InterruptPollingHandlerRoutine(void)
{
	DECLARE_LOCAL_page
	BYTE temp_INT_STATUS_ACC;
	BYTE temp_INT_STATUS2_ACC;

	BYTE ret;
	BYTE r004;
	BYTE not_detected;

	WORD main_VH_Loss_Changed;
	BYTE main_INT_STATUS;
	BYTE main_INT_MASK;
	BYTE main_INT_STATUS2;
	//BYTE main_INT_STATUS3;	//for SW 7FF. ext4 intr



	//------------ Chip Interrupt --------------	
	SFR_EX0 = 0; 	//disable INT0
	//--copy
	main_INT_STATUS  = INT_STATUS;
	main_INT_MASK    = INT_MASK;         
	main_INT_STATUS2 = INT_STATUS2;
	main_VH_Loss_Changed = VH_Loss_Changed;
	temp_INT_STATUS_ACC = INT_STATUS_ACC;
	temp_INT_STATUS2_ACC = INT_STATUS2_ACC;
	//--clear
	INT_STATUS = 0;			//can be removed
	INT_STATUS2 = 0;		//can be removed
	VH_Loss_Changed = 0;
	INT_STATUS_ACC = 0;
	INT_STATUS2_ACC = 0;
	SFR_EX0 = 1;	//enable INT0

	//mask
	main_INT_STATUS &= ~main_INT_MASK;

	//
	// print INT debug message
	//
	if(main_INT_STATUS & 0x07) {
		ePrintf("\nInterrupt !!! R002[%02bx] ", main_INT_STATUS);
		//adjust from _ACC
		if(main_INT_STATUS != temp_INT_STATUS_ACC) {
			temp_INT_STATUS_ACC	&= ~main_INT_MASK;
			if(main_INT_STATUS != temp_INT_STATUS_ACC) {
				ePrintf(" [ACC:%02bx]", temp_INT_STATUS_ACC);
				if(temp_INT_STATUS_ACC & 0x01)
					main_INT_STATUS |= 0x01;				//NOTE
			}
		}
		ePrintf(" R003[%bx]",main_INT_MASK);
		ePrintf(" R004[%02bx] ", main_INT_STATUS2);
		//adjust from _ACC
		if(main_INT_STATUS2 != temp_INT_STATUS2_ACC) {
			ePrintf(" [ACC2:%02bx]", temp_INT_STATUS2_ACC);
			if((main_INT_MASK & 0x04) == 0) {
				main_INT_STATUS2 |= (temp_INT_STATUS2_ACC & 0x06);	//NOTE
				ePrintf("->[%02bx]",main_INT_STATUS2);	
			}
		}

	}
	//if( main_INT_STATUS & 0x80 ) ePrintf("\n   - SPI-DMA completion ");
	//if( main_INT_STATUS & 0x40 ) ePrintf("\n   - V display end ");
	//if( main_INT_STATUS & 0x20 ) ePrintf("\n   - Measurement Ready ");
	//if( main_INT_STATUS & 0x08 ) ePrintf("\n   - VSync leading edge ");

	//if( main_INT_STATUS & 0x04 ) {
	//	ePrintf("\n   - Sync Changed ");
	//	if(main_INT_STATUS2 & 0x02) ePrintf(" - HSync changed ");
	//	if(main_INT_STATUS2 & 0x04) ePrintf(" - VSync changed ");
	//}

	if( (main_INT_STATUS & 0x04 ) && (main_INT_STATUS2 & 0x04)) {
		ePrintf("\n   - Sync Changed ");
		ePrintf(" - VSync changed ");
	}			

	if(main_VH_Loss_Changed) {		//INT_STATUS[1] use accumulated VH_Loss_Changed value.
		//Video change happen.
		ePrintf("\n   - V/H Loss Changed:%d ", main_VH_Loss_Changed);
		if(main_INT_STATUS2 & 0x01)	ePrintf(" - Video Loss ");
		else						ePrintf(" - Video found ");
	}
	if(main_INT_STATUS & 0x01) {
		//Printf("\nR00F[%02bx]",ReadTW88(REG00F));
		if(SW_INTR_cmd == SW_INTR_VIDEO_CHANGED)
			dPrintf("\n*****SW_INTR_VIDEO_CHANGED");
#if defined(SUPPORT_I2CCMD_SLAVE_V1)
		else {
			BYTE cmd;
			cmd = ReadTW88(REG00F);
			if(cmd & SW_INTR_EXTERN) {
				if(cmd & I2CCMD_CHECK) {
					WriteTW88(REG009, 0xA1);
					//return;	
				}
				else if(cmd & I2CCMD_EXEC) {
					F_i2ccmd_exec = 1;	//request loop routine
					WriteTW88(REG009, 0xA1);
					//return;						
				}
			}	
		}		
#endif
	}

	//----------------------------------------
	// now, We uses 
	//	main_INT_STATUS
	//  main_INT_STATUS2
	//  main_VH_Loss_Changed


	//========================================
	// Handler routine
	//========================================

	ReadTW88Page(page);
	if(main_INT_STATUS & 0x01) {
		if(SW_INTR_cmd == SW_INTR_VIDEO_CHANGED) {
			SW_INTR_cmd = 0;

			LedBackLight(OFF);
			ScalerSetMuteManual( ON );

			//start negotiation right now
			TaskNoSignal_setCmd(TASK_CMD_RUN_FORCE);		
		}
		else {
			//assume external MCU requests interrupts.
			//read DMA buffer registers that the external MCU write the commmand.
			//we need a pre-defined format
			//execute

		}
		//NOTE:TASK_CMD_RUN2 can be replaced on the following condition. LedBackLight(OFF) can make a problem.
	}
	//Check SYNCH first and than check VDLoss. VDLoss will have a high priority.
	if( main_INT_STATUS & 0x04 ) {
		//check only VSync.	I have too many HSync.
		//service SYNC only when we have a video.
		if(( (main_INT_STATUS2 & 0x05) == 0x04 ) && (Task_NoSignal_cmd==TASK_CMD_DONE || Task_NoSignal_cmd==TASK_CMD_WAIT_MODE)) {
			if(InputMain==INPUT_CVBS || InputMain==INPUT_SVIDEO) {
				dPrintf("\n*****SYNC CHANGED");

				ret=DecoderReadDetectedMode();
				not_detected = ret & 0x08 ? 1 : 0;	//if not_detected is 1, not yet detected(in progress).
				ret &= 0x07;
				dPrintf(" InputSubMode %bd->%bd",InputSubMode,ret);
				if(not_detected || (ret == 7)) {
					dPrintf(" WAIT");
					TaskNoSignal_setCmd(TASK_CMD_WAIT_MODE);
				}
				else if(InputSubMode != ret) {
					dPrintf(" NEGO");
					LedBackLight(OFF);
					ScalerSetMuteManual( ON );
	
					//start negotiation	right now
					TaskNoSignal_setCmd(TASK_CMD_RUN_FORCE);
				}
				else
					dPrintf(" SKIP");
			}
#ifdef SUPPORT_COMPONENT
			else if(InputMain==INPUT_COMP) {
				dPrintf("\n*****SYNC CHANGED");
	
				ret = aRGB_GetInputStatus();	//detected input.
				not_detected = ret & 0x08 ? 0:1;	 //if not_detected is 1, not yet detected.
				ret &= 0x07;
				dPrintf(" InputSubMode %bd->%bd",InputSubMode,ret);
				if(not_detected || (ret == 7)) {
					dPrintf(" WAIT");
					TaskNoSignal_setCmd(TASK_CMD_WAIT_MODE);
				}
				else if(InputSubMode != ret) {
					dPrintf(" NEGO");
					LedBackLight(OFF);
					ScalerSetMuteManual( ON );
	
					//start negotiation right now
					TaskNoSignal_setCmd(TASK_CMD_RUN_FORCE);
				}
				else
					dPrintf(" SKIP");
			}
#endif
#ifdef SUPPORT_PC
			else if(InputMain==INPUT_PC) {
				//Need to verify.
	
				//-------------------------------
				// Video Signal is already changed. I can not use a FreeRun with FOSD message.
				//WaitVBlank(1);
				LedBackLight(OFF);
				ScalerSetMuteManual( ON );
	
				dPrintf("\n*****SYNC CHANGED");
				dPrintf(" NEGO");

				//start negotiation	right now
				TaskNoSignal_setCmd(TASK_CMD_RUN_FORCE);
			}
#endif
			else {
				//Need to verify.

				//-------------------------------
				// Video Signal is already changed. I can not use a FreeRun with FOSD message.
				//WaitVBlank(1);
				LedBackLight(OFF);
				ScalerSetMuteManual( ON );

				dPrintf("\n*****SYNC CHANGED");
				dPrintf(" NEGO");

				//start negotiation	right now
				TaskNoSignal_setCmd(TASK_CMD_RUN_FORCE);
			}
		}
	}


	if(main_VH_Loss_Changed) {		//INT_STATUS[1] use accumulated VH_Loss_Changed value.
		//Video change happen.
		main_VH_Loss_Changed = 0;
		if(InputMain==INPUT_PC) {
			;
		}
		else {
			//------------------------
			//read INT_STATUS2 value from HW.
			//ReadTW88Page(page);
			WriteTW88Page(PAGE0_GENERAL);
			r004 = ReadTW88(REG004);
			//WriteTW88Page(page);
			if(((main_INT_STATUS2 ^ r004) & 0x01) == 0x01) {						
				ePrintf("\nWarning SW replace Video Loss");
			//	main_INT_STATUS2 ^= 0x01;
			}
		}
		//--OK, what is a current status
		if(main_INT_STATUS2 & 0x01) {
			//Video Loss Happen
			if(SW_Video_Status) {
				dPuts("\nVideo Loss Happen");
				//turn off SYNC							
				SW_Video_Status = 0;
				//dPrintf("\nSW_Video_Status:%bd",SW_Video_Status);

				Interrupt_enableSyncDetect(OFF);
#ifdef SUPPORT_COMPONENT
				if(InputMain == INPUT_COMP) {
					//Change to 0 for fast recover.
					aRGB_SetFilterBandwidth(0, 0);		
				}
#endif
				//free run
				ScalerCheckPanelFreerunValue();										
				ScalerSetFreerunManual(ON);	// turn on Free Run Manual
				ScalerSetMuteManual( ON );	// turn on Mute Manual
			
				//start "No Signal" blinking
				if(MenuGetLevel()==0) {
					FOsdIndexMsgPrint(FOSD_STR2_NOSIGNAL);
//#ifdef NOSIGNAL_LOGO
//					if(getNoSignalLogoStatus() == 0)
//						InitLogo1();						
//#endif
				}

				tic_task = 0;
				TaskNoSignal_setCmd(TASK_CMD_WAIT_VIDEO);	//block the negotiation until you have a Video Signal
			}
			else {
				//tic_task = 0;
				TaskNoSignal_setCmd(TASK_CMD_WAIT_VIDEO);	//block the negotiation until you have a Video Signal
			}
		}
		else {
			//Video Found Happen
			if(SW_Video_Status==0) {
				dPuts("\nVideo found Happen");
				SW_Video_Status = 1;
				//dPrintf("\nSW_Video_Status:%bd",SW_Video_Status);

				//turn ON SYNC
				//Interrupt_enableSyncDetect(ON);  not yet. turn on it after it decide the video mode.
			}						

			if(Task_NoSignal_cmd==TASK_CMD_DONE) {
				dPrintf("\n********RECHECK");
				tic_task = NOSIGNAL_TIME_INTERVAL;			//do it right now..	
			}
			else {
				tic_task=NOSIGNAL_TIME_INTERVAL - 500;		//wait 500ms. 100ms is too short.
			}
			//start negotiation
			TaskNoSignal_setCmd(TASK_CMD_RUN);	
		}
	}
	WriteTW88Page(page);
}

