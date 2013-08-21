/**
 * @file
 * Monitor.c 
 * @author Harry Han
 * @author YoungHwan Bae
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	Interface between TW_Terminal2 and Firmware.
*/

//*****************************************************************************
//
//								Monitor.c
//
//*****************************************************************************
//
//
//#include <intrins.h>

#include "config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "global.h"
#include "cpu.h"
#include "printf.h"	
#include "util.h"
#include "monitor.h"
#include "monitor_MCU.h"
#include "monitor_SPI.h"
#include "monitor_MENU.h"
#include "monitor_ChipTest.h"
		  
#include "i2c.h"
#include "spi.h"

#include "main.h"
#include "SOsd.h"
#include "FOsd.h"
#include "Measure.h"
#include "Settings.h"
#include "Remo.h"
#include "scaler.h"
#ifdef SUPPORT_DELTA_RGB
#include "DeltaRGB.h"
#endif
#include "InputCtrl.h"
#include "ImageCtrl.h"
#include "OutputCtrl.h"
#include "TouchKey.h"
#include "measure.h"
#include "HDMI_EP9351.h"
#include "HDMI_EP9553.h"

#include "Decoder.h"
#include "aRGB.h"
#include "DTV.h"
#include "EEPROM.H"

#include "SOsdMenu.h"
#include "DebugMsg.h"
#ifdef SUPPORT_HDMI_EP9351
#include "EP9351_RegDef.h"
#endif
#include "HDMI_EP907M.h"
#include "BT656.h"


		BYTE 	DebugLevel = 0;
XDATA	BYTE	MonAddress = TW88I2CAddress;	
XDATA	WORD	MonIndex;
XDATA	BYTE	MonIndexLen;
XDATA	BYTE	MonRdata, MonWdata;
XDATA	BYTE	monstr[50];				// buffer for input string
XDATA	BYTE 	*argv[12];
XDATA	BYTE	argc=0;
		bit		echo=1;
		bit		access=1;
XDATA	BYTE	SW_key;

#ifdef SUPPORT_UART1
XDATA	BYTE	mon1str[40];				// buffer for input string
XDATA	BYTE 	*argv1[10];
XDATA	BYTE	argc1=0;
#endif

// function declear
void Test_Checkclock2(void);
void Test_Checkclock(void);



//#ifdef MODEL_TW8836
//==============================================
// BT656 TEST routine
//==============================================

// BT601 use a seperate HSync & VSync
void Monitor_BT601(void)
{
	BYTE fDeInterlaced;

	if(argc < 2) {
		Printf("\nusage: BT601 {off|dec|argb|dtv|lvds|panel}");
		Printf("\n\tBT601 off			; off");
		Printf("\n\tBT601 dec [p]		; decoder. p:Deinterlaced. ");
		Printf("\n\tBT601 argb			; ");
		Printf("\n\tBT601 dtv			; ");
		Printf("\n\tBT601 lvds          ; ");
		Printf("\n\tBT601 panel         ; ");
		return;
	}
	if( !stricmp( argv[1], "off" ) ) {
		BT656_Enable(OFF);
	}
	else if(!stricmp( argv[1], "dec" )) {
		if(!stricmp( argv[1], "p" ))
			fDeInterlaced = 1;
		else
			fDeInterlaced = 0;

		BT656_SelectSource(BT656_SRC_DEC);
		BT656_Enable(ON);

/*
w ff 00			w ff 00
w 07 0a			w 07 0a   enable BT656
w 67 41			w 67 41	  source aRGB. use HSync for EAV

w ff 01			w ff 01
w 03 24			w 03 24	   ?? hidden 
w e9 41	LLCLK2		w e9 20    ;change clko polarity
w 05 16			w 05 00  w 05 01
w e6 e2
w 37 1d
w 38 61

w 02 49
w 06 06
w 07 0a
w 48 40			w 48 40
w 66 00
w 67 00
w 68 00


w ff 01
w 05 16

w ff 00

*/
		if(fDeInterlaced) {
			//w ff 01
			//w 37 1d
			//w 38 61
			BT656_A_DeInterlace_Set(29, 97);	//   0x1d, 0x61);

			//w ff 01
			//w 08 12
			//w 07 02
			//w 0a 0b
			//w 0b d1
			//w 09 f3
//??			DecoderSetHActive(0x2D1);	//720=>721??	
//??			DecoderSetVActive(0x0f3); 	//240=>243??
//??			DecoderSetHDelay(0x00B);
//??			DecoderSetVDelay(0x012);
			BT656_A_SelectOutput(1,0,1,2); 	//R105=0x16

			WriteTW88(REG166,0x00);
			WriteTW88(REG167,0x00);
			WriteTW88(REG168,0x00);

			BT656_A_SelectCLKO(1,OFF);
			BT656_A_SetLLCLK_Pol(OFF);
			BT656_A_Set_CK54(ON);

		}
		else {
			BT656_A_SelectOutput(0,0,0,1);	//R105=0x01


			BT656_A_SelectCLKO(0,OFF);
			BT656_A_SetLLCLK_Pol(OFF);
			BT656_A_Set_CK54(OFF);
		}
		BT656_D_SetRGB(ON);
		BT656_SelectSource(BT656_SRC_DEC);	 //??

		WriteTW88(REG067,0x41);
		WriteTW88(REG148, 0x40); //??

	}
	else if (!stricmp( argv[1], "aRGB" )) {
		BT656_SelectSource(BT656_SRC_ARGB);
		BT656_Enable(ON);
	}
	else if (!stricmp( argv[1], "dtv" )) {
		BT656_SelectSource(BT656_SRC_DTV);
		BT656_Enable(ON);
	}
	else if (!stricmp( argv[1], "lvds" )) {
		BT656_SelectSource(BT656_SRC_LVDS);
		BT656_Enable(ON);
	}
	else if (!stricmp( argv[1], "panel" )) {
		BT656_SelectSource(BT656_SRC_PANEL);
		BT656_Enable(ON);
	}
	else {
		Printf("\n invalid...");
	}
}
void Monitor_BT656(void)
{
	if(argc < 2) {
		Printf("\nusage: BT656 {off|dec|argb|dtv|lvds|panel}");
		Printf("\n\tBT656 off			; off");
		Printf("\n\tBT656 dec [p]		; decoder. p:Deinterlaced. ");
		Printf("\n\tBT656 argb			; ");
		Printf("\n\tBT656 dtv			; ");
		Printf("\n\tBT656 lvds          ; ");
		Printf("\n\tBT656 panel         ; ");
		return;
	}
	if( !stricmp( argv[1], "0" ) ) {
		TEMP_init_BT656(0);
	}
	else if( !stricmp( argv[1], "1" ) ) {
		TEMP_init_BT656(1);
	}
	else if( !stricmp( argv[1], "2" ) ) {
		TEMP_init_BT656(2);
	}
	else if( !stricmp( argv[1], "6" ) ) {
		TEMP_init_BT656(6);
	}
	else if( !stricmp( argv[1], "off" ) ) {
		BT656_Enable(OFF);
	}
	else if(!stricmp( argv[1], "dec" )) {
		//BT656_SelectSource(BT656_SRC_DEC);
		//BT656_Enable(ON);
		SetBT656Output(BT656_SRC_DEC);
	}
	else if (!stricmp( argv[1], "aRGB" )) {
		//BT656_SelectSource(BT656_SRC_ARGB);
		//BT656_Enable(ON);
		SetBT656Output(BT656_SRC_ARGB);
	}
	else if (!stricmp( argv[1], "dtv" )) {
		//BT656_SelectSource(BT656_SRC_DTV);
		//BT656_Enable(ON);
		SetBT656Output(BT656_SRC_DTV);
	}
	else if (!stricmp( argv[1], "lvds" )) {
		//BT656_SelectSource(BT656_SRC_LVDS);
		//BT656_Enable(ON);
		SetBT656Output(BT656_SRC_LVDS);
	}
	else if (!stricmp( argv[1], "panel" )) {
		//BT656_SelectSource(BT656_SRC_PANEL);
		//BT656_Enable(ON);
		SetBT656Output(BT656_SRC_PANEL);
	}
	else {
		Printf("\n invalid...");
	}

}

//#endif


#if defined(MODEL_TW8835_SLAVE) && defined(SUPPORT_I2CCMD_TEST)
#define ReadTscData_TEST(TscData) TscData = ReadTW88(REG0B2); TscData <<= 4; TscData += ReadTW88(REG0B3) 
extern void InterruptPollingHandlerRoutine(void);
/*
	init mode:
		0: Read Test.
		1: Read Test. with EA
		2: Read/Write test.
		3: Write test.
		4: Write multiple register.
		5: Read Test.
		6: Write Test
	step:
*/
void i2c_test_slave_conflict(void)
{
	DWORD StartTime;
	volatile BYTE volatile_value;
	WORD init;	//mode
	WORD step;	//duration

	init = a2i( argv[1] );
	step = a2i( argv[2] );
	

	StartTime = init; //skip warning
	StartTime = SystemClock;
	ePrintf("\nStartTime:%ld.%ldsec", StartTime/100, StartTime%100 );
	if(init==0) {
		//init:0 - Read test
		do {
			volatile_value=ReadTW88(REG310);
		} while ((SystemClock - StartTime) < (step*100)); //step*1sec
	}
	else if(init==1) {
		//init:1 - Read test with SFR_EA
		do {
		SFR_EA=0;
			volatile_value=ReadTW88(REG310);	
		SFR_EA=1;
		} while ((SystemClock - StartTime) < (step*100)); //step*1sec
	}
	else if(init==2) {
		//init:2 - Read/Write test
		do {
			volatile_value=ReadTW88(REG310);	
			WriteTW88(REG310,volatile_value);	
		} while ((SystemClock - StartTime) < (step*100)); //step*1sec
	}
	else if(init==3) {
		//init:3 - Write test
		do {
			WriteTW88(REG310,0x00);	
		} while ((SystemClock - StartTime) < (step*100)); //step*1sec
	}
	else if(init==4) {
		//init:4 - Write test
		do {
			WriteTW88(REG310,0x00);	
			WriteTW88(REG311,0x00);	
			WriteTW88(REG312,0x00);	
			WriteTW88(REG313,0x00);	
			WriteTW88(REG314,0x00);	
			WriteTW88(REG315,0x00);	
			WriteTW88(REG316,0x00);	
			WriteTW88(REG317,0x00);	
			WriteTW88(REG318,0x00);	
			WriteTW88(REG319,0x00);	
			WriteTW88(REG320,0x00);	
			WriteTW88(REG321,0x00);	
			WriteTW88(REG322,0x00);	
			WriteTW88(REG323,0x00);	
			WriteTW88(REG324,0x00);	
			WriteTW88(REG325,0x00);	
			WriteTW88(REG326,0x00);	
			WriteTW88(REG327,0x00);	
			WriteTW88(REG328,0x00);	
			WriteTW88(REG329,0x00);	
		} while ((SystemClock - StartTime) < (step*100)); //step*1sec
	}
	else if(init==5) {
		//init:5 Read test
		do {
			volatile_value=ReadTW88(REG4DC);
		} while ((SystemClock - StartTime) < (step*100)); //step*1sec
	}
	else if(init==6) {
		//ubut:6 
		do {
			WriteTW88(REG4DC,0xAB);	
		} while ((SystemClock - StartTime) < (step*100)); //step*1sec
	}
	else
		Puts("\Invalid parameter");

	StartTime = SystemClock;
	ePrintf("\nStartTime:%ld.%ldsec", StartTime/100, StartTime%100 );
	Puts("\nDone");
#if 0
	DWORD StartTime;
	StartTime = SystemClock;
	ePrintf("\nStartTime:%ld.%ldsec", StartTime/100, StartTime%100 );
	do {
		WriteTW88(REG310,0x00);	
	} while ((SystemClock - StartTime) < 1000); //10sec
	StartTime = SystemClock;
	ePrintf("\nStartTime:%ld.%ldsec", StartTime/100, StartTime%100 );
	Puts("\nDone");
#endif
#if 0
	WORD i,target,/*step,init,*/ value;
	volatile BYTE test;
	volatile WORD TscData;
	BYTE r003;

//	BYTE ii,jj,kk;

	//init = 1024;
	//step = 32;
	i = 0;
	target = 2048;
	for(i=0; i < 100000; i++) {
		value=init+ i*step;
		//Printf("\n%d:value %d",i,value);
		//Printf("  reverse:%d", target-value);
		//Printf(" R000: %bx",ReadTW88(REG000));
//EA=0;
		WriteTW88(REG115,0xAB);
		ReadTscData_TEST(TscData);

		test = TW8835_R11C;
		test = TW8835_R130;
		test = TW8835_R1C1;
		test = TW8835_R1CD;
		test = TW8835_R412;
		test = TW8835_R508;

		test = TW8835_R101;


//-----------------
//				InterruptPollingRoutine();
//-----------------
	ReadTW88Page(page);
	WriteTW88Page(PAGE0_GENERAL);
	r003 = ReadTW88(REG003);
	WriteTW88Page(page);

//--------------------
//				InterruptHandlerRoutine();
//-------------------
	ReadTW88Page(page);
 	WriteTW88Page(page);

//   	for(ii=0; ii< 255; ii++)
//		for(jj=0; jj< 255;jj++)
//			for(kk=0;kk < 255;kk++);
//				;

//EA=1;
		Printf("R101:%bx %x %bx ",test, TscData, r003);
		//(64-i)*4.6874
		if(value >= target)
			break;
	}
	Printf("\ni=%d",i);
#endif
#if 0
	BYTE ii,jj;
	WORD i;

	i = init; i = step;
	for(ii=0; ii<100;ii++) {
		for(jj=0; jj < 6; jj++) {
			if(INT_STATUS || VH_Loss_Changed ) {
				InterruptPollingHandlerRoutine();
			}
			CheckKeyIn();
			CheckRemo();
			DumpRegister(jj);
		}
		Printf("=========%bd",ii);
	}
#endif
}
#endif
#if defined(SUPPORT_I2CCMD_TEST)
/*
mode 0:	EA=0, read REG4D0 to REG4D8. If 0, stop.
mode 1:	EA=0. Write dummy value, and then read back. If not equal, stop.
mode 2: EA=0. Write dummy value, and then read back. If not equal, readback again. If still not equal, stop.
mode 3: EA=0. 
		Write and Readback REG009. If fail, stop.
		Write and Readback REG103. If fail, stop.
		Write and Readback REG281~288. If fail, stop.
		Write and Readback REG341~348. If fail, stop.
		Write and Readback REG4FA~4FE. If fail, stop.
*/
void i2ccmd_test(BYTE mode, WORD duration)
{
	DWORD StartTime;
	volatile BYTE vdata;
	//BYTE ii;
	BYTE k;
	BYTE wdata;
	BYTE error=0;
	WORD ii;
		BYTE wbuff[5];					

	StartTime = SystemClock;
	ePrintf("\nStartTime:%ld.%ldsec", StartTime/100, StartTime%100 );

	if(mode==0) {
		SFR_EA=0;
		for(ii=0; ii < duration; ii++) {
			for(k=0; k < 8; k++) {
				vdata = ReadTW88(REG4D0+k);
				if(vdata==0) {
					SFR_EA=1;
					Printf("\nFound 0 at %x:%02bx", ii,k);
					return;
				}
			}
					
			SFR_EA=1;
			Puts(".");
			SFR_EA=0;	
		}
		SFR_EA=1;
		return;
	}

	if(mode==1) {
		SFR_EA=0;
		for(ii=0; ii < duration; ii++) {
			for(k=0; k < 5; k++) {
				wdata = ((ii << 4) | (ii & 0x000F)) + k;
				WriteTW88(REG4FA+k, wdata);
				vdata = ReadTW88(REG4FA+k);
				if(vdata !=wdata) {
					SFR_EA=1;
					Printf("\nFound ERROR at %x:%02bx w:%bx r:%bx", ii,k, wdata,vdata);
					return;
				}
			}
			SFR_EA=1;
			Puts(".");
			SFR_EA=0;	
		}
		SFR_EA=1;
		return;
	}

	if(mode==2) {
		SFR_EA=0;
		for(ii=0; ii < duration; ii++) {
			for(k=0; k < 5; k++) {
				wdata = ((ii << 4) | (ii & 0x000F)) + k;
				WriteTW88(REG4FA+k, wdata);
				vdata = ReadTW88(REG4FA+k);
				if(vdata !=wdata) {
					SFR_EA=1;
					Printf("\nFound ERROR at %x:%02bx w:%bx r:%bx old:%bx", 
						ii,k, wdata,vdata,wbuff[k]);

					for(ii=0; ii < 0x8000; ii++) {
						vdata = ReadTW88(REG4FA+k);
						if(vdata == wdata) {
							Printf(" same at %d", ii);
							return;
						}
					}
					Printf(" fail");
					return;
				}
				wbuff[k] = wdata;
			}
			SFR_EA=1;
			Puts(".");
			SFR_EA=0;	
		}
		SFR_EA=1;
		return;
	}
	if(mode==3) {
		SFR_EA=0;
		ReadTW88Page(page);
		for(ii=0; ii < duration; ii++) {
			wdata = ((ii << 4) | (ii & 0x000F)) + k;
			//REG009
			WriteTW88Page(0);
			WriteTW88(REG009,wdata);
			vdata = ReadTW88(REG009);
			if(vdata !=wdata) {
				WriteTW88Page(page);
				SFR_EA=1;	
				Printf("\nFound ERROR at %x:%02bx REG009 w:%bx r:%bx ", 
					ii,k, wdata,vdata);
					return;
			}
			//REG103
 			WriteTW88Page(1);
			WriteTW88(REG103,wdata);
			vdata = ReadTW88(REG103);
			if(vdata !=wdata) {
				WriteTW88Page(page);
				SFR_EA=1;	
				Printf("\nFound ERROR at %x:%02bx REG103 w:%bx r:%bx ", 
					ii,k, wdata,vdata);
					return;
			}

			//-------------
			//REG281~288
			WriteTW88Page(2);
			for(k=1; k <= 8; k++) {
				wdata = ((ii << 4) | (ii & 0x000F)) + k;
				WriteTW88(REG280+k, wdata);
				vdata = ReadTW88(REG280+k);
				if(vdata !=wdata) {
					WriteTW88Page(page);
					SFR_EA=1;
					Printf("\nFound ERROR at %x:%02bx REG3%02bx w:%bx r:%bx", ii,k, (BYTE)(REG280+k), wdata,vdata);
					return;
				}
			}
			//-------------
			//REG341~348
			WriteTW88Page(3);
			for(k=1; k <= 8; k++) {
				wdata = ((ii << 4) | (ii & 0x000F)) + k;
				WriteTW88(REG340+k, wdata);
				vdata = ReadTW88(REG340+k);
				if(vdata !=wdata) {
					WriteTW88Page(page);
					SFR_EA=1;
					Printf("\nFound ERROR at %x:%02bx REG3%02bx w:%bx r:%bx", ii,k, (BYTE)(REG340+k), wdata,vdata);
					return;
				}
			}
			//-------------
			//REG4FA~4FE
			WriteTW88Page(4);
			for(k=0; k < 5; k++) {
				wdata = ((ii << 4) | (ii & 0x000F)) + k;
				WriteTW88(REG4FA+k, wdata);
				vdata = ReadTW88(REG4FA+k);
				if(vdata !=wdata) {
					WriteTW88Page(page);
					SFR_EA=1;
					Printf("\nFound ERROR at %x:%02bx w:%bx r:%bx", ii,k, wdata,vdata);
					return;
				}
			}
			SFR_EA=1;
			Puts(".");
			SFR_EA=0;	
		}
		WriteTW88Page(page);
		SFR_EA=1;
		return;
	}
}


