/**
 * @file
 * DTV.c 
 * @author Brian Kang
 * @version 1.1
 * @section LICENSE
 *	Copyright (C) 2011~2012 Intersil Corporation
 * @section DESCRIPTION
 *	DTV for DVI,HDMI,BT656, and LVDS.
 *
 * history
 *  120803	add MeasStartMeasure before FW use a measured value.
 *			update Freerun Htotal,VTotal at VBlank.
 *			add checkroutine for HDMI detect flag.
*/
#include "Config.h"
#include "reg.h"
#include "typedefs.h"
#include "TW8836.h"
#include "Global.h"

#include "CPU.h"
#include "Printf.h"
#include "Monitor.h"

#include "I2C.h"

#include "Scaler.h"
#include "InputCtrl.h"
#include "eeprom.h"
#include "Decoder.h"
#include "aRGB.h"
#include "dtv.h"
#include "measure.h"
#include "PC_modes.h"	//for DVI_PrepareInfoString

#ifdef SUPPORT_HDMI_EP9351
#include "HDMI_EP9351.h"
#include "EP9351_RegDef.h"
#endif
#ifdef SUPPORT_HDMI_EP9553
#include "HDMI_EP9553.h"
#include "EP9553_RegDef.h"
#endif
#ifdef SUPPORT_HDMI_EP907M
#include "HDMI_EP907M.h"
#include "EP907M_RegDef.h"
#endif

#include "Settings.h"

#include "DebugMsg.h"
#include "util.h"
#include "FOsd.h"
#include "SOsdMenu.h"	//remove logo


#if !defined(SUPPORT_DVI) && !defined(SUPPORT_HDMI)
//----------------------------
/**
* Trick for Bank Code Segment
*/
//----------------------------
CODE BYTE DUMMY_DTV_CODE;
void Dummy_DTV_func(void)
{
	BYTE temp;
	temp = DUMMY_DTV_CODE;
}
#endif

						

#if defined(SUPPORT_DVI) || defined(SUPPORT_HDMI)
/**
* set DTV Polarity
*
*	register
*	R050[2]	HPol	1:AvtiveLow
*	R050[1]	VPol	1:ActiveLow
*
* @param	HPol	1:ActiveLow
* @param	VPol	1:ActiveLow
*/
void DtvSetPolarity(BYTE HPol, BYTE VPol)
{
	BYTE value;

	WriteTW88Page(PAGE0_DTV);
	value = ReadTW88(REG050) & 0xF9;
	if(HPol)	value |= 0x04;	//H Active Low		
	if(VPol)	value |= 0x02;	//V Active Low		
	WriteTW88(REG050, value);	
	
	//dPrintf("\nDTV Pol H:%bd V:%bd",HPol,VPol);	
}
#endif


#if defined(SUPPORT_DVI)  || defined(SUPPORT_HDMI)
/**
* set DTV clock delay
*
*	register
*	R051[2:0]
*/
void DtvSetClockDelay(BYTE delay)
{
	WriteTW88Page(PAGE0_DTV);
	WriteTW88(REG051, (ReadTW88(REG051) & 0xF8) | delay);
}
#endif

#ifdef MODEL_TW8836
/**
* set DTV Color bus order
*
*	register
*	R051[3]	 1: Reverse data line order on RGB565 bus.
*/
void DtbSetReverseBusOrder(BYTE fReverse)
{
	BYTE value;

	WriteTW88Page(PAGE0_DTV);

	value = ReadTW88(REG051) & 0xF7;
	if(fReverse)	value |=  0x08;		

	WriteTW88(REG051, value);		
}
#endif

