#ifndef __SETTINGS_H__
#define __SETTINGS_H__

extern CODE BYTE TW8835_GENERAL[];
extern CODE BYTE TW8835_SSPLL[];
extern CODE BYTE TW8835_DECODER[];
extern CODE BYTE TW8835_ADCLLPLL[];
//extern CODE BYTE TW8835_SCALER[];
extern CODE BYTE TW8835_TCON[];
extern CODE BYTE TW8835_GAMMA[];
extern CODE BYTE TW8835_EN_OUTPUT[];
//extern CODE BYTE TW8835_INIT_TCON_SCALER[];
//extern CODE BYTE TW8835_INIT_ADC_PLL[];

void SW_Reset(void);
//for Monitor
void 	ClockHigh(void);
void 	ClockLow(void);
void 	Clock27(void);

void  SSPLL_PowerUp(BYTE fOn);

void  SspllSetFreqReg(DWORD FPLL);
DWORD SspllGetFreqReg(void);
void  SspllSetFreqAndPll(DWORD _PPF);
void SspllSetFreqAndPllAndDiv(DWORD _PPF, BYTE div); //new
DWORD SspllFREQ2FPLL(DWORD FREQ, BYTE POST);
DWORD SspllFPLL2FREQ(DWORD FPLL, BYTE POST);
BYTE  SspllGetPost(void);
DWORD SspllGetPPF(void);
void  SspllSetAnalogControl(BYTE value);

void  PclkSetDividerReg(BYTE divider);
DWORD PclkGetFreq(DWORD sspll);
DWORD PclkoGetFreq(DWORD pclk);
void  PclkoSetDiv(/*BYTE pol,*/ BYTE div);
void  PclkSetPolarity(BYTE pol);

void  ClkPllSetSelectReg(BYTE ClkPllSet);
void  ClkPllSetSpiInputClockLatch(BYTE property);
void  ClkPllSetDividerReg(BYTE divider);
void  ClkPllSetSelDiv(BYTE ClkPllSel, BYTE ClkPllDiv);
DWORD ClkPllGetFreq(void);

void AMuxSetInput(BYTE InputMode);

extern BYTE shadow_r4e0;
extern BYTE shadow_r4e1;
BYTE McuSpiClkToPclk(BYTE divider);
void McuSpiClkRestore(void);
BYTE McuSpiClkReadSelectReg(void);
#define MCUSPI_CLK_27M		0
#define MCUSPI_CLK_32K		1
#define MCUSPI_CLK_PCLKPLL	2

#define CLKPLL_SEL_PCLK		0
#define CLKPLL_SEL_PLL		1

#define CLKPLL_DIV_1P0		0
#define CLKPLL_DIV_1P5		1
#define CLKPLL_DIV_2P0		2
#define CLKPLL_DIV_2P5		3
#define CLKPLL_DIV_3P0		4
#define CLKPLL_DIV_4P0		6
#define CLKPLL_DIV_5P0		7

void McuSpiClkSet(BYTE McuSpiClkSel, BYTE ClkPllSel, BYTE ClkPllDiv); 
void McuSpiClkSelect(BYTE McuSpiClkSel);
DWORD McuGetClkFreq(void);


DWORD SpiClkGetFreq(DWORD mcu_clk);

void LLPLLSetClockSource(BYTE use_27M);
void SetDefaultPClock(void);

void DumpClock(void);
void DumpRegister(BYTE page);

BYTE DCDC_On(BYTE step);
BYTE DCDC_StartUP(void);

void FP_BiasOnOff(BYTE fOn);
void FP_PWC_OnOff(BYTE fOn);
void FP_GpioDefault(void);

void EnableExtLvdsTxChip(BYTE fOn);

//void InitCVBS(void);
//void InitWithNTSC(void);
//void EnOutput2DCDC2LEDC(BYTE on);
//void InitTconScaler(void);
//void InitWithNTSC(void);
void Init8836AsDefault(BYTE _InputMain, BYTE _fPowerOn);

#define BT656_8BIT_525I_YCBCR_TO_CVBS		0
#define BT656_8BIT_625I_YCBCR_TO_CVBS		1
#define BT656_8BIT_525P_YCBCR_TO_CVBS		2
#define BT656_8BIT_625P_YCBCR_TO_CVBS		3
#define BT656_8BIT_720P_YCBCR_TO_CVBS		4
#define BT656_8BIT_1080I_YCBCR_TO_CVBS		5
void BT656_InitExtEncoder(BYTE mode);

void InitComponentReg(BYTE mode);

//BYTE CheckVDLossAndSetFreeRun(void);
void TEMP_init_BT656(BYTE test);

#endif
