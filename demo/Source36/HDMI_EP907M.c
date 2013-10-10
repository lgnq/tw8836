/**
 * @file
 * HDMI_EP907M.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	HDMI EP9553 device driver
*/

#include "Config.h"
#include "typedefs.h"

#include "cpu.h"
#include "printf.h"
#include "util.h"

#include "i2c.h"

#include "main.h"
#include "dtv.h"

#include "EP907M_RegDef.h"
//#include "HDMI_EP9553E.h"
#include "EP9553_RegDef.h"

#include "HDMI_EP907M.h"

/*
Debug Message

MHL Connect => TW8836 Powerup SUCCESS
==============================
 [KL_166 130129]

HDCP Key download from MCU
HDMI(EP9553E) Controller Version 0.26

Select Port1 MCLK256 
EP9x53C: Power Up
TV System: Switch to MHL mode
AC Power: On
------------------------------
LINK: CBUS Connect
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Retry 7 SET_HPD Success
LINK: Set DCAP_RDY OK
LINK: Set DCAP_CHG OK
-------------------------
LINK: Set PATH_EN OK
-------------------------
LINK: Set HPD OK
EP9x53C: Timing Chg ...
EP9x53C: Signal Valid
EP9x53C: HDMI
EP9x53C: ACP Chg
EP9x53C: AVI Chg
EP9x53C: Timing Chg VTIME_Code=4
EP9x53C: VMute Dsb
LINK: Signal found
-------------------------
LINK: Device Cap waiting Timeout
LINK: RCP support = 1
LINK: LD_GUI support = 1

MHL Connect => TW8836 Powerup FAIL
==============================
 [KL_166 130129]

HDCP Key download from MCU
HDMI(EP9553E) Controller Version 0.26

Select Port1 MCLK256 
EP9x53C: Power Up
TV System: Switch to MHL mode
AC Power: On

MHL Connect => TW8836 Powerup FAIL
==============================
 [KL_166 130129]

HDCP Key download from MCU
HDMI(EP9553E) Controller Version 0.26

Select Port1 MCLK256 
EP9x53C: Power Up
TV System: Switch to MHL mode
AC Power: On
------------------------------
LINK: CBUS Connect
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Retry 7 SET_HPD Success
LINK: Set DCAP_RDY OK
LINK: Set DCAP_CHG OK
-------------------------
LINK: Set PATH_EN OK
-------------------------
LINK: Set HPD OK
EP9x53C: Timing Chg ...
EP9x53C: Signal Valid
EP9x53C: HDMI
EP9x53C: Timing Chg VTIME_Code=4
EP9x53C: VMute Dsb
LINK: Signal found
-------------------------
LINK: Device Cap waiting Timeout
LINK: RCP support = 1
LINK: LD_GUI support = 1
EP9x53C: VMute Enabled
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg VTIME_Code=4
EP9x53C: VMute Dsb
EP9x53C: VMute Enabled
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg VTIME_Code=4
EP9x53C: VMute Dsb
EP9x53C: VMute Enabled
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg VTIME_Code=4
EP9x53C: VMute Dsb




TW8836 POWERUP => Connect MHL
==============================
 [KL_166 130129]

HDCP Key download from MCU
HDMI(EP9553E) Controller Version 0.26

Select Port1 MCLK256 
EP9x53C: Power Up
TV System: Switch to MHL mode
AC Power: On
------------------------------
LINK: CBUS Connect
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Retry 7 SET_HPD Success
LINK: Set DCAP_RDY OK
LINK: Set DCAP_CHG OK
-------------------------
LINK: Set PATH_EN OK
-------------------------
LINK: Set HPD OK
EP9x53C: Timing Chg ...
EP9x53C: Signal Valid
EP9x53C: HDMI
EP9x53C: Timing Chg VTIME_Code=4
EP9x53C: VMute Dsb
LINK: Signal found
-------------------------
LINK: Device Cap waiting Timeout
LINK: RCP support = 1
LINK: LD_GUI support = 1
EP9x53C: VMute Enabled
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg VTIME_Code=4
EP9x53C: VMute Dsb
EP9x53C: VMute Enabled
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg VTIME_Code=4
EP9x53C: VMute Dsb
EP9x53C: VMute Enabled
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg VTIME_Code=4
EP9x53C: VMute Dsb
EP9x53C: VMute Enabled
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg VTIME_Code=4
EP9x53C: VMute Dsb
EP9x53C: VMute Enabled
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg VTIME_Code=4
EP9x53C: VMute Dsb
EP9x53C: VMute Enabled
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg VTIME_Code=4
EP9x53C: VMute Dsb
EP9x53C: VMute Enabled
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg VTIME_Code=4
EP9x53C: VMute Dsb
EP9x53C: VMute Enabled
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg VTIME_Code=4
EP9x53C: VMute Dsb

HOT PLUG
========
........Disconnect Cable
TV System: Switch to HDMI mode
EP9x53C: VMute Enabled
EP9x53C: No Signal
EP9x53C: AVI Chg
EP9x53C: Timing Chg ...
EP9x53C: Timing Chg VTIME_Code=0
EP9x53C: ACP Time Out
EP9x53C: VMute Dsb
.............Connect Cable
TV System: Switch to MHL mode
LINK: Signal missing
TV System: Switch to HDMI mode
TV System: Switch to MHL mode
------------------------------
LINK: CBUS Connect
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Err: CBUS RQ - No Data Received
Retry 7 SET_HPD Success
LINK: Set DCAP_RDY OK
LINK: Set DCAP_CHG OK
-------------------------
LINK: Set PATH_EN OK
-------------------------
LINK: Set HPD OK
EP9x53C: VMute Enabled
EP9x53C: Timing Chg ...
EP9x53C: Signal Valid
EP9x53C: HDMI
EP9x53C: ACP Chg
EP9x53C: AVI Chg
EP9x53C: Timing Chg VTIME_Code=4
EP9x53C: VMute Dsb
LINK: Signal found
-------------------------
LINK: Device Cap waiting Timeout
LINK: RCP support = 1
LINK: LD_GUI support = 1


*/