#if defined(SUPPORT_DVI) || defined(SUPPORT_BT656_LOOP) || defined(SUPPORT_HDMI)
/**
* set DTV DataRouting and InputFormat
*
*	register
*	R052[2:0]	DataRouting:100 Y(R):Pb(G):Pr(B)
*	R053[3:0]	InputFormat:1000=RGB565
*
* @param 	route:	Data bus routing selection for DTV.
* @param	format
*	0: Interlaced ITU656		i656  	
*	1: Progressive ITU656		p656
*	2: 8bit 601					422 8bit
*	3: 16bit 601				422 16bit
*	4: 24bit 601				444 24bit
*	5: 16/18/24 bit RGB			RGB 24bit
*	6: ITU1120					BT1120					
*	7: SMP 720P					SMPTE296M
*	8: RGB565					RGB565
*
*
* Data bus routing
* ================
* 
* For 24 bit YPbPr
* ----------------
* 	DTVD	DTVD	DTVD
*     [23:16]	[15:8]	[7:0]	
* 0:	Pr		Y		Pb
* 1:	Pr		Pb		Y
* 2:	Pb		Y		Pr
* 3:	Pb		Pr		Y
* 4:	Y		Pb		Pr
* 5:	Y		Pr		Pb
*
* For 24bit RGB
* ----------------
*	DTVD	DTVD	DTVD
*    [23:16]	[15:8]	[7:0]	
* 0:	R		G		B
* 1:	R		B		G
* 2:	B		G		R
* 3:	B		R		G
* 4:	G		B		R
* 5:	G		R		B
* 
* For 16bit RGB565
* ----------------
* 	DTVD	DTVD	DTVD
*     [23:16]	[15:8]	[7:0]	
* 0:	LSB				MSB
* 1:	LSB		MSB		
* 2:	MSB				LSB
* 3:	MSB		LSB		
* 4:			MSB		LSB
* 5:			LSB		MSB
* 
* For 16 bit YPbPr
* ----------------
* 	DTVD	DTVD	DTVD
*     [23:16]	[15:8]	[7:0]	
* 0:			MSB		LSB
* 1:			LSB		MSB
* 2:	LSB		MSB		
* 3:	LSB				MSB
* 4:	MSB		LSB		
* 5:	MSB				PbLSB
* 
* For 8 bit YPbPr
* ----------------
* 	DTVD	DTVD	DTVD
*     [23:16]	[15:8]	[7:0]	
* 0:	data	
* 1:	data
* 2:					data
* 3:			data
* 4:					data
* 5:			data
* 
* 8 bit data order
* ----------------
* DTV_DODR is set, DE_POL is ignored.	
* (??HW captures signal at DE edge.)
* Pb or Pr always comes frist depend on DTV_CR601.
* 
* DTV_DODR	DTV_CR601	Data Order
* R051[5]		R052[3]
*    1		  0			Pb-Y-Pr-Y			
*    1          1         Pr-Y-Pb-Y
* 
* DTV_DODR is reset,  DE_POL makes affect on the Data Order.
* (??HW captures signal at HSync)
* If DE_POL is set, Y comes first.
* 
* DTV_DODR	DE_POL		DTV_CR601	Data Order
* R051[5]		R050[3]		R052[3]
*    0		  0			0			Pb-Y-Pr-Y			
*    0          0			1         	Pr-Y-Pb-Y
*    0		  1			0			Y-Pb-Y-Pr			
*    0          1			1         	Y-Pr-Y-Pb
* 
* The BT656 uses Cb-Y-Cr-Y order.	 
* 
*        +-+-+-+-+----------
*  Y	 |0|1|2|3|
*        +-+-+-+-+----------
*  Cr	 | 0 | 1 |
*        +---+---+-----------
*  Cb    | 0 | 1 |	
*        +---+---+-----------
* 
* data order  Cb0 Y0 Cr0 Y1, Cb1 Y2
* So, choose R051[5]=1, R052[3]=0.
* 
* 
* Data Bus Bit Order
* ===================
* REG051[3] 1:Reverse 0:Normal.
* 
* example. For 16bit RGB565
* ----------------
* 	DTVD	DTVD	DTVD
*    [23:16]	[15:8]	[7:0]	
* 1:	LSB		MSB		
* 
* 
* If input source order is reversed, select Route Order "1" and set REG051[3].
* 
* INPUT	   			Connect		On Reverse.
* ===========			======		========
* Dinput03(B3)		DTVD23		08	[LSB]
* Dinput04(B4)		DTVD22		09
* Dinput05(B5)		DTVD21		10
* Dinput06(B6)		DTVD20		11
* Dinput07(B7)		DTVD19		12
* .
* Dinput10(G2)		DTVD18		13
* Dinput11(G3)		DTVD17		14
* Dinput12(G4)		DTVD16		15
* Dinput13(G5)		DTVD15		16
* Dinput14(G6)		DTVD14		17
* Dinput15(G7)		DTVD13		18
* .
* Dinput19(R3)		DTVD12		19
* Dinput20(R4)		DTVD11		20
* Dinput21(R5)		DTVD10		21
* Dinput22(R6)		DTVD09		22
* Dinput23(R7)		DTVD08		23	[MSB]
* 
* 
*
* If input order is normal, select Route Order "3".
* For 16bit RGB565
* ----------------
* 	DTVD	DTVD	DTVD
*     [23:16]	[15:8]	[7:0]	
* 3:	MSB		LSB		
* 
* INPUT	TW8836		
* 		======
* R7		DTV23  [MSB]		
* R6		DTV22		
* R5		DTV21		
* R4		DTV20		
* R3		DTV19		
* 		
* G7		DTV18		
* G6		DTV17		
* G5		DTV16		
* G4		DTV15		
* G3		DTV14		
* G2		DTV13		
* 		
* B7		DTV12		
* B6		DTV11		
* B5		DTV10		
* B4		DTV09		
* B3		DTV08  [LSB]		
*
*/
void DtvSetRouteFormat(BYTE route, BYTE format)
{
	WriteTW88Page(PAGE0_DTV);
	WriteTW88(REG052, (ReadTW88(REG052) & 0xF8) | route);
	WriteTW88(REG053, (ReadTW88(REG053) & 0xF0) | format);
}
#endif

#if defined(SUPPORT_DVI) || defined(SUPPORT_HDMI)
/**
* set DTV Field Detection Register
* param
*	register
*	R054[7:4]	End Location
*	R054[3:0]	Start Location
* example:
*	DtvSetFieldDetectionRegion(0x11)
*/
void DtvSetFieldDetectionRegion(BYTE fOn, BYTE r054)
{
	WriteTW88Page(PAGE0_DTV );
	if(fOn) {
		WriteTW88(REG050, ReadTW88(REG050) | 0x80 );	// set Det field by WIN
		WriteTW88(REG054, r054 );						// set window
	}
	else {
		WriteTW88(REG050, ReadTW88(REG050) & ~0x80 );	// use VSync/HSync Pulse
	}
}
#endif

#if defined(SUPPORT_DVI) || defined(SUPPORT_HDMI)
/**
* set DTV VSyncDelay value
*/
void DtvSetVSyncDelay(BYTE value)
{
	WriteTW88Page(PAGE0_DTV );
	WriteTW88(REG056, value);
}
#endif


//=============================================================================
// DVI
//=============================================================================

