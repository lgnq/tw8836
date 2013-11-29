/**
 * @file
 * Monitor_MENU.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	Interface between TW_Terminal2 and Firmware.
*/
//*****************************************************************************
//
//								Monitor_Menu.c
//
//*****************************************************************************
//
//
#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "Global.h"
#include "CPU.h"
#include "printf.h"
#include "util.h"
#include "monitor_MENU.h"
#include "reg_debug.h"

#include "i2c.h"
//#include "menu8835.h"
#include "Remo.h"

#include "SOsd.h"
#include "FOsd.h"
#include "SOsdMenu.h"

#include "Demo.h"
#include "SpiFlashMap.h"

#ifdef SUPPORT_FOSD_MENU
#include "FOsdMenu.h"
#endif

//temp..please move to .h file and remove it
extern 		void MenuCheckTouchInput(BYTE TscStatus,int x,int y);
extern		void proc_home_menu_start(void);

extern void DumpFontOsdPalette(void);
extern void Test16x32FontSet(void);
extern void TestUpper256Char(void);
extern void TestMultiBPP4(void);
extern void TestNum1BPP3BPP(void);

extern void TaskSetGridCmd(BYTE cmd);
extern BYTE TaskGetGrid(void);


//extern XDATA BYTE Task_Grid_on;
extern void MenuKeyInput(BYTE key);

//extern TestMainMenuImage(BYTE type);



//=============================================================================
//
//=============================================================================
/*
	menu start
	menu [up|down|right|left]
	menu enter
	menu exit
*/
void MonitorMenu(void)
{
//	int x, y;
	if(!stricmp( argv[1], "?")) {
		Puts("\n  === Help for MENU command ===");
		Puts("\nmenu start					;start main menu");
		Puts("\nmenu [up|down|right|left]	;menu arrow key");
		Puts("\nmenu enter					;enter key");
		Puts("\nmenu exit					;exit key");
		return;
	}

	if(argc != 3)	{
		//Puts("\nmenu x y");

		if(TaskGetGrid()) {
			if(!stricmp( argv[1], "up")) {
				TaskSetGridCmd(NAVI_KEY_UP);
			}
			else if(!stricmp( argv[1], "down")) {
				TaskSetGridCmd(NAVI_KEY_DOWN);
			}
			else if(!stricmp( argv[1], "left")) {
				TaskSetGridCmd(NAVI_KEY_LEFT);
			}
			else if(!stricmp( argv[1], "right")) {
				TaskSetGridCmd(NAVI_KEY_RIGHT);
			}
			else if(!stricmp( argv[1], "enter")) {
				TaskSetGridCmd(NAVI_KEY_ENTER);
			}
			return;
		}
		if(MenuGetLevel()) {
			if(!stricmp( argv[1], "up")) {
				MenuKeyInput(NAVI_KEY_UP);
			}
			else if(!stricmp( argv[1], "down")) {
				MenuKeyInput(NAVI_KEY_DOWN);
			}
			else if(!stricmp( argv[1], "left")) {
				MenuKeyInput(NAVI_KEY_LEFT);
			}
			else if(!stricmp( argv[1], "right")) {
				MenuKeyInput(NAVI_KEY_RIGHT);
			}
			else if(!stricmp( argv[1], "enter")) {
				MenuKeyInput(NAVI_KEY_ENTER);
			}
		}


		if(!stricmp( argv[1], "start" ))  {
			Printf("\ncall MenuStart");
			//proc_home_menu_start();
			MenuStart();
		}
		else if(!stricmp( argv[1], "exit" ))  {
			Printf("\ncall MenuEnd");
			MenuEnd();
		}
		else if(!stricmp( argv[1], "test1")) {
			//draw 1bpp FOSD Palette
			//environmemt
			//	w ff 3
			//	b 8a 0c 66 40
			//	fosd dnfont 0
			//
			DumpFontOsdPalette();
		}
		else if(!stricmp( argv[1], "test2")) {
			//FOSD FIFO test.
			Test16x32FontSet();
		}
		else if(!stricmp( argv[1], "test3")) {
			TestUpper256Char();
		}
		else if(!stricmp( argv[1], "test4")) {
			TestMultiBPP4();
		}
		else if(!stricmp( argv[1], "test5")) {
			TestNum1BPP3BPP();
		}
		else if(!stricmp( argv[1], "main1")) {
			TestMainMenuImage(1);
		}
		else if(!stricmp( argv[1], "main2")) {
			TestMainMenuImage(2);
		}
		else if(!stricmp( argv[1], "main3")) {
			TestMainMenuImage(3);
		}


		return;
	}
//	x=a2h(argv[1]);
//	y=a2h(argv[2]);
//   	Printf("\n ECHO: menu x:%d y:%d", x,y);
//	MenuCheckTouchInput(x,y);
}