#endif
//=============================================================================
//
//=============================================================================
/**
* print prompt
*/
void Prompt(void)
{
#ifdef BANKING
	if ( MonAddress == TW88I2CAddress )
		Printf("\n[B%02bx]MCU_I2C[%02bx]>", BANKREG, MonAddress);
	else
#else
	if ( MonAddress == TW88I2CAddress )
		Printf("\nMCU_I2C[%02bx]>", MonAddress);
	else
#endif
		Printf("\nI2C[%02bx]>", MonAddress);
}
#ifdef SUPPORT_UART1
void Prompt1(void)
{
	if ( MonAddress == TW88I2CAddress )
		Printf1("\niAP>");
	else
	Printf1("\nRS1_I2C[%02bx]>", MonAddress);
}
#endif

#if 0
void WaitUserInput(void)
{
	Printf("\nPress any key...");
	while ( !RS_ready() );
	Puts("\n");
}
#endif

void Mon_tx(BYTE ch)
{
	RS_tx(ch);
}
#ifdef SUPPORT_UART1
void Mon1_tx(BYTE ch)
{
	RS1_tx(ch);
}
#endif
//=============================================================================
//
//=============================================================================
/*
	format								description			function
	------								-----------			---------
	c I2CId								set I2C device ID.	SetMonAddress
	r idx								read data			MonReadI2CByte
	w idx data							write data			MonWriteI2CByte
	,									decrease 1			MonIncDecI2C
	<									decrease 10
	.									increase 1
	>									increase 10
	d idx_from [idx_to]					dump				MonDumpI2C
	(	slave index						read				MonNewReadI2CByte
	) slave idx data					write				MonNewWriteI2CByte
	& i2cid idx_from idx_to	dump		dump				MonNewDumpI2C
	b i2cid index startbit|endbit data	bitwise wirte		MonWriteBit
	wait reg mask result max_wait		wait				MonWait

	;									comment
	/									repeat last command
	`									repeat command without CR
*/

/**
* format: c I2cId
*/
void SetMonAddress(BYTE addr)
{
	MonAddress = addr;
}

/**
* format: r idx
*/
BYTE MonReadI2CByte(void)
{
	////////////////////////////////////
//	BYTE speed;
			
	if(argc < 2) {
		Printf("   --> Missing parameter !!!");
#ifdef DEBUG_UART
		DEBUG_dump_uart0();
#endif
		return 1;
	}

	MonIndex = a2h( argv[1] );
	MonIndexLen = TWstrlen(argv[1]); //index length. 8bit or more.

	if(MonAddress == TW88I2CAddress) {
		if(MonIndexLen <= 2)
			MonIndex |= ((WORD)ReadTW88Byte(0xff) << 8);	
		MonRdata = ReadTW88(MonIndex);
	}
	else {
//		speed = SetI2CSpeed(I2C_SPEED_SLOW);
		if(MonIndexLen <= 2)
			MonRdata = ReadI2CByte(MonAddress, (BYTE)MonIndex);		//8bit index
		else
			MonRdata = ReadI2CI16Byte(MonAddress, MonIndex);	//16bit index
//		SetI2CSpeed(speed);
	}


	if( echo ) {
		if(MonIndexLen <= 2)
			Printf("\nRead %02bxh:%02bxh", (BYTE)MonIndex, MonRdata);	
		else
			Printf("\nRead %04xh:%02bxh", MonIndex, MonRdata);	
	}
	//save
	MonWdata = MonRdata;
	return 0;
}

#if 0
//!xdata	BYTE B1 _at_ 0xC412;
//!void MonReadInternalReg(void)
//!{
//!	WORD MonReg;
//!	BYTE B0;
//!	BYTE page;
//!
//!	if( argc !=2 ) {
//!		Printf("   --> Missing parameter !!!");
//!		return;
//!	}
//!
//!	MonReg = a2h( argv[1] );
//!
//!	ReadTW88Page(page);
//!	SFR_CAMC = 1;
//!	MonRdata = ReadTW88(MonReg);
//!	SFR_CAMC = 0;
//!	Printf("\nCurrPage:%bd",ReadTW88Page());
//!	WriteTW88Page(page);
//!	if( echo )
//!		Printf("\nRead %03xh:%02bxh", MonReg, MonRdata);	
//!
//!
//!	//test
//!	ReadTW88Page(page);
//!	WriteTW88Page(1 );
//!	B0 = TW8835_R412;
//!	Printf("==test B0:%bx ",B0);			
//!	SFR_CAMC = 1;
//!	B0 = B1;
//!	SFR_CAMC = 0;
//!	WriteTW88Page(page);
//!	Printf("==test B0:%bx ",B0);			
//!
//!}
#endif

/**
* write
* format: w idx data
*/
BYTE MonWriteI2CByte(void) 
{
	////////////////////////////////////
//	BYTE speed;

	if( argc<3 ) {
		Printf("   --> Missing parameter !!!");
#ifdef DEBUG_UART
		DEBUG_dump_uart0();
#endif
		return 1;
	}
	MonIndex = a2h( argv[1] );
	MonIndexLen = TWstrlen(argv[1]); //index length. 8bit or more.
	MonWdata = a2h( argv[2] );

	if(echo) {
		if(MonIndexLen <= 2)
			Printf("\nWrite %02bxh:%02bxh ", (BYTE)MonIndex, MonWdata);
		else
			Printf("\nWrite %04xh:%02bxh ", MonIndex, MonWdata);
	}

	if(MonAddress == TW88I2CAddress) {
		if(MonIndexLen <= 2)
			MonIndex |= ((WORD)ReadTW88Byte(0xff) << 8);	
		WriteTW88(MonIndex, MonWdata);
	}
	else {
		if(MonIndexLen <= 2)
			WriteI2CByte(MonAddress, (BYTE)MonIndex, MonWdata);	//8bit index
		else
			WriteI2CI16Byte(MonAddress, MonIndex, MonWdata);	//16bit index
	}

	if(echo==0)
		return 0;

	if(MonAddress == TW88I2CAddress) {
		MonRdata = ReadTW88(MonIndex);
	}
	else {
		if(MonIndexLen <= 2)
			MonRdata = ReadI2CByte(MonAddress, (BYTE)MonIndex);		//8bit index
		else
			MonRdata = ReadI2CI16Byte(MonAddress, MonIndex);	//16bit index
	}
	if(MonIndexLen <= 2)
   		Printf("==> Read %02bxh:%02bxh", (BYTE)MonIndex, MonRdata);
	else
   		Printf("==> Read %04xh:%02bxh", MonIndex, MonRdata);
	if( MonWdata != MonRdata ) Printf(" [%02bx]", MonWdata);
	return 0;
}
#if 0
//!void MonWriteInternalReg(void)
//!{
//!	WORD MonReg;
//!	if( argc !=3 ) {
//!		Printf("   --> Missing parameter !!!");
//!		return;
//!	}
//!
//!	MonReg = a2h( argv[1] );
//!	MonWdata = a2h( argv[2] );
//!	if( echo )
//!		Printf("\nWrite %03xh:%02bxh ", MonReg, MonWdata);
//!	TW8835WriteReg(MonReg, MonWdata);
//!	MonRdata = TW8835ReadReg(MonReg);
//!	if( echo )
//!		Printf("==> Read %02bxh", MonRdata);
//!}
#endif

