/**
 * @file
 * Monitor_SPI.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	Interface between TW_Terminal2 and Firmware.
*/
//*****************************************************************************
//
//								Monitor_SPI.c
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
#include "monitor_SPI.h"

#include "i2c.h"
#include "SPI.h"

#include "eeprom.h"

static void SPI_Status(void);
static void SPI_dump(DWORD spiaddr); 

//=============================================================================
//
//=============================================================================
void MonitorSPI(void)
{
#ifdef USE_SFLASH_EEPROM
	BYTE j;
#endif

	//---------------------- Dump SPI --------------------------
	if( !stricmp( argv[1], "D" ) ) {
		static DWORD spiaddr = 0;

		if( argc>=3 ) spiaddr = a2h( argv[2] );

		SPI_dump(spiaddr); 

		spiaddr += 0x80L;
	}
	//--------------------------------------------------------
	else if( !stricmp( argv[1], "status" ) ) {
		SPI_Status();
	}
//	else if( !stricmp( argv[1], "quad" ) ) {
//		SPI_quadio();
//	}
	//--------------------------------------------------------
#ifdef USE_SFLASH_EEPROM
	else if( !stricmp( argv[1], "t" ) ) {
	
		SPI_SectorErase( E3P_SPI_SECTOR0 );
		
		for(j=0; j<128; j++) {
			SPI_Buffer[j] = j;
		}
		SPI_PageProgram( E3P_SPI_SECTOR0, (WORD)SPI_Buffer, 128 );
	}
#endif
	//--------------------------------------------------------
	else if( !stricmp( argv[1], "Sector" ) ) {
		static DWORD spiaddr = 0x10000L;

		if( argc>=3 ) spiaddr = a2h( argv[2] );

		Printf("\nSector Erase = %06lx", spiaddr);

		SPI_SectorErase(spiaddr);
		spiaddr += 0x1000L;
	}
	//--------------------------------------------------------
	else if( !stricmp( argv[1], "Block" ) ) {
		static DWORD spiaddr = 0x10000L;

		if( argc>=3 ) spiaddr = a2h( argv[2] );

		Printf("\nBlock Erase = %06lx", spiaddr);

		SPI_BlockErase(spiaddr);
		spiaddr += 0x1000L;
	}
	//--------------------------------------------------------
	else if( !stricmp( argv[1], "Copy" ) ) {
		DWORD source = 0x0L, dest = 0x10000L;
		DWORD cnt=0x100L;

		if( argc<4 ) {
			Printf("\nMissing Parameters !!!" );
			return;
		}
		source = a2h( argv[2] );
		dest   = a2h( argv[3] );
		cnt    = a2h( argv[4] );

		Printf("\nSPI copy from %06lx to %06lx", source, dest);
		Printf("  %03x bytes", (WORD)cnt);
		SpiFlashDmaRead2XMem(SPI_Buffer,source,cnt);

		SPI_PageProgram( dest, (WORD)SPI_Buffer, cnt );
	}
	//--------------------------------------------------------
	else if( !stricmp( argv[1], "rmode" ) ) {
		BYTE mod;

		if(argc < 3)
			mod = 0x05;		//QuadIO
		else
			mod = a2h( argv[2] );

		SPI_SetReadModeByRegister(mod);
		Printf("\nSPI Read Mode = %bd 0x%bx", mod, ReadTW88(REG4C0) & 0x07);
#ifdef MODEL_TW8835
#else
		Puts(" Please, set 4B value!!");
#endif
	}
#ifdef MODEL_TW8835
#else
	else if( !stricmp( argv[1], "4B" ) ) {
		BYTE mod;
		BYTE i;
		volatile BYTE vdata;

		if(argc == 3) {
			mod = a2h( argv[2] );
			SPI_Set4BytesAddress(mod);
		}
		//read HW value.
		ReadTW88Page(page);
		WriteTW88Page(4);
		WriteTW88(REG4C3,(DMA_DEST_CHIPREG << 6) | 1);
		if(SpiFlashVendor==SFLASH_VENDOR_MX_256)
			WriteTW88(REG4CA, SPICMD_RDCR );			//for MX. CMD:0x15
		else
			WriteTW88(REG4CA, SPICMD_RDINFO );
	 	WriteTW88(REG4C6, 0x04 );						// data Buffer address 0x04D0
		WriteTW88(REG4C7, 0xD0 );						// data Buffer address
		WriteTW88(REG4DA, 0x00 );						// data Buff count high
		WriteTW88(REG4C8, 0x00 );						// data Buff count middle
		WriteTW88(REG4C9, 0x01 );						// data Buff count Lo
		WaitVBlank(1);
		WriteTW88(REG4C4, 0x01 );						// DMA-Read
		//wait
		for(i=0; i < 200; i++) {
			vdata = ReadTW88(REG4C4);
			if((vdata & 0x01)==0)
				break;
			delay1ms(10);
		}
		mod = ReadTW88(REG4D0);
		WriteTW88Page(page);
		//report current status
		if(SpiFlashVendor==SFLASH_VENDOR_MX_256)
			Printf("\nSPI 4Byte Mode SW:%bd HW:%bx", SpiFlash4ByteAddr, (mod & 0x20) ? 1:0 );
		else
			Printf("\nSPI 4Byte Mode SW:%bd HW:%bx", SpiFlash4ByteAddr, (mod & 0x04) ? 1:0 );

	}
#endif
	//---------------------- Write SPI --------------------------
	else if( !stricmp( argv[1], "W" ) ) {
		static DWORD spiaddr = 0;
		BYTE *ptr = SPI_Buffer;
		DWORD size;
		BYTE i;

		if( argc<3 ) {
			Printf("\nMissing Parameters !!!" );
			return;
		}
		if( argc > 7 ) {
			Printf("\nonly support 4 bytes !!!" );
		}
		

		spiaddr = a2h( argv[2] );
		//only support eeprom area....please
		//if((spiaddr < EE_SPI_SECTOR0) || (spiaddr > EE_SPI_SECTOR0+0x010000)) {
		//	Printf("\nout of range %06lx~%06lx!!!", EE_SPI_SECTOR0, EE_SPI_SECTOR0+0x010000 );
		//	return;
		//}
		Printf("\nWrite SPI %06lx ", spiaddr);


		size=0;
		for(i=3; i < argc; i++) {
			*ptr++ = (BYTE)a2h(argv[i]);
			Printf(" %02bx ",(BYTE)a2h(argv[i]));
			size++;
		}	
		SPI_PageProgram( spiaddr, (WORD)SPI_Buffer, (WORD)size);
	}


	else if(argc >= 2 && argv[1][0] >= '0' && argv[1][0] <= '9' ) {
		BYTE rcnt,wcnt;
		BYTE i;
		volatile BYTE vdata;
		//old spi command style
		if(argc < 3) {
			Puts("\nSPI nRead cmd...	;command old style.");
			return;
		}

		rcnt = a2h(argv[1]);

		wcnt = argc - 2;
		WriteTW88(REG4C3,(DMA_DEST_CHIPREG << 6) | wcnt);
	 	WriteTW88(REG4C6, 0x04 );						// data Buffer address 0x04D0
		WriteTW88(REG4C7, 0xD0 );						// data Buffer address
		WriteTW88(REG4DA, 0x00 );						// data Buff count high
		WriteTW88(REG4C8, 0x00 );						// data Buff count middle
		WriteTW88(REG4C9, rcnt );						// data Buff count Lo
		for(i=0; i < wcnt; i++) {
			WriteTW88(REG4CA+i, a2h(argv[2+i]) );
		}
		WaitVBlank(1);
		if(rcnt)
			WriteTW88(REG4C4, 0x01 );						// DMA-Read
		else
			WriteTW88(REG4C4, 0x03 );						// DMA-Write
		for(i=0; i < 200; i++) {
			vdata = ReadTW88(REG4C4);
			if((vdata & 0x01)==0)
				break;
			delay1ms(10);
		}
		Printf("wait:%bd,%bx data:",i,vdata);

		if(rcnt) {
			Puts("\bSPI Read==> ");
			for(i=0; i<rcnt; i++) {
				Printf("%02bx ",ReadTW88(REG4D0+i));
			}
		}
	}

	//--------------------------------------------------------
	else if( !stricmp( argv[1], "?" ) ) {
		Printf("\n\n  === Help for SPI command ===");
		Printf("\nSPI D [addr]      ; dump");
		Printf("\nSPI sector [addr] ; sector erase");
		Printf("\nSPI block [addr]  ; block erase");
	   	Printf("\nSPI rmode [mode]	; set the read mode");
#ifdef MODEL_TW8835
#else
	   	Printf("\nSPI 4B [mode]		; set the 4Byte Addr mode");
#endif
		//Printf("\nSPI W addr data   ; write 1 byte. Use PageProgram");
		Printf("\nSPI W addr data ...  ; write max 8 byte. Use PageProgram");
	   	Printf("\nSPI copy [src] [dst] [cnt] ; copy");
		Printf("\nSPI status      	; status");
		Printf("\n");
	}
	//--------------------------------------------------------
	else
		Printf("\nInvalid command...");	
}
//=============================================================================
//
//=============================================================================
void HelpMonitorSPIC(void)
{
	Printf("\n\n  === Help for SPIFlash command ===");
	Printf("\nSPIC 9f               ; SPICMD_RDID: read JEDEC id");
	Printf("\nSPIC  6               ; SPICMD_WREN: write enable");
	Printf("\nSPIC  4               ; SPICMD_WRDI: write disable");
	Printf("\nSPIC  5               ; SPICMD_RDSR: read status register");
	Printf("\nSPIC 35               ; SPICMD_RDSR2: read status register");
	Printf("\nSPIC 2B               ; SPICMD_RDINFO: read information register.S[7]:HighBank S[2]:4byte mode");
	Printf("\nSPIC  1 state1 state2 ; SPICMD_WRSR: write status register");
	Printf("\nSPIC B7               ; SPICMD_EN4B: enter 4Byte mode.");
	Printf("\nSPIC E9               ; SPICMD_EX4B: exit 4Byte mode.");
	Printf("\nSPIC 67               ; SPICMD_ENHBL: enter high bank latch mode. Note:need FW at HighBank");
	Printf("\nSPIC 98               ; SPICMD_EXHBL: exit high bank latch mode.");
	Printf("\nSPIC  2 addr data..	; SPICMD_PP:PageProgram. Max 8 data");
	Printf("\nSPIC 20 addr          ; SPICMD_SE: sector erase");
	Printf("\nSPIC d8 addr          ; SPICMD_BE: block erase");
	Printf("\nSPIC  3 addr [n]      ; SPICMD_READ: read data. default 8 data. Need REG4C0[2:0]=0");
	Printf("\nSPIC  b addr [n]      ; SPICMD_FASTREAD: fast read data. default 8 data. Need REG4C0[2:0]=1");
	Printf("\nSPIC eb addr [n]      ; SPICMD_4READ: QuadIO read data. default 8 data. NG. Only 1Byte");
	Printf("\nSPIC 65               ; SPICMD_RDVEREG: read volatile status register");
	Printf("\nSPIC 61 data          ; SPICMD_WDVEREG: write volatile enhanced status register");
	Printf("\nSPIC 85               ; SPICMD_RDVREG: read volatile enhanced status register");
	Printf("\nSPIC 81 data          ; SPICMD_WDVREG: write volatile status register");
	Printf("\nSPIC B5               ; SPICMD_RDNVREG: read non-volatile status register");
	Printf("\nSPIC B1 data1 data0   ; SPICMD_WDNVREG: write non-volatile status register");
	Printf("\nSPIC 15               ; SPICMD_RDCR: read configuration register. S[5]:4byte mode. for MX");
	Printf("\n");
}
void MonitorSPIC(void)
{
	DECLARE_LOCAL_page
	BYTE cmd;
	BYTE dat0;
	BYTE i;
	BYTE cnt;
	DWORD spiaddr;
	BYTE ret;

	if(argc < 2) {
		HelpMonitorSPIC();
		return;
	}
	ReadTW88Page(page);
	WriteTW88Page(4);

	cmd = a2h( argv[1] );
	if(cmd == SPICMD_RDID) {
		Printf("\nRDID(JEDEC) ");
		ret = SpiFlashChipRegCmd(SPICMD_RDID,0,3, 1);
		if(!ret) {
			Printf("%02bx %02bx %02bx", SPI_CmdBuffer[0], SPI_CmdBuffer[1],SPI_CmdBuffer[2]);
			switch(SPI_CmdBuffer[0]) {
			case 0x1C:	 	Puts("\nEON");		break;
			case 0xC2:		Puts("\nMX");		break;
			case 0xEF:		Puts("\nWB");		break;
			case 0x20:		Puts("\nMicron");	break;
			default:		Puts("\nUnknown");	break;
			}
			if(SPI_CmdBuffer[2] == 0x18) Puts("128"); 
		}
	}
	else if(cmd == SPICMD_WREN || cmd == SPICMD_WRDI) {
		if(cmd == SPICMD_WRDI) Puts("\nWRDI ");
		else				   Puts("\nWREN ");
		ret=SpiFlashChipRegCmd(cmd,0,0, 1);
		//if(ret) {
		//	Puts(" Fail");
		//}
	}
	else if(cmd == SPICMD_RDSR || cmd == SPICMD_RDSR2) {
		if(cmd == SPICMD_RDSR2)	Puts("\nRDSR2 ");
		else					Puts("\nRDSR ");

		ret = SpiFlashChipRegCmd(cmd,0,1, 1);
		if(!ret) {
			Printf("data:%bx",SPI_CmdBuffer[0]);
			if(cmd == SPICMD_RDSR2) {
				if(SPI_CmdBuffer[0] & 0x02) Puts(" Quad");
			}
			else {
				if(SPI_CmdBuffer[0] & 0x40) Puts(" Quad");
				if(SPI_CmdBuffer[0] & 0x02) Puts(" WEL");
				if(SPI_CmdBuffer[0] & 0x01) Puts(" WIP");
			}
		}
	}
	else if(cmd == SPICMD_WRSR) {
		if( argc<4 ) {
			Printf("\nMissing Parameters !!!" );
			return;
		}
		SPI_CmdBuffer[0] = a2h(argv[2]);
		SPI_CmdBuffer[1] = a2h(argv[3]);
		ret = SpiFlashChipRegCmd(cmd,2,0, 1);
	}
	else if(cmd == SPICMD_PP) {
		if( argc<4 ) {
			Printf("\nMissing Parameters !!!" );
			return;
		}
		if( argc > 11 ) {
			Printf("\nonly support 8 bytes !!!" );
			argc = 11;
		}
		spiaddr = a2h( argv[2] );
		Printf("\nPP %06lx ", spiaddr);

		SPI_CmdBuffer[0] = spiaddr >> 16;
		SPI_CmdBuffer[1] = spiaddr >> 8;
		SPI_CmdBuffer[2] = spiaddr;
		//PP uses a REG4D0~4D7 for buffer.
		for(i=3,cnt=0; i <argc; i++,cnt++) {
			dat0 = a2h(argv[i]);
			Printf(" %02bx",dat0);
			WriteTW88(REG4D0+cnt, dat0); 
		}

		ret = SpiFlashChipRegCmd(cmd,3,0, 1);
	}
	else if(cmd == SPICMD_SE || cmd == SPICMD_BE) {
		if( argc<3 ) {
			Printf("\nMissing Parameters !!!" );
			return;
		}
		spiaddr = a2h( argv[2] );
		if(cmd==cmd == SPICMD_BE)
			Printf("\nBE %06lx ", spiaddr);
		else
			Printf("\nSE %06lx ", spiaddr);

		SPI_CmdBuffer[0] = spiaddr >> 16;
		SPI_CmdBuffer[1] = spiaddr >> 8;
		SPI_CmdBuffer[2] = spiaddr;

		ret = SpiFlashChipRegCmd(cmd,3,0, 1);
	}
	else if(cmd == SPICMD_READ || cmd == SPICMD_FASTREAD || cmd == SPICMD_4READ ) {
		BYTE w_len;
		if( argc < 3 ) {
			Printf("\nMissing Parameters !!!" );
			return;
		}
		if(cmd==SPICMD_4READ) {
			Puts("\nQ_READ");
			dat0 = ReadTW88(REG4C0) & 0x07;
			if(dat0 != 5) {
				Puts(" need REG4C0[2:0]=5");
				return;
			}
		}

		spiaddr = a2h( argv[2] );
		Printf(" %06lx ", spiaddr);

		if(argc >= 4)
			cnt = a2h(argv[3]);
		else cnt = 8;


		SPI_CmdBuffer[0] = spiaddr >> 16;
		SPI_CmdBuffer[1] = spiaddr >> 8;
		SPI_CmdBuffer[2] = spiaddr;
		w_len = 3;		
		if(cmd==SPICMD_FASTREAD)
			w_len += 1;		
		else if(cmd==SPICMD_4READ)
			w_len += 3;	//note: w_len will be 6.	

		ret = SpiFlashChipRegCmd(cmd,w_len, cnt, 1);
		if(!ret) {
			for(i=0; i < cnt; i++)
				Printf(" %02bx", SPI_CmdBuffer[i]);
		}
	}
	else if(cmd == SPICMD_RDVEREG || cmd == SPICMD_RDVREG || cmd == SPICMD_RDNVREG) {
		//only for micron
		BYTE r_len;
		if(cmd== SPICMD_RDVEREG){
			Printf("\nRDVEREG:%bx",SPICMD_RDVEREG);
			r_len = 1;
		}
		else if(cmd== SPICMD_RDVREG) {
			Printf("\nRDVREG:%bx",SPICMD_RDVREG);
			r_len = 1;
		}
		else {
			Printf("\nRDNVREG:%bx",SPICMD_RDNVREG);
			r_len = 2;
		}
		ret = SpiFlashChipRegCmd(cmd,0, r_len, 1);
		if(!ret) {
			Printf("%02bx", SPI_CmdBuffer[0]);
			if(cmd== SPICMD_RDNVREG) {
				Printf(" %02bx", SPI_CmdBuffer[1]);
			}
			if(cmd== SPICMD_RDVREG) {
				Printf(" DummyCycle:%bd", SPI_CmdBuffer[0] >> 4);
			}
			else {
				Printf(" DummyCycle:%bd", SPI_CmdBuffer[1] >> 4);
			}
		}
	}
	else if(cmd==SPICMD_WDVEREG || cmd==SPICMD_WDVREG || cmd==SPICMD_WDNVREG) {
		//only for micron
		BYTE w_len;
		if(cmd==SPICMD_WDVEREG || cmd==SPICMD_WDVREG) {
			if( argc<3 ) {
				Printf("\nMissing Parameters !!!" );
				return;
			}
			SPI_CmdBuffer[0] = a2h(argv[2]);
			w_len = 1;
		}
		else {
			if( argc<4 ) {
				Printf("\nMissing Parameters !!!" );
				return;
			}
			SPI_CmdBuffer[0] = a2h(argv[2]);
			SPI_CmdBuffer[1] = a2h(argv[3]);
			w_len = 2;
		}
		ret=SpiFlashChipRegCmd(cmd,w_len, 0, 1);
	}
	else {
		Printf("\nUnknown CMD:%02bx",cmd);
	}

	WriteTW88Page(page);
}

