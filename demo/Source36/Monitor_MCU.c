/**
 * @file
 * Monitor_MCU.c 
 * @author Harry Han
 * @author YoungHwan Bae
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	Interface between TW_Terminal2 and Firmware.
*/

//*****************************************************************************
//
//								Monitor_CPU.c
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
#include "reg_debug.h"
#include "monitor_mcu.h"

typedef struct SFR_map_s {
	BYTE idx;
	char name[6];
} SFR_map_t;
//format	XX:nnnnn:dd XX:nnnnn. So each item uses 12 chars.
	//TW8836 DP80390
code struct SFR_map_s SFR_map[] = {
	{0x80,"P0"},	{0x81,"SP"},	{0x82,"DPL0"},	{0x83,"DPH0"},	{0x84,"DPL1"},	{0x85,"DPH1"},	{0x86,"DPS"},	{0x87,"PCON"},
	{0x88,"TCON"},	{0x89,"TMOD"},	{0x8A,"TL0"},	{0x8B,"TL1"},	{0x8C,"TH0"},	{0x8D,"TH1"},	{0x8E,"CKCON"},

	{0x90,"P1"},	{0x91,"EXIF"},	{0x92,"_WTST"},	{0x93,"_DPX0"},					{0x95,"_DPX1"},
	{0x98,"SCON0"},	{0x99,"SBUF0"},	{0x9A,"BANK"},	{0x9B,"ESP"},	{0x9C,"CACHE"},	{0x9D,"ACON"},	{0x9E,"FIFO0"},	{0x9F,"FIFO1"},	

	{0xA0,"P2"},
	{0xA8,"IE"},

	{0xB0,"P3"},
	{0xB8,"IP"},

	{0xC0,"SCON1"},	{0xC1,"SBUF1"},	{0xC2,"CCL1"},	{0xC3,"CCH1"},	{0xC4,"CCL2"},	{0xC5,"CCH2"},	{0xC6,"CCL3"},	{0xC7,"CCH3"},
	{0xC8,"T2CON"},	{0xC9,"T2IF"},	{0xCA,"RLDL"},	{0xCB,"RLDH"},	{0xCC,"TL2"},	{0xCD,"TH2"},	{0xCE,"CCEN"},

	{0xD0,"PSW"},
	{0xD8,"WDCON"},

	{0xE0,"ACC"},					{0xE2,"CAMC"},
	{0xE8,"EIE"},	{0xE9,"STAT"},	{0xEA,"_MXAX"},{0xEB,"TA"},

	{0xF0,"B"},
	{0xF8,"EIP"},					{0xFA,"E2IF"},	{0xFB,"E2IE"},	{0xFC,"E2IP"},	{0xFD,"E2IM"},	{0xFE,"E2IT"},	 {0,0}
};

