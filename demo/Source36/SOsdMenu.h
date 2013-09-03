#ifndef __TW8835_MENU_B_H__
#define __TW8835_MENU_B_H__

//sosd: Spi OSD
//fosd: Font OSD

#define SOSD_WIN0	0x00
#define SOSD_WIN1	0x01
#define SOSD_WIN2	0x02
#define SOSD_WIN3	0x03
#define SOSD_WIN4	0x04
#define SOSD_WIN5	0x05
#define SOSD_WIN6	0x06
#define SOSD_WIN7	0x07
#define SOSD_WIN8	0x08

//#define FOSD_WIN1	0x10
//#define FOSD_WIN2	0x20
//#define FOSD_WIN3	0x30
//#define FOSD_WIN4	0x40


//WIN0: used for FillColor window that is used when menu change the background image.
//WIN1: WIN0 do not support RLE. We have to use WIN1 for Background.
//WIN8: Highest window(except WIN0) is used for the pressed icon.
#define SOSD_WIN_FILL	0
#define SOSD_WIN_BG		1
#define SOSD_WIN_ICON	8

//#define OSDWINTYPE_ICON		0
//#define OSDWINTYPE_BG		1
//#define OSDWINTYPE_PBAR		2
//#define OSDWINTYPE_TUNER	3

#define NAVI_KEY_NONE	0
#define NAVI_KEY_ENTER	1
#define NAVI_KEY_UP		2
#define NAVI_KEY_DOWN	3
#define NAVI_KEY_LEFT	4
#define NAVI_KEY_RIGHT	5


#define ALIGN_TOPLEFT		0
#define ALIGN_TOPMIDDLE		1
#define ALIGN_TOPRIGHT		2
#define ALIGN_CENTER		3
#define ALIGN_BOTTOMLEFT	4
#define ALIGN_BOTTOMRIGHT	5
#define ALIGN_BOTTOMMIDDLE	6

//#define IMG_TYPE_BK			0
//#define IMG_TYPE_BTN		1
//#define IMG_TYPE_ICON		2
//#define IMG_TYPE_STR		3
//#define IMG_TYPE_BMP		4


//if use s first, I can assign 9 items, but if use f first, compiler will accept 12 items on the code segment.
//typedef union {
//	struct sosd_menu_item_s	s;
//	struct fosd_menu_item_s f;
//	BYTE test[12];
//} osd_type_meun_item_t;

// image table
//#define OSD_LANG_ENG	0
//#define OSD_LANG_KOR	1
//#define OSD_LANG_JAP	2
//#define OSD_LANG_CHN	3
//#define OSD_LANG_MAX	3

struct fosd_menu_item_s {
	BYTE  	ram;				//osd_ram_start_offset;
	BYTE	zoom;				//upper:h bottom:v
	BYTE 	w,h;				//n_char, 1
	BYTE 	ch_space_low;
	//some more....
	BYTE	color;				//bgColor index + fgColor index
	BYTE	win;
	BYTE	alpha_value;
	BYTE	alpha_color_index;	//only used when alpha_value is not 0
	BYTE	reserved9;
	BYTE	reserved10;
	BYTE	reserved11;
};

#define LUT_TYPE_G	0
#define LUT_TYPE_S	1

//menu_image_header
typedef struct menu_image_header_s {
	DWORD lut_loc;
	DWORD image_loc;

	BYTE lut_type;	//1:luts(byte pointer method.default on TW8835MENU), 0:lut(address method)
	BYTE bpp;
	BYTE rle;		//upper:bpp bottom:rle_counter.
	WORD dx,dy;		//width, height
	WORD lut_size;	//we don't need it. we can assume LUT size & image size.
} menu_image_header_t;


typedef struct image_info_s {
	BYTE lut_type;	//1:luts(byte pointer method.default on TW8835MENU), 0:lut(address method)
	BYTE rle;		//upper:bpp bottom:rle_counter.
	WORD dx,dy;		//width, height
	// DWORD size; -- we don't need it. we can assume LUT size & image size.
} image_info_t;

#define IMAGE_ITEM_TYPE_0		0	// LUT+IMAGE and use image_info_s
#define IMAGE_ITEM_TYPE_1		1	// RLE2_HEADER+LUT+IMAGE.
#define IMAGE_ITEM_TYPE_2		2	// RLE2_HEADER+LUT+IMAGE but, use image_info_s
#define IMAGE_ITEM_TYPE_3		3	// N/A	IMAGE only
#define IMAGE_ITEM_TYPE_4		4	// N/A  LUT only
#define IMAGE_ITEM_TYPE_5		5	// N/A
#define IMAGE_ITEM_TYPE_6		6	// N/A

#define IMG_TYPE_LUTIMG		0	//LUT+IMG, no size info
#define IMG_TYPE_RLE2		1	//REL2
#define IMG_TYPE_RLE2INFO	2	//RLE2, but use image_info_s
#define IMG_TYPE_IMG		3	//IMAGE ONLY. no size ifno
#define IMG_TYPE_LUT		4	//LUT ONLY.	no size info

typedef struct image_item_info_s {
	BYTE type;	//0:LUT+image, 1:header+lut+image 2:image_only 3: lut_only 4:header_only	
	DWORD loc;  //absolute location on SpiFlash
	image_info_t *info;
	BYTE alpha;		//alpha index. 0xFF:ignore.
} image_item_info_t;



