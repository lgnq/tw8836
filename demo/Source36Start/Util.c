/**
 * @file
 * util.c 
 * @author Harry Han
 * @author YoungHwan Bae
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	utility library
 *
*/
#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"
#include <stdarg.h>
#include <intrins.h>

#include "cpu.h"
#include "Util.h"
#include "printf.h"
#include "Global.h"
#include "i2c.h"

//=============================================================================
//		Convert ASCII to Binery                                             
//		Convert ASCII to ASCII HEX
//		Convert ASCII to Integer                                    
//=============================================================================

//-----------------------------------------------------------------------------
/**
* return binary value of 'asc' or null for error
*/
BYTE Asc1Bin(BYTE asc)
{
	if(asc>='0' && asc <='9') return (asc - '0');
	if(asc>='a' && asc <='f') return (asc - 'a' + 0x0a);
	if(asc>='A' && asc <='F') return (asc - 'A' + 0x0a);
	return 0;
}
//-----------------------------------------------------------------------------
/**
* Convert *str ASCII to HEX (8 digits max)
*/
DWORD a2h(BYTE *str)
{
	DWORD num=0;
	BYTE i;

	for(i=0; i<8; i++, str++) {
		if( *str=='\0' || *str==' ' ) break;
		num = num*0x10 + Asc1Bin(*str);
	}
	return num;
}
//-----------------------------------------------------------------------------
/**
* Convert *str ASCII to integer(DOUBLE size)
*/
DWORD a2i(BYTE *str)
{
	DWORD num=0;
	BYTE i;

	for(i=0; ; i++, str++) {
		if( *str=='\0' || *str==' ' ) break;
		num = num*10 + *str - '0';
	}
	return num;
}

//=============================================================================
//		Convert to Upper Case
//		String Compare
//		Integer to Ascii
//		Return string length
//		Copy String
//		Cat String
//=============================================================================

//-----------------------------------------------------------------------------
/** 
* return uppercase ASCII character of 'ch'
*/
BYTE toupper(BYTE ch)
{
	if( ch>='a' && ch<='z' )
		return (ch - 'a' + 'A');

	return ch;
}

//-----------------------------------------------------------------------------
/**
* Compare *ptr1 to *ptr2. Return 1 for no compare, return 0 for compare 
*
* *ptr1 and *ptr2 modified
* Case insensitve
* Length of *ptr1 and *ptr2 must be equal to compare
*/
int stricmp(BYTE *ptr1, BYTE *ptr2)
{
#if 0
	int	i, ret;
	
	for(i=0; *ptr1; i++) {
		ret = toupper(*ptr1++) - toupper(*ptr2++);
		if( ret ) return ret;
	}
	ret = toupper(*ptr1) - toupper(*ptr2);
	if( ret ) return ret;

	return 0;
#else
	while (1) {
		if( toupper(*ptr1) != toupper(*ptr2) ) return 1;
		if (*ptr1 == 0) return 0;
		ptr1++;
		ptr2++;
	}
#endif
}


//-----------------------------------------------------------------------------
/**
* itoa
*
* max 10000
* @param n 		input number
* @ buff *buff	output buffer. 
*	I do not check buff size. buff[] have to be big enough.	
* @return
*	- 0 something wrong
*	- n length of ascii character
*/
#if 0
BYTE TWitoa(short n, BYTE *buff)
{
	BYTE next,start;
	WORD k, q;

	next = 0;
	start = 0;

	if(n < 0) {
		buff[next++] = '-';
		n = 0 - n;
		start++;
	}
	if(n == 0)
		buff[next++] = '0';
	else 
	{
		k = 10000;		 //max
		while( k > 0) 
		{
			if(n==0) 	q = 0;
			else		q = n / k;
			if((next!=start) || (q > 0)) { 
				buff[next++] = '0' + q;
			}
			n -= (q*k);
			k = k / 10;
		} 
	}
	buff[next] = 0x00;	//make string.		  
	return next;
}
#endif

