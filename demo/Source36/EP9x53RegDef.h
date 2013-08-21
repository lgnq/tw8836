/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

 Please review the terms of the license agreement before using this file.
 If you are not an authorized user, please destroy this source code file  
 and notify Explore Semiconductor Inc. immediately that you inadvertently 
 received an unauthorized copy.  

--------------------------------------------------------------------------------

  File        :  EP9351RegDef.h

  Description :  Register Address definitions of EP9351.

\******************************************************************************/

#ifndef EP9351REGDEF_H
#define EP9351REGDEF_H

// Registers								Word	BitMask		ByteAddress
#define EP9351_HDMI_HDCP_KEY_AREA			0x00
#define EP9351_HDMI_HDCP_BKSV				0x28
#define EP9351_HDMI_INT						0x29
#define EP9351_HDMI_INT__INT_POL_Active_High		0x80
#define EP9351_HDMI_INT__AVMC						0x40
#define EP9351_HDMI_INT__AVMS						0x20
#define EP9351_HDMI_INT__SEL2						0x10
#define EP9351_HDMI_INT__SEL1						0x08
#define EP9351_HDMI_INT__MS							0x04
#define EP9351_HDMI_INT__ADO						0x02
#define EP9351_HDMI_INT__AVI						0x01
#define EP9351_AVI_InfoFrame				0x2A	
#define EP9351_ADO_InfoFrame				0x2B	
#define EP9351_MS_InfoFrame					0x2C	
#define EP9351_Select_Packet_1				0x2D	
#define EP9351_Select_Packet_2				0x2E
#define EP9351_M0_Register					0x2F
#define EP9351_BSTATUS                      0x30
#define EP9351_Bcap_Register								0x00
#define EP9351_Bstatus_Register_H							0x01
#define EP9351_Bstatus_Register_L							0x02
#define EP9351_V1_Register					0x31
#define EP9351_KSV0							0x32
#define EP9351_KSV1							0x33
#define EP9351_KSV2							0x34
#define EP9351_KSV3							0x35
#define EP9351_KSV4							0x36
//#define EP9351_KSV5							0x37
//#define EP9351_KSV6							0x38
//#define EP9351_KSV7							0x39

#define EP9351_Timing_Registers				0x3B
#define EP9351_Timing_Registers__								0x0C
#define EP9351_Timing_Registers__VPW				0x7F
#define EP9351_Timing_Registers__INTL				0x80

#define EP9351_Status_Register_0			0x3C
//#define EP9351_Status_Register_0__PF				0xC0
#define EP9351_Status_Register_0__AVMUTE			0x20
#define EP9351_Status_Register_0__HDMI				0x10
#define EP9351_Status_Register_0__AUTH				0x08
#define EP9351_Status_Register_0__ENC_EN			0x04
#define EP9351_Status_Register_0__DST_double		0x02
#define EP9351_Status_Register_0__LAYOUT			0x01

#define EP9351_Status_Register_1			0x3D
#define EP9351_Status_Register_1__LINK_ON			0x80
#define EP9351_Status_Register_1__DE_VALID			0x40
//#define EP9351_Status_Register_1__TX_PWR			0x20
#define EP9351_Status_Register_1__A_UF				0x10
#define EP9351_Status_Register_1__A_OF				0x08
//#define EP9351_Status_Register_1__CD				0x07

#define EP9351_SPDIF_Channel_Status			0x3E
#define EP9351_EE_Checksum					0x3F

#define EP9351_General_Control_0			0x40	
#define EP9351_General_Control_0__DDC_DIS			0x40
#define EP9351_General_Control_0__PWR_DWN			0x04
//#define EP9351_General_Control_0__CH				0x03
#define EP9351_General_Control_0__ON_CHIP_EDID		0x02

#define EP9351_General_Control_1			0x41	

#define EP9351_General_Control_2			0x42	

#define EP9351_General_Control_3			0x43	

#define EP9351_General_Control_4			0x44	
#define EP9351_General_Control_4__SFreq_R			0x07

#define EP9351_General_Control_5			0x45

#define EP9351_General_Control_6			0x46	

//#define EP9351_General_Control_7			0x47

#define EP9351_General_Control_8			0x48
//#define EP9351_General_Control_8__DAC_EN			0x80
//#define EP9351_General_Control_8__SOG_EN			0x40
#define EP9351_General_Control_8__EE_DIS			0x04

#define EP9351_General_Control_9			0x49

#define EP9351_EDID_Data_Register			0xFF


//**********************************************//
// Registers									Address		BitMask		Value
#define EP907M_Power_Status						0x0000
#define EP907M_Power_Status__HDMI_PWR						0x01

#define EP907M_Interrupt_Flags					0x0100
#define EP907M_Interrupt_Flags__TIM_CH						0x80
#define EP907M_Interrupt_Flags__AVMC						0x40
#define EP907M_Interrupt_Flags__AVMS						0x20
#define EP907M_Interrupt_Flags__ACP_CH						0x10
#define EP907M_Interrupt_Flags__SEL1_CH						0x08
#define EP907M_Interrupt_Flags__ADO_CH						0x02
#define EP907M_Interrupt_Flags__AVI_CH						0x01