//	Format is needed only once
//	Init is needed when starting program
#ifdef USE_SFLASH_EEPROM
void MonitorEE(void)
{
	BYTE dat; //, i, j;
	BYTE dat1;
	WORD index;

	index = a2h(argv[2]);
	dat   = a2h(argv[3]);

	if (!stricmp(argv[1], "format"))
	{
		Printf("\nFormat EEPROM...");
		E3P_Format();
		return;
	}
	else if (!stricmp(argv[1], "init"))
	{
		Printf("\nFind EEPROM variables...");
		//E3P_Init();
		E3P_Configure();
		return;
	}
	else if (!stricmp(argv[1], "repair"))
	{
		Printf("\nRepair MoveDone error..call only when EE find have a MoveDone error");
		E3P_Repair();
		return;
	}
	else if (!stricmp(argv[1], "default"))
	{
		Printf("\nEE initialize........");
		ClearBasicEE();
		SaveDebugLevelEE(0);
		SaveFWRevEE(FWVER);
		E3P_PrintInfo();
		return;
	}
	else if (!stricmp(argv[1], "check"))
	{
		Printf("\nEE check");
		E3P_Check();
		return;
	}
	else if (!stricmp(argv[1], "info"))
	{
		Printf("\nEE info");
		E3P_PrintInfo();
		return;
	}
	else if (!stricmp(argv[1], "clean"))
	{
		Printf("\nEE clean blocks");
		E3P_Clean();
		return;
	}
	else if (!stricmp(argv[1], "W"))
	{
		if (argc == 4)
		{
			Printf("\nWrite EEPROM %03x:%02bx ", index, dat);
			EE_Write(index, dat);
			dat1 = EE_Read(index);  //BUG
			dat = EE_Read(index);
			Printf(" ==> Read EEPROM[%03x] = %02bx %02bx", index, dat1, dat );
		}
	}
	else if (!stricmp(argv[1], "R"))
	{
		if (argc == 3)
		{
			dat = EE_Read(index);
			Printf("\n ==> Read EEPROM[%03x] = %02bx ", index, dat );
		}
	}
#ifdef USE_SFLASH_EEPROM
	else if (!stricmp(argv[1], "D"))
	{
		Printf("\nDump EEPROM");
		E3P_DumpBlocks();
/*
		for (j=0; j<E3P_MAX_INDEX/16; j++)
		{
			Printf("\nEEPROM %02bx:", j*0x10);
			for (i=0; i<8; i++)
				Printf(" %02bx", EE_Read(j*16 + i));
			Printf("-");
			for (; i<16; i++)
				Printf("%02bx ", EE_Read(j*16 + i));
		}
*/		
	}
#endif
	else if (!stricmp(argv[1], "?"))
	{
		Printf("\n\n  === Help for EE command ===");
		Printf("\nEE format         ; format and initialize");
		Printf("\nEE find           ; initialze internal variables");
		Printf("\nEE init           ; initialze default EE values");
		Printf("\nEE check          ; report map,dump,corrupt");
		Printf("\nEE clean          ; move & clean bank sector");
		Printf("\nEE repair         ; call when you have a movedone error");
		Printf("\nEE w [idx] [dat]  ; write data");
	   	Printf("\nEE r [idx]        ; read data");
		Printf("\nEE d              ; dump all data");
		Printf("\nFYI %bx:DebugLevel %bx:InputMain ",EEP_DEBUGLEVEL,EEP_INPUTSELECTION);
		Printf("\n");
	}
	else
		Printf("\nInvalid command...");	
}
#endif

