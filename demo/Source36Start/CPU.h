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
 

#define RS_BUF_MAX 	32

extern 			BYTE 	ext1_intr_flag;
extern DATA 	WORD 	tic_pc;
extern DATA 	WORD 	tic_task;


void InitCPU(void);
void InitISR(BYTE flag);
void EnableWatchdog(BYTE mode);
void DisableWatchdog(void);
void RestartWatchdog(void);

#define EXINT_7		0x01
//..
#define EXINT_14	0x80
void EnableExtInterrupt(BYTE intrn);
void DisableExtInterrupt(BYTE intrn);

BYTE RS_ready(void);
BYTE RS_rx(void);
void RS_ungetch(BYTE ch);
void RS_tx(BYTE tx_buf);
#ifdef DEBUG_UART
void DEBUG_dump_uart0(void);
#endif

BYTE RS1_ready(void);
BYTE RS1_rx(void);
void RS1_ungetch(BYTE ch);
void RS1_tx(BYTE tx_buf);

#define ClearRemoTimer()	{ SFR_T2CON = 0x00; }
#ifdef SUPPORT_TW8835
//TW8835 uses INT9
#define	EnableRemoInt()		{ RemoDataReady = 0;	SFR_E2IE |= 0x04; }
#define DisableRemoInt() 	{ SFR_E2IE &= 0xfb; }
#else
//TW8836 EBV1.0 uses INT11
#define	EnableRemoInt()		{ RemoDataReady = 0;	SFR_E2IE |= 0x10; }
#define DisableRemoInt() 	{ SFR_E2IE &= 0xEF; }
#endif

//void InitRemoTimer(void);


void delay1ms(WORD cnt);
void delay1s(WORD cnt_1s, WORD line);
//void delay10ms(WORD cnt);


WORD DiffTime_ms( WORD stime, WORD etime );

#ifdef SUPPORT_FOSD_MENU
WORD GetTime_ms(void);
BYTE GetTime_H(void);
BYTE GetTime_M(void);
BYTE GetSleepTimer(void);
void SetSleepTimer(BYTE stime);
#endif



#endif // __CPU_H__
