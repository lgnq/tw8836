/*
 *  config.h - System Configurations 
 *
 *  Copyright (C) 2011 Intersil Corporation
 *
 */
/*****************************************************************************/
/*																			 */
/*                           System Configurations                           */
/*																			 */
/*****************************************************************************/		   
#ifndef __CONFIG_H__
#define __CONFIG_H__

//#define BK_TEST_130117
//-----------------------------------------------------------------------------
//project definitions
//-----------------------------------------------------------------------------
//	CHIP_MANUAL_TEST
//	EVB_31
//	EVB_30
//	EVB_21
//	EVB_20
//	EVB_10
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// MCU
//-----------------------------------------------------------------------------
// DS80320 - TW8835, TW8830
// DP80390 - TW8836

//-----------------------------------------------------------------------------
// MODEL
//-----------------------------------------------------------------------------
//#undef MODEL_TW8830
//#define MODEL_TW8835
#define MODEL_TW8836

//-----------------------------------------------------------------------------
//		Firmware Version
//-----------------------------------------------------------------------------
//	#define	FWVER			0x001		//REV 0.01 120214
//	#define	FWVER			0x002		//REV 0.02 121218 add LVDS EEPROM ImageAdjust data.
	#define	FWVER			0x003		//REV 0.03 121226 use 4 blocks.

#define	REG_START_ADDRESS	0xc000	// register start 0xc000, for page0, 0xc100-page1 


//-----------------------------------------------------------------------------
// Hardware
//-----------------------------------------------------------------------------
#define USE_SFLASH_EEPROM		//FEE(FlashEepromEmulator)
#undef  NO_EEPROM				
#define SUPPORT_SPIOSD

#define SUPPORT_UDFONT			//RAM Font. Only for TEST
#undef SUPPORT_UART1			//cannot share with BT656 output(ExtCVBS)

#define SUPPORT_TOUCH
#undef  SUPPORT_DIPSW
#define SUPPORT_KEYPAD

#undef  SUPPORT_WATCHDOG

#define SUPPORT_SPIFLASH_4BYTES_ADDRESS

//-----------------------------------------------------------------------------
// Software	MODELs
//
//	MODEL_TW8835_EXTI2C
//		Host/Slave model.
//		The internal MCU on the Slave board will be turn OFF.
//		All control for the slave board will be handle by the Host board.
//		If we use it, the Host board will initialize itself 
//		and then initialize the slave board by I2C.
//		The FW for the slave board will be saved at the Host board.
//		The SPIOSD image and the FontOSD font set will be placed at the slave board.
//		The host board will control the IR and EEPROM.
//		The slave board will control the Panel, Touch, Keypad, and the video inputs.
//
//		To support the Video Interrupt Service, it need a following connection.
//		Master:INT10(PORT1.3)   <|==|>  Slave:TEST_GPO
//
//	MODEL_TW8835_MASTER
//		Master/Slave model.
//		The internal MCU on the Slave board will be turn ON.
//		The Master CPU and TW8835 Slave are connected as I2C with one more GPIO pin.
//		This GPIO pin will be connecte to INT10 interrupt on the TW8835 Slave board.
//		(Master:PORT1.7  <|==|> Slave:INT10(PORT1.3))
//		To support it, the Slave board need SUPPORT_I2CCMD_SLAVE_V1
//
//		The Master will not control the video input, panel, touch,eeprom on the master board.
//		
//		This mode can co-work with SUPPORT_EXTMCU_ISP 
//
//	MODEL_TW8835_SLAVE
//		Master/Slave model.
//		Co-Work with MODEL_TW8835_MASTER.
//		Use INT10.
//		It can use SUPPORT_EXTMCU_ISP.    
//
//  Normal TW8835 model
//		undefine MODEL_TW8835_EXTI2C,MODEL_TW8835_MASTER,MODEL_TW8835_SLAVE.
//
//-----------------------------------------------------------------------------
//#undef MODEL_TW8835_EXTI2C  		//move to project
//#undef MODEL_TW8835_MASTER		//move to project
#define MODEL_TW8835_SLAVE

