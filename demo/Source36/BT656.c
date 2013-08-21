/**
 * @file
 * BT656.c 
 * @author Brian Kang
 * @version 1.0
 * @section LICENSE
 *	Copyright (C) 2012-2013 Intersil Corporation
 * @section DESCRIPTION
 *	BT656
*/
#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"

#include "CPU.h"
#include "Printf.h"
#include "Monitor.h"

#include "I2C.h"

#include "Scaler.h"
#include "InputCtrl.h"
#include "eeprom.h"
#include "decoder.h"
#include "aRGB.h"
#include "dtv.h"
#include "measure.h"
#include "PC_modes.h"	//for DVI_PrepareInfoString

#ifdef SUPPORT_HDMI_EP9351
#include "HDMI_EP9351.h"
#include "EP9x53RegDef.h"
#endif

#include "DebugMsg.h"
#include "BT656.h"
#include "Settings.h"

//REG006[6]
#if 0
void BT656_SwapBitOrder(BYTE fOn)
{
	BYTE bTemp;
	WriteTW88Page(0);
	bTemp = ReadTW88(REG006) & ~0x04;
	if(fOn) bTemp |= 0x04;
	WriteTW88(REG006, bTemp);
}
#endif

/**
* Enable BT656 Module
*
* REG007[3]
* @param fOn 1:On 0:Off.
* @return : 1:changed
*/
int BT656_Enable(BYTE fOn)
{
	BYTE temp;
	BYTE ret=1;

	WriteTW88Page(0);
	temp = ReadTW88(REG007);
	if(fOn) {
		if(temp & 0x08) ret = 0;
		temp |= 0x08;
	}
	else {
		if((temp & 0x08) == 0) ret = 0;
	   temp &= ~0x08;
	}
	WriteTW88(REG007,temp);
	return ret;
}

//REG048[6] Enable BT656 H/VSYNC output through aRGB H/V.
//REG066[1] Output VSYNC polarity
//REG066[0] Putput HSYNC polarity
//REG06E[7:0]			vSync Width
//REG06F[7:0]			hSync Width
//fOn : enable HSync & VSync pin Output
//h	 Horizontal Polarity 0:Normal, 1:Inverse.
//v	 Vertical   Polarity 0:Normal, 1:Inverse.
#if 0
void BT656_SyncPolarity(BYTE fOn, BYTE hPol, BYTE hSync, BYTE vPol, BYTE vSync)
{
	BYTE bTemp;
	if(fOn) {
		WriteTW88(REG048, ReadTW88(REG048) | 0x40);
		bTemp = ReadTW88(REG066) & 0xFC;
		bTemp |= (vPol << 1);
		bTemp |= hPol;
		WriteTW88(REG066, bTemp);
		WriteTW88(REG06E,vSync);
		WriteTW88(REG06F,hSync);
	}
	else {
		WriteTW88(REG048, ReadTW88(REG048) & ~0x40);
	}
}
#endif
//REG067[7]

/**
* only for DTV,LVDSRX, Panel.
* REG067[6]
*/
void BT656_D_SetRGB(BYTE fOn)
{
	BYTE bTemp;
	WriteTW88Page(0);
	bTemp = ReadTW88(REG067) & ~0x40;
	if(fOn)	bTemp |= 0x40;
	WriteTW88(REG067,bTemp);
}

//REG067[5]
//REG067[4]
//REG067[3]

/**
* @param fMode 0:dec,1:aRGB,2:DTV,3:LVDS,4:Panel
* REG067[2:0]
*/
void BT656_SelectSource(BYTE fMode)
{
	BYTE bTemp;

	WriteTW88Page(0);
	bTemp = ReadTW88(REG067) & 0xF8;
	bTemp |= fMode;
	WriteTW88(REG067,bTemp);
}

//REG068

//Horizontal Start Active Video
//Vertical Start Active Video
//REG068[]
//REG069[]
//REG06A[]
//REG06B[]
//REG06C[]
//REG06D[]