//=============================================================================
BYTE WriteSFR(BYTE addr, BYTE dat)
{
	switch (addr) {
		case 0x80:	SFR_80h = dat;	if( echo ) return SFR_80h; else return 0;
		case 0x81:	SFR_81h = dat;	if( echo ) return SFR_81h; else return 0;
		case 0x82:	SFR_82h = dat;	if( echo ) return SFR_82h; else return 0;
		case 0x83:	SFR_83h = dat;	if( echo ) return SFR_83h; else return 0;
		case 0x84:	SFR_84h = dat;	if( echo ) return SFR_84h; else return 0;
		case 0x85:	SFR_85h = dat;	if( echo ) return SFR_85h; else return 0;
		case 0x86:	SFR_86h = dat;	if( echo ) return SFR_86h; else return 0;
		case 0x87:	SFR_87h = dat;	if( echo ) return SFR_87h; else return 0;
		case 0x88:	SFR_88h = dat;	if( echo ) return SFR_88h; else return 0;
		case 0x89:	SFR_89h = dat;	if( echo ) return SFR_89h; else return 0;
		case 0x8a:	SFR_8ah = dat;	if( echo ) return SFR_8ah; else return 0;
		case 0x8b:	SFR_8bh = dat;	if( echo ) return SFR_8bh; else return 0;
		case 0x8c:	SFR_8ch = dat;	if( echo ) return SFR_8ch; else return 0;
		case 0x8d:	SFR_8dh = dat;	if( echo ) return SFR_8dh; else return 0;
		case 0x8e:	SFR_8eh = dat;	if( echo ) return SFR_8eh; else return 0;
		case 0x8f:	SFR_8fh = dat;	if( echo ) return SFR_8fh; else return 0;

		case 0x90:	SFR_90h = dat;	if( echo ) return SFR_90h; else return 0;
		case 0x91:	SFR_91h = dat;	if( echo ) return SFR_91h; else return 0;
		case 0x92:	SFR_92h = dat;	if( echo ) return SFR_92h; else return 0;
		case 0x93:	SFR_93h = dat;	if( echo ) return SFR_93h; else return 0;
		case 0x94:	SFR_94h = dat;	if( echo ) return SFR_94h; else return 0;
		case 0x95:	SFR_95h = dat;	if( echo ) return SFR_95h; else return 0;
		case 0x96:	SFR_96h = dat;	if( echo ) return SFR_96h; else return 0;
		case 0x97:	SFR_97h = dat;	if( echo ) return SFR_97h; else return 0;
		case 0x98:	SFR_98h = dat;	if( echo ) return SFR_98h; else return 0;
		case 0x99:	SFR_99h = dat;	if( echo ) return SFR_99h; else return 0;
		case 0x9a:	SFR_9ah = dat;	if( echo ) return SFR_9ah; else return 0;
		case 0x9b:	SFR_9bh = dat;	if( echo ) return SFR_9bh; else return 0;
		case 0x9c:	SFR_9ch = dat;	if( echo ) return SFR_9ch; else return 0;
		case 0x9d:	SFR_9dh = dat;	if( echo ) return SFR_9dh; else return 0;
		case 0x9e:	SFR_9eh = dat;	if( echo ) return SFR_9eh; else return 0;
		case 0x9f:	SFR_9fh = dat;	if( echo ) return SFR_9fh; else return 0;
                                                  
		case 0xa0:	SFR_a0h = dat;	if( echo ) return SFR_a0h; else return 0;
		case 0xa1:	SFR_a1h = dat;	if( echo ) return SFR_a1h; else return 0;
		case 0xa2:	SFR_a2h = dat;	if( echo ) return SFR_a2h; else return 0;
		case 0xa3:	SFR_a3h = dat;	if( echo ) return SFR_a3h; else return 0;
		case 0xa4:	SFR_a4h = dat;	if( echo ) return SFR_a4h; else return 0;
		case 0xa5:	SFR_a5h = dat;	if( echo ) return SFR_a5h; else return 0;
		case 0xa6:	SFR_a6h = dat;	if( echo ) return SFR_a6h; else return 0;
		case 0xa7:	SFR_a7h = dat;	if( echo ) return SFR_a7h; else return 0;
		case 0xa8:	SFR_a8h = dat;	if( echo ) return SFR_a8h; else return 0;
		case 0xa9:	SFR_a9h = dat;	if( echo ) return SFR_a9h; else return 0;
		case 0xaa:	SFR_aah = dat;	if( echo ) return SFR_aah; else return 0;
		case 0xab:	SFR_abh = dat;	if( echo ) return SFR_abh; else return 0;
		case 0xac:	SFR_ach = dat;	if( echo ) return SFR_ach; else return 0;
		case 0xad:	SFR_adh = dat;	if( echo ) return SFR_adh; else return 0;
		case 0xae:	SFR_aeh = dat;	if( echo ) return SFR_aeh; else return 0;
		case 0xaf:	SFR_afh = dat;	if( echo ) return SFR_afh; else return 0;
                                                  
		case 0xb0:	SFR_b0h = dat;	if( echo ) return SFR_b0h; else return 0;
		case 0xb1:	SFR_b1h = dat;	if( echo ) return SFR_b1h; else return 0;
		case 0xb2:	SFR_b2h = dat;	if( echo ) return SFR_b2h; else return 0;
		case 0xb3:	SFR_b3h = dat;	if( echo ) return SFR_b3h; else return 0;
		case 0xb4:	SFR_b4h = dat;	if( echo ) return SFR_b4h; else return 0;
		case 0xb5:	SFR_b5h = dat;	if( echo ) return SFR_b5h; else return 0;
		case 0xb6:	SFR_b6h = dat;	if( echo ) return SFR_b6h; else return 0;
		case 0xb7:	SFR_b7h = dat;	if( echo ) return SFR_b7h; else return 0;
		case 0xb8:	SFR_b8h = dat;	if( echo ) return SFR_b8h; else return 0;
		case 0xb9:	SFR_b9h = dat;	if( echo ) return SFR_b9h; else return 0;
		case 0xba:	SFR_bah = dat;	if( echo ) return SFR_bah; else return 0;
		case 0xbb:	SFR_bbh = dat;	if( echo ) return SFR_bbh; else return 0;
		case 0xbc:	SFR_bch = dat;	if( echo ) return SFR_bch; else return 0;
		case 0xbd:	SFR_bdh = dat;	if( echo ) return SFR_bdh; else return 0;
		case 0xbe:	SFR_beh = dat;	if( echo ) return SFR_beh; else return 0;
		case 0xbf:	SFR_bfh = dat;	if( echo ) return SFR_bfh; else return 0;
                                                  
		case 0xc0:	SFR_c0h = dat;	if( echo ) return SFR_c0h; else return 0;
		case 0xc1:	SFR_c1h = dat;	if( echo ) return SFR_c1h; else return 0;
		case 0xc2:	SFR_c2h = dat;	if( echo ) return SFR_c2h; else return 0;
		case 0xc3:	SFR_c3h = dat;	if( echo ) return SFR_c3h; else return 0;
		case 0xc4:	SFR_c4h = dat;	if( echo ) return SFR_c4h; else return 0;
		case 0xc5:	SFR_c5h = dat;	if( echo ) return SFR_c5h; else return 0;
		case 0xc6:	SFR_c6h = dat;	if( echo ) return SFR_c6h; else return 0;
		case 0xc7:	SFR_c7h = dat;	if( echo ) return SFR_c7h; else return 0;
		case 0xc8:	SFR_c8h = dat;	if( echo ) return SFR_c8h; else return 0;
		case 0xc9:	SFR_c9h = dat;	if( echo ) return SFR_c9h; else return 0;
		case 0xca:	SFR_cah = dat;	if( echo ) return SFR_cah; else return 0;
		case 0xcb:	SFR_cbh = dat;	if( echo ) return SFR_cbh; else return 0;
		case 0xcc:	SFR_cch = dat;	if( echo ) return SFR_cch; else return 0;
		case 0xcd:	SFR_cdh = dat;	if( echo ) return SFR_cdh; else return 0;
		case 0xce:	SFR_ceh = dat;	if( echo ) return SFR_ceh; else return 0;
		case 0xcf:	SFR_cfh = dat;	if( echo ) return SFR_cfh; else return 0;
                                                  
		case 0xd0:	SFR_d0h = dat;	if( echo ) return SFR_d0h; else return 0;
		case 0xd1:	SFR_d1h = dat;	if( echo ) return SFR_d1h; else return 0;
		case 0xd2:	SFR_d2h = dat;	if( echo ) return SFR_d2h; else return 0;
		case 0xd3:	SFR_d3h = dat;	if( echo ) return SFR_d3h; else return 0;
		case 0xd4:	SFR_d4h = dat;	if( echo ) return SFR_d4h; else return 0;
		case 0xd5:	SFR_d5h = dat;	if( echo ) return SFR_d5h; else return 0;
		case 0xd6:	SFR_d6h = dat;	if( echo ) return SFR_d6h; else return 0;
		case 0xd7:	SFR_d7h = dat;	if( echo ) return SFR_d7h; else return 0;
		case 0xd8:	SFR_ebh = 0xaa;
					SFR_ebh = 0x55;
					SFR_d8h = dat;	if( echo ) return SFR_d8h; else return 0;
		case 0xd9:	SFR_d9h = dat;	if( echo ) return SFR_d9h; else return 0;
		case 0xda:	SFR_dah = dat;	if( echo ) return SFR_dah; else return 0;
		case 0xdb:	SFR_dbh = dat;	if( echo ) return SFR_dbh; else return 0;
		case 0xdc:	SFR_dch = dat;	if( echo ) return SFR_dch; else return 0;
		case 0xdd:	SFR_ddh = dat;	if( echo ) return SFR_ddh; else return 0;
		case 0xde:	SFR_deh = dat;	if( echo ) return SFR_deh; else return 0;
		case 0xdf:	SFR_dfh = dat;	if( echo ) return SFR_dfh; else return 0;
                                                  
		case 0xe0:	SFR_e0h = dat;	if( echo ) return SFR_e0h; else return 0;
		case 0xe1:	SFR_e1h = dat;	if( echo ) return SFR_e1h; else return 0;
		case 0xe2:	SFR_e2h = dat;	if( echo ) return SFR_e2h; else return 0;
		case 0xe3:	SFR_e3h = dat;	if( echo ) return SFR_e3h; else return 0;
		case 0xe4:	SFR_e4h = dat;	if( echo ) return SFR_e4h; else return 0;
		case 0xe5:	SFR_e5h = dat;	if( echo ) return SFR_e5h; else return 0;
		case 0xe6:	SFR_e6h = dat;	if( echo ) return SFR_e6h; else return 0;
		case 0xe7:	SFR_e7h = dat;	if( echo ) return SFR_e7h; else return 0;
		case 0xe8:	SFR_e8h = dat;	if( echo ) return SFR_e8h; else return 0;
		case 0xe9:	SFR_e9h = dat;	if( echo ) return SFR_e9h; else return 0;
		case 0xea:	SFR_eah = dat;	if( echo ) return SFR_eah; else return 0;
		case 0xeb:	SFR_ebh = dat;	if( echo ) return SFR_ebh; else return 0;
		case 0xec:	SFR_ech = dat;	if( echo ) return SFR_ech; else return 0;
		case 0xed:	SFR_edh = dat;	if( echo ) return SFR_edh; else return 0;
		case 0xee:	SFR_eeh = dat;	if( echo ) return SFR_eeh; else return 0;
		case 0xef:	SFR_efh = dat;	if( echo ) return SFR_efh; else return 0;
                                                  
		case 0xf0:	SFR_f0h = dat;	if( echo ) return SFR_f0h; else return 0;
		case 0xf1:	SFR_f1h = dat;	if( echo ) return SFR_f1h; else return 0;
		case 0xf2:	SFR_f2h = dat;	if( echo ) return SFR_f2h; else return 0;
		case 0xf3:	SFR_f3h = dat;	if( echo ) return SFR_f3h; else return 0;
		case 0xf4:	SFR_f4h = dat;	if( echo ) return SFR_f4h; else return 0;
		case 0xf5:	SFR_f5h = dat;	if( echo ) return SFR_f5h; else return 0;
		case 0xf6:	SFR_f6h = dat;	if( echo ) return SFR_f6h; else return 0;
		case 0xf7:	SFR_f7h = dat;	if( echo ) return SFR_f7h; else return 0;
		case 0xf8:	SFR_f8h = dat;	if( echo ) return SFR_f8h; else return 0;
		case 0xf9:	SFR_f9h = dat;	if( echo ) return SFR_f9h; else return 0;
		case 0xfa:	SFR_fah = dat;	if( echo ) return SFR_fah; else return 0;
		case 0xfb:	SFR_fbh = dat;	if( echo ) return SFR_fbh; else return 0;
		case 0xfc:	SFR_fch = dat;	if( echo ) return SFR_fch; else return 0;
		case 0xfd:	SFR_fdh = dat;	if( echo ) return SFR_fdh; else return 0;
		case 0xfe:	SFR_feh = dat;	if( echo ) return SFR_feh; else return 0;
		case 0xff:	SFR_ffh = dat;	if( echo ) return SFR_ffh; else return 0;
	}
	return 0;
}