#define TYPE_SOSD	0	//normal SOSD item
#define TYPE_FOSD	1	//normal FOSD item
#define TYPE_BG		2	//SOSD FIXED Background
#define TYPE_MBG	3	//SOSD MOVING Background
#define TYPE_SLIDE	4	//SOSD Slide area
#define TYPE_BOX	5	//SOSD BOX area

//menu spi osd item information
//26
typedef struct menu_sitem_info_s
{
	BYTE osd_type;						//0:SpiOSD 1:FontOSD..failed. not using

	void (*p)(void);					//linked function

	WORD x,y;							//start x, start y
	BYTE align;							//align type

	struct image_item_info_s *image;	//focused image
	struct image_item_info_s *image1;	//pressed image
	BYTE image_type;					//0:Background, icon, ....., fSOD info

	BYTE win;							//upper:fosd, bottom:sosd
	WORD lut;							//lut start offset	   1110207 become 9bit
	BYTE up,left,down,right;			//navi_key
} menu_sitem_info_t;

//menu font osd item information
//15
typedef struct menu_fitem_info_s {
	BYTE winno;			//win#. start from 0.
	WORD osdram;		//start of osdram
	WORD x,y;
	BYTE w,h;
	BYTE zoom_h,zoom_v;

	BYTE BPP1Color;		//bgColor << 4 | fgColor;
	BYTE BPP3_palette;	//for 3BPP
	WORD BPP3_color;	//for 3BPP
} menu_fitem_info_t;

//13
#define MENU_TYPE_NORMAL	0
#define MENU_TYPE_LIST		1
#define MENU_TYPE_SCRLDN	2
#define MENU_TYPE_DIALOG	3
#define MENU_TYPE_SLIDE		4
#define MENU_TYPE_SLIDE3	5
#define MENU_TYPE_NOTE		0x10	//need the selected note.
#define MENU_TYPE_TEST		0x20

typedef struct menu_page_info_s {
	BYTE type;					//menu_type	move_left2right, popup,...
	BYTE focus;					//focused item number
	BYTE select;				//current selected item number. for input_page aspect_page...
	BYTE item_start;
	BYTE item_total;			// for touch area check. not include bg, and select bar.
	menu_sitem_info_t *items;	//sosd item
	menu_fitem_info_t *texts;	//fosd item	
} menu_page_info_t;

