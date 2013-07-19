//=============================================================================
//		DP8051 SFRs
//=============================================================================
#ifndef __REGISTERS_H__
#define __REGISTERS_H__
/*
SFR Memory Map (TW8835)
===============
	0/8		1/9		2/A		3/B		4/C		5/D		6/E		7/F
	---		---		---		---		---		---		---		---
80:	P0		SP		DPL0	DPH0	DPL1	DPH1	DPS		PCON
88:	TCON	TMOD	TL0		TL1		TH0		TH1		CKCON
90:	P1		EIF		(WTST)	(DPX0)			(DPX1)
98:	SCON0	SBUF0	BANK	CACHE
A0:	P2
A8:	IE
B0:	P3
B8:	IP
C0:	SCON1	SBUF1	CCL1	CCH1	CCL2	CCH2	CCL3	CCH3
C8:	T2CON	T2IF	RLDL	RLDH	TL2		TH2		CCEN
D0:	PSW
D8:	WDCON
E0:	ACC				CAMC
E8:	EIE		STATUS	(MXAX) 	TA
F0:	B	
F8:	EIP				E2IF	E2IE	E2IP	E2IM	E2IT	__
------------------------------------------------------------------
Standard - standard 8051 register, not described in this document
bolded - new or modified nonstandard 8051 register, described in this document
( ) - reserved register, do not change
* Any reserved register bit shouldn.t be changed from reset value.
*/
/*
SFR Memory Map (TW8836)
===============
	0/8		1/9		2/A		3/B		4/C		5/D		6/E		7/F
	---		---		---		---		---		---		---		---
80:	P0		SP		DPL0	DPH0	DPL1	DPH1	DPS		PCON
88:	TCON	TMOD	TL0		TL1		TH0		TH1		CKCON
90:	P1		EXIF	(WTST)	(DPX0)			(DPX1)
98:	SCON0	SBUF0	BANK	ESP		CACHE	ACON	UART0FIFO UART1FIFO
A0:	P2
A8:	IE
B0:	P3
B8:	IP
C0:	SCON1	SBUF1	CCL1	CCH1	CCL2	CCH2	CCL3	CCH3
C8:	T2CON	T2IF	RLDL	RLDH	TL2		TH2		CCEN
D0:	PSW
D8:	WDCON
E0:	ACC				CAMC
E8:	EIE		STATUS	(MXAX) 	TA
F0:	B	
F8:	EIP				E2IF	E2IE	E2IP	E2IM	E2IT	__
------------------------------------------------------------------
Standard - standard 8051 register, not described in this document
bolded - new or modified nonstandard 8051 register, described in this document
( ) - reserved register, do not change
* Any reserved register bit shouldn.t be changed from reset value.

*/


//===========================================
//				BYTE Registers
//===========================================
sfr SFR_P0			= 0x80;	/* Port 0                    */
sfr SFR_SP			= 0x81;	/* Stack Pointer             */
sfr SFR_DPL			= 0x82;	/* Data Pointer 0 Low byte   */
sfr SFR_DPH			= 0x83;	/* Data Pointer 0 High byte  */
sfr SFR_DPL1		= 0x84; /* TW88 :: Data Pointer 1 Low byte */
sfr SFR_DPH1		= 0x85; /* TW88 :: Data Pointer 1 High byte */
sfr SFR_DPS			= 0x86; /* TW88 :: Data Pointers Select register */
sfr SFR_PCON		= 0x87;	/* Power Configuration       */
sfr SFR_TCON		= 0x88;	/* Timer 0,1 Configuration   */
sfr SFR_TMOD		= 0x89;	/* Timer 0,1 Mode            */
sfr SFR_TL0			= 0x8A;	/* Timer 0 Low byte counter  */
sfr SFR_TL1			= 0x8B;	/* Timer 1 Low byte counter  */
sfr SFR_TH0			= 0x8C;	/* Timer 0 High byte counter */
sfr SFR_TH1			= 0x8D;	/* Timer 1 High byte counter */
sfr SFR_CKCON		= 0x8E;	/* XDATA Wait States. DP8051 */

sfr SFR_P1			= 0x90;	/* Port 1                    */
sfr SFR_EXIF		= 0x91;	/* TW88 :: Extended Interrupt Priority register, aka. EIF */
sfr SFR_WTST		= 0x92;	/* Program Wait States       */
sfr SFR_DPX			= 0x93;	/* Data Page Pointer 0       */
sfr     DPX			= 0x93;	/* i2c.src need it */
sfr SFR_DPX1		= 0x95;	/* TW88 :: reserved */
sfr SFR_SCON		= 0x98;	/* Serial 0 Configuration    */
sfr SFR_SBUF		= 0x99;	/* Serial 0 I/O Buffer       */
sfr TWBASE			= 0x9a;	/* TW88 :: TW88xx base address. Code Bank Address */
sfr	SFR_BANKREG		= 0x9A;	/*      ::aka BANKREG */
							/*      [7-0]	RG_PGMBASE	R/W	CodeBank Address	def:0 */
