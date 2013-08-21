/**
 * @file
 * SpiFlashMap.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	SpiFlash Map for images
*/
//*****************************************************************************
//
//								SPI_MAP.c
//
//*****************************************************************************
// SPI FLASH total MAP
// +----------------------------+    
// |                            |
// |     CODE BANK              |
// |                            |
// +----------------------------+    
// | 080000~0x09FFFF            |
// |     EEPROM Emulation       |
// |                            |
// +----------------------------+    
// | 0A0000                     |
// |     Blank Space            |
// +----------------------------+    
// | 100000                     |
// |     Demo Image             |
// |                            |
// | ParkGrid                   |
// |  start 0x100000            |
// | Pigeon                     |
// |  start 0x170000            |
// | Rose                       |
// |  start 0x190D00            |
// | LUT for Pigion&Rose        |
// |  start 0x317700            |
// +----------------------------+    
// | 400000                     |
// |     Default Font           |
// |     max 0x2800             |
// | FontAll.bin                |
// |  start:0x400000 size:0x10F0|
// | consolas16x26_606C90.bin   |
// |  start:0x402000 size:0x1B38|
// |                            |
// | other test fonts           |
// | test16x32xA0_A0_A0_A0      |
// |  start:0x404000 size:0x2800|
// | test16x32xA0_20_20_20      |
// |  start:0x407000 size:0x2800|
// | consolas22_16x26_2BPP      |
// |  start:0x40A000 size:0x2700|
// | otehr                      |
// |  start:0x40D00             |
// +----------------------------+    
// | 410000                     |
// |     MENU IMG               |
// |                            |
// |                            |
// | 7FFFFF                     |
// +----------------------------+    
// | test image                 |
// |                            |
// |                            |
// +----------------------------+    
// 
//
// detail DEMO IMG MAP
// +----------------------------+    
// | PIGEON                     |
// +----------------------------+    
// | ROSE                       |
// +----------------------------+    
// | LUT for PIGEON&ROSE        |
// +----------------------------+    
// | GENESIS & LUT              |
// +----------------------------+    
// | GRID & LUT                 |
// +----------------------------+    
// | MESSAGE & LUT              |
// +----------------------------+    
// | COMPASS & LUT              |
// +----------------------------+    
// | Dynamic Grid & LUT         |
// +----------------------------+    
// | Dynamic Message & LUT      |
// +----------------------------+    

#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "Global.h"
#include "CPU.h"
#include "printf.h"
#include "util.h"
#include "monitor.h"

#include "I2C.h"
#include "InputCtrl.h"

#include "SOsd.h"
#include "FOsd.h"

#include "SpiFlashMap.h"
#include "SOsdMenu.h"

#ifdef SUPPORT_SPIOSD

#define MAP0_START	SFLASH_IMG_ADDR
//#define MAP0_START	0

//===================================
//BKANG test area
//0x300000
//===================================



code image_info_t img_main_test1_header = {1, 0x86, 800, 480 /*,0x01803E*/};
code image_info_t img_main_test2_header = {1, 0x86, 800, 480 /*,0x015307*/};
code image_info_t img_main_test3_header = {1, 0x86, 800, 480 /*,0x0136E8*/};


code image_item_info_t img_main_test1	= {0, MENU_TEST_FLASH_START+0x000000, &img_main_test1_header, 0xff};    
code image_item_info_t img_main_test2	= {0, MENU_TEST_FLASH_START+0x01803E, &img_main_test1_header, 0xff};    
code image_item_info_t img_main_test3	= {0, MENU_TEST_FLASH_START+0x02D345, &img_main_test1_header, 0xff};    





//===============
// DEFAULT FONT
// 0x400000
//===============


code WORD default_LUT_bpp2[4] 		= { 0x0000,0x001F,0xF800,0xFFFF };
code WORD default_LUT_bpp3[8] 		= { 0x0000,0x001F,0x07E0,0x07FF,0xF800,0xF81F,0xFFE0,0xFFFF };
code WORD default_LUT_bpp4[16] 		= { 0x0000,0x0010,0x0400,0x0410,0x8000,0x8010,0x8400,0x8410,
										0xC618,0x001F,0x07E0,0x07FF,0xF800,0xF81F,0xFFE0,0xFFFF	};

//code WORD consolas_LUT_bpp2[4] 	= {	0x0000,0x001F,0xF800,0xFFFF };
//code WORD consolas_LUT_bpp3[8] 	= { 0x0000,0x001F,0x07E0,0x07FF,0xF800,0xF81F,0xFFE0,0xFFFF };
//code WORD consolas_LUT_bpp4[16] 	= { 0x07FF,0x20E3,0xF79E,0x62E8,0xE104,0xA944,0x39A6,0x7BAC,
// 											0x51A6,0xC617,0x9CD1,0xB5B5,0x9BC9,0xDD85,0xF643,0xAC87 };

//code WORD graynum_LUT_bpp2[4] 	= { 0xF7DE,0x0000,0x5AAB,0xC000  };
code WORD graynum_LUT_bpp3[8] 		= { 0xFFFF,0x0000,0xDEDB,0x9492,0x6B6D,0xB5B6,0x4A49,0x2124 };
//code WORD graynum_LUT_bpp4[16] 	= {	0xD6BA,0x20E3,0xF79E,0x62E8,0xE104,0xA944,0x39A6,0x7BAC,
//										0x51A6,0xC617,0x9CD1,0xB5B5,0x9BC9,0xDD85,0xF643,0xAC87};


//TW8835 max FontRam size is 0x2800. But FW use 0x3000 space. The remain 0x800 will be used for Font Information.
//                                              loc,      size    W   H   2BPP   3BPP   4BPP   MAX    palette for 2bpp,3bpp,4bpp
code FONT_SPI_INFO_t default_font 		   	= { 0x400000, 0x27F9, 12, 18, 0x100, 0x120, 0x15F, 0x17B, default_LUT_bpp2, default_LUT_bpp3, default_LUT_bpp4 };
code FONT_SPI_INFO_t consolas16x26_606C90 	= { 0x403000, 0x2080, 16, 26, 0x060, 0x06C, 0x090, 0x0A0, NULL, NULL, NULL };
code FONT_SPI_INFO_t consolas16x26_graynum 	= { 0x406000, 0x0618, 16, 26, 0x000, 0x000, 0x01E, 0x01E, NULL, graynum_LUT_bpp3, NULL };
code FONT_SPI_INFO_t kor_font		 		= { 0x409000, 0x0A20, 12, 18, 0x000, 0x000, 0x000, 0x060, NULL, NULL, NULL };
code FONT_SPI_INFO_t ram_font		 		= { 0x40B000, 0x2080, 16, 18, 0x060, 0x06C, 0x090, 0x0A0, NULL, NULL, NULL };
//next 0x400000+0xB000
//next you have to move the menu images.


//===================================
// for TEST
FAR CONST MY_SLIDEIMAGE test_IMG[] = {
    { MAP0_START+0x0EF71A, 0x0100, 0x002B1D },    // Test_PBARPTR100_64
};
FAR CONST MY_RLE_INFO test_INFO[] = {
	{ 0x60, 327,45  },		//Test_PBARPTR100_64
};


//====================================================
// MENU IMAGE MAP
// 0x410000
//====================================================