extern code image_item_info_t img_logo;
extern code image_item_info_t img_navi_menu;
extern code image_item_info_t img_navi_return;				
extern code image_item_info_t img_navi_return1;			
extern code image_item_info_t img_navi_home;  					
extern code image_item_info_t img_navi_home1;  					
extern code image_item_info_t img_navi_close;  					
extern code image_item_info_t img_navi_close1;  				
extern code image_item_info_t img_navi_demo;  					
extern code image_item_info_t img_navi_demo1;  					
extern code image_item_info_t img_navi_setup;  					
extern code image_item_info_t img_navi_setup1;  				
extern code image_item_info_t img_main_bg;  					
extern code image_item_info_t img_main_input;  					
extern code image_item_info_t img_main_input1;  				
extern code image_item_info_t img_main_audio;  					
extern code image_item_info_t img_main_audio1;  				
extern code image_item_info_t img_main_system;  				
extern code image_item_info_t img_main_system1;  				
extern code image_item_info_t img_main_gps;  					
extern code image_item_info_t img_main_gps1;  					
extern code image_item_info_t img_main_phone;  					
extern code image_item_info_t img_main_phone1;  				
extern code image_item_info_t img_main_carinfo;  				
extern code image_item_info_t img_main_carinfo1;  				
extern code image_item_info_t img_input_bg_bottom;  			
extern code image_item_info_t img_input_bg_top;  				
extern code image_item_info_t img_input_nodvi_bg_top;
extern code image_item_info_t img_input_select;  				
//extern code image_item_info_t img_input_nodvi_bg_bottom;  			
//extern code image_item_info_t img_input_nodvi_select;  				
extern code image_item_info_t img_input_cvbs; 		 			
extern code image_item_info_t img_input_cvbs1; 		 			
extern code image_item_info_t img_input_svideo;  				
extern code image_item_info_t img_input_svideo1;  				
extern code image_item_info_t img_input_ypbpr;  				
extern code image_item_info_t img_input_ypbpr1;  				
extern code image_item_info_t img_input_pc;  					
extern code image_item_info_t img_input_pc1;  					
extern code image_item_info_t img_input_dvi;  					
extern code image_item_info_t img_input_dvi1;  					
extern code image_item_info_t img_input_hdmi;  					
extern code image_item_info_t img_input_hdmi1;  				
extern code image_item_info_t img_input_ext;  					
extern code image_item_info_t img_input_ext1;  					
extern code image_item_info_t img_input_return;  					
extern code image_item_info_t img_input_return1;  					
extern code image_item_info_t img_audio_bg;			  			
extern code image_item_info_t img_system_bg_bottom;  			
extern code image_item_info_t img_system_bg_top;  				
extern code image_item_info_t img_system_touch;  				
extern code image_item_info_t img_system_touch1;  				
extern code image_item_info_t img_system_display; 				
extern code image_item_info_t img_system_display1; 				
extern code image_item_info_t img_system_btooth;  				
extern code image_item_info_t img_system_btooth1;  				
extern code image_item_info_t img_system_restore;  				
extern code image_item_info_t img_system_restore1;  			
extern code image_item_info_t img_system_sys_info;  			
extern code image_item_info_t img_system_sys_info1;  			
extern code image_item_info_t img_gps_bg;			  			
extern code image_item_info_t img_phone_bg;	
extern code image_item_info_t img_phone_00; 		
extern code image_item_info_t img_phone_01;		
extern code image_item_info_t img_phone_10; 		
extern code image_item_info_t img_phone_11; 		
extern code image_item_info_t img_phone_20; 		
extern code image_item_info_t img_phone_21; 		
extern code image_item_info_t img_phone_30; 		
extern code image_item_info_t img_phone_31; 		
extern code image_item_info_t img_phone_40; 		
extern code image_item_info_t img_phone_41; 		
extern code image_item_info_t img_phone_50; 		
extern code image_item_info_t img_phone_51; 		
extern code image_item_info_t img_phone_60; 		
extern code image_item_info_t img_phone_61; 		
extern code image_item_info_t img_phone_70; 		
extern code image_item_info_t img_phone_71; 		
extern code image_item_info_t img_phone_80; 		
extern code image_item_info_t img_phone_81; 		
extern code image_item_info_t img_phone_90; 		
extern code image_item_info_t img_phone_91; 		
extern code image_item_info_t img_phone_star0; 		
extern code image_item_info_t img_phone_star1; 		
extern code image_item_info_t img_phone_sharp0; 	
extern code image_item_info_t img_phone_sharp1; 	
extern code image_item_info_t img_phone_dial0; 		
extern code image_item_info_t img_phone_dial1; 		
extern code image_item_info_t img_phone_up0; 		
extern code image_item_info_t img_phone_up1; 		
extern code image_item_info_t img_phone_down0; 		
extern code image_item_info_t img_phone_down1; 		
extern code image_item_info_t img_phone_left0; 		
extern code image_item_info_t img_phone_left1; 		
extern code image_item_info_t img_phone_right0; 	
extern code image_item_info_t img_phone_right1; 	
extern code image_item_info_t img_phone_check0; 	
extern code image_item_info_t img_phone_check1; 	
extern code image_item_info_t img_phone_help0; 		
extern code image_item_info_t img_phone_help1; 		
extern code image_item_info_t img_phone_dir0; 		
extern code image_item_info_t img_phone_dir1; 		
extern code image_item_info_t img_phone_set0; 		
extern code image_item_info_t img_phone_set1; 		
extern code image_item_info_t img_phone_msg0; 		
extern code image_item_info_t img_phone_msg1; 		
extern code image_item_info_t img_phone_menu0; 		
extern code image_item_info_t img_phone_menu1; 		
extern code image_item_info_t img_carinfo_bg;			  		
extern code image_item_info_t img_demo_bg;  					
extern code image_item_info_t img_demo_grid;  					
extern code image_item_info_t img_demo_grid1;  					
extern code image_item_info_t img_demo_rose;  					
extern code image_item_info_t img_demo_rose1;  	 				
extern code image_item_info_t img_demo_ani;						
extern code image_item_info_t img_demo_ani1;					
extern code image_item_info_t img_demo_palette;					
extern code image_item_info_t img_demo_palette1; 				
extern code image_item_info_t img_demo_demoA;					
extern code image_item_info_t img_demo_demoA1;					
extern code image_item_info_t img_demo_demoB;					
extern code image_item_info_t img_demo_demoB1;	 				
extern code image_item_info_t img_touch_bg;  					
extern code image_item_info_t img_touch_bg_end;					
extern code image_item_info_t img_touch_button;  				
extern code image_item_info_t img_touch_button1;  				
extern code image_item_info_t img_btooth_bg;  					
extern code image_item_info_t img_yuv_menu_bg;  				
extern code image_item_info_t img_yuv_bright;  					
extern code image_item_info_t img_yuv_bright1;  				
extern code image_item_info_t img_yuv_contrast;  				
extern code image_item_info_t img_yuv_contrast1;  				
extern code image_item_info_t img_yuv_hue;  					
extern code image_item_info_t img_yuv_hue1;  					
extern code image_item_info_t img_yuv_saturate;  				
extern code image_item_info_t img_yuv_saturate1;  				
extern code image_item_info_t img_yuv_sharp;  					
extern code image_item_info_t img_yuv_sharp1;  					
extern code image_item_info_t img_rgb_menu_bg;  				
extern code image_item_info_t img_rgb_bright;  					
extern code image_item_info_t img_rgb_bright1; 					
extern code image_item_info_t img_rgb_contrast;  				
extern code image_item_info_t img_rgb_contrast1;  				
extern code image_item_info_t img_rgb_color; 					
extern code image_item_info_t img_rgb_color1; 					
extern code image_item_info_t img_apc_menu_bg;  				
extern code image_item_info_t img_apc_bright;  					
extern code image_item_info_t img_apc_bright1; 					
extern code image_item_info_t img_apc_contrast;  				
extern code image_item_info_t img_apc_contrast1;  				
extern code image_item_info_t img_apc_color;
extern code image_item_info_t img_apc_color1;	  				
extern code image_item_info_t img_apc_position;  				
extern code image_item_info_t img_apc_position1; 				
extern code image_item_info_t img_apc_phase;  					
extern code image_item_info_t img_apc_phase1;  					
extern code image_item_info_t img_apc_pclock;  					
extern code image_item_info_t img_apc_pclock1; 					
extern code image_item_info_t img_apc_autoadj;  				
extern code image_item_info_t img_apc_autoadj1;  				
extern code image_item_info_t img_apc_autocolor;  				
extern code image_item_info_t img_apc_autocolor1;  				
extern code image_item_info_t img_hdmi_menu_bg;  				
extern code image_item_info_t img_hdmi_mode;  					
extern code image_item_info_t img_hdmi_mode1;  					
extern code image_item_info_t img_hdmi_setting;  				
extern code image_item_info_t img_hdmi_setting1;  				
extern code image_item_info_t img_display_bg;  					
extern code image_item_info_t img_display_aspect;  				
extern code image_item_info_t img_display_aspect1; 				
extern code image_item_info_t img_display_osd;  				
extern code image_item_info_t img_display_osd1;  				
extern code image_item_info_t img_display_flip;  				
extern code image_item_info_t img_display_flip1;  				
extern code image_item_info_t img_display_backlight;  			
extern code image_item_info_t img_display_backlight1;  			
extern code image_item_info_t img_display_resolution;  			
extern code image_item_info_t img_display_resolution1;  		
extern code image_item_info_t img_osd_bg;  						
extern code image_item_info_t img_osd_timer;  					
extern code image_item_info_t img_osd_timer1;  					
extern code image_item_info_t img_osd_trans;  					
extern code image_item_info_t img_osd_trans1;  					
extern code image_item_info_t img_dialog_ok;		  			
extern code image_item_info_t img_dialog_ok1;	  				
extern code image_item_info_t img_dialog_cancel;  				
extern code image_item_info_t img_dialog_cancel1;  				
extern code image_item_info_t img_autoadj_bg;  					
extern code image_item_info_t img_autocolor_bg;  				
extern code image_item_info_t img_flip_bg;  					
extern code image_item_info_t img_sysrestore_bg;  				
extern code image_item_info_t img_sysinfo_bg;  
extern code image_item_info_t img_resolution_bg; 			
extern code image_item_info_t img_slide_bg;  					
extern code image_item_info_t img_slide3_bg;  					
extern code image_item_info_t img_slide_gray;  					
extern code image_item_info_t img_slide_red;  					
extern code image_item_info_t img_slide_left;  					
extern code image_item_info_t img_slide_left1;  				
extern code image_item_info_t img_slide_right;  				
extern code image_item_info_t img_slide_right1;  				
extern code image_item_info_t img_slide_backlight;  			
extern code image_item_info_t img_slide_bright;  				
extern code image_item_info_t img_slide_clock;  				
extern code image_item_info_t img_slide_contrast;  				
extern code image_item_info_t img_slide_rgb;  					
extern code image_item_info_t img_slide_hue;  					
extern code image_item_info_t img_slide_phase;  				
extern code image_item_info_t img_slide_saturate;  				
extern code image_item_info_t img_slide_sharp;  				
extern code image_item_info_t img_slide_timer;  				
extern code image_item_info_t img_slide_trans;  				
extern code image_item_info_t img_position_bg;  				
extern code image_item_info_t img_position_box_gray;  			
extern code image_item_info_t img_position_box_red;  			
extern code image_item_info_t img_position_up;  				
extern code image_item_info_t img_position_down;  				
extern code image_item_info_t img_position_left;  				
extern code image_item_info_t img_position_right;  				
extern code image_item_info_t img_popup_aspect_bg;  			
extern code image_item_info_t img_popup_aspect_normal; 			
extern code image_item_info_t img_popup_aspect_normal1; 		
extern code image_item_info_t img_popup_aspect_normal_select;	
extern code image_item_info_t img_popup_aspect_zoom;  			
extern code image_item_info_t img_popup_aspect_zoom1;  			
extern code image_item_info_t img_popup_aspect_zoom_select;		
extern code image_item_info_t img_popup_aspect_full;  			
extern code image_item_info_t img_popup_aspect_full1;  			
extern code image_item_info_t img_popup_aspect_full_select;		
extern code image_item_info_t img_popup_aspect_pano;  			
extern code image_item_info_t img_popup_aspect_pano1;  			
extern code image_item_info_t img_popup_aspect_pano_sel;		
extern code image_item_info_t img_dvi_mode_bg;  				
extern code image_item_info_t img_dvi_mode_24bit;  				
extern code image_item_info_t img_dvi_mode_24bit1;  			
extern code image_item_info_t img_dvi_mode_16bit;  				
extern code image_item_info_t img_dvi_mode_16bit1;  			
extern code image_item_info_t img_dvi_mode_select24;  			
extern code image_item_info_t img_dvi_mode_select16;  			
extern code image_item_info_t img_hdmi_mode_bg;  				
extern code image_item_info_t img_hdmi_mode_pc;  				
extern code image_item_info_t img_hdmi_mode_pc1;  				
extern code image_item_info_t img_hdmi_mode_tv;  				
extern code image_item_info_t img_hdmi_mode_tv1;  				
extern code image_item_info_t img_hdmi_mode_selectPC;  			
extern code image_item_info_t img_hdmi_mode_selectTV;  			
extern code image_item_info_t img_wait;

