#ifndef __IMAGECONTROL_H__
#define __IMAGECONTROL_H__


#define IA_HUE_DEFAULT			0x20	//0x280[5:0]
#define IA_CONTRAST_R_DEFAULT	0x80
#define IA_CONTRAST_G_DEFAULT	0x80
#define IA_CONTRAST_B_DEFAULT	0x80
#define IA_CONTRAST_Y_DEFAULT	0x80
#define IA_CONTRAST_CB_DEFAULT	0x80
#define IA_CONTRAST_CR_DEFAULT	0x80
#define IA_BRIGHTNESS_R_DEFAULT	0x80
#define IA_BRIGHTNESS_G_DEFAULT	0x80
#define IA_BRIGHTNESS_B_DEFAULT	0x80
#define IA_BRIGHTNESS_Y_DEFAULT	0x80
#define IA_SHARP_DEFAULT		0x00	//0x208[3:0]


//Decoder Display & Image control

BYTE DecoderGetContrast(void);
BYTE DecoderSetContrast(BYTE val);
BYTE DecoderChangeContrast(void);

BYTE DecoderGetBrightness(void);
BYTE DecoderSetBrightness(BYTE val);
BYTE DecoderChangeBrightness(void);

BYTE DecoderGetSaturation(void);
BYTE DecoderSetSaturation(BYTE val);
BYTE DecoderChangeSaturation(void);

BYTE DecoderGetHUE(void);
BYTE DecoderSetHUE(BYTE val);
BYTE DecoderChangeHUE(void);

BYTE DecoderGetSharpness(void);
BYTE DecoderSetSharpness(BYTE val);
BYTE DecoderChangeSharpness(void);

void DecoderImageReset( void );

//ImageAdjust YCbCr Display & Image control
BYTE ImgAdjGetContrastY(void);
BYTE ImgAdjSetContrastY(BYTE val);
BYTE ImgAdjChangeContrastY(void);

BYTE ImgAdjGetBrightnessY(void);
BYTE ImgAdjSetBrightnessY(BYTE val);
BYTE ImgAdjChangeBrightnessY(void);

BYTE ImgAdjChangeSaturation(void);
BYTE ImgAdjGetSaturation(void);
BYTE ImgAdjSetSaturation(BYTE val);

BYTE ImgAdjChangeHUE(void);
BYTE ImgAdjGetHUE(void);
BYTE ImgAdjSetHUE(BYTE val);
BYTE ImgAdjVaryHUE(BYTE val);

BYTE ImgAdjChangeSharpness(void);
BYTE ImgAdjGetSharpness(void);
BYTE ImgAdjSetSharpness(BYTE val);

void YCbCrImageReset( void );


//ImageAdjust RGB Display & Image control
#if 0
BYTE ImgAdjChangeContrastRGB(void);
BYTE ImgAdjGetContrastRGB(void);
BYTE ImgAdjSetContrastRGB(BYTE val);

BYTE ImgAdjChangeBrightnessRGB(void);
BYTE ImgAdjGetBrightnessRGB(void);
BYTE ImgAdjSetBrightnessRGB(BYTE val);
#else
BYTE ImgAdjChangeContrastRGB(BYTE index);
BYTE ImgAdjGetContrastRGB(BYTE index);
BYTE ImgAdjSetContrastRGB(BYTE index, BYTE val);

BYTE ImgAdjChangeBrightnessRGB(BYTE index);
BYTE ImgAdjGetBrightnessRGB(BYTE index);
BYTE ImgAdjSetBrightnessRGB(BYTE index, BYTE val);
#endif
BYTE ImgAdjDither(BYTE value);

void ResetRGBSharp( void );

void RGBImageReset( void );

void RGBAutoAdj( void );

void RestoreRGBPosBase(void);
BYTE ChangeRGBPosH(void);
BYTE GetRGBPosH(void);
BYTE SetRGBPosH(BYTE val);

BYTE ChangeRGBPosV(void);
BYTE GetRGBPosV(void);
BYTE SetRGBPosV(BYTE val);

//for setup menu
BYTE GetaRGB_Phase(void);
BYTE VaryaRGB_Phase(BYTE val);
BYTE SetaRGB_Phase(BYTE val);
BYTE ChangeaRGB_Phase(void);
BYTE GetaRGB_Clock(void);
BYTE SetaRGB_Clock(BYTE val);
BYTE ChangeaRGB_Clock(void);

BYTE GetHdmiPhase(void);
BYTE SetHdmiPhase(BYTE val);
BYTE ChangeHdmiPhase(void);


void InitRGBPosition(void);
void ReadyRGBPosition(void);
void RestoreRGBPosition(void);
void ResetRGBPosition(void);


BYTE GetBackLight(void);
BYTE SetBackLight(BYTE value);
BYTE ChangeBackLight(BYTE value);


//BT656 Display & Image control
BYTE ChangeTW9900Contrast(void);
WORD GetTW9900Contrast(void);
WORD SetTW9900Contrast(WORD val);

BYTE ChangeTW9900Bright(void);
WORD GetTW9900Bright(void);
WORD SetTW9900Bright(WORD val);

BYTE ChangeTW9900Saturation(void);
WORD GetTW9900Saturation(void);
WORD SetTW9900Saturation(WORD val);

BYTE ChangeTW9900HUE(void);
WORD GetTW9900HUE(void);
WORD SetTW9900HUE(WORD val);

BYTE ChangeTW9900Sharp(void);
WORD GetTW9900Sharp(void);
WORD SetTW9900Sharp(WORD val);

void TW9900ImageReset( void );


void SetImage(BYTE type);

BYTE ChangeDviContrast(void);
WORD GetDviContrast(void);
WORD SetDviContrast(WORD val);

BYTE ChangeDviBright(void);
WORD GetDviBright(void);
WORD SetDviBright(WORD val);

BYTE ChangeDviHue(void);
WORD GetDviHue(void);
WORD SetDviHue(WORD val);

BYTE ChangeDviSharp(void);
WORD GetDviSharp(void);
WORD SetDviSharp(WORD val);

BYTE SetAspectHW(BYTE mode);

BYTE OsdGetTime(void);
BYTE OsdVaryTime(BYTE dat);
BYTE OsdSetTime(BYTE dat);
BYTE OsdChangeTime(void);
BYTE OsdGetTransparent(void);
BYTE OsdGetTrans(void);
BYTE OsdSetTransparent(BYTE dat);
BYTE OsdChangeTransparent(void);
BYTE BackLightGetRate(void);
BYTE BackLightGet(void);
BYTE BackLightSetRate(BYTE dat);
#ifdef SUPPORT_UART1
BYTE BackLightUp(void);
BYTE BackLightDown(void);
#endif
BYTE BackLightChangeRate(void);

//#define PCEEPROM_KEEP_BASE	0
//#define PCEEPROM_UPDATE_BASE	1
void PcEepromSetEffect(BYTE fClock, BYTE fPhase, BYTE fPosition);
void PcEepromUseHwValue(BYTE fClock, BYTE fPhase, BYTE fPosition);

#ifdef SUPPORT_GAMMA
extern CODE BYTE GammaRed[];
extern CODE BYTE GammaBlue[];
extern CODE BYTE GammaGreen[];
#endif
#endif