//for fast
code image_info_t img_navi_close0_header 	=  {1, 0x70, 0x30, 0x30 };
code image_info_t img_navi_close1_header 	=  {1, 0x70, 0x30, 0x30 };
code image_info_t img_navi_setup0_header 	=  {1, 0x70, 0x30, 0x30 };
code image_info_t img_navi_setup1_header 	=  {1, 0x70, 0x30, 0x30 };



code image_info_t img_main_input_header =  {1, 0x80, 118, 110 /*,0x01803E*/};
code image_info_t img_main_audio_header =  {1, 0x80, 106, 114 /*,0x01803E*/};
code image_info_t img_main_system_header = {1, 0x80, 138, 117 /*,0x01803E*/};
code image_info_t img_wait_header 		 = {1, 0x70,  48, 50 /*,0x01803E*/};

code image_info_t img_input_bg_bottom_header 	= {1, 0x88, 0x320, 0x042 }; //:45F13D:49 54 88 88 20 03 42 00 95 18 00 00 01 FF 60 01 
code image_info_t img_input_nodvi_bg_top_header = {1, 0x80, 0x320, 0x046 }; //:46EFD2:49 54 88 00 20 03 46 00 C0 DA 00 00 01 FF 60 01 
code image_info_t img_input_select_header		= {1, 0x80, 0x009, 0x009 }; //:47CEA2:49 54 88 00 09 00 09 00 51 00 00 00 01 FF 60 01 
code image_info_t img_input_cvbs0_header 	= {1, 0x80, 0x3E, 0x40 };
code image_info_t img_input_cvbs1_header 	= {1, 0x80, 0x3E, 0x40 };
code image_info_t img_input_svideo0_header 	= {1, 0x80, 0x4F, 0x3E };
code image_info_t img_input_svideo1_header 	= {1, 0x80, 0x4F, 0x3E };
code image_info_t img_input_ypbpr0_header 	= {1, 0x80, 0x46, 0x42 };
code image_info_t img_input_ypbpr1_header 	= {1, 0x80, 0x46, 0x42 };
code image_info_t img_input_pc0_header 		= {1, 0x80, 0x47, 0x3F };
code image_info_t img_input_pc1_header 		= {1, 0x80, 0x47, 0x3F };
//code image_info_t img_input_dvi0_header 	= {1, 0x80, 0x4F, 0x3E };
//code image_info_t img_input_dvi1_header 	= {1, 0x80, 0x4F, 0x3E };
code image_info_t img_input_hdmi0_header 	= {1, 0x80, 0x48, 0x3F };
code image_info_t img_input_hdmi1_header 	= {1, 0x80, 0x48, 0x3F };
code image_info_t img_input_ext0_header 	= {1, 0x80, 0x3D, 0x3D };
code image_info_t img_input_ext1_header 	= {1, 0x80, 0x3D, 0x3D };
code image_info_t img_input_return0_header 	= {1, 0x70, 0x22, 0x22 };
code image_info_t img_input_return1_header 	= {1, 0x70, 0x22, 0x22 };







