#include "cpu.h"
#include "reg.h"
#include "TW8836.h"

volatile BYTE	XDATA *DATA regTW88 = REG_START_ADDRESS;

void init_cpu(void)
{
	TWBASE = 0x00;					// Base address of TW88xx
	SFR_CAMC = 1;					// Chip Access Mode Control. E2[0]=1b:16bit mode

	//---------- Initialize Timer Divider ---------
//	WriteTW88Page(PAGE4_CLOCK);

	WriteTW88(REG4E2, 0x69);		// Timer0 Divider : system tic 0. 
	WriteTW88(REG4E3, 0x78);		// 27M/27000 = 1msec

	WriteTW88(REG4E4, 0x01);		// Timer1 Divider : for Touch
	WriteTW88(REG4E5, 0x0e);		// 27M/270 = 10usec	

	WriteTW88(REG4E6, 0);			// Timer2 Divider : remo timer
	WriteTW88(REG4E7, 0x1b);		// 27M/27 = 1usec

	WriteTW88(REG4E8, 0);			// Timer3 Divider : baudrate for UART0
	WriteTW88(REG4E9, 0x0c);		// (22.1184M/16) / 12 = 115200bps on SM0=1	

	WriteTW88(REG4EA, 0);			// Timer4 Divider : baudrate for UART1
	WriteTW88(REG4EB, 0x0c);		// (22.1184M/16) / 12 = 115200bps on SM1=1	

	//---------- Initialize interrupt -------------
	SFR_CKCON = 0x00;		// Clock control register			
							// 0000 0000
							// |||| |||+-- MD0 \.
							// |||| ||+--- MD1 	> MD[2:0] Stretch RD/WR timing
							// |||| |+---- MD2 /
							// |||| +----- T0M:  Timer0 Pre-Divider 0=div by 12,  1=div by 4
							// |||+------- T1M:  Timer1 Pre-Divider 0=div by 12,  1=div by 4
							// ||+-------- ---
							// |+--------- WD0 \ Watchdong Timeout Period
							// +---------- WD1 / 00=2^17,  01=2^20,  10=2^23,  11=2^26

	SFR_TMOD = 0x66;		// 0110 0110
							        // |||| ||||   << Timer 0 >>
							        // |||| |||+-- M0 \  00= 8bit timer,counter /32  01= 16bit timer,counter
							        // |||| ||+--- M1 /  10= 8bit auto reload        11= 8bit timer,counter
							        // |||| |+---- CT:   0=Timer Mode,    1=Counter Mode
							        // |||| +----- GATE: 0=GATE not used, 1=GATE used
							        // ||||        << Timer 1 >>
							        // |||+------- M0 \  00= 8bit timer,counter /32  01= 16bit timer,counter
							        // ||+-------- M1 /  10= 8bit auto reload        11= 8bit timer,counter
							        // |+--------- CT:   0=Timer Mode,    1=Counter Mode
							        // +---------- GATE: 0=GATE not used, 1=GATE used

	SFR_TCON = 0x55;		// 0101 0101
											// |||| |||+-- IT0:  INT0 Trigger 0=level, 1=edge
											// |||| ||+--- IE0:  INT0 Interrupt Flag
											// |||| |+---- IT1:  INT1 Trigger 0=level, 1=edge
											// |||| +----- IE1:  INT1 Interrupt Flag
											// |||+------- TR0:  Timer0 Run
											// ||+-------- TF0:  Timer0 Flag
											// |+--------- TR1:  Timer1 Run
											// +---------- TF0:  Timer1 Flag
							
	SFR_TH0 = 0xf6;			// 10 msec
	SFR_TL0 = 0xf6;			//

							// for TOUCH SAR sensing timer
	SFR_TH1 = 206;			// 
							// TH1 = 156. 1ms
							// TH1 = 206. 0.5ms = 50*10usec

	SFR_PCON = 0xc0;		// 1100 0000
							// |||| |||+-- PMM:  Power Management Mode 0=Disable,  1=Enable
							// |||| ||+--- STOP: Stop Mode             0=Disable,  1=Enable
							// |||| |+---- SWB:  Switch Back from STOP 0=Disable,  1=Enable
							// |||| +----- ---
							// |||+------- PWE:	 (Program write Enable)
							// ||+-------- ---
							// |+--------- SMOD1:UART1 Double baudrate bit
							// +---------- SMOD0:UART0 Double baudrate bit

	SFR_SCON = 0x50;		// 0101 0000
							// |||| |||+-- RI:   Receive Interrupt Flag
							// |||| ||+--- TI:   Transmit Interrupt Flag
							// |||| |+---- RB08: 9th RX data
							// |||| +----- TB08: 9th TX data
							// |||+------- REN:	 Enable Serial Reception
							// ||+-------- SMO2: Enable Multiprocessor communication
							// |+--------- SM01 \   Baudrate Mode
							// +---------- SM00 / 00=f/12,  01=8bit var,  10=9bit,f/32,f/64,  11=9bit var

	SFR_SCON1 = 0x50;		// 0101 0000
							// |||| |||+-- RI:   Receive Interrupt Flag
							// |||| ||+--- TI:   Transmit Interrupt Flag
							// |||| |+---- RB08: 9th RX data
							// |||| +----- TB08: 9th TX data
							// |||+------- REN:	 Enable Serial Reception
							// ||+-------- SMO2: Enable Multiprocessor communication
							// |+--------- SM11 \   Baudrate Mode
							// +---------- SM10 / 00=f/12,  01=8bit var,  10=9bit,f/32,f/64,  11=9bit var

	SFR_IP	 = 0x02;		// 0000 0000 interrupt priority					
							// -  PS1 PT2 PS PT1 PX1 PT0 PX0	 		         

	//---------- Enable Individual Interrupt ----------
	// IE(0xA8) = EA ES1 ET2 ES  ET1 EX1 ET0 EX0
	SFR_EX0  = 0;			// INT0		: Chip Interrupt
	SFR_ET0  = 1;			// Timer0	: System Tic
	SFR_EX1  = 0;			// INT1		: DE end
	SFR_ET1  = 0;			// Timer1	: touch
	SFR_ET2  = 0;			// Timer2	: Remote
	SFR_ES   = 0;			// UART0  	: Debug port
	SFR_UART0FIFO = 0x80;	//          : UART0 FIFO

#ifdef SUPPORT_UART1
	SFR_ES1  = 1;			// UART1  	: External MCU
	SFR_UART1FIFO = 0x80;	//          : UART1 FIFO
#else
	SFR_ES1  = 0;			// UART1  	: External MCU
#endif

	SFR_EA   = 0;			// Global Interrupt

	//---------- Extended Interrupt -------------------
	// 0xe8	xx xx EWDI EINT6 EINT5 EINT4 EINT3 EINT2
	SFR_EINT2 = 0;			// INT2		: SPI-DMA done
	SFR_EINT3 = 0;			// INT3		: Touch Ready

//#ifdef MODEL_TW8835
//	SFR_EINT4 = 0;			// INT4		: reserved 
//#else
	SFR_EINT4 = 0;			// INT4		: SW 7FF 
//#endif

	SFR_EINT5 = 0;			// INT5		: reserved
	SFR_EINT6 = 0;			// INT6		: Pen
	//---------- Extended Interrupt 7~14 Config. ---------------
							// EINT14 EINT13 EINT12 EINT11 EINT10 EINT9 EINT8 EINT7
							//
							// EINT9 : Remocon. SW will enable 9 later.
							// EINT10: MODEL_TW8835_EXTI2C. VideoISR. 
							//		   MODEL_TW8835_SLAVE. Master I2C active. SW will enable INT10 later.
							// EINT14:
	SFR_E2IF  = 0x00;		// (0xFA)	0000 0000 : Interrupt Flag         
	SFR_E2IE  = 0x00;		// (0xFB)	0000 0000 : Interrupt Enable.
	SFR_E2IP  = 0x00;		// (0xFC)	0000 0000 : Interrupt Priority
	SFR_E2IM  = 0xff;		// (0xFD)	0000 0000 : Interrupt Mode(0=Edge,  1=Level)
	SFR_E2IT  = 0x00;		// (0xFE)	0000 0000 : Level/Edge Polarity  (0=Low/Falling, 1=High/Rising)	  0xFE
	//BKTODO120606 Why Level ?. 0x08

//	Puts("\n\n\n\n\nInitCPU ");
//	Puts("16Bit Access");

	//------- Remote Controller (INT9, Timer2) --------

	SFR_T2CON = 0x00;		//ClearRemoTimer. RemoINTR(EINT9) will be activateed in RemoTimer		

	//cache on :: sfr 9b = 1;
//	Puts(" CACHE");
//	SFR_CACHE_EN = 0x01;	//cache ON. No Power Down

//#ifdef MODEL_TW8835
//	Puts(" SFR EX0 ET0 EA");
//#else
//	Puts(" SFR EX0 ET0 EA EINT4");
//#endif

#ifdef MODEL_TW8836FPGA
	//TW8836 don't have PORT_NOINIT_MODE. FW starts as NOINIT
	SFR_EX0 = 0;
//	Printf("\nSKIP EX0");
#endif
}

