#ifndef __CPU_H__
#define __CPU_H__

//===================================================================
// TW8835 8051 Special Function Register (SFR)
//===================================================================

/*
I2C[8A]>mcu ds
Dump DP8051 SFR
SFR 80: FF 44 09 4B 00 00 00 C0 - D5 66 FF A3 FF 00 00 BF 
SFR 90: DF 04 07 00 BF 00 00 90 - 54 0D 01 01 00 90 00 BF 
SFR A0: FF BF BF BF BF BF BF BF - D7 BF BF BF BF BF BF BF 
SFR B0: 0F BF BF BF BF BF BF BF - 02 BF BF BF BF BF BF BF 
SFR C0: 50 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 BF 
SFR D0: 00 BF BF BF BF BF BF BF - 08 BF BF BF BF BF BF BF 
SFR E0: 00 BF 00 BF BF BF BF BF - 03 00 00 00 BF BF BF BF 
SFR F0: 01 BF BF BF BF BF BF BF - 00 BF 24 00 00 FF 00 BF 
I2C[8A]>mcu ws fb 4
MCU SFR Write: FB=04 (00) 00000000
I2C[8A]>mcu ds
Dump DP8051 SFR
SFR 80: FF 44 09 4B 00 00 00 C0 - D5 66 FF DA FF 00 00 BF 
SFR 90: DF 04 07 00 BF 00 00 90 - 54 0D 01 01 00 90 00 BF 
SFR A0: FF BF BF BF BF BF BF BF - F7 BF BF BF BF BF BF BF 
SFR B0: 0F BF BF BF BF BF BF BF - 02 BF BF BF BF BF BF BF 
SFR C0: 50 00 00 00 00 00 00 00 - 00 00 2E FF 3A FF 00 BF 
SFR D0: 00 BF BF BF BF BF BF BF - 08 BF BF BF BF BF BF BF 
SFR E0: 00 BF 00 BF BF BF BF BF - 03 00 00 00 BF BF BF BF 
SFR F0: 01 BF BF BF BF BF BF BF - 00 BF 20 04 00 FF 00 BF 
I2C[8A]>
I2C[8A]>
I2C[8A]>mcu ei
MCU extend Interrupt Status: 04, count: 2
I2C[8A]>
*/
 
#define	REG_START_ADDRESS	0xc000	// register start 0xc000, for page0, 0xc100-page1 

void init_cpu(void);

#endif // __CPU_H__
