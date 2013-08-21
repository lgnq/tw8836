//*****************************************************************************
//
//									Printf
//
//  Copyright(C) 2011-2012 Intersil Corporation
// use %bx or %bd for BYTE.
//*****************************************************************************
//
//
#include "Config.h"
#include "reg.h"
#include "Typedefs.h"
#include "TW8836.h"

#include "CPU.h"
#include "main.h"
#include "debug.h"
#include "printf.h"
#include <stdarg.h>
#include "Global.h"
#include "inputctrl.h"

CODE BYTE Hex[] = "0123456789ABCDEF";	//static

BYTE FOsdMsgBuff[40];  //for FontOsd Message

//=============================================================================
//                                                                           
//=============================================================================
void DoPrint( const char *fmt, va_list ap )
{
	char  ch;
	char  i;
	long  value;
	bit   fl_zero;
	bit   fl_num;
	BYTE  fl_len;
	BYTE  cnt;
	DWORD mask=1;

	char *ptr;

	while(1) {
		
		//----- Find Formatter % -----

		switch( ch = *fmt++ ) {
			case 0:		return;
			case '%':	if( *fmt != '%' ) break;
						fmt++;
			default:	_outchar( ch );
						continue;
		}

		//----- Get Count -------------
		
		fl_zero = 0;
		fl_num = 0;
		cnt = 0;

		ch = *fmt++;

		if( ch=='0' ) {
			fl_zero = 1;
			ch = *fmt++;
			cnt = ch - '0';
			ch = *fmt++;
		}
		else if( ch>='0' && ch<='9' ) {
			cnt = ch - '0';
			ch = *fmt++;
		}

		//----- Get char(B) / int / long(L) ----------------

		fl_len = 2;

		switch(ch) {
		case 'l':
		case 'L':	ch = *fmt++;	fl_len = 4;		break;
		case 'b':
		case 'B':	ch = *fmt++;	fl_len = 1;		break;
		}		

		//----- Get Type Discriptor -----
		
		switch( ch ) {

			case 'd':
			case 'u':

				switch (fl_len) {
				case 1:
					if( ch=='d' ) value = (char)va_arg( ap, char );
					else          value = (BYTE)va_arg( ap, BYTE );
					break;

				case 2:
					if( ch=='d' ) value = (int)va_arg( ap,  int );
					else          value = (WORD)va_arg( ap, WORD );
					break;

				case 4:
					if( ch=='d' ) value = (long)va_arg( ap, long );
					else          value = (DWORD)va_arg( ap, DWORD );
					break;
				}

				if( value<0 ) {
					_outchar('-');
					value = value*(-1);
				}

				if(cnt==0) {
					if( value==0 ) { _outchar('0'); continue; }

					for(cnt=0, mask=1; cnt<10; cnt++) {
						if( (value/mask)==0 ) break;
						mask = mask*10;
					}
				}

				for(i=0, mask=1; i<cnt-1; i++) mask = mask*10;

				while(1) {
					ch = (value / mask) + '0';
					if( ch=='0' && fl_zero==0 && mask!=1 ) ch=' ';
					else fl_zero = 1;
					_outchar(ch);

					value = value % (mask);
					mask = mask / 10;
					
					if( mask==0 )
						break;
				}
				continue;

			case 'x':
			case 'X':

				switch (fl_len) {
				case 1:	value = (BYTE)va_arg( ap, BYTE );		break;
				case 2:	value = (WORD)va_arg( ap, WORD );		break;
				case 4:	value = (DWORD)va_arg( ap, DWORD );		break;
				}

				if(cnt==0) cnt = fl_len*2;

				for(i=0; i<cnt; i++) {
					_outchar( Hex[(value >> (cnt-i-1)*4) & 0x000f] );
				}
				continue;

			case 's':

				ptr = (char *)va_arg( ap, char* );
				while(*ptr!='\0')
					_outchar(*ptr++);
				continue;

			case 'c':
				value = va_arg( ap, char );
					_outchar((BYTE)value);
				continue;

			default:
				value = (WORD)va_arg( ap, int );
				continue;
		}
	}
}

