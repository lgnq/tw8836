/**
 * @file
 * i2c.c
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	a device driver for the iic-bus interface 
 ******************************************************************************
 */
#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"
#include "global.h"

#include "printf.h"
#include "CPU.h"
#include "util.h"

#include "I2C.h"

#include "SOsd.h"

#include <intrins.h>

//------------------------
// only for FAST or Slow environment
//------------------------

//BYTE I2C_Bus;	//0:Bus0 for TW8835, 1:BUS1 for TW8809, 2:BUS2 for TW8832.
//BYTE I2C_Idx;	//I2C address index mode. 0:8bit 1:16bit.
//BYTE I2C_Speed=1;	//0:normal, 1:slow, 2:for HS_mode, but not implememted.

/**
* I2C Buffer
*/
BYTE I2C_Buffer[16];
/**
* I2C delay base value
*/
BYTE I2C_delay_base = 1;


//=============================
// I2C TIME CHART
//=============================
/**
* I2C TIME CHART
*
*
    -+      +----------+---------+...----+   ...-+     +-------+
SDA  |      |          |         |       |       |     |       |
     +------+----------+---------+...    +---...-+-----+       +--
												
SCL ---+     +----+      +----+      +----+          +------------+
       |     |    |      |    |      |    |          |         	  |
       +-----+    +------+    +-..---+    +--......--+            +--
     | |     |    |    | |           |  |          | | |
     |1|     |  3 |5   |6|           | 4|          |9|7|   8   |
     |                                  |              |
     |                                  |              |
     +START                              +RESTART      +STOP
   (4 1 5) (6  3   5)                              (9 7 8)
																	   TW8835 Slave	
																	   MIN		MAX
1: Hold Time START Condition											74ns	-
2: Clock Low Time(5+6)
3: Clock High Time								i2c_delay_clockhigh		
4: Setup Time for a Repeated START Condition							370ns	-
5: Data Hold Time								i2c_delay_datahold		50ns	900ns
6: Data Setup Time								i2c_delay_datasetup		74ns	-
7: Set-up time for STOP condition										370ns	-
8: Bus Free Time between a STOP and a START Condition 					740ns	 -
9: prepare time for STOP condition
A: ack wait time


I2C SPEED
=========
	slow			depend on system.
	Standard		100kbps
	Fast mode		400kbps
	Fast mode plus	1Mbit
	High-Speed		3.4Mbit				HS-mode

	TW8836 uses about 35kHz.

*/

//-----------------------
// I2C DELAY
// Note: It depends on CACHE, MCUSPI clock, & SPIOSD.
//-----------------------

#pragma SAVE
#pragma OPTIMIZE(2,SIZE)
static void dd(BYTE delay)
{
	BYTE i,j;
	j = I2C_delay_base;
	while(j--) {
		for(i=0; i < delay; i++);
	}
}
#pragma RESTORE

                                    //32kHz
#define I2CDelay_1		dd(8)		//13uS				
#define I2CDelay_2		dd(13)		//22uS		
#define I2CDelay_3		dd(5)		//9.6uS		
#define I2CDelay_4		dd(37)		//	
#define I2CDelay_5		dd(5)		//
#define I2CDelay_6		dd(8)		//
#define I2CDelay_7		dd(37)		//	
#define I2CDelay_8		dd(74)		//	
#define I2CDelay_9		dd(10)		//
#define I2CDelay_ACK	dd(8)		//

//=============================================================================
/**
* I2C subroutines
*/
//=============================================================================
#pragma SAVE
#pragma OPTIMIZE(2,SIZE)
static BYTE I2C_SetSclWait(void)
{
#if 0 //I2C TEST 
	I2C_SCL=1;		
	while(I2C_SCL==0); //NOTE:It can hangup the system.
	return 0;	
#else
	BYTE i;
	I2C_SCL=1;		
	for(i=0; i < 250; i++) {
		dd(10);
		if(I2C_SCL != 0)
			break;
	}
	if(i>=250) {
		Printf("\nI2C_SetSclWait Fail");
		return 1;	//fail
	}
	return 0;
#endif
}
#pragma RESTORE