#ifndef SUPPORT_HDMI_EP907M
//==========================================
//----------------------------
/**
* Trick for Bank Code Segment
*/
//----------------------------
CODE BYTE DUMMY_HDMI_EP907M_CODE;
void Dummy_HDMI_EP907M_func(void)
{
	BYTE temp;
	temp = DUMMY_HDMI_EP907M_CODE;
}
#else //..SUPPORT_HDMI_EP907M
//==========================================

/**
* EDID data
*/

//#ifdef ON_CHIP_EDID_ENABLE
//code BYTE HDMI_EDID_DATA[] = {..};
//#endif


/*	HDCP Program

	HDCP key Area	$00~$27 : encrypted 40 56-bit HDCP keys
	BKSV key Area   $28     : 40-bit BKSV

	$48,Byte0[2] = 1; //EE_DIS: disable HDCP key downloading from external EE. HDCP keys are written by MCU.
	download HDCP
*/

/**
* HDCP key data
*/

//#ifdef ON_CHIP_HDCP_ENABLE
//#include "../HDCP_KEY/EP907M.c"
//#endif													   		



#ifdef UNCALLED_SEGMENT
#endif //..UNCALLED_SEGMENT





/**
* download EDID data from FW to EP9351 RAM
*
* On the develop
* @param addr. if 0, use HDMI_EDID_DATA[] on the code segment.
*/
//#ifdef ON_CHIP_EDID_ENABLE
//BYTE HdmiDownloadEdid(DWORD addr)
//{}
//#endif

/**
* download HDCP key from FW to EP9351 RAM
*
* On the develop
*/
//#ifdef ON_CHIP_HDCP_ENABLE
//BYTE HdmiDownloadDhcp(DWORD addr)
//{}
//#endif //..ON_CHIP_HDCP_ENABLE


/**
* init EP9351 HW.
*
* call from InitSystem.
* follow up the power up sequency. (curr, ignore)
* download EDID and HDCP.
* result:
*	PowerDown state & Mute state
*
* BugFix120802. Increase TempByte[6] to TempByte[20]
*/
//#if 0
//void Hdmi_SystemInit_EP907M(void)
//{}
//#endif
/**
* init EP9351 CHIP
* Power up, unmute and assign some default values.
*/
void HdmiInitEp907MChip(void)
{
	BYTE i;
	BYTE bTemp;

	//select PORT1.(HDMI port)
	//Printf("\nselect PORT1.(HDMI port)");
	Printf("\nUse KL_166 130123 or higher.");

	//check EP907M before we request something.
	for (i=0; i < 100; i++)
	{
		WriteI2CI16Byte(I2CID_EP907M, EP907M_System_Control, EP907M_System_Control__PORT_SEL__P2);

		//bTemp = ReadI2CI16Byte(I2CID_EP907M, 0x2100);
		//bTemp = ReadI2CI16Byte(I2CID_EP907M, 0x2100);
		delay1ms(10);
		bTemp = ReadI2CI16Byte(I2CID_EP907M, EP907M_System_Control);
		if ((bTemp & 0x30) == 0x10)
			break;
		delay1ms(10);
	}
	Printf(" wait:%d", (WORD)i);
	//WriteI2CI16Byte(I2CID_EP907M, EP907M_System_Control, EP907M_System_Control__PORT_SEL__P2);

	//Audio. MCLK=128.
	//WriteI2CI16Byte(I2CID_EP907M, EP907M_Audio_Output_Control, EP907M_Audio_Output_Control__AAM_EN | 0x01);

	WriteI2CI16Byte(I2CID_EP907M, EP907M_Audio_Output_Control, EP907M_Audio_Output_Control__AAM_EN | 0x01);

#if defined(EVB_10)
	//this board prefers the falling dege DCLK.
	WriteI2CI16Byte(I2CID_EP907M, EP907M_Video_Output_Control, 0x00);	 
#endif
}

#endif //..ifdef SUPPORT_HDMI_EP907M