//code image_item_info_t img_ = {+0x000000, 0x0010F0 },    // FontAll 
code image_item_info_t img_logo							= {1, MENU_B_FLASH_START+0x000000, NULL,	0xff};    // Intersil-Techwell
code image_item_info_t img_navi_menu  					= {1, MENU_B_FLASH_START+0x0049C0, NULL,	0x00};    // img_navi_bg 
code image_item_info_t img_navi_return  				= {1, MENU_B_FLASH_START+0x0071E0, NULL,	0xff};    // img_navi_return 
code image_item_info_t img_navi_return1  				= {1, MENU_B_FLASH_START+0x007CF0, NULL,	0xff};    // img_navi_return1 
code image_item_info_t img_navi_home  					= {1, MENU_B_FLASH_START+0x008800, NULL,	0xff};    // img_navi_home 
code image_item_info_t img_navi_home1  					= {1, MENU_B_FLASH_START+0x009310, NULL,	0xff};    // img_navi_home1 
code image_item_info_t img_navi_close  					= {2, MENU_B_FLASH_START+0x009E20, &img_navi_close0_header,	0xff};    // img_navi_close 
code image_item_info_t img_navi_close1  				= {1, MENU_B_FLASH_START+0x00A930, NULL,	0xff};    // img_navi_close1 
code image_item_info_t img_navi_demo  					= {1, MENU_B_FLASH_START+0x00B440, NULL,	0xff};    // img_navi_demo 
code image_item_info_t img_navi_demo1  					= {1, MENU_B_FLASH_START+0x00BF50, NULL,	0xff};    // img_navi_demo1 
code image_item_info_t img_navi_setup  					= {2, MENU_B_FLASH_START+0x00CA60, &img_navi_setup0_header,	0xff};    // img_navi_setup 
code image_item_info_t img_navi_setup1  				= {1, MENU_B_FLASH_START+0x00D570, NULL,	0xff};    // img_navi_setup1 
code image_item_info_t img_main_bg  					= {1, MENU_B_FLASH_START+0x00E080, NULL,	0xff};    // img_main_bg 
code image_item_info_t img_main_input  					= {2, MENU_B_FLASH_START+0x0236C0, &img_main_input_header,	0xff};    // img_main_input 
code image_item_info_t img_main_input1  				= {1, MENU_B_FLASH_START+0x026D90, NULL,	0xff};    // img_main_input1 
code image_item_info_t img_main_audio  					= {2, MENU_B_FLASH_START+0x02A460, &img_main_audio_header,	0xff};    // img_main_audio 
code image_item_info_t img_main_audio1  				= {1, MENU_B_FLASH_START+0x02D7B0, NULL,	0xff};    // img_main_audio1 
code image_item_info_t img_main_system  				= {2, MENU_B_FLASH_START+0x030B00, &img_main_system_header,	0xff};    // img_main_system 
code image_item_info_t img_main_system1  				= {1, MENU_B_FLASH_START+0x034E30, NULL,	0xff};    // img_main_system1 
code image_item_info_t img_main_gps  					= {1, MENU_B_FLASH_START+0x039160, NULL,	0xff};    // img_main_gps 
code image_item_info_t img_main_gps1  					= {1, MENU_B_FLASH_START+0x03C990, NULL,	0xff};    // img_main_gps1 
code image_item_info_t img_main_phone  					= {1, MENU_B_FLASH_START+0x0401C0, NULL,	0xff};    // img_main_phone 
code image_item_info_t img_main_phone1  				= {1, MENU_B_FLASH_START+0x0437F0, NULL,	0xff};    // img_main_phone1 
code image_item_info_t img_main_carinfo  				= {1, MENU_B_FLASH_START+0x046E20, NULL,	0xff};    // img_main_carinfo 
code image_item_info_t img_main_carinfo1  				= {1, MENU_B_FLASH_START+0x04AFF0, NULL,	0xff};    // img_main_carinfo1 
code image_item_info_t img_input_bg_bottom  			= {2, MENU_B_FLASH_START+0x04F1C0, &img_input_bg_bottom_header,	0x5B};    // img_input_bg_bottom 
code image_item_info_t img_input_bg_top  				= {1, MENU_B_FLASH_START+0x050E70, NULL,	0x5B};    // img_input_bg_top 
code image_item_info_t img_input_nodvi_bg_top  			= {2, MENU_B_FLASH_START+0x05F060, &img_input_nodvi_bg_top_header,	0x5B};    // img_input_nodvi_bg_top 
code image_item_info_t img_input_select  				= {2, MENU_B_FLASH_START+0x06CF30, &img_input_select_header,	0x5B};    // img_input_select 
code image_item_info_t img_input_cvbs 		 			= {2, MENU_B_FLASH_START+0x06D3A0, &img_input_cvbs0_header,	0xff};    // img_input_cvbs 
code image_item_info_t img_input_cvbs1 		 			= {1, MENU_B_FLASH_START+0x06E730, NULL,	0xff};    // img_input_cvbs1 
code image_item_info_t img_input_svideo  				= {2, MENU_B_FLASH_START+0x06FB00, &img_input_svideo0_header,	0xff};    // img_input_svideo 
code image_item_info_t img_input_svideo1  				= {1, MENU_B_FLASH_START+0x071240, NULL,	0xff};    // img_input_svideo1 
code image_item_info_t img_input_ypbpr  				= {2, MENU_B_FLASH_START+0x072980, &img_input_ypbpr0_header,	0xff};    // img_input_Ypbpr 
code image_item_info_t img_input_ypbpr1  				= {1, MENU_B_FLASH_START+0x073FA0, NULL,	0xff};    // img_input_Ypbpr1 
code image_item_info_t img_input_pc  					= {2, MENU_B_FLASH_START+0x075580, &img_input_pc0_header,	0xff};    // img_input_pc 
code image_item_info_t img_input_pc1  					= {1, MENU_B_FLASH_START+0x076B10, NULL,	0xff};    // img_input_pc1 
code image_item_info_t img_input_dvi  					= {1, MENU_B_FLASH_START+0x0780D0, NULL,	0xff};    // img_input_dvi 
code image_item_info_t img_input_dvi1  					= {1, MENU_B_FLASH_START+0x079520, NULL,	0xff};    // img_input_dvi1 
code image_item_info_t img_input_hdmi  					= {2, MENU_B_FLASH_START+0x07A970, &img_input_hdmi0_header,	0xff};    // img_input_hdmi 
code image_item_info_t img_input_hdmi1  				= {1, MENU_B_FLASH_START+0x07BF40, NULL,	0xff};    // img_input_hdmi1 
code image_item_info_t img_input_ext  					= {2, MENU_B_FLASH_START+0x07D490, &img_input_ext0_header,	0xff};    // img_input_ext 
code image_item_info_t img_input_ext1  					= {1, MENU_B_FLASH_START+0x07E730, NULL,	0xFF};    // img_input_ext1 
code image_item_info_t img_input_return  				= {2, MENU_B_FLASH_START+0x07F9D0, &img_input_return0_header,	0xff};    // img_input_return 
code image_item_info_t img_input_return1  				= {1, MENU_B_FLASH_START+0x080070, NULL,	0xFF};    // img_input_return1 
code image_item_info_t img_audio_bg			  			= {1, MENU_B_FLASH_START+0x080710, NULL,	0xff};    // img_audio_bg 
code image_item_info_t img_system_bg_bottom  			= {1, MENU_B_FLASH_START+0x090DD0, NULL,	0x61};    // img_system_bg_bottom 
code image_item_info_t img_system_bg_top  				= {1, MENU_B_FLASH_START+0x092890, NULL,	0x61};    // img_system_bg_top 
code image_item_info_t img_system_touch  				= {1, MENU_B_FLASH_START+0x0A0A80, NULL,	0xff};    // img_system_touch 
code image_item_info_t img_system_touch1  				= {1, MENU_B_FLASH_START+0x0A1B10, NULL,	0xff};    // img_system_touch1 
code image_item_info_t img_system_display 				= {1, MENU_B_FLASH_START+0x0A2BA0, NULL,	0xff};    // img_system_display 
code image_item_info_t img_system_display1 				= {1, MENU_B_FLASH_START+0x0A3EC0, NULL,	0xff};    // img_system_display1 
code image_item_info_t img_system_btooth  				= {1, MENU_B_FLASH_START+0x0A51E0, NULL,	0xff};    // img_system_btooth 
code image_item_info_t img_system_btooth1  				= {1, MENU_B_FLASH_START+0x0A63D0, NULL,	0xff};    // img_system_btooth1 
code image_item_info_t img_system_restore  				= {1, MENU_B_FLASH_START+0x0A7580, NULL,	0xff};    // img_system_restore 
code image_item_info_t img_system_restore1  			= {1, MENU_B_FLASH_START+0x0A85D0, NULL,	0xff};    // img_system_restore1 
code image_item_info_t img_system_sys_info  			= {1, MENU_B_FLASH_START+0x0A9620, NULL,	0xff};    // img_system_sys_info 
code image_item_info_t img_system_sys_info1  			= {1, MENU_B_FLASH_START+0x0AA7D0, NULL,	0xff};    // img_system_sys_info1 
code image_item_info_t img_gps_bg			  			= {1, MENU_B_FLASH_START+0x0AB980, NULL,	0xff};    // img-gps-bg 
code image_item_info_t img_phone_bg			  			= {1, MENU_B_FLASH_START+0x0DD030, NULL,	0xff};    // img-phone-bg
code image_item_info_t img_phone_00 					= {1, MENU_B_FLASH_START+0x0E8ED0, NULL,	0xff};    // img_phone_00 	 
code image_item_info_t img_phone_01 					= {1, MENU_B_FLASH_START+0x0E9D80, NULL,	0xff};    // img_phone_01 	 
code image_item_info_t img_phone_10 					= {1, MENU_B_FLASH_START+0x0EAC70, NULL,	0xff};    // img_phone_10 	 
code image_item_info_t img_phone_11 					= {1, MENU_B_FLASH_START+0x0EBAF0, NULL,	0xff};    // img_phone_11 	 
code image_item_info_t img_phone_20 					= {1, MENU_B_FLASH_START+0x0EC9E0, NULL,	0xff};    // img_phone_20 	 
code image_item_info_t img_phone_21 					= {1, MENU_B_FLASH_START+0x0ED860, NULL,	0xff};    // img_phone_21 	 
code image_item_info_t img_phone_30 					= {1, MENU_B_FLASH_START+0x0EE750, NULL,	0xff};    // img_phone_30 	 
code image_item_info_t img_phone_31 					= {1, MENU_B_FLASH_START+0x0EF5D0, NULL,	0xff};    // img_phone_31 	 
code image_item_info_t img_phone_40 					= {1, MENU_B_FLASH_START+0x0F04C0, NULL,	0xff};    // img_phone_40 	 
code image_item_info_t img_phone_41 					= {1, MENU_B_FLASH_START+0x0F1340, NULL,	0xff};    // img_phone_41 	 
code image_item_info_t img_phone_50 					= {1, MENU_B_FLASH_START+0x0F2230, NULL,	0xff};    // img_phone_50 	 
code image_item_info_t img_phone_51 					= {1, MENU_B_FLASH_START+0x0F30B0, NULL,	0xff};    // img_phone_51 	 
code image_item_info_t img_phone_60 					= {1, MENU_B_FLASH_START+0x0F3FA0, NULL,	0xff};    // img_phone_60 	 
code image_item_info_t img_phone_61 					= {1, MENU_B_FLASH_START+0x0F4E20, NULL,	0xff};    // img_phone_61 	 
code image_item_info_t img_phone_70 					= {1, MENU_B_FLASH_START+0x0F5D10, NULL,	0xff};    // img_phone_70 	 
code image_item_info_t img_phone_71 					= {1, MENU_B_FLASH_START+0x0F6B90, NULL,	0xff};    // img_phone_71 	 
code image_item_info_t img_phone_80 					= {1, MENU_B_FLASH_START+0x0F7A80, NULL,	0xff};    // img_phone_80 	 
code image_item_info_t img_phone_81 					= {1, MENU_B_FLASH_START+0x0F8900, NULL,	0xff};    // img_phone_81 	 
code image_item_info_t img_phone_90 					= {1, MENU_B_FLASH_START+0x0F97F0, NULL,	0xff};    // img_phone_90 	 
code image_item_info_t img_phone_91 					= {1, MENU_B_FLASH_START+0x0FA670, NULL,	0xff};    // img_phone_91 	 
code image_item_info_t img_phone_star0 					= {1, MENU_B_FLASH_START+0x0FB560, NULL,	0xff};    // img_phone_star0 	 
code image_item_info_t img_phone_star1 					= {1, MENU_B_FLASH_START+0x0FC3E0, NULL,	0xff};    // img_phone_star1 	 
code image_item_info_t img_phone_sharp0 				= {1, MENU_B_FLASH_START+0x0FD2D0, NULL,	0xff};    // img_phone_sharp0  
code image_item_info_t img_phone_sharp1 				= {1, MENU_B_FLASH_START+0x0FE150, NULL,	0xff};    // img_phone_sharp1  
code image_item_info_t img_phone_dial0 					= {1, MENU_B_FLASH_START+0x0FF040, NULL,	0xff};    // img_phone_dial0 	 
code image_item_info_t img_phone_dial1 					= {1, MENU_B_FLASH_START+0x101B80, NULL,	0xff};    // img_phone_dial1 	 
code image_item_info_t img_phone_up0 					= {1, MENU_B_FLASH_START+0x1047D0, NULL,	0xff};    // img_phone_up0 	 
code image_item_info_t img_phone_up1 					= {1, MENU_B_FLASH_START+0x105650, NULL,	0xff};    // img_phone_up1 	 
code image_item_info_t img_phone_down0 					= {1, MENU_B_FLASH_START+0x106540, NULL,	0xff};    // img_phone_down0 	 
code image_item_info_t img_phone_down1 					= {1, MENU_B_FLASH_START+0x1073C0, NULL,	0xff};    // img_phone_down1 	 
code image_item_info_t img_phone_left0 					= {1, MENU_B_FLASH_START+0x1082B0, NULL,	0xff};    // img_phone_left0 	 
code image_item_info_t img_phone_left1 					= {1, MENU_B_FLASH_START+0x109130, NULL,	0xff};    // img_phone_left1 	 
code image_item_info_t img_phone_right0 				= {1, MENU_B_FLASH_START+0x10A020, NULL,	0xff};    // img_phone_right0  
code image_item_info_t img_phone_right1 				= {1, MENU_B_FLASH_START+0x10AEA0, NULL,	0xff};    // img_phone_right1  
code image_item_info_t img_phone_check0 				= {1, MENU_B_FLASH_START+0x10BD90, NULL,	0xff};    // img_phone_check0  
code image_item_info_t img_phone_check1 				= {1, MENU_B_FLASH_START+0x10CC10, NULL,	0xff};    // img_phone_check1  
code image_item_info_t img_phone_help0 					= {1, MENU_B_FLASH_START+0x10DB00, NULL,	0xff};    // img_phone_help0 	 
code image_item_info_t img_phone_help1 					= {1, MENU_B_FLASH_START+0x10E9C0, NULL,	0xff};    // img_phone_help1 	 
code image_item_info_t img_phone_dir0 					= {1, MENU_B_FLASH_START+0x10F870, NULL,	0xff};    // img_phone_dir0 	 
code image_item_info_t img_phone_dir1 					= {1, MENU_B_FLASH_START+0x1112A0, NULL,	0xff};    // img_phone_dir1 	 
code image_item_info_t img_phone_set0 					= {1, MENU_B_FLASH_START+0x112CD0, NULL,	0xff};    // img_phone_set0 	 
code image_item_info_t img_phone_set1 					= {1, MENU_B_FLASH_START+0x114700, NULL,	0xff};    // img_phone_set1 	 
code image_item_info_t img_phone_msg0 					= {1, MENU_B_FLASH_START+0x1161B0, NULL,	0xff};    // img_phone_msg0 	 
code image_item_info_t img_phone_msg1 					= {1, MENU_B_FLASH_START+0x117BE0, NULL,	0xff};    // img_phone_msg1 	 
code image_item_info_t img_phone_menu0 					= {1, MENU_B_FLASH_START+0x119610, NULL,	0xff};    // img_phone_menu0 	 
code image_item_info_t img_phone_menu1 					= {1, MENU_B_FLASH_START+0x11B040, NULL,	0xff};    // img_phone_menu1 	 
code image_item_info_t img_carinfo_bg			  		= {1, MENU_B_FLASH_START+0x11CA70, NULL,	0xff};    // img_carinfo_bg 
code image_item_info_t img_demo_bg  					= {1, MENU_B_FLASH_START+0x1299F0, NULL,	0xff};    // img_demo_bg 
code image_item_info_t img_demo_grid  					= {1, MENU_B_FLASH_START+0x13BDA0, NULL,	0xff};    // img_demo_grid  	
code image_item_info_t img_demo_grid1  					= {1, MENU_B_FLASH_START+0x13ED20, NULL,	0xff};    // img_demo_grid1  	
code image_item_info_t img_demo_rose  					= {1, MENU_B_FLASH_START+0x141CA0, NULL,	0xff};    // img_demo_rose  	
code image_item_info_t img_demo_rose1  					= {1, MENU_B_FLASH_START+0x145100, NULL,	0xff};    // img_demo_rose1  	
code image_item_info_t img_demo_ani		  				= {1, MENU_B_FLASH_START+0x148560, NULL,	0xff};    // img_demo_ani		
code image_item_info_t img_demo_ani1	  				= {1, MENU_B_FLASH_START+0x14B360, NULL,	0xff};    // img_demo_ani1	
code image_item_info_t img_demo_palette	  				= {1, MENU_B_FLASH_START+0x14E1C0, NULL,	0xff};    // img_demo_palette	
code image_item_info_t img_demo_palette1  				= {1, MENU_B_FLASH_START+0x1512B0, NULL,	0xff};    // img_demo_palette1
code image_item_info_t img_demo_demoA	  				= {1, MENU_B_FLASH_START+0x1543A0, NULL,	0xff};    // img_demo_demoA	
code image_item_info_t img_demo_demoA1	  				= {1, MENU_B_FLASH_START+0x1571A0, NULL,	0xff};    // img_demo_demoA1	
code image_item_info_t img_demo_demoB	  				= {1, MENU_B_FLASH_START+0x159FA0, NULL,	0xff};    // img_demo_demoB	
code image_item_info_t img_demo_demoB1	  				= {1, MENU_B_FLASH_START+0x15CDA0, NULL,	0xff};    // img_demo_demoB1	
code image_item_info_t img_touch_bg  					= {1, MENU_B_FLASH_START+0x15FBA0, NULL,	0xff};    // img_touch_bg 
code image_item_info_t img_touch_bg_end					= {1, MENU_B_FLASH_START+0x161660, NULL,	0xff};    // img_touch_bg 
code image_item_info_t img_touch_button  				= {1, MENU_B_FLASH_START+0x164510, NULL,	0xff};    // img_touch_button 
code image_item_info_t img_touch_button1  				= {1, MENU_B_FLASH_START+0x164730, NULL,	0xff};    // img_touch_button1 
code image_item_info_t img_btooth_bg  					= {1, MENU_B_FLASH_START+0x164950, NULL,	0xff};    // img_btooth_bg 
code image_item_info_t img_yuv_menu_bg  				= {1, MENU_B_FLASH_START+0x188C10, NULL,	0x00};    // img_yuv_menu_bg 
code image_item_info_t img_yuv_bright  					= {1, MENU_B_FLASH_START+0x19B4A0, NULL,	0xff};    // img_yuv_bright 
code image_item_info_t img_yuv_bright1  				= {1, MENU_B_FLASH_START+0x19E090, NULL,	0xff};    // img_yuv_bright1 
code image_item_info_t img_yuv_contrast  				= {1, MENU_B_FLASH_START+0x1A0C80, NULL,	0xff};    // img_yuv_contrast 
code image_item_info_t img_yuv_contrast1  				= {1, MENU_B_FLASH_START+0x1A3870, NULL,	0xff};    // img_yuv_contrast1 
code image_item_info_t img_yuv_hue  					= {1, MENU_B_FLASH_START+0x1A6460, NULL,	0xff};    // img_yuv_hue 
code image_item_info_t img_yuv_hue1  					= {1, MENU_B_FLASH_START+0x1A9050, NULL,	0xff};    // img_yuv_hue1 
code image_item_info_t img_yuv_saturate  				= {1, MENU_B_FLASH_START+0x1ABC40, NULL,	0xff};    // img_yuv_saturate 
code image_item_info_t img_yuv_saturate1  				= {1, MENU_B_FLASH_START+0x1AE830, NULL,	0xff};    // img_yuv_saturate1 
code image_item_info_t img_yuv_sharp  					= {1, MENU_B_FLASH_START+0x1B1420, NULL,	0xff};    // img_yuv_sharp 
code image_item_info_t img_yuv_sharp1  					= {1, MENU_B_FLASH_START+0x1B4010, NULL,	0xff};    // img_yuv_sharp1 
code image_item_info_t img_rgb_menu_bg  				= {1, MENU_B_FLASH_START+0x1B6C00, NULL,	0x00};    // img_rgb_menu_bg 
code image_item_info_t img_rgb_bright  					= {1, MENU_B_FLASH_START+0x1C9490, NULL,	0xff};    // img_rgb_bright 
code image_item_info_t img_rgb_bright1 					= {1, MENU_B_FLASH_START+0x1CDD30, NULL,	0xff};    // img_rgb_bright1 
code image_item_info_t img_rgb_contrast  				= {1, MENU_B_FLASH_START+0x1D25D0, NULL,	0xff};    // img_rgb_contrast 
code image_item_info_t img_rgb_contrast1  				= {1, MENU_B_FLASH_START+0x1D6E70, NULL,	0xff};    // img_rgb_contrast1 
code image_item_info_t img_rgb_color 					= {1, MENU_B_FLASH_START+0x1DB710, NULL,	0xff};    // img_rgb_color 
code image_item_info_t img_rgb_color1 					= {1, MENU_B_FLASH_START+0x1DFFB0, NULL,	0xff};    // img_rgb_color1 
code image_item_info_t img_apc_menu_bg  				= {1, MENU_B_FLASH_START+0x1E4850, NULL,	0x00};    // img_apc_menu_bg 
code image_item_info_t img_apc_bright  					= {1, MENU_B_FLASH_START+0x1F70E0, NULL,	0xff};    // img_apc_bright 
code image_item_info_t img_apc_bright1 					= {1, MENU_B_FLASH_START+0x1F8CB0, NULL,	0xff};    // img_apc_bright1 
code image_item_info_t img_apc_contrast  				= {1, MENU_B_FLASH_START+0x1FA880, NULL,	0xff};    // img_apc_contrast 
code image_item_info_t img_apc_contrast1  				= {1, MENU_B_FLASH_START+0x1FC450, NULL,	0xff};    // img_apc_contrast1 
code image_item_info_t img_apc_color	  				= {1, MENU_B_FLASH_START+0x1FE020, NULL,	0xff};    // img_apc_color 
code image_item_info_t img_apc_color1	  				= {1, MENU_B_FLASH_START+0x1FFBF0, NULL,	0xff};    // img_apc_color1
code image_item_info_t img_apc_position  				= {1, MENU_B_FLASH_START+0x2017C0, NULL,	0xff};    // img_apc_position 
code image_item_info_t img_apc_position1  				= {1, MENU_B_FLASH_START+0x203390, NULL,	0xff};    // img_apc_position1 
code image_item_info_t img_apc_phase  					= {1, MENU_B_FLASH_START+0x204F60, NULL,	0xff};    // img_apc_phase 
code image_item_info_t img_apc_phase1  					= {1, MENU_B_FLASH_START+0x206B30, NULL,	0xff};    // img_apc_phase1 
code image_item_info_t img_apc_pclock  					= {1, MENU_B_FLASH_START+0x208700, NULL,	0xff};    // img_apc_pclock 
code image_item_info_t img_apc_pclock1 					= {1, MENU_B_FLASH_START+0x20A2D0, NULL,	0xff};    // img_apc_pclock1 
code image_item_info_t img_apc_autoadj  				= {1, MENU_B_FLASH_START+0x20BEA0, NULL,	0xff};    // img_apc_autoadj 
code image_item_info_t img_apc_autoadj1  				= {1, MENU_B_FLASH_START+0x20DA70, NULL,	0xff};    // img_apc_autoadj1 
code image_item_info_t img_apc_autocolor  				= {1, MENU_B_FLASH_START+0x20F640, NULL,	0xff};    // img_apc_autocolor
code image_item_info_t img_apc_autocolor1  				= {1, MENU_B_FLASH_START+0x211210, NULL,	0xff};    // img_apc_autocolor1
code image_item_info_t img_hdmi_menu_bg  				= {1, MENU_B_FLASH_START+0x212DE0, NULL,	0x00};    // img_hdmi_menu_bg 
code image_item_info_t img_hdmi_mode  					= {1, MENU_B_FLASH_START+0x225670, NULL,	0xff};    // img_hdmi_mode 
code image_item_info_t img_hdmi_mode1  					= {1, MENU_B_FLASH_START+0x22C2F0, NULL,	0xff};    // img_hdmi_mode1 
code image_item_info_t img_hdmi_setting  				= {1, MENU_B_FLASH_START+0x232EB0, NULL,	0xff};    // img_hdmi_setting 
code image_item_info_t img_hdmi_setting1  				= {1, MENU_B_FLASH_START+0x239B30, NULL,	0xff};    // img_hdmi_setting1 
code image_item_info_t img_display_bg  					= {1, MENU_B_FLASH_START+0x2407B0, NULL,	0x00};    // img_display_bg 
code image_item_info_t img_display_aspect  				= {1, MENU_B_FLASH_START+0x253040, NULL,	0xff};    // img_display_aspect 
code image_item_info_t img_display_aspect1 				= {1, MENU_B_FLASH_START+0x255C30, NULL,	0xff};    // img_display_aspect1 
code image_item_info_t img_display_osd  				= {1, MENU_B_FLASH_START+0x258820, NULL,	0xff};    // img_display_osd 
code image_item_info_t img_display_osd1  				= {1, MENU_B_FLASH_START+0x25B410, NULL,	0xff};    // img_display_osd1 
code image_item_info_t img_display_flip  				= {1, MENU_B_FLASH_START+0x25E000, NULL,	0xff};    // img_display_flip 
code image_item_info_t img_display_flip1  				= {1, MENU_B_FLASH_START+0x260BF0, NULL,	0xff};    // img_display_flip1 
code image_item_info_t img_display_backlight  			= {1, MENU_B_FLASH_START+0x2637E0, NULL,	0xff};    // img_display_backlight 
code image_item_info_t img_display_backlight1  			= {1, MENU_B_FLASH_START+0x2663D0, NULL,	0xff};    // img_display_backlight1 
code image_item_info_t img_display_resolution  			= {1, MENU_B_FLASH_START+0x268FC0, NULL,	0xff};    // img_display_resolution 
code image_item_info_t img_display_resolution1  		= {1, MENU_B_FLASH_START+0x26BBB0, NULL,	0xff};    // img_display_resolution1 
code image_item_info_t img_osd_bg  						= {1, MENU_B_FLASH_START+0x26E7A0, NULL,	0x00};    // img_osd_bg 
code image_item_info_t img_osd_timer  					= {1, MENU_B_FLASH_START+0x281030, NULL,	0xff};    // img_osd_timer 
code image_item_info_t img_osd_timer1  					= {1, MENU_B_FLASH_START+0x287CB0, NULL,	0xff};    // img_osd_timer1 
code image_item_info_t img_osd_trans  					= {1, MENU_B_FLASH_START+0x28E930, NULL,	0xff};    // img_osd_trans 
code image_item_info_t img_osd_trans1  					= {1, MENU_B_FLASH_START+0x2955B0, NULL,	0xff};    // img_osd_trans1 
code image_item_info_t img_dialog_ok		  			= {1, MENU_B_FLASH_START+0x29C230, NULL,	0xff};    // img_dialog_ok 
code image_item_info_t img_dialog_ok1	  				= {1, MENU_B_FLASH_START+0x29CE40, NULL,	0xff};    // img_dialog_ok1 
code image_item_info_t img_dialog_cancel  				= {1, MENU_B_FLASH_START+0x29DA50, NULL,	0xff};    // img_dialog_cancel 
code image_item_info_t img_dialog_cancel1  				= {1, MENU_B_FLASH_START+0x29ECA0, NULL,	0xff};    // img_dialog_cancel1 
code image_item_info_t img_autoadj_bg  					= {1, MENU_B_FLASH_START+0x29FEF0, NULL,	0xff};    // img_autoadj_bg 
code image_item_info_t img_autocolor_bg  				= {1, MENU_B_FLASH_START+0x2A3600, NULL,	0xff};    // img_autocolor_bg 
code image_item_info_t img_flip_bg  					= {1, MENU_B_FLASH_START+0x2A6640, NULL,	0xff};    // img_flip_bg 
code image_item_info_t img_sysrestore_bg  				= {1, MENU_B_FLASH_START+0x2A87C0, NULL,	0xff};    // img_sysrestore_bg 
code image_item_info_t img_sysinfo_bg  					= {1, MENU_B_FLASH_START+0x2AB940, NULL,	0xff};    // img_sysinfo_bg 
code image_item_info_t img_resolution_bg  				= {1, MENU_B_FLASH_START+0x2AD400, NULL,	0xff};    // img_resolution_bg 
code image_item_info_t img_slide_bg  					= {1, MENU_B_FLASH_START+0x2AE8E0, NULL,	0xff};    // img_slide_bg 
code image_item_info_t img_slide3_bg  					= {1, MENU_B_FLASH_START+0x2B0130, NULL,	0xff};    // img_slide3_bg 
code image_item_info_t img_slide_gray  					= {1, MENU_B_FLASH_START+0x2B1A90, NULL,	0xff};    // img_slide_gray 
code image_item_info_t img_slide_red  					= {1, MENU_B_FLASH_START+0x2B22A0, NULL,	0xff};    // img_slide_red 
code image_item_info_t img_slide_left  					= {1, MENU_B_FLASH_START+0x2B2AB0, NULL,	0xff};    // img_slide_left 
code image_item_info_t img_slide_left1  				= {1, MENU_B_FLASH_START+0x2B3560, NULL,	0xff};    // img_slide_lef1t 
code image_item_info_t img_slide_right  				= {1, MENU_B_FLASH_START+0x2B4010, NULL,	0xff};    // img_slide_right 
code image_item_info_t img_slide_right1  				= {1, MENU_B_FLASH_START+0x2B4AC0, NULL,	0xff};    // img_slide_right1 
code image_item_info_t img_slide_backlight  			= {1, MENU_B_FLASH_START+0x2B5570, NULL,	0xff};    // img_slide_backlight 
code image_item_info_t img_slide_bright  				= {1, MENU_B_FLASH_START+0x2B5C60, NULL,	0xff};    // img_slide_bright 
code image_item_info_t img_slide_clock  				= {1, MENU_B_FLASH_START+0x2B6420, NULL,	0xff};    // img_slide_clock 
code image_item_info_t img_slide_contrast  				= {1, MENU_B_FLASH_START+0x2B6AD0, NULL,	0xff};    // img_slide_contrast 
code image_item_info_t img_slide_rgb	  				= {1, MENU_B_FLASH_START+0x2B7010, NULL,	0xff};    // img_slide_rgb 
code image_item_info_t img_slide_hue  					= {1, MENU_B_FLASH_START+0x2B7320, NULL,	0xff};    // img_slide_hue 
code image_item_info_t img_slide_phase  				= {1, MENU_B_FLASH_START+0x2B7630, NULL,	0xff};    // img_slide_phase 
code image_item_info_t img_slide_saturate  				= {1, MENU_B_FLASH_START+0x2B7A50, NULL,	0xff};    // img_slide_saturate 
code image_item_info_t img_slide_sharp  				= {1, MENU_B_FLASH_START+0x2B8080, NULL,	0xff};    // img_slide_sharp 
code image_item_info_t img_slide_timer  				= {1, MENU_B_FLASH_START+0x2B8810, NULL,	0xff};    // img_slide_timer 
code image_item_info_t img_slide_trans  				= {1, MENU_B_FLASH_START+0x2B8C10, NULL,	0xff};    // img_slide_trasn 
code image_item_info_t img_position_bg  				= {1, MENU_B_FLASH_START+0x2B95B0, NULL,	0xff};    // img_position_bg 
code image_item_info_t img_position_box_gray  			= {1, MENU_B_FLASH_START+0x2BC5D0, NULL,	0xff};    // img_position_box_gray 
code image_item_info_t img_position_box_red  			= {1, MENU_B_FLASH_START+0x2C07F0, NULL,	0xff};    // img_position_box_red 
code image_item_info_t img_position_up  				= {1, MENU_B_FLASH_START+0x2C4970, NULL,	0xff};    // img_position_up 
code image_item_info_t img_position_down  				= {1, MENU_B_FLASH_START+0x2C5050, NULL,	0xff};    // img_position_down 
code image_item_info_t img_position_left  				= {1, MENU_B_FLASH_START+0x2C5730, NULL,	0xff};    // img_position_left 
code image_item_info_t img_position_right  				= {1, MENU_B_FLASH_START+0x2C5E10, NULL,	0xff};    // img_position_right 
code image_item_info_t img_popup_aspect_bg  			= {1, MENU_B_FLASH_START+0x2C64F0, NULL,	0xff};    // img_aspect_bg 
code image_item_info_t img_popup_aspect_normal 			= {1, MENU_B_FLASH_START+0x2CA6A0, NULL,	0xff};    // img_aspect_normal 
code image_item_info_t img_popup_aspect_normal1 		= {1, MENU_B_FLASH_START+0x2CB000, NULL,	0xff};    // img_aspect_normal1 
code image_item_info_t img_popup_aspect_normal_select	= {1, MENU_B_FLASH_START+0x2CB940, NULL,	0xff};    // img_aspect_normal_select 
code image_item_info_t img_popup_aspect_zoom  			= {1, MENU_B_FLASH_START+0x2CC170, NULL,	0xff};    // img_aspect_zoom 
code image_item_info_t img_popup_aspect_zoom1  			= {1, MENU_B_FLASH_START+0x2CC900, NULL,	0xff};    // img_aspect_zoom1 
code image_item_info_t img_popup_aspect_zoom_select		= {1, MENU_B_FLASH_START+0x2CD110, NULL,	0xff};    // img_aspect_zoom_select 
code image_item_info_t img_popup_aspect_full  			= {1, MENU_B_FLASH_START+0x2CD920, NULL,	0xff};    // img_aspect_full
code image_item_info_t img_popup_aspect_full1  			= {1, MENU_B_FLASH_START+0x2CDF90, NULL,	0xff};    // img_aspect_full1
code image_item_info_t img_popup_aspect_full_select		= {1, MENU_B_FLASH_START+0x2CE620, NULL,	0xff};    // img_aspect_full_select
code image_item_info_t img_popup_aspect_pano  			= {1, MENU_B_FLASH_START+0x2CEE00, NULL,	0xff};    // img_aspect_pano
code image_item_info_t img_popup_aspect_pano1  			= {1, MENU_B_FLASH_START+0x2CF940, NULL,	0xff};    // img_aspect_pano1
code image_item_info_t img_popup_aspect_pano_sel		= {1, MENU_B_FLASH_START+0x2D04A0, NULL,	0xff};    // img_aspect_pano_sel
code image_item_info_t img_dvi_mode_bg  				= {1, MENU_B_FLASH_START+0x2D0C80, NULL,	0xff};    // img_dvi_mode_bg 
code image_item_info_t img_dvi_mode_24bit  				= {1, MENU_B_FLASH_START+0x2E3B60, NULL,	0xff};    // img_dvi_mode_24bit 
code image_item_info_t img_dvi_mode_24bit1  			= {1, MENU_B_FLASH_START+0x2E4B90, NULL,	0xff};    // img_dvi_mode_24bit1 
code image_item_info_t img_dvi_mode_16bit  				= {1, MENU_B_FLASH_START+0x2E5C80, NULL,	0xff};    // img_dvi_mode_16bit 
code image_item_info_t img_dvi_mode_16bit1  			= {1, MENU_B_FLASH_START+0x2E6D50, NULL,	0xff};    // img_dvi_mode_16bit1 
code image_item_info_t img_dvi_mode_select24  			= {1, MENU_B_FLASH_START+0x2E7E70, NULL,	0xff};    // img_dvi_mode_select24 
code image_item_info_t img_dvi_mode_select16  			= {1, MENU_B_FLASH_START+0x2E8AE0, NULL,	0xff};    // img_dvi_mode_select16 
code image_item_info_t img_hdmi_mode_bg  				= {1, MENU_B_FLASH_START+0x2E9750, NULL,	0xff};    // img_hdmi_mode_bg 
code image_item_info_t img_hdmi_mode_pc  				= {1, MENU_B_FLASH_START+0x2FC630, NULL,	0xff};    // img_hdmi_mode_pc 
code image_item_info_t img_hdmi_mode_pc1  				= {1, MENU_B_FLASH_START+0x2FD4E0, NULL,	0xff};    // img_hdmi_mode_pc1 
code image_item_info_t img_hdmi_mode_tv  				= {1, MENU_B_FLASH_START+0x2FE330, NULL,	0xff};    // img_hdmi_mode_tv 
code image_item_info_t img_hdmi_mode_tv1  				= {1, MENU_B_FLASH_START+0x2FF1C0, NULL,	0xff};    // img_hdmi_mode_tv1 
code image_item_info_t img_hdmi_mode_selectPC  			= {1, MENU_B_FLASH_START+0x300030, NULL,	0xff};    // img_hdmi_mode_selectPC 
code image_item_info_t img_hdmi_mode_selectTV  			= {1, MENU_B_FLASH_START+0x3010E0, NULL,	0xff};    // img_hdmi_mode_selectTV 
code image_item_info_t img_wait  						= {2, MENU_B_FLASH_START+0x3021D0, &img_wait_header,	0x00};    // img_wait. 48x50x10 