static void I2C_Start(void)
{
	I2C_SDA = 1;	
	I2C_SCL = 1;	
							I2CDelay_4;
	I2C_SDA = 0;			I2CDelay_1;
	I2C_SCL = 0;			I2CDelay_5;
}

static void I2C_Stop(void)
{
	I2C_SDA = 0;			I2CDelay_9;
	I2C_SetSclWait();		I2CDelay_7;
	I2C_SDA = 1;			I2CDelay_8;	
}
static BYTE I2C_WriteData(BYTE value)
{
	BYTE error;
	BYTE i;

	for(i=0;i<8;i++) {
		if(value & 0x80)	I2C_SDA = 1;
		else				I2C_SDA = 0;
							I2CDelay_6;
		I2C_SCL = 1; 		I2CDelay_3;
		I2C_SCL = 0;		I2CDelay_5;

		value <<=1;
	}
	I2C_SDA = 1;			//listen for ACK
	                        
	I2C_SetSclWait();		I2CDelay_ACK;
	if(I2C_SDA)	error=1;
	else        error=0;
	                        
	I2C_SCL=0;				I2CDelay_5;

	return error;
}
static BYTE I2C_ReadData(BYTE fLast)
{
	BYTE i;
	BYTE val=0;

	for(i=0; i <8; i++) {
							I2CDelay_6;
		I2C_SetSclWait();	I2CDelay_3;
		val <<= 1;
		if(I2C_SDA)
			val |= 1;
		I2C_SCL=0;			I2CDelay_5;
	}
	if(fLast)	I2C_SDA = 1;	//last byte
	else		I2C_SDA = 0;

	I2C_SetSclWait();		I2CDelay_3;
	I2C_SCL=0;
	I2C_SDA=1;				I2CDelay_5;
	return val;
}



//=============================================================================
/**
* I2C global Functions
*
* BYTE CheckI2C(BYTE i2cid)
*
* BYTE WriteI2CByte(BYTE i2cid, BYTE index, BYTE val)
* void WriteI2C(BYTE i2cid, BYTE index, BYTE *val, WORD cnt)
* BYTE ReadI2CByte(BYTE i2cid, BYTE index)
* void ReadI2C(BYTE i2cid, BYTE index, BYTE *val, BYTE cnt)
*
BYTE WriteI2CI16Byte(BYTE i2cid, WORD index, BYTE val)
BYTE WriteI2CI16(BYTE i2cid, WORD index, BYTE *val, WORD cnt)
BYTE ReadI2CI16Byte(BYTE i2cid, WORD index)
BYTE ReadI2CI16(BYTE i2cid, WORD index, BYTE *val, BYTE cnt)
*/	                                                                          
//=============================================================================
/**
* check I2C device
*
* I2C commands use a infinity loop. 
* Use CheckI2C() first before you use other I2C commands.
*
* @return
*	0: success
*	1: NAK
*	2: I2C dead
*
* NOTE: I am not using I2CSetSclWait().
*/
BYTE CheckI2C_B0(BYTE i2cid)
{
	BYTE error;
	BYTE i;
	BYTE value;

	value = i2cid;
	SFR_EA = 0;
	I2C_Start();

	for (i=0; i<8; i++)
	{
		if (value & 0x80)
			I2C_SDA = 1;
		else
			I2C_SDA = 0;

		I2CDelay_6;
		I2C_SCL = 1;
		I2CDelay_3;
		I2C_SCL = 0;
		I2CDelay_5;

		value <<=1;
	}
	I2C_SDA = 1;			//listen for ACK.                      
	I2C_SCL = 1; 				I2CDelay_ACK;
	dd(100);
	if (I2C_SCL == 0)
		error = 2;	//I2C dead
	else
	{
		if (I2C_SDA)
			error = 1;	//NAK
		else
			error = 0;	//ACK
	}                        
	I2C_SCL = 0;
	I2CDelay_5;

	//stop routine
	I2C_SDA = 0;
	I2CDelay_9;                      
	I2C_SCL = 1;
	I2CDelay_7;
	I2C_SDA = 1;
	I2CDelay_8;	

	SFR_EA = 1;
	
	return error;
}

