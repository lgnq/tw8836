/**
 * @file
 * misc.c 
 * @author Harry Han
 * @author YoungHwan Bae
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	main file
 * @section DESCRIPTION
 *	developer code
 ******************************************************************************
 */

#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "Global.h"
#include "CPU.h"
#include "Printf.h"
#include "Monitor.h"

#include "I2C.h"
#include "SPI.h"

#include "main.h"
#include "misc.h"

#include "SOsd.h"
#include "FOsd.h"
#include "Scaler.h"
#include "InputCtrl.h"
#include "EEPROM.h"
#include "ImageCtrl.h"
#include "decoder.h"
#include "InputCtrl.h"
#include "OutputCtrl.h"
#include "Settings.h"
#include "measure.h"

#include "SOsdMenu.h"


//=============================================================================
// I2CCMD routines
//=============================================================================

#if defined(MODEL_TW8835_SLAVE)
//-----------------------------------------------------------------------------
/**
* stop internal MCU
*
* To resume, write REG0D4[1]=1 by I2C
*/
#ifdef USE_EXTMCU_ISP_I2CCMD
void StopCPU(void)
{
	Printf("\nGoto STOP mode");
	Printf("\nTo resume, write REG0D4[1]=1 by I2C");
	SFR_PCON |= 0x12;
	//----- need nop to clean up the 8051 pipeline.
	_nop_(); _nop_(); _nop_(); _nop_(); _nop_();
	_nop_(); _nop_(); _nop_(); _nop_(); _nop_();
	//----------------------------
	//to reboot, 
	//	write REG0D4[0]=1.
	//----------------------------		
}
#endif
#endif

#if defined(SUPPORT_I2CCMD_SLAVE_V1)
//-----------------------------------------------------------------------------
// I2CCMD proto type
//-----------------------------------------------------------------------------
BYTE I2CCMD_Read(void);
BYTE I2CCMD_Exec(void);
BYTE I2CCMD_Sfr(void);
BYTE I2CCmd_eeprom(void);
BYTE I2CCmd_key(void);