//=================================
// FPGA TEST IMAGE
//=================================
code image_item_info_t img_fpga_200			= {1, FPGA_TEST_IMG+0x000000, NULL, 0xff};
code image_item_info_t img_fpga_300			= {1, FPGA_TEST_IMG+0x0005D7, NULL, 0xff};
code image_item_info_t img_fpga_400			= {1, FPGA_TEST_IMG+0x000AE9, NULL, 0xff};
code image_item_info_t img_fpga_800			= {1, FPGA_TEST_IMG+0x000FF3, NULL, 0xff};

#define FPGA_TEST_IMG2	0x330000
code image_item_info_t img_bgr_bar			= {1, FPGA_TEST_IMG2+0x000000, NULL, 0xff};
code image_item_info_t img_alpha_bar		= {1, FPGA_TEST_IMG2+0x013C90, NULL, 0xff};
#ifdef MODEL_TW8835
#else
code image_item_info_t img_1366_768			= {1, 0x800000, NULL, 0xff};  /* 0x800000~0xA00C20 */
#define FPGA_TEST_BAR864	0xA10000		/* +0x9F4A */
code image_item_info_t img_bar864_8			= {1, FPGA_TEST_BAR864+0x000000, NULL, 0xff}; // Bar327x45_8bpp   
code image_item_info_t img_bar864_8C		= {1, FPGA_TEST_BAR864+0x003D8B, NULL, 0xff}; // Bar327x45_8bpp_Rle  
code image_item_info_t img_bar864_6			= {1, FPGA_TEST_BAR864+0x004904, NULL, 0xff}; // Bar327x45_6bpp  
code image_item_info_t img_bar864_6C		= {1, FPGA_TEST_BAR864+0x007531, NULL, 0xff}; // Bar327x45_6bpp_Rle  
code image_item_info_t img_bar864_4			= {1, FPGA_TEST_BAR864+0x007C9D, NULL, 0xff}; // Bar327x45_4bpp  
code image_item_info_t img_bar864_4C		= {1, FPGA_TEST_BAR864+0x0099AB, NULL, 0xff}; // Bar327x45_4bpp_Rle  
#endif