#if defined(SUPPORT_DVI)
//-----------------------------------------------------------------------------
//		void 	DVISetInputCrop( void )
//-----------------------------------------------------------------------------
/**
* set InputCrop for DVI
*
* extern
*	MeasHPulse ->Removed
*	MeasVPulse ->Removed
*	MeasVStart
*/
static void DVISetInputCrop( void )
{
	BYTE	offset, VPulse, HPulse;
	WORD	hstart, vstart, vtotal, hActive;
	BYTE HPol, VPol;
	WORD Meas_HPulse,Meas_VPulse;

	Meas_HPulse = MeasGetHSyncRiseToFallWidth();
	Meas_VPulse = MeasGetVSyncRiseToFallWidth();
	vtotal = MeasGetVPeriod();
	hActive = MeasGetHActive( &hstart );

#ifdef DEBUG_DTV
	dPuts("\nMeas");
	dPrintf("\n\tH           Pulse:%4d BPorch:%3d Active:%4d hAvtive:%d",Meas_HPulse,hstart,MeasHLen,hActive);
	dPrintf("\n\tV Total:%4d Pulse:%4d BPorch:%3d Active:%4d",vtotal,Meas_VPulse,MeasVStart,MeasVLen);
#endif

	offset = 5;  //meas delay value:4
	//hstart = MeasHStart + offset;
	hstart += offset;
	if ( Meas_HPulse > (hActive/2) ) {
		if(hActive > Meas_HPulse)
			HPulse = hActive - Meas_HPulse;
		else
			HPulse = Meas_HPulse - hActive;
		HPol = 0;	
	}
	else  {
		HPulse = Meas_HPulse;
		HPol = 1;
		hstart -= HPulse;	// correct position
	}

	if ( Meas_VPulse > (vtotal/2) ) {
		VPulse = vtotal - Meas_VPulse;
		VPol = 0;
	}
	else  {
		VPulse = Meas_VPulse;
		VPol = 1;
	}
	vstart = MeasVStart + VPulse;

	DtvSetPolarity(HPol,VPol);

#ifdef DEBUG_DTV
	dPuts("\nmodified");
	dPrintf("\n\tH           Pulse:%2bd BPorch:%3d Active:%4d Pol:%bd hActive:%4d ",HPulse,hstart,MeasHLen,HPol, hActive);
	dPrintf("\n\tV Total:%4d Pulse:%2bd BPorch:%3d Active:%4d Pol:%bd",vtotal,VPulse,vstart,MeasVLen,VPol);
#endif
	//BKFYI. The calculated method have to use "InputSetCrop(hstart, vstart, MeasHLen, MeasVLen);"
	//		 But, we using a big VLen value to extend the vblank area.
	InputSetCrop(hstart, 1, MeasHLen, 0x7fe);
}

/**
* set Output for DVI
*/
static void DVISetOutput( void )
{
	BYTE	HDE;
	WORD temp16;

	ScalerSetHScale(MeasHLen);
	ScalerSetVScale(MeasVLen);

	//=============HDE=====================
	HDE = ScalerCalcHDE();
#ifdef DEBUG_DTV
	dPrintf("\n\tH-DE start = %bd", HDE);
#endif
	ScalerWriteHDEReg(HDE);


	//=============VDE=====================
	// 	MeasVStart ??R536:R537
	//	MeasVPulse ??R52A,R52B
	temp16 = ScalerCalcVDE();
#ifdef DEBUG_DTV
	dPrintf("\n\tV-DE start = %d", temp16);
#endif
	ScalerWriteVDEReg((BYTE)temp16);

	//=================  Free Run settings ===================================
	temp16=ScalerCalcFreerunHtotal();
#ifdef DEBUG_DTV
	dPrintf("\n\tFree Run Htotal: 0x%x", temp16);
#endif
	ScalerWriteFreerunHtotal(temp16);

	//================= V Free Run settings ===================================
	temp16=ScalerCalcFreerunVtotal();
#ifdef DEBUG_DTV
	dPrintf("\n\tFree Run Vtotal: 0x%x", temp16);
#endif
	ScalerWriteFreerunVtotal(temp16);

	//================= FreerunAutoManual, MuteAutoManual =====================
	ScalerSetFreerunAutoManual(ON,OFF);
	ScalerSetMuteAutoManual(ON,OFF);
}

/**
* Check and Set DVI
*
* extern
*	MeasVStart,MeasVLen,MeasHLen	
* @return 
*	-0:ERR_SUCCESS
*	-1:ERR_FAIL
*/
BYTE CheckAndSetDVI( void )
{
	DECLARE_LOCAL_page
	WORD	Meas_HStart;
	WORD	MeasVLenDebug, MeasHLenDebug;
	WORD    MeasVStartDebug, MeasHStartDebug;

	ReadTW88Page(page);
	//DtvSetPolarity(0,0);

	do {														
		if(MeasStartMeasure()) {
			WriteTW88Page(page );
			return ERR_FAIL;
		}
		MeasVLen = MeasGetVActive( &MeasVStart );				//v_active_start v_active_perios
		MeasHLen = MeasGetHActive( &Meas_HStart );				//h_active_start h_active_perios
#ifdef DEBUG_DTV
		dPrintf("\nDVI Measure Value: %dx%d HS:%d VS:%d",MeasHLen,MeasVLen, Meas_HStart, MeasVStart);
		dPrintf("==>Htotal:%d",  MeasGetVsyncRisePos());
#endif

		DVISetInputCrop();
		DVISetOutput();

		MeasVLenDebug = MeasGetVActive( &MeasVStartDebug );		//v_active_start v_active_perios
		MeasHLenDebug = MeasGetHActive( &MeasHStartDebug );		//h_active_start h_active_perios

	} while (( MeasVLenDebug != MeasVLen ) || ( MeasHLenDebug != MeasHLen )) ;

	
	AdjustPixelClk(0, 0);	//NOTE:it uses DVI_Divider.

	//prepare info
	DVI_PrepareInfoString(MeasHLen,MeasVLen,MeasGetVFreq() /*0*/ /*freq*/);

	WriteTW88Page(page );

	//for debug. check the measure value again
	CheckMeasure();

	return ERR_SUCCESS;
}
#endif	//..SUPPORT_DVI

//=============================================================================
// Change to DVI
//=============================================================================