//=============================================================================
BYTE ReadSFR(BYTE addr)
{
	switch (addr) {
		case 0x80:	return SFR_80h;
		case 0x81:	return SFR_81h;
		case 0x82:	return SFR_82h;
		case 0x83:	return SFR_83h;
		case 0x84:	return SFR_84h;
		case 0x85:	return SFR_85h;
		case 0x86:	return SFR_86h;
		case 0x87:	return SFR_87h;
		case 0x88:	return SFR_88h;
		case 0x89:	return SFR_89h;
		case 0x8a:	return SFR_8ah;
		case 0x8b:	return SFR_8bh;
		case 0x8c:	return SFR_8ch;
		case 0x8d:	return SFR_8dh;
		case 0x8e:	return SFR_8eh;
		case 0x8f:	return SFR_8fh;
                                  
		case 0x90:	return SFR_90h;
		case 0x91:	return SFR_91h;
		case 0x92:	return SFR_92h;
		case 0x93:	return SFR_93h;
		case 0x94:	return SFR_94h;
		case 0x95:	return SFR_95h;
		case 0x96:	return SFR_96h;
		case 0x97:	return SFR_97h;
		case 0x98:	return SFR_98h;
		case 0x99:	return SFR_99h;
		case 0x9a:	return SFR_9ah;
		case 0x9b:	return SFR_9bh;
		case 0x9c:	return SFR_9ch;
		case 0x9d:	return SFR_9dh;
		case 0x9e:	return SFR_9eh;
		case 0x9f:	return SFR_9fh;
                                  
		case 0xa0:	return SFR_a0h;
		case 0xa1:	return SFR_a1h;
		case 0xa2:	return SFR_a2h;
		case 0xa3:	return SFR_a3h;
		case 0xa4:	return SFR_a4h;
		case 0xa5:	return SFR_a5h;
		case 0xa6:	return SFR_a6h;
		case 0xa7:	return SFR_a7h;
		case 0xa8:	return SFR_a8h;
		case 0xa9:	return SFR_a9h;
		case 0xaa:	return SFR_aah;
		case 0xab:	return SFR_abh;
		case 0xac:	return SFR_ach;
		case 0xad:	return SFR_adh;
		case 0xae:	return SFR_aeh;
		case 0xaf:	return SFR_afh;
                                  
		case 0xb0:	return SFR_b0h;
		case 0xb1:	return SFR_b1h;
		case 0xb2:	return SFR_b2h;
		case 0xb3:	return SFR_b3h;
		case 0xb4:	return SFR_b4h;
		case 0xb5:	return SFR_b5h;
		case 0xb6:	return SFR_b6h;
		case 0xb7:	return SFR_b7h;
		case 0xb8:	return SFR_b8h;
		case 0xb9:	return SFR_b9h;
		case 0xba:	return SFR_bah;
		case 0xbb:	return SFR_bbh;
		case 0xbc:	return SFR_bch;
		case 0xbd:	return SFR_bdh;
		case 0xbe:	return SFR_beh;
		case 0xbf:	return SFR_bfh;
                                  
		case 0xc0:	return SFR_c0h;
		case 0xc1:	return SFR_c1h;
		case 0xc2:	return SFR_c2h;
		case 0xc3:	return SFR_c3h;
		case 0xc4:	return SFR_c4h;
		case 0xc5:	return SFR_c5h;
		case 0xc6:	return SFR_c6h;
		case 0xc7:	return SFR_c7h;
		case 0xc8:	return SFR_c8h;
		case 0xc9:	return SFR_c9h;
		case 0xca:	return SFR_cah;
		case 0xcb:	return SFR_cbh;
		case 0xcc:	return SFR_cch;
		case 0xcd:	return SFR_cdh;
		case 0xce:	return SFR_ceh;
		case 0xcf:	return SFR_cfh;
                                  
		case 0xd0:	return SFR_d0h;
		case 0xd1:	return SFR_d1h;
		case 0xd2:	return SFR_d2h;
		case 0xd3:	return SFR_d3h;
		case 0xd4:	return SFR_d4h;
		case 0xd5:	return SFR_d5h;
		case 0xd6:	return SFR_d6h;
		case 0xd7:	return SFR_d7h;
		case 0xd8:	return SFR_d8h;
		case 0xd9:	return SFR_d9h;
		case 0xda:	return SFR_dah;
		case 0xdb:	return SFR_dbh;
		case 0xdc:	return SFR_dch;
		case 0xdd:	return SFR_ddh;
		case 0xde:	return SFR_deh;
		case 0xdf:	return SFR_dfh;
                                  
		case 0xe0:	return SFR_e0h;
		case 0xe1:	return SFR_e1h;
		case 0xe2:	return SFR_e2h;
		case 0xe3:	return SFR_e3h;
		case 0xe4:	return SFR_e4h;
		case 0xe5:	return SFR_e5h;
		case 0xe6:	return SFR_e6h;
		case 0xe7:	return SFR_e7h;
		case 0xe8:	return SFR_e8h;
		case 0xe9:	return SFR_e9h;
		case 0xea:	return SFR_eah;
		case 0xeb:	return SFR_ebh;
		case 0xec:	return SFR_ech;
		case 0xed:	return SFR_edh;
		case 0xee:	return SFR_eeh;
		case 0xef:	return SFR_efh;
                                  
		case 0xf0:	return SFR_f0h;
		case 0xf1:	return SFR_f1h;
		case 0xf2:	return SFR_f2h;
		case 0xf3:	return SFR_f3h;
		case 0xf4:	return SFR_f4h;
		case 0xf5:	return SFR_f5h;
		case 0xf6:	return SFR_f6h;
		case 0xf7:	return SFR_f7h;
		case 0xf8:	return SFR_f8h;
		case 0xf9:	return SFR_f9h;
		case 0xfa:	return SFR_fah;
		case 0xfb:	return SFR_fbh;
		case 0xfc:	return SFR_fch;
		case 0xfd:	return SFR_fdh;
		case 0xfe:	return SFR_feh;
		case 0xff:	return SFR_ffh;

		default: return 0;

	}
}

