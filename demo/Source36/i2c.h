#ifndef	__I2C_H__
#define	__I2C_H__

/**
* assign I2C port
*/
#define	I2C_SCL		P1_0
#define	I2C_SDA		P1_1

extern BYTE I2C_delay_base;
extern BYTE I2C_Buffer[16];

BYTE CheckI2C_B0(BYTE i2cid);
BYTE ReadI2CByte_B0(BYTE i2cid, BYTE index);
BYTE ReadI2C_B0(BYTE i2cid, BYTE index, BYTE *buff, BYTE cnt);
BYTE WriteI2CByte_B0(BYTE i2cid, BYTE index, BYTE val);
BYTE WriteI2C_B0(BYTE i2cid, BYTE index, BYTE *buff, WORD cnt);	//Note: cnt uses WORD.

BYTE ReadI2CI16Byte(BYTE i2cid, WORD index);
BYTE ReadI2CI16(BYTE i2cid, WORD index, BYTE *buff, BYTE cnt);
BYTE WriteI2CI16Byte(BYTE i2cid, WORD index, BYTE val);
BYTE WriteI2CI16(BYTE i2cid, WORD index, BYTE *buff, WORD cnt); //Note: cnt uses WORD.

//=================
// global macro
//=================
#define CheckI2C(i2cid)					CheckI2C_B0(i2cid)
#define ReadI2CByte(i2cid,index)		ReadI2CByte_B0(i2cid,index) 
#define ReadI2C(i2cid,index,ptr,len)	ReadI2C_B0(i2cid,index,ptr,len) 
#define WriteI2CByte(i2cid,index,val)	WriteI2CByte_B0(i2cid,index,val) 
#define WriteI2C(i2cid,index,ptr,len)	WriteI2C_B0(i2cid,index,ptr,len) 

//=================
// I2C device
//=================
#define I2CID_TW9910		0x88
#define I2CID_SX1504		0x40	//4CH GPIO
#define I2CID_BU9969		0xE0	//Digital video encoder
#define I2CID_ADV7390		0xD6	//Digital video encoder. 12bit
#define I2CID_ADV7391		0x56	//Digital video encoder. 10bit

#define I2CID_ISL97671A		0x58	//LED BackLight

#define I2CID_SIL9127_DEV0	0x60	//SiliconImage HDMI receiver
#define I2CID_SIL9127_DEV1	0x68
#define I2CID_SIL9127_HDCP	0x74
#define I2CID_SIL9127_COLOR	0x64
#define I2CID_SIL9127_CEC	0xC0
#define I2CID_SIL9127_EDID	0xE0
#define I2CID_SIL9127_CBUS	0xE6

#define I2CID_EP9351		0x78	//Explorer HDMI receiver
#define I2CID_EP907M		0x78
#define I2CID_EP9553		0x78

#define I2CID_ISL97901		0x50	//ISL RGB LED Driver
#define I2CID_ADC121C021	0xAC	//12bit Analog2Digital Converter
#define I2CID_E330_FLCOS	0x7C	//FLCOS	 
#define I2CID_ISL97671		0x58	//PWM Dimming
//#define I2CID_PCA9306		0xFF	//Dual Bidir I2C Bus & SMBus Voltage Level Translator


#endif	// __I2C_H__