#ifdef SUPPORT_DVI
//-----------------------------------------------------------------------------
/**
* Change to DVI
*
* linked with SIL151
* @return
*	- 0: success
*	- 1: No Update happen
*	- 2: No Signal or unknown video sidnal.
*/
BYTE	ChangeDVI( void )
{
	BYTE ret;

	if ( InputMain == INPUT_DVI ) {
		dPrintf("\nSkip ChangeDVI");
		return(1);
	}

	InputMain = INPUT_DVI;

	if(GetInputMainEE() != InputMain)
		SaveInputMainEE( InputMain );

	//----------------
	// initialize video input
	InitInputAsDefault();

	//
	// Check and Set VADC,mesaure,Scaler for Analog PC input
	//
	ret = CheckAndSetDVI();		//same as CheckAndSetInput()
	if(ret==0) {
		//success
		VInput_enableOutput(0);
		return 0;
	}

	//------------------
	// NO SIGNAL
	// Prepare NoSignal Task...
	VInput_gotoFreerun(0);

	//dPrintf("\nChangeDVI--END");
	return(2);
}
#endif

//=============================================================================
// HDMI
//=============================================================================

#if defined(SUPPORT_HDMI)
/**
* Set Scaler Output for HDMI
*/
static void	HDMISetOutput(WORD HActive, WORD VActive, BYTE	vde )
{
	DECLARE_LOCAL_page
	BYTE	HDE;
	WORD HTotal, VTotal;
	WORD VScale;
	DWORD dTemp;

	ReadTW88Page(page);

	ScalerSetHScale(HActive);
	ScalerSetVScale(VActive);

	//=============HDE=====================
	HDE = ScalerCalcHDE();
#ifdef DEBUG_DTV
	dPrintf("\n\tH-DE start = %bd", HDE);
#endif


	//=============VDE=====================
	// 	MeasVStart ??R536:R537
	//	MeasVPulse ??R52A,R52B
	
//	temp16 = ScalerCalcVDE();
//	dPrintf("\n\tV-DE start = %d", temp16);

	VScale = ScalerReadVScaleReg();

#ifdef DEBUG_DTV
	dPrintf("\n\tV-DE start = %bd", vde);
#endif
	dTemp = vde;
	dTemp = (dTemp * 8192L) / VScale;
	dTemp++;
	vde = dTemp;
#ifdef DEBUG_DTV
	dPrintf("=> %bd", vde);
#endif


	//-------------------------------
	//BK120803. Did you run the measure ?
	//If you did not, you can not get the correct calculated VTotal value.

	//=================  Free Run settings ===================================
	HTotal=ScalerCalcFreerunHtotal();
#ifdef DEBUG_DTV
	dPrintf("\n\tFree Run Htotal: 0x%x", HTotal);
#endif

	//================= V Free Run settings ===================================
	VTotal=ScalerCalcFreerunVtotal();
#ifdef DEBUG_DTV
	dPrintf("\n\tFree Run Vtotal: 0x%x", VTotal);
#endif

	//---------------------UPDATE-----------------------------
//delay1s(1,__LINE__);
	//BK120803
	// If FW updates HDE Start and VDE Start, we will see the flick on the logo image.
	// Remove logo here and redraw an video image will remove a logo blink.
	WaitVBlank(1);
	WriteTW88Page(PAGE2_SCALER );							//Trick.
	WriteTW88(REG400, ReadTW88(REG400) & ~0x04);			//disable SpiOSD
	//ScalerWriteHDEReg(HDE);
	//ScalerWriteVDEReg(vde);
	//ScalerWriteFreerunHtotal(HTotal);
	//ScalerWriteFreerunVtotal(VTotal);
#if 0
	WriteTW88Page(PAGE2_SCALER );
	WriteTW88(REG210, HDE);
	WriteTW88(REG215, vde);
	WriteTW88(REG21C, (ReadTW88(REG21C)&0x0F)|(HTotal>>4)&0xF0);
	WriteTW88(REG21D, (BYTE)HTotal );
	WriteTW88(REG20D, (ReadTW88(REG20D)&0x3F)|(VTotal>>2)&0xC0);
	WriteTW88(REG219, (BYTE)VTotal );
#else
	ScalerWriteHDEReg(HDE);
	ScalerWriteVDEReg(vde);
	ScalerWriteFreerunHtotal(HTotal);
	ScalerWriteFreerunVtotal(VTotal);
#endif


	//================= FreerunAutoManual, MuteAutoManual =====================
	ScalerSetFreerunAutoManual(ON,OFF);
	ScalerSetMuteAutoManual(ON,OFF);

	WriteTW88Page(page);
}