//-----------------------------------------------------------------------------
/**
* convert hex byte to ASCII string
*/
#if 0
BYTE TWhtos(BYTE n, BYTE *buff)
{
	BYTE nibble;
	nibble = n >> 4;
	if(nibble <= 9)	buff[0] = '0'+nibble;
	else            buff[0] = 'A'+nibble - 0x0A;
	nibble = n & 0x0F;
	if(nibble <= 9)	buff[1] = '0'+nibble;
	else            buff[1] = 'A'+nibble - 0x0A;
	buff[2] = 0;
	if(n >> 4)
		return 1;
	return 0;		
}
#endif

#ifdef SUPPORT_FOSD_MENU
//-----------------------------------------------------------------------------
/**
* convert uchar to string ASCII string. use radix
*/
BYTE TWutoa(WORD value, BYTE *str, BYTE radix)
{
	BYTE cnt, *str1;
	WORD i, div;

	#ifdef DEBUG_OSD
//	dPrintf("\r\n++(utoa):0x%x__", value);
	#endif

	str1 = str;
	cnt=0;
	if( radix==10) {
		for(div = 10000; div>=10; div/=10) {
			i = value/div;
			if( i ) {
				*str++ = i+'0';
				cnt++;
			}
			else if( cnt )		// 100, 200, 20,,,,
				*str++ = '0';

			value%=div;
		}
		*str++ = value+'0';
		*str = 0;
	}
	else if( radix==16) {
		for(div = 0x1000; div>=0x10; div/=0x10) {
			i = value/div;
			if( i ) {
				if( i>=0xa ) 
					*str++ = i-0xa+'A';
				else
					*str++ = i+'0';
				value%=div;
			}
		}
		if( value>=0xa ) 
			*str++ = value-0xa+'A';
		else
			*str++ = value+'0';
		*str = 0;
	}
	return 1;
}
#endif

#if 0
//-----------------------------------------------------------------------------
/**
* strlen
*/
BYTE strlen( BYTE *str )
{
	BYTE i=0;

	while( *str++ ) i++;
	return i;
}
#endif
#if 0
//-----------------------------------------------------------------------------
/**
* return legth of *s string
*/
BYTE TWstrlen(BYTE *s)
{
	BYTE *p = s;
	while(*p) p++;
	return p - s;
}
#endif
//-----------------------------------------------------------------------------
/**
* return legth of *s string. check an escape code.
*
* support an ESCAPE char( >= 0xF0)
*/
BYTE TWstrlen(BYTE *s)
{
	BYTE *p = s;
	BYTE escape=0;
	while(1) {
		if(*p >= 0xF0) {  //??Escape
			escape = 1;
			if(*p==0xFF/*FONT_ROM*/)
				escape = 0;
			p++;
			continue;
		}
		if(escape) {
			p++;
		}
		else {
			if(*p==0)
				break;
			p++;
		}
	}
	return p - s;
}

//-----------------------------------------------------------------------------
/**
* copy *s into *d. 
*
* note, *d must have same or more buffer reserved as *s
*/
#if 0
BYTE *TWstrcpy(BYTE *d,BYTE *s)
{
	BYTE *dd = d;
	while(*s)
		*dd++ = *s++;
	*dd = 0;

	return d;
}
#endif

//-----------------------------------------------------------------------------
/**
* append *s onto *d. note, *d must have buffer reserved for *s
*/
#if 0
BYTE *TWstrcat(BYTE *d,BYTE *s)
{
	BYTE *dd = d;
	while(*dd)
		dd++;
	while(*s)
		*dd++ = *s++;
	*dd = 0;

	return d;
}
#endif
//-----------------------------------------------------------------------------
/**
* is a digit character
*/
#if 0
BYTE IsDigit(BYTE c)
{
	if(c >= 0x30 && c <= 0x39)
		return 1;
	return 0;
}
#endif

#if 0 //defined(SUPPORT_TOUCH)
//-----------------------------------------------------------------------------
/**
* ABS for short 
*/
short TWabsShort(short value)
{
	if(value >0)	return value;
	return -value;
}
#endif



//=============================================================================
//memcmp                                             
//memcpy
//memset                                   
//=============================================================================