//-----------------------------------------------------------------------------
// Software
//
//	SUPPORT_RCD
//		Support Rear Camera Display (or back camera display) mode.
//		Use PORT 1.6.(GPIO60/PWM2/INT13/P1.6/VD0)
//
//	USE_FRONT_IMAGECTRL
//		TW8835 has two video image control module if input is a CVBS.
//		One is on the decoder and the ohter is on the Backend.
//		If USE_FRONT_IMAGECTRL, TW8835 FW uses the decoder image control module.
//
//	NOSIGNAL_LOGO
//		If TW8835 losses the video signal, it can draw the SPIOSD logo screen.
//
//	SUPPORT_FOSD_MENU
//		If customer wants to use a FontOSD menu, define SUPPORT_FOSD_MENU.
//		It is not perfect, but it shows how to implement the FontOSD based menu.
//
//	SUPPORT_8BIT_CHIP_ACCESS
//		TW8835 uses 16bit internal chip access mode as a default.
//		If FW uses MODEL_TW8835_EXTI2C, FW needs a SUPPORT_8BIT_CHIP_ACCESS.
//
//	SUPPORT_EXTMCU_ISP
//		download FW(binary) from host/master to slave SPIFLASH by I2C
//		need MODEL_TW8835_MASTER & MODEL_TW8835_SLAVE	
//
//
//	SUPPORT_I2CCMD_MASTER_V1
//	SUPPORT_I2CCMD_SLAVE_V1
//		supports I2CCMD Version 1.
//		If uses MODEL_TW8835_MASTER, define SUPPORT_I2CCMD_MASTER_V1.
//		If uses MODEL_TW8835_SLAVE, define SUPPORT_I2CCMD_SLAVE_V1.
//-----------------------------------------------------------------------------
#undef SUPPORT_RCD			//RearCameraDisplay. Conflict with BT656 Tx

#define SUPPORT_I2CI16		//for EP9553E.


//#define NOSIGNAL_LOGO

//uncomment the SUPPORT_FOSD_MENU to start the OSD menu. eamon.fang 
#define SUPPORT_FOSD_MENU	//FontOSD MENU

//-----------------------------------------------------------------------------
// I2CCMD PROTOCOL
//-----------------------------------------------------------------------------
//#ifdef MODEL_TW8835_SLAVE
//	#define SUPPORT_I2CCMD_SLAVE_V1
//#endif

//-----------------------------------------------------------------------------
// model dependency
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
//Panel TYPE
//		#define PANEL_TCON
//		//#define PANEL_SRGB
//		//#define PANEL_FP_LSB
//
//-----------------------------------------------------------------------------
//Panel Resolution
//		#define PANEL_WVGA
//		//#define PANEL_WXGA	//TW8836 
//		
//		#ifdef PANEL_WVGA
//		#define	PANEL_H		800
//		#define	PANEL_V		480
//		#endif
//		#ifdef PANEL_WXGA
//		#define	PANEL_H		1366
//		#define	PANEL_V		768
//		#endif
//  
//Panel PixelClock
//		#define PANEL_PCLK_MIN	30
//		#define PANEL_PCLK_MAX	45
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//		Panel Vendor Specific
//-----------------------------------------------------------------------------
//#define PANEL_INNOLUX_AT080TN03  <== default
//#define PANEL_INNILUX_AT070TN84  <== default
//#define PANEL_CHIMEI_CN_0HF9D2
//#define SUPPORT_E1H_PANEL

	//default
#if 1
	//defined(PANEL_INNOLUX_AT080TN03) || defined(PANEL_INNILUX_AT070TN84)
	#define PANEL_TCON
	#undef  PANEL_SRGB
	#undef	PANEL_FP_LSB

	#define PANEL_H			800
	#define PANEL_V			480
	#define PANEL_PCLK_MIN	30
	#define PANEL_PCLK_MAX	45
#endif

#if 0
#define PANEL_AUO_B133EW01
	#define PANEL_LVDS
	#define PANEL_H			1280
	#define PANEL_V			800
	#define PANEL_PCLK_MIN	50
	#define PANEL_PCLK_TYP	75		//68.9   
	#define PANEL_PCLK_MAX	80
#endif

//-----------------------------------------------------------------------------
//		IR Remote Controller Type
//-----------------------------------------------------------------------------
#define REMO_RC5					// RC5 style
#define TECHWELL_REMOCON
//#define REMO_NEC					// NEC style
//#define PHILIPS_REMOCON 			// New remocon 


//-----------------------------------------------------------------------------
//		I2C Device Access Address Mapping
//-----------------------------------------------------------------------------