#ifdef SUPPORT_HDMI_EP9351
/**
* check AVI InfoFrame
* 
* call when $3D[7:6] == 11b
*           $3C[4] == 1
* check $29[0] first.
*/
BYTE CheckAviInfoFrame(void)
{
	BYTE TempByte[15];
	BYTE bTemp;
	BYTE result;
	//------------------------------------
	//check AVI InfoFrame

	bTemp = ReadI2CByte(I2CID_EP9351,EP9351_HDMI_INT);			//$29

#ifdef DEBUG_DTV
	Printf("\nCheckAviInfoFrame $29:%02bx", bTemp);
#endif
	if((bTemp & 0x01) == 0) {
#ifdef DEBUG_DTV
		Puts(" FAIL");
#endif
		return ERR_FAIL;
	}

	//---------------------
	// found AVI InfoFrame.
	//---------------------
	
	//read AVI InfoFrame from $2A
//	DBG_PrintAviInfoFrame();
	ReadI2C_B0(I2CID_EP9351, EP9351_AVI_InfoFrame, TempByte, 15);

	//---------------------
	//color convert to RGB
	//---------------------
	//	 Y [2][6:5]	   Input HDMI format
	//			0:RGB
	//			1:YUV(422)
	//			2:YUV(444)
	//			3:Unused
	bTemp = (TempByte[2] & 0x60) >> 5;
//#ifdef DEBUG_DTV
//	Puts("\nInput HDMI format ");
//	if(bTemp == 0) 		Puts("RGB");
//	else if (bTemp==1)  Puts("YUV(422)");
//	else if (bTemp==2)  Puts("YUV(444)");
//	else				Puts("unknown");
//#endif
	result = 0;
	if (bTemp==1)  		result = 0x50;
	else if (bTemp==2)  result = 0x10;

	// TempByte3[7:6] Colorimetry
	bTemp = (TempByte[3] & 0xc0)>>6;
	if(bTemp==2) result |= 0x04;	//BT.709
	//else if(bTemp==3) {
	//	//Extended Colorimetry Info
	//	i = TempByte[4]&0x70)>>4;
	//	...
	//}

	//	TempByte6[3:0] Pixel Repetition Factor
	//bTemp = TempByte[6] & 0x0F;
	//if(bTemp > 3) i = 0x03;	//EP9351 supports only 2 bits.
	//result |= bTemp;

//BK120731 test
//		TempBit03 =	(!TempBit04 && !(pEP9351C_Registers->Video_Status[0] & EP907M_Video_Status_0__VIN_FMT_Full_Range) || // Input RGB LR
//			 		 !TempBit05 && !(pEP9351C_Registers->Output_Format_Control & EP907M_Output_Format_Control__VOUT_FMT_Full_Range) );	// or Output RGB LR
//	result |= 0x08; //full range

	WriteI2CByte(I2CID_EP9351, EP9351_General_Control_2, result);

	return ERR_SUCCESS;
}
#endif
#if 0 //see BKFYI130224
#ifdef SUPPORT_HDMI_EP907M
/**
* check AVI InfoFrame
* 
* call when $3D[7:6] == 11b
*           $3C[4] == 1
* check $29[0] first.
*/
BYTE CheckAviInfoFrame(void)
{
	BYTE TempByte[15];
	BYTE bTemp;
	BYTE result;
	//------------------------------------
	//check AVI InfoFrame

	bTemp = ReadI2CI16Byte(I2CID_EP907M,EP907M_Interrupt_Flags);			//$29

#ifdef DEBUG_DTV
	Printf("\nCheckAviInfoFrame $0100:%02bx", bTemp);
#endif
	if((bTemp & 0x01) == 0) {
#ifdef DEBUG_DTV
		Puts(" FAIL");
#endif
		return ERR_FAIL;
	}

	//---------------------
	// found AVI InfoFrame.
	//---------------------
	
	//read AVI InfoFrame from $2A
	ReadI2CI16(I2CID_EP907M, EP907M_AVI_Information, TempByte, 6);
	//DBG_PrintAviInfoFrame();

	//---------------------
	//color convert to RGB
	//---------------------
	//	 Y [2][6:5]	   Input HDMI format
	//			0:RGB
	//			1:YUV(422)
	//			2:YUV(444)
	//			3:Unused
	bTemp = (TempByte[1] & 0x60) >> 5;
//#ifdef DEBUG_DTV
//	Puts("\nInput HDMI format ");
//	if(bTemp == 0) 		Puts("RGB");
//	else if (bTemp==1)  Puts("YUV(422)");
//	else if (bTemp==2)  Puts("YUV(444)");
//	else				Puts("unknown");
//#endif
	result = 0;
	if (bTemp==1)  		result = 0x50;
	else if (bTemp==2)  result = 0x10;

	// TempByte3[7:6] Colorimetry
	bTemp = (TempByte[2] & 0xc0)>>6;
	if(bTemp==2) result |= 0x04;	//BT.709
	//else if(bTemp==3) {
	//	//Extended Colorimetry Info
	//	i = TempByte[4]&0x70)>>4;
	//	...
	//}

	//	TempByte6[3:0] Pixel Repetition Factor
	//bTemp = TempByte[6] & 0x0F;
	//if(bTemp > 3) i = 0x03;	//EP9351 supports only 2 bits.
	//result |= bTemp;

//BK120731 test
//		TempBit03 =	(!TempBit04 && !(pEP9351C_Registers->Video_Status[0] & EP907M_Video_Status_0__VIN_FMT_Full_Range) || // Input RGB LR
//			 		 !TempBit05 && !(pEP9351C_Registers->Output_Format_Control & EP907M_Output_Format_Control__VOUT_FMT_Full_Range) );	// or Output RGB LR
//	result |= 0x08; //full range

	WriteI2CI16Byte(I2CID_EP907M, EP907M_Video_Format_Control, result);

	return ERR_SUCCESS;
}
#endif
#endif