//-----------------------------------
//main page
//-----------------------------------
//	video   audio  system
//	gps     phone  carinfo
//
//                demo exit
//-----------------------------------
#define MENU_MAIN_BG		0
#define MENU_MAIN_DEMO		1
#define MENU_MAIN_EXIT		2
#define MENU_MAIN_VIDEO		3
#define MENU_MAIN_AUDIO		4
#define MENU_MAIN_SYSTEM	5
#define MENU_MAIN_GPS		6
#define MENU_MAIN_PHONE		7
#define MENU_MAIN_INFO		8
extern code struct menu_sitem_info_s menu_main_page_items[];
extern struct menu_page_info_s menu_main_page;			

//-----------------------------------
//video input page
//-----------------------------------
//	cvbs svideo Ypbpr rgb dvi hdmi
//
//					return setting exit
//-----------------------------------
//description
//background: use 8bit RLE. 128 Color. Use SOSD_WIN0
//            color 1: transparent color. normally color 0 is a black.
#ifdef SUPPORT_DVI
#define MENU_SRC_BG			0
#define MENU_SRC_RETURN		2
#define MENU_SRC_SETTING	3
#define MENU_SRC_EXIT		4
#define MENU_SRC_CVBS		5
#define MENU_SRC_SVIDEO		6
#define MENU_SRC_YPBPR		7
#define MENU_SRC_PC			8
#define MENU_SRC_DVI		9
#define MENU_SRC_HDMI		10
#define MENU_SRC_BT656		11