/**
* write one byte data to I2C slave device
*
* @param i2cid - 8bit.
* @param index - 8bit 
* @param data
*/
BYTE WriteI2CByte_B0(BYTE i2cid, BYTE index, BYTE val)
{
	BYTE ret;

	//test only
	//if(i2cid==I2CID_SX1504)
	//	return 0;


	SFR_EA=0;
	I2C_Start();
	ret  = I2C_WriteData(i2cid);	ret <<=1;
	ret |= I2C_WriteData(index);	ret <<= 1;
	ret |= I2C_WriteData(val);
	I2C_Stop();
	SFR_EA=1;

#if defined(DEBUG_I2C)
	if(ret)
		Printf("\nWriteI2CByte[%bx:%bx,%bx] FAIL:%bx",i2cid, index,val, ret);
#endif
	return ret;
}

/**
* write one byte data to I2C slave device
*
* @param i2cid - 8bit.
* @param index - 16bit 
* @param data
*/
BYTE WriteI2CI16Byte(BYTE i2cid, WORD index, BYTE val)
{
	BYTE ret;

	SFR_EA=0;
	I2C_Start();
	ret  = I2C_WriteData(i2cid);		ret <<=1;
	ret |= I2C_WriteData(index>>8);		ret <<= 1;
	ret |= I2C_WriteData((BYTE)index);	ret <<= 1;
	ret |= I2C_WriteData(val);
	I2C_Stop();
	SFR_EA=1;

#if defined(DEBUG_I2C)
	if(ret)
		Printf("\nWriteI2CI16Byte[%bx:%x,%bx] FAIL:%bx",i2cid, index,val, ret);
#endif
	return ret;
}

/**
* write data to I2C slave device
*
* @param i2cid - 8bit
* @param index - 8bit
* @param *val. NOTE: Do not use a CodeSegment
* @param count
*/
BYTE WriteI2C_B0(BYTE i2cid, BYTE index, BYTE *val, WORD cnt)
{
	WORD i;
	BYTE ret;

	SFR_EA=0;
	I2C_Start();
	ret  = I2C_WriteData(i2cid);	ret <<=1;
	ret |= I2C_WriteData(index);	ret <<=1;
	for(i=0;i<cnt;i++) 
		ret |= I2C_WriteData(val[i]);
	I2C_Stop();
	SFR_EA=1;
#if defined(DEBUG_I2C)
	if(ret)
		Printf("\nWriteBlock2C[%bx:%bx,%bx] FAIL:%bx",i2cid, index,val, ret);
#endif
	return ret;
}

#if 0
/**
* write data to I2C slave device
*
* @param i2cid - 8bit
* @param index - 16bit
* @param *val. NOTE: Do not use a CodeSegment
* @param count
*/
BYTE WriteI2CI16(BYTE i2cid, WORD index, BYTE *val, WORD cnt)
{
	WORD i;
	BYTE ret;

	SFR_EA=0;
	I2C_Start();
	ret  = I2C_WriteData(i2cid);		ret <<=1;
	ret |= I2C_WriteData(index>>8);		ret <<=1;
	ret |= I2C_WriteData((BYTE)index);	ret <<=1;
	for(i=0;i<cnt;i++) 
		ret |= I2C_WriteData(val[i]);
	I2C_Stop();
	SFR_EA=1;
#if defined(DEBUG_I2C)
	if(ret)
		Printf("\nWriteSlowI2C[%bx:%bx,%bx] FAIL:%bx",i2cid, index,val, ret);
#endif
	return ret;
}
#endif