//CheckAndSetHDMI and CheckAndSetLVDS use a same routines to detect the HDMI chip output.
//let's merge it.
/*
	DEBUG_DTV__TEST.

	EP907M time register and TW8836 meas value have a same result.
	So, if some assumeption is correct, we can reduce the code size.

				pol		Sync	BPorch		ScalerStart
		1080p	h:0 	44(44)	148(189)	192+1(189+4)
				v:0		5(5)	36(42)		41+2(42+1)
		1080i	h:0		44(44)	148(188)	192+1(188+4+1)
				v:0		5(5)	15(21)		20+2(21+1)
		 720p	h:0		40(40)	220(256)	260+1(256+4+1)
		 		v:0		5(5)	20(26)		25+2(26+1)
		480p	h:0		62(62)	60(118)		122+1(118+4)
				v:0		6(6)	30(37)		36+2(37+1)
		480i	h:0		124(62)	114(115)	(124+114+1)/2(115+4)
				v:0		3(3)	15(19)		18+2(19+1)		
		note:() is for meas value.	

		Timing Register value and the measued value have a same result.
		We can ignore one of them.
*/
BYTE CheckHdmiChipRegister(void)
{
	DECLARE_LOCAL_page
	BYTE ret;
	BYTE i;


#ifdef DEBUG_DTV__TEST
	BYTE TempByte[8];
	WORD hFPorch,vFPorch;
	WORD hBPorch,vBPorch;
#endif
	BYTE hPol,vPol;		
	WORD hTotal,vTotal;
	WORD hActive,vActive;
	WORD hSync,vSync;
		
	WORD hCropStart,vCropStart;
	BYTE bTemp;

	LVDS_Video_Status = 0;

	//read System Status register $201.
	//If it is too fast, VISR will take care.
	bTemp = ReadI2CI16Byte(I2CID_EP907M, EP907M_System_Status_1 );
	if((bTemp & 0xC0) != 0xC0) {
		Printf(" => NoSignal");
#ifdef DEBUG_DTV
		dPrintf(" $201:%bx",bTemp);
#endif
		return ERR_FAIL;
	}
	//read System Status register $200.
	//if $200[4]==1, HDMI mode.
	//actually, ...
	for(i=0; i < 10; i++) {
		bTemp = ReadI2CI16Byte(I2CID_EP907M, EP907M_System_Status_0 );
		if(bTemp & EP907M_System_Status_0__HDMI)
			break;
		delay1ms(10);
	}
	if(bTemp & EP907M_System_Status_0__HDMI)
		Puts(" HDMI mode");
	else
		Puts(" DVI or VESA mode");
#ifdef DEBUG_DTV
	dPrintf(" $200:%bx @%bx",bTemp, i);
#endif

#if 0
	//BKFYI130224. If we using AUTO_VFMTb==0, we don't need it.
	//
	//set color space.
	if(bTemp & EP907M_System_Status_0__HDMI) {
		ret = CheckAviInfoFrame();
	}
	else {
		//BK130204. We need a EEPROM value.
		WriteI2CI16Byte(I2CID_EP907M, EP907M_Video_Format_Control, 0x00 ); 	//clear
	}
#endif

#ifdef DEBUG_DTV__TEST
	//-----------------------------
	// read HSync timing register value, $0600~$0607.
	//-----------------------------
	ReadI2CI16(I2CID_EP907M, EP907M_Hsync_Timing, TempByte, 8);

	hActive = TempByte[0]; 	hActive <<= 8;		hActive += TempByte[1];
	hFPorch = TempByte[2]; 	hFPorch <<= 8;		hFPorch += TempByte[3];
	hBPorch = TempByte[4]; 	hBPorch <<= 8;		hBPorch += TempByte[5];
	hSync   = TempByte[6]; 	hSync <<= 8;		hSync   += TempByte[7];
	hTotal = hSync + hBPorch + hActive + hFPorch;

	//-----------------------------
	// read VSync timing register value, $0700~0704.
	//-----------------------------
	ReadI2CI16(I2CID_EP907M, EP907M_Vsync_Timing, TempByte, 5);
	vActive = TempByte[0]; 	vActive <<= 8;		vActive += TempByte[1];
	vFPorch =  TempByte[2];
	vBPorch =  TempByte[3];
	vSync =  TempByte[4]&0x7F;
	vTotal = vSync + vBPorch + vActive + vFPorch;

	//-----------------------------
	// read Polarity Control register. $2001
	// $2001[0] HS_POL
	//		1 = HSYNC pin is negative polarity (high during video active period).
	// $2001[1] VS_POL
	//		1 = VSYNC pin is negative polarity (high during video active period).
	//
	// I do not change $2001[1:0], so I assume, it is a 00b.
	//-----------------------------
	bTemp = ReadI2CI16Byte(I2CID_EP907M,EP907M_Polarity_Control);  /* NOTE: $41 */
	hPol = bTemp & EP907M_Polarity_Control__HS_POL_Low ? 1:0;		//ActiveLow
	vPol = bTemp & EP907M_Polarity_Control__VS_POL_Low ? 1:0;		//ActiveLow

	//select hStart, vStart for InputCrop.
	hCropStart = hBPorch + 1;
	if(hPol==0)
		hCropStart += hSync;	
	vCropStart = vBPorch + 2;
	if(vPol==0)
		vCropStart += vSync;


	//if source is 720x240, EP907M reports it as 1440x240.
	//I assume, 720x288, also same.
	if(hActive==1440) {
		if(vActive==240 || vActive==288) {
			hActive = 720;
			vCropStart >>=1;	 //div2
		}
	}
	Printf("\nTimeReg %d hPol:%bx hSync:%d hBPorch:%d hCropStart:%d",hActive, hPol, hSync, hBPorch, hCropStart);
	Printf("\n        %d vPol:%bx vSync:%d vBPorch:%d vCropStart:%d",vActive, vPol, vSync, vBPorch, vCropStart);
#endif	


	ret=MeasStartMeasure();
	if(ret) {
		//if measure fail, it measn no signal... BK130104.
#ifdef DEBUG_DTV
		Printf(" meas=> NoSignal");
#endif
		return ERR_FAIL;
	}
	//=================================
	// check polarity.
	//=================================
	hSync = MeasGetHSyncRiseToFallWidth();
	vSync = MeasGetVSyncRiseToFallWidth();
	hTotal = MeasGetVsyncRisePos();
	vTotal = MeasGetVPeriod();  //BK130204, vtotal is better. We can use a meas vCropStart. 
	hActive = MeasGetHActive( &hCropStart );
	vActive = MeasGetVActive( &vCropStart );

	if ( hSync > (hActive/2) )	hPol = 1;	//active low. something wrong.
	else						hPol = 0;	//active high
	if ( vSync > (vTotal/2) )	vPol = 1;	//active low. something wrong.
	else						vPol = 0;	//active high


#ifdef DEBUG_DTV
	Printf("\nMeas %d hPol:%bx hCropStart:%d+4",hActive, hPol, hCropStart);
	Printf("\n     %d vPol:%bx vCropStart:%d+1",vActive, vPol, vCropStart);
#endif
	if(hPol || vPol)
		Printf("\nBUG hPol:%bx vPol:%bx",hPol,vPol);

	//
	//set DTV hPol and vPol polarity.
	DtvSetPolarity(hPol, vPol);	//base ActiveHigh		
//#ifdef EVB_10
//BK130306 always use clear.
//	if(vActive ==1080 && (hActive > 1910 && hActive < 1930))
//			WriteTW88(REG047, ReadTW88(REG047) & ~0x20);	//clear
//	else	WriteTW88(REG047, ReadTW88(REG047) |  0x20);	//normal
//#endif

	hCropStart = hCropStart + 4;	//hBPorch = meas_hBPorch+4. hCropStart = hBPorch+1. 			
	vCropStart = vCropStart + 1;	//vBPorch = meas_vBPorch-1. vCropStart = vBPorch+2;



#ifdef DEBUG_DTV
	dPrintf("\nInputCrop hStart:%d vStart:%d",hCropStart,vCropStart);
#endif


	//-------------------------------------------
	// input crop
	// if implicit DE, use 1 for VStart.
	// if explicit DE, use VStart crop value.
	//InputSetCrop(HActiveStart, 1, HActive, 0x7fe);		//implicit DE
	InputSetCrop(hCropStart, vCropStart, hActive, vActive);	//explicit DE

	HDMISetOutput( hActive, vActive,  vCropStart );

	if(InputMain==INPUT_HDMIPC || InputMain==INPUT_HDMITV)
		AdjustPixelClk(hTotal, 0);	//NOTE:it uses DVI_Divider.

	if(InputMain==INPUT_LVDS)
		LVDS_Video_Status = 1;
	return ERR_SUCCESS;
}


