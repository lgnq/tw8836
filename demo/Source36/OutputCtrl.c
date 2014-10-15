/**
 * @file
 * OutputCtrl.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	video output module 
*/

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
//#include "main.h"
#include "OutputCtrl.h"
#include "SOsd.h"
#include "FOsd.h"



//LEDC
//R0E0
 //==============================================================================
// void	LedBackLight( BYTE on )
//==============================================================================
/**
* control LEDC digital block
*/
void LedBackLight(BYTE on)
{
	WaitVBlank(1);
	WriteTW88Page(PAGE0_LEDC);
	if (on)
		WriteTW88(REG0E0, ReadTW88(REG0E0 ) | 1 );
	else
		WriteTW88(REG0E0, ReadTW88(REG0E0 ) & ~0x01 );
}

//=============================================================================
//	void	BlackScrren( BYTE on )
//=============================================================================
#ifdef UNCALLED_SEGMENT_CODE
void	BlackScreen( BYTE on )
{...}
#endif


//-----------------------------------------------------------------------------
/**
* LEDOn step
*/
#if 0
#ifndef MODEL_TW8836FPGA	//BK120914
BYTE LEDCOn(BYTE step)
{
	BYTE i;

	WriteTW88Page(PAGE0_LEDC);

	switch(step)
	{
	case 0:
		WriteTW88(REG0E0, 0x72);	//default. & disable OverVoltage
		WriteTW88(REG0E5, 0x80);	//LEDC digital output enable.
		WriteTW88(REG0E0, 0x12);	//Off OverCurrent. Disable Protection
		WriteTW88(REG0E0, 0x13);	//LEDC digital block enable
		break;
	case 1:
		WriteTW88(REG0E0, 0x11);	//Analog block powerup
		break;
	case 2:
		WriteTW88(REG0E0, 0x71);	//enable OverCurrent, enable Protection control
		break;
	//default:
	//	ePuts("\nBUG");
	//	return;
	}
	for(i=0; i < 10; i++) {
		if((ReadTW88(REG0E2) & 0x30)==0x30) {	//wait normal
			//dPrintf("\nLEDC(%bd):%bd",step,i);
			return ERR_SUCCESS;	//break;
		}
		delay1ms(2);
	}
	dPrintf("\nLEDC(%bd) FAIL",step);
	return ERR_FAIL;
}
#endif
#endif

void LedPowerUp(void)
{
	volatile BYTE bTemp;
	
#ifdef SUPPORT_E1H_PANEL
	BYTE speed;
#endif

	Printf("\nLED Powerup");

#ifdef SUPPORT_E1H_PANEL
	WriteI2CByte(0x58, 0x01, 0x05);
#endif

	//WaitVBlank(1);

	//Printf("\nI2CID_ISL97671A INDEX:1 DATA:0x5");
	WriteI2CByte(I2CID_ISL97671A, 0x01, 0x05);
	bTemp = ReadI2CByte(I2CID_ISL97671A, 0x01);
	if ((bTemp & 0x05) != 0x05)
	{
		Printf(" ID:58 Idx:1 W:0x05 R:%bx", bTemp);

		WriteI2CByte(I2CID_ISL97671A, 0x01, 0x05);
		bTemp = ReadI2CByte(I2CID_ISL97671A, 0x01);
		if ((bTemp & 0x05) != 0x05)
		{
			Printf(" W:0x05 R:%bx", bTemp);
		}
	}
}

void BT656OutputEnable(BYTE fOn, BYTE clear_port)
{
	DECLARE_LOCAL_page

	ReadTW88Page(page);
	WriteTW88Page(PAGE0_INPUT);
	if (fOn)
	{
#ifdef SUPPORT_UART1
		Printf("\nBUGBUG:BT656 and UART1..");
#else
		WriteTW88(REG007, ReadTW88(REG007) | 0x08);	 
#endif
	}
	else
	{
		WriteTW88(REG007, ReadTW88(REG007) & ~0x08);	//DataInitNTSC clear it. 
		//clear port
		if (clear_port)
		{
			if (P1_6 == 0)
				P1_6 = 1;
		}
	}
	WriteTW88Page(page);
}

//-----------------------------------------------------------------------------
/**
* enable Output pin
*
* DataOut need EnableOutputPin(ON,ON)
* target R008 = 0x89
*/
void OutputEnablePin(BYTE fFPDataPin, BYTE fOutputPin)
{
	BYTE value;

	Printf("\nFP_Data:%s OutputPin:%s", fFPDataPin ? "On" : "Off", fOutputPin ? "On" : "Off");

	WriteTW88Page(PAGE0_GENERAL);
//	WriteTW88(REG008, 0x80 | (ReadTW88(REG008) & 0x0F));	//Output enable......BUGBUG
	value = ReadTW88(REG008) & ~0x30;
	if (fFPDataPin == 0)
		value |= 0x20;
	if (fOutputPin == 0)
		value |= 0x10;
	WriteTW88(REG008, value);
}