//=============================================================================
//
//=============================================================================
//FONT OSD
void MonitorFOsd(void)
{
	DECLARE_LOCAL_page

	if (!stricmp(argv[1], "?"))
	{
		Puts("\n  === Help for FOSD command ===");
		Puts("\nFOSD BPP3			;calculate 3BPP alpha order");
		Puts("\nFOSD BPP2	[winno]	;draw 2BPP intersil icon");
		Puts("\nFOSD lutd			;dump 64 lut value");
		Puts("\nFOSD lutw	dat		;overwrite all 64 LUT with value");
		Puts("\nFOSD dnlut  [0|1|2]	;download LUT");
		Puts("\nFOSD info			;");
		Puts("\nFOSD dnfont	[n]		;download font");
		Puts("\nFOSD fontd			;dump downloaded font");
	}
	//=============================================
	// font
	//	font info
	// 	download font
	//	dump font
	// 
	//palette
	//	palette dump
	// 	download palette
	//
	//osdram
	//	
	//

	//-----------------------------------------------------
	// print Font information
	else if (!stricmp(argv[1], "info"))
	{
		extern void FontInfoByNum(BYTE FontMode);
		
		//FOsdMon_info();
		Printf("\nFont");
		Printf("\n\t0: default_font 12x18");
		Printf("\n\t1: consolas 16x26");
		Printf("\n\t2: consolas graynum 16x26");
		Printf("\n\t3: con+graynum 16x26");
		Printf("\n\t4: def+kor 16x26");
		FontInfoByNum(0);
		FontInfoByNum(1);
		FontInfoByNum(2);
		FontInfoByNum(3);
	}
	//-----------------------------------------------------
	// download font
	else if (!stricmp(argv[1], "dnfont"))
	{
		BYTE num;
		WORD loc;
		if(argc < 3) {
			num = 0;
			loc = 0;
		}
		else {
			num = a2h(argv[2]);
			if(argc < 4) {
				loc = 0;
			}
			else {
				loc = a2h(argv[3]);
			}
		}
		InitFontRamByNum(num, loc);
	}
	//-----------------------------------------------------
	// dump font.
	else if(!stricmp( argv[1], "fontd" )) {
		extern void DumpFont(void);
		DumpFont();
	}
	//-----------------------------------------------------
	// dump palette.
	else if(!stricmp( argv[1], "lutd")) {
		//dump palette table
		BYTE winno;

		winno=3;
		if(argc == 3) {
			winno = a2h(argv[2]);
			if(winno > 3)
				winno=3;
		}
		ReadTW88Page(page);
		FOsdDumpPalette(winno);
		WriteTW88Page(page);
	}
	else if(!stricmp( argv[1], "lutw")) {
		//fosd lutw 0 ffff  0
		BYTE loc;
		WORD color;
		if ( argc < 3 ) {
			Printf("\nflutw need a WORDSIZE value");
		}
		else {
			loc = a2h(argv[2]);
			color = a2h(argv[3]);
			FOsdSetPaletteColor(loc, color, 1, 0);
		}
	}
	else if(!stricmp( argv[1], "dnlut")) {
		//download palette table
		BYTE lut_num;

		lut_num = 0;
		if(argc == 3) {
			lut_num = a2h(argv[2]);
		}
		FOsdSetDefPaletteColor(lut_num);


	}
	else if(!stricmp( argv[1], "cpwin" )) {
#ifdef MODEL_TW8836
		/*
		TW8836 has 8 FontOsdWindow. FW uses 4 windows for MENU.
		To test WIN5,6,7,8 copy WIN1,2,3,4 register to WIN5,6,7,8 
		and then adjust start x, start y.
		*/
		BYTE src,dest,i;

		if(argc >=4) {	
			src = a2h(argv[2]);
			dest = a2h(argv[3]);
			if(src >=8 || dest >=8)
				return;
			WriteTW88Page(3);
			for(i=0; i < 0x10; i++) {
				WriteTW88(REG310+dest*0x10+i, ReadTW88(REG310+src*0x10+i));
			}
		}
#endif
	}
	else if ( !stricmp( argv[1], "test1" )) {
		//extern void FOsdDisplayInput(void);
		//FOsdDisplayInput();
		extern void FOsdTest_1(BYTE mode);
		FOsdTest_1(0);
	}
	else if ( !stricmp( argv[1], "test2" )) {
		extern void FOsdTest_1(BYTE mode);
		FOsdTest_1(1);
	}
	else if ( !stricmp( argv[1], "test3" )) {
#ifdef SUPPORT_FOSD_MENU
//		extern void FOsdMenuOpen(void);

		ReadTW88Page(page);
		//InitFontRam(0, &default_font,"def");
		InitFontRamByNum(FONT_NUM_DEF12X18, 0);
		FOsdRamSetFifo(ON, 1);
		FOsdMenuOpen();
		WriteTW88Page(page);
#endif
	}
	else if ( !stricmp( argv[1], "test4" )) {
#ifdef SUPPORT_FOSD_MENU
		//extern void FOsdDisplayInput(void);
		//FOsdDisplayInput();

		extern void FOsdDisplayInput(void);
//		extern BYTE CloseOSDMenu(void);
		extern void WriteStringToAddr(WORD addr, BYTE *str, BYTE cnt);
//		extern void OSDSelect(void);
		extern void DisplayPCInfo(BYTE CODE *ptr);
//		extern void OSDCursorMove(BYTE flag );



//		CloseOSDMenu();
		FOsdDisplayInput();
		WriteStringToAddr(0, "test", 4);
//		OSDSelect();
		DisplayPCInfo("CHECK 656 signal");
//		OSDCursorMove(0);
#endif


	}
	else if(!stricmp( argv[1], "BPP3")) {
		FOsdInitBpp3AlphaTable(1);
	}
	else if(!stricmp( argv[1], "BPP2")) {
		BYTE winno;

		winno=3;
		if(argc == 3) {
			winno = a2h(argv[2]);
			if(winno > 3)
				winno=3;
		}
//		FOsdIntersil(winno);
	}
	else
		Printf("\nInvalid command...");	
}

