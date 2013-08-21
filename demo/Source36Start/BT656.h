#ifndef __BT656_H__
#define	__BT656_H__

#define BT656_SRC_OFF	(BT656_SRC_START+0)
#define BT656_SRC_AUTO	(BT656_SRC_START+1)	//use InputMain
#define BT656_SRC_PANEL	(BT656_SRC_START+6)

#define BT656_SRC_DEC	(BT656_SRC_START+2)
#define BT656_SRC_ARGB	(BT656_SRC_START+3)
#define BT656_SRC_DTV	(BT656_SRC_START+4)
#define BT656_SRC_LVDS	(BT656_SRC_START+5)
#define BT656_SRC_NONE	(BT656_SRC_START+7)



#define BT656_INTERLACE		0
#define BT656_PROGRESSIVE	1

int BT656_Enable(BYTE fOn);
void BT656_SelectSource(BYTE fMode);
void BT656_A_DeInterlace_Set(BYTE hdelay, BYTE hstart);
void BT656_A_SelectOutput(BYTE mode,BYTE Hpol,BYTE Vpol, BYTE hv_sel);
void BT656_A_SetLLCLK_Pol(BYTE pol);
void BT656_A_SelectCLKO(BYTE mode, BYTE YOut);
void BT656_A_Set_CK54(BYTE fOn);
void BT656_D_SetRGB(BYTE fOn);

void SetBT656Output(BYTE mode);
void ChangeBT656Module(BYTE mode);

//new130221
BYTE CheckAndSetDtv656(BYTE mode);
BYTE ChangeBT656Input(BYTE mode);

#endif //..__BT656_H__