typedef struct MonOsdData_s {
	struct image_item_info_s *image;
	BYTE name[10];	
} MonOsdData_t;

code MonOsdData_t MonSOsdImgTable[] = {
	{ &img_logo, 			"logo     "},
	{ &img_main_bg, 		"main_bg  "},
	{ &img_main_input,		"input0   " },
	{ &img_main_input1,		"input1   " },
	{ &img_main_system,		"system0  "},
	{ &img_main_system1,	"system1  "},
	{ &img_input_bg_bottom,	"input_bg0"},
	{ &img_input_bg_top,	"input_bg1"},
	{ &img_input_cvbs,		"cvbs0    "	},
	{ &img_input_cvbs1,		"cvbs1    " },
	{ &img_navi_return,		"return0  "},
	{ &img_navi_return1,	"return1  " },
	//===================================
	{ &img_fpga_200, 		"fpga_200 "},
	{ &img_fpga_300, 		"fpga_300 "},
	{ &img_fpga_400, 		"fpga_400 "},
	{ &img_fpga_800, 		"fpga_800 "},
	//===================================
	{ &img_bgr_bar, 		"bgr_bar "},
	{ &img_alpha_bar, 		"alpha_bar"},
#ifdef MODEL_TW8835
#else
	{ &img_1366_768, 		"1366_768"},
	//===================================
	{ &img_bar864_8, 		"Bar8bpp"},
	{ &img_bar864_8C, 		"Bar8bppC"},
	{ &img_bar864_6, 		"Bar6bpp"},
	{ &img_bar864_6C, 		"Bar6bppC"},
	{ &img_bar864_4, 		"Bar4bpp"},
	{ &img_bar864_4C, 		"Bar4bppC"},
	//===================================
#endif
};