void BT656_D_Crop(WORD h_SAV, WORD v_SAV, WORD HLen, WORD VLen)
{
	BYTE bTemp;
	Printf("\nBT656_D_Crop h:%d v:%d %dx%d", h_SAV, v_SAV, HLen, VLen);

	WriteTW88Page(0);
	bTemp = ReadTW88(REG068) & 0xFC;
	WriteTW88(REG068, bTemp | (h_SAV >> 8));
	WriteTW88(REG06C, (BYTE)h_SAV);

	WriteTW88(REG06A, (BYTE)v_SAV);

	bTemp = ReadTW88(REG069) & 0xF0;
	WriteTW88(REG069, bTemp | (HLen >> 8));
	WriteTW88(REG06D, (BYTE)HLen);

	bTemp = ReadTW88(REG069) & 0x8F;
	WriteTW88(REG069, bTemp | (VLen >> 8) <<4);
	WriteTW88(REG06B, (BYTE)VLen);
}

//??for BT601 ?. passthru aRGB H & V.
//REG06E
//REG06F
#if 0
void BT656_SyncWidth(BYTE h, BYTE v)
{
	WriteTW88Page(0);
	WriteTW88(REG06F,h);
	WriteTW88(REG06E,v);
}
#endif

//
//BT656I BT656 interlaced
//BT656P BT656 DeInterlaced. Progressive
//ADC
//
/**
* description
*	select analog BT656 output.
* @param mode video data outputmode
*		0: interlaced BT656
*		1: Progressive BT656
*		2: BT656 ADC
* @param hPol
*		1: invert hSync ouput pin
* @param vPol
*		1: invert vSync output pin
* @param hv_sel hSync Output, vSync Output control
*		00b : use aRGB module hSync(HSY_SEL), vSync(VSY_SEL). See REG1CC[4] and REG1CC[3:2].
*		01b : use decoder generated hSync and vSync
*		10b : use de-interlaced hSync and vSync.
* REG105
*/

void BT656_A_SelectOutput(BYTE mode,BYTE hPol,BYTE vPol, BYTE hv_sel)
{
	BYTE bTemp;
	WriteTW88Page(1);
	bTemp = mode << 4;
	if(hPol==0) bTemp |= 0x08;
	if(vPol==0) bTemp |= 0x04;
	bTemp |= hv_sel;
	WriteTW88(REG105,bTemp);
}

#if 0
/**
* 
* REG136[3:2]
* REG136[1:0]
*/
void BT656_A_SetDeInterlaceFieldOffset(BYTE Odd, BYTE Even)
{
	BYTE bTemp;
	WriteTW88Page(1);
	bTemp = ReadTW88(REG136) & 0xF0;
	bTemp |= (Odd < 2);
	bTemp |= (Even);
	WriteTW88(REG136,bTemp);
}
#endif

/**
* REG137[7:0]
* REG138[7:0]
*/
void BT656_A_DeInterlace_Set(BYTE hdelay, BYTE hstart)
{
	WriteTW88Page(1);
	WriteTW88(REG137,hdelay);
	WriteTW88(REG138,hstart);
}

//REG1E8[]. move from TW8835 REG105.


//----------------------------
// BT656 Analog Control.
// for ADC and aRGB only
//----------------------------
void BT656_A_SetLLCLK_Pol(BYTE pol)
{
	BYTE bTemp;
	WriteTW88Page(1);
	bTemp = ReadTW88(REG1E9) & 0xFD;
	bTemp |= pol << 1;
	WriteTW88(REG1E9,bTemp);
}
void BT656_A_SelectCLKO(BYTE mode, BYTE YOut)
{
	BYTE bTemp;
	WriteTW88Page(1);
	bTemp = ReadTW88(REG1E9) & 0x1F;
	bTemp = mode << 6;
	bTemp |= YOut << 5;
	WriteTW88(REG1E9,bTemp);
}
void BT656_A_Set_CK54(BYTE fOn)
{
	BYTE bTemp;
	WriteTW88Page(1);
	bTemp = ReadTW88(REG1E9) & 0xFE;
	bTemp |= fOn;
	WriteTW88(REG1E9,bTemp);
}