#ifdef SUPPORT_UART1
void DoPrint1( const char *fmt, va_list ap )
{
	char  ch;
	char  i;
	long  value;
	bit   fl_zero;
	bit   fl_num;
	BYTE  fl_len;
	BYTE  cnt;
	DWORD mask=1;

	char *ptr;

	while(1) {
		
		//----- Find Formatter % -----

		switch( ch = *fmt++ ) {
			case 0:		return;
			case '%':	if( *fmt != '%' ) break;
						fmt++;
			default:	_outchar1( ch );
						continue;
		}

		//----- Get Count -------------
		
		fl_zero = 0;
		fl_num = 0;
		cnt = 0;

		ch = *fmt++;

		if( ch=='0' ) {
			fl_zero = 1;
			ch = *fmt++;
			cnt = ch - '0';
			ch = *fmt++;
		}
		else if( ch>='0' && ch<='9' ) {
			cnt = ch - '0';
			ch = *fmt++;
		}

		//----- Get char(B) / int / long(L) ----------------

		fl_len = 2;

		switch(ch) {
		case 'l':
		case 'L':	ch = *fmt++;	fl_len = 4;		break;
		case 'b':
		case 'B':	ch = *fmt++;	fl_len = 1;		break;
		}		

		//----- Get Type Discriptor -----
		
		switch( ch ) {

			case 'd':
			case 'u':

				switch (fl_len) {
				case 1:
					if( ch=='d' ) value = (char)va_arg( ap, char );
					else          value = (BYTE)va_arg( ap, BYTE );
					break;

				case 2:
					if( ch=='d' ) value = (int)va_arg( ap,  int );
					else          value = (WORD)va_arg( ap, WORD );
					break;

				case 4:
					if( ch=='d' ) value = (long)va_arg( ap, long );
					else          value = (DWORD)va_arg( ap, DWORD );
					break;
				}

				if( value<0 ) {
					_outchar1('-');
					value = value*(-1);
				}

				if(cnt==0) {
					if( value==0 ) { _outchar1('0'); continue; }

					for(cnt=0, mask=1; cnt<10; cnt++) {
						if( (value/mask)==0 ) break;
						mask = mask*10;
					}
				}

				for(i=0, mask=1; i<cnt-1; i++) mask = mask*10;

				while(1) {
					ch = (value / mask) + '0';
					if( ch=='0' && fl_zero==0 && mask!=1 ) ch=' ';
					else fl_zero = 1;
					_outchar1(ch);

					value = value % (mask);
					mask = mask / 10;
					
					if( mask==0 )
						break;
				}
				continue;

			case 'x':
			case 'X':

				switch (fl_len) {
				case 1:	value = (BYTE)va_arg( ap, BYTE );		break;
				case 2:	value = (WORD)va_arg( ap, WORD );		break;
				case 4:	value = (DWORD)va_arg( ap, DWORD );		break;
				}

				if(cnt==0) cnt = fl_len*2;

				for(i=0; i<cnt; i++) {
					_outchar1( Hex[(value >> (cnt-i-1)*4) & 0x000f] );
				}
				continue;

			case 's':

				ptr = (char *)va_arg( ap, char* );
				while(*ptr!='\0')
					_outchar1(*ptr++);
				continue;

			case 'c':
				value = va_arg( ap, char );
					_outchar1((BYTE)value);
				continue;

			default:
				value = (WORD)va_arg( ap, int );
				continue;
		}
	}
}
#endif

//=============================================================================
//                                                                           
//=============================================================================
//BKTODO: If compiler use a register parameter, it will have a trouble.
/**
* printf
*
* use %bx or %bd for BYTE.
*/
void Printf( const char *fmt, ... )
{
	va_list ap;

	va_start(ap, fmt);
	DoPrint( fmt, ap );
	va_end( ap );
}
/**
* puts
*/
void Puts( char *ptr )
{
	while(*ptr!='\0')
		RS_tx(*ptr++);
}
#ifdef SUPPORT_UART1
void Printf1( const char *fmt, ... )
{
	va_list ap;

	va_start(ap, fmt);
	DoPrint1( fmt, ap );
	va_end( ap );
}

void Puts1( char *ptr )
{
	while(*ptr!='\0')
		RS1_tx(*ptr++);
}
#endif

//=============================================================================
//                                                                           
//=============================================================================
#ifdef DEBUG
/**
* Printf debug level 3
*/
void dPrintf( const char *fmt, ... )
{
	va_list ap;

	if( DebugLevel >= DEBUG_INFO ) {
		va_start(ap, fmt);
		DoPrint( fmt, ap );
		va_end( ap );
	}
}

/**
* Printf debug level 2
*/
void wPrintf( const char *fmt, ... )
{
	va_list ap;

	if( DebugLevel >= DEBUG_WARN ) {
		va_start(ap, fmt);
		DoPrint( fmt, ap );
		va_end( ap );
	}
}

/**
* Printf debug level 1
*/
void ePrintf( const char *fmt, ... )
{
	va_list ap;

	if( DebugLevel >= DEBUG_ERR ) {
		va_start(ap, fmt);
		DoPrint( fmt, ap );
		va_end( ap );
	}
}

//=============================================================================
//                                                                           
//=============================================================================

/**
* Puts debug level 3
*/
void dPuts( char *ptr )
{
	if( DebugLevel >= DEBUG_INFO ) {
		while(*ptr!='\0')
			RS_tx(*ptr++);
	}
}

/**
* Puts debug level 2
*/
void wPuts( char *ptr )
{
	if( DebugLevel >= DEBUG_WARN ) {
		while(*ptr!='\0')
			RS_tx(*ptr++);
	}
}

/**
* Puts debug level 1
*/
void ePuts( char *ptr )
{
	if( DebugLevel >= DEBUG_ERR ) {
		while(*ptr!='\0')
			RS_tx(*ptr++);
	}
}
#endif // DEBUG

#if 0
//-----------------------------------------------------------------------------
/**
* read & print the chip register
*/
void	DebugRegister( WORD reg )
{
	Printf("\nLine No=%d, Reg[0x%03x] = 0x%02bx", __LINE__, reg, ReadTW88(reg) );
}
#endif


//=============================================================================
//                                                                           
//=============================================================================
/**
* print SystemClock
*
* use debug level 1
*/
void PrintSystemClockMsg(char *msg)
{
	DWORD time;
	SFR_EA=0;
	time = SystemClock;
	SFR_EA=1;
	ePrintf("\nSystemTime:%ld.%02ld %s", time/100, time%100, msg );
	
}