#define EP907M_Interrupt_Flags_2				0x0101
#define EP907M_Interrupt_Flags_2__TMDS_CH					0x80
#define EP907M_Interrupt_Flags_2__MCLK_NORMAL				0x40
#define EP907M_Interrupt_Flags_2__LINK_OFF					0x04
#define EP907M_Interrupt_Flags_2__LINK_ON					0x02
#define EP907M_Interrupt_Flags_2__AVI_LOST					0x01

#define EP907M_System_Status_0					0x0200
#define EP907M_System_Status_0__AVMUTE_F					0x20
#define EP907M_System_Status_0__HDMI						0x10
#define EP907M_System_Status_0__AUTHEN						0x08
#define EP907M_System_Status_0__DECRYPT						0x04
#define EP907M_System_Status_0__DST_double					0x02
#define EP907M_System_Status_0__LAYOUT						0x01

#define EP907M_System_Status_1					0x0201
#define EP907M_System_Status_1__LINK_ON						0x80
#define EP907M_System_Status_1__DE_VALID					0x40
#define EP907M_System_Status_1__PWR5V_P2					0x20
#define EP907M_System_Status_1__PWR5V_P1					0x10
#define EP907M_System_Status_1__PWR5V_P0					0x08
#define EP907M_System_Status_1__CD							0x07

#define EP907M_System_Status_2					0x0202
#define EP907M_System_Status_2__A_UF						0x10
#define EP907M_System_Status_2__A_OF						0x08

#define EP907M_Video_Status_0					0x0300
#define EP907M_Video_Status_0__VIN_FREQ						0x30
#define EP907M_Video_Status_0__VIN_FMT						0x0F
#define EP907M_Video_Status_0__VIN_FMT_YCC					0x08
#define EP907M_Video_Status_0__VIN_FMT_ITU709				0x04
#define EP907M_Video_Status_0__VIN_FMT_Extended				0x02
#define EP907M_Video_Status_0__VIN_FMT_Full_Range			0x02
#define EP907M_Video_Status_0__VIN_FMT_422					0x01
#define EP907M_Video_Status_0__VIN_FMT_RGB					0x00

#define EP907M_Video_Status_1					0x0301

#define EP907M_Audio_Status_0					0x0400
#define EP907M_Audio_Status_0__DST_AUDIO					0x40
#define EP907M_Audio_Status_0__HBR_AUDIO					0x20
#define EP907M_Audio_Status_0__DSD_AUDIO					0x10
#define EP907M_Audio_Status_0__STD_AUDIO					0x08
#define EP907M_Audio_Status_0__SAMP_FREQ					0x07

#define EP907M_Audio_Status_1					0x0401
#define EP907M_Audio_Status_1__MODE	  						0xC0
#define EP907M_Audio_Status_1__PREEMPHASIS					0x38
#define EP907M_Audio_Status_1__COPYRIGHT					0x04
#define EP907M_Audio_Status_1__NONE_PCM						0x02
#define EP907M_Audio_Status_1__PROFESSION					0x01

#define EP907M_Hsync_Timing 					0x0600

#define EP907M_Vsync_Timing 					0x0700

#define EP907M_AVI_Information					0x0800

#define EP907M_ADO_Information					0x0900

#define EP907M_Core_Version_Major				0x1000
#define EP907M_Core_Version_Minor				0x1001

#define EP907M_Reset_Status						0x1100
#define EP907M_Reset_Status__RST_WDT						0x02
#define EP907M_Reset_Status__RST_PIN						0x01

#define EP907M_Select_Packet_1					0x0B00

#define EP907M_ACP_Packet						0x0C00
#define EP907M_ACP_Type							0x0C01

#define EP907M_Interrupt_Enable					0x2000
#define EP907M_Interrupt_Enable__TIM_CH						0x80
#define EP907M_Interrupt_Enable__AVMC						0x40
#define EP907M_Interrupt_Enable__AVMS						0x20
#define EP907M_Interrupt_Enable__ACP_CH						0x10
#define EP907M_Interrupt_Enable__SEL1_CH					0x08
#define EP907M_Interrupt_Enable__ADO_CH						0x02
#define EP907M_Interrupt_Enable__AVI_CH						0x01

#define EP907M_Polarity_Control					0x2001
#define EP907M_Polarity_Control__MUTE_POL_Low				0x80
#define EP907M_Polarity_Control__HOTPLUG_POL_High			0x40
#define EP907M_Polarity_Control__OCD						0x30
#define EP907M_Polarity_Control__INT_MODE_Level				0x08
#define EP907M_Polarity_Control__INT_POL_High				0x04
#define EP907M_Polarity_Control__VS_POL_Low					0x02
#define EP907M_Polarity_Control__HS_POL_Low					0x01