/**
* SPI Read Status
*/
static void SPI_Status(void)
{
	BYTE dat1;
	BYTE vid;
	BYTE cid;
	BYTE ret;

	ret = SpiFlashChipRegCmd(SPICMD_RDID,0,3, 0);
	if (ret)
		Puts("\nSPICMD_RDID fail");
	vid  = SPI_CmdBuffer[0];
	dat1 = SPI_CmdBuffer[1];
	cid  = SPI_CmdBuffer[2];

	Printf("\nJEDEC ID: %02bx %02bx %02bx", vid, dat1, cid );

	switch(vid) {
	case 0x1C:	 	Puts("\nEON");		break;
	case 0xC2:		Puts("\nMX");		break;
	case 0xEF:		Puts("\nWB");		if(cid == 0x18) Puts("128"); break;
	case 0x20:		Puts("\nMicron");	break;
	default:		Puts("\nUnknown");	break;
	}

	if (vid == 0xC2 || vid == 0x1c) {
		ret=SpiFlashChipRegCmd(SPICMD_RDSR,0, 1, 0);
		Printf("	CMD:%02bx Data:%02bx",SPICMD_RDSR,SPI_CmdBuffer[0]);
		if(SPI_CmdBuffer[0] & 0x40)	Puts(" Quad Enabled");
	}
	else if (vid == 0xEF) {					// WB
		if(cid == 0x18) {				//Q128 case different status read command
			ret=SpiFlashChipRegCmd(SPICMD_RDSR2,0, 1, 0);
			Printf("	CMD:%02bx Data:%02bx",SPICMD_RDSR2,SPI_CmdBuffer[0]);
		}
		else {
			ret=SpiFlashChipRegCmd(SPICMD_RDSR,0, 2, 0);
			Printf("	CMD:%02bx Data:%02bx,%02bx",SPICMD_RDSR,SPI_CmdBuffer[0],SPI_CmdBuffer[1]);
		}
	}
	else if(vid==0x20) {
		if(cid !=0x18) {
			Puts(" NEED 128b!!!");
			return;
		}
		// Volatile
 		ret=SpiFlashChipRegCmd(SPICMD_RDVREG,0, 1, 0);	//cmd, read Volatile register
		dPrintf("	Volatile Register: %02bx", SPI_CmdBuffer[0] );

		// non-Volatile
		ret=SpiFlashChipRegCmd(SPICMD_RDNVREG, 0, 2, 0);	//cmd, read Non-Volatile register
		dPrintf("	Non-Volatile Register: %02bx, %02bx", SPI_CmdBuffer[0], SPI_CmdBuffer[1] );
	}
}

/**
* read and dump SPIFLASH data
*/
static void SPI_dump(DWORD spiaddr) 
{
	BYTE *ptr = SPI_Buffer;
	DWORD cnt = 0x80L;
	BYTE i, j, c;

	SpiFlashDmaRead2XMem(SPI_Buffer, spiaddr, cnt);  //same SpiFlashDmaRead 

	for (j=0; j<8; j++)
	{
		Printf("\nSPI %06lx: ", spiaddr + j*0x10);
		for(i=0; i<8; i++) Printf("%02bx ", SPI_Buffer[j*0x10+i] );
		Printf("- ");
		for(; i<16; i++) Printf("%02bx ", SPI_Buffer[j*0x10+i] );
		Printf("  ");
		for(i=0; i<16; i++) {
			c = SPI_Buffer[j*0x10+i];
			if( c>=0x20 && c<0x80 ) Printf("%c", c);
			else Printf(".");
		}
	}
}