//=============================================================================
//			Help Message
//=============================================================================
void MCUHelp(void)
{

	Puts("\n=======================================================");
	Puts("\n>>>     Welcome to Intersil Monitor  Rev 1.00     <<<");
	Puts("\n=======================================================");
	Puts("\n   MCU Rs ii             ; Read SFR");
	Puts("\n   MCU Ws ii dd          ; Write SFR");
	Puts("\n   MCU Ds                ; Dump SFR");
	Puts("\n   MCU Bs ii se dd       ; bitwise SFR. s:start bit, e:end bit");
	Puts("\n   MCU sfr               ; Dump SFR map");
	Puts("\n   MCU Rx ii             ; Read XDATA");
	Puts("\n   MCU Wx ii dd          ; Write XDATA");
	Puts("\n   MCU Dx [ii] [cc]      ; Dump XDATA");
#if defined(SUPPORT_WATCHDOG) || defined(DEBUG_WATCHDOG)
	Puts("\n   MCU WDT [0/1/2]       ; WatchDog disable/enable/test");
#endif
	Puts("\n   MCU stop              ; stop mode");
	Puts("\n   MCU IE n              ; enable interrupt");
	Puts("\n   MCU DE n              ; disable interrupt");
	Puts("\n   MCU EI [n]            ; extended interrupt");
	Puts("\n   MCU INT               ; help interrupt");
	Puts("\n");

}
#if 0
void MCUHelp1(void)
{

	Puts1("\n=======================================================");
	Puts1("\n>>>     Welcome to Intersil Monitor  Rev 1.00     <<<");
	Puts1("\n=======================================================");
	Puts1("\n   MCU Rs ii             ; Read SFR");
	Puts1("\n   MCU Ws ii dd          ; Write SFR");
	Puts1("\n   MCU Ds                ; Dump SFR");
	Puts1("\n   MCU Bs ii se dd       ; bitwise SFR. s:start bit, e:end bit");
	Puts1("\n   MCU sfr               ; Dump SFR map");
	Puts1("\n   MCU Rx ii             ; Read XDATA");
	Puts1("\n   MCU Wx ii dd          ; Write XDATA");
	Puts1("\n   MCU Dx [ii] [cc]      ; Dump XDATA");
#if defined(SUPPORT_WATCHDOG) || defined(DEBUG_WATCHDOG)
	Puts1("\n   MCU WDT [0/1/2]       ; WatchDog disable/enable/test");
#endif
	Puts1("\n   MCU stop              ; stop mode");
	Puts1("\n=======IE,DE,EI [n]====================================");
	Puts1("\n   MCU IE n              ; enable interrupt");
	Puts1("\n   MCU DE n              ; disable interrupt");
	Puts1("\n   MCU EI [n]            ; extended interrupt");
	Puts1("\n   MCU INT               ; help interrupt");
	Puts1("\n");

}
#endif