#define TW88I2CAddress		0x8A	// TW88xx
#define ADDRESS_EEPROM		0xA0	// 24C16 2Kx8bit
//BKFYI. See I2C.h also.


//-----------------------------------------------------------------------------
//		Options for Possible Inputs
//-----------------------------------------------------------------------------

#ifdef MODEL_TW8836
	#define SUPPORT_CVBS
	#define SUPPORT_SVIDEO
	#define SUPPORT_COMPONENT			// support component analog
	#define SUPPORT_PC  				// support PC
	#define SUPPORT_HDMI
	#ifdef SUPPORT_HDMI
		#undef SUPPORT_HDMI_SiIRX
		#undef SUPPORT_HDMI_EP9351
		#undef SUPPORT_HDMI_EP9553
		#define SUPPORT_HDMI_EP907M
		#undef SUPPORT_HDMI_24BIT		/* conflict with BT656 Tx */
		#ifdef SUPPORT_HDMI_EP9351
			#undef ON_CHIP_EDID_ENABLE
			#undef ON_CHIP_HDCP_ENABLE
		#endif
	#endif
	#define SUPPORT_DTV656		//DTV i656 input. Not related with BT656 module.
	#define SUPPORT_BT656
	#define SUPPORT_LVDSRX
	#define SUPPORT_LVDSTX

	#if defined(SUPPORT_CVBS) && defined(SUPPORT_BT656) && defined(SUPPORT_DTV656)
		#define SUPPORT_BT656_LOOP		//CVBS=>BT656=>DVI(i656)=>Scaler=>Panel.
	#endif
#endif


//-----------------------------------------------------------------------------
//		Options for Possible Standards
//		Default:NTSC
//-----------------------------------------------------------------------------
#define SUPPORT_PAL			
#define SUPPORT_SECAM
#define SUPPORT_NTSC4		
#define SUPPORT_PALM	
#define SUPPORT_PALN		
#define SUPPORT_PAL60		

//-----------------------------------------------------------------------------
//		Options for Debugging/Release
//-----------------------------------------------------------------------------
#define DEBUG						// include debug information
#ifdef DEBUG
	//#define DEBUG_MAIN
	//#define DEBUG_TIME
	#undef DEBUG_ISR
	#undef DEBUG_TW88
	#undef DEBUG_UART
	//#define DEBUG_DECODER		  DEBUG_DEC
	#undef DEBUG_I2C
	#undef DEBUG_SPI
	//#define DEBUG_EEP
	//#define DEBUG_SFLASH_EEPROM
	#undef DEBUG_OSD
		#undef DEBUG_FOSD
		#undef DEBUG_SOSD
	//#define DEBUG_AUDIO
	//#define DEBUG_SETPANEL
	#define DEBUG_DTV		//DVI,HDMI
	//#define DEBUG_PC
	//#define DEBUG_BANK
	//#define DEBUG_PAUSE
	#undef DEBUG_MENU
	#undef DEBUG_KEYREMO
	#undef DEBUG_TOUCH_HW
	#undef DEBUG_TOUCH_SW
	#undef DEBUG_REMO
	#undef DEBUG_REMO_NEC
	#ifdef SUPPORT_WATCHDOG
		//#define DEBUG_WATCHDOG
	#endif
#endif
//#define TW8836_CHIPDEBUG

//-----------------------------------------------------------------------------
//		Special Features
//-----------------------------------------------------------------------------
//#define SUPPORT_USERCOLORFROMTXT	// Use color setting of text file in Color mode=USER MODE which is as Brightness(Reg10), Contast(Reg11),
									//    Sat_U(Reg13),Sat_V(Reg14),Sharpness(Reg12,Reg78)									
//#define SUPPORT_GAMMA

//#define CHIP_MANUAL_TEST			// DIPSW#1
//#define NO_INITIALIZE				// After Power switch turn on with DIP SW4 Enable(Low), or pressed MENU button for internal MCU
                                    //     micom works to access only I2C bus by serial command.

//#define SUPPORT_DELTA_RGB

//#define SUPPORT_SELECTKEY			// choose the keymap of using select key or left,right key.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//		TEST
//-----------------------------------------------------------------------------
//#define SUPPORT_I2CCMD_TEST

#endif // __CONFIG_H__