#if defined(SUPPORT_HDMI_SiIRX)
/**
* memcmp
*
* same: int memcmp( const void * ptr1, const void * ptr2, size_t num )
*/
int TWmemcmp( const void * ptr1, const void * ptr2, WORD num )
{
	BYTE *d,*s;
	WORD i;
	d = ptr1;
	s = ptr2;
	for(i=1; i <= num; i++) {
		if(d > s)	return i;
		if(d < s)   return -i;
	}
	return 0;

}
/**
* memcpy
*/
void *TWmemcpy( void * destination, const void * source, WORD num )
{
	BYTE *d,*s;
	d = destination;
	s = source;
	while(num--) {
		*d++ = *s++;
	}
	return destination;

}
#endif
#if defined(SUPPORT_HDMI)
/**
* memset
*/
void *TWmemset( void * ptr, int value, WORD num )
{
	BYTE *d;
	d = ptr;
	while(num--) {
		*d++ = (BYTE)value;
	}
	return ptr;
}
#endif


#if 0
//-----------------------------------------------------------------------------
/**
* check a register value
*
* only for debug
*/
void CheckRegister(BYTE page, BYTE reg, WORD line)
{
	WORD addr;
	addr = page;
	addr <<= 8;
	addr |= reg;
	Printf("\nCHECK:%d 0x03x:%bx",line, addr, ReadTW88(addr));
}
#endif

/**
* initialize registers with text array
*
*	format
*		0xII, 0x00	//start. If II is not 00, use WriteI2CByte.  
*		0xff, 0xXX	//assign page
*		0xRR, 0xDD	//register & data
*		...
*		0xff, 0xXX	//assign page
*		0xRR, 0xDD	//register & data
*		...
*		0xff, 0xff	//end
*/
#if 1
void I2CDeviceInitialize(BYTE *RegSet, BYTE delay)
{
	int	cnt=0;
	BYTE addr, index, val;
	WORD w_page=0;
//	BYTE speed;

	addr = *RegSet;
#ifdef DEBUG_TW88
	dPrintf("\nI2C address : %02bx", addr);
#endif
	cnt = *(RegSet+1);	//ignore cnt
	RegSet+=2;

//	if(addr)
//		speed = SetI2CSpeed(I2C_SPEED_SLOW);


	while (( RegSet[0] != 0xFF ) || ( RegSet[1]!= 0xFF )) {			// 0xff, 0xff is end of data
		index = *RegSet;
		val = *(RegSet+1);

		if ( addr == 0 ) {
			if(index==0xFF) {
				w_page=val << 8;
			}
			else {
				WriteTW88(w_page+index, val);
			}
		}
		else
			WriteI2CByte(addr, index, val);

		if(delay)
			delay1ms(delay);
#ifdef DEBUG_TW88
		dPrintf("\n    addr=%02x  index=%03x   val=%02x", (WORD)addr, w_page | index, (WORD)val );
#endif
		RegSet+=2;
	}

//	if(addr)
//		SetI2CSpeed(speed);
															   
}
#endif

#if 0
void WriteBlockTW88(WORD reg, BYTE *Buff, BYTE len)
{
	BYTE i;
	for(i=0; i < len; i++)
		WriteTW88(reg+i, Buff[i]);
}
#endif

//-----------------------------------------------------------------------------
/**
* wait Vertical Blank
*
* You can use this function after you turn off the PD_SSPLL(REG0FC[7]).
* 0xFFFE value uses max 40ms on Cache + 72MHz.
*/
#define VBLANK_WAIT_VALUE	0xFFFE 
void WaitVBlank(BYTE cnt)
{
	XDATA	BYTE i;
	WORD loop;
	DECLARE_LOCAL_page

	ReadTW88Page(page);
	WriteTW88Page(PAGE0_GENERAL );

	for ( i=0; i<cnt; i++ ) {
		WriteTW88(REG002, 0xff );
		loop = 0;
		while (!( ReadTW88(REG002 ) & 0x40 ) ) {
			// wait VBlank
			loop++;
			if(loop > VBLANK_WAIT_VALUE  ) {
				wPrintf("\nERR:WaitVBlank");
				break;
			}
		}		
	}
	WriteTW88Page(page);
}