#define MENU_SRC_ITEMS		7
#else
#define MENU_SRC_BG			0
#define MENU_SRC_RETURN		2
#define MENU_SRC_SETTING	3
#define MENU_SRC_EXIT		4
#define MENU_SRC_CVBS		5
#define MENU_SRC_SVIDEO		6
#define MENU_SRC_YPBPR		7
#define MENU_SRC_PC			8
#define MENU_SRC_HDMI		9
#define MENU_SRC_BT656		10

#define MENU_SRC_ITEMS		6
#endif



extern code struct menu_sitem_info_s menu_input_page_items[];
extern struct menu_page_info_s menu_input_page;	//auto focus	

//-----------------------------------
//audio page
//-----------------------------------
#define MENU_AUDIO_RETURN		1
extern code struct menu_sitem_info_s menu_audio_page_items[];
extern struct menu_page_info_s menu_audio_page;		


//-----------------------------------
// system page
//-----------------------------------
//	touch display restore sysinfo				
//
//					   return exit
//-----------------------------------
//description
#define MENU_SYSTEM_RETURN		2
#define MENU_SYSTEM_EXIT		3
#define MENU_SYSTEM_TOUCH		4
#define MENU_SYSTEM_DISPLAY		5
#define MENU_SYSTEM_BTOOTH		6
#define MENU_SYSTEM_RESTORE		7
#define MENU_SYSTEM_SYSINFO		8
extern code struct menu_sitem_info_s menu_system_page_items[];
extern struct menu_page_info_s menu_system_page;		

//-----------------------------------
//GPS page
//-----------------------------------
#define MENU_GPS_RETURN		1
extern code struct menu_sitem_info_s menu_gps_page_items[];	  
extern struct menu_page_info_s menu_gps_page;		

//-----------------------------------
//PHONE page
//-----------------------------------
#define MENU_PHONE_RETURN		1
extern code struct menu_sitem_info_s menu_phone_page_items[];
extern struct menu_page_info_s menu_phone_page;		

//-----------------------------------
//CARINFO Page
//-----------------------------------
#define MENU_CARINFO_RETURN		1
extern code struct menu_sitem_info_s menu_carinfo_page_items[];
extern struct menu_page_info_s menu_carinfo_page;		

//-----------------------------------
//DEMO page
//-----------------------------------
extern code struct menu_sitem_info_s menu_demo_page_items[];	  
extern struct menu_page_info_s menu_demo_page;	


//-----------------------------------
//TOUCH Page
//-----------------------------------
#define MENU_TOUCH_OK	2
extern code struct menu_sitem_info_s menu_touch_page_items[];
extern struct menu_page_info_s menu_touch_page;		


//-----------------------------------
// YUV LIST Page
//-----------------------------------
// Brightness
// Contrast
// Hue
// Saturation
// Sharpness
//					return home exit
//-----------------------------------
//description
//use global input
//background: use 8bit RLE. 128 Color. Use SOSD_WIN0
//            color 1: transparent color. normally color 0 is a black.
extern code struct menu_sitem_info_s menu_yuv_list_page_items[];
extern struct menu_page_info_s menu_yuv_list_page;		

//manu->input->setting with RGB/HDMI->color
extern code struct menu_sitem_info_s menu_rgb_list_page_items[];
extern struct menu_page_info_s menu_rgb_list_page;		

//-----------------------------------
// aPC List Page
//-----------------------------------
//					return setting exit
//-----------------------------------
//description
//use global input
//background: use 8bit RLE. 128 Color. Use SOSD_WIN0
//            color 1: transparent color. normally color 0 is a black.
extern code struct menu_sitem_info_s menu_apc_list_page_items[];
extern struct menu_page_info_s menu_apc_list_page;		

//-----------------------------------
// hdmi list page
//-----------------------------------
extern code struct menu_sitem_info_s menu_hdmi_list_page_items[];
extern struct menu_page_info_s menu_hdmi_list_page;		

//-----------------------------------
// hdmipc list page
//-----------------------------------
extern code struct menu_sitem_info_s menu_hdmipc_list_page_items[];
extern struct menu_page_info_s menu_hdmipc_list_page;		



//-----------------------------------
// display page
//-----------------------------------
//description
extern code struct menu_sitem_info_s menu_display_page_items[];
extern struct menu_page_info_s menu_display_page;		

//-----------------------------------
// OSD List page
//-----------------------------------
//description
extern code struct menu_sitem_info_s menu_osd_page_items[];
extern struct menu_page_info_s menu_osd_page;		

//-----------------------------------
// DVI Mode dialog
//-----------------------------------
#define MENU_DVI_MODE_OK		1
#define MENU_DVI_MODE_24BIT		3
#define MENU_DVI_MODE_16BIT		4