extern menu_image_header_t header_table;
extern void MenuPrepareImageHeader(struct image_item_info_s *image);
void MonSOsdImgInfo(void)
{
	struct image_item_info_s *image;
	//image_info_t *info;
	menu_image_header_t *header = &header_table;	//link header buffer.
	BYTE i;

#ifdef MODEL_TW8835
	for(i=0; i < (12+4+2); i++)
#else
	for(i=0; i < (12+4+2+1+6); i++)
#endif
	{
		Printf("\n%02bd %s",i,MonSOsdImgTable[i].name);
		image = MonSOsdImgTable[i].image;
		Printf(" loc:%lx alpha:%bx",image->loc,image->alpha);

		//prepare header
		MenuPrepareImageHeader(image);

		//header info
		Printf(" bpp%bd", header->bpp);
		Printf(" rle%bd", header->rle);
		Printf(" %dx%d", header->dx, header->dy);
		Printf(" alpha:%2bx",image->alpha);
		Printf(" lut%s size:%d*4",header->lut_type? "s": " ", header->lut_size >> 2);		 
	}
}
void MonOsdLutLoad(BYTE img_n, BYTE sosd_win, WORD lut)
{
	struct image_item_info_s *image;
	menu_image_header_t *header = &header_table;	//link header buffer.


	Printf("\nMonOsdLutLoad(%bd,%bd,%d)",img_n,lut);

	//BYTE i;
	image = MonSOsdImgTable[img_n].image;
	//prepare header
	MenuPrepareImageHeader(image);
	
	//Load Palette
	SpiOsdLoadLUT(sosd_win, header->lut_type, lut, header->lut_size, header->lut_loc,0xFF);
}