// REG1EA[1] 0: RGBCLK div2, 1:RGBCLK
// REG1EA[0] 0: ADCCLK div2, 1:ADCCLK
void BT656_AdcRgbNoClkDivider2(BYTE fAdcOn, BYTE fRgbOn)
{
	BYTE bTemp;
	WriteTW88Page(1);
	bTemp = ReadTW88(REG1EA);
	if(fRgbOn) bTemp |= 0x02;
	if(fAdcOn) bTemp |= 0x01;
	WriteTW88(REG1EA, bTemp);
}

//=============================================================================
//
//=============================================================================
/*
ChangeBT656
*/
//parameter: BT656_SRC_DEC,....



//parameter InputMain value or BT565...
/**
* desc: Select BT656 MUX
* @param 
*		BT656_SRC_DEC
*		BT656_SRC_ARGB
*		BT656_SRC_DTV
*		BT656_SRC_LVDS
*		BT656_SRC_PANEL
*		BT656_SRC_OFF
*
*		BT656_SRC_AUTO can not use.
*
* assume, TW8836 already has a correct InMux path.
*
* set BT656 mux. 
*/
void SetBT656Output(BYTE bt656_src_mode)
{
	BYTE HPol,VPol;
	BYTE p;
	WORD h_SAV,v_SAV,HLen,VLen;
	WORD hDelay,vDelay,hActive,vActive;

	BYTE bTemp;
	WORD wTemp;
	BYTE failed;
	//BYTE OLD_InputMain;
	//BYTE bt656_src_mode;
	BYTE mode;


	BT656_Enable(OFF);

	//recover some test or temp value.
//	PclkoSetDiv(2 /* div 3 = 108 / 3 = 36MHz */);		//panel use 27MHz. I just want to recover before I start.
	WriteTW88(REG068, ReadTW88(REG068) & ~0x80);		//clear "use Input Field"
	WriteTW88(REG303, (ReadTW88(REG303) + 1) & 0xFE);	//EVEN FOSD DE.

	if(bt656_src_mode == BT656_SRC_OFF)
		return;

	//check video input. Do not change video mux.
	failed=0;
	switch(bt656_src_mode) {
	case BT656_SRC_DEC:	//DEC
		WriteTW88Page(0);
		mode = ReadTW88(REG101);		//read Chip Status
		if (mode & 0x80) {
			//video loss
			failed = 1;
			break;
		}
		mode >>= 4;
		BT656_A_SelectOutput(BT656_INTERLACE, 0,0, 1);	//I, ?,?, Dec
		BT656_A_SelectCLKO(0,0);

		//BKTODO: Do not scale the video source.

		h_SAV = DecoderGetHDelay();
		v_SAV = DecoderGetVDelay();
		HLen = DecoderGetHActive();	HLen = 720;
		VLen = DecoderGetVActive();
		BT656_D_Crop(h_SAV,v_SAV,HLen,VLen);
		BT656_SelectSource(0x00);

		if(mode==0 || mode==3 || mode==4 || mode==6) {
			//ntsc
			BT656_InitExtEncoder(BT656_8BIT_525I_YCBCR_TO_CVBS);
		}
		else if(mode==1 || mode==2 || mode==5) {
			//pal
			BT656_InitExtEncoder(BT656_8BIT_625I_YCBCR_TO_CVBS);
		}
		else {
			//BUG...
		}

		break;
	case BT656_SRC_ARGB: //aRGB
		mode = aRGB_GetInputStatus();
		if((mode & 0x38) == 0) {
			// no signal
			failed = 1;
			break;
		}
		if(mode & 0x08) { // component
			BYTE ck2s;

			HPol = mode & 0x80 ? 1:0;
			VPol = mode & 0x40 ? 1:0;
			switch(mode & 0x07) {
#if 0
			case 0:	p = 0; ck2s=0;		break;	//480i
			case 1:	p = 0; ck2s=0;		break;	//576i
			case 2:	p = 1; ck2s=2;		break;	//480p
			case 3:	p = 1; ck2s=2;		break;	//576p
#else
			//ADC, ADC,
			case 0:	p = 2; ck2s=2;		break;	//480i
			case 1:	p = 2; ck2s=2;		break;	//576i
			case 2:	p = 2; ck2s=2;		break;	//480p
			case 3:	p = 2; ck2s=2;		break;	//576p
#endif
			case 4:	failed=1;			break; 	//1080i out of BT565
			case 5:	failed=1;			break; 	//720p  out of BT656
			case 6: failed=1;			break; 	//1080p out of BT656
			default:
					failed=1;			break;
			}
			if(failed)
				break;

			BT656_A_SelectOutput(p, HPol, VPol, 0);
			BT656_A_SelectCLKO(ck2s,0);

			//make LLPLL double
			//wTemp = PCMDATA[ mode & 0x07 ].htotal;
			//wTemp = YUVDividerPLL[mode & 0x07];
			switch(mode & 0x07) {
			case 0:	/*BT656_InitExtEncoder(BT656_8BIT_525I_YCBCR_TO_CVBS);*/ wTemp = 858;	break;	//480i
			case 1:	/*BT656_InitExtEncoder(BT656_8BIT_625I_YCBCR_TO_CVBS);*/ wTemp = 864;	break;	//576i
			case 2:	/*BT656_InitExtEncoder(BT656_8BIT_525P_YCBCR_TO_CVBS);*/ wTemp = 858;	break;	//480p
			case 3:	/*BT656_InitExtEncoder(BT656_8BIT_625P_YCBCR_TO_CVBS);*/ wTemp = 864;	break;	//576p
			}
			wTemp *= 2;
			wTemp += 1;
			Write2TW88(REG1C3,REG1C4, wTemp);
			//use div2 ADC clock
			WriteTW88(REG1EA, ReadTW88(REG1EA) & ~0x02);

		}
		else {	//PC input
			//read LLPLL Divider & assume input video format.
			//need measure...
			//wTemp = VAdcLLPLLGetDivider();
			//if(wTemp == )

			//use Input_aRGBMode.
			extern code struct _PCMODEDATA PCMDATA[];
			struct _PCMODEDATA *pTbl;

			if(Input_aRGBMode > 8) {
				failed=1;
				break;	
			}
			pTbl = &PCMDATA[Input_aRGBMode];

			HPol = pTbl->syncpol & HPOL_P ? 1: 0;
			VPol = pTbl->syncpol & VPOL_P ? 1: 0;

			BT656_A_SelectOutput(1, HPol, VPol, 0);
			BT656_A_SelectCLKO(0,0);

			BT656_InitExtEncoder(BT656_8BIT_525P_YCBCR_TO_CVBS);  //480p
		}
		h_SAV=InputGetHStart();
		v_SAV=InputGetVStart();
		HLen=InputGetHLen();
		VLen=InputGetVLen();
		BT656_D_Crop(h_SAV,v_SAV,HLen,VLen);
		BT656_SelectSource(0x01);
		break;
	case BT656_SRC_DTV: //DTV
#if 1 //BK130109
		HLen=InputGetHLen();
		VLen=InputGetVLen();
		BT656_D_Crop(32,32,HLen,VLen);
		BT656_SelectSource(0x02);
#else
		HPol = 0;
		VPol = 0;
		h_SAV=InputGetHStart();
		v_SAV=InputGetVStart();
		HLen=InputGetHLen();
		VLen=InputGetVLen();

		if(HLen > 720) {
			failed=1;
			break;				
		}
		BT656_A_SelectOutput(1, HPol, VPol, 0);
		BT656_A_SelectCLKO(0,0);
		BT656_D_Crop(h_SAV,v_SAV,HLen,VLen);


		BT656_InitExtEncoder(BT656_8BIT_525P_YCBCR_TO_CVBS);  //480p
#endif
		break;

	case BT656_SRC_LVDS: //LVDS
#if 1	//480p
		//{3,		720,480,60,		HPOL_N | VPOL_N,	858,16,62,60,		525,9,6,30},
		//==>       1440,482,         +H +V            ????,?,8,204,     ???,??,1,29       
		hDelay = 56;
		vDelay = 46;
		hActive = 720;
		vActive = 480;

		//720P
		//{4,		1280,720,60,	HPOL_P | VPOL_P,	1650,110,40,220,	750,5,5,20},

		//576P	{18,	720,576,50,		HPOL_N | VPOL_N,	864,12,64,68,		625,5,5,39	},



		BT656_D_Crop(hDelay,vDelay,hActive,vActive);
		BT656_SelectSource(0x03);
#endif

#if 0 //BK130109
		HLen=InputGetHLen();
		VLen=InputGetVLen();

		BT656_D_Crop(32,32,HLen,VLen);
		BT656_SelectSource(0x03);
#endif
#if 0
		//I don't know how to ...yet.
		//failed = 1;

		//let's try some fixed value
		HPol = 0;
		VPol = 0;
		h_SAV= 60; //InputGetHStart();
		v_SAV= 30; //InputGetVStart();
		HLen=720; //InputGetHLen();
		VLen=480; //InputGetVLen();

		BT656_A_SelectOutput(1, HPol, VPol, 0);
		BT656_A_SelectCLKO(0,0);
		BT656_D_Crop(h_SAV,v_SAV,HLen,VLen);


		BT656_InitExtEncoder(BT656_8BIT_525P_YCBCR_TO_CVBS);  //480p
#endif
		break;

	case BT656_SRC_PANEL: //Panel
#if 1 //BK130109
		//h_SAV = ScalerReadHDEReg();
		h_SAV = ScalerReadLineBufferDelay() + 26;
		bTemp = ReadTW88(REG20D) & 0x04;
		wTemp = ReadTW88(REG221) & 0x03; wTemp <<= 8;
		wTemp |= ReadTW88(REG213);
		if(h_SAV < wTemp)
			h_SAV = 0;
		else
			h_SAV -= wTemp; //HSyncPos.
		if(bTemp ==0) {
			bTemp = ReadTW88(REG214); //HSyncWidth
			if(h_SAV < bTemp)
				h_SAV = 0;
			else 
				h_SAV -= bTemp;
		}
		h_SAV &= 0xFFFE;	//prefer EVEN.

		v_SAV = ScalerReadVDEReg() + 7;
		Printf("\nSAV h:%d, v:%d",h_SAV,v_SAV);
		BT656_D_Crop(h_SAV,v_SAV,800,480);
		BT656_SelectSource(0x04);
		WriteTW88(REG068, ReadTW88(REG068) | 0x80);		//use Input Field
//		PclkoSetDiv(3 /* div 4 = 108 / 4 = 27MHz */);

#else
		failed = 1;
#endif
		break;
	}

	if(failed) {
		//try PANEL.
		Printf("\=======BT656 PANEL======>Force 720x480p");
		ScalerTest(2); //force 720x480p
		BT656_InitExtEncoder(BT656_8BIT_525P_YCBCR_TO_CVBS);  //480p

		//current EVB can support only SD. not 800x480.
		//at first, call "Scaler x" for 720x480p output.


		bTemp = ReadTW88(REG20D);
		HPol = bTemp & 0x04 ? 1: 0;
		VPol = bTemp & 0x08 ? 1: 0;
		BT656_A_SelectOutput(1, HPol, VPol, 0);
		BT656_A_SelectCLKO(1,0);
				
		h_SAV=ScalerReadHDEReg();
		v_SAV=ScalerReadLineBufferDelay()+33;
		HLen=720;
		VLen=480;
		BT656_D_Crop(h_SAV,v_SAV,HLen,VLen);

	}
	BT656_Enable(ON);
}

