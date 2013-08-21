#ifndef _SPI_MAP_H_
#define _SPI_MAP_H_

//=============================================================================
//
//		SPI Image Map
//
//=============================================================================

//for MREL form
struct RLE2_HEADER {
    char id[2];     // 'IT' - IntersilTechwell

    unsigned char op_bmpbits;
    unsigned char rledata_cnt;

    unsigned short w;
    unsigned short h;

    unsigned long  size;
    
    unsigned char LUT_format; // bit0- 0:RGBa, 1:BGRa, bit1- 0:LUTS, 1:LUT;
    unsigned char LUT_colors; // 0xff:256 colors,  0x7F:128 colors,...
    unsigned char dummy0;
    unsigned char dummy1;

};

//for LUT+IMAGE
typedef struct { 
    DWORD start; 
	WORD lut_size;		 //lut size...Please remove. You don't need. Use rle.upper.
    DWORD length; 
} MY_SLIDEIMAGE; 


// *********** RLE file information ***********
typedef struct {
	BYTE rle;	// Upper Nibble is data bits, Lower Nibble is count bits
				// if Lower Nibble is 0, upper nibble means, LUT bits, 4, 6, or 8
    WORD w;
    WORD h;
} MY_RLE_INFO;

#define LUT_4BIT	0
#define LUT_6BIT	1
#define LUT_8BIT	2

//minimum unit: 64KB = 0x10000
#define SFLASH_CODE_ADDR			0x000000
#define SFLASH_EEPROM_ADDR			0x080000L
#define SFLASH_DEMO_ADDR			0x0A0000
#define SFLASH_IMG_ADDR				0x400000
//===============================
// SFLASH_TEST MAP
//===============================
#define MENU_TEST_FLASH_START		0x700000


//===============================
// SFLASH_DEMO MAP
//===============================
//-----------------
// DEMO ParkGrid
//-----------------
#define DGRID_START			0x100000L	//SFLASH_DEMO_ADDR
#define DGRID_H				730
#define DGRID_V				245

#define DGRID_LUT_START		DGRID_START + 0x000000L
#define DGRID_LUT_LEN		0x001000L							// (256*4) * 4
#define DGRID_MSG_START		DGRID_LUT_START + DGRID_LUT_LEN		// 680x234x1
#define	DGRID_MSG_LEN		0x02EE00L							// 800x60x4
#define DGRID_IMG_START		DGRID_MSG_START + DGRID_MSG_LEN		// 730x245x19
//#define	DGRID_IMG_LEN		0x026D90L
#define	DGRID_IMG_LEN		0x03EFD1L
#define DGRID_IMG_END		DGRID_IMG_START + DGRID_IMG_LEN		// 0x06EDD1


extern FAR CONST SLIDEIMAGE d_grid_IMG[]; 
extern FAR CONST BYTE d_grid_rle[];

//-----------------
// DEMO Animation
//-----------------
#define ANI_DEMO_START		DGRID_START+0x070000L	//DGRID_IMG_END

#define	PIGEON_START		ANI_DEMO_START+0x000000L	 	// 120x140x8
#define	PIGEON_LEN			0x020D00L
#define	PIGEON_LUT_LOC		ANI_DEMO_START+0x1A7700L

#define	ROSE_START			ANI_DEMO_START+0x020D00L	  	// 400x400x10
#define	ROSE_LEN			0x186A00L
#define	ROSE_LUT_LOC		ANI_DEMO_START+0x1A7700L

#define PIGEON_ROSE_LUT_LEN	0x001000

#define ANI_DENO_END	ROSE_LUT_LOC + PIGEON_ROSE_LUT_LEN


#define FPGA_TEST_IMG		0x320000




//===============================
// SFLASH_IMG MAP
//===============================
#define FONT_ADDR					SFLASH_IMG_ADDR					//400000
#define FONT_SIZE					0x2800
#define LOGO_ADDR					FONT_ADDR + FONT_SIZE			//402800
#define LOGO_LUT_SIZE				0x000400
#define LOGO_IMG_SIZE				18270
#define LOGO_SIZE					LOGO_LUT_SIZE+LOGO_IMG_SIZE		
#define LOGO_END					0x40735E	

#define BPP2_START			0x100
#define BPP3DIGIT_START		0x80	//0x6C
#define BPP4PENGUIN_START	0x98

#define MENU_IMAGE					SFLASH_IMG_ADDR+0x010000

//test110207
#define MENU_B_FLASH_START			MENU_IMAGE
//#define MENU_B_FLASH_START			0x402000

//old 12x18 font set.
//#define FONT0_ADDR				SFLASH_IMG_ADDR
//#define FONT0_LEN					0x10E0


//-----------------------------------
// FONT
//-----------------------------------
//extern code FONT_SPI_INFO_t test16x32xA0_A0_A0_A0;
//extern code FONT_SPI_INFO_t test16x32xA0_20_20_20; 
//extern code FONT_SPI_INFO_t consolas22_16x26_2BPP;
extern code FONT_SPI_INFO_t consolas16x26_606C90;
extern code FONT_SPI_INFO_t consolas16x26_graynum;
extern code FONT_SPI_INFO_t default_font; 		
extern code FONT_SPI_INFO_t kor_font; 		
extern code FONT_SPI_INFO_t ram_font; 		


void MonSOsdImgInfo(void);
void MonOsdLutLoad(BYTE img_n, BYTE sosd_win, WORD lut);
void MonOsdImgLoad(BYTE img_n, BYTE sosd_win, WORD item_lut);



#endif
