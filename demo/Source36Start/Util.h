#ifndef __UTIL_H__
#define __UTIL_H__

#define _between(x, a, b)	(a<=x && x<=b)
#define _swap(a, b)			{ a^=b; b^=a; a^=b; }



DWORD a2i(BYTE *str);
DWORD a2h(BYTE *str);

BYTE Asc1Bin(BYTE asc);
//BYTE Asc2Bin(BYTE *s);
WORD Asc4Bin(BYTE *s);
BYTE toupper(BYTE ch);
int stricmp(BYTE *ptr1, BYTE *ptr2);

BYTE TWitoa(short n, BYTE *buff);
BYTE TWhtos(BYTE n, BYTE *buff);
	#define NUM_RADIX				10
	#define HEX_RADIX				16
BYTE TWutoa(WORD value, BYTE *str, BYTE radix);
BYTE TWstrlen(BYTE *s);
BYTE *TWstrcpy(BYTE *d,BYTE *s);
BYTE *TWstrcat(BYTE *d,BYTE *s);
BYTE IsDigit(BYTE c);
short TWabsShort(short value);
BYTE SPI_cmd_protocol(BYTE max, ...);
void CheckRegister(BYTE page, BYTE reg, WORD line);


int TWmemcmp( const void * ptr1, const void * ptr2, WORD num );
void * TWmemcpy ( void * destination, const void * source, WORD num );
void * TWmemset ( void * ptr, int value, WORD num );

void I2CDeviceInitialize(BYTE *RegSet, BYTE delay);
#endif // __UTIL_H__