/**
* read one byte data from I2C slave device
*
* @param i2cid - 8bit
* @param index - 8bit
* @return data
*/
BYTE ReadI2CByte_B0(BYTE i2cid, BYTE index)
{
	BYTE value;
	BYTE ret;

	//test only
	//if(i2cid==I2CID_SX1504)
	//	return 0xFF;

	SFR_EA=0;
	I2C_Start();		  
	ret = I2C_WriteData(i2cid);		ret<<=1;
	ret +=I2C_WriteData(index);		ret<<=1;
	I2C_Start();
	ret +=I2C_WriteData(i2cid | 0x01);
	value=I2C_ReadData(1);
	I2C_Stop();
	SFR_EA=1;

#if defined(DEBUG_I2C)
	if(ret)
		Printf("\nReadI2CByte[%bx:%bx] FAIL:%bx",i2cid,index, ret);
#endif
	return value;
}

/**
* read one byte data from I2C slave device
*
* @param i2cid - 8bit
* @param index - 16bit
* @return data
*/
BYTE ReadI2CI16Byte(BYTE i2cid, WORD index)
{
	BYTE value;
	BYTE ret;

	SFR_EA=0;
	I2C_Start();		  
	ret= I2C_WriteData(i2cid);				ret<<=1;
	ret+=I2C_WriteData(index>>8);			ret<<=1;
	ret+=I2C_WriteData((BYTE)index);		ret<<=1;
	I2C_Start();
	ret+=I2C_WriteData(i2cid | 0x01);
	value=I2C_ReadData(1);
	I2C_Stop();
	SFR_EA=1;

#if defined(DEBUG_I2C)
	if(ret)
		Printf("\nReadI2CI16Byte[%bx:%x] FAIL:%bx",i2cid,index, ret);
#endif
	return value;
}

/**
* read data from I2C slave device
*
* @param i2cid - 8bit
* @param index - 8bit 
* @param *val - read back buffer
* @param count
* @return 0:success, other:fail
*/
BYTE ReadI2C_B0(BYTE i2cid, BYTE index, BYTE *val, BYTE cnt)
{
	BYTE i;
	BYTE ret;

	SFR_EA=0;	
	I2C_Start();
	ret  = I2C_WriteData(i2cid);		ret <<=1;
	ret |= I2C_WriteData(index);		ret <<=1;
	I2C_Start();
	ret |= I2C_WriteData(i2cid | 0x01);
	cnt--;
	for(i=0; i<cnt; i++){
		val[i]=I2C_ReadData(0);
	}
	val[i]=I2C_ReadData(1);
  
	I2C_Stop();
	SFR_EA=1;
#if defined(DEBUG_I2C)
	if(ret)
		Printf("\nReadI2C[%bx:%bx] FAIL:%bx",i2cid,index, ret);
#endif
	return ret;
}

/**
* read data from I2C slave device.
*
* @param i2cid - 8bit
* @param index - 16bit
* @param *val - read back buffer
* @param count
* @return 0:success, other:fail
*/
BYTE ReadI2CI16(BYTE i2cid, WORD index, BYTE *val, BYTE cnt)
{
	BYTE i;
	BYTE ret;

	SFR_EA=0;	
	I2C_Start();
	ret  = I2C_WriteData(i2cid);		ret <<=1;
	ret |= I2C_WriteData(index>>8);		ret <<=1;
	ret |= I2C_WriteData((BYTE)index);	ret <<=1;
	I2C_Start();
	ret |= I2C_WriteData(i2cid | 0x01);
	cnt--;
	for(i=0; i<cnt; i++){
		val[i]=I2C_ReadData(0);
	}
	val[i]=I2C_ReadData(1);

	I2C_Stop();
	SFR_EA=1;
#if defined(DEBUG_I2C)
	if(ret)
		Printf("\nReadI2CI16[%bx:%x] FAIL:%bx",i2cid,index, ret);
#endif
	return ret;
}


//=============================================================================
//                                                                           
//=============================================================================

/**
*
* speed  I2C_SPEED_SLOW 
* 		0:normal, 1:slow, 2:for HS_mode, but not implememted.
*       Current code supports only normal & slow.
*
* NOTE: We can not use ReadI2C() and WriteI2C() function pointer.
*       See error C212.
*/

/**
* set I2C speed
*/

/** 
* Print current I2C property
*/