/**
* Check and Set HDMI
*
* Hot Boot: Reset only TW8835.
*		Hot boot needs a EP9351 Software Reset, but FW does not support it anymore.
*		Please, use a Reset button or Power Switch.
*/
BYTE CheckAndSetHDMI(void)
{
	dPuts("\nCheckAndSetHDMI START");
	return CheckHdmiChipRegister();
}
#endif //..defined(SUPPORT_HDMI_EP9351)

/**
* some HDMI chip needs to download EDID & HDCP key.
*/
#if 0
void Init_HdmiSystem(void)
{
#ifdef SUPPORT_HDMI_EP9351
	//it will download EDID & HDCP
	Hdmi_SystemInit_EP9351();
#endif
#ifdef SUPPORT_HDMI_EP9553
	Hdmi_SystemInit_EP9553();
#endif
#ifdef SUPPORT_HDMI_EP907M
	Hdmi_SystemInit_EP907M();
#endif
}
#endif

//=============================================================================
// Change to HDMI
//=============================================================================

//-----------------------------------------------------------------------------
/**
* Change to HDMI
*
* linked with EP9553E.
* @return
*	- 0: success
*	- 1: No Update happen
*	- 2: No Signal or unknown video sidnal.
*/
BYTE ChangeHDMI(void)
{
	BYTE ret;

	if ( InputMain == INPUT_HDMIPC || InputMain == INPUT_HDMITV ) {
		dPrintf("\nSkip ChangeHDMI");
		return(1);
	}

	if(GetHdmiModeEE())  InputMain = INPUT_HDMITV;
	else 				 InputMain = INPUT_HDMIPC;

	if(GetInputMainEE() != InputMain)
		SaveInputMainEE( InputMain );

	dPrintf("\nChangeHDMI InputMain:%02bx",InputMain);

	//----------------
	//remove logo first. 
	//BK130123. FW will not support Logo on HDMI mode.


	//----------------
	// initialize video input
	InitInputAsDefault();

#ifdef SUPPORT_HDMI_EP907M
	HdmiInitEp907MChip();
#endif

	//
	// Check and Set 
	//
#if defined(SUPPORT_HDMI)
	ret = CheckAndSetHDMI();
#else
	ret=ERR_FAIL;
#endif
	if(ret==ERR_SUCCESS) {
		//success
		VInput_enableOutput(0);
		return 0;
	}

	//------------------
	// NO SIGNAL
	// Prepare NoSignal Task...
	VInput_gotoFreerun(0);
	//dPrintf("\nChangeHDMI--END");

	return(2);
}


//=============================================================================
// Change to LVDSRx
//=============================================================================
#ifdef SUPPORT_LVDSRX

/**
* TW8836 EVB1.0 uses a HDMI output for LVDS Tx chip input.
*
* video path
* HDMI=>LVDS Tx chip =>TW8836 LVDSRx => Scaler => Panel
*/
BYTE CheckAndSetLVDSRx(void)
{
	dPuts("\nCheckAndSetLVDS START");
	/*
		REG004[0] has a problem on the first silicon.
		So, I am using the same HDMI routine.
	*/
	return CheckHdmiChipRegister();
}

BYTE ChangeLVDSRx(void)
{
	BYTE ret;
	BYTE i;

	if ( InputMain == INPUT_LVDS ) {
		dPrintf("\nSkip ChangeLVDSRx");
		return(1);
	}
	InputMain = INPUT_LVDS;

	if(GetInputMainEE() != InputMain)
		SaveInputMainEE( InputMain );

	//----------------
	// initialize video input
	InitInputAsDefault();

	HdmiInitEp907MChip();

	//BK121218.  VISR does not work.
	//			read EP9553E status register.
	for(i=0; i < 200; i++) {
		ret = ReadI2CI16Byte(I2CID_EP907M, 0x000);
		if(ret==0x01) {
			ret = ReadI2CI16Byte(I2CID_EP907M, 0x201);
			if(ret & 0x80) {
				ret = ReadI2CI16Byte(I2CID_EP907M, 0x200);
				if((ret & 0x20) == 0) {
#ifdef DEBUG_DTV
					Printf("\nLINK_ON@%d $R200:%bx $R201:%bx",(WORD)i, ret, ReadI2CI16Byte(I2CID_EP907M, 0x201));
#endif
					break;	
				}
			}
		}
		delay1ms(10);
	}

	//
	// Check and Set Input crop, scale rate, scaler output time.
	//
	ret = CheckAndSetLVDSRx();
	//dPrintf("\nLVDS input Detect: %s", ret ? "No" : "Yes" );

	//add timer. Need SFR_EA
	LVDS_timer =  SystemClock + 200;
	if(LVDS_timer < SystemClock)
		LVDS_timer = 200;
		
 	if(ret==0) {
		//success
		VInput_enableOutput(0);
		return 0;
	}

	//------------------
	// NO SIGNAL
	// Prepare NoSignal Task...

	VInput_gotoFreerun(0);
	return(2);
}
#endif





