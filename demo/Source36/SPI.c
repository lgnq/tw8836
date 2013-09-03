/**
 * @file
 * SPI.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	a device driver for the spi-bus interface 
 ******************************************************************************
 */
#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "global.h"
#include "printf.h"
#include "CPU.h"
#include "util.h"

#include "I2C.h"
#include "SPI.h"
#include "Settings.h"	//for CLKPLL_DIV_

//----------------- SPI Mode Definition ---------------------


XDATA BYTE SPI_Buffer[SPI_BUFFER_SIZE];
XDATA BYTE SPI_CmdBuffer[8];

BYTE SPICMD_x_READ      	= 0x03;
BYTE SPICMD_x_BYTES			= 5;
BYTE SpiFlashVendor;
BYTE SpiFlash4ByteAddr;		//4Byte mode for big SPIFLASH

//-----------------------------------------------
//internal prototype
//-----------------------------------------------


//-----------------------------------------------------------------------------
/**
* stop SpiFlashDMA
*
*/
//void SpiFlashDmaStop(void)
//{
//	WriteTW88Page(PAGE4_SPI);
//	//if(ReadTW88(REG4C4) & 0x01)
//	//	Printf("\nLINE:%d DMA STOP at BUSY",__LINE__);
//  
//	WriteTW88(REG4C4, 0);
//}


//-----------------------------------------------------------------------------
/**
* set read cmd that depend on the read mode
*
* same as 
* SpiFlashDmaDestType(dest,0); 
* SpiFlashCmd(SPICMD_x_READ, BYTE SPICMD_x_BYTES);
* 
*/
//void SpiFlashCmdRead(BYTE dest)
//{
//	WriteTW88(REG4C3, dest << 6 | SPICMD_x_BYTES);
//	WriteTW88(REG4CA, SPICMD_x_READ);
//}

//-----------------------------------------------------------------------------
/**
* set DMA destination
*
* need SpiFlashSetCmdLength()
*/
//void SpiFlashDmaDestType(BYTE dest, BYTE access_mode)
//{
//	BYTE dat;
//	dat = ReadTW88(REG4C3) & 0x0F;
//	dat |= (dest << 6);
//	dat |= (access_mode << 4);
//	WriteTW88(REG4C3, dat);
//}

//-----------------------------------------------------------------------------
/**
* set DMA Length
*/
//void SpiFlashSetCmdLength(BYTE len)
//{
//	WriteTW88(REG4C3, (ReadTW88(REG4C3) & 0xF0) | len);
//}

//-----------------------------------------------------------------------------
/**
* wait until DMA start flag is cleared.
*
* SpiFlashDmaStart() call it.
* If DMA was a read, we can not use a busy flag.
* So, FW checks the start bit that is a self clear bit.
* 
* R4C4[0] : start command execution. Self cleared.
*/
#pragma SAVE
#pragma NOREGPARMS
//static BYTE SpiFlashDmaWait(BYTE wait, BYTE delay, WORD call_line)
//{
//	BYTE i;
//	volatile BYTE vdata;
//
//	//BUGFIX. 120718
//	for(i=0; i < 5; i++) ;
//
//	//------------------------
//	//FYI:Assume it is a Page4
//	//WriteTW88Page(4);
//	//------------------------
//	for(i=0; i < wait; i++) {
//		vdata = ReadTW88(REG4C4);
//		if((vdata & 0x01)==0)	//check a self clear bit
//			break;
//		if(delay)
//			delay1ms(delay);
//	}
//	if(i==wait) {
//		Printf("\nSpiFlashDmaWait DMA Busy. LINE:%d",call_line);
//		return ERR_FAIL;
//	}
//	return ERR_SUCCESS;
//}
#pragma RESTORE

//-----------------------------------------------------------------------------
/**
* start SpiFlashDMA
*
* use REG4C1[0]=1 on ExtI2C mode.
* see REG4C1[0]: At Vertical Blank
*
* @param fWrite
*	- 0:read, 1:write
* @param fBusy
*	busy check. see REG4D8 and REG4D9. So, only works with a write mode
* @param call_line for debug
*/
//void SpiFlashDmaStart(BYTE fWrite, BYTE fBusy, WORD call_line)
//{
//	BYTE dat;
//
//	dat = 0x01;					//start
//	if(fWrite) 	dat |= 0x02;	//write
//	if(fBusy)	dat |= 0x04;	//busy
//
//	WriteTW88Page(4);
//	WriteTW88(REG4C4, dat);
//	SpiFlashDmaWait(100/*200*/,1,call_line);
//}