extern BYTE UseSOsdHwBuff;
extern WORD SOsdHwBuff_alpha_A;
extern WORD SOsdHwBuff_alpha_B;

void MonOsdImgLoad(BYTE img_n, BYTE sosd_win, WORD item_lut)
{
	struct image_item_info_s *image;
	menu_image_header_t *header = &header_table;	//link header buffer.
//	BYTE i;
	WORD sx,sy;

	Printf("\nMonOsdImgLoad(%bd,%bd,%d)",img_n,sosd_win,item_lut);

#if 0
	UseSOsdHwBuff=1;
#endif
	sx=sy=0;
//	SOsdWinBuffClean(0);

	image = MonSOsdImgTable[img_n].image;

	//prepare header
	MenuPrepareImageHeader(image);


	//see MenuDrawCurrImage
	//fill out sosd_buff
	SpiOsdWinImageLoc( sosd_win, header->image_loc); 
	SpiOsdWinImageSizeWH( sosd_win, header->dx, header->dy );
	SpiOsdWinScreen( sosd_win, sx, sy, header->dx, header->dy );
	if(sosd_win==0) {
		SpiOsdWin0ImageOffsetXY( 0, 0 );
		SpiOsdWin0Animation( 1, 0, 0, 0);
	}
	if(image->alpha != 0xFF)
		SpiOsdWinPixelAlpha( sosd_win, ON );
	else {
		SpiOsdWinGlobalAlpha( sosd_win, 0 /*EE_Read(EEP_OSD_TRANSPARENCY)*/);
	}
	SpiOsdWinPixelWidth(sosd_win, header->bpp);
	SpiOsdWinLutOffset(sosd_win, item_lut);

	SpiOsdWinBuffEnable( sosd_win, ON );
	//
	//write to HW
	//
#if 1
	if(UseSOsdHwBuff) 
	{
		if(header->rle)
			SOsdHwBuffSetRle(sosd_win,header->bpp,header->rle);
		SOsdHwBuffSetLut(sosd_win, /*header->lut_type,*/ item_lut, header->lut_size, header->lut_loc);
	
		//pixel alpha blending. after load Palette
		if(image->alpha != 0xFF)
			SOsdHwBuffSetAlpha(sosd_win, item_lut+image->alpha);

		SOsdWinBuffWrite2Hw(sosd_win, sosd_win); //SOsdHwBuffWrite2Hw();
		UseSOsdHwBuff = 0;

		//update ALPHA
		if(SOsdHwBuff_alpha_A != 0xFFFF) {
			WriteTW88Page(4);
			WriteTW88(REG410, 0xc3 );    		// LUT Write Mode, En & byte ptr inc.

			if(SOsdHwBuff_alpha_A >> 8)	WriteTW88(REG410, ReadTW88(REG410) | 0x08);	//support 512 palette
			else            			WriteTW88(REG410, ReadTW88(REG410) & 0xF7);
			WriteTW88(REG411, (BYTE)SOsdHwBuff_alpha_A ); 	// alpha index
			WriteTW88(REG412, 0x7F/*value*/ ); 			// alpha value

			SOsdHwBuff_alpha_A = 0xFFFF;
		}
		if(SOsdHwBuff_alpha_B != 0xFFFF) {
			WriteTW88Page(4);
			WriteTW88(REG410, 0xc3 | 0x04);    		// LUT Write Mode, En & byte ptr inc.

			WriteTW88(REG410, ReadTW88(REG410) & 0xF7);
			WriteTW88(REG411, (BYTE)SOsdHwBuff_alpha_B ); 	// alpha index
			WriteTW88(REG412, 0x7F/*value*/ ); 			// alpha value

			SOsdHwBuff_alpha_B = 0xFFFF;
		}
	}
#endif
	//else 
#if 1
	{
		//WaitVBlank(1);
		if(header->rle) {	//need RLE ?
			SpiOsdRlcReg( sosd_win, header->bpp,header->rle);
		}	
		else {
			//We using RLE only on the background.
			//if(item == 0) {
			//	SpiOsdDisableRlcReg(??winno)
			//	SpiOsdRlcReg( 0,0,0); //BK110217
			//}
		}
		WaitVBlank(1);
	
		//Load Palette
		//SpiOsdLoadLUT(header->lut_type, menu_item->lut, header->lut_size, header->lut_loc, image->alpha);
		SpiOsdLoadLUT(sosd_win, header->lut_type, item_lut, header->lut_size, header->lut_loc, image->alpha);
	
		//WaitVBlank(1);
		//update HW
		SOsdWinBuffWrite2Hw(sosd_win, sosd_win);
	
	}
#endif
}


#endif //..SUPPORT_SPIOSD