/*
* Change InputMain & BT656 Mux.
* usage: iBT656 {OFF|AUTO|PANEL|DEC|ARGB|DTV|LVDS}
* It will call SetBT656Output() at the end of this procedure.
*
* @param mode
*	BT656_SRC_OFF	
*	BT656_SRC_AUTO	
*	BT656_SRC_PANEL	
*	BT656_SRC_DEC	
*	BT656_SRC_ARGB	
*	BT656_SRC_DTV	
*	BT656_SRC_LVDS	
*	BT656_SRC_NONE	
*/
void ChangeBT656Module(BYTE mode)
{
	BYTE fError;
	BYTE bt656_src_mode;

	BT656_Enable(OFF);
	if(mode == BT656_SRC_OFF)
		return;

	//------------------------------------
	//
	//OLD_InputMain = 0xFF;
	//if(mode != BT656_SRC_AUTO && mode != BT656_SRC_PANEL) {	
	//	if(InputMain != mode) {
	//		OLD_InputMain = InputMain;
	//		
	//		InputMain = 0xff;			// start with new input						
	//		ChangeInput( mode );	
	//	}
	//}
	
	if(mode==BT656_SRC_DEC || mode==BT656_SRC_ARGB || mode==BT656_SRC_DTV || mode==BT656_SRC_LVDS) {
		//check input combination
		fError = 0;
		switch(InputMain) {
		case INPUT_CVBS:
		case INPUT_SVIDEO:
			if(mode != BT656_SRC_DEC) 
				fError = 1;
			break;
		case INPUT_COMP:
		case INPUT_PC:
			if(mode != BT656_SRC_ARGB)
				fError = 1;
			break;
		case INPUT_DVI:
		case INPUT_HDMIPC:
		case INPUT_HDMITV:
			if(mode != BT656_SRC_DTV)
				fError = 1;
			break;
		case INPUT_LVDS:
			if(mode != BT656_SRC_LVDS)
				fError = 1;
			break;
		case INPUT_BT656:
			//it is a loopback. input and output have to different.
			if(mode == BT656_SRC_DTV)
				fError = 1;			
			break;
		}
		if(fError) {
			Puts("\n FAIL");	
			PrintfInput(InputMain,0);
			Puts(" ");	
			PrintfBT656Input(mode, 0);
			return;
		}
		bt656_src_mode = mode;
	}
	else if(mode==BT656_SRC_AUTO) {
		//select BT656_SRC_MODE
		switch(InputMain) {
		case INPUT_CVBS:
		case INPUT_SVIDEO:
		case INPUT_BT656:	//loopback
			bt656_src_mode = BT656_SRC_DEC;
		case INPUT_COMP:
		case INPUT_PC:
			bt656_src_mode = BT656_SRC_ARGB;
			break;
		case INPUT_DVI:
		case INPUT_HDMIPC:
		case INPUT_HDMITV:
			bt656_src_mode = BT656_SRC_DTV;
			break;
		case INPUT_LVDS:
			bt656_src_mode = BT656_SRC_LVDS;
			break;
		default:
			bt656_src_mode = mode;
			break;	
		}
	}
	else {
		//panel
		bt656_src_mode = mode;
	}
	SetBT656Output(bt656_src_mode);


	//if(OLD_InputMain != 0xFF) {
	//	//recover.
	//	InputMain = 0xff;			// start with new input						
	//	ChangeInput( OLD_InputMain );	
	//}
}