BYTE SpiFlashDmaStart(BYTE dma_option)
{
	BYTE i;
	volatile BYTE vdata;

	// DMA-start
	WriteTW88(REG4C4, 0x01 | dma_option);	

	//wait a done flag
	//REG4C4[0] is a self clear flag register.
	for(i=0; i < 100; i++) {
		vdata = ReadTW88(REG4C4);
		if((vdata & 0x01)==0)
			break;
		delay1ms(2);
	}
	if(i==100) {
		Printf("\nSpiFlashDmaWait DMA Busy.");
		return 2;	//fail:busy
	}
	return 0;
}



//-----------------------------------------------------------------------------
/**
* assign SpiFlashDMA buffer address
*/
//void SpiFlashDmaBuffAddr(WORD addr)
//{
//	WriteTW88(REG4C6, (BYTE)(addr >> 8));	//page
//	WriteTW88(REG4C7, (BYTE)addr);			//index
//}


//-----------------------------------------------------------------------------
/**
* assign the read length
*/
//void SpiFlashDmaReadLen(DWORD len)
//{
//	WriteTW88(REG4DA, len>>16 );
//	WriteTW88(REG4C8, len>>8 );
//	WriteTW88(REG4C9, len );
//}
//-----------------------------------------------------------------------------
/**
* assign the read length (only low byte)
*/
#if 0
//void SpiFlashDmaReadLenByte(BYTE len_l)
//{
//	WriteTW88(REG4C9, len_l );
//}
#endif

#if defined(MODEL_TW8836_MASTER)
//void I2cSpiFlashDmaReadLen(DWORD len)
//{
//	WriteTW88(REG4F5, len>>16 );
//	WriteTW88(REG4F8, len>>8 );
//	WriteTW88(REG4F9, len );
//}
//void I2cSpiFlashDmaReadLenByte(BYTE len_l)
//{
//	WriteTW88(REG4F9, len_l );
//}
#endif



//-----------------------------------------------------------------------------
/**
* assign a flash address
*
* To support 4BYTE address mode.
* extern: SpiFlash4ByteAddr.
*/
void SpiFlashSetAddress2Hw(DWORD addr)
{
	if(SpiFlash4ByteAddr) {
		WriteTW88(REG4CB, (BYTE)(addr >> 24));
		WriteTW88(REG4CC, (BYTE)(addr >> 16));
		WriteTW88(REG4CD, (BYTE)(addr >> 8));
		WriteTW88(REG4CE, (BYTE)(addr));
	}
	else {
		WriteTW88(REG4CB, (BYTE)(addr >> 16));
		WriteTW88(REG4CC, (BYTE)(addr >> 8));
		WriteTW88(REG4CD, (BYTE)(addr));
	}
}

void SpiFlashSetAddress2CmdBuffer(DWORD spiaddr)
{
	if(SpiFlash4ByteAddr) {
		SPI_CmdBuffer[0] = spiaddr >> 24;
		SPI_CmdBuffer[1] = spiaddr >> 16;
		SPI_CmdBuffer[2] = spiaddr >> 8;
		SPI_CmdBuffer[3] = spiaddr;
	}
	else {
		SPI_CmdBuffer[0] = spiaddr >> 16;
		SPI_CmdBuffer[1] = spiaddr >> 8;
		SPI_CmdBuffer[2] = spiaddr;
	}
}



