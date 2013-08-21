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
			}	
			else if( !stricmp( argv[1], "core" ) ) {	
				InitCore(0);
			}
			else if( !stricmp( argv[1], "NTSC" ) ) {
			}
			else if( !stricmp( argv[1], "panel" ) ) {
				DCDC_StartUP();
//				ScalerSetMuteManual(OFF);
				LedPowerUp();
			}
			else if( !stricmp( argv[1], "sspll" ) ) {
			}
			else if( !stricmp( argv[1], "logo" ) ) {
			}
			else if( !stricmp( argv[1], "default" ) ) {
//				InitInputAsDefault();
			} 
		}
	}
	//-----input select------------------------------------------
	else if ( !stricmp( argv[0], "imain") ) {
	}
	else if ( !stricmp( argv[0], "iBT656") ) {
	}
	//---------------- CheckAndSet ---------------------
	else if ( !stricmp( argv[0], "CheckAndSet" ) ) {
	}
	//---------------- check -------------------------
	else if ( !stricmp( argv[0], "check" ) ) {
	}

	//---------------- SPI Debug -------------------------
	else if( !stricmp( argv[0], "SPI" ) ) {
		MonitorSPI();
	}
	else if( !stricmp( argv[0], "SPIC" ) ) {
		MonitorSPIC();
	}
	//---------------- EEPROM Debug -------------------------
	//---------------- MENU Debug -------------------------

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
	}
	//---------------- System Clock Display -----------------
	else if ( !stricmp( argv[0], "time" ) ) {
			Printf("\nSystem Clock: %ld:%5bd", SystemClock, tic01);
	}
	//---------------- HDMI test -------------------------
	
	//====================================================
	// OTHER TEST ROUTINES
	//====================================================

	//==========================================
	// FontOSD Test
	//==========================================
	//==========================================
	// SpiOSD Test
	//==========================================

	//---------------- TOUCH Debug -------------------------	
	//---------------- Touch Calibration -------------------------
	//---------------- Delta RGB Panel Test -------------------------
	//---------------- HDMI -------------------

	//----------------------------------------------------
	// scaler test
	//
	// 0: old
	// 1: new
	// 2: target:720x480P
	//----------------------------------------------------

	//----------------------------------------------------
	//make compiler happy.
	//Please, DO NOT EXECUTE
	//----------------------------------------------------	
	else if(!stricmp( argv[0], "compiler" )) {
		WORD wTemp;
		BYTE bTemp;

		wPuts("\nwPuts");
		delay1s(1, __LINE__);
		//----------------------
		// util.c
		//----------------------
		TWmemset((BYTE *)&wTemp, 0, 0);
		ePuts("\ntest");

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
		ReadI2CI16(0x00, 0x000, (BYTE *)&wValue, 1);

		SpiFlashDmaRead(0,0,0,0);

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
		//--------------------
		// Measure
		//--------------------
		

		//----------------------------
		//Trick for Bank Code Segment
		//----------------------------


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