/**
*
* Check DTV 656 input and set scaler.
* If InputMain has a conflict, stop and print only a debug message.
*
* extern
*	InputMain
*	InputBT656
*
*	select i656(orp656) DTV input.
* 	read BT656 module.
*	read measured value.
*
* If loopback is avaiable,
*	adjust input crop, scaler.
*/
BYTE CheckAndSetDtv656(BYTE mode)
{
	BYTE ret = mode;
	BYTE hPol,vPol;		
	WORD /*hTotal,*/ vTotal;
	WORD hActive,vActive;
	WORD hSync,vSync;
		
	WORD hCropStart,vCropStart;

	//select DTV routing and format.
	DtvSetRouteFormat(DTV_ROUTE_PbYPr, DTV_FORMAT_INTERLACED_ITU656);	//DTV_FORMAT_PROGRESSIVE_ITU656
	DtbSetReverseBusOrder(ON);

	//select InMux
	InputSetSource(INPUT_PATH_DTV, INPUT_FORMAT_RGB);
	WriteTW88(REG040,ReadTW88(REG040) | 0x04);	//enable DTVCLK2

	if(MeasStartMeasure())
		return 1;

	hActive = MeasGetHActive( &hCropStart );
	vActive = MeasGetVActive( &vCropStart );

	if ( hSync > (hActive/2) )	hPol = 1;	//active low. something wrong.
	else						hPol = 0;	//active high
	if ( vSync > (vTotal/2) )	vPol = 1;	//active low. something wrong.
	else						vPol = 0;	//active high


//#ifdef DEBUG_DTV
	Printf("\nMeas %d hPol:%bx hCropStart:%d+4",hActive, hPol, hCropStart);
	Printf("\n     %d vPol:%bx vCropStart:%d+1",vActive, vPol, vCropStart);
//#endif

	return ret;
}