//#ifdef MODEL_TW8835
//sfr SFR_CACHE_EN	= 0x9B;	/* TW88 :: Cache Control */
//#else
sfr SFR_ESP			= 0x9B;	/* DP80390 */
sfr SFR_CACHE_EN	= 0x9C;	/* TW88 :: Cache Control */
//#endif
							/*      [1]	PG_PWND		R/W PowerDown Cache */
							/*      [0]	CACHE_EN	R/W 1:Enable Cache 0:Disable Cache */

//#ifdef MODEL_TW8835
//#else
sfr SFR_ACON		= 0x9D;	/* TW88 :: Address Control FLAT/LARGE*/
                            /*      [0] AM0 1:24 bit address */  
sfr	SFR_UART0FIFO 	= 0x9E;	/* TW88 :: UART0 FIFO */
                            /*      [7] UART0FFEN UART0 FIFO enable  */ 
                            /*      [4] UART0FFOVF UART0 overflow status; write this bit with "1" to clear it */ 
                            /*      [3:0] UART0FFBYT UART0 FIFO valid byte number  */ 
sfr	SFR_UART1FIFO 	= 0x9F;	/* TW88 :: UART1 FIFO */
                            /*      [7] UART1FFEN UART1 FIFO enable  */ 
                            /*      [4] UART1FFOVF UART1 overflow status; write this bit with "1" to clear it */ 
                            /*      [3:0] UART1FFBYT UART1 FIFO valid byte number  */ 
//#endif

sfr SFR_P2			= 0xA0;	/* Port 2                    */		
sfr SFR_IE			= 0xA8;	/* Interrupt Enable          */	

sfr SFR_P3			= 0xB0;	/* Port 3                    */
sfr SFR_IP			= 0xB8;	/* Interrupt Priority        */

sfr SFR_SCON1		= 0xC0; /* TW88 :: UART1 configuration */
sfr SFR_SBUF1		= 0xC1;	/* TW88 :: UART1 I/O Buffer */
sfr	SFR_CCL1		= 0xC2; /* TW88 */
sfr	SFR_CCH1		= 0xC3; /* TW88 */
sfr	SFR_CCL2		= 0xC4; /* TW88 */
sfr	SFR_CCH2		= 0xC5;	/* TW88 */
sfr	SFR_CCL3		= 0xC6; /* TW88 */
sfr	SFR_CCH3		= 0xC7;	/* TW88 */

sfr SFR_T2CON		= 0xC8;	/* TW88 :: Timer2 configuration*/
sfr SFR_T2IF		= 0xC9;	/* TW88 :: Timer2 interrupt flag */
sfr SFR_CRCL		= 0xCA;	/* TW88 aka RLDL */
sfr SFR_CRCH		= 0xCB;	/* TW88 aka RLDH */
sfr SFR_TL2			= 0xCC;	/* TW88 :: Timer2 Low byte counter */	
sfr SFR_TH2			= 0xCD;	/* TW88 :: Timer2 High byte counter*/
sfr SFR_CCEN		= 0xCE;	/* TW88 */

sfr SFR_PSW			= 0xD0;	/* Program Status Word       */
sfr SFR_WDCON		= 0xD8;	/* TW88 :: watchdog control */

sfr SFR_ACC			= 0xE0;	/* Accumulator               */
sfr SFR_CAMC		= 0xE2;	/* TW88 :: Chip Access Mode Control. [0]=1b:16bit mode */
sfr SFR_EIE			= 0xE8;	/* TW88 ::Extended Interrupt Enable. watchdog & 6,5,4,3,2 */
sfr SFR_STATUS		= 0xE9;	/* Status register           */
sfr	SFR_MXAX		= 0xEA;	/* MOVX @Ri High address. DP80390     */
sfr SFR_TA			= 0xEB;	/* TW88 :: watchdog timed access register */
						
sfr SFR_B			= 0xF0;	/* B Working register        */
sfr SFR_EIP			= 0xF8;	/* TW88 :: Extended Interrupt Priority. watchdog & 6,5,4,3,2*/