//=============================================================================
//
//=============================================================================
void MonitorMCU(void)
{
	BYTE ret, i, j;
	BYTE idx;
	 
	BYTE argv_num1, argv_num2; //, argv_num3;

	//if ( argc>2) {						 BKFYI: It will hurt a speed
	//	argv_num1 = a2h( argv[2] );
	//	if ( argc>3 ) {
	//		argv_num2 = a2h( argv[3] );
	//		if (argc > 4) {
	//			argv_num3 = a2h( argv[4] );
	//		}
	//	}
	//}

	//---------------------- Write SFR -----------------------
	if( !stricmp( argv[1], "Ws" ) ) {
		if( argc < 4 ) {
			Printf(" ----> Missing parameter !!!");
			return;
		}
		argv_num1 = a2h( argv[2] );
		argv_num2 = a2h( argv[3] );
		ret = WriteSFR(argv_num1, argv_num2);
		if( echo ) {
			Printf("\nMCU SFR Write: %02bx=%02bx (%02bx) ", argv_num1, argv_num2, ret);
			for(i=0; i<8; i++) {
				if( ret & 0x80 ) Puts("1"); else Puts("0");
				ret <<=1;
			}
		}
	}

	//---------------------- Read SFR ------------------------
	else if( !stricmp( argv[1], "Rs" ) ) {
		if( argc < 3 ) {
			Printf(" ----> Missing parameter !!!");
			return;
		}
		argv_num1 = a2h( argv[2] );
		ret = ReadSFR(argv_num1);
		if( echo ) {
			Printf("\nMCU SFR Read: %02bx=%02bx ", argv_num1, ret);
			for(i=0; i<8; i++) {
				if( ret & 0x80 ) Puts("1"); else Puts("0");
				ret <<=1;
			}
		}
	}

	//---------------------- Dump SFR ------------------------
	else if( !stricmp( argv[1], "Ds" ) ) {

		Printf("\nDump DP8051 SFR");
		for (j=0x8; j<=0x0f; j++) {
			Printf("\nSFR %02bx: ", j*0x10);
			for(i=0; i<8; i++) Printf("%02bx ", ReadSFR(j*0x10+i) );
			Printf("- ");
			for(; i<16; i++) Printf("%02bx ", ReadSFR(j*0x10+i) );
		}

	}
	//---------------------- Bitwise SFR control---------------------
	else if( !stricmp( argv[1], "Bs" ) ) {
		Printf("\nSorry!!");
	}
	//---------------------- Dump SFR map---------------------
	else if( !stricmp( argv[1], "SFR" ) ) {
		BYTE len;

		DWORD UsedTime;
		UsedTime = SystemClock;

		Printf("\nDump DP80390 SFR");



		for(j=0x00; j < 0x80; j++) {
			idx = j+0x80;
			for(i=0; i < 0x80; i++) {
				if(SFR_map[i].idx == idx)
					break;	//success
				if(SFR_map[i].idx == 0) {
					i = 0x80;
					break;  //fail
				}
			}
			if((j&0x07) == 0)
				Puts("\n");
			if(i != 0x80) {
				Printf("%02bx:%05s",SFR_map[i].idx, SFR_map[i].name);
				len = TWstrlen(SFR_map[i].name);
				len = 5-len;  //max 5.
				for(i=0; i < len; i++)
					Puts(" ");
				Printf(":%02bx ",ReadSFR(idx));
			}
			else {
			  //Printf("--:-----:--_");   
			  //Printf("            ");   
				Printf("   .....    ");   
			}
		}
		UsedTime = SystemClock - UsedTime;
		Printf("\nUsedTime:%ld.%ldsec", UsedTime/100, UsedTime%100 );

	}
	//---------------------- Dump xdata ----------------------
	else if( !stricmp( argv[1], "Wx" ) ) {

		WORD addr;

		if( argc < 4 ) {
			Printf(" ----> Missing parameter !!!");
			return;
		}
		addr      = a2h( argv[2] );
		argv_num2 = a2h( argv[3] );
		
		*(BYTE xdata *)(addr) = argv_num2;
		ret = *(BYTE xdata *)(addr);
		
		if( echo ) {
			Printf("\nMCU XDATA Write: %04x=%02bx (%02bx) ", addr, argv_num2, ret);
		}
	}

	//---------------------- Dump xdata ----------------------
	else if( !stricmp( argv[1], "Rx" ) ) {

		WORD addr;

		if( argc < 3 ) {
			Printf(" ----> Missing parameter !!!");
			return;
		}
		addr = a2h(argv[2]);

		ret = *(BYTE xdata *)(addr);

		if( echo ) {
			Printf("\nMCU XDATA Read: %04x=%02bx ", addr, ret);
		}
	}

	//---------------------- Dump xdata ----------------------
	else if( !stricmp( argv[1], "Dx" ) ) {

		//data BYTE xdata *osddata = (BYTE xdata *)0xc002;

		WORD addr=0x0000, len=0x100;

		if( argc>=3 ) addr = a2h(argv[2]);
		if( argc>=4 ) len  = a2h(argv[3]);

		Printf("\nDump DP8051 XDATA 0x%04x ", addr);
		for (j=0; j<len/0x10; j++) {
			Printf("\nXDATA %04x: ", (WORD)(addr+j*0x10) );
			for(i=0; i<8; i++) Printf("%02bx ", *(BYTE xdata *)(addr + j*0x10+i) );
			Printf("- ");
			for(; i<16; i++) Printf("%02bx ", *(BYTE xdata *)(addr + j*0x10+i) );
		}

	}

	//---------------------- Enable WDT ------------------------
#if defined(SUPPORT_WATCHDOG) || defined(DEBUG_WATCHDOG)
	else if( !stricmp( argv[1], "WDT" ) ) {
		if(argc > 2) {
			if( argv[2][0]=='1' ) {
				BYTE mode;
				if(argc > 3) 
					mode=a2h(argv[3]);
				else	
					mode = 0;

				EnableWatchdog(mode);
				Printf("\nWatchdog Timer is enabled. mode:%bx", mode);
			}
			else if( argv[2][0]=='2' ) {
				Puts("Watchdog test....");
				RestartWatchdog();
				Puts("start....");
				SFR_EWDI = 0;

				delay1s(1,__LINE__);
				delay1s(1,__LINE__);
				delay1s(1,__LINE__);
				delay1s(1,__LINE__);
				delay1s(1,__LINE__);
				delay1s(1,__LINE__);
				delay1s(1,__LINE__);
				//system reset before you here.
				Puts("..FAIL");
			}
			else {
				DisableWatchdog();
				Printf("\nWatchdog Timer is disabled.");
			}
		}
		else {
			Puts("\nWatchdog ");
			if(SFR_WDCON & 0x02) Puts("On");
			else                 Puts("Off");
#ifdef SUPPORT_WATCHDOG
			Puts(" with SUPPORT_WATCH");
#endif
#ifdef DEBUG_WATCHDOG
			Puts(" with DEBUG_WATCH");
#endif
		}

	}
#endif
	//---------------------- Enable INT ------------------------
	else if( !stricmp( argv[1], "IE" ) ) {
		if ( argc < 3 ) {
			Puts("\nMCU IE n 1/0 - MCU interrupt en/disable #n ");
			return;
		}
		argv_num1 = a2h( argv[2] );

		if( argv[3][0]=='1' ) {
			EnableExtInterrupt( argv_num1 );
			Printf("\nInterrupt #%d is enabled.", argv_num1);
		}
		else {
			DisableExtInterrupt( argv_num1 );
			Printf("\nInterrupt #%d is disabled.", argv_num1);
		}
	}
	//---------------------- Extended INT ------------------------
	else if( !stricmp( argv[1], "DE" ) ) {
		Printf("\nEnable DE-end Interrupt");
		SFR_IE1 = 0;	// Clear Pending Interrupt
		SFR_EX1 = 1;	// Enable Interrupt
	}
	//---------------------- Extended INT ------------------------
	else if( !stricmp( argv[1], "EI" ) ) {
	extern DWORD ExtIntCount;
		if( argc < 3 ) {
			Printf("\nMCU extend Interrupt Status: %02bx, count: %ld", EXINT_STATUS, ExtIntCount );
			ExtIntCount = 0;
#ifdef DEBUG_ISR
			Printf(" MCU %04x", MCU_INT_STATUS);
			MCU_INT_STATUS = 0;
#endif
			return;
		}
		argv_num1 = a2h( argv[2] );

		EXINT_STATUS &= argv_num1;
		
	}
	//---------------------- INTERRUPT Counter---------------------
#ifdef DEBUG_ISR
	else if( !stricmp( argv[1], "INTC" ) ) {
		BYTE i;
		WORD counter;
		for(i=0; i <= 20; i++) {
			SFR_EA=0;
			counter = INTR_counter[i];	//read
			INTR_counter[i]=0;			//clear
			SFR_EA=1;
			
			if(counter) {
				Printf("\n%02bd:%d",i,counter);
			}
		}
	}
#endif
#ifdef DEBUG_UART
	else if( !stricmp( argv[1], "UART" ) )
		DEBUG_dump_uart0();
#if 0
		 {
		BYTE i;
		if(UART0_OVERFLOW_counter) {
			Printf("\nUART0 Overflow:%bd",UART0_OVERFLOW_counter);
			UART0_OVERFLOW_counter = 0;  //clear
		}
		if(UART0_MAX_counter) {
			Printf("\nUART0 Max:%bd",UART0_MAX_counter);
			UART0_MAX_counter = 0;  //clear
		}
		Printf("\nRS_buf[%bd], RS_in:%bd RS_out:%bd",RS_BUF_MAX,RS_in,RS_out);
		for(i=0; i <  RS_BUF_MAX; i++)
			Printf(" %02bx", RS_buf[i]);
	}
#endif
#endif

	//---------------------- Interrupt Help ------------------------
	else if( !stricmp( argv[1], "INT" ) ) {
		Puts("\n #:Description          PORT    Enable  Flag");
		Puts("\n 0:INT0 Chip status     P2.0	A8[0]");
		Puts("\n 1:timer0               Px.x	A8[1]");
		Puts("\n 2:INT1 DE End          P2.1	A8[2]");
		Puts("\n 3:timer1 Tsc           Px.x	A8[3]");
		Puts("\n 4:UART0                Px.x	A8[4]");
		Puts("\n 5:timer2 Remocon       P2.0	A8[5]");
		Puts("\n 6:UART1                Px.x	A8[6]");
		Puts("\n 7:INT2 DMA Done        Px.x	E8[0]");
		Puts("\n 8:INT3 Tsc Ready       Px.x	E8[1]");
		Puts("\n 9:INT4 Reserved        Px.x	E8[2]");
		Puts("\n10:INT5 Reserved        Px.x	E8[3]");
		Puts("\n11:INT6 Pen INTR        Px.x	E8[4]");
		Puts("\n12:Watchdog             Px.x	E8[5]");
		Puts("\n13:INT7                 P1.0	FB[0]  FA[0]");
		Puts("\n14:INT8                 P1.1	FB[1]  FA[1]");
		Puts("\n15:INT9 (Remocon)       P1.2	FB[2]  FA[2]");
		Puts("\n16:INT10                P1.3	FB[3]  FA[3]");
		Puts("\n17:INT11                P1.4	FB[4]  FA[4]");
		Puts("\n18:INT12 (PowerDown     P1.5	FB[5]  FA[5]");
		Puts("\n19:INT13                P1.6	FB[6]  FA[6]");
		Puts("\n20:INT14                P1.7	FB[7]  FA[7]");


		//# on/off
		//

	}
	//---------------------- PMU stop------------------------
	else if( !stricmp( argv[1], "stop" ) ) {
		Printf("\nGoto STOP mode");
		Printf("\nTo resume, write REG0D4[1]=1 by I2C");
		SFR_PCON |= 0x12;
		//----- need nop to clean up the pipeline.
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();		
	}
	//---------------------- Read SFR ------------------------
	else if( !stricmp( argv[1], "H" ) || !stricmp( argv[1], "HELP" ) || !stricmp( argv[1], "?" ) ) {
		MCUHelp();
	}
	//--------------------------------------------------------
	else
		Printf("\nInvalid command...");	
	
	//--------------------------------------------------------
}