//-----------------------------------------------------------------------------
/**
 * I2CCMD main
 *
 * @ingroup I2CCMD
 * @see	I2CCMD_Read
 * @see	I2CCMD_Exec
 * @see	I2CCMD_Sfr
 * @see	I2CCmd_eeprom
 * @see	I2CCmd_key
*/
BYTE I2CCMD_exec_main(void)
{
	BYTE ret;
	//I2CCMD_REG0~I2CCMD_REG4

	Printf("\nI2CCMD %bx:%bx:%bx:%bx:%bx",
		ReadTW88(I2CCMD_REG0),
		ReadTW88(I2CCMD_REG1),
		ReadTW88(I2CCMD_REG2),
		ReadTW88(I2CCMD_REG3),
		ReadTW88(I2CCMD_REG4));

	switch(ReadTW88(I2CCMD_REG0)) {
	case 0:		ret = I2CCMD_Read();	break;
	case 1:		ret = I2CCMD_Exec();	break;
	case 2:		ret = I2CCMD_Sfr();		break;
	case 3:		ret = I2CCmd_eeprom();	break;
	case 4:		ret = I2CCmd_key();		break;
	default:	ret = ERR_FAIL;
	}
	if(ret!=ERR_SUCCESS) {	
		WriteTW88(REG009, 0xE0);
	}
	return ret;
}
//-----------------------------------------------------------------------------
/**
 * I2CCMD read commad
 *
 * @ingroup I2CCMD
 * @see	I2CCMD_exec_main
*/
BYTE I2CCMD_Read(void)
{
	BYTE cmd;
	BYTE ret;

	ret = ERR_FAIL;

	cmd = ReadTW88(I2CCMD_REG1);
	if(cmd==0) {
		WriteTW88(I2CCMD_REG4,InputMain);
		ret = ERR_SUCCESS;
	}
	else if(cmd==1) {
		WriteTW88(I2CCMD_REG4,MenuGetLevel());
		ret = ERR_SUCCESS;
	}
	else if(cmd==2) {
		WriteTW88(I2CCMD_REG4,DebugLevel);
		ret = ERR_SUCCESS;
	}
	else if(cmd==3) {
		WriteTW88(I2CCMD_REG4,access);
		ret = ERR_SUCCESS;
	}
	else if(cmd==4) {
		WriteTW88(I2CCMD_REG4,access);
		if(SFR_WDCON & 0x02) WriteTW88(I2CCMD_REG4,0x01);	//On
		else                 WriteTW88(I2CCMD_REG4,0x00);	//Off
		ret = ERR_SUCCESS;
	}
	else if(cmd==5) {
		WriteTW88(I2CCMD_REG3,(WORD)SPI_Buffer>>8);
		WriteTW88(I2CCMD_REG4,(BYTE)SPI_Buffer);
		ret = ERR_SUCCESS;
	}
	else if(cmd==6) {
		WriteTW88(I2CCMD_REG3,SPI_BUFFER_SIZE>>8);
		WriteTW88(I2CCMD_REG4,(BYTE)SPI_BUFFER_SIZE);
		ret = ERR_SUCCESS;
	}

	if(ret==ERR_SUCCESS)
		WriteTW88(REG009,0xD0);	//Done
	else
		WriteTW88(REG009,0xF0);	//FAIL

	return ret;
}
//-----------------------------------------------------------------------------
/**
 * I2CCMD EXE commad
 *
 * @ingroup I2CCMD
 * @see	I2CCMD_exec_main
*/
BYTE I2CCMD_Exec(void)
{
	BYTE cmd;
	BYTE ret;

	ret = ERR_FAIL;

	cmd = ReadTW88(I2CCMD_REG1);
	if(cmd < 0x0F) {
		//if(MenuGetLevel())
		//	MenuEnd();
		//if(SpiOsdIsOn())
		//	SpiOsdEnable(OFF);
		if(getNoSignalLogoStatus())
			RemoveLogo();

		//Change input
		switch(cmd) {
		case INPUT_CVBS:	ChangeCVBS();		ret=ERR_SUCCESS;	break;
		case INPUT_SVIDEO:	ChangeSVIDEO();		ret=ERR_SUCCESS;	break;
#ifdef SUPPORT_COMPONENT
		case INPUT_COMP:	ChangeCOMPONENT();	ret=ERR_SUCCESS;	break;
#endif
#ifdef SUPPORT_PC
		case INPUT_PC:		ChangePC();			ret=ERR_SUCCESS;	break;
#endif
#ifdef SUPPORT_DVI
		case INPUT_DVI:		ChangeDVI();		ret=ERR_SUCCESS;	break;
#endif
		case INPUT_HDMIPC:	
		case INPUT_HDMITV:	ChangeHDMI();		ret=ERR_SUCCESS;	break;
#ifdef SUPPORT_BT656_LOOP
		case INPUT_BT656:	ChangeBT656Loop();		ret=ERR_SUCCESS;	break;
#endif
#ifdef SUPPORT_LVDSRX
		case INPUT_LVDS:	ChangeLVDSRx();		ret=ERR_SUCCESS;	break;
#endif
		default: break;
		}		
	}
	else if(cmd==0x0F) {
		//if(MenuGetLevel())
		//	MenuEnd();
		//if(SpiOsdIsOn())
		//	SpiOsdEnable(OFF);

		InputModeNext();
		ret=ERR_SUCCESS;
	}
	else if(cmd==0x10) {
		ret = CheckAndSetInput();	
	}
	//FYI: if master changes an access as 0, we can not support i2ccmd anymore.
	//else if(cmd==0x11) {
	//	access = ReadTW88(I2CCMD_REG2);	
	//	ret=ERR_SUCCESS;
	//}
	else if(cmd==0x12) {
		DebugLevel = ReadTW88(I2CCMD_REG2);	
		ret=ERR_SUCCESS;	
	}
	else if(cmd==0x80) {
		WriteTW88(REG0D4, ReadTW88(REG0D4) | 0x01);
	}
#ifdef USE_EXTMCU_ISP_I2CCMD
	else if(cmd==0x8A) {
		//Write Done flag frist. Because, I will stop myself.
		WriteTW88(REG009,0xD0);	
		StopCPU();
		return 0;
	}
#endif
#if defined(SUPPORT_WATCHDOG) || defined(DEBUG_WATCHDOG)
	else if(cmd==0x90) {
		DisableWatchdog();
		ret=ERR_SUCCESS;
	}
	else if(cmd==0x91) {
		EnableWatchdog(0);
		ret=ERR_SUCCESS;
	}
#endif
	if(ret==ERR_SUCCESS)
		WriteTW88(REG009,0xD0);	//Done
	else
		WriteTW88(REG009,0xF0);	//FAIL

	return ret;
}

