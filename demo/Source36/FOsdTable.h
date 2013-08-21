#ifndef __FOSDINITTABLE_H__
#define	__FOSDINITTABLE_H__

#ifndef SUPPORT_FOSD_MENU
//----------------------------
//Trick for Bank Code Segment
//----------------------------
extern CODE BYTE DUMMY_FOSD_INITTABLE_CODE;
void Dummy_FosdInitTable_func(void);
#else //..SUPPORT_FOSD_MENU

// Index 0x00 Defination
#define OSD_Win_CONTROL_00(a)		a
#define WIN_EN				0x80
#define WIN_EN0				0x00
#define WIN_MULTI_EN		0x40
#define WIN_MULTI_EN0		0x00
#define WIN_V_EXT_EN		0x20
#define WIN_V_EXT_EN0		0x00
#define WIN_HZOOM(a)		((a-1)<<2)
#define WIN_VZOOM(a)		(a-1)

//Attributes for OSD
#define WINZOOMx1			0x01
#define WINZOOMx2			0x02
#define WINZOOMx3			0x03
#define WINZOOMx4			0x04
#define SHOW				0x08	

// index 0x01
#define OSD_Win_Blending_01(a)		a

// index 0x02 ~ 0x04
#define OSD_Win_Start_02_4(h,v)		(BYTE)(((WORD)v>>8) | (((WORD)h>>8)<<4)),(BYTE)h, (BYTE)v

// index 0x05
#define OSD_Win_Height_05(a)		a

// index 0x06
#define OSD_Win_Width_06(a)			a

// index 0x07
#define OSD_Win_BorderColor_07(a)	a

// index 0x08 - Border_Color_Enable
#define OSD_Win_Border_08(a)		a
#define WIN_BD_EN			0x80
#define WIN_BD_EN0			0x00
#define WIN_BD_WIDTH(a)		a

// index 0x09
#define OSD_Win_HBorder_09(a)		a
#define WIN_H_BD_W(a)		a

// index 0x0a
#define OSD_Win_VBorder_0A(a)		a
#define WIN_V_BD_W(a)		a

// index 0x0b
#define	OSD_Win_3D_0B(a)			a
#define WIN_3D_EN				0x80
#define WIN_3D_EN0				0x00
#define WIN_3D_TYPE				0x40
#define WIN_3D_TYPE0			0x00
#define WIN_3D_LEVEL			0x20
#define WIN_3D_LEVEL0			0x00
#define WIN_CH_BDSH_SEL(a)		a
#define SEL_BORDER				0
#define SEL_SHADOW				0x10

// index 0x0c
//#define OSD_Win_Shadow_0C(a)		a
#define WIN_SHADOW_EN		0x80
#define WIN_SHADOW_EN0		0x00
#define WIN_SHADOW_W(a)		a

// index 0x0d
//#define OSD_Win_CH_Space_0D(a)		a			
#define WIN_CH_VSPACE(a)	a
#define WIN_CH_HSPACE(a)	a

#define OSD_Win_0C_0D(a,h,v)		a | ((h & 0x10)<<2) | ((v&0x10)<<1), (h<<4) | (v&0x0F)

// index 0x0e
#define OSD_Win_Attr_0E(a)			a			

// index 0x0f
#define OSD_Win_SAddr_0F(a)			a			
#define WIN_SADDR(a)		a

#define WIN_BG_COLOR_EX		0x80
#define WIN_BG_COLOR_EX0	0x00

#define WIN_BD_COLOR_EX		0x80
#define WIN_BD_COLOR_EX0	0x00

#define WIN_SHADOW_COLOR_EX		0x80
#define WIN_SHADOW_COLOR_EX0	0x00

#define WIN_CH_BDSH_COLOR_EX	0x08
#define WIN_CH_BDSH_COLOR_EX0	0x00


#define OSD_Win_Num(a)				(a)

//#define	SetOSDWindowNum(winno)		WriteTW88(TW88_WINNUM, winno-1)

//#define  OSD_Window_Attr(Trans_en,Color,3D_Type,3D_en,W_en)       (((( Trans_en | Color ) | 3D_Type) | 3D_en) | W_en)
#define  OSD_Window_Attr(a,b,c,d,e)									(((( a | b ) | c) | d) | e)
#define  OSD_WShadow_Attr(W_Shadow_en,Color,Width)					W_Shadow_en | Color | Width
//#define  OSD_Blending(en,level)										en | (level & 0x0f)
//#define  OSD_Ch_Effect(Multi, Effect_En, ShadowBorder, Color)		Multi | Effect_En | ShadowBorder | Color  


#define FOSD_UNKNOWN	0xfe


struct struct_IdName {
	BYTE	Id;
	BYTE	Name[16];
};


//---------------------------------
// DefineMenu
//---------------------------------
extern CODE struct DefineMenu DefaultMenu[];



extern CODE BYTE Init_Osd_DisplayLogo[];
#ifdef ADD_ANALOGPANEL
extern CODE BYTE Init_Osd_DisplayLogo_A[];
#endif
extern CODE BYTE Init_Osd_DisplayVchipWindow[];
extern CODE BYTE Init_Osd_DisplayMuteInfo[];
extern CODE BYTE Init_Osd_DisplayTVChannel[];
extern CODE BYTE Init_Osd_DisplayPCInfo[];
extern CODE BYTE Init_Osd_DisplayInput[];
#ifdef ADD_ANALOGPANEL
extern CODE BYTE Init_Osd_DisplayInput_A[];
#endif
extern CODE BYTE Init_Osd_MainMenu[];
extern CODE BYTE Init_Osd_BarWindow[];
extern CODE struct struct_IdName	struct_InputSelection[];
extern CODE struct struct_IdName struct_VInputStd[];
#endif  //..#else //..SUPPORT_FOSD_MENU
#endif //..__FOSDINITTABLE_H__