#if 0
void Monitor1MCU(void)
{
	BYTE ret, i, j;
	BYTE idx;
	 
	BYTE argv_num1, argv_num2; //, argv_num3;

	//if ( argc>2) {						 BKFYI: It will hurt a speed
	//	argv_num1 = a2h( argv[2] );
	//	if ( argc>3 ) {
	//		argv_num2 = a2h( argv[3] );
	//		if (argc > 4) {
	//			argv_num3 = a2h( argv[4] );
	//		}
	//	}
	//}

	//---------------------- Write SFR -----------------------
	if( !stricmp( argv1[1], "Ws" ) ) {
		if( argc1 < 4 ) {
			Printf1(" ----> Missing parameter !!!");
			return;
		}
		argv_num1 = a2h( argv1[2] );
		argv_num2 = a2h( argv1[3] );
		ret = WriteSFR(argv_num1, argv_num2);
		if( echo ) {
			Printf1("\nMCU SFR Write: %02bx=%02bx (%02bx) ", argv_num1, argv_num2, ret);
			for(i=0; i<8; i++) {
				if( ret & 0x80 ) Puts1("1"); else Puts1("0");
				ret <<=1;
			}
		}
	}

	//---------------------- Read SFR ------------------------
	else if( !stricmp( argv1[1], "Rs" ) ) {
		if( argc1 < 3 ) {
			Printf1(" ----> Missing parameter !!!");
			return;
		}
		argv_num1 = a2h( argv1[2] );
		ret = ReadSFR(argv_num1);
		if( echo ) {
			Printf1("\nMCU SFR Read: %02bx=%02bx ", argv_num1, ret);
			for(i=0; i<8; i++) {
				if( ret & 0x80 ) Puts1("1"); else Puts1("0");
				ret <<=1;
			}
		}
	}

	//---------------------- Dump SFR ------------------------
	else if( !stricmp( argv1[1], "Ds" ) ) {

		Printf1("\nDump DP8051 SFR");
		for (j=0x8; j<=0x0f; j++) {
			Printf1("\nSFR %02bx: ", j*0x10);
			for(i=0; i<8; i++) Printf1("%02bx ", ReadSFR(j*0x10+i) );
			Printf1("- ");
			for(; i<16; i++) Printf1("%02bx ", ReadSFR(j*0x10+i) );
		}

	}
	//---------------------- Bitwise SFR control---------------------
	else if( !stricmp( argv1[1], "Bs" ) ) {

	}
	//---------------------- Dump SFR map---------------------
	else if( !stricmp( argv1[1], "SFR" ) ) {
		BYTE len;
#ifdef SUPPORT_TW8836
		Printf1("\nDump DP8051 SFR");
#else 
		Printf1("\nDump DP8051 SFR");
#endif	
		for(j=0x00; j < 0x80; j++) {
			idx = j+0x80;
			for(i=0; i < 0x80; i++) {
				if(SFR_map[i].idx == idx)
					break;	//success
				if(SFR_map[i].idx == 0) {
					i = 0x80;
					break;  //fail
				}
			}
			if((j&0x07) == 0)
				Puts1("\n");
			if(i != 0x80) {
				Printf1("%02bx:%05s",SFR_map[i].idx, SFR_map[i].name);
				len = TWstrlen(SFR_map[i].name);
				len = 5-len;  //max 5.
				for(i=0; i < len; i++)
					Puts1(" ");
				Printf1(":%02bx ",ReadSFR(idx));
			}
			else {
			  //Printf("--:-----:--_");   
			  //Printf("            ");   
				Printf1("   .....    ");   
			}
		}
	}
	//---------------------- Dump xdata ----------------------
	else if( !stricmp( argv1[1], "Wx" ) ) {

		WORD addr;

		if( argc1 < 4 ) {
			Printf1(" ----> Missing parameter !!!");
			return;
		}
		addr      = a2h( argv1[2] );
		argv_num2 = a2h( argv1[3] );
		
		*(BYTE xdata *)(addr) = argv_num2;
		ret = *(BYTE xdata *)(addr);
		
		if( echo ) {
			Printf1("\nMCU XDATA Write: %04x=%02bx (%02bx) ", addr, argv_num2, ret);
		}
	}

	//---------------------- Dump xdata ----------------------
	else if( !stricmp( argv1[1], "Rx" ) ) {

		WORD addr;

		if( argc1 < 3 ) {
			Printf(" ----> Missing parameter !!!");
			return;
		}
		addr = a2h(argv1[2]);

		ret = *(BYTE xdata *)(addr);

		if( echo ) {
			Printf1("\nMCU XDATA Read: %04x=%02bx ", addr, ret);
		}
	}

	//---------------------- Dump xdata ----------------------
	else if( !stricmp( argv1[1], "Dx" ) ) {

		//data BYTE xdata *osddata = (BYTE xdata *)0xc002;

		WORD addr=0x0000, len=0x100;

		if( argc>=3 ) addr = a2h(argv1[2]);
		if( argc>=4 ) len  = a2h(argv1[3]);

		Printf1("\nDump DP8051 XDATA 0x%04x ", addr);
		for (j=0; j<len/0x10; j++) {
			Printf1("\nXDATA %04x: ", (WORD)(addr+j*0x10) );
			for(i=0; i<8; i++) Printf1("%02bx ", *(BYTE xdata *)(addr + j*0x10+i) );
			Printf1("- ");
			for(; i<16; i++) Printf1("%02bx ", *(BYTE xdata *)(addr + j*0x10+i) );
		}

	}

	//---------------------- Enable WDT ------------------------
#if defined(SUPPORT_WATCHDOG) || defined(DEBUG_WATCHDOG)
	else if( !stricmp( argv1[1], "WDT" ) ) {
		if(argc1 > 2) {
			if( argv1[2][0]=='1' ) {
				BYTE mode;
				if(argc1 > 3) 
					mode=a2h(argv1[3]);
				else	
					mode = 0;

				EnableWatchdog(mode);
				Printf1("\nWatchdog Timer is enabled. mode:%bx", mode);
			}
			else if( argv1[2][0]=='2' ) {
				Puts1("Watchdog test....");
				RestartWatchdog();
				Puts1("start....");
				delay1s(1,__LINE__);
				delay1s(1,__LINE__);
				delay1s(1,__LINE__);
				delay1s(1,__LINE__);
				delay1s(1,__LINE__);
				//system reset before you here.
				Puts1("..FAIL");
			}
			else {
				DisableWatchdog();
				Printf1("\nWatchdog Timer is disabled.");
			}
		}
		else {
			Puts1("\nWatchdog ");
			if(SFR_WDCON & 0x02) Puts1("On");
			else                 Puts1("Off");
#ifdef SUPPORT_WATCHDOG
			Puts1(" with SUPPORT_WATCH");
#endif
#ifdef DEBUG_WATCHDOG
			Puts1(" with DEBUG_WATCH");
#endif
		}

	}
#endif
	//---------------------- Enable INT ------------------------
	else if( !stricmp( argv1[1], "IE" ) ) {
		if ( argc < 3 ) {
			Puts1("\nMCU IE n 1/0 - MCU interrupt en/disable #n ");
			return;
		}
		argv_num1 = a2h( argv1[2] );

		if( argv1[3][0]=='1' ) {
			EnableExtInterrupt( argv_num1 );
			Printf1("\nInterrupt #%d is enabled.", argv_num1);
		}
		else {
			DisableExtInterrupt( argv_num1 );
			Printf1("\nInterrupt #%d is disabled.", argv_num1);
		}
	}
	//---------------------- Extended INT ------------------------
	else if( !stricmp( argv1[1], "DE" ) ) {
		Printf1("\nEnable DE-end Interrupt");
		SFR_IE1 = 0;	// Clear Pending Interrupt
		SFR_EX1 = 1;	// Enable Interrupt
	}
	//---------------------- Extended INT ------------------------
	else if( !stricmp( argv1[1], "EI" ) ) {
	extern DWORD ExtIntCount;
		if( argc1 < 3 ) {
			Printf1("\nMCU extend Interrupt Status: %02bx, count: %ld", EXINT_STATUS, ExtIntCount );
			ExtIntCount = 0;
#ifdef DEBUG_ISR
			Printf1(" MCU %04x", MCU_INT_STATUS);
			MCU_INT_STATUS = 0;
#endif
			return;
		}
		argv_num1 = a2h( argv1[2] );

		EXINT_STATUS &= argv_num1;
		
	}
	//---------------------- Interrupt Help ------------------------
	else if( !stricmp( argv1[1], "INT" ) ) {
		Puts1("\n #:Description          PORT    Enable  Flag");
		Puts1("\n 0:INT0 Chip status     P2.0	A8[0]");
		Puts1("\n 1:timer0               Px.x	A8[1]");
		Puts1("\n 2:INT1 DE End          P2.1	A8[2]");
		Puts1("\n 3:timer1 Tsc           Px.x	A8[3]");
		Puts1("\n 4:UART0                Px.x	A8[4]");
		Puts1("\n 5:timer2 Remocon       P2.0	A8[5]");
		Puts1("\n 6:UART1                Px.x	A8[6]");
		Puts1("\n 7:INT2 DMA Done        Px.x	E8[0]");
		Puts1("\n 8:INT3 Tsc Ready       Px.x	E8[1]");
		Puts1("\n 9:INT4 Reserved        Px.x	E8[2]");
		Puts1("\n10:INT5 Reserved        Px.x	E8[3]");
		Puts1("\n11:INT6 Pen INTR        Px.x	E8[4]");
		Puts1("\n12:Watchdog             Px.x	E8[5]");
		Puts1("\n13:INT7                 P1.0	FB[0]  FA[0]");
		Puts1("\n14:INT8                 P1.1	FB[1]  FA[1]");
		Puts1("\n15:INT9 (Remocon)       P1.2	FB[2]  FA[2]");
		Puts1("\n16:INT10                P1.3	FB[3]  FA[3]");
		Puts1("\n17:INT11                P1.4	FB[4]  FA[4]");
		Puts1("\n18:INT12 (PowerDown     P1.5	FB[5]  FA[5]");
		Puts1("\n19:INT13                P1.6	FB[6]  FA[6]");
		Puts1("\n20:INT14                P1.7	FB[7]  FA[7]");


		//# on/off
		//

	}
	//---------------------- PMU stop------------------------
	else if( !stricmp( argv1[1], "stop" ) ) {
		Printf1("\nGoto STOP mode");
		Printf1("\nTo resume, write REG0D4[1]=1 by I2C");
		SFR_PCON |= 0x12;
		//----- need nop to clean up the pipeline.
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();		
	}
	//---------------------- Read SFR ------------------------
	else if( !stricmp( argv1[1], "H" ) || !stricmp( argv1[1], "HELP" ) || !stricmp( argv1[1], "?" ) ) {
		MCUHelp1();
	}
	//--------------------------------------------------------
	else
		Printf1("\nInvalid command...");	
	
	//--------------------------------------------------------
}


#endif