//=============================================================================
//
//=============================================================================
//SPI OSD
void MonitorSOsd(void)
{
	if (!stricmp(argv[1], "?"))
	{
		Puts("\n=== Help for SOSD command ===");
		Puts("\n\ton/off		;SOSD on/off");
		Puts("\n\t [rose|pigeon]");
		Puts("\n\t ??");
		Puts("\n\t lut img_n lut_n");
		Puts("\n\t img img_n lut_n win_n");
		Puts("\nAdd Your Test Function			;add comment");
 	}
	//-------<<on>>-----------------------------
	else if (!stricmp(argv[1], "on"))
	{
		SpiOsdEnable(ON);
	}
	//-------<<off>>-----------------------------
	else if (!stricmp(argv[1], "off"))
	{
		SpiOsdEnable(OFF);
	}
	//-------<<rose>>-----------------------------
	else if (!stricmp(argv[1], "rose"))
	{
		if (argc >= 3)
		{
			if (!stricmp(argv[2], "info"))
			{
				Printf("\nimg loc:%lx size:%lx lut loc:%lx size:%lx 400x400x10", ROSE_START, ROSE_LEN, ROSE_LUT_LOC, PIGEON_ROSE_LUT_LEN);
			}
			else
				Printf("\nInvalid command...");	
		}
		RoseDemo();
	}
	//-------<<pigeon>>-----------------------------
	else if (!stricmp(argv[1], "pigeon"))
	{
		if (argc >= 3)
		{
			if (!stricmp(argv[2], "info"))
			{
				Printf("\nimg loc:%lx size:%lx lut loc:%lx size:%lx 400x400x10",PIGEON_START,PIGEON_LEN, PIGEON_LUT_LOC,PIGEON_ROSE_LUT_LEN);
			}
			else
				Printf("\nInvalid command...");	
		}
		PigeonDemo();
	}
	//-------<<?? test image information>>-----------------------------
	else if (!stricmp(argv[1], "??"))
	{
		MonSOsdImgInfo();
	}
	//-------<<lut offset# addr#>>-----------------------------
	else if (!stricmp(argv[1], "lut"))
	{
		BYTE img_n;
		WORD lut;
		if (argc < 4)
			Printf("\nInvalid command...");		 
		else
		{
			img_n = a2i(argv[2]);
			lut   = a2i(argv[3]);
			MonOsdLutLoad(img_n, 3, lut); //I don't know winno. so tempolary assign 3.
		}	
	}
	//-------<<img0 win# lut#>>-----------------------------
	else if (!stricmp(argv[1], "img"))
	{
		BYTE img_n,winno;
		WORD lut;

		if (argc < 5)
			Printf("\nInvalid command...");		 
		else
		{
			img_n = a2i(argv[2]);
			lut   = a2i(argv[3]);
			winno = a2i(argv[4]);
			MonOsdImgLoad(img_n, winno, lut);
		}	
	}
	//-------<<img1 win# lut#>>-----------------------------
	else if (!stricmp(argv[1], "dnitem"))
	{
		BYTE img_n,winno;
		WORD lut;
		if (argc < 4) 
			Printf("\nInvalid command...");
		else
		{
			if(!stricmp( argv[3], "lut")) {
				img_n = a2i(argv[2]);
				winno = ReadTW88(REG009) >> 5;
				winno++;	
				lut = ReadTW88(REG009) & 0x1F;
				lut <<= 4;
				MonOsdLutLoad(img_n,winno, lut);
			} 
			else if(!stricmp( argv[3], "img")) {
				img_n = a2i(argv[2]);
				winno = ReadTW88(REG009) >> 5;
				winno++;	
				lut = ReadTW88(REG009) & 0x1F;
				lut <<= 4;
				MonOsdImgLoad(img_n,winno,lut);
			}
			else
				Printf("\nInvalid command...");	
		}
	}
	//-------<<sosd download lut>>-----------------------------
	else if(!stricmp( argv[1], "dnlut")) {
		//download palette table
		//format: sosd dnlut lut_offset image_address
		//step: read header	& download lut
		extern menu_image_header_t header_table;
		extern BYTE MenuReadRleHeader(DWORD spi_loc,struct RLE2_HEADER *header);
		extern void rle2_to_header(struct RLE2_HEADER *rle_header);
		#define MRLE_INFO_SIZE		0x10
		WORD lut_loc;
		BYTE lut_type;
		WORD nColor;
		DWORD image_loc;
		BYTE winno;		//TW8836 need.
		menu_image_header_t *header = &header_table;
		struct RLE2_HEADER rle_header;


		if(argc < 3) {
			Puts("\nuse: sosd dnlut ?");
			return;
		}
		else {
			if(argv[2][0]=='?') {
				Puts("\nFormat: sosd dnlut winno lut_type(0 or 1) lut_offset flash_addr");
				Puts("\n      : sosd dnlut winno lut_type(2 or 3) lut_offset flash_addr [nColor].");
				Puts("\n\twinno 0~8. TW8836 needs winno");
				Puts("\n\tlut_type 0: TW8832 style. 256 color.");
				Puts("\n\t         1: TW8835 menu style. image has a header+palette+data.");
				Puts("\n\t         2: LUT type(BBB...GGG..RRR...AAA...).");
				Puts("\n\t         3: LUTS type(BGRA BGRA BGRA...).");
				Puts("\n\tlut_offset 0~511 (TW8836 win1~win2:0~255, win3~win8 and win0:0~511)");
				Puts("\n\tflash_addr:flash address. if lut_type is 1, it has a header location");
				Puts("\n\tnColor:number of color. default:256");
				Puts("\nexample:");
				Puts("\n\t sosd dnlut 1 1 0 41e071      ; menu background");
				Puts("\n\t sosd dnlut 1 3 0 41e081 128  ; menu background");
				Puts("\n\t sosd dnlut 8 1 256 4336a7    ; menu video icon");
				return;
			}
			if(argc < 6) {
				Puts("\nuse: sosd dnlut ?");
				return;
			}
			winno = a2h(argv[2]);
			lut_type = a2h(argv[3]);
			lut_loc = a2i(argv[4]);
			image_loc = a2h(argv[5]);
			if(lut_type == 2 || lut_type == 3) {
				if(argc < 7) 
					nColor = 256;
				else
					nColor = a2i(argv[6]);
			}
		}
		
	
		//MenuPrepareImageHeader(image);	//update header_table
		if(lut_type==0) { //TW8832 style. only LUT. size 256*4. group style.
			header->lut_type = 0;
			header->lut_size = 0x400;
			header->lut_loc = image_loc;
		}
		else if(lut_type==1) { //TW8835 menu style
			MenuReadRleHeader(image_loc,&rle_header);		
			rle2_to_header(&rle_header);
			header->lut_loc = image_loc + MRLE_INFO_SIZE;
			header->image_loc = image_loc + header->lut_size + MRLE_INFO_SIZE;
		} 
		else if(lut_type==2) {
			header->lut_type = 0;
			header->lut_size = nColor*4;
			header->lut_loc = image_loc;
		}
		else if(lut_type==3) {
			header->lut_type = 1;
			header->lut_size = nColor*4;
			header->lut_loc = image_loc;
		}
		else {
			Printf("\nInvalid command...");	
			return;
		}	
		WaitVBlank(1);
		SpiOsdLoadLUT(winno, header->lut_type, lut_loc, header->lut_size, header->lut_loc, 0xFF);

	}
	//-------<<img4 win# lut#>>-----------------------------
	else if(!stricmp( argv[1], "??")) {
	}
	//-------<<img5 win# lut#>>-----------------------------
	else if(!stricmp( argv[1], "??")) {
	}
	//-------<<img6 win# lut#>>-----------------------------
	else if(!stricmp( argv[1], "??")) {
	}
	//-------<<img7 win# lut#>>-----------------------------
	else if(!stricmp( argv[1], "??")) {
	}
	//-------<<??>>-----------------------------
	else if(!stricmp( argv[1], "??")) {
	}
	else
		Printf("\nInvalid command...");	
}