#define EP907M_Video_Output_Control				0x2002
#define EP907M_Video_Output_Control__F_POL					0x80
#define EP907M_Video_Output_Control__ESYNC					0x40
#define EP907M_Video_Output_Control__ES_PORT				0x20
#define EP907M_Video_Output_Control__UVSW					0x10
#define EP907M_Video_Output_Control__D02SW					0x08
#define EP907M_Video_Output_Control__BIT_REV				0x04
#define EP907M_Video_Output_Control__DCLK_POL				0x02
#define EP907M_Video_Output_Control__DCLK_DDR				0x01

#define EP907M_Audio_Output_Control				0x2003
#define EP907M_Audio_Output_Control__SC_POL					0x80
#define EP907M_Audio_Output_Control__WS_POL					0x40
#define EP907M_Audio_Output_Control__WS_M					0x20
#define EP907M_Audio_Output_Control__IIS_SS					0x10
#define EP907M_Audio_Output_Control__DSD_OPT				0x08
#define EP907M_Audio_Output_Control__AAM_EN					0x04
#define EP907M_Audio_Output_Control__MCLK_SEL				0x03

#define EP907M_Audio_Channel_Mapping			0x2004

#define EP907M_Interrupt_Enable_2				0x2005
#define EP907M_Interrupt_Enable_2__TMDS_CH					0x80
#define EP907M_Interrupt_Enable_2__MCLK_NORMAL				0x40
#define EP907M_Interrupt_Enable_2__AAMS_IEN					0x08
#define EP907M_Interrupt_Enable_2__LINK_OFF					0x04
#define EP907M_Interrupt_Enable_2__LINK_ON					0x02
#define EP907M_Interrupt_Enable_2__AVI_LOST					0x01

#define EP907M_Audio_Mute_Time					0x2006

#define EP907M_ACR_Control						0x2007
#define EP907M_ACR_Control__ACR1							0x01
#define EP907M_ACR_Control__ACR1_CTS_ADJ					0x02
#define EP907M_ACR_Control__ACR_SOFT_CTS_ADJ				0x04

#define EP907M_System_Control					0x2100
#define EP907M_System_Control__DDC_DIS						0x80
#define EP907M_System_Control__LOCK_SLOW					0x40
#define EP907M_System_Control__PORT_SEL						0x30
#define EP907M_System_Control__PORT_SEL__P1								0x00
#define EP907M_System_Control__PORT_SEL__P2								0x10
#define EP907M_System_Control__PORT_SEL__P3								0x20
#define EP907M_System_Control__CEC_EN						0x08
#define EP907M_System_Control__EQ_BOOST						0x04
#define EP907M_System_Control__PD_HDMI						0x02
#define EP907M_System_Control__PD_TOT						0x01

#define EP907M_GPIO_Control						0x2101
#define EP907M_GPIO_Control__AUTO_HOTPLUG_b					0x08
#define EP907M_GPIO_Control__HP2_EN							0x04
#define EP907M_GPIO_Control__HP1_EN							0x02
#define EP907M_GPIO_Control__HP0_EN							0x01

#define EP907M_Mute_Control						0x2200
#define EP907M_Mute_Control__SIMPLAY_M5S					0x08
#define EP907M_Mute_Control__AVMUTE_R						0x04
#define EP907M_Mute_Control__A_MUTE							0x02
#define EP907M_Mute_Control__V_MUTE							0x01

#define EP907M_Output_Format_Control			0x2300
#define EP907M_Output_Format_Control__AUTO_VFMTb			0x40
#define EP907M_Output_Format_Control__A_SOURCE				0x30
#define EP907M_Output_Format_Control__A_SOURCE_STD_ADO					0x00
#define EP907M_Output_Format_Control__A_SOURCE_DSD_ADO					0x10
#define EP907M_Output_Format_Control__A_SOURCE_HBR_ADO					0x20
#define EP907M_Output_Format_Control__A_SOURCE_DST_ADO					0x30
#define EP907M_Output_Format_Control__VOUT_FMT_YCC			0x08
#define EP907M_Output_Format_Control__VOUT_FMT_Full_Range	0x02
#define EP907M_Output_Format_Control__VOUT_FMT_422			0x01
#define EP907M_Output_Format_Control__VOUT_FMT_RGB			0x00

#define EP907M_Output_Disable_Control			0x2400
#define EP907M_Output_Disable_Control__SPDIF_DIS			0x80
#define EP907M_Output_Disable_Control__IIS_DIS				0x40
#define EP907M_Output_Disable_Control__VOUT_DIS				0x10
#define EP907M_Output_Disable_Control__DAC_EN				0x04 // Not avaliable in PJ2
#define EP907M_Output_Disable_Control__SOG_EN				0x02 // Not avaliable in PJ2
#define EP907M_Output_Disable_Control__I2S_PCM				0x01 // Enable I2S only when PCM


#endif
