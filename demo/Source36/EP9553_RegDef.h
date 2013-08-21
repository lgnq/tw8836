/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

 Please review the terms of the license agreement before using this file.
 If you are not an authorized user, please destroy this source code file  
 and notify Explore Semiconductor Inc. immediately that you inadvertently 
 received an unauthorized copy.  

--------------------------------------------------------------------------------

  File        :  EP9553RegDef.h

  Description :  Register Address definitions of EP9553.

\******************************************************************************/

#ifndef EP9x53REGDEF_H
#define EP9x53REGDEF_H

//////////////////////////////////////////////////////////////////////////////////////
// Registers								Word	BitMask 	ByteAddress
//
#define EP9x53_HDMI_HDCP_KEY_AREA			0x00
#define EP9x53_HDMI_HDCP_BKSV				0x28

#define EP9x53_HDMI_INT						0x29
#define EP9x53_HDMI_INT__INT_POL_Active_High		0x80
#define EP9x53_HDMI_INT__AVMC						0x40
#define EP9x53_HDMI_INT__AVMS						0x20
#define EP9x53_HDMI_INT__SEL2						0x10
#define EP9x53_HDMI_INT__SEL1						0x08
#define EP9x53_HDMI_INT__MS							0x04
#define EP9x53_HDMI_INT__ADO						0x02
#define EP9x53_HDMI_INT__AVI						0x01

#define EP9x53_AVI_InfoFrame				0x2A	

#define EP9x53_ADO_InfoFrame				0x2B	

#define EP9x53_MS_InfoFrame					0x2C	

#define EP9x53_Select_Packet_1				0x2D	

#define EP9x53_Select_Packet_2				0x2E

#define EP9x53_M0_Register					0x2F

#define EP9x53_Bcap_Bstatus_Register		0x30
#define EP9x53_Bcap_Register									0x00
#define EP9x53_Bstatus_Register_H								0x01
#define EP9x53_Bstatus_Register_L								0x02

#define EP9x53_V1_Register					0x31

#define EP9x53_KSV0							0x32
#define EP9x53_KSV1							0x33
#define EP9x53_KSV2							0x34
#define EP9x53_KSV3							0x35


#define EP9x53_Timing_Registers				0x3B
#define EP9x53_Timing_Registers__VPW				0x7F
#define EP9x53_Timing_Registers__INTL				0x80

#define EP9x53_Status_Register_0			0x3C
#define EP9x53_Status_Register_0__AVMUTE			0x20
#define EP9x53_Status_Register_0__HDMI				0x10
#define EP9x53_Status_Register_0__AUTH				0x08
#define EP9x53_Status_Register_0__ENC_EN			0x04
#define EP9x53_Status_Register_0__DST_double		0x02
#define EP9x53_Status_Register_0__LAYOUT			0x01

#define EP9x53_Status_Register_1			0x3D
#define EP9x53_Status_Register_1__LINK_ON			0x80
#define EP9x53_Status_Register_1__DE_VALID			0x40
#define EP9x53_Status_Register_1__A_UF				0x10
#define EP9x53_Status_Register_1__A_OF				0x08
#define EP9x53_Status_Register_1__CD				0x07

#define EP9x53_SPDIF_Channel_Status			0x3E

#define EP9x53_EE_Checksum					0x3F

#define EP9x53_General_Control_0			0x40	
#define EP9x53_General_Control_0__DDC_DIS			0x40
#define EP9x53_General_Control_0__PWR_DWN			0x04
#define EP9x53_General_Control_0__ON_CHIP_EDID		0x02
#define EP9x53_General_Control_0__MHL_TEST			0x01 // EP9553

#define EP9x53_General_Control_1			0x41	

#define EP9x53_General_Control_2			0x42	

#define EP9x53_General_Control_3			0x43	

#define EP9x53_General_Control_4			0x44	
#define EP9x53_General_Control_4__SFreq_R			0x07

#define EP9x53_General_Control_5			0x45

#define EP9x53_General_Control_6			0x46	