/**
* Change BT656 input.
* Do not change InputMain.
* If InputMain has a conflict, stop and print only a debug message.	
*
* extern
*	InputMain
*	InputBT656
*
* @param mode
*	BT656_SRC_DEC	
*	BT656_SRC_ARGB	
*	BT656_SRC_DTV	
*	BT656_SRC_LVDS	
*	BT656_SRC_PANEL	
*	BT656_SRC_OFF
* @return
*	0: success
*	1:invalid combination
*	2:out of range. only for 480i,576i,480p and 576p.	
*/
BYTE ChangeBT656Input(BYTE mode)
{
	BYTE bt656_src_mode;
	BYTE error;
	WORD hDelay,vDelay,hActive,vActive;

	if(mode == BT656_SRC_OFF) {
		BT656_Enable(OFF);
		return ERR_SUCCESS;
	}

	//check input combination
	error = 0;
	switch(InputMain) {
	case INPUT_CVBS:
	case INPUT_SVIDEO:
		if(mode != BT656_SRC_DEC)
			error = 1;
		break;
	case INPUT_COMP:
	case INPUT_PC:
		if(mode != BT656_SRC_ARGB)
			error = 1;
		break;
	case INPUT_DVI:
	case INPUT_HDMIPC:
	case INPUT_HDMITV:
		if(mode != BT656_SRC_DTV)
			error = 1;
		break;
	case INPUT_LVDS:
		if(mode != BT656_SRC_LVDS)
			error = 1;
		break;
	case INPUT_BT656: //for Loopback
	default:
		error=1;
		break;
	}
	if(error) {
		Printf("\nInvalid InputMain:");
		PrintfInput(InputMain, 0);
		Printf(" and BT656_SRC_%bx",mode);
		return 1;
	}
	bt656_src_mode = mode;
	BT656_Enable(ON);
	BT656_SelectSource(bt656_src_mode);
	if(bt656_src_mode == BT656_SRC_DEC) {
		//at first, select DEC(I).
		//If you need DEC(DI), you have to adjust it by manually.
		BT656_A_SelectOutput(0, 0,0, 1);
		BT656_A_SelectCLKO(0, 1);
		BT656_A_SetLLCLK_Pol(0);
		if(InputMain == INPUT_COMP || InputMain == INPUT_PC) {
			BT656_AdcRgbNoClkDivider2(0,0);	//use div2
		}
		else {
			BT656_AdcRgbNoClkDivider2(1,1);	//no divider
		}
	}	
	else if(bt656_src_mode == BT656_SRC_ARGB) {
		//select ADC.
		BT656_A_SelectOutput(2, 0,0, 0);
		BT656_A_SelectCLKO(2, 1);
		BT656_A_SetLLCLK_Pol(0);
		BT656_AdcRgbNoClkDivider2(1,1);	//no divider
	}
	else {
		//all digital. (DTV,LVDSRx,Panel)
		//read input & adjust delay,active value.
		//
		//for example: LVDS 480P
		hDelay = 56;
		vDelay = 46;
		hActive = 720;
		vActive = 480;
		BT656_D_Crop(hDelay,vDelay,hActive,vActive);
	}	
	
	
	return ERR_SUCCESS;		
}