//-----------------------------------------------------------------------------
/**
* read SpiFlash
*
* @param dest_type
*	DMA_DEST_FONTRAM,DMA_DEST_CHIPREG,DMA_DEST_SOSD_LUT,DMA_DEST_MCU_XMEM
* @param dest_loc
*	destination location. WORD
* @param src_loc source location
* @param size	size
*/
void SpiFlashDmaRead(BYTE dest_type, WORD dest_loc, DWORD src_loc, WORD size)
{
	BYTE dma_option;

	WriteTW88Page(PAGE4_SPI);
	WriteTW88(REG4C3, (dest_type << 6) | SPICMD_x_BYTES + SpiFlash4ByteAddr);
	WriteTW88(REG4C6, (BYTE)(dest_loc>>8));				//page
	WriteTW88(REG4C7, (BYTE)(dest_loc));				//index
	WriteTW88(REG4C8, (BYTE)(size >> 8) );				// data Buff count middle
	WriteTW88(REG4C9, (BYTE)size );						// data Buff count Lo
	WriteTW88(REG4CA, SPICMD_x_READ );
	SpiFlashSetAddress2Hw(src_loc);
	WriteTW88(REG4DA, (BYTE)(size >> 16) );				// data Buff count high

	//vblank wait
	//if(vblank)
	//	WaitVBlank(vblank);

	//dma option
	dma_option = 0x04;		//SPIDMA_BUSYCHECK

	// DMA-start
	SpiFlashDmaStart(dma_option);

}
void SpiFlashDmaRead2XMem(BYTE * dest_loc, DWORD src_loc, WORD size)
{
	WORD dest_w_loc = (WORD)dest_loc;
	BYTE dma_option;

	WriteTW88Page(PAGE4_SPI);
	WriteTW88(REG4C3, (DMA_DEST_MCU_XMEM << 6) | SPICMD_x_BYTES + SpiFlash4ByteAddr);
	WriteTW88(REG4C6, (BYTE)(dest_w_loc>>8));			//page
	WriteTW88(REG4C7, (BYTE)(dest_w_loc));				//index
	WriteTW88(REG4C8, (BYTE)(size >> 8) );				// data Buff count middle
	WriteTW88(REG4C9, (BYTE)size );						// data Buff count Lo
	WriteTW88(REG4CA, SPICMD_x_READ );
	SpiFlashSetAddress2Hw(src_loc);
	WriteTW88(REG4DA, (BYTE)(size >> 16) );				// data Buff count high

	//vblank wait
	//if(vblank)
	//	WaitVBlank(vblank);

	//dma option
	dma_option = 0x04;		//SPIDMA_BUSYCHECK

	// DMA-start
	SpiFlashDmaStart(dma_option);
}

//=============================================================================
//
//=============================================================================
/**
* SpiFlash command routine that using a ChipRegister 0x4D0.
*
* use SPI_CmdBuffer[] for Write & Read.
*
*	SpiFlashChipRegCmd(SPICMD_RDID,0,3)
*	SpiFlashChipRegCmd(SPICMD_WREN,0,0)
*
* need REG4C1[0] before you use.
* SPICMD_4READ can read only 1 byte. See TW8835 Errata.
*/
BYTE SpiFlashChipRegCmd(BYTE cmd, BYTE w_len, BYTE r_len, BYTE vblank)
{
	BYTE dma_option;
	BYTE i;
	volatile BYTE vdata;

	WriteTW88Page(PAGE4_SPI);
	WriteTW88(REG4C3,(DMA_DEST_CHIPREG << 6) | (1+w_len));
	WriteTW88(REG4C6, DMA_BUFF_REG_ADDR_PAGE);
	WriteTW88(REG4C7, DMA_BUFF_REG_ADDR_INDEX);
	WriteTW88(REG4C8, 0x00 );							// data Buff count middle
	WriteTW88(REG4C9, r_len );							// data Buff count Lo
	WriteTW88(REG4CA, cmd );
	for(i=0; i < w_len; i++)
		WriteTW88(REG4CB+i, SPI_CmdBuffer[i]);
	WriteTW88(REG4DA, 0x00 );							// data Buff count high
	//vblank wait
	if(vblank)
		WaitVBlank(vblank);

	//dma option
	dma_option = 0x00;
	if(cmd==SPICMD_PP
	|| cmd==SPICMD_SE
	|| cmd==SPICMD_BE
	|| cmd==SPICMD_BE32K
	|| cmd==SPICMD_CE
	)
		dma_option |= 0x02;	//DMA Buff Write Mode

	if(cmd==SPICMD_WDVEREG 
	|| cmd==SPICMD_WDVREG 
	|| cmd==SPICMD_WDNVREG

	|| cmd==SPICMD_READ
	|| cmd==SPICMD_FASTREAD
	|| cmd==SPICMD_4READ

	|| cmd==SPICMD_WRSR

	|| cmd==SPICMD_SE
	|| cmd==SPICMD_BE
	|| cmd==SPICMD_BE32K
	|| cmd==SPICMD_CE
	)
		dma_option |= 0x04;	//BUSY CHECK

	// DMA-start
	WriteTW88(REG4C4, 0x01 | dma_option);	


//	if(cmd==SPICMD_SE)
//		return 0;

	//wait done
	for(i=0; i < 200; i++) {
		vdata = ReadTW88(REG4C4);
		if((vdata & 0x01)==0)
			break;
		delay1ms(10);
	}
	if(i==200) {
		Printf("\nSpiFlashChipRegCmd Busy cmd:%bx",cmd);
		return 2;	//fail:busy
	}
	//read
	for(i=0; i < r_len; i++) 
		SPI_CmdBuffer[i] = ReadTW88(REG4D0+i);		

	return 0;	//success
}