//=============================================================================
// Change to BT656
//=============================================================================
#ifdef SUPPORT_BT656_LOOP
//-----------------------------------------------------------------------------
/**
* Check and Set BT656
*
* @return
*	- 0: success
*	- other: error
* @todo REG04A[0] does not work.
*
* video path
* CVBS=>BT656Out=>DTV i656=>Scaler=>Panel.
*/
BYTE CheckAndSetBT656Loop(void)
{
	//BYTE value;
	BYTE	mode;
	BYTE hDelay,vDelay;
	WORD hActive,vActive;
	DWORD dTemp;
	BYTE bTemp;
	WORD MeasVStart, MeasHStart;
	WORD MeasHLen, MeasVLen;

	dPrintf("\nCheckAndSetBT656Loop start.");

	//
	//check decoder
	//
	if ( DecoderCheckVDLOSS(100) ) {
		return 1; //no decoder signal
	}
	mode = DecoderCheckSTD(100);
	if ( mode == 0x80 ) {
	    ePrintf("\n NoSTD");
		return( 2 );
	}
	mode >>= 4;
	InputSubMode = mode;

	//
	//init decoder input crop.
	//
	if(mode==0 || mode==3 || mode==4 || mode ==6) {
		//NTSC.
		hActive = 720; vActive = 240;
		//Decoder & BT656Loopback using different value.
		//Decoder using	 8,720,21,240.
		//BT656 will use 13,720,21,240.
		hDelay = 13;   vDelay = 21;

	}
	else if(mode==1 || mode==2 || mode==5) {
		// PAL.
		hActive = 720; vActive = 288;
		//Decoder & BT656Loopback using different value.
		//Decoder using	 6,720,23,288.
		//BT656 will use 11,720,22,288
		hDelay = 11;   vDelay = 22;

	}
	else {
		Printf("\nUnknown mode;%bx",mode);
		return 3;
	}
	DecoderSetInputCrop(hDelay,hActive,vDelay,vActive);

	//
	//set External BT656 Encoder
	//
	if(mode==0 || mode==3 || mode==4 || mode ==6)
		BT656_InitExtEncoder(BT656_8BIT_525I_YCBCR_TO_CVBS);
	else
		BT656_InitExtEncoder(BT656_8BIT_625I_YCBCR_TO_CVBS);
	
	//
	//init scaler inputcrop,scale rate, scale output time.	
	//
	if(MeasStartMeasure()) {
		//it is comes from decoder. let's use a measure.
		//if meas success, please remove this routine...
		ScalerSetLineBufferSize(hActive);	//BK120116	- temp location. pls remove
	
		ScalerSetHScale(hActive);
		ScalerSetVScale(vActive);
		dTemp = vDelay;
		dTemp *= PANEL_V;
		dTemp /= vActive;

		//BK130129. If NTSC, use 38(it was 42). so reduce 4;
		if(mode==0)
			dTemp -=4;

		ScalerWriteVDEReg((WORD)dTemp);
		return 0;
	}

	MeasVLen = MeasGetVActive( &MeasVStart );				//v_active_start v_active_perios
	MeasHLen = MeasGetHActive( &MeasHStart );				//h_active_start h_active_perios
	dPrintf("\nBT656 Measure Value: %dx%d hStart:%d+4 vStart:%d-1",MeasHLen,MeasVLen, MeasHStart, MeasVStart);
	dPrintf("==>Htotal:%d",  MeasGetVsyncRisePos());
	InputSetCrop(MeasHStart+4,MeasVStart+1, MeasHLen,MeasVLen);	 //add 2 more for scaler on H value.

	//implicit DE.
	bTemp = ReadTW88(REG041);
	bTemp &= ~0x01;		//YUV;
	bTemp |= 0x10;		//Implicit DE
	WriteTW88(REG041,bTemp);

	ScalerSetLineBufferSize(MeasHLen / 2);
	ScalerSetHScale(MeasHLen / 2);
	ScalerSetVScale(MeasVLen);
	ScalerSetVScale(vActive);

	dTemp = MeasVStart + 1;
	dTemp *= PANEL_V;
	dTemp /= MeasVLen;
	if(mode==1)
		dTemp += 6; 
	ScalerWriteVDEReg((WORD)dTemp);

	return 0;
}
#endif

//-----------------------------------------------------------------------------
//		BYTE	ChangeBT656Loop( void )
//-----------------------------------------------------------------------------
#ifdef SUPPORT_BT656_LOOP
//-----------------------------------------------------------------------------
/**
* Change to BT656Loop
*
* VideoPath
*	CVBS=>BT656 Out=>VD[] bus=>DTV(i656)=>Scaler=>Panel.
*
* @return
*	- 0: success
*	- 1: No Update happen
*	- 2: No Signal or unknown video sidnal.
*/
BYTE ChangeBT656Loop(void)
{
	BYTE ret;

	if ( InputMain == INPUT_BT656 ) {
		dPrintf("\nSkip ChangeBT656");
		return(1);
	}
	InputMain = INPUT_BT656;

	if(GetInputMainEE() != InputMain)
		SaveInputMainEE( InputMain );

	//----------------
	// initialize video input
	InitInputAsDefault();

#ifdef SUPPORT_BT656_LOOP
	delay1ms(350);	//decoder need a delay.
#endif

	//
	// Check and Set DEC,mesaure,Scaler for BT656 Loopback input
	//
	ret = CheckAndSetBT656Loop();
 	if(ret==0) {
		//success
		VInput_enableOutput(0);
		return 0;
	}

	//------------------
	// NO SIGNAL
	// Prepare NoSignal Task...

	VInput_gotoFreerun(0);

	return(2);
}
#endif

