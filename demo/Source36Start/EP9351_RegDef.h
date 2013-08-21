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

#endif  //.. EP9351REGDEF_H

