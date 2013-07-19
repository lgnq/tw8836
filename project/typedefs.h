#ifndef	__TYPEDEFS_H__
#define	__TYPEDEFS_H__

#include <intrins.h>

#define DATA		data
#define PDATA		pdata
#define IDATA		idata
#define XDATA		xdata
#define CODE		code
#define CONST		code
#define CODE_P
#define FAR

//typedef	unsigned char	Register;
typedef	unsigned char	BYTE;
typedef	unsigned int	WORD;
//typedef	unsigned long	DWORD;

#define	TRUE	1
#define	FALSE	0
#ifndef NULL
 #define NULL ((void *) 0)
#endif
#define NIL		0xFF

#define ERR_SUCCESS		0
#define ERR_FAIL		1

#define ON		1
#define OFF		0

//VInputStdDetectMode
#define AUTO	0

//VInputStd
#define NTSC	1			
#define PAL		2
#define SECAM	3
#define NTSC4	4
#define PALM	5
#define PALN	6
#define PAL60	7

// SUPPORT  HAN     VAN     VFREQ  	HTOTAL  VTOTAL  HS  VS	Hst  	Vst		OffsetH OffsetV Dummy0 	Dummy1 	Dummy2 	Dummy3
#define HPOL_P	0x40	//Positive
#define HPOL_N	0x00	//Negative
#define VPOL_P	0x80
#define VPOL_N	0x00

struct _PCMODEDATA{
	BYTE	support;
	WORD	hActive;
	WORD	vActive;
	BYTE	vFreq;
	WORD	hTotal;
	WORD	vTotal;
	BYTE	syncpol;	//HPOL_ & VPOL_
	WORD	hstart;		//hSync+hBPorch
	WORD	vstart;		//vSync+vBPorch
	char    vDeOffset;	//note: signed value
//	WORD	offseth;	//
//	WORD	offsetv;	//
//	BYTE	dummy0;
//	BYTE	dummy1;
//	WORD	dummy2;
//	BYTE	dummy3;
} ;

struct RegisterInfo
{
   int	Min;
   int	Max;
   int	Default;
};
struct LongRegisterInfo
{
   WORD	Min;
   WORD	Max;
   WORD	Default;
};


#endif // __TYPEDEFS_H__