//=============================================================================
//
//=============================================================================
//-----------------------------------------------------------------------------
/**
* set SpiFlash ReadMode
*
* updata HW and, SPICMD_x_READ and SPICMD_x_BYTES.
*
* @param mode
*	- 0: slow	CMD:0x03	BYTE:4
*	- 1: fast	CMD:0x0B	BYTE:5
*	- 2: dual	CMD:0x3B	BYTE:5
*	- 3: quad	CMD:0x6B	BYTE:5
*	- 4: Dualo	CMD:0xBB	BYTE:5
*	- 5: QuadIo	CMD:0xEB	BYTE:7
*/
void SPI_SetReadModeByRegister( BYTE mode )
{
	WriteTW88Page(PAGE4_SPI);
	WriteTW88(REG4C0, (ReadTW88(REG4C0) & ~0x07) | mode);

	switch( mode ) {
		case 0:	//--- Slow
			SPICMD_x_READ	= 0x03;	
			SPICMD_x_BYTES	= 4;	//(8+24)/8
			break;
		case 1:	//--- Fast
			SPICMD_x_READ	= 0x0b;	
			SPICMD_x_BYTES	= 5;   //(8+24+8)/8. 8 dummy
			break;
		case 2:	//--- Dual
			SPICMD_x_READ	= 0x3b;
			SPICMD_x_BYTES	= 5;
			break;
		case 3:	//--- Quad
			SPICMD_x_READ	= 0x6b;	
			SPICMD_x_BYTES	= 5;
			break;
		case 4:	//--- Dual-IO
			SPICMD_x_READ	= 0xbb;	
			SPICMD_x_BYTES	= 5;	//(8+12*2+4*2)/8. Note:*2 means 2 lines.
			break;
		case 5:	//--- Quad-IO
			SPICMD_x_READ	= 0xeb;	 
			SPICMD_x_BYTES	= 7;   //(8+6*4+2*4+4*4)/8. Note:*4 means 4 lines.
			break;
 	}
}

/**
*	set 4Bytes address mode to support more than 128Mbit.
*	call after SPI_SetReadModeByRegister()
*
* #define SPICMD_EN4B				0xB7	//enter 4Byte mode
* #define SPICMD_EX4B				0xE9	//exit 4Byte mode
*
* SpiFlash4ByteAddr value will be used to check the 4Byte mode or not.
*/
void SPI_Set4BytesAddress(BYTE fOn)
{
	BYTE cmd;
	
	cmd = 0;	
	if (fOn)
	{
		if (SpiFlash4ByteAddr == 0)
		{
			SpiFlash4ByteAddr = 1;
			cmd = SPICMD_EN4B;
		}	
	}
	else
	{
		if (SpiFlash4ByteAddr)
		{
			SpiFlash4ByteAddr = 0;
			cmd = SPICMD_EX4B;
		}	
	}
	if (cmd)
	{	
		//BKTODO: skip the done flag check.
		SpiFlashChipRegCmd(cmd, 0, 0, 0);
	}
}

//=============================================================================
/**
* SPI Write Enable
*
* SPI Command = WRITE_ENABLE
*
*	#ifdef FAST_SPIFLASH
*	WriteTW88Page(PAGE4_SPI );	// Set Page=4
*	WriteTW88(REG4C3, 0x41 );	// Mode = command write, Len=1
*	WriteTW88(REG4CA, 0x06 );	// SPI Command = WRITE_ENABLE
*	WriteTW88(REG4C8, 0x00 );	// Read count
*	WriteTW88(REG4C9, 0x00 );	// Read count
*	WriteTW88(REG4C4, 0x03 );	// DMA-Write start
*
*/
//!void SPI_WriteEnable(void)
//!{
//!	WriteTW88Page(PAGE4_SPI );
//!	SpiFlashDmaDestType(DMA_DEST_CHIPREG,0);
//!	SpiFlashCmd(SPICMD_WREN, 1);				
//!	SpiFlashDmaReadLen(0);
//!	SpiFlashDmaStart(SPIDMA_WRITE,0, __LINE__);
//!}

