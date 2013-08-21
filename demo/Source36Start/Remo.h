#ifndef __REMO_H__
#define __REMO_H__
//
// Remo.h
// Default function for Key Input and Remocon Input
//
//=============================================================================
//		                    Operation Key Mapping
//=============================================================================

#define REQUEST_POWER_OFF	0x04

#ifdef SUPPORT_SELECTKEY

//#define ResetKey()		{ P1 = 0xff;}
//#define ReadKey()		(~P1 & 0xfc);

#define POWERKEY		0x04
#define MENUKEY			0x08
#define UPKEY			0x10
#define DOWNKEY			0x20
#define SELECTKEY		0x40
#define INPUTSOURCEKEY	0x80

#else  //..SUPPORT_LEFTRIGHT

//#define ResetKey()		{ P1 = 0xff; P4 = 0xff;}
//#define ReadKey()		( ((~P4>>2)& 0x01) | (~P1 & 0xfc) );
/*
#define POWERKEY		0x01
#define MENUKEY			0x04
#define UPKEY			0x08
#define DOWNKEY			0x10
#define LEFTKEY			0x20
#define RIGHTKEY		0x40
#define INPUTSOURCEKEY	0x80
*/
#define INPUTSOURCEKEY	0x01
#define POWERKEY		0x04
#define LEFTKEY			0x08
#define DOWNKEY			0x10
#define MENUKEY			0x20
#define UPKEY			0x40
#define RIGHTKEY		0x80

#endif	//..SUPPORT_LEFTRIGHT

//=============================================================================
//                                 RC5 type
//=============================================================================
#ifdef REMO_RC5

#ifdef TECHWELL_REMOCON
#define REMO_CUSTOM			0x00

#define REMO_NUM0			0x00
#define REMO_NUM1			0x01
#define REMO_NUM2			0x02
#define REMO_NUM3			0x03
#define REMO_NUM4			0x04
#define REMO_NUM5			0x05
#define REMO_NUM6			0x06
#define REMO_NUM7			0x07
#define REMO_NUM8			0x08
#define REMO_NUM9			0x09

#define REMO_STANDBY		0x0C	//12
#define REMO_CHNUP			0x20	//32
#define REMO_CHNDN			0x21	//33
#define REMO_VOLUP			0x10	//16
#define REMO_VOLDN			0x11	//17
#define REMO_SELECT			0x15	//21
#define REMO_MENU			0x3F	//63
//#define REMO_TV			0x3F	//63
//#define REMO_VCR			0x3C	//60
//#define REMO_SVIDEO		0x38	//56
#define REMO_MUTE			0x0D	//13
#define REMO_INPUT			0x38	//56

#define REMO_AUTO			0x0A	//10
#define REMO_INFO			0x3A	//58
#define REMO_AUDIO			0x26	//38
#define REMO_ASPECT			0x3E	//62
#define REMO_CC				0x0F	//15
#define REMO_EXIT			0x16

#define REMO_TTXRED			0x37
#define REMO_TTXGREEN		0x36
#define REMO_TTXYELLOW		0x32
#define REMO_TTXCYAN		0x34

#define REMO_PIPON			0x25	//37
#define REMO_SWAP			0x33	//51
#define REMO_PIPINPUT		0x28	//40
#define REMO_STILL			0x24	//36
#define REMO_PIPCHUP		0x27	//39
#define REMO_PIPCHDN		0x13	//19

#ifdef DEBUG_REMO
#define REMO_DEBUG		0x2b
#define REMO_READ		0x29
#define REMO_WRITE		0x2a
#define REMO_RESERVED	0x30
#endif	//..DEBUG_REMO

#else //..TECHWELL_REMOCON 
#define REMO_CUSTOM		0

#define REMO_NUM0		0
#define REMO_NUM1		1
#define REMO_NUM2		2
#define REMO_NUM3		3
#define REMO_NUM4		4
#define REMO_NUM5		5
#define REMO_NUM6		6
#define REMO_NUM7		7
#define REMO_NUM8		8
#define REMO_NUM9		9

#define REMO_STANDBY	12
#define REMO_CHNUP		32
#define REMO_CHNDN		33
#define REMO_VOLUP		16
#define REMO_VOLDN		17
#define REMO_SELECT		18
#define REMO_MENU		48
#define REMO_TV			63
#define REMO_VCR		60
#define REMO_SVIDEO		56
#define REMO_MUTE		13
#define REMO_INPUT		0xff

#define REMO_INFO		0x0a //??
#endif //..TECHWELL_REMOCON