#define MENU_DVI_MODE_ITEMS		2
extern code struct menu_sitem_info_s menu_dvi_mode_page_items[];
extern struct menu_page_info_s menu_dvi_mode_page;		

//-----------------------------------
// hdmi mode dialog
//-----------------------------------
#define MENU_HDMI_MODE_PC		3
#define MENU_HDMI_MODE_TV		4
#define MENU_HDMI_MODE_OK		1
extern code struct menu_sitem_info_s menu_hdmi_mode_page_items[];
extern struct menu_page_info_s menu_hdmi_mode_page;		

//-----------------------------------
// autoadj dialog
//-----------------------------------
//description
#define MENU_AUTOADJ_OK		1
#define MENU_AUTOADJ_WAIT	3
extern code struct menu_sitem_info_s menu_autoadj_page_items[];	  
extern struct menu_page_info_s menu_autoadj_page;		

//-----------------------------------
// autocolor dialog
//-----------------------------------
#define MENU_AUTOCOLOR_OK		1
#define MENU_AUTOCLOCK_WAIT		3
extern code struct menu_sitem_info_s menu_autocolor_page_items[];	  
extern struct menu_page_info_s menu_autocolor_page;		

//-----------------------------------
// aspect dialog
//-----------------------------------
//description
#define MENU_ASPECT_OK			1
#define MENU_ASPECT_NORMAL		3
#define MENU_ASPECT_ZOOM		4
#define MENU_ASPECT_FULL		5
#define MENU_ASPECT_PANO		6

extern code struct menu_sitem_info_s menu_aspect_page_items[];
extern struct menu_page_info_s menu_aspect_page;		

//-----------------------------------
//mirror dialog
//-----------------------------------
//description
#define MENU_FLIP_OK	1
extern code struct menu_sitem_info_s menu_mirror_page_items[];
extern struct menu_page_info_s menu_mirror_page;		

//-----------------------------------
// disp resolution dialog
//-----------------------------------
#define MENU_DISP_RESOLUTION_OK		1
extern code struct menu_sitem_info_s menu_disp_resolution_page_items[];
extern struct menu_page_info_s menu_disp_resolution_page;		


//-----------------------------------
// restore dialog
//-----------------------------------
//description
#define MENU_RESTORE_OK		1
extern code struct menu_sitem_info_s menu_restore_page_items[];
extern struct menu_page_info_s menu_restore_page;		

//-----------------------------------
// system info dialog
//-----------------------------------
//description
#define MENU_SYS_INFO_OK		1
extern code struct menu_sitem_info_s menu_sys_info_page_items[];	  
extern struct menu_page_info_s menu_sys_info_page;		


//-----------------------------------
// slider dialog
//-----------------------------------
// slider popup MENU
//manu->input->setting with CVBS/SVideo->bright->slide
//manu->input->setting with CVBS/SVideo->contrast->slide
//manu->input->setting with CVBS/SVideo->hue->slide
//manu->input->setting with CVBS/SVideo->saturation->slide
//manu->input->setting with CVBS/SVideo->sharpness->slide
//manu->input->setting with RGB/HDMI->display->sharpness
//manu->input->setting with RGB/HDMI->display->pixel clock
//manu->input->setting with RGB/HDMI->display->phase
//manu->input->setting with RGB/HDMI->color->contrast
//manu->input->setting with RGB/HDMI->color->hue
//manu->input->setting with RGB/HDMI->color->bright
//manu->input->setting with RGB/HDMI->color->saturate
//manu->input->setting with DVI->contrast
//manu->input->setting with DVI->hue
//manu->input->setting with DVI->bright
//manu->input->setting with DVI->sharpness
//-----------------------------------
//description
#define SLIDER_NIDDLE_HALF_WIDTH	20	  //BKTODO.

#define MENU_SLIDER_BAR_TITLE		1

#define MENU_SLIDER_BAR_OK			2
#define MENU_SLIDER_BAR_CANCEL		3

#define MENU_SLIDER_BAR_INPUT_GRAY	4
#define MENU_SLIDER_BAR_INPUT_RED	5
#define MENU_SLIDER_BAR_DIGIT		6 	//NotUsed

#define MENU_SLIDER_LEFT			6
#define MENU_SLIDER_RIGHT			7

extern code struct menu_sitem_info_s menu_slider_page_items[];
extern code struct menu_fitem_info_s menu_slider_text_items[];
extern struct menu_page_info_s menu_slider_page;

//-----------------------------------
// slider3 dialog
//-----------------------------------
#define SLIDE3_OK		2
#define SLIDE3_CANCEL	3
#define SLIDE3_1		5
#define SLIDE3_2		7
#define SLIDE3_3		9

#define SLIDE3_LEFT		10
#define SLIDE3_RIGHT	11
extern code struct menu_sitem_info_s menu_slider3_page_items[];
extern code struct menu_fitem_info_s menu_slider3_text_items[];
extern struct menu_page_info_s menu_slider3_page;

//-----------------------------------
// position dialog
//-----------------------------------
//description
#define MENU_POSITION_INPUT_GRAY		4
#define MENU_POSITION_INPUT_RED			3
#define MENU_POSITION_OK				1
extern code struct menu_sitem_info_s menu_position_page_items[];	  
extern struct menu_page_info_s menu_position_page;		

