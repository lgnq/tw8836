#ifndef __VIDEO_ADC__
#define	__VIDEO_ADC__

#if !defined(SUPPORT_COMPONENT) && !defined(SUPPORT_PC)
//----------------------------
//Trick for Bank Code Segment
//----------------------------
void Dummy_ARGB_func(void);
#endif




extern XDATA	BYTE	Input_aRGBMode;

//void VAdcCheckReg(BYTE reg);

BYTE GetInput_aRGBMode(void);

void SetExtVAdcI2C(BYTE addr, BYTE mode);
//void SetExtVideoAdcI2C(BYTE addr, BYTE type, BYTE mode);

BYTE aRGB_SetVcoRange(DWORD _IPF);

void aRGB_LLPLLSetDivider(WORD value, BYTE fInit);
WORD aRGB_LLPLLGetDivider(void);

void aRGB_SetLLPLLControl(BYTE value);

void aRGB_SetClampModeHSyncEdge(BYTE fOn);
void aRGB_SetClampPosition(BYTE value);


void aRGB_SetPhase(BYTE value, BYTE fInit); 	//WithInit
BYTE aRGB_GetPhase(void);
void aRGB_SetFilterBandwidth(BYTE value, WORD delay);

void AutoColorAdjust(void);

BYTE aRGB_CheckInput(BYTE type);
BYTE aRGB_DoubleCheckInput(BYTE detected);
void aRGB_SetPowerDown(void);

void aRGB_SetChannelGainReg(WORD GainG,WORD GainB,WORD GainR);
WORD aRGB_ReadGChannelGainReg(void);
WORD aRGB_ReadBChannelGainReg(void);
WORD aRGB_ReadRChannelGainReg(void);

void aRGB_SetDefaultFor(void);
//void aRGB_SetDefaultForYUV(void);
//void aRGB_SetDefaultForRGB(void);
//BYTE aRGB_SetForRGB( void );
void aRGB_SetPolarity(BYTE fUseCAPAS);
BYTE aRGB_GetInputStatus(void);
//void aRGB_AdjustByMode(BYTE mode);
void aRGB_AdjustPhase(BYTE mode);
//void aRGB_LoopFilter(BYTE value);
void aRGB_setSignalPath(BYTE fInputPC);

//void SetADCMode(BYTE mode);

BYTE WaitStableLLPLL(WORD delay);

//void aRGB_UpdateLoopFilter(WORD delay, BYTE value);
BYTE aRGB_LLPLLUpdateDivider(WORD divider, /*BYTE ctrl,*/ BYTE fInit, BYTE delay);

//BYTE FindInputModePC(WORD *vt);
//BYTE FindInputModeCOMP( void );


//void PCSetInputCrop( BYTE mode );
//void PCSetOutput( BYTE mode );

//void YUVSetOutput(BYTE mode);

//BYTE PCCheckMode(void);
void AdjustPixelClk(WORD divider, BYTE mode );

//BYTE CheckAndSetYPBPR( void );
BYTE CheckAndSetPC(void);
BYTE CheckAndSetComponent( void );


//=============================================================================
//setup menu interface
//=============================================================================
extern void PCRestoreH(void);
extern void PCRestoreV(void);
extern void PCResetCurrEEPROMMode(void);

BYTE ChangeCOMPONENT( void );
BYTE ChangePC( void );

#endif //__VIDEO_ADC__