/**
* increase/decrease value
*
* @param inc 0:decrease 1:increase 10:decrease 10 value. 11:increase 10 value
*
* format
* decrease 1:  ,
* decrease 10: <
* increase 1:  .
* increase 10: >
*
* extern
*	MonIndex,MonWdata
*/
void MonIncDecI2C(BYTE inc)
{
	switch(inc){
		case 0:  MonWdata--;		break;
		case 1:  MonWdata++;		break;
		case 10: MonWdata-=0x10;	break;
		case 11: MonWdata+=0x10;	break;
	}

	if ( MonAddress == TW88I2CAddress ) {
		WriteTW88(MonIndex, MonWdata);
		MonRdata = ReadTW88(MonIndex);
	}
	else {
		//BK120822: It can not detect 8bit/16bit index mode.
		//          Use previouse MonIndexLen.
		if(MonIndexLen <= 2) {
			WriteI2CByte(MonAddress, MonIndex, MonWdata);
			MonRdata = ReadI2CByte(MonAddress, MonIndex);
		}
		else {
			WriteI2CI16Byte(MonAddress, MonIndex, MonWdata);
			MonRdata = ReadI2CI16Byte(MonAddress, MonIndex);
		}
	}

	if( echo ) {
		//if(MonIndexLen <= 2) {
			Printf("Write %02bxh:%02bxh ", (BYTE)MonIndex, MonWdata);
			Printf("==> Read %02bxh:%02bxh", (BYTE)MonIndex, MonRdata);
		//}
		//else {
		//	Printf("Write %02bxh:%02bxh ", MonIndex, MonWdata);
		//	Printf("==> Read %02bxh:%02bxh", MonIndex, MonRdata);
		//}
	}

	Prompt();
}
/**
* dump
*
* format: d idx_from [idx_to]
*/
void MonDumpI2C(void)
{
	BYTE ToMonIndex;
	int  cnt=7;

	if( argc>=2 ) {
		MonIndex   = a2h(argv[1]);
		MonIndexLen = TWstrlen(argv[1]);
	}
	if( argc>=3 )	ToMonIndex = (BYTE)a2h(argv[2]);
	else			ToMonIndex = MonIndex+cnt;
	
	if ( ToMonIndex < (BYTE)MonIndex ) ToMonIndex = 0xFF;
	cnt = ToMonIndex - (BYTE)MonIndex + 1;

	if( echo ) {
		if ( MonAddress == TW88I2CAddress ) {
			if(MonIndexLen <= 2)
				MonIndex |= ((WORD)ReadTW88Byte(0xff) << 8);	
			for ( ; cnt > 0; cnt-- ) {
				MonRdata = ReadTW88(MonIndex);
				Printf("\n==> Read %02bxh:%02bxh", (BYTE)MonIndex, MonRdata);
				MonIndex++;
			}
		}
		else {
			if(MonIndexLen <= 2) {
				for ( ; cnt > 0; cnt-- ) {
					MonRdata = ReadI2CByte(MonAddress, MonIndex);
					Printf("\n==> Read %02bxh:%02bxh", (BYTE)MonIndex, MonRdata);
					//if( MonWdata != MonRdata ) Printf(" [%02bx]", MonWdata);
					MonIndex++;
				}
			}
			else {
				for ( ; cnt > 0; cnt-- ) {
					MonRdata = ReadI2CI16Byte(MonAddress, MonIndex);
					Printf("\n==> Read %02bxh:%02bxh", (BYTE)MonIndex, MonRdata);
					//if( MonWdata != MonRdata ) Printf(" [%02bx]", MonWdata);
					MonIndex++;
				}
			}
		}
	}
	else {
		if ( MonAddress == TW88I2CAddress ) {
			for ( ; cnt > 0; cnt-- ) {
				MonRdata = ReadTW88(MonIndex);
				MonIndex++;
			}
		}
		else {
			if(MonIndexLen <= 2) {
				for ( ; cnt > 0; cnt-- ) {
					MonRdata = ReadI2CByte(MonAddress, MonIndex);
					MonIndex++;
				}
			}
			else {
				for ( ; cnt > 0; cnt-- ) {
					MonRdata = ReadI2CI16Byte(MonAddress, MonIndex);
					MonIndex++;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
/**
* new read
* 
* format: "(" slave index
*/
void MonNewReadI2CByte(void)
{
	BYTE Slave;
#ifdef DEBUG_UART
	BYTE i;
#endif

	if( argc <3 ) {
		Printf("   --> Missing parameter !!!");
#ifdef DEBUG_UART
		for(i=0; i < 20; i++) {
			Printf(" %02bx",monstr[i]);
		}
#endif
		return;
	}

	Slave = a2h(argv[1]);
	MonIndex = a2h( argv[2] );
	MonIndexLen = TWstrlen(argv[2]);


	if ( Slave == TW88I2CAddress ) {
		if(MonIndexLen <= 2)
			MonIndex |= ((WORD)ReadTW88Byte(0xff) << 8);
		MonRdata = ReadTW88(MonIndex);
	}	
	else {
		if(MonIndexLen <= 2)
			MonRdata = ReadI2CByte(Slave, (BYTE)MonIndex);
		else
			MonRdata = ReadI2CI16Byte(Slave, MonIndex);
	}
	if( echo ) {
		//if(MonIndexLen <= 2)
			Printf("\n<R>%02bx[%02bx]=%02bx", Slave, (BYTE)MonIndex, MonRdata);
		//else
		//	Printf("\n<R>%02bx[%04x]=%02bx", Slave, MonIndex, MonRdata);
	}
	MonWdata = MonRdata;
}

/**
* write
* format: ")" slave idx data
*/
void MonNewWriteI2CByte(void)
{
	BYTE Slave;

	if( argc<4 ) {
		Printf("   --> Missing parameter !!!");
#ifdef DEBUG_UART
		DEBUG_dump_uart0();
#endif
		return;
	}
	
	Slave    = a2h( argv[1] );
	MonIndex = a2h( argv[2] );
	MonWdata = a2h( argv[3] );
	MonIndexLen = TWstrlen(argv[2]);

	
	if ( Slave == TW88I2CAddress ) {
		if(MonIndexLen <= 2)
			MonIndex |= ((WORD)ReadTW88Byte(0xff) << 8);
		WriteTW88(MonIndex, MonWdata);
		MonRdata = ReadTW88(MonIndex);

	}
	else {
		if(MonIndexLen <= 2) {
			WriteI2CByte(Slave, (BYTE)MonIndex, MonWdata);
			MonRdata = ReadI2CByte(Slave, (BYTE)MonIndex);
		}
		else {
			WriteI2CI16Byte(Slave, MonIndex, MonWdata);
			MonRdata = ReadI2CI16Byte(Slave, MonIndex);
		}
   	}
	if( echo ) {
		//if(MonIndexLen <= 2)
			Printf("\n<R>%02bx[%02bx]=%02bx", Slave, (BYTE)MonIndex, MonRdata);
		//else
		//	Printf("\n<R>%02bx[%04x]=%02bx", Slave, MonIndex, MonRdata);
	}
}
/**
* new dump
*
* format: "&" i2cid idx_from idx_to
*/
void MonNewDumpI2C(void)
{
	WORD 	ToMonIndex;
	BYTE    Slave;
	WORD	i;
	
	if( argc>=2 ) {
		MonIndex = a2h(argv[2]);
		MonIndexLen = TWstrlen(argv[2]);
	}
	if( argc>=3 ) ToMonIndex = a2h(argv[3]);
	Slave = a2h(argv[1]);

	if( echo ) {
		if ( Slave == TW88I2CAddress ) {
			if(MonIndexLen <= 2)
				MonIndex |= ((WORD)ReadTW88Byte(0xff) << 8);
			ToMonIndex |= (MonIndex & 0xFF00);
			for(i=MonIndex; i<=ToMonIndex; i++) {
				MonRdata = ReadTW88(i);
        		Printf("\n<R>%02bx[%02x]=%02bx", Slave, i, MonRdata);
			}
		}
		else {
			if(MonIndexLen <= 2) {
				for(i=MonIndex; i<=ToMonIndex; i++) {
					MonRdata = ReadI2CByte(Slave, i);
	        		Printf("\n<R>%02bx[%02x]=%02bx", Slave, i, MonRdata);
				}
			}
			else {
				ToMonIndex |= (MonIndex & 0xFF00);
				for(i=MonIndex; i<=ToMonIndex; i++) {
					MonRdata = ReadI2CI16Byte(Slave, i);
	        		Printf("\n<R>%02bx[%02x]=%02bx", Slave, i, MonRdata);
				}
			}
		}
	}
	else {
		if ( Slave == TW88I2CAddress ) {
			if(MonIndexLen <= 2)
				MonIndex |= ((WORD)ReadTW88Byte(0xff) << 8);
			for(i=MonIndex; i<=ToMonIndex; i++)
				MonRdata = ReadTW88(i);
		}
		else {
			if(MonIndexLen <= 2) {
				for(i=MonIndex; i<=ToMonIndex; i++) {
					MonRdata = ReadI2CByte(Slave, i);
				}
			}
			else {
				ToMonIndex |= (MonIndex & 0xFF00);
				for(i=MonIndex; i<=ToMonIndex; i++) {
					MonRdata = ReadI2CI16Byte(Slave, i);
				}
			}
		}
	}
}

/**
* bitwise wirte
*
* format:
* 	b slave index startbit|endbit data
*/
void MonWriteBit(void)
{
	BYTE mask, i, FromBit, ToBit,  MonMask, val;
	BYTE Slave;

	if( argc<5 ) {
		Printf("   --> Missing parameter !!!");
#ifdef DEBUG_UART
		DEBUG_dump_uart0();
#endif
		return;
	}
	Slave = a2h(argv[1]);

	MonIndex = a2h( argv[2] );
	MonIndexLen = TWstrlen(argv[2]);
	FromBit  =(a2h( argv[3] ) >> 4) & 0x0f;
	ToBit    = a2h( argv[3] )  & 0x0f;
	MonMask  = a2h( argv[4] );

	if( FromBit<ToBit || FromBit>7 || ToBit>7) {
		Printf("\n   --> Wrong range of bit operation !!!");
		return;
	}
	
	mask = 0xff; 
	val=0x7f;
	for(i=7; i>FromBit; i--) {
		mask &= val;
		val = val>>1;
	}

	val=0xfe;
	for(i=0; i<ToBit; i++) {
		mask &= val;
		val = val<<1;
	}

	if ( Slave == TW88I2CAddress ) {
		if(MonIndexLen <= 2)
			MonIndex |= ((WORD)ReadTW88Byte(0xff) << 8);
			
		MonRdata = ReadTW88(MonIndex);
		MonWdata = (MonRdata & (~mask)) | (MonMask & mask);
					
		WriteTW88(MonIndex, MonWdata);
		MonRdata = ReadTW88(MonIndex);
	}
	else {
		if(MonIndexLen<=2) {
			MonRdata = ReadI2CByte(Slave, MonIndex);
			MonWdata = (MonRdata & (~mask)) | (MonMask & mask);
					
			WriteI2CByte(Slave, MonIndex, MonWdata);
			MonRdata = ReadI2CByte(Slave, MonIndex);
		}
		else {
			MonRdata = ReadI2CI16Byte(Slave, MonIndex);
			MonWdata = (MonRdata & (~mask)) | (MonMask & mask);
					
			WriteI2CI16Byte(Slave, MonIndex, MonWdata);
			MonRdata = ReadI2CI16Byte(Slave, MonIndex);
		}
	}
	if( echo )
		//TW_TERMINAL need this syntax
		Printf("\n<R>%02bx[%02bx]=%02bx", Slave, (BYTE)MonIndex, MonRdata);
}
/**
* wait reg mask result max_wait
*/
void MonWait(void)
{
	WORD i,max;
	BYTE reg, mask, result;
	if( argc<5 ) {
		Printf("   --> Missing parameter !!!");
#ifdef DEBUG_UART
		DEBUG_dump_uart0();
#endif
		return;
	}
	reg = a2h( argv[1] );
	mask = a2h( argv[2] );
	result = a2h( argv[3] );
	max = a2h( argv[4] );
	for(i=0; i < max; i++) {
		if((ReadTW88(reg) & mask)==result) {
			Printf("=>OK@%bd",i);
			break;
		}
		delay1ms(2);
	}
	if(i >= max)
		Printf("=>fail wait %bx %bx %bx %d->fail",reg,mask,result,max);
}

//=============================================================================
//			Help Message
//=============================================================================
void MonHelp(void)
{
	Puts("\n=======================================================");
	Puts("\n>>>     Welcome to Intersil Monitor  Rev 1.02       <<<");
	Puts("\n=======================================================");
	Puts("\n   R ii             ; Read data.(");
	Puts("\n   W ii dd          ; Write data.)");
	Puts("\n   D [ii] [cc]      ; Dump.&");
	Puts("\n   B AA II bb DD    ; Bit Operation. bb:StartEnd");
	Puts("\n   C aa             ; Change I2C address");
	Puts("\n   Echo On/Off      ; Terminal Echoing On/Off");
	Puts("\n   HDMI [init start dnedid dnhdcp avi timereg]");
	Puts("\n=======================================================");
	Puts("\n=== DEBUG ACCESS time init MCU SPI EE menu task [on] ====");
	Puts("\nM [CVBS|SVIDEO|COMP|PC|DVI|HDMI|BT656]      ; Change Input Mode");
	Puts("\nselect [CVBS|SVIDEO|COMP|PC|DVI|HDMI|BT656] ; select Input Mode");
	Puts("\ninit default                               : default for selected input");
	Puts("\nCheckAndSet                                ; CheckAndSet selected input");
	Puts("\nISR [ON|OFF]								; On/Off ISR");
	Puts("\n=======================================================");
#if defined(MODEL_TW8835_SLAVE) && defined(SUPPORT_I2CCMD_TEST)
	Puts("\ni2ctest mode duration");
	Puts("\n	mode:0	read");
	Puts("\n	mode:1	read with SFR_EA");
	Puts("\n	mode:2	read and write");
	Puts("\n	mode:3	write");
	Puts("\n	mode:4	write x 20 times");
	Puts("\n	mode:5	read R4DC");
	Puts("\n	mode:6	write R4DC=0xAB");
	Puts("\ni2ctestpage page           ; set compare page");
	Puts("\n=======================================================");
#endif
#if defined(SUPPORT_I2CCMD_TEST)
	Puts("\ni2ccmdtest mode duration");
	Puts("\n	mode:0	read REG4D0 to REG4D8. If 0, stop");
	Puts("\n	mode:1	Write dummy value, and then read back. If not equal, stop");
	Puts("\n	mode:2	Write dummy value, and then read back. If not equal, readback again. If still not equal, stop");
#endif
	Puts("\n");

}
#ifdef SUPPORT_UART1
void Mon1Help(void)
{
	Puts1("\n=======================================================");
	Puts1("\n>>>     Welcome to Intersil Monitor1  Rev 1.01       <<<");
	Puts1("\n=======================================================");
	Puts1("\n GE nn");
	Puts1("\n IR nn");
	Puts1("\n RE ii");
	Puts1("\n WE ii dd");
	Puts1("\n RA ii");
	Puts1("\n WA ii dd");
	Printf1("\nHELP or H or ?");
}
#endif
//=============================================================================
//
//=============================================================================
/**
* Mon GetCommand
*
* @return 0: nothing. 1: found command.
*/
BYTE MonGetCommand(void)
{
	static BYTE comment=0;
	static BYTE incnt=0, last_argc=0;
	BYTE i, ch;
	BYTE ret=0;

	if( !RS_ready() ) return 0;
	ch = RS_rx();

	//----- if comment, echo back and ignore -----
	if( comment ) {
		if( ch=='\r' || ch==0x1b ) comment = 0;
		else { 
			Mon_tx(ch);
			return 0;
		}
	}
	else if( ch==';' ) {
		comment = 1;
		Mon_tx(ch);
		return 0;
	}

	//=====================================
	switch( ch ) {
	//--- ESC
	case 0x1b:	
		argc = 0;
		incnt = 0;
		comment = 0;
		Prompt();
		return 0;

	//--- end of string
	case '\r':

		if( incnt==0 ) {
			Prompt();
			break;
		}

		monstr[incnt++] = '\0';
		argc=0;
		//ignore control char.
		for(i=0; i<incnt; i++) 
			if( monstr[i] > ' ' ) 
				break;

		if( !monstr[i] ) {
			incnt = 0;
			comment = 0;
			Prompt();
			return 0;
		}
		argv[0] = &monstr[i];
		for(; i<incnt; i++) {
			if( monstr[i] <= ' ' ) {
				monstr[i]='\0';
#ifdef DEBUG_UART
    			Printf("(%s) ",  argv[argc]);
#endif
				i++;
				while( monstr[i]==' ' || monstr[i]=='\t') i++;
				argc++;
				if( monstr[i] ){
     			 	argv[argc] = &monstr[i];
				}
			}
		}

		ret = 1;
		last_argc = argc;
		incnt = 0;
		break;

	//--- repeat command
	case '/':
		argc = last_argc;
		ret = 1;
		break;

	//--- repeat command without CR
	case '`':
	{
		BYTE i, j, ch;

		for(j=0; j<last_argc; j++) {
			for(i=0; i<100; i++) {
				ch = argv[j][i];
				if( ch==0 ) {
					if( j==last_argc-1 ) return 0;
					ch = ' ';
					RS_ungetch( ch );
					break;
				}
				else {
					RS_ungetch( ch );
				}
			}
		}
		break;
	}

	//--- back space
	case 0x08:
		if( incnt ) {
			incnt--;
			Mon_tx(ch);
			Mon_tx(' ');
			Mon_tx(ch);
		}
		break;

	//--- decreamental write
	case ',':
		if( incnt ) {
			Mon_tx(ch);
			monstr[incnt++] = ch;
		}
		else
			MonIncDecI2C(0);
		break;

	case '<':
		if( incnt ) {
			Mon_tx(ch);
			monstr[incnt++] = ch;
		}
		else
			MonIncDecI2C(10);
		break;

	//--- increamental write
	case '.':
		if( incnt ) {
			Mon_tx(ch);
			monstr[incnt++] = ch;
		}
		else
			MonIncDecI2C(1);
		break;

	case '>':
		if( incnt ) {
			Mon_tx(ch);
			monstr[incnt++] = ch;
		}
		else
			MonIncDecI2C(11);
		break;

	default:
		Mon_tx(ch);
		monstr[incnt++] = ch;
#ifdef DEBUG_UART
		if(incnt==50) {	   //BK130116
			Puts("###");  //overflow
		}
#endif
		break;
	}

	if( ret ) {
		comment = 0;
		last_argc = argc;
		return ret;
	}
	else {
		return ret;
	}
}

#ifdef SUPPORT_UART1
/**
* Mon1 GetCommand
*
*/
BYTE Mon1GetCommand(void)
{
	static BYTE comment1=0;
	static BYTE incnt1=0, last_argc1=0;
	BYTE i, ch;
	BYTE ret=0;

	if( !RS1_ready() ) return 0;
	ch = RS1_rx();

	//----- if comment, echo back and ignore -----
	if( comment1 ) {
		if( ch=='\r' || ch==0x1b ) comment1 = 0;
		else { 
			Mon1_tx(ch);
			return 0;
		}
	}
	else if( ch==';' ) {
		comment1 = 1;
		Mon1_tx(ch);
		return 0;
	}

	//=====================================
	switch( ch ) {

	case 0x1b:
		argc1 = 0;
		incnt1 = 0;
		comment1 = 0;
		Prompt1();
		return 0;

	//--- end of string
	case '\r':

		if( incnt1==0 ) {
			Prompt1();
			break;
		}

		mon1str[incnt1++] = '\0';
		argc1=0;

		for(i=0; i<incnt1; i++) if( mon1str[i] > ' ' ) break;

		if( !mon1str[i] ) {
			incnt1 = 0;
			comment1 = 0;
			Prompt1();
			return 0;
		}
		argv1[0] = &mon1str[i];
		for(; i<incnt1; i++) {
			if( mon1str[i] <= ' ' ) {
				mon1str[i]='\0';
     			 //Printf("(%s) ",  argv[argc]);
				i++;
				while( mon1str[i]==' ' ) i++;
				argc1++;
				if( mon1str[i] ){
     			 argv1[argc1] = &mon1str[i];
				}
			}
		}

		ret = 1;
		last_argc1 = argc1;
		incnt1 = 0;
		
		break;

	//--- repeat command
	case '/':
		argc1 = last_argc1;
		ret = 1;
		break;

	//--- repeat command without CR
	case '`':
	{
		BYTE i, j, ch;

		for(j=0; j<last_argc1; j++) {
			for(i=0; i<100; i++) {
				ch = argv1[j][i];
				if( ch==0 ) {
					if( j==last_argc1-1 ) return 0;
					ch = ' ';
					RS1_ungetch( ch );
					break;
				}
				else {
					RS1_ungetch( ch );
				}
			}
		}
		break;
	}

	//--- back space
	case 0x08:
		if( incnt1 ) {
			incnt1--;
			Mon1_tx(ch);
			Mon1_tx(' ');
			Mon1_tx(ch);
		}
		break;

	//--- decreamental write
	case ',':
		if( incnt1 ) {
			Mon1_tx(ch);
			mon1str[incnt1++] = ch;
		}
		//else
		//	MonIncDecI2C(0);	  //BKTODO??
		break;

	case '<':
		if( incnt1 ) {
			Mon1_tx(ch);
			mon1str[incnt1++] = ch;
		}
		//else
		//	MonIncDecI2C(10);	//??BKTODO??
		break;

	//--- increamental write
	case '.':
		if( incnt1 ) {
			Mon1_tx(ch);
			mon1str[incnt1++] = ch;
		}
		//else
		//	MonIncDecI2C(1);	//??BKTODO
		break;

	case '>':
		if( incnt1 ) {
			Mon1_tx(ch);
			mon1str[incnt1++] = ch;
		}
		//else
		//	MonIncDecI2C(11);	//BKTODO
		break;

	default:
		Mon1_tx(ch);
		mon1str[incnt1++] = ch;
		break;
	}

	if( ret ) {
		comment1 = 0;
		last_argc1 = argc1;
		return ret;
	}
	else {
		return ret;
	}
}
#endif

//*****************************************************************************
//				Monitoring Command
//*****************************************************************************

BYTE *MonString = 0;
extern CODE BYTE DataInitADC[];
extern CODE BYTE DataInitYUV[];
extern CODE BYTE DataInitNTSC[];
extern CODE BYTE DataInitDTV[];
extern CODE BYTE DataInitTCON[];

#if 1 // OSPOSD Move test
void WaitVBlank1(void)
{
	//XDATA	BYTE i;
	WORD loop;
	volatile BYTE vdata;

	WriteTW88Page(0);	//WriteI2CByte(0x8a,0xff,0x00);

//	PORT_DEBUG = 0;
	WriteTW88(REG002, 0xFF);	//WriteI2CByte( 0x8a,0x02, 0xff ); //clear
	loop = 0;
	while(1) {
		vdata = ReadTW88(REG002);	//vdata = ReadI2CByte( 0x8a,0x02 );
		//Printf("\nREG002:%bx", vdata);		
		if(vdata & 0x40) 
			break;
		loop++;
		if(loop > 0xFFFE) {
			ePrintf("\nERR:WaitVBlank");
			break;
		}
	}
//	PORT_DEBUG = 1;
	//Printf("\nWaitVBlank1 loop:%d", loop);
}
#endif

//extern void TestCrC8(void);	//test


void monitor_i2c(void)
{
	if(argc==1) {
		Printf("\nI2C {check|delay|BUS}");
		return;
	}
	if(!stricmp(argv[1],"check")) {
		BYTE id,temp;
		if(argc < 3) {
			Printf("\nI2C check devid");
			return;
		}
		id = (BYTE)a2h( argv[2] );
		temp=CheckI2C(id);
		Printf("\nresult:%bx",temp);
		Printf("  I2C_delay_base:%bd", I2C_delay_base);	
	}
	else if(!stricmp(argv[1],"bus")) {
	}
	//else if(!stricmp(argv[1],"delay")) {
	//	BYTE temp;
	//	if(argc < 4) {
	//		Printf("\ni2c delay start:%d",i2c_delay_start);
	//		Printf("\n          restart:%d",i2c_delay_restart);
	//		Printf("\n          datasetup:%d",i2c_delay_datasetup);
	//		Printf("\n          clockhigh:%d",i2c_delay_clockhigh);
	//		Printf("\n          datahold:%d",i2c_delay_datahold);
	//		return;
	//	}
	//	temp = (BYTE)a2h( argv[3] );
	//	if(!stricmp(argv[2],"start"))
	//		i2c_delay_start = temp;
	//	else if(!stricmp(argv[2],"restart"))
	//		i2c_delay_restart = temp;
	//	else if(!stricmp(argv[2],"datasetup"))
	//		i2c_delay_datasetup = temp;
	//	else if(!stricmp(argv[2],"clockhigh"))
	//		i2c_delay_clockhigh = temp;
	//	else if(!stricmp(argv[2],"datahold"))
	//		i2c_delay_datahold = temp;
	//	else 
	//		Printf("\nubkbown");
	//}  
}



/**
* monitor.
*
* get user command and execute it.
*/
void Monitor(void)
{
	WORD wValue;

	if( MonString ) {																				  
		RS_ungetch( *MonString++ );
		if( *MonString==0 ) MonString = 0;
	}

	if( !MonGetCommand() ) return;

	//---------------- Write Register -------------------
	// add W21 for dongle command
	if( !stricmp( argv[0], "W" ) || !stricmp( argv[0], "W21" )) {
		MonWriteI2CByte();
	}
	else if( !stricmp( argv[0], ")" ) ) {
		MonNewWriteI2CByte();
	}
	//---------------- Read Register --------------------
	// add R21 for dongle command
	else if ( !stricmp( argv[0], "R" ) || !stricmp( argv[0], "R21" ) ) {
		MonReadI2CByte();
	}
	else if ( !stricmp( argv[0], "(" ) ) {
		MonNewReadI2CByte();
	}
	//---------------- Dump Register --------------------
	else if( !stricmp( argv[0], "D" ) ) {
		Puts("\ndump start");
		MonDumpI2C();
	}
	else if( !stricmp( argv[0], "&" ) ) {
		MonNewDumpI2C();
	}

	//---------------- Bit Operation --------------------
	else if( !stricmp( argv[0], "B" ) ) {// Write bits - B AA II bb DD
		MonWriteBit();
	}
	//---------------- Change I2C -----------------------
	else if( !stricmp( argv[0], "C" ) ) {
		MonAddress = a2h( argv[1] );
		Printf("\nSetMonAddress:%d",__LINE__);
	}
	//---------------- wait -----------------------
	else if( !stricmp( argv[0], "wait" ) ) {
		MonWait();
	}
	//---------------- delay -----------------------
	else if( !stricmp( argv[0], "delay" ) ) {
		wValue = a2i( argv[1] );
		delay1ms(wValue);
	}
	//---------------- Help -----------------------------
	else if( !stricmp( argv[0], "H" ) || !stricmp( argv[0], "HELP" ) || !stricmp( argv[0], "?" ) ) {
		MonHelp();
	}


#if defined(REMO_RC5) && defined(DEBUG_REMO)
	else if(!stricmp( argv[0], "ddremo")) {
		PrintRemoCapture();
	}
#endif



	//---------------- HOST SPIOSD TEST  ------
#if 1 // SPIOSD Move test
	else if(!stricmp( argv[0], "move")) {
		WORD i;
		BYTE xpositionH,xpositionL;
		WORD delay;
		BYTE win;

		if(argc < 2) {
			win = 1;
			if(SFR_CACHE_EN) 	delay = 50;
			else				delay=10;
		}
		else {
			win = a2h(argv[1]);
			if(win==0)	win = 1;
			if(argc < 3)
				delay = a2i(argv[2]);

		}


		for(i = 0; i< 801; i+= 1) {
			xpositionH = i >>8;
			xpositionL = (BYTE)i & 0xff;
			
			WaitVBlank1();
			//TEST   delay1ms(15);
//			PORT_DEBUG = 0;
			WriteTW88Page(4);	//WriteI2CByte(0x8a,0xff, 0x04);
			WriteTW88(REG431+0x10*win, xpositionH);	//WriteI2CByte(0x8a,0x41, xpositionH);
			WriteTW88(REG432+0x10*win, xpositionL);	//WriteI2CByte(0x8a,0x42, xpositionL);
//			PORT_DEBUG = 1;

			delay1ms(delay);
		}
	}
#endif
#if 1
	else if(!stricmp( argv[0], "move1")) {
		WORD i;
		BYTE xpositionH,xpositionL;
		WORD delay;
		BYTE wait;

		if(argc < 2) {
			if(SFR_CACHE_EN) 	delay = 50;
			else				delay=10;
		}
		else
			delay = a2i(argv[1]);

		if(argc==3)
			wait=a2i(argv[2]);
		else
			wait=2;

		//for(i = 0; i< 801; i+= 1) {
		i = 0;
		while(!RS_ready()) {
			i++;
			i %= 2;
			xpositionH = i >>8;
			xpositionL = (BYTE)i & 0xff;
			
			WaitVBlank1();
			delay1ms(wait);
//			PORT_DEBUG = 0;
			WriteTW88Page(4);	//WriteI2CByte(0x8a,0xff, 0x04);
			WriteTW88(REG441, xpositionH);	//WriteI2CByte(0x8a,0x41, xpositionH);
			WriteTW88(REG442, xpositionL);	//WriteI2CByte(0x8a,0x42, xpositionL);
//			PORT_DEBUG = 1;

			delay1ms(delay);
		}
	}
	else if(!stricmp( argv[0], "move2")) {
		WORD i;
		BYTE xpositionH,xpositionL;
		WORD delay;
		BYTE wait;

		if(argc < 2) {
			if(SFR_CACHE_EN) 	delay = 50;
			else				delay=10;
		}
		else
			delay = a2i(argv[1]);

		if(argc==3)
			wait=a2i(argv[2]);
		else
			wait=2;

		//for(i = 0; i< 801; i+= 1) {
		i = 0;
		while(!RS_ready()) {
			i++;
			i %= 2;
			xpositionH = i >>8;
			xpositionL = (BYTE)i & 0xff;
			
			WaitVBlank1();
			//test::delay1ms(wait);
//			PORT_DEBUG = 0;
			WriteTW88Page(4);	//WriteI2CByte(0x8a,0xff, 0x04);
			WriteTW88(REG445, 0x20-i);	
//			PORT_DEBUG = 1;

			delay1ms(delay);
		}
	}
	else if(!stricmp( argv[0], "move3")) {
		WORD i;
		BYTE xpositionH,xpositionL;
		WORD delay;

		if(argc < 2) {
			if(SFR_CACHE_EN) 	delay = 50;
			else				delay=10;
		}
		else
			delay = a2i(argv[1]);

		//for(i = 0; i< 801; i+= 1) {
		i = 0;
		while(!RS_ready()) {
			i++;
			i %= 2;
			xpositionH = i >>8;
			xpositionL = (BYTE)i & 0xff;
			
			WaitVBlank1();
			//test::delay1ms(1);
//			PORT_DEBUG = 0;
			WriteTW88Page(4);	//WriteI2CByte(0x8a,0xff, 0x04);
			WriteTW88(REG461, xpositionH);	//WriteI2CByte(0x8a,0x41, xpositionH);
			WriteTW88(REG462, xpositionL);	//WriteI2CByte(0x8a,0x42, xpositionL);
//			PORT_DEBUG = 1;

			delay1ms(delay);
		}
	}
#endif
#ifdef DEBUG_UART
	//---------------- UART TEST  ------
	// to check the UART FIFO.
	//	"mcu uart"
	//	"uartdump" and hold "a" key for 5 sec and then press "x".
	//	type "mcu uart" and, check the UART0 Max value.
	//----------------------------------
	else if(!stricmp( argv[0], "UARTDUMP")) {
		BYTE	ch;

		do {
			SFR_ES = 0;			//disable SFR_ES 
			delay1ms(100);
			delay1ms(100);
			SFR_ES = 1;			//enable SFR_ES
			delay1ms(100);

			if( !RS_ready() ) continue;
			ch = RS_rx();
			Printf("%02bx ", ch );
		} while ( ch != 'x' );
	}
#endif

//#if 1
//	else if( !stricmp( argv[0], "crc8" ) ) {
//		TestCrC8();
//	}
//#endif

	//---------------- Read/Write Register for slow I2C  ------

	//---------------- Read/Write Register for internal register ------
#if 1
	//i2c debug routine
	else if( !stricmp( argv[0], "i2c" ) ) {
		monitor_i2c();
	}
#endif


	//---------------------------------------------------
/*
	else if( !stricmp( argv[0], "*" ) ) {
			
				if( argc==1 ) {
					Printf("\n  * 0 : Program default Loader");
					Printf("\n  * 1 : Program external Loader");
					Printf("\n  * 2 : Execute Loader");
				}
				else { 
					BYTE mode;
					mode = a2h(argv[1]);
					//Loader(mode);
				}
	}
*/
	//----------------------------------------------------
	//---------------- ISR ------------------------------
	//----------------------------------------------------
	else if(!stricmp( argv[0], "ISR" ) ) {
		if(argc < 2) {
			Printf("\nISR status:");
			if(SFR_EX0) Printf("EX0 ");						// ext0
			if(SFR_ET1) Printf("ET1 ");						// timer1 - touch
			if(SFR_ET2) Printf("ET2 ");						// timer2 - Remo
			if(SFR_E2IE) Printf("E2IE:%02bx ",SFR_E2IE);	// E2IE[4] - Remo				
		}
		else if(!stricmp( argv[1], "ON" )) {
			if(SFR_EX0) Printf("-->Skip");
			else {
				SFR_EX0 = 1;
#ifdef SUPPORT_TOUCH
				SFR_ET1 = 1;
#endif
				SFR_E2IE |= 0x04;
			}
		}	
		else if(!stricmp( argv[1], "OFF" ))	{
			if(SFR_EX0 == 0) Printf("-->Skip");
			else {
				SFR_EX0 = 0;
				SFR_ET1 = 0;
				SFR_E2IE = 0;
			}
		}
	}
	//----------------------------------------------------
	//---------------- Init ------------------------------
	//----------------------------------------------------
	else if(!stricmp( argv[0], "init" ) ) {
		if(argc < 2) {
			//InitWithNTSC();
			Init8836AsDefault(InputMain, 1);

			FP_GpioDefault();
			SSPLL_PowerUp(ON);
			//DCDC needs 100ms, but we have enough delay on...
			DCDC_StartUP();
	
			ScalerSetMuteManual(OFF);
			LedPowerUp();
		}
		else {
			if( !stricmp( argv[1], "?") || !stricmp( argv[1], "help") ) {
				Puts("\nInit {?|core|fpga|NTSC|panel|sspll|clock {H|L}|logo {on|off}|default}");

				Puts("\n\nExample:BootUp");
				Puts("\n init core");
				Puts("\n init | init NTSC");
				Puts("\n      | init panel");
				Puts("\n m");
				Puts("\n init logo");
				Puts("\n\nExample:change input");
				Puts("\n m [CVBS|SVIDEO|COMP|PC|DVI|HDMI|BT656]");
				Puts("\n or");
				Puts("\n input [CVBS|SVIDEO|COMP|PC|DVI|HDMI|BT656]");
				Puts("\n init default");
				Puts("\n checkandset");
			}
			else if( !stricmp( argv[1], "fpga" ) ) {	
#if defined(SUPPORT_I2CCMD_TEST_SLAVE)
				InitISR(0);	//InitCPU();
#endif
				InitCore(0);
				/*ee_mode = */ CheckEEPROM();
				Init8836AsDefault(InputMain, 1);
				//InitWithNTSC();
				DCDC_StartUP();
				ScalerSetMuteManual(OFF);
				LedPowerUp();
				ClkPllSetDividerReg(CLKPLL_DIV_1P5);
				SpiOsdEnable(ON);
				SpiOsdSetDeValue();
				DebugLevel = 0;
			}	
#if defined(SUPPORT_I2CCMD_TEST_SLAVE)
			else if( !stricmp( argv[1], "ISR" ) ) {	
				InitISR(0);
			}
#endif
			else if( !stricmp( argv[1], "core" ) ) {	
				InitCore(0);
			}
			else if( !stricmp( argv[1], "NTSC" ) ) {
				Init8836AsDefault(0/*InputMain*/, 1);//InitWithNTSC();
				FP_GpioDefault();
				SSPLL_PowerUp(ON);
			}
			else if( !stricmp( argv[1], "panel" ) ) {
				DCDC_StartUP();
				ScalerSetMuteManual(OFF);
				LedPowerUp();
			}
			else if( !stricmp( argv[1], "sspll" ) ) {
				McuSpiClkSelect(MCUSPI_CLK_27M);
				I2C_delay_base = 1;
#if defined(PANEL_AUO_B133EW01)
			 	//SspllSetFreqAndPll(150000000L);
			 	SspllSetFreqAndPll(110000000L);
#else
			 	SspllSetFreqAndPll(108000000L);
#endif
				if(SpiFlashVendor==SFLASH_VENDOR_MX)
					ClkPllSetSelDiv(CLKPLL_SEL_PLL,CLKPLL_DIV_2P0);
				else
					ClkPllSetSelDiv(CLKPLL_SEL_PLL,CLKPLL_DIV_1P5);
			}
			else if( !stricmp( argv[1], "clock" ) ) {
				if( argc>=3 ) {
					if     ( toupper(argv[2][0])=='H' ) ClockHigh();
					else if( toupper(argv[2][0])=='L' ) ClockLow();
					else                                Clock27();
				}
				else
					Clock27();
			}
			else if( !stricmp( argv[1], "logo" ) ) {
				if( argc>=3 ) {
					if( !stricmp( argv[2], "on" ) )
						InitLogo1();
					else if( !stricmp( argv[2], "off" ) )
						RemoveLogoWithWait(1);
					else
						Printf("\nInvalid command...");
				}
				else {
					//auto
					InitLogo1();
					if(1) {
						RemoveLogoWithWait(1);
					}
				}
			}
			else if( !stricmp( argv[1], "default" ) ) {
				InitInputAsDefault();
			} 
		}
	}
	//-----input select------------------------------------------
	else if ( !stricmp( argv[0], "imain") ) {
		if(argc < 2) {
			//print current value
			Printf("\nInputMain:");
			PrintfInput(InputMain,1);
			Printf("\t\tInputMainEE:");
			PrintfInput(GetInputMainEE(),1);

			Printf("\nusage: imain {CVBS|SVIDEO|COMP|PC|HDMI|BT656|LVDS}");
		}
		else {
			BYTE new_InputMain;
			if     ( !stricmp( argv[1], "CVBS" ))   InputMain = INPUT_CVBS;
			else if( !stricmp( argv[1], "SVIDEO" )) InputMain = INPUT_SVIDEO;
			else if( !stricmp( argv[1], "COMP" ))   InputMain = INPUT_COMP;
			else if( !stricmp( argv[1], "PC" ))     InputMain = INPUT_PC;
			else if( !stricmp( argv[1], "DVI" ))    InputMain = INPUT_DVI;
			else if( !stricmp( argv[1], "HDMI" )) {
				if(GetHdmiModeEE())  				InputMain = INPUT_HDMITV;
				else 				 				InputMain = INPUT_HDMIPC;
  			}
			else if( !stricmp( argv[1], "BT656" ))  InputMain = INPUT_BT656;
			else if( !stricmp( argv[1], "LVDS" ))   InputMain = INPUT_LVDS;
			else                                    InputMain = INPUT_CVBS;
			InputSubMode = 7; //N/A

			LinkCheckAndSetInput();
		
			if(GetInputMainEE() != InputMain) 	
				SaveInputMainEE( InputMain );

			new_InputMain = InputMain;
			InputMain = 0xff;
			ChangeInput( new_InputMain );	
		}
	}
	else if ( !stricmp( argv[0], "iBT656") ) {
		if(argc < 2) {
			//print current value
			Printf("\nInputBT656:");
			PrintfBT656Input(InputBT656,1);
			Printf("\t\tInputBT656EE:");
			PrintfBT656Input(GetInputBT656EE(),1);

			Printf("\nusage: iBT656 {OFF|AUTO|PANEL|DEC|ARGB|DTV|LVDS|LOOP}");
			//Printf("\nusage: iBT656 {OFF|AUTO|PANEL|...}");
		}
		else {
			     if( !stricmp( argv[1], "OFF" ))  	InputBT656 = BT656_SRC_OFF;
			else if( !stricmp( argv[1], "AUTO" ))  	InputBT656 = BT656_SRC_AUTO;
			else if( !stricmp( argv[1], "PANEL" ))  InputBT656 = BT656_SRC_PANEL;
			else if( !stricmp( argv[1], "DEC" ))   	InputBT656 = BT656_SRC_DEC;
			else if( !stricmp( argv[1], "ARGB" )) 	InputBT656 = BT656_SRC_ARGB;
			else if( !stricmp( argv[1], "DTV" ))   	InputBT656 = BT656_SRC_DTV;
			else if( !stricmp( argv[1], "LVDS" ))   InputBT656 = BT656_SRC_LVDS;
			else if( !stricmp( argv[1], "loop" )) {
				//loopback mode.
				if(InputMain == INPUT_CVBS
				|| InputMain == INPUT_SVIDEO
				|| InputMain == INPUT_COMP
				|| InputMain == INPUT_PC
				|| InputMain == INPUT_LVDS) {
					switch(InputMain) {
					case INPUT_COMP:
					case INPUT_PC:	
						InputBT656 = BT656_SRC_ARGB;	
						break;	
					case INPUT_LVDS:	
						InputBT656 = BT656_SRC_LVDS;		
						break;
					default:		
						InputBT656 = BT656_SRC_DEC;		
						break;
					}
					//passthru. or
					//ChangeBT656Module(InputBT656);
					//CheckAndSetBT656Loop();
					CheckAndSetDtv656(InputBT656);
					ChangeBT656Input(InputBT656);
					return;
				}
				else {
					Printf("\niBT656 loop supports CVBS,SVideo,Component,PC,LVDS only");
					Printf("\nuse imain command first."); 
				}
			}
			else {
				Printf("\nInvalid command...");
				Prompt();
				return;                                    
			}
			if(GetInputBT656EE() != InputBT656) 	
				SaveInputBT656EE( InputBT656 );

			//-------------------------------
			//NOTE: InputMain has a high priority when InputBT656 has a conflict.
			ChangeBT656Module(InputBT656);
		}
	}
	//---------------- Change Input Mode ---------------------
	// M [CVBS|SVIDEO|COMP|PC|DVI|HDMI|BT656]
	//--------------------------------------------------------      
	//else if ( !stricmp( argv[0], "M" ) ) {
	//	if(argc < 2) {
	//		StartVideoInput();
	//	}
	//	else {
	//		if ( !stricmp( argv[1], "CVBS" ) || !stricmp( argv[1], "C" )) {
	//			InputMain = 0xff;			// start with saved input						
	//			ChangeInput( INPUT_CVBS );	
	//		}
	//		else if ( !stricmp( argv[1], "SVIDEO" ) || !stricmp( argv[1], "S" )) {
	//			InputMain = 0xff;			// start with saved input						
	//			ChangeInput( INPUT_SVIDEO );	
	//		}
	//		else if ( !stricmp( argv[1], "COMP" ) || !stricmp( argv[1], "Y" )) { //NOTE:Y
	//			InputMain = 0xff;			// start with saved input						
	//			ChangeInput( INPUT_COMP );	
	//		}
	//		else if ( !stricmp( argv[1], "PC" ) || !stricmp( argv[1], "P" )) {
	//			InputMain = 0xff;			// start with saved input						
	//			ChangeInput( INPUT_PC );	
	//		}
	//		else if ( !stricmp( argv[1], "DVI" ) || !stricmp( argv[1], "D" )) {
	//			InputMain = 0xff;			// start with saved input						
	//			ChangeInput( INPUT_DVI );	
	//		}
	//		else if ( !stricmp( argv[1], "HDMI" ) || !stricmp( argv[1], "H" )) {
	//			BYTE InputMainEE;
	// 
	//			if(GetHdmiModeEE())  InputMainEE = INPUT_HDMITV;
	//			else 				 InputMainEE = INPUT_HDMIPC;
	//  
	//			InputMain = 0xff;			// start with saved input						
	//			ChangeInput( InputMainEE );	
	//		}
	//		else if ( !stricmp( argv[1], "HDMIPC" )) {
	//			InputMain = 0xff;			// start with saved input						
	//			ChangeInput( INPUT_HDMIPC );
	//		}	
	//		else if ( !stricmp( argv[1], "HDMITV" )) {
	//			InputMain = 0xff;			// start with saved input						
	//			ChangeInput( INPUT_HDMITV );
	//		}	
	//		else if ( !stricmp( argv[1], "BT656" ) || !stricmp( argv[1], "B" )) {
	//			InputMain = 0xff;			// start with saved input						
	//			ChangeInput( INPUT_BT656 );	
	//		}
	//	}
	//}
	//---------------- CheckAndSet ---------------------
	else if ( !stricmp( argv[0], "CheckAndSet" ) ) {
		BYTE ret;

		switch(InputMain) {
		case INPUT_CVBS:
		case INPUT_SVIDEO:	ret=CheckAndSetDecoderScaler();	break;
#ifdef SUPPORT_COMPONENT
		case INPUT_COMP:	ret=CheckAndSetComponent();		break;
#endif
#ifdef SUPPORT_PC
		case INPUT_PC:		ret=CheckAndSetPC();			break;
#endif
#ifdef SUPPORT_DVI
		case INPUT_DVI:		ret=CheckAndSetDVI();			break;
#endif
#if defined(SUPPORT_HDMI_EP9351)
		case INPUT_HDMIPC:
		case INPUT_HDMITV:
							ret= CheckAndSetHDMI();			break;
#endif
#ifdef SUPPORT_BT656_LOOP
		case INPUT_BT656:	ret = CheckAndSetBT656Loop();		break;
#endif
		case INPUT_LVDS:	ret = CheckAndSetLVDSRx();	break;
		default:			ret = CheckAndSetUnknown();		break;
		}
		if(ret==ERR_SUCCESS) {
			//success
			VInput_enableOutput(0);
			Puts("\n==>SUCCESS");
#ifdef NOSIGNAL_LOGO
			if(getNoSignalLogoStatus())
				RemoveLogo();
#endif
		} else {
			//------------------
			// NO SIGNAL
			// Prepare NoSignal Task...
			VInput_gotoFreerun(0);
			Puts("\n==>FAIL");
		}
	}
	//---------------- check clock-------------------------
	else if ( !stricmp( argv[0], "checkclock" ) ) {
		Test_Checkclock();
	}
	else if ( !stricmp( argv[0], "checkclock2" ) ) {
		Test_Checkclock2();
	}
	//---------------- check -------------------------
	else if ( !stricmp( argv[0], "check" ) ) {
		if(argc < 2) {
			Puts("\n ? Phase|Color");
		}
		else {
#ifdef SUPPORT_PC
			if ( !stricmp( argv[1], "PHASE" )|| !stricmp( argv[1], "PH" ))
				AutoTunePhase();
			else if ( !stricmp( argv[1], "COLOR" ))
				AutoTuneColor();
#endif	
		}
	}

	//---------------- SPI Debug -------------------------
	else if( !stricmp( argv[0], "SPI" ) ) {
		MonitorSPI();
	}
	else if( !stricmp( argv[0], "SPIC" ) ) {
		MonitorSPIC();
	}
	//---------------- EEPROM Debug -------------------------
#ifdef USE_SFLASH_EEPROM
	else if( !stricmp( argv[0], "EE" ) ) {
		MonitorEE();
	}
#endif
	//---------------- MENU Debug -------------------------
	else if( !stricmp( argv[0], "menu" ) ) {
		MonitorMenu();
	}
	//---------------- Font Osd Debug -------------------------
	else if( !stricmp( argv[0], "fosd" ) ) {
		MonitorFOsd();
	}
	//---------------- SPI Osd Debug -------------------------
	else if( !stricmp( argv[0], "sosd" ) ) {
		MonitorSOsd();
	}
	//---------------- MCU Debug -------------------------
	else if( !stricmp( argv[0], "MCU" ) ) {
		MonitorMCU();
	}
	//---------------- Debug Level ---------------------
	else if ( !stricmp( argv[0], "DEBUG" ) ) {
		if( argc==2 ) {
			DebugLevel = a2h(argv[1]);
		}
		Printf("\nDebug Level = %2bx", DebugLevel);
	}
	//---------------- SW Key pad ---------------------
	else if ( !stricmp( argv[0], "KEY" ) ) {
		if( argc==2 ) {
			SW_key = a2h(argv[1]);
		}
		Printf("\nSW_key = %2bx", SW_key);
	}
	//---------------- Echo back on/off -----------------
	else if ( !stricmp( argv[0], "echo" ) ) {
		if( !stricmp( argv[1], "off" ) ) {
			echo = 0;
			Printf("\necho off");
		}
		else {
			echo = 1;
			Printf("\necho on");
		}
	}
	//---------------- access on/off -----------------
	else if ( !stricmp( argv[0], "ACCESS" ) ) {
		if( !stricmp( argv[1], "0" ) ) {
			access = 0;
			Printf("\nAccess off");
			//disable interrupt.
			WriteTW88Page(PAGE0_GENERAL );
			WriteTW88(REG003, 0xFE );	// enable only SW interrupt
		}
		else {
			access = 1;
			Printf("\nAccess on");
		}
	}
	//---------------- cache on/off -----------------
	else if ( !stricmp( argv[0], "cache" ) ) {
		if(argc >=2) {
			if( !stricmp( argv[1], "on" ) )
				SFR_CACHE_EN = 1;
			else if( !stricmp( argv[1], "off" ) )
				SFR_CACHE_EN = 0;
			else
				Printf("\nusage:cache [on|off]");
		}
		//cache status
		if(SFR_CACHE_EN)	Printf("\ncache on");
		else				Printf("\ncache off");
	}
	//---------------- task on/off -----------------
	else if ( !stricmp( argv[0], "task" ) ) {
		if( !stricmp( argv[1], "on") ) {
			tic_task = 10*100;	// NOSIGNAL_TIME_INTERVAL
			TaskNoSignal_setCmd(TASK_CMD_RUN);
		}
		else {
			TaskSetGrid(0);
	
			TaskNoSignal_setCmd(TASK_CMD_DONE);
			WriteTW88Page(PAGE0_GENERAL );
			//WriteTW88(REG003, 0xFF );	// disable all INTR
		}
	}
	//---------------- System Clock Display -----------------
	else if ( !stricmp( argv[0], "time" ) ) {
			Printf("\nSystem Clock: %ld:%5bd", SystemClock, tic01);
	}
	//---------------- HDMI test -------------------------
#ifdef SUPPORT_HDMI_EP9351
#ifdef ON_CHIP_EDID_ENABLE
	else if( !stricmp( argv[0], "edid" ) ) {
		DWORD addr;
		if(argc < 2)
			addr = 0;
		else
			addr = a2h( argv[1] );
		HdmiDownloadEdid(addr);
	}
#endif
#ifdef ON_CHIP_HDCP_ENABLE
	else if( !stricmp( argv[0], "hdcp" ) ) {
		DWORD addr;
		if(argc < 2)
			addr = 0;
		else
			addr = a2h( argv[1] );
		HdmiDownloadDhcp(addr);
	}
#endif
	else if( !stricmp( argv[0], "aviinfo" ) ) {
		BYTE TempByte[16];
		ReadI2C(I2CID_EP9351, EP9351_AVI_InfoFrame, TempByte, 15);
		DBG_PrintAviInfoFrame();
	}
#endif
	
#if 0
	//---------------- pclk -------------------------
	//	pclk 1080 means 108MHz
	//	pclk 27 means 27MHz
	else if( !stricmp( argv[0], "pclk" ) ) {
		if(argc >= 2) {
			dValue = a2i( argv[1] );
			dValue *= 100000L;
		 	SspllSetFreqAndPll(dValue);
		}
		//print current pclk info
		Printf("\nsspll:%ld",SspllGetPPF());
	}
#endif
	//====================================================
	// OTHER TEST ROUTINES
	//====================================================
#if defined(MODEL_TW8835_SLAVE) && defined(SUPPORT_I2CCMD_TEST)
//def SUPPOR T_I2CCMD_SLAVE
	//---------------- test -------------------
	else if( !stricmp( argv[0], "i2ctest" ) )	 {
		i2c_test_slave_conflict();
	}
	else if( !stricmp( argv[0], "i2ctestpage" ) )	 {
		BYTE page;
		page = a2i( argv[1] );
		test_set_i2c_slave_compare_page(page);
	}
#endif
#if defined(SUPPORT_I2CCMD_TEST)
	else if( !stricmp( argv[0], "i2ccmdtest" ) )	 {
		BYTE mode;
		WORD duration;

		if(argc < 2) {
			Printf("\nInvalid parameter");
			return;
		}
		else if(argc < 3) {
			duration = 0xFFFF;	//max 65536
		}
		else {
			duration = a2i( argv[2] );
		}
		mode = a2i( argv[1] );
		i2ccmd_test(mode,duration);
	}
#endif

	else if( !stricmp( argv[0], "testfont" ) )	 {
		extern void TestInitFontRam(WORD start);
		WORD start;
		start = a2h( argv[1] );
		TestInitFontRam(start);
	}	

	//==========================================
	// FontOSD Test
	//==========================================
#if 1
	else if ( !stricmp( argv[0], "FT0" )) {
		//FontOSD Test
		WORD	OsdRamAddr;
		DECLARE_LOCAL_page
		BYTE	i;
		static	BYTE	attr = 0x1a;
		static	BYTE	fdata = 1;

		ReadTW88Page(page);
		WriteTW88Page(PAGE3_FOSD );

		OsdRamAddr =  0x1fD;

		for(i=0; i < 16; i++) {
			FOsdRamSetAddress(OsdRamAddr);
			WriteTW88(REG304, 0 );
			WriteTW88(REG307, fdata);
			WriteTW88(REG308, attr ); 			// bgColor + fgColor in 1BPP
			OsdRamAddr++;
			fdata++;
		}

		if ( fdata>60 ) fdata=1;
		attr = ~attr;
		WriteTW88Page(page );
	}
	else if ( !stricmp( argv[0], "FT1" )) {
		//FontOSD test
		WORD	OsdRamAddr;
		DECLARE_LOCAL_page
		BYTE	i;
		static	BYTE	attr = 0x1a;
		static	BYTE	fdata = 1;


		ReadTW88Page(page);

		while ( !RS_ready() ) {
		
			WriteTW88Page(PAGE3_FOSD );
		
			OsdRamAddr =  0x1fD;
		
			//set address again for index data
		
			FOsdRamSetAddress(OsdRamAddr);
			WriteTW88(REG308, attr ); 			// bgColor + fgColor in 1BPP
		
			WriteTW88(REG304, 0x0C ); // Auto Inc.
			FOsdRamSetAddress(OsdRamAddr);
		
			for(i=0; i < 16; i++) {
				WriteTW88(REG307, fdata);
				fdata++;
			}
		
			if ( fdata>60 ) fdata=1;
			attr = ~attr;
		}
		WriteTW88Page(page );
	}
	else if ( !stricmp( argv[0], "FT2" )) {
		//FontOSD test
		WORD	OsdRamAddr;
		DECLARE_LOCAL_page
		BYTE	i;
		static	BYTE	attr = 0x1a;
		static	BYTE	fdata = 1;

		ReadTW88Page(page);
		while ( !RS_ready() ) {
			WriteTW88Page(PAGE3_FOSD );
	
			OsdRamAddr =  0x1fD;
	
			//set address again for index data
	
			FOsdRamSetAddress(OsdRamAddr);
			WriteTW88(REG308, attr ); 			// bgColor + fgColor in 1BPP
		
			WriteTW88(REG304, 0x0C ); // Auto Inc.
			FOsdRamSetAddress(OsdRamAddr);
	
			WriteTW88(REG300, ReadTW88(REG300) | 0x01);		// bypass on
			for(i=0; i < 16; i++) {
				WriteTW88(REG307, fdata);
				fdata++;
			}
			WriteTW88(REG300, ReadTW88(REG300) & 0xFE);		 // bypass off
	
			if ( fdata>60 ) fdata=1;
			attr = ~attr;
		}
		WriteTW88Page(page );
	}

	else if ( !stricmp( argv[0], "FT3" )) {
		//FontOSD test
		WORD	OsdRamAddr;
		DECLARE_LOCAL_page
		BYTE	i;
		static	BYTE	attr = 0x1a;
		static	BYTE	fdata = 1;

		ReadTW88Page(page);
		while ( !RS_ready() ) {
			WriteTW88Page(PAGE3_FOSD );
	
			OsdRamAddr =  0x1fD;
	
			//set address again for index data
	
			FOsdRamSetAddress(OsdRamAddr);
			WriteTW88(REG308, attr ); 			// bgColor + fgColor in 1BPP
		
			WriteTW88(REG304, 0x0C ); // Auto Inc.
			FOsdRamSetAddress(OsdRamAddr);

			WriteTW88(REG300, ReadTW88(REG300) | 0x01);		// bypass on
			for(i=0; i < 16; i++) {
				WriteTW88(REG307, fdata);
				fdata++;
			}
			delay1ms(300);
			WriteTW88(REG300, ReadTW88(REG300) & 0xFE);		 // bypass off
	
			if ( fdata>60 ) fdata=1;
			attr = ~attr;
		}
		WriteTW88Page(page );
	}

	else if ( !stricmp( argv[0], "FT4" )) {
		WORD	OsdRamAddr;
		DECLARE_LOCAL_page
		BYTE	i;
		static	BYTE	attr = 0x1a;
		static	BYTE	fdata = 1;

		ReadTW88Page(page);
		while ( !RS_ready() ) {
			WriteTW88Page(PAGE3_FOSD );
	
			OsdRamAddr =  0x1fD;
	
			//set address again for index data
	
			FOsdRamSetAddress(OsdRamAddr);
			WriteTW88(REG308, attr ); 			// bgColor + fgColor in 1BPP
		
			WriteTW88(REG304, 0x0C ); // Auto Inc.
			FOsdRamSetAddress(OsdRamAddr);
	
			WriteTW88(REG300, ReadTW88(REG300) | 0x01);		// bypass on
			for(i=0; i < 16; i++) {
				WriteTW88(REG307, fdata);
				fdata++;
			}
			WriteTW88(REG304, 0x0D);		// change to FontRAM
			WriteTW88(REG300, ReadTW88(REG300) & 0xFE);		 // bypass off
	
			if ( fdata>60 ) fdata=1;
			attr = ~attr;
		}
		WriteTW88Page(page );
	}

	else if ( !stricmp( argv[0], "FT5" )) {
		static	WORD	OsdRamAddr=0x1fd;
		DECLARE_LOCAL_page
		BYTE	i;
		static	BYTE	attr = 0x1a;
		static	BYTE	fdata = 1;

		ReadTW88Page(page);
		while ( !RS_ready() ) {
			WriteTW88Page(PAGE3_FOSD );
	
			//OsdRamAddr =  0x1fD;
	
			//set address again for index data
	
			WriteTW88(REG304, 0x00 ); 			// Auto Inc OFF
			FOsdRamSetAddress(OsdRamAddr);
			delay1ms(1000);
			WriteTW88(REG308, attr ); 			// bgColor + fgColor in 1BPP
		
			WriteTW88(REG304, 0x0C ); // Auto Inc.
			FOsdRamSetAddress(OsdRamAddr);
	
			WriteTW88(REG300, ReadTW88(REG300) | 0x01);		// bypass on
			delay1ms(1000);
			for(i=0; i < 16; i++) {
				WriteTW88(REG307, fdata);
				fdata++;
				OsdRamAddr++;
			}
			OsdRamAddr++;
			WriteTW88(REG304, 0x0D);		// change to FontRAM
			WriteTW88(REG300, ReadTW88(REG300) & 0xFE);		 // bypass off
	
			if ( fdata>60 ) fdata=1;
			attr = ~attr;
		}
		WriteTW88Page(page );
	}

	else if ( !stricmp( argv[0], "FT6" )) {
		static	WORD	OsdRamAddr=0x1fd;
		DECLARE_LOCAL_page
		BYTE	i;
		static	BYTE	attr = 0x1a;
		static	BYTE	fdata = 1;

		ReadTW88Page(page);
		WriteTW88Page(PAGE3_FOSD );
		WriteTW88(REG300, ReadTW88(REG300) | 0x01);		// bypass on
		WriteTW88(REG304, 0x0C ); 			// Auto Inc OFF
		while ( !RS_ready() ) {
			WriteTW88Page(PAGE3_FOSD );
	
			OsdRamAddr =  0xfD;
	
			//set address again for index data
	
			FOsdRamSetAddress(OsdRamAddr);
			//delay1ms(500);
			WaitVBlank(1);
			WriteTW88(REG308, attr ); 			// bgColor + fgColor in 1BPP
		
			FOsdRamSetAddress(OsdRamAddr);
	
			for(i=0; i < 16; i++) {
				WriteTW88(REG307, fdata);
				fdata++;
				OsdRamAddr++;
			}
			OsdRamAddr++;
	
			if ( fdata>60 ) fdata=1;
			attr = ~attr;
		}
		WriteTW88Page(page );
	}
#endif
	//==========================================
	// SpiOSD Test
	//==========================================
	//void SpiOsdLoadLUT(BYTE _winno, BYTE _type, WORD LutOffset, WORD size, DWORD address, BYTE alpha)
	else if ( !stricmp( argv[0], "lutgrid" )) {
		DWORD addr;
		WORD offset;

		if ( argc == 2 ) 	offset = a2i(argv[1]);
		else 				offset = 0;

		addr = offset * 0x400;
		addr += 0x100000;

		SpiOsdLoadLUT(1, 0, 0, 0x400, addr, 0xFF); 
	}
#if 0
	else if ( !stricmp( argv[0], "LUT0" )) {
		BYTE	page, i;

		ReadTW88Page(page);

		WriteTW88Page(3 );

		// Win4 enable and OSD RAM set
		WriteTW88(REG305, 0 );
		WriteTW88(REG30B, 0 );
		WriteTW88(REG30C, 0x3f );
		WriteTW88(REG340, 0xcb );
		WriteTW88(REG345, 0x01 );
		WriteTW88(REG346, 0x10 );


		// Enable OSD RAM
		WriteTW88(REG300, 0x11 );
		WriteTW88(REG305, 0 );
		WriteTW88(REG306, 0 );
		WriteTW88(REG304, 0 );
		WriteTW88(REG307, 0 );
		for ( i=0; i<16; i++ ) {
			FOsdRamSetAddress((WORD)i);	//WriteTW88(REG306, i );
			WriteTW88(REG307, 0 );
			WriteTW88(REG308, i );
			delay1ms(10);
		}

#if 0
		// download Font
		WriteTW88(REG300, 0x11 );
		WriteTW88(REG304, 0x0d );
		WriteTW88(REG_FOSD_CHEIGHT, 0x0d /* 26/2 */ );
		WriteTW88(REG_FOSD_MUL_CON, 0x34 );
		WriteTW88(REG309, 0x00 );

		for ( i=0; i<13; i++ ) {
			WriteTW88(REG30A, 0x00 );
			WriteTW88(REG30A, 0x00 );
			WriteTW88(REG30A, 0xff );
			WriteTW88(REG30A, 0xff );
		}
		for ( i=0; i<13; i++ ) {
			WriteTW88(REG30A, 0x00 );
			WriteTW88(REG30A, 0xff );
			WriteTW88(REG30A, 0x00 );
			WriteTW88(REG30A, 0xff );
		}
		
		color = 0;
		for ( i=0; i<64; i++ ) {
			WriteTW88(REG30c, i );
			WriteTW88(REG30D, color>>8 );
			WriteTW88(REG30E, color );

		}
#endif
		WriteTW88Page(page );
	}

	else if ( !stricmp( argv[0], "LUTF" )) {	 // font download
		BYTE	page, i;

		ReadTW88Page(page);

		WriteTW88Page(3 );

		// download Font
		WriteTW88(REG300, 0x11 );
		WriteTW88(REG304, 0x0d );
		WriteTW88(REG305, 0x00 );
		WriteTW88(REG309, 0x00 );
		WriteTW88(REG30B, 0x00 );
		WriteTW88(REG_FOSD_CHEIGHT, (0x1a >> 1) );	 //?? 0x1A=26 
		WriteTW88(REG_FOSD_MUL_CON, 0x34 );
		WriteTW88(REG_FOSD_MADD3, 0x00 );
		WriteTW88(REG_FOSD_MADD4, 0x00 );

		for ( i=0; i<13; i++ ) {
			WriteTW88(REG30a, 0x00 );
			WriteTW88(REG30a, 0x00 );
			WriteTW88(REG30a, 0xff );
			WriteTW88(REG30a, 0xff );
			delay1ms( 10 );
		}
		for ( i=0; i<13; i++ ) {
			WriteTW88(REG30a, 0x00 );
			WriteTW88(REG30a, 0xff );
			WriteTW88(REG30a, 0x00 );
			WriteTW88(REG30a, 0xff );
			delay1ms( 10 );
		}
		for ( i=0; i<52; i++ ) {
			WriteTW88(REG30a, 0x33 );
			delay1ms( 2 );
		}

		WriteTW88Page(page );
	}

	else if ( !stricmp( argv[0], "LUTC" )) {	 // clear = black
		BYTE	page, i;
		WORD	color;

		ReadTW88Page(page);

		WriteTW88Page(3 );


		color = 0;
		for ( i=0; i<64; i++ ) {
			WriteTW88(REG30c, i );
			WriteTW88(REG30d, color>>8 );
			WriteTW88(REG30e, color );
		}
		WriteTW88Page(page );
	}

	else if ( !stricmp( argv[0], "LUTB" )) {	 // blue
		BYTE	page, i;
		WORD	color;

		ReadTW88Page(page);

		WriteTW88Page(3 );


		color = 0;
		for ( i=0; i<64; i++ ) {
			WriteTW88(REG30c, i );
			WriteTW88(REG30d, color>>8 );
			WriteTW88(REG30e, color );
			if ( i%2 ) color ++;
		}
		WriteTW88Page(page );
	}

	else if ( !stricmp( argv[0], "LUTG" )) {	 // blue
		BYTE	page, i;
		WORD	color;

		ReadTW88Page(page);

		WriteTW88Page(3 );


		color = 0;
		for ( i=0; i<64; i++ ) {
			WriteTW88(REG30c, i );
			WriteTW88(REG30d, color>>8 );
			WriteTW88(REG30e, color );
			color += 0x20;
		}
		WriteTW88Page(page );
	}

	else if ( !stricmp( argv[0], "LUTR" )) {	 // red
		BYTE	page, i;
		WORD	color;

		ReadTW88Page(page);

		WriteTW88Page(3 );


		color = 0;
		for ( i=0; i<64; i++ ) {
			WriteTW88(REG30c, i );
			WriteTW88(REG30d, color>>8 );
			WriteTW88(REG30e, color );
			if ( i%2 ) color += 0x800;
		}
		WriteTW88Page(page );
	}

	else if ( !stricmp( argv[0], "LUTW" )) {	 // white
		BYTE	page, i;
		WORD	color;

		ReadTW88Page(page);

		WriteTW88Page(3 );


		color = 0;
		for ( i=0; i<64; i++ ) {
			WriteTW88(REG30c, i );
			WriteTW88(REG30d, color>>8 );
			WriteTW88(REG30e, color );
			color += 0x20;
			if ( i%2 ) color += 0x801;
		}
		WriteTW88Page(page );
	}

	else if ( !stricmp( argv[0], "SLUT0" )) {	 // white
		BYTE page;
		ReadTW88Page(page);

		WaitVBlank(1);
		SpiOsdLoadLUT(3, 1, 0, 1024, 0xb0000, 0xFF);	 //winno ?3

		WriteTW88Page(page );
	}

	else if ( !stricmp( argv[0], "SLUT1" )) {	 // white
		BYTE page;
		ReadTW88Page(page);

		WaitVBlank(1);
		SpiOsdLoadLUT( 3, 1, 256, 1024, 0xb0400, 0xFF); //winno ?3

		WriteTW88Page(page );
	}

	else if ( !stricmp( argv[0], "SLUT2" )) {	 // white
		BYTE page, i;
		ReadTW88Page(page);

		i = 0;
		while ( !RS_ready() ) {
			#if 0
			WaitVBlank(1);
			SpiOsdLoadLUT( 3, 1, 128, 512, 0xb0800, 0xFF); //winno ?3
			delay1ms(100);
			
			WaitVBlank(1);
			SpiOsdLoadLUT(3,  1, 128, 512, 0xb0a00, 0xFF); //winno ?3
			delay1ms(100);
			#endif
			WaitVBlank(1);
			SpiOsdLoadLUT(3,  1, 128+i++, 512, 0xb0c00, 0xFF);	//winno ? 3
			//WaitVBlank(1);
			//SpiOsdLoadLUT( 1, 128+i++, 512, 0xb0e00);
			i %= 128;
		}
		WriteTW88Page(page );
	}
#endif

	//---------------- TOUCH Debug -------------------------
#ifdef SUPPORT_TOUCH
	else if( !stricmp( argv[0], "TOUCH" ) ) {
		if ( argc == 2 ) {
			if ( !stricmp( argv[1], "ON" ) ) {
				SFR_ET1 = 1;	//BK110531
				SetTouchAuto(1);
				Puts("\r\nTurn ON Touch Sense");
			}
			else if ( !stricmp( argv[1], "DATA" ) ) {
				PrintCalibData();
			}
			else {
				SetTouchAuto(0);
				SFR_ET1 = 0;	//BK110531
				Puts("\r\nTurn OFF Touch Sense");
			}
		}
		else {
			GetTouch2();
		}
	}
	else if( !stricmp( argv[0], "TSC_DEBUG" ) ) {
		tsc_debug = 0;
		if ( argc == 2 ) {
			if ( !stricmp( argv[1], "ON" ) )
				tsc_debug = 1;
		}
		if(tsc_debug) {
			SFR_ET1 = 0;	//disable touch timer
		}
		else {
			InitAuxADC();
		}
	}
	else if( !stricmp( argv[0], "TRACE" ) ) {
		if ( argc == 2 ) {
			if ( !stricmp( argv[1], "ON" ) ) {
				Puts("\r\nTurn ON Touch Trace");
				SFR_ET1 = 0;	//BK110531
				SetTraceAuto(1);
			}
			else {
				Puts("\r\nTurn OFF Touch Trace");
				SFR_ET1 = 1;	//BK110531
				SetTraceAuto(0);
			}
		}
		else {
//			TraceTouch();
//			extern 	bit FLAG_TOUCH;
			extern	WORD	TouchX, TouchY, TouchZ1; //, AUX[4];
			//extern	BYTE	/*CpuTouchChanged,*/ AUX_Changed[4];
			BYTE	temp, atemp[4], i;
			temp = CpuTouchChanged;
			atemp[0] = CpuAUX0_Changed;
			atemp[1] = CpuAUX1_Changed;
			atemp[2] = CpuAUX2_Changed;
			atemp[3] = CpuAUX3_Changed;
			while ( !RS_ready() ) {
				delay1ms(500);
				if ( CpuTouchPressed ) {
					if (CpuTouchChanged != temp) {
						Printf("\nTouchX: 0x%04x, TouchY: 0x%04x, Z1: 0x%04x", TouchX, TouchY, TouchZ1);
						temp = CpuTouchChanged;
					}
				}
				if ( atemp[0] != CpuAUX0_Changed ) {
					Printf("\n0x%02bx - AUX[0]: 0x%04x",atemp[i], CpuAUX0);
					atemp[0] = CpuAUX0_Changed;
				}
				if ( atemp[1] != CpuAUX1_Changed ) {
					Printf("\n0x%02bx - AUX[1]: 0x%04x",atemp[i], CpuAUX1);
					atemp[1] = CpuAUX1_Changed;
				}
				if ( atemp[2] != CpuAUX2_Changed ) {
					Printf("\n0x%02bx - AUX[2]: 0x%04x",atemp[i], CpuAUX2);
					atemp[2] = CpuAUX2_Changed;
				}
				if ( atemp[3] != CpuAUX3_Changed ) {
					Printf("\n0x%02bx - AUX[3]: 0x%04x",atemp[i], CpuAUX3);
					atemp[3] = CpuAUX3_Changed;
				}
			}
		}
	}
	else if(!stricmp( argv[0], "TCOUNT" ) ) {
		CpuTouchSkipCount = 100;
		if ( argc == 2 ) {
			CpuTouchSkipCount = a2i(argv[1]);
		}
	}
	else if( !stricmp( argv[0], "TDUMP" ) ) {
		Puts("\r\nTouch Dump");
		TouchDump();
	}
	
	//---------------- Touch Calibration -------------------------
	else if( !stricmp( argv[0], "CALIB" ) ) {
		BYTE	n;
		if ( argc == 2 ) {
			n = a2h(argv[1]);
			if ( n > 4 ) n = 4;
			CalibTouch(n);
		}
	}
#endif
	//---------------- Delta RGB Panel Test -------------------------
#ifdef SUPPORT_DELTA_RGB

	else if( !stricmp( argv[0], "delta" ) ) {
		extern	BYTE	SPI_ID;
		if( argc==1 )	DeltaRGBPanelInit();
		else {
			
			if( !stricmp( argv[1], "w" ) ) {

				WORD val;
				//val= Asc2Bin(argv[3])<<8 | Asc2Bin(argv[4]);
				val= a2h(argv[3]);
				Printf("\r\n SPI Write: Addr:%2x  Data:%4x", (WORD)a2h(argv[2]), val);
				WriteSPI(a2h(argv[2]), val );
			}
			else if( !stricmp( argv[1], "r" ) ) {
				WORD val;
				val = ReadSPI(a2h(argv[2]));
				Printf("\r\n SPI Read: Addr:%2x  Data:%4x", (WORD)a2h(argv[2]), val);
			}
			else if( !stricmp( argv[1], "c" ) ) {
				if ( argc == 3 ) 
					SPI_ID = a2h(argv[2]) & 0xfc;
				Printf("\r\n SPI Device ID:%2bx", SPI_ID);
			}
		}
	}
	else if( !stricmp( argv[0], "AUO" ) ) {
		if( argc==1 )	Puts( "\r\nIncorrect commad - AUO r ii or AUO w ii ddd" );
		else {
			
			if( !stricmp( argv[1], "w" ) ) {

				WORD val;
				val= a2h(argv[3]);
				Printf("\r\n SPI Write: Addr:%2x  Data:%4x", (WORD)a2h(argv[2]), val);
				WriteAUO(a2h(argv[2]), val );
			}
			else if( !stricmp( argv[1], "r" ) ) {
				WORD val;
				val = ReadAUO(a2h(argv[2]));
				Printf("\r\n SPI Read: Addr:%2x  Data:%4x", (WORD)a2h(argv[2]), val);
			}
		}
	}
	else if( !stricmp( argv[0], "AUO2" ) ) {
		if( argc==1 )	Puts( "\r\nIncorrect commad - AUO2 r ii or AUO2 w ii ddd" );
		else {
			
			BYTE val, addr;
			if( !stricmp( argv[1], "w" ) ) {

				addr = a2h( argv[2] );
				val= a2h( argv[3] );
				Printf("\r\n SPI Write: Addr:%02bx  Data:%02bx", addr, val);
				WriteAUO2( addr, val );
			}
			else if( !stricmp( argv[1], "r" ) ) {
				addr = a2h( argv[2] );
				val = ReadAUO2( addr );
				Printf("\r\n SPI Read: Addr:%02bx  Data:%02bx", addr, val);
			}
		}
	}
#endif
	//---------------- HDMI -------------------
	//Read HDMI register
	else if( !stricmp( argv[0], "HDMI" ) ) {
		//BYTE val[20];
		//BYTE cnt = 1, i;
		//if ( argc > 2 )
		//	cnt	= a2h(argv[2]);
		//if ( cnt > 20 )
		//	cnt = 20;
		//ReadI2C(I2CID_EP9351, a2h(argv[1]), val, cnt);
		//for	(i=0; i<cnt; i++) 
		//	Printf("\n%3bd %02bx", i, val[i]);
		if(argc < 2) {
			Printf(" init start avi timereg dnedid dnhdcp");
		}
#ifdef SUPPORT_HDMI_EP9351
		else if( !stricmp( argv[1], "init" )) {
			Hdmi_SystemInit_EP9351();
		}
		else if( !stricmp( argv[1], "start" )) {
			HdmiInitEp9351Chip();
		}
#endif
#if defined(SUPPORT_HDMI_EP9351) || defined(SUPPORT_HDMI_EP907M)
		else if( !stricmp( argv[1], "avi" )) {
			DBG_PrintAviInfoFrame();
		}
		else if( !stricmp( argv[1], "timereg" )) {
			DBG_PrintTimingRegister();
		}
#endif
#ifdef SUPPORT_HDMI_EP9351
		else if( !stricmp( argv[1], "dump" )) {
			DBG_DumpControlRegister();
		}
#ifdef ON_CHIP_EDID_ENABLE
		else if( !stricmp( argv[1], "dnedid" )) {
			HdmiDownloadEdid(0);
		}
#endif
#ifdef ON_CHIP_HDCP_ENABLE
		else if( !stricmp( argv[1], "dnhdcp" )) {
			HdmiDownloadDhcp(0);
		}
#endif
#endif
	}

//	else if( !stricmp( argv[0], "HDINIT" ) ) {
//#ifdef SUPPORT_HDMI_EP9351
//		CheckAndSet_EP9351();
//#endif
//	}
//	else if( !stricmp( argv[0], "HDINFO" ) ) {
//		BYTE val[13];
//		WORD tmp;
//		ReadI2C(I2CID_EP9351, 0x3B, val, 13);
//		if ( val[12] & 0x80 ) 
//			Puts("\nInterlace Video Signal");
//		else
//			Puts("\nNON-Interlace Video Signal");
//		tmp = val[1]; 		tmp <<= 8;		tmp += val[0];
//		Printf("\nActive Pixels Per Line = %d", tmp ); 
//		tmp = val[3]; 		tmp <<= 8;		tmp += val[2];
//		Printf("\nHorizontal Front Porch = %d", tmp ); 
//		tmp = val[5]; 		tmp <<= 8;		tmp += val[4];
//		Printf("\nHorizontal Back Porch  = %d", tmp ); 
//		tmp = val[7]; 		tmp <<= 8;		tmp += val[6];
//		Printf("\nHorizontal Pulse Width = %d", tmp ); 
//		tmp = val[9]; 		tmp <<= 8;		tmp += val[8];
//		Printf("\nActive Lines Per Frame = %d", tmp ); 
//		Printf("\nVertical Front Porch   = %bd", val[10] ); 
//		Printf("\nVertical Back Porch    = %bd", val[11] ); 
//		Printf("\nVertical Pulse Width   = %bd", val[12]&0x7f ); 
//	}

#if 0
	//----<<TEST customer PARKGRID>>------------------------------------
	//before you call it, enable SPIOSD, update MCUSPICLK.
	else if( !stricmp( argv[0], "tgrid" ) ) {
		extern void TestDParkGridAction(void);
		TestDParkGridAction();
	}
	//BKTODO: Add "autogrid"
	else if( !stricmp( argv[0], "autogrid" ) ) {
		extern void TestAutoDParkGridAction(BYTE positionX, BYTE positionY);
		BYTE positionX, positionY;

		if(argc==3) {
			positionX = a2h(argv[1]);
			positionY = a2h(argv[2]);
		}
		else {
			positionX = 22;
			positionY = 5;
		}
		TestAutoDParkGridAction(positionX, positionY);
	}
	else if( !stricmp( argv[0], "stepgrid" ) ) {
		extern void TestStepDParkGridAction(BYTE positionX, BYTE positionY);
		BYTE positionX, positionY;

		if(argc==3) {
			positionX = a2h(argv[1]);
			positionY = a2h(argv[2]);
		}
		else {
			positionX = 22;
			positionY = 5;
		}
		TestStepDParkGridAction(positionX, positionY);
	}
#endif
//#ifdef MODEL_TW8836
	else if(!stricmp( argv[0], "bt601" )) {
		Monitor_BT601();	
	}
	else if(!stricmp( argv[0], "bt656" )) {
		Monitor_BT656();	
	}
//#endif


	//----------------------------------------------------
	// scaler test
	//
	// 0: old
	// 1: new
	// 2: target:720x480P
	//----------------------------------------------------
	else if(!stricmp( argv[0], "scaler" )) {
		BYTE mode;
		if(argc < 2) 
			mode = 0;
		else {
			mode = a2h(argv[1]);
		}
		ScalerTest(mode);
	}

	//----------------------------------------------------
	//make compiler happy.
	//Please, DO NOT EXECUTE
	//----------------------------------------------------	
	else if(!stricmp( argv[0], "compiler" )) {
#ifndef SUPPORT_FOSD_MENU
		extern void Dummy_FosdMenu_func(void);
		extern void Dummy_FosdInitTable_func(void);
		extern void Dummy_FosdDispInfo_func(void);
		extern void Dummy_FosdString_func(void);
#endif
#ifdef SUPPORT_FOSD_MENU
		extern BYTE CheckAndClearFOsd(void);
#endif
#if !defined(SUPPORT_DVI) && !defined(SUPPORT_HDMI_EP9351)
		extern void Dummy_DTV_func(void);
#endif
		WORD wTemp;
		BYTE bTemp;

//#ifdef USE_SFLASH_EEPROM
//		E3P_SetBanks(4);
//		E3P_SetBufferSize(SPI_BUFFER_SIZE);			//128
//		E3P_SetSize(EE_INDEX_PER_BLOCK,EE_BLOCKS);	//512 bytes 64 * 8.
//#endif
//		MonitorChipTest();
		wPuts("\nwPuts");
		delay1s(1, __LINE__);
		//----------------------
		// util.c
		//----------------------
		TWmemset((BYTE *)&wTemp, 0, 0);

		//----------------------
		// main.c
		//----------------------
		//----------------------
		// host.c
		//----------------------

		//----------------------
		// I2C
		//--------------------
		CheckI2C(0x8A);
		WriteI2C_B0(0x8A, 0, &bTemp, 0);
		ReadI2C_B0(0x8A, 0, &bTemp, 0);

		//--------------------
		// Scaler
		//--------------------
		//ScalerSetOutputFixedVline(OFF);
		//ScalerSetFieldOffset(OFF);
		//ScalerWriteXUpReg(0);
		//ScalerReadXUpReg();
//		ScalerWriteXDownReg(0);
//#if defined(SUPPORT_PC) || defined(SUPPORT_DVI) || defined(SUPPORT_HDMI_EP9351)
//		ScalerReadXDownReg();
//#endif
		//ScalerSetHScaleReg(0,0);
		//ScalerSetHScale_FULL(0); //internal	
		//ScalerSetHScale_Panorama(0);	//internal
//		ScalerSetHScaleWithRatio(0,0);	
//		ScalerSetHScale(0);	
//		ScalerWriteVScaleReg(0);
//		ScalerReadVScaleReg();
//		ScalerSetVScaleWithRatio(0,0);
//		ScalerSetVScale(0);
//		ScalerPanoramaOnOff(OFF);
//		ScalerSetPanorama(0,0);
//		ScalerReadLineBufferDelay();
//		ScalerSetLineBufferSize(0);
		//ScalerSetLineBuffer(0,0);
		//ScalerSetFPHSOutputPolarity(BYTE fInvert)
		//ScalerWriteOutputHBlank(WORD length)
//		ScalerWriteHDEReg(0);
//		ScalerReadHDEReg();
//		ScalerCalcHDE();
//		ScalerReadOutputWidth();
		//ScalerWriteOutputWidth(WORD width)

		//ScalerSetHSyncPosLen(BYTE pos, BYTE len)
//		ScalerReadVDEReg();
//		ScalerWriteVDEReg(0);
		//ScalerWriteOutputHeight(WORD height)
		//ScalerReadOutputHeight();
//#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC) || defined(SUPPORT_DVI)
//		ScalerCalcVDE();
//#endif
		//ScalerSetVDEPosHeight(BYTE pos, WORD len)
		//ScalerSetOutputWidthAndHeight(WORD width, WORD height)
		//ScalerSetVSyncPosLen(BYTE pos, BYTE len)
//		ScalerWriteFreerunVtotal(0);
		//ScalerReadFreerunVtotal();
//		ScalerCalcFreerunVtotal();
		//ScalerSetVDEMask(BYTE top, BYTE bottom)
//		ScalerWriteFreerunHtotal(0);
		//ScalerReadFreerunHtotal();
//		ScalerCalcFreerunHtotal();
//		ScalerSetFreerunManual(OFF);
		//ScalerIsFreerunManual( void )
//		ScalerSetFreerunAutoManual(OFF,OFF);
//		ScalerSetMuteAutoManual(OFF,OFF);
//		ScalerSetMuteManual(OFF);
//		ScalerSetFreerunValue(ON);
//		ScalerCheckPanelFreerunValue();
		//--------------------
		// Measure
		//--------------------
		//void MeasSetWindow( WORD x, WORD y, WORD w, WORD h )
		//void MeasSetField(BYTE field)
		//void MeasLockData(BYTE fLock)
		//BYTE MeasStartMeasure(void)
		//BYTE MeasSetInputClkForHPeriodReg(BYTE fOn)
		//MeasGetInputClkForHPeriodReg();
		//void MeasSetNoiseMask(BYTE mask)
		MeasSetErrTolerance(0);
		MeasEnableChangedDetection(0);
		MeasEnableDeMeasure(0);
		MeasSetThreshold(0);
		//DWORD MeasReadPhaseR(void)
		//DWORD MeasReadPhaseG(void)
		//DWORD MeasReadPhaseB(void)
		//DWORD MeasReadPhaseRGB(void)
		MeasGetVPeriod();
		MeasGetHPeriod();
		MeasGetHTotal(1);
		//BYTE MeasGetHFreq(void)
		MeasGetHSyncRiseToFallWidth();
		MeasGetHSyncRiseToHActiveEnd();
		MeasGetVSyncRiseToFallWidth();
		MeasGetVsyncRisePos();
		MeasGetHActive(&wTemp);
		//MesaGetHPol(void)
		//#if defined(SUPPORT_PC) || defined(SUPPORT_DVI) || defined(SUPPORT_HDMI)
		//WORD MeasGetDviDivider(void)
		//#endif
		
		//MeasGetHActive2();
		MeasGetVActive(&wTemp);
		//MeasGetVActive2();
#if defined(SUPPORT_COMPONENT) || defined(SUPPORT_PC) || defined(SUPPORT_DVI) || defined(SUPPORT_HDMI_EP9351)
		MeasGetVPeriod27();
#endif
		//MeasGetVFreq();

		//#if defined(SUPPORT_PC)	BYTE MeasRoundDownVFreqValue(BYTE vfreq)
		//#ifdef SUPPORT_PC	void AutoTunePhase(void)
		//#ifdef SUPPORT_PCvoid AutoTuneColor(void)
		//#ifdef SUPPORT_PC MeasCheckVPulse(void)
#if defined(SUPPORT_DVI)
		CheckMeasure();
#endif
		// 
		//--------------------
#if !defined(SUPPORT_COMPONENT) && !defined(SUPPORT_PC)
//		Dummy_ARGB_func();
#endif
#if !defined(SUPPORT_DVI) && !defined(SUPPORT_HDMI)
		Dummy_DTV_func();
#endif
#ifndef SUPPORT_HDMI_EP907M
		Dummy_HDMI_EP907M_func();
#endif


		//----------------------------
		//Trick for Bank Code Segment
		//----------------------------
#ifndef SUPPORT_FOSD_MENU
		Dummy_FosdMenu_func();
		Dummy_FosdInitTable_func();
		Dummy_FosdDispInfo_func();
		Dummy_FosdString_func();
#endif
#ifdef SUPPORT_FOSD_MENU
		CheckAndClearFOsd();
#endif


#if defined(SUPPORT_HDMI_EP9351) || defined(SUPPORT_HDMI_EP907M)
#else
		Dummy_DebugMsg_func();
#endif


	}
	//----------------------------------------------------
	else {
		Printf("\nInvalid command...");
	}
	Prompt();
}

//=============================================================================
//  UART1. 57600bps 8Data 1Stop NoParity NoFlowControl
//=============================================================================

#ifdef SUPPORT_UART1
BYTE *Mon1String = 0;
void ProcessFinishMessage(void)
{
	Puts1("OK");
}
/**
* monitor1
*
*/

void Monitor1(void)
{
#if 0
	//test routine. - echo
	BYTE	ch;

	if( !RS1_ready() ) return;
	ch = RS1_rx();					// get data
	RS1_tx(ch);		   				// echo back
	RS_tx(ch);						// doublecheck at UART0
#endif
#if 1
	if( Mon1String ) {
		RS1_ungetch( *Mon1String++ );
		if( *Mon1String==0 ) Mon1String = 0;
	}

	if( !Mon1GetCommand() ) return;

	//---------------- Get TW8835 Info -------------------
	if( !stricmp( argv1[0], "GE" ) ) {
		BYTE cmd;
		if(argc1 < 2)
			return;
		cmd = a2h( argv1[1] );
		switch(cmd) {
		case 0x01:	//get FW revision
			//_outchar1((BYTE)(FWVER>>8));
			//_outchar1((BYTE)FWVER);
			Printf1("\n%02bx.%02bx",(BYTE)(FWVER>>8),(BYTE)FWVER);
			ProcessFinishMessage();
			break;
		}	
	}
	//---------------- IR Key data -------------------
	else if( !stricmp( argv1[0], "IR" ) ) {
		BYTE RemoDataCode;
		BYTE cmd;
		BYTE value;

		if(argc1 < 2)
			return;
		cmd = a2h( argv1[1] );
		switch(cmd) {
		case 0x01:	RemoDataCode=REMO_STANDBY;	break;	//power
		case 0x02:	RemoDataCode=REMO_CHNUP;	break;	//up
		case 0x03:	RemoDataCode=REMO_CHNDN;	break;	//down
		case 0x04:	RemoDataCode=REMO_VOLDN;	break;	//left
		case 0x05:	RemoDataCode=REMO_VOLUP;	break;	//right
		case 0x06:	RemoDataCode=REMO_MENU;		break;	//enter
		case 0x07:	RemoDataCode=REMO_MUTE;		break;	//mute
		case 0x08:	RemoDataCode=REMO_INPUT;	break;	//source
		case 0x09:	RemoDataCode=REMO_SELECT;	break;	//set
		case 0x0A:	RemoDataCode=REMO_EXIT;		break;	//end
		case 0x0B:	BackLightUp();	ProcessFinishMessage();	return;	//light up
		case 0x0C:	BackLightDown(); ProcessFinishMessage();	return;	//light down
		case 0x0D:	RemoDataCode=REMO_TTXRED;	break;	//function key1: define to Display "All RED"
		case 0x0E:	RemoDataCode=REMO_TTXGREEN;	break;	//function key2: define to Display "All Green"
		case 0x0F:	RemoDataCode=REMO_TTXCYAN;	break;	//function key3: define to Display "All Blue"
		case 0x1F:	
			value = 25; //a2h( argv1[2] );
			ImgAdjSetBrightnessY(value);
			break;	//test
		case 0x20:	
			value = 35; //a2h( argv1[2] );
			ImgAdjSetContrastY(value);
			break;	//test
		default:	RemoDataCode = 0;			break;
		}
		if(RemoDataCode) {
			ActionRemo(RemoDataCode,0);
			ProcessFinishMessage();
		}
	}
	//---------------- Read data from EEPROM -------------------
	else if( !stricmp( argv1[0], "RE" ) ) {
		BYTE index;
		if(argc1 < 2)
			return;
		index = a2h( argv1[1] );
		if(index >= EE_MAX_INDEX)
			return;
		Printf1("\n%02bx",EE_Read(index));
		ProcessFinishMessage();
	}
	//---------------- Write data to EEPROM -------------------
	else if( !stricmp( argv1[0], "WE" ) ) {
		BYTE index,dat;
		if(argc1 < 3)
			return;

		index = a2h( argv1[1] );
		if(index >= EE_MAX_INDEX)
			return;
		dat = a2h( argv1[2] );
		
		EE_Write(index, dat);
		ProcessFinishMessage();
	}
	//---------------- Read data from audio -------------------
	else if( !stricmp( argv1[0], "RA" ) ) {
		if(argc1 < 2)
			return;
	}
	//---------------- Read data to audio amplifier -------------------
	else if( !stricmp( argv1[0], "WA" ) ) {
		if(argc1 < 2)
			return;
	}
	//---------------- Help -----------------------------
	else if( !stricmp( argv1[0], "H" ) || !stricmp( argv1[0], "HELP" ) || !stricmp( argv1[0], "?" ) ) {
		Mon1Help();
		ProcessFinishMessage();
	}
	//----------------------------------------------------
	else {
		Printf1("\nInvalid command...");
	}
	Prompt1();

#endif
}
#endif

//---------------- check clock-------------------------
// command checkclock
//
// step.
//	access 0
//	mcu intc
//	checkclock and than type enter.
//  mcu intc again
//	test checkclock again
//..
//Note: When FW read SystemClock, use SFR_EA.
//
void Test_Checkclock2(void)
{}
void Test_Checkclock(void)
{
	//WORD ii, loop_max;
	volatile DWORD count;
	volatile DWORD WaitTime,CapturedSystemClock;
	DWORD count_72_low=0, count_72_high;
	DWORD count_54_low=0, count_54_high;
	DWORD count_36_low=0, count_36_high;
	DWORD count_27_low=0, count_27_high;
	DWORD lloopp;
	WORD WaitInterval=50; //200*10mSec


	//loop_max = 0x1000;
	Printf("\nPress any key to Quit...");

	SFR_CACHE_EN = 0;
	WriteTW88(REG400, ReadTW88(REG400) & ~0x04);
	SFR_EX0	= 0;
	SFR_EINT4 = 0;
	SFR_ET1 = 0;
	DisableRemoInt();
	lloopp = 0;

	while(!RS_ready()) {
		Printf("\r%ld",lloopp);
		lloopp++;
		//--------------------
		//Printf("\n27MHz..");
		WriteTW88(REG4E1, ReadTW88(REG4E1) & 0xCF);
		count = 0;
		WaitOneVBlank();
		SFR_EA = 0;
		CapturedSystemClock = SystemClock;
		SFR_EA = 1;
		WaitTime = CapturedSystemClock + WaitInterval;
		do {
			count++;
			SFR_EA = 0;
			CapturedSystemClock = SystemClock;
			SFR_EA = 1;
		} while( WaitTime > CapturedSystemClock);
		//Printf(" count:%ld",count);
		if(count_27_low==0) {
			count_27_low = count * 0.8;
			count_27_high = count * 1.2;
			Printf("\n27MHz...");
			Printf(" count:%ld",count);
			Printf(" %ld~%ld",count_27_low,count_27_high);
		}
		else {
			if(count < count_27_low || count > count_27_high) {
				Printf("\nFAIL:27MHz..");
				Printf(" count:%ld",count);
				Printf(" WaitTime:%ld,SystemClock:%ld",WaitTime,CapturedSystemClock);
				break;
			}
			Printf(".");
		}


		//Printf("\nPLL 72..");
		WriteTW88(REG4E0, ReadTW88(REG4E0) | 0x01);
		WriteTW88(REG4E1, 0x21);
		count = 0;
		WaitOneVBlank();
		SFR_EA = 0;
		CapturedSystemClock = SystemClock;
		SFR_EA = 1;
		WaitTime = CapturedSystemClock + WaitInterval;
		do {
			count++;
			SFR_EA = 0;
			CapturedSystemClock = SystemClock;
			SFR_EA = 1;
		} while( WaitTime > CapturedSystemClock);
		//Printf(" count:%ld",count);
		if(count_72_low==0) {
			count_72_low = count * 0.8;
			count_72_high = count * 1.2;
			Printf("\nPLL 72..");
			Printf(" count:%ld",count);
			Printf(" %ld~%ld",count_72_low,count_72_high);
		}
		else {
			if(count < count_72_low || count > count_72_high) {
				Printf("\nFAIL:PLL 72..");
				Printf(" count:%ld",count);
				Printf(" WaitTime:%ld,SystemClock:%ld",WaitTime,CapturedSystemClock);
				break;
			}
			Printf(".");
		}


		//Printf("\nPLL 54..");
		WriteTW88(REG4E1, 0x22);
		count = 0;
		WaitOneVBlank();
		SFR_EA = 0;
		CapturedSystemClock = SystemClock;
		SFR_EA = 1;
		WaitTime = CapturedSystemClock + WaitInterval;
		do {
			count++;
			SFR_EA = 0;
			CapturedSystemClock = SystemClock;
			SFR_EA = 1;
		} while( WaitTime > CapturedSystemClock);
		//Printf(" count:%ld",count);
		if(count_54_low==0) {
			count_54_low = count * 0.8;
			count_54_high = count * 1.2;
			Printf("\nPLL 54..");
			Printf(" count:%ld",count);
			Printf(" %ld~%ld",count_54_low,count_54_high);
		}
		else {
			if(count < count_54_low || count > count_54_high) {
				Printf("\nFAIL:PLL 54..");
				Printf(" count:%ld",count);
				Printf(" WaitTime:%ld,SystemClock:%ld",WaitTime,CapturedSystemClock);
				break;
			}
			Printf(".");
		}




		//Printf("\nPLL 36..");
		WriteTW88(REG4E1, 0x24);
		count = 0;
		WaitOneVBlank();
		SFR_EA = 0;
		CapturedSystemClock = SystemClock;
		SFR_EA = 1;
		WaitTime = CapturedSystemClock + WaitInterval;
		do {
			count++;
			SFR_EA = 0;
			CapturedSystemClock = SystemClock;
			SFR_EA = 1;
		} while( WaitTime > CapturedSystemClock);
		//Printf(" count:%ld",count);
		if(count_36_low==0) {
			count_36_low = count * 0.8;
			count_36_high = count * 1.2;
			Printf("\nPLL 36..");
			Printf(" count:%ld",count);
			Printf(" %ld~%ld",count_36_low,count_36_high);

			Printf("\n");
		}
		else {
			if(count < count_36_low || count > count_36_high) {
				Printf("\nFAIL:PLL 36..");
				Printf(" count:%ld",count);
				Printf(" WaitTime:%ld,SystemClock:%ld",WaitTime,CapturedSystemClock);
				break;
			}
			Printf(".");
		}



		//Printf("\nPLL 27..");
		WriteTW88(REG4E1, 0x26);
		count = 0;
		WaitOneVBlank();
		SFR_EA = 0;
		CapturedSystemClock = SystemClock;
		SFR_EA = 1;
		WaitTime = CapturedSystemClock + WaitInterval;
		do {
			count++;
			SFR_EA = 0;
			CapturedSystemClock = SystemClock;
			SFR_EA = 1;
		} while( WaitTime > CapturedSystemClock);
		//Printf(" count:%ld",count);
		if(count < count_27_low || count > count_27_high) {
			Printf("\nFAIL:PLL 27..");
			Printf(" count:%ld",count);
			Printf(" WaitTime:%ld,SystemClock:%ld",WaitTime,CapturedSystemClock);
			break;
		}
		Printf(".");



		//--------------------
		//Printf("\nPCLK1 72..");
		WriteTW88(REG4E1, 0x01);	//27MHz first
		WriteTW88(REG4E0, ReadTW88(REG4E0) & ~0x01);
		WriteTW88(REG4E1, 0x21);
		count = 0;
		WaitOneVBlank();
		SFR_EA = 0;
		CapturedSystemClock = SystemClock;
		SFR_EA = 1;
		WaitTime = CapturedSystemClock + WaitInterval;
		do {
			count++;
			SFR_EA = 0;
			CapturedSystemClock = SystemClock;
			SFR_EA = 1;
		} while( WaitTime > CapturedSystemClock);
		//Printf(" count:%ld",count);
		if(count < count_72_low || count > count_72_high) {
			Printf("\nFAIL:PCLK 72..");
			Printf(" count:%ld",count);
			Printf(" WaitTime:%ld,SystemClock:%ld",WaitTime,CapturedSystemClock);
			break;
		}
		Printf(".");


		//Printf("\nPCLK1 54..");
		WriteTW88(REG4E1, 0x22);
		count = 0;
		WaitOneVBlank();
		SFR_EA = 0;
		CapturedSystemClock = SystemClock;
		SFR_EA = 1;
		WaitTime = CapturedSystemClock + WaitInterval;
		do {
			count++;
			SFR_EA = 0;
			CapturedSystemClock = SystemClock;
			SFR_EA = 1;
		} while( WaitTime > CapturedSystemClock);
		//Printf(" count:%ld",count);
		if(count < count_54_low || count > count_54_high) {
			Printf("\nFAIL:PCLK 54..");
			Printf(" count:%ld",count);
			Printf(" WaitTime:%ld,SystemClock:%ld",WaitTime,CapturedSystemClock);
			break;
		}
		Printf(".");


		//Printf("\nPCLK1 36..");
		WriteTW88(REG4E1, 0x24);
		count = 0;
		WaitOneVBlank();
		SFR_EA = 0;
		CapturedSystemClock = SystemClock;
		SFR_EA = 1;
		WaitTime = CapturedSystemClock + WaitInterval;
		do {
			count++;
			SFR_EA = 0;
			CapturedSystemClock = SystemClock;
			SFR_EA = 1;
		} while( WaitTime > CapturedSystemClock);
		//Printf(" count:%ld",count);
		if(count < count_36_low || count > count_36_high) {
			Printf("\nFAIL:PCLK 36..");
			Printf(" count:%ld",count);
			Printf(" WaitTime:%ld,SystemClock:%ld",WaitTime,CapturedSystemClock);
			break;
		}
		Printf(".");

		//Printf("\nPCLK1 27..");
		WriteTW88(REG4E1, 0x26);
		count = 0;
		WaitOneVBlank();
		SFR_EA = 0;
		CapturedSystemClock = SystemClock;
		SFR_EA = 1;
		WaitTime = CapturedSystemClock + WaitInterval;
		do {
			count++;
			SFR_EA = 0;
			CapturedSystemClock = SystemClock;
			SFR_EA = 1;
		} while( WaitTime > CapturedSystemClock);
		//Printf(" count:%ld",count);
		if(count < count_27_low || count > count_27_high) {
			Printf("\nFAIL:PCLK 27..");
			Printf(" count:%ld",count);
			Printf(" WaitTime:%ld,SystemClock:%ld",WaitTime,CapturedSystemClock);
			break;
		}
		Printf(".");


		//restore
		WriteTW88(REG4E1, 0x01);	//27MHz

		//Printf("+++");
		//Printf("\r");
	}
	SFR_CACHE_EN = 1;
}