/*
#define EP9x53_General_Control_7			0x47
*/

#define EP9x53_General_Control_8			0x48
#define EP9x53_General_Control_8__CTS_ADJ_MODE		0xC0
#define EP9x53_General_Control_8__PP_MODE			0x20
#define EP9x53_General_Control_8__MHL_MODE			0x10
#define EP9x53_General_Control_8__PP_MODE			0x20
#define EP9x53_General_Control_8__CBUS_HPD			0x08
#define EP9x53_General_Control_8__EE_DIS			0x04
#define EP9x53_General_Control_8__PORT_SEL			0x03

#define EP9x53_RX_PHY_Control_Register		0x4C


//////////////////////////////////////////////////////////////////////////////////////
// MHL MSC   								Word	BitMask		ByteAddress
//

#define EP9x53_CBUS_MSC_Dec_Capability		0xA0 // There are 16 bytes in this address
#define EP9x53_CBUS_MSC_Dec_Interrupt		0xA1 // There are  4 bytes in this address
#define EP9x53_CBUS_MSC_Dec_Status			0xA2 // There are  4 bytes in this address
#define EP9x53_CBUS_MSC_Dec_SrcPad			0xA3 // There are 16 bytes in this address
#define EP9x53_CBUS_MSC_RAP_RCP				0xA4 // There are  2 bytes in this address

#define EP9x53_CBUS_MSC_Interrupt			0xA5
#define EP9x53_CBUS_MSC_Interrupt__WB_SPT			0x40

#define EP9x53_CBUS_MSC_Interrupt__MSG_IE			0x04
#define EP9x53_CBUS_MSC_Interrupt__SCR_IE			0x02
#define EP9x53_CBUS_MSC_Interrupt__INT_IE			0x01

#define EP9x53_CBUS_MSC_Interrupt__MSG_F			0x04
#define EP9x53_CBUS_MSC_Interrupt__SCR_F			0x02
#define EP9x53_CBUS_MSC_Interrupt__INT_F			0x01

#define EP9x53_CBUS_RQ_Control				0xA6
#define EP9x53_CBUS_RQ_Control__RQ_DONE				0x80
#define EP9x53_CBUS_RQ_Control__RQ_ERR				0x40
#define EP9x53_CBUS_RQ_Control__CBUS_DSCed			0x20
#define EP9x53_CBUS_RQ_Control__RQ_AUTO_EN			0x08
#define EP9x53_CBUS_RQ_Control__CBUS_TRI			0x04
#define EP9x53_CBUS_RQ_Control__RQ_ABORT			0x02
#define EP9x53_CBUS_RQ_Control__RQ_START			0x01

#define EP9x53_CBUS_RQ_SIZE					0xA7
#define EP9x53_CBUS_RQ_SIZE__RX_SIZE				0x60
#define EP9x53_CBUS_RQ_SIZE__TX_SIZE				0x1F
//#define EP9x53_CBUS_RQ_HEADER				0xA7
#define EP9x53_CBUS_RQ_HEADER__DDC_Packet			0x00
#define EP9x53_CBUS_RQ_HEADER__VS_Packet			0x02
#define EP9x53_CBUS_RQ_HEADER__MSC_Packet			0x04
#define EP9x53_CBUS_RQ_HEADER__isCommand			0x01
//#define EP9x53_CBUS_RQ_CMD				0xA7
//#define EP9x53_CBUS_RQ_TD					0xA7

#define EP9x53_CBUS_RQ_ACT_RX_SIZE			0xA8
//#define EP9x53_CBUS_RQ_RD					0xA8

#define EP9x53_CBUS_Vendor_ID				0xA9

#define EP9x53_CBUS_BR_ADJ					0xAA

#define EP9x53_CBUS_TX_Re_Try				0xAB

#define EP9x53_CBUS_Time_Out				0xAC


//////////////////////////////////////////////////////////////////////////////////////
// Others   								Word	BitMask		ByteAddress
//
#define EP9x53_EDID_Data_Register			0xFF

#endif