//=============================================================================
/**
* SPI Sector Erase
*
* SPI Command = SECTOR_ERASE
*
*	WriteTW88(REG4C3, 0x44 );		// Mode = command write, Len=4
*	WriteTW88(REG4CA, 0x20 );		// SPI Command = SECTOR_ERASE
*	WriteTW88(REG4CB, spiaddr>>16);	// SPI address
*	WriteTW88(REG4CC, spiaddr>>8 );	// SPI address
*	WriteTW88(REG4CD, spiaddr );	// SPI address
*	WriteTW88(REG4C8, 0x00 );		// Read count
*	WriteTW88(REG4C9, 0x00 );		// Read count
*	WriteTW88(REG4C4, 0x07 );		// DMA-Write start, Busy check
*
* @see SPI_WriteEnable
*/
void SPI_SectorErase( DWORD spiaddr )
{
	SpiFlashChipRegCmd(SPICMD_WREN,0,0,0);

	SpiFlashSetAddress2CmdBuffer(spiaddr);
	SpiFlashChipRegCmd(SPICMD_SE,3+SpiFlash4ByteAddr,0,0);
}

//=============================================================================
/**
* SPI Block Erase
*
* SPI Command = BLOCK_ERASE
*
*	WriteTW88(REG4C3, 0x44 );		// Mode = command write, Len=4
*	WriteTW88(REG4CA, 0xd8 );		// SPI Command = BLOCK_ERASE
*	WriteTW88(REG4CB, spiaddr>>16);	// SPI address
*	WriteTW88(REG4CC, spiaddr>>8);	// SPI address
*	WriteTW88(REG4CD, spiaddr );	// SPI address
*	WriteTW88(REG4C8, 0x00 );		// Read count
*	WriteTW88(REG4C9, 0x00 );		// Read count
*	WriteTW88(REG4C4, 0x07 );		// DMA-Write start, Busy check
*
* @see SPI_WriteEnable
*/
void SPI_BlockErase( DWORD spiaddr )
{
	SpiFlashChipRegCmd(SPICMD_WREN,0,0,0);

	SpiFlashSetAddress2CmdBuffer(spiaddr);
	SpiFlashChipRegCmd(SPICMD_BE,3+SpiFlash4ByteAddr,0,0);
}

//=============================================================================
/**
* SPI PageProgram
*
* SPI Command = PAGE_PROGRAM
*
*	#ifdef FAST_SPIFLASH
*	WriteTW88(REG4C3, (DMA_DEST_MCU_XMEM << 6) | 4 );	// Mode = xdata, Len=4
*	WriteTW88(REG4CA, SPICMD_PP );				// SPI Command = PAGE_PROGRAM
*	WriteTW88(REG4CB, spiaddr>>16 );			// SPI address
*	WriteTW88(REG4CC, spiaddr>>8 );				// SPI address
*	WriteTW88(REG4CD, spiaddr );				// SPI address
*	WriteTW88(REG4C6, xaddr>>8 );				// Buffer address
*	WriteTW88(REG4C7, xaddr );					// Buffer address
*	WriteTW88(REG4C8, cnt>>8 );					// Write count
*	WriteTW88(REG4C9, cnt );					// Write count
*	WriteTW88(REG4C4, 0x07 );					// DMA-Write start, Busy check
*	#endif
*
* @see SPI_WriteEnable
* NOTE: Use SPI_Buffer[]
*/
void SPI_PageProgram( DWORD spiaddr, WORD xaddr, WORD cnt )
{
	BYTE dma_option;

	SpiFlashChipRegCmd(SPICMD_WREN,0,0,0);

	WriteTW88Page(PAGE4_SPI);
	WriteTW88(REG4C3, (DMA_DEST_MCU_XMEM << 6) | 4 + SpiFlash4ByteAddr);
	WriteTW88(REG4C6, (BYTE)(xaddr>>8));				//page
	WriteTW88(REG4C7, (BYTE)(xaddr));					//index
	WriteTW88(REG4C8, (BYTE)(cnt >> 8) );				// data Buff count middle
	WriteTW88(REG4C9, (BYTE)cnt );						// data Buff count Lo
	WriteTW88(REG4CA, SPICMD_PP );
	SpiFlashSetAddress2Hw(spiaddr);
	WriteTW88(REG4DA, (BYTE)(cnt >> 16) );				// data Buff count high

	//vblank wait
	//if(vblank)
	//	WaitVBlank(vblank);

	//dma option
	dma_option = 0x04;		//SPIDMA_BUSYCHECK
	dma_option |= 0x02;		//SPIDMA_WRITE. BK121011. I think, we don't need it.

	// DMA-start
	SpiFlashDmaStart(dma_option);
}