//-----------------------------------
// longuage dialog
//-----------------------------------
//description
//code struct menu_sitem_info_s menu_lang_info[] = {
//	{ proc_lang_bg,			300,200,ALIGN_TOPLEFT,	image_lang_bg,	0xC0, SOSD_WIN2, 0,0,0,0},
//	{ proc_lang_eng,		300,200,ALIGN_TOPLEFT,	image_lang_eng,	0xC0, SOSD_WIN3, 0,0,0,0},
//	{ proc_lang_kor,		450,350,ALIGN_TOPLEFT,	image_lang_kor,	0xC0, SOSD_WIN4, 0,0,0,0},
//	{ proc_lang_jpn,		450,350,ALIGN_TOPLEFT,	image_lang_jpn,	0xC0, SOSD_WIN4, 0,0,0,0},
////	{ proc_lang_chn,		450,350,ALIGN_TOPLEFT,	img_ok,	0xC0, SOSD_WIN4, 0,0,0,0},
//	{ proc_lang_ok,			450,350,ALIGN_TOPLEFT,	img_ok,	0xC0, SOSD_WIN4, 0,0,0,0},
//};




//test main menu images
extern code image_item_info_t img_main_test1;    
extern code image_item_info_t img_main_test2;    
extern code image_item_info_t img_main_test3;    

//=========================
// stack
//=========================
void push_menu_stack_level(void);
void pop_menu_stack_level(void);
#define push_menu()	push_menu_stack_level()
//#define pop_menu()	proc_return_icon()
BYTE MenuGetLevel(void);

//=========================
// other init
//=========================
//void InitLogo(void);
void InitLogo1(void);
void RemoveLogoWithWait(BYTE fPowerUpBoot);
void RemoveLogo(void);
BYTE getNoSignalLogoStatus(void);
void TestMainMenuImage(BYTE type);


//=========================
// DRAW
//=========================
void MenuDrawCurrImage(BYTE use1,BYTE item);
void MenuDrawScrollImage(BYTE item_no, BYTE direction);
void MenuTurnOffCurrImage(BYTE item);
void MenuDrawDeactivatedItemImage(BYTE sosd_win, BYTE item);
void proc_draw_digit(void);
void FontOsdWinPuts2(struct menu_sitem_info_s *item_info, /*BYTE winno, WORD sx, WORD sy, BYTE align, */ BYTE *str);
void MenuFontOsdItemString(BYTE index, BYTE *str);

// background
void proc_menu_bg(void);

//=========================
// KEY & TOUCH
//=========================
void MenuKeyInput(BYTE key);
extern BYTE MenuCheckItemPosition(int x, int y);
extern void MenuCheckTouchInput(BYTE TscStatus, int x,int y);

//=========================
// NAVI ICON
//=========================
void proc_demo_icon(void);
void proc_exit_icon(void);
void proc_return_icon(void);
void proc_home_icon(void);

//=========================
// SLIDE DIALOG
//=========================
void InitSlideCtrl(image_item_info_t *title, void *pGet, void *pVary, void *pSet, void *pSave);
void InitSlide3Ctrl(image_item_info_t *title, void *pGet, void *pVary, void *pSet, void *pSave);
void proc_slider_bg(void);
void proc_slider3_bg(void);
void proc_slider_ctrl(void);
void proc_slider_left_tsc(void);
void proc_slider_right_tsc(void);
void proc_slider_ok(void);
void proc_slider_undo(void);
BYTE MenuIsSlideMode(void);

//=========================
// POSITION DIALOG
//=========================
void proc_position_bg(void);
void proc_position_box(void);
void proc_position_ok(void);
void proc_position_undo(void);
void proc_position_left_tsc(void);
void proc_position_right_tsc(void);
void proc_position_up_tsc(void);
void proc_position_down_tsc(void);

void proc_position_H_digit(void);
void proc_position_V_digit(void);

//=========================
// DIALOG 
//=========================
void proc_autoadj_bg(void);
void proc_autoadj_ok(void);
void proc_autoadj_cancel(void);
//------------------------
void proc_autocolor_bg(void);
void proc_autocolor_ok(void);
void proc_autocolor_cancel(void);
//------------------------
void proc_restore_bg(void);
void proc_restore_ok(void);
void proc_restore_undo(void);
//------------------------
void proc_info_bg(void);
void proc_info_ok(void);
//------------------------
void proc_flip_bg(void);
void proc_flip_ok(void);
void proc_flip_undo(void);
//------------------------
void proc_disp_resolution_bg(void);
void proc_disp_resolution_ok(void);

//=========================
// NOTE DIALOG
//=========================
void proc_dvi_mode_bg(void);
void proc_dvi_mode_24bit(void);
void proc_dvi_mode_16bit(void);
void proc_dvi_mode_ok(void);
void proc_dvi_mode_cancel(void);
//------------------------
void proc_hdmi_mode_bg(void);
void proc_hdmi_mode_pc(void);
void proc_hdmi_mode_tv(void);
void proc_hdmi_mode_ok(void);
void proc_hdmi_mode_cancel(void);
//------------------------
void proc_aspect_bg(void);
void proc_aspect_normal(void);
void proc_aspect_zoom(void);
void proc_aspect_full(void);
void proc_aspect_pano(void);
void proc_aspect_ok(void);
void proc_aspect_cancel(void);