sfr SFR_E2IF		= 0xFA;	/* TW88 :: INT 14~7 Flag */
sfr SFR_E2IE		= 0xFB;	/* TW88 :: INT 14~7 Enable */
sfr SFR_E2IP		= 0xFC;	/* TW88 :: INT 14-7 Priority */
sfr SFR_E2IM		= 0xFD;	/* TW88 :: INT 14~7 Active Control. 1:Edge Active, 0:Level Active */
sfr SFR_E2IT		= 0xFE;	/* TW88 :: INT 14~7 Edge/Level Polarity. 1:Rising 0:Folling Edge or Low Active */



//===========================================
//				BIT Registers
//===========================================

//--- P0
sbit P0_0  		= SFR_P0^0; //0x80;
sbit P0_1  		= SFR_P0^1; //0x81;
sbit P0_2  		= SFR_P0^2; //0x82;
sbit P0_3  		= SFR_P0^3; //0x83;
sbit P0_4  		= SFR_P0^4; //0x84;
sbit P0_5  		= SFR_P0^5; //0x85;
sbit P0_6  		= SFR_P0^6; //0x86;
sbit P0_7  		= SFR_P0^7; //0x87;
//--- P1
sbit P1_0  		= SFR_P1^0; //0x90;
sbit P1_1  		= SFR_P1^1; //0x91;
sbit P1_2  		= SFR_P1^2; //0x92;
sbit P1_3  		= SFR_P1^3; //0x93;
sbit P1_4  		= SFR_P1^4; //0x94;
sbit P1_5  		= SFR_P1^5; //0x95;
sbit P1_6  		= SFR_P1^6; //0x96;
sbit P1_7  		= SFR_P1^7; //0x97;
//--- (P1)
sbit SFR_T2    	= SFR_P1^0; //0x90;
sbit SFR_T2EX  	= SFR_P1^1;	//0x91;
//--- P2
sbit P2_0  		= SFR_P2^0; //0xA0;
sbit P2_1  		= SFR_P2^1; //0xA1;
sbit P2_2  		= SFR_P2^2; //0xA2;
sbit P2_3  		= SFR_P2^3; //0xA3;
sbit P2_4  		= SFR_P2^4; //0xA4;
sbit P2_5  		= SFR_P2^5; //0xA5;
sbit P2_6  		= SFR_P2^6; //0xA6;
sbit P2_7  		= SFR_P2^7; //0xA7;
//--- P3
sbit P3_0  		= SFR_P3^0; //0xB0;
sbit P3_1  		= SFR_P3^1; //0xB1;
sbit P3_2  		= SFR_P3^2; //0xB2;
sbit P3_3  		= SFR_P3^3; //0xB3;
sbit P3_4  		= SFR_P3^4; //0xB4;
sbit P3_5  		= SFR_P3^5; //0xB5;
sbit P3_6  		= SFR_P3^6; //0xB6;
sbit P3_7  		= SFR_P3^7; //0xB7;
//--- (P3)
sbit SFR_RXD   	= SFR_P3^0; //0xB0;
sbit SFR_TXD   	= SFR_P3^1; //0xB1;
sbit SFR_INT0  	= SFR_P3^2; //0xB2;
sbit SFR_INT1  	= SFR_P3^3; //0xB3;
sbit SFR_T0    	= SFR_P3^4; //0xB4;
sbit SFR_T1    	= SFR_P3^5; //0xB5;
sbit SFR_WR    	= SFR_P3^6; //0xB6;
sbit SFR_RD    	= SFR_P3^7; //0xB7;

//--- PSW
sbit SFR_P     = SFR_PSW^0; //0xD0;
sbit SFR_OV    = SFR_PSW^2; //0xD2;
sbit SFR_RS0   = SFR_PSW^3; //0xD3;
sbit SFR_RS1   = SFR_PSW^4; //0xD4;
sbit SFR_F0    = SFR_PSW^5; //0xD5;
sbit SFR_AC    = SFR_PSW^6; //0xD6;
sbit SFR_CY    = SFR_PSW^7; //0xD7;

//--- TCON
sbit SFR_IT0   = SFR_TCON^0; //0x88;
sbit SFR_IE0   = SFR_TCON^1; //0x89;
sbit SFR_IT1   = SFR_TCON^2; //0x8A;
sbit SFR_IE1   = SFR_TCON^3; //0x8B;
sbit SFR_TR0   = SFR_TCON^4; //0x8C;
sbit SFR_TF0   = SFR_TCON^5; //0x8D;
sbit SFR_TR1   = SFR_TCON^6; //0x8E;
sbit SFR_TF1   = SFR_TCON^7; //0x8F;
//--- T2CON
sbit SFR_CP_RL2= SFR_T2CON^0; //0xC8;
sbit SFR_C_T2  = SFR_T2CON^1; //0xC9;
sbit SFR_TR2   = SFR_T2CON^2; //0xCA;
sbit SFR_EXEN2 = SFR_T2CON^3; //0xCB;
sbit SFR_TCLK  = SFR_T2CON^4; //0xCC;
sbit SFR_RCLK  = SFR_T2CON^5; //0xCD;
sbit SFR_EXF2  = SFR_T2CON^6; //0xCE;
sbit SFR_TF2   = SFR_T2CON^7; //0xCF;

