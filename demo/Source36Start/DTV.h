#ifndef __DTV_H__
#define	__DTV_H__

#define DTV_ROUTE_RGB	0
#define DTV_ROUTE_RBG	1
#define DTV_ROUTE_BGR	2
#define DTV_ROUTE_BRG	3
#define DTV_ROUTE_GBR	4
#define DTV_ROUTE_GRB	5

#define DTV_ROUTE_PrYPb	0
#define DTV_ROUTE_PrPbY	1
#define DTV_ROUTE_PbYPr	2
#define DTV_ROUTE_PbPrY	3
#define DTV_ROUTE_YPbPr	4
#define DTV_ROUTE_YPrPb	5

#define DTV_ROUTE_565_MSB_B_LSB_R_REVERSED	1



#define DTV_FORMAT_INTERLACED_ITU656	0
#define DTV_FORMAT_PROGRESSIVE_ITU656	1
#define DTV_FORMAT_8BIT_601				2
#define DTV_FORMAT_16BIT_601			3
#define DTV_FORMAT_24BIT_601			4
#define DTV_FORMAT_RGB					5
#define DTV_FORMAT_ITU1120				6
#define DTV_FORMAT_SMP_702P				7
#define DTV_FORMAT_RGB565				8	

void DtvSetPolarity(BYTE HPol, BYTE VPol);
void DtvSetClockDelay(BYTE delay);

void DtbSetReverseBusOrder(BYTE fReverse);
void DtvSetRouteFormat(BYTE route, BYTE format);
void DtvSetFieldDetectionRegion(BYTE fOn, BYTE r054);
void DtvSetVSyncDelay(BYTE value);


BYTE CheckAndSetDVI( void );
BYTE ChangeDVI( void );

BYTE CheckAndSetHDMI( void );
BYTE ChangeHDMI(void);

BYTE CheckAndSetBT656Loop(void);
BYTE ChangeBT656Loop( void );

BYTE CheckAndSetLVDSRx(void);
BYTE ChangeLVDSRx(void);


void Init_HdmiSystem(void);
 
#endif //..__DTV_H__