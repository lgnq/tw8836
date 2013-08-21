#ifndef _INPUTCTRL_H_
#define _INPUTCTRL_H_

#define	INPUT_CVBS		0   //composite
#define	INPUT_SVIDEO	1	//Y & C
#define INPUT_COMP		2	//INPUT_YUV. analog. SOG
#define INPUT_PC		3	//INPUT_RGB	 RGB with HSYNC & VSYHC
#define	INPUT_DVI		4	//BT656, BT709	- digital
#define INPUT_HDMIPC	5
#define INPUT_HDMITV	6
#define	INPUT_BT656		7	//DVI-8bit: BT656
#define INPUT_LVDS		8
#define INPUT_TOTAL		9
#define BT656_SRC_START	INPUT_TOTAL

//#define	BT656INPUT_CVBS		0   //composite
//#define	BT656INPUT_SVIDEO	1	//Y & C
//#define BT656INPUT_COMP		2	//INPUT_YUV. analog. SOG
//#define BT656INPUT_PC		3	//INPUT_RGB	 RGB with HSYNC & VSYHC
//#define	BT656INPUT_HDMI		4	//BT656, BT709	- digital
//#define	BT656INPUT_BT656	5	//DVI-8bit: BT656. It can not support on EVB. because it is a loopback.
//#define BT656INPUT_LVDS		6
//#define BT656INPUT_PANEL	7	
//#define BT656INPUT_TOTAL	8	
//#define BT656INPUT_NONE		BT656INPUT_TOTAL	


#define	INPUT_CHANGE_DELAY	300
#define	INPUT_CHANGE_DELAY_BASE10MS	30


//DVI, HDMI  -> digital
//PC         -> Analog RGB
//component  -> Analog YCbCr. Please Do not use YUV.

extern XDATA	BYTE	InputMain;
extern XDATA	BYTE	InputBT656;
extern XDATA	BYTE	InputSubMode;
extern XDATA	BYTE	OldInputMain;

//extern BYTE last_position_h;
//extern BYTE last_position_v;
//extern BYTE temp_position_h;
//extern BYTE temp_position_v;



//--------------------------------
// input module
//--------------------------------
BYTE GetInputMain(void);			//friend
void SetInputMain(BYTE input);		//friend..only update InputMain global variable
void InitInputAsDefault(void);



//scaler input
#define INPUT_PATH_DECODER	0x00
#define INPUT_PATH_VADC		0x01
#define INPUT_PATH_DTV		0x02
#define INPUT_PATH_LVDS		0x03	//LVDS_RX
#define INPUT_PATH_BT656	0x06	//DTV+2ndDTV_CLK

#define INPUT_FORMAT_YCBCR	0
#define INPUT_FORMAT_RGB		1
void InputSetSource(BYTE path, BYTE format);
void InputSetFieldPolarity(BYTE fInv);
void InputSetProgressiveField(fOn);
void InputSetCrop( WORD x, WORD y, WORD w, WORD h );
void InputSetCropStart( WORD x, WORD y);
void InputSetHStart( WORD x);
void InputSetVStart( WORD y);
WORD InputGetHStart(void);
WORD InputGetVStart(void);
WORD InputGetHLen(void);
WORD InputGetVLen(void);
void InputSetPolarity(BYTE V,BYTE H, BYTE F);

void BT656InputFreerunClk(BYTE fFreerun, BYTE fInvClk);
void BT656InputSetFreerun(BYTE fOn);




void PrintfInput(BYTE Input, BYTE debug);
void PrintfBT656Input(BYTE Input, BYTE debug);


void ChangeInput( BYTE mode );
void InputModeNext( void );

BYTE CheckInput( void );

//void SetDefault_Decoder(void);
void VInput_enableOutput(BYTE fRecheck);
void VInput_gotoFreerun(BYTE reason);






#endif