//--- IE
sbit SFR_EX0   = SFR_IE^0; //0xA8;
sbit SFR_ET0   = SFR_IE^1; //0xA9;
sbit SFR_EX1   = SFR_IE^2; //0xAA;
sbit SFR_ET1   = SFR_IE^3; //0xAB;
sbit SFR_ES    = SFR_IE^4; //0xAC;
sbit SFR_ET2   = SFR_IE^5; //0xAD;
sbit SFR_ES1   = SFR_IE^6; //0xAE;
sbit SFR_EA    = SFR_IE^7; //0xAF;

//--- IP
sbit SFR_PX0   = SFR_IP^0; //0xB8;
sbit SFR_PT0   = SFR_IP^1; //0xB9;
sbit SFR_PX1   = SFR_IP^2; //0xBA;
sbit SFR_PT1   = SFR_IP^3; //0xBB;
sbit SFR_PS    = SFR_IP^4; //0xBC;
sbit SFR_PT2   = SFR_IP^5; //0xBD;
sbit SFR_PS1   = SFR_IP^6; //0xBE;

//--- SCON
sbit SFR_RI    = SFR_SCON^0; //0x98;
sbit SFR_TI    = SFR_SCON^1; //0x99;
sbit SFR_RB8   = SFR_SCON^2; //0x9A;
sbit SFR_TB8   = SFR_SCON^3; //0x9B;
sbit SFR_REN   = SFR_SCON^4; //0x9C;
sbit SFR_SM2   = SFR_SCON^5; //0x9D;
sbit SFR_SM1   = SFR_SCON^6; //0x9E;
sbit SFR_SM0   = SFR_SCON^7; //0x9F;
//--- SCON1
sbit SFR_RI1   = SFR_SCON1^0; //0xc0;
sbit SFR_TI1   = SFR_SCON1^1; //0xc1;
sbit SFR_RB18  = SFR_SCON1^2; //0xc2;
sbit SFR_TB18  = SFR_SCON1^3; //0xc3;
sbit SFR_REN1  = SFR_SCON1^4; //0xc4;
sbit SFR_SM12  = SFR_SCON1^5; //0xc5;
sbit SFR_SM11  = SFR_SCON1^6; //0xc6;
sbit SFR_SM10  = SFR_SCON1^7; //0xc7;

//--- WDCON
sbit SFR_RWT   = SFR_WDCON^0;	//0xd8;	Run Watchdog Timer
sbit SFR_EWT   = SFR_WDCON^1;	//0xd9;	Enable Watchdog Timer
sbit SFR_WTRF  = SFR_WDCON^2;	//0xda;	Watchdog Timer Reset Flag
sbit SFR_WDIF  = SFR_WDCON^3;	//0xdb;	Watchdog Interrupt Flag

//--- EIE 
sbit SFR_EINT2 = SFR_EIE^0; //0xe8;	Enable Ex.Int 2
sbit SFR_EINT3 = SFR_EIE^1; //0xe9;	Enable Ex.Int 3
sbit SFR_EINT4 = SFR_EIE^2; //0xea;	Enable Ex.Int 4
sbit SFR_EINT5 = SFR_EIE^3; //0xeb;	Enable Ex.Int 5
sbit SFR_EINT6 = SFR_EIE^4; //0xec;	Enable Ex.Int 6
sbit SFR_EWDI  = SFR_EIE^5; //0xed;	Enable WatchDog Int

//--- EIP
sbit SFR_PINT2 = SFR_EIP^0; //0xf8;	Priority Ex.Int 2
sbit SFR_PINT3 = SFR_EIP^1; //0xf9;	Priority Ex.Int 3
sbit SFR_PINT4 = SFR_EIP^2; //0xfa;	Priority Ex.Int 4
sbit SFR_PINT5 = SFR_EIP^3; //0xfb;	Priority Ex.Int 5
sbit SFR_PINT6 = SFR_EIP^4; //0xfc;	Priority Ex.Int 6
sbit SFR_PWDI  = SFR_EIP^5; //0xfd;	Priority Watchdog Int

#endif  //__REGISTERS_H__