//=============================================================================
/**
* Init QuadIO mode
*
* @return
*	fail:0 or unknown.
*	success: SFLASH_VENDOR name
*
*
* N25Q128: 0x20 0xBA 0x18
* EON128	1C 30 18
*
* init QuadIO mode for MICRON chip
* ---------------------------------------------------------------
* dummy cycle
* FAST_READ		Read Data Bytes at Higher Speed			0x0B	8
* DOFR 			Dual Output Fast Read					0x3B	8
* DIOFR			Dual Input/Output Fast Read				0xBB	8
* QOFR			Quad Output Fast Read					0x6B	8
* QIOFR			Quad Input/Output Fast Read				0xEB	10
* ROTP			Read OTP(Read of OTP area)				0x4B	8
*/	
BYTE SPI_QUADInit(void)
{
	BYTE dat0;
	BYTE vid;
	BYTE cid;
	BYTE ret;
	BYTE temp;
							 
	ret = SpiFlashChipRegCmd(SPICMD_RDID, 0, 3, 0);
	if (ret)
	{
		Puts("\nSPICMD_RDID fail");
		
		return 0;
	}
	
	vid  = SPI_CmdBuffer[0];
	dat0 = SPI_CmdBuffer[1];
	cid  = SPI_CmdBuffer[2];

	Printf("\n\tSPI JEDEC ID: %02bx %02bx %02bx", vid, dat0, cid);
	//C2 20 19 - MX 256

	if (vid == 0x1C)
	{
		SpiFlashVendor = SFLASH_VENDOR_EON;
		if (dat0==0x70 && cid==0x19)
			SpiFlashVendor = SFLASH_VENDOR_EON_256;
	}
	else if (vid == 0xC2)
	{
	 	SpiFlashVendor = SFLASH_VENDOR_MX;
		if (dat0==0x20 && cid==0x19)
			SpiFlashVendor = SFLASH_VENDOR_MX_256;
	} 
	else if (vid == 0xEF)
		SpiFlashVendor = SFLASH_VENDOR_WB;
	else if (vid == 0x20)
	{
		if (cid != 0x18)
		{
			Printf(" UNKNOWN MICRON SPIFLASH !!");			

			return 0;
		}
		SpiFlashVendor = SFLASH_VENDOR_MICRON; //numonyx
	}
#ifdef SUPPORT_SFLASH_SPANSION
	else if (vid == 0x01)
	{
		SpiFlashVendor = SFLASH_VENDOR_SPANSION;	
	}
#endif
	else
	{
		Printf(" UNKNOWN SPIFLASH !!");

		return 0;
	}

	//----------------------------
	//read status register
	//----------------------------

	if (vid == 0xC2 || vid == 0x1C) 							//C2:MX 1C:EON
	{
		ret = SpiFlashChipRegCmd(SPICMD_RDSR, 0, 1, 0);
		temp = SPI_CmdBuffer[0] & 0x40;							
		//if 0, need to enable quad
	}
	else if (vid == 0xEF)					// WB
	{
		//if(cid == 0x18) {				//Q128 case different status read command
			ret=SpiFlashChipRegCmd(SPICMD_RDSR2,0, 1, 0);
			temp = SPI_CmdBuffer[0];							//dat0[1]:QE
			dPrintf("\nStatus2 before QUAD: %02bx", temp);
			//if 0, need to enable quad
		//}
		//else {
		//	SpiFlashCmd(SPICMD_RDSR, 1);
		//	SpiFlashDmaReadLenByte(2);
		//	SpiFlashDmaStart(SPIDMA_READ,0, __LINE__);
		//	dat0 = SPIDMA_READDATA(0);
		//	dat1 = SPIDMA_READDATA(1);
		//	dPrintf("\nStatus before QUAD: %02bx, %02bx", dat0, dat1 );	
		//	temp = dat1;
		//}
	}
	else if (vid == 0x20) //SFLASH_VENDOR_MICRON
	{
		ret = SpiFlashChipRegCmd(SPICMD_RDVREG, 0, 1, 0);	//cmd, read Volatile register
		temp = SPI_CmdBuffer[0];
		Printf("\nVolatile Register: %02bx", temp );
		if (temp != 0x6B)
			temp = 0; //need an enable routine
	}
#ifdef SUPPORT_SFLASH_SPANSION
	else if (vid == 0x01) 	  //SFLASH_VENDOR_SPANSION
	{
		ret = SpiFlashChipRegCmd(SPICMD_RDSR, 0, 1, 0);	//cmd, read Volatile register
		temp = SPI_CmdBuffer[0];
		Printf("\nVolatile Register: %02bx", temp);
		temp = SPI_CmdBuffer[0] & 0x02;	//if 0, need an enable routine
	}
#endif
	if (temp)
		return SpiFlashVendor;

	//----------------------------
	// enable quad
	//----------------------------
	Puts("\nEnable quad mode" );
	if (vid == 0xC2 || vid == 0x1c) 
	{
		SpiFlashChipRegCmd(SPICMD_WREN, 0, 0, 0);
#ifdef DEBUG_SPIFLASH	
		SpiFlashChipRegCmd(SPICMD_RDSR, 0, 1, 0);
		Printf("\nStatus after Write enable %02bx", SPI_CmdBuffer[0] );
#endif	
		SPI_CmdBuffer[0] = 0x40;	//en QAUD mode
		SpiFlashChipRegCmd(SPICMD_WRSR, 1, 0, 0);
		Puts("\nQUAD ENABLED" );
	
#ifdef DEBUG_SPIFLASH	
		SpiFlashChipRegCmd(SPICMD_RDSR, 0, 1, 0);
		Printf("\nStatus after Write enable %02bx", SPI_CmdBuffer[0] );
#endif
		SpiFlashChipRegCmd(SPICMD_WRDI, 0, 0, 0);
	}
	else if (vid == 0xEF)
	{
		SpiFlashChipRegCmd(SPICMD_WREN, 0, 0, 0);
		SPI_CmdBuffer[0] = 0x00;	//cmd, en QAUD mode
		SPI_CmdBuffer[1] = 0x02;	
		SpiFlashChipRegCmd(SPICMD_WRSR, 2, 0, 0);
		dPuts("\nQUAD ENABLED" );
#ifdef DEBUG_SPIFLASH
		//if(cid == 0x18) {				//Q128 case different status read command
			SpiFlashChipRegCmd(SPICMD_RDSR2, 0, 1, 0);
			Printf("\nStatus2 after Write enable %02bx", SPI_CmdBuffer[0] );
		//}
		//else {
		//	SpiFlashCmd(SPICMD_RDSR, 1);
		//	SpiFlashDmaReadLenByte(2);
		//	SpiFlashDmaStart(SPIDMA_READ,0, __LINE__);
		//	dat0 = SPIDMA_READDATA(0);
		//	dat1 = SPIDMA_READDATA(1);
		//	dPrintf("\nStatus before QUAD: %02bx, %02bx", dat0, dat1 );	
		//}
#endif
		SpiFlashChipRegCmd(SPICMD_WRDI, 0, 0, 0);
	}
	else if (vid == 0x20) 	 //SFLASH_VENDOR_MICRON
	{
		SpiFlashChipRegCmd(SPICMD_WREN, 0, 0, 0);

		SPI_CmdBuffer[0] = 0x6B;						// cmd, write Volatile. set 6 dummy cycles
		SpiFlashChipRegCmd(SPICMD_WDVREG, 1, 0, 0);
		Puts("\nVolatile 6 dummy SET" );
		SpiFlashChipRegCmd(SPICMD_WRDI, 0, 0, 0);

		// set non-Volatile
#if 0
		SpiFlashChipRegCmd(SPICMD_RDNVREG, 0, 2, 0);	//cmd, read Non-Volatile register
		dat0 = SPI_CmdBuffer[0];
		temp = SPI_CmdBuffer[1];
		Printf("\nNon-Volatile Register: %02bx, %02bx", dat0, temp );
		if ( temp != 0x6F ) {
			SpiFlashChipRegCmd(SPICMD_WREN, 0, 0, 0);
	
			SPI_CmdBuffer[0] = 0xFF;							// B7~B0, B15~B8, set 6 dummy cycles
			SPI_CmdBuffer[1] = 0x6F;
			SpiFlashChipRegCmd(SPICMD_WDNVREG, 2, 0, 0);	// cmd, write Non-Volatile.
			Puts("\nnon-Volatile 6 dummy SET" );
	
			SpiFlashChipRegCmd(SPICMD_WRDI, 0, 0, 0);
		}
#endif
	}
#ifdef SUPPORT_SFLASH_SPANSION
	else if (vid == 0x01) 	//SPANSION
	{
		SpiFlashChipRegCmd(SPICMD_WREN, 0, 0, 0);
		SPI_CmdBuffer[0] = 0x02;	//en QAUD mode
		SpiFlashChipRegCmd(SPICMD_WRSR, 1, 0, 0);
		Puts("\nQUAD ENABLED" );
		SpiFlashChipRegCmd(SPICMD_WRDI, 0, 0, 0);
	}
#endif

	return SpiFlashVendor;
}