//-----------------------------------------------------------------------------
/**
 * I2CCMD SFR commad
 *
 * @ingroup I2CCMD
 * @see	I2CCMD_exec_main
*/
BYTE I2CCMD_Sfr(void)
{
	BYTE index;
	BYTE value;
	BYTE ret;

	index = ReadTW88(I2CCMD_REG2);

	if(ReadTW88(I2CCMD_REG1)==1) {
		//write
		value = ReadTW88(I2CCMD_REG3);
		ret = WriteSFR(index, value);
	}
	else {
		//read
		ret = ReadSFR(index);
		WriteTW88(I2CCMD_REG4,ret);	//result
	}
	WriteTW88(REG009,0xD0);	//Done

	return ERR_SUCCESS;
}
//-----------------------------------------------------------------------------
/**
 * I2CCMD eeprom commad
 *
 * @ingroup I2CCMD
 * @see	I2CCMD_exec_main
*/
BYTE I2CCmd_eeprom(void)
{
	WORD addr;
	BYTE value;

	addr = ReadTW88(I2CCMD_REG2);	addr <<=8;
	addr |= ReadTW88(I2CCMD_REG3);

	if(ReadTW88(I2CCMD_REG1)==1) {
		//write
		value = ReadTW88(I2CCMD_REG4);
		EE_Write(addr,value);
	}
	else {
		//read
		value = EE_Read(addr);
		WriteTW88(I2CCMD_REG4,value);
	}
	WriteTW88(REG009,0xD0);	//Done

	return ERR_SUCCESS;
}

//-----------------------------------------------------------------------------
/**
 * I2CCMD Key commad
 *
 * @ingroup I2CCMD
 * @see	I2CCMD_exec_main
*/
BYTE I2CCmd_key(void)
{
	BYTE key;
	BYTE ret;

	ret = ERR_FAIL;
	key = ReadTW88(I2CCMD_REG1);

	if(key <= NAVI_KEY_RIGHT) {
		if(MenuGetLevel()) {
			MenuKeyInput(key);
			ret = ERR_SUCCESS;
		}
	}
	else if(key==0x06) {
		if(MenuGetLevel()==0) {
			MenuStart();
			ret = ERR_SUCCESS;
		}
	}
	else if(key==0x07) {
		if(MenuGetLevel()) {
			MenuEnd();
			ret = ERR_SUCCESS;
		}
	}
	else 
		ret = ERR_FAIL;

	if(ret==ERR_SUCCESS)
		WriteTW88(REG009, 0xD0);	//done

	return ret;
}
#endif //..SUPPORT_I2CCMD_SLAVE_V1

//-----------------------------------------------------------------------------
// I2CCMD 
//-----------------------------------------------------------------------------
#if defined(SUPPORT_I2CCMD_SLAVE_V1)
bit F_i2ccmd_exec=0;				/*!< I2CCMD flag */
#endif


#if defined(SUPPORT_I2CCMD_TEST_SLAVE)
BYTE ext_i2c_cmd;
#endif

#if defined(MODEL_TW8835_SLAVE) && defined(SUPPORT_I2CCMD_TEST)
BYTE i2c_compare_page = 0xff;
BYTE i2c_compare_buff[256];
//-----------------------------------------------------------------------------
/**
* test routine.
*/
void test_set_i2c_slave_compare_page(BYTE test_page)
{
	WORD i;
	i2c_compare_page = test_page;
	for(i=0; i <= 255; i++) {
		i2c_compare_buff[i] = ReadTW88(test_page << 8 | i);
	}
	Printf("\nsave page %bd",test_page);
}
#endif

	//
	// check PORT
	//
#if defined(MODEL_TW8835_SLAVE)
#ifdef MODEL_TW8835
void Init_I2CCMD_Slave(void)
{
	Puts("\nI2C with GPIO(P1_3/INT10) ");
	if(PORT_I2CCMD_GPIO_SLAVE) {
		Printf("Ready!!"); 

		SFR_E2IE  |= 0x08; //enable INT10 interrupt
	}
	else
		Printf("FAIL!!"); 
}
#endif
#endif

//=============================================================================
//	CHIP_MANUAL_TEST		                                               
//=============================================================================