//=========================
// LIST MENU
//=========================
void proc_menu_list_return(void);
//------------------------
void proc_yuv_list_bg(void);
void proc_yuv_list_bright(void);
void proc_yuv_list_contrast(void);
void proc_yuv_list_hue(void);
void proc_yuv_list_saturate(void);
void proc_yuv_list_sharp(void);
//------------------------
void proc_rgb_list_bg(void);
void proc_rgb_list_bright(void);
void proc_rgb_list_contrast(void);
void proc_rgb_list_color(void);
//------------------------
void proc_apc_list_bg(void); 
void proc_apc_list_bright(void); 
void proc_apc_list_contrast(void);
void proc_apc_list_color(void); 
void proc_apc_list_position(void); 
void proc_apc_list_phase(void); 
void proc_apc_list_clock(void); 
void proc_apc_list_autoadj(void); 
void proc_apc_list_autocolor(void); 
//------------------------
void proc_hdmi_list_bg(void);
void proc_hdmi_list_mode(void);
void proc_hdmi_list_setting(void);
//------------------------
void proc_hdmipc_list_bg(void);
void proc_hdmipc_list_bright(void);
void proc_hdmipc_list_contrast(void);
void proc_hdmipc_list_phase(void);
//------------------------
void proc_osd_bg(void);
void proc_osd_lang(void);
void proc_osd_time(void);
void proc_osd_trans(void);


//=========================
// PAGE MENU
//=========================
void proc_main_bg(void);
void proc_main_input(void);
void proc_main_audio(void);
void proc_main_system(void);
void proc_main_gps(void);
void proc_main_phone(void);
void proc_main_info(void);
//------------------------
void proc_input_bg(void);
void proc_input_cvbs(void);
void proc_input_svideo(void);
void proc_input_ypbpr(void);
void proc_input_pc(void);
void proc_input_dvi(void);
void proc_input_hdmi(void);
void proc_input_bt656(void);
void proc_input_setting(void);
//------------------------
void proc_audio_bg(void);
//------------------------
void proc_system_bg(void);
void proc_system_touch(void);
void proc_system_display(void);
void proc_system_btooth(void);
void proc_system_restore(void);
void proc_system_info(void);
BYTE MenuIsSystemPage(void);
//------------------------
void proc_gps_bg(void);
//------------------------
void proc_phone_bg(void);
void proc_phone_return(void);
void proc_phone_1(void);
void proc_phone_2(void);
void proc_phone_3(void);
void proc_phone_4(void);
void proc_phone_5(void);
void proc_phone_6(void);
void proc_phone_7(void);
void proc_phone_8(void);
void proc_phone_9(void);
void proc_phone_star(void);
void proc_phone_0(void);
void proc_phone_sharp(void);
void proc_phone_dial(void); 
void proc_phone_up(void);
void proc_phone_down(void);
void proc_phone_check(void);
void proc_phone_left(void);
void proc_phone_right(void);
void proc_phone_help(void);
void proc_phone_dir(void);
void proc_phone_set(void);
void proc_phone_msg(void);
void proc_phone_menu(void);

//------------------------
void proc_carinfo_bg(void);
//------------------------
void proc_demo_bg(void);
void proc_grid_demo_start(void);
void proc_pigeon_demo_start(void);
void proc_rose_demo_start(void);
void proc_test1_demo_start(void);
void proc_wait_demo_start(void);
//------------------------
void proc_wait_menu_start(void);
//------------------------
void proc_touch_bg(void);
void proc_touch_bg_end(void);
void proc_touch_button1(void);
void proc_touch_button2(void);
void proc_touch_button3(void);
void proc_touch_button4(void);
void proc_touch_button5(void);
void proc_touch_ok(void);
void proc_touch_cancel(void);
BYTE MenuIsTouchCalibMode(void);
void MenuTouchCalibStart(void);
//------------------------
void proc_display_bg(void);			
void proc_display_aspect(void);		
void proc_display_osd(void);			
void proc_display_flip(void);			
void proc_display_backlight(void);	
void proc_display_resolution(void);	
//void proc_display_return(void);
//------------------------

//=========================
// REMOVED
//=========================
//------------------------
//void proc_rgb_image_bg(void);
//void proc_rgb_color(void);
//void proc_rgb_display(void);
//void proc_rgb_image_return(void);

//------------------------
//void proc_rgb_color_bg(void);
//void proc_rgb_color_bright(void);
//void proc_rgb_color_contrast(void);
//void proc_rgb_color_hue(void);	 		//will be removed
//void proc_rgb_color_saturate(void);		//will be removed

//------------------------
//void proc_rgb_display_bg(void);
//void proc_rgb_display_position(void);
//void proc_rgb_display_phase(void);
//void proc_rgb_display_clock(void);
//void proc_rgb_display_sharp(void);		//will be removed
//void proc_rgb_display_auto(void);

//void proc_dvi_image_hue(void);
//void proc_dvi_image_sharp(void);
//void proc_dvi_image_return(void);

//void proc_ie_image_bg(void);
//void proc_ie_image_contrast(void);
//void proc_ie_image_hue(void);
//void proc_ie_image_saturate(void);
//void proc_ie_image_sharp(void);		
//void proc_ie_image_backlight(void);
//void proc_ie_image_bright(void);
//void proc_ie_image_more(void);
//
//void proc_more_bg(void);
//void proc_more_aspect(void);
//void proc_more_position(void);
//void proc_more_osd(void);
//void proc_more_flip(void);
//void proc_more_restore(void);
//void proc_more_backlight(void);
//
//void proc_lang_bg(void);
//void proc_lang_eng(void);
//void proc_lang_kor(void);
//void proc_lang_jpn(void);
//void proc_lang_chn(void);
//void proc_lang_ok(void);


//=========================
// START / END
//=========================
void MenuStart(void);
void MenuEnd(void);
void MenuQuitMenu(void);


#endif //.__TW8835_MENU_B_H__