//=============================================================================
//                                 NEC type
//=============================================================================
#elif defined REMO_NEC		//..REMO_RC5
/*
// Techwell Origin
#define REMO_CUSTOM1	0x20
#define REMO_CUSTOM2	0xdf


#define REMO_NUM0		0x08
#define REMO_NUM1		0x88
#define REMO_NUM2		0x48
#define REMO_NUM3		0xc8
#define REMO_NUM4		0x28
#define REMO_NUM5		0xa8
#define REMO_NUM6		0x68
#define REMO_NUM7		0xe8
#define REMO_NUM8		0x18
#define REMO_NUM9		0x98

#define REMO_STANDBY	0x10
#define REMO_CHNUP		0x00
#define REMO_CHNDN		0x80
#define REMO_VOLUP		0x40
#define REMO_VOLDN		0xc0
#define REMO_SELECT		0x70
#define REMO_MENU		0x9c
#define REMO_MUTE		0x90
#define REMO_INPUT		0xd0
*/

// Techwell New Remocon after 4/7/2005
#ifdef PHILIPS_REMOCON
#define REMO_CUSTOM1	0x02
#define REMO_CUSTOM2	0xfd

#define REMO_NUM0		0x00
#define REMO_NUM1		0x80
#define REMO_NUM2		0x40
#define REMO_NUM3		0xc0
#define REMO_NUM4		0x20
#define REMO_NUM5		0xa0
#define REMO_NUM6		0x60
#define REMO_NUM7		0xe0
#define REMO_NUM8		0x10
#define REMO_NUM9		0x90

#define REMO_STANDBY	0x48
#define REMO_CHNUP		0xd8
#define REMO_CHNDN		0xf8
#define REMO_VOLUP		0x58
#define REMO_VOLDN		0x78
#define REMO_SELECT		0xe8
#define REMO_MENU		0x01
#define REMO_MUTE		0x08
#define REMO_INPUT		0xf0

#define REMO_INFO		0x38

#define REMO_TTXRED			0x4E
#define REMO_TTXGREEN		0x8E
#define REMO_TTXYELLOW		0xC6
#define REMO_TTXCYAN		0x86

//need debug
#define REMO_PIPON			0x25
#define REMO_SWAP			0x33
#define REMO_PIPINPUT		0x28
#define REMO_STILL			0x24
#define REMO_PIPCHUP		0x27
#define REMO_PIPCHDN		0x13
#define REMO_AUTO			0x0A	//10
#define REMO_EXIT			0x16
#define REMO_ASPECT			0x3E	//62



#else  //..PHILIPS_REMOCON
// RYU For Test
#define REMO_CUSTOM1		0x04
#define REMO_CUSTOM2		0xfb

#define REMO_NUM0		0xf8
#define REMO_NUM1		0x48
#define REMO_NUM2		0xc8
#define REMO_NUM3		0x28
#define REMO_NUM4		0xa8
#define REMO_NUM5		0x18
#define REMO_NUM6		0x98
#define REMO_NUM7		0x58
#define REMO_NUM8		0xd8
#define REMO_NUM9		0x78

#define REMO_STANDBY	0xC0		//0x00
#define REMO_CHNUP		0x40	//
#define REMO_CHNDN		0x70	//
#define REMO_VOLUP		0x90	//
#define REMO_VOLDN		0xe0	//
#define REMO_SELECT		0x11//??
#define REMO_MENU		0x10

#define REMO_MUTE		0x60
#define REMO_INPUT		0xC1  //xc0
#define REMO_AUTO		0x30
#define REMO_AUTOSCAN	0x80
#define REMO_TV			0x20
#define REMO_EXIT		0xa0
#define REMO_INFO		0x08
#define REMO_ASPECT		0x62

#define REMO_TTXRED		0xb8
#define REMO_TTXGREEN	0x02
#define REMO_TTXYELLOW	0x82
#define REMO_TTXCYAN	0x42

#ifdef DEBUG_REMO
#define REMO_DEBUG		0xb8
#define REMO_READ		0x02
#define REMO_WRITE		0x82
#endif

//need debug
#define REMO_PIPON			0x25
#define REMO_SWAP			0x33
#define REMO_PIPINPUT		0xab //??28
#define REMO_STILL			0xaa //??24
#define REMO_PIPCHUP		0x27
#define REMO_PIPCHDN		0x13
//#define REMO_AUTO			0x0A	//10
//#define REMO_EXIT			0x16
//#define REMO_ASPECT			0x3E	//62


#endif //..PHILIPS_REMOCON

#endif //.. REMO_NEC


void InitAUX( BYTE auxn );
BYTE CheckRemo(void);
void CheckKeyIn(void);

//BYTE GetRemo( void );
void PrintRemoCapture(void);
BYTE IsRemoDataReady(BYTE *, BYTE *);
BYTE	DipSW( void );
BYTE GetKey(BYTE repeat);
#define HINPUT_NO		0
#define HINPUT_REMO		1
#define HINPUT_KEY		2
#define HINPUT_TSC		3
#define HINPUT_SERIAL	4
BYTE	CheckHumanInputs( BYTE skip_tsc );

BYTE ActionRemo(BYTE, BYTE);
BYTE ActionTouch(void);

//#ifdef INTERNAL_MCU
BYTE ReadKeyPad( void );
//#endif
#endif
