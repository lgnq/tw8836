#ifndef __E3PROM_H__
#define __E3PROM_H__ 

/* external variable */
extern BYTE DebugLevel;
/* external functions */
extern void Printf ( const char *fmt, ... );
extern void dPrintf( const char *fmt, ... );
extern void E3P_SectorErase( DWORD spiaddr );
extern void E3P_PageProgram( DWORD spiaddr, BYTE * xaddr, WORD cnt );
extern void E3P_Read2XMem(BYTE * dest_loc, DWORD src_loc, WORD size);


BYTE E3P_SetStartAddr(DWORD addr);				//set SpiFlash start address for E3PROM.
BYTE E3P_SetBuffer(BYTE XDATA *p, WORD len);	//max 256. default 128.
BYTE E3P_SetSize(BYTE index, BYTE blocks);		//index default 64, blocks default 8.

BYTE E3P_Init(void);		
void E3P_Format(void);		
void E3P_Repair(void);		
void E3P_Clean(void);		
void E3P_Check(void);		
void E3P_DumpBlocks(BYTE with_block);

void E3P_PrintInfo(void);	
BYTE E3P_GetVersion(void);


BYTE E3P_Read(WORD index);
void E3P_Write(WORD index, BYTE value);
#endif