#ifdef SUPPORT_SPIOSD
//-----------------------------------------------------------------------------
/**
* Description
*	download LUT by IO (without DMA).
*	It is only for old image files that was use on TW8832.
*
*	TW8836 has two LUTs.
*	LUTB(LUT group B) has 256 palettes, We can use DMA on LUTTYPE_ADDR.
*	LUTA(LUT group A) has 512 palettes. If use LUTTYPE_ADDR, we must use a IO method.
*
*	This function needs a buffer because SPI.c is located on Code Bank 0.
*	TW8836 has only 2K data memory. It is a really painfull solution.
*
*
* @param
* @param	type
*	- 1:Byte pointer - LUTS type
*	- 0:Address pointer - LUT type
* @param
* @param
* @param
* @return
*/
extern void SpiOsdPixelAlphaAttr(BYTE win, WORD lutloc, BYTE value);

#define SOSD_LUTTYPE_BYTE	1	//LUTS
#define SOSD_LUTTYPE_ADDR	0
void SpiOsdIoLoadLUT(BYTE _winno, BYTE type, WORD LutOffset, WORD size, DWORD spiaddr, BYTE alpha)
{
	BYTE i,j,k;
	BYTE R410_data;
	
#ifdef DEBUG_OSD
	dPrintf("\nSpiOsdIoLoadLUT%s win:%bd, LutLoc:%d size:%d 0x%06lx", type ? "S":" ", _winno, LutOffset, size, spiaddr);
#endif

#if defined(TW8836_CHIPDEBUG)
#else
	McuSpiClkToPclk(CLKPLL_DIV_3P0);
#endif

	WriteTW88Page(PAGE4_SOSD );

	//--- SPI-OSD config
	if (type == SOSD_LUTTYPE_ADDR)
		R410_data = 0xC0;			// LUT Write Mode, En & address ptr inc.
	else
		R410_data = 0xA0;			// LUT Write Mode, En & byte ptr inc.
	if (LutOffset >> 8)
		R410_data |= 0x08;   //BK130121 bugfix
	if (_winno==1 || _winno==2)
		R410_data |= 0x04;		

	if (type == SOSD_LUTTYPE_ADDR)
	{
		//
		//ignore size. it is always 0x400.(256*4)
		//BKTODO130124. If it is a LUTB,...
		//		
		for (i=0; i < 4; i++)
		{	 
			WriteTW88(REG410, R410_data | i );	//assign byte ptr	
			WriteTW88(REG411, (BYTE)LutOffset);	//reset address ptr.
			for (j=0; j<(256/64); j++)
			{
				SpiFlashDmaRead2XMem(SPI_Buffer,spiaddr + i*256 + j*64,64);	 //BUGBUG120606 BANK issue
				for (k=0; k < 64; k++)
				{
					WriteTW88(REG412, SPI_Buffer[k]);		//write data
				}
			}
		}
	}
	else 
	{
		WriteTW88(REG410, R410_data);			//assign byte ptr. always start from 0.
		WriteTW88(REG411, (BYTE)LutOffset);		//reset address ptr.

		for (i=0; i < (size / 64); i++)		//min size is a 64(16*4)
		{
			SpiFlashDmaRead2XMem(SPI_Buffer, spiaddr + i*64, 64);
			for (k=0; k < 64; k++)
			{
				WriteTW88(REG412, SPI_Buffer[k]);		//write data
			}
		}
	}
	//pixel alpha
	if (alpha != 0xFF)
	{
		SpiOsdPixelAlphaAttr(_winno, LutOffset+alpha, 0x7F);	
	}

#if defined(TW8836_CHIPDEBUG)
#else
	McuSpiClkRestore();
#endif
}
#endif

//*****************************************************************************
//
//		EEPROM Emulation
//
//*****************************************************************************
//	Format: For each 4 bytes [Index] [Index^FF] [Data] [Data^FF]
//
//
#ifdef USE_SFLASH_EEPROM
//moved to E3P_c24.lib or E3P_bank.lib
#endif

//=============================================================================
//=============================================================================



