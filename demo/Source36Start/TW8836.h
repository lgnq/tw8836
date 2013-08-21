#ifndef _TW8836_H_
#define	_TW8836_H_

//==<<PAGE & TWBASE>>=================================================
#define PAGE0_GENERAL		0x00
#define PAGE0_OUTPUT		0x00	//0x007::0x00F & 0x01F
#define PAGE0_INPUT			0x00	//0x040::0x046
#define PAGE0_BT656			0x00	//0x047::0x04A
#define PAGE0_DTV			0x00	//0x050::0x05F
#define PAGE0_GPIO			0x00	//0x080::0x09E
#define PAGE0_MBIST			0x00	//0x0A0::0x0A4
#define PAGE0_TOUCH			0x00	//0x0B0::0x0B4
#define PAGE0_LOPOR			0x00	//0x0D4::0x0DF
#define PAGE0_LEDC			0x00	//0x0E0::0x0E6
#define PAGE0_DCDC			0x00	//0x0E8::0x0EC
#define PAGE0_VCOM			0x00	//0x0ED::0x0EE
#define PAGE0_SSPLL			0x00	//0x0F6::0x0FD
#define PAGE1_DECODER		0x01	//0x101::0x142
#define PAGE1_VADC			0x01	//0x1C0::0x1E7	include LLPLL
#define PAGE1_LLPLL			0x01	//0x1C0::0x1E7	include LLPLL
#define PAGE2_SCALER		0x02	//0x201::0x21E
#define PAGE2_TCON			0x02	//0x240::0x24E
#define PAGE2_IMAGE			0x02	//0x280::0x2BF
#define PAGE2_GAMMA			0x02	//0x2E0::0x2E3 include LCDC
#define PAGE2_DITHER		0x02	//0x2E4::0x2E4
#define PAGE2_RGBLEVEL		0x02	//0x2F0::0x2F5
#define PAGE2_8BITPANEL		0x02	//0x2F8::0x2F9
#define PAGE3_FOSD			0x03	//0x300::0x354
#define PAGE4_SOSD			0x04	//0x400::0x3BE
#define PAGE4_SPI			0x04	//0x4C0::0x4DA include MCU
#define PAGE4_CLOCK			0x04	//0x4E0::0x4EB
#define PAGE5_MEAS			0x05	//0x500::0x545


//===========================
// PAGE 0
//===========================
	#define REG000	0x000
	#define REG001	0x001
	#define REG002	0x002
	#define REG003	0x003
	#define REG004	0x004
	#define REG005	0x005
	#define REG006	0x006
	#define REG007	0x007
	#define REG008	0x008
	#define REG009	0x009
	#define REG00A	0x00A
	#define REG00B	0x00B
	#define REG00C	0x00C
	#define REG00D	0x00D
	#define REG00E	0x00E
	#define REG00F	0x00F

	#define REG010	0x010
	#define REG011	0x011
	#define REG012	0x012
	#define REG013	0x013
	#define REG014	0x014
	#define REG015	0x015
	#define REG016	0x016
	#define REG017	0x017
	#define REG018	0x018
	#define REG019	0x019
	#define REG01A	0x01A
	#define REG01B	0x01B
	#define REG01C	0x01C
	#define REG01D	0x01D
	#define REG01E	0x01E
	#define REG01F	0x01F

	#define REG020	0x020
	#define REG021	0x021
	#define REG022	0x022
	#define REG023	0x023
	#define REG024	0x024
	#define REG025	0x025
	#define REG026	0x026
	#define REG027	0x027
	#define REG028	0x028
	#define REG029	0x029
	#define REG02A	0x02A
	#define REG02B	0x02B
	#define REG02C	0x02C
	#define REG02D	0x02D
	#define REG02E	0x02E
	#define REG02F	0x02F

	#define REG030	0x030
	#define REG031	0x031
	#define REG032	0x032
	#define REG033	0x033
	#define REG034	0x034
	#define REG035	0x035
	#define REG036	0x036
	#define REG037	0x037
	#define REG038	0x038
	#define REG039	0x039
	#define REG03A	0x03A
	#define REG03B	0x03B
	#define REG03C	0x03C
	#define REG03D	0x03D
	#define REG03E	0x03E
	#define REG03F	0x03F

	#define REG040	0x040
	#define REG041	0x041
	#define REG042	0x042
	#define REG043	0x043
	#define REG044	0x044
	#define REG045	0x045
	#define REG046	0x046
	#define REG047	0x047
	#define REG048	0x048
	#define REG049	0x049
	#define REG04A	0x04A
	#define REG04B	0x04B
	#define REG04C	0x04C
	#define REG04D	0x04D
	#define REG04E	0x04E
	#define REG04F	0x04F

	#define REG050	0x050
	#define REG051	0x051
	#define REG052	0x052
	#define REG053	0x053
	#define REG054	0x054
	#define REG055	0x055
	#define REG056	0x056
	#define REG057	0x057
	#define REG058	0x058
	#define REG059	0x059
	#define REG05A	0x05A
	#define REG05B	0x05B
	#define REG05C	0x05C
	#define REG05D	0x05D
	#define REG05E	0x05E
	#define REG05F	0x05F

	#define REG060	0x060
	#define REG061	0x061
	#define REG062	0x062
	#define REG063	0x063
	#define REG064	0x064
	#define REG065	0x065
	#define REG066	0x066
	#define REG067	0x067
	#define REG068	0x068
	#define REG069	0x069
	#define REG06A	0x06A
	#define REG06B	0x06B
	#define REG06C	0x06C
	#define REG06D	0x06D
	#define REG06E	0x06E
	#define REG06F	0x06F

	#define REG070	0x070
	#define REG071	0x071
	#define REG072	0x072
	#define REG073	0x073
	#define REG074	0x074
	#define REG075	0x075
	#define REG076	0x076
	#define REG077	0x077
	#define REG078	0x078
	#define REG079	0x079
	#define REG07A	0x07A
	#define REG07B	0x07B
	#define REG07C	0x07C
	#define REG07D	0x07D
	#define REG07E	0x07E
	#define REG07F	0x07F

	#define REG080	0x080
	#define REG081	0x081
	#define REG082	0x082
	#define REG083	0x083
	#define REG084	0x084
	#define REG085	0x085
	#define REG086	0x086
	#define REG087	0x087
	#define REG088	0x088
	#define REG089	0x089
	#define REG08A	0x08A
	#define REG08B	0x08B
	#define REG08C	0x08C
	#define REG08D	0x08D
	#define REG08E	0x08E
	#define REG08F	0x08F

	#define REG090	0x090
	#define REG091	0x091
	#define REG092	0x092
	#define REG093	0x093
	#define REG094	0x094
	#define REG095	0x095
	#define REG096	0x096
	#define REG097	0x097
	#define REG098	0x098
	#define REG099	0x099
	#define REG09A	0x09A
	#define REG09B	0x09B
	#define REG09C	0x09C
	#define REG09D	0x09D
	#define REG09E	0x09E
	#define REG09F	0x09F

	#define REG0A0	0x0A0
	#define REG0A1	0x0A1
	#define REG0A2	0x0A2
	#define REG0A3	0x0A3
	#define REG0A4	0x0A4
	#define REG0A5	0x0A5
	#define REG0A6	0x0A6
	#define REG0A7	0x0A7
	#define REG0A8	0x0A8
	#define REG0A9	0x0A9
	#define REG0AA	0x0AA
	#define REG0AB	0x0AB
	#define REG0AC	0x0AC
	#define REG0AD	0x0AD
	#define REG0AE	0x0AE
	#define REG0AF	0x0AF

	#define REG0B0	0x0B0
	#define REG0B1	0x0B1
	#define REG0B2	0x0B2
	#define REG0B3	0x0B3
	#define REG0B4	0x0B4
	#define REG0B5	0x0B5
	#define REG0B6	0x0B6
	#define REG0B7	0x0B7
	#define REG0B8	0x0B8
	#define REG0B9	0x0B9
	#define REG0BA	0x0BA
	#define REG0BB	0x0BB
	#define REG0BC	0x0BC
	#define REG0BD	0x0BD
	#define REG0BE	0x0BE
	#define REG0BF	0x0BF

	#define REG0C0	0x0C0
	#define REG0C1	0x0C1
	#define REG0C2	0x0C2
	#define REG0C3	0x0C3
	#define REG0C4	0x0C4
	#define REG0C5	0x0C5
	#define REG0C6	0x0C6
	#define REG0C7	0x0C7
	#define REG0C8	0x0C8
	#define REG0C9	0x0C9
	#define REG0CA	0x0CA
	#define REG0CB	0x0CB
	#define REG0CC	0x0CC
	#define REG0CD	0x0CD
	#define REG0CE	0x0CE
	#define REG0CF	0x0CF

	#define REG0D0	0x0D0
	#define REG0D1	0x0D1
	#define REG0D2	0x0D2
	#define REG0D3	0x0D3
	#define REG0D4	0x0D4
	#define REG0D5	0x0D5
	#define REG0D6	0x0D6
	#define REG0D7	0x0D7
	#define REG0D8	0x0D8
	#define REG0D9	0x0D9
	#define REG0DA	0x0DA
	#define REG0DB	0x0DB
	#define REG0DC	0x0DC
	#define REG0DD	0x0DD
	#define REG0DE	0x0DE
	#define REG0DF	0x0DF

	#define REG0E0	0x0E0
	#define REG0E1	0x0E1
	#define REG0E2	0x0E2
	#define REG0E3	0x0E3
	#define REG0E4	0x0E4
	#define REG0E5	0x0E5
	#define REG0E6	0x0E6
	#define REG0E7	0x0E7
	#define REG0E8	0x0E8
	#define REG0E9	0x0E9
	#define REG0EA	0x0EA
	#define REG0EB	0x0EB
	#define REG0EC	0x0EC
	#define REG0ED	0x0ED
	#define REG0EE	0x0EE
	#define REG0EF	0x0EF

	#define REG0F0	0x0F0
	#define REG0F1	0x0F1
	#define REG0F2	0x0F2
	#define REG0F3	0x0F3
	#define REG0F4	0x0F4
	#define REG0F5	0x0F5
	#define REG0F6	0x0F6
	#define REG0F7	0x0F7
	#define REG0F8	0x0F8
	#define REG0F9	0x0F9
	#define REG0FA	0x0FA
	#define REG0FB	0x0FB
	#define REG0FC	0x0FC
	#define REG0FD	0x0FD
	#define REG0FE	0x0FE
	//#define REG0FF	0x0FF

//===========================
// PAGE 1
//===========================
	#define REG100	0x100
	#define REG101	0x101
	#define REG102	0x102
	#define REG103	0x103
	#define REG104	0x104
	#define REG105	0x105
	#define REG106	0x106
	#define REG107	0x107
	#define REG108	0x108
	#define REG109	0x109
	#define REG10A	0x10A
	#define REG10B	0x10B
	#define REG10C	0x10C
	#define REG10D	0x10D
	#define REG10E	0x10E
	#define REG10F	0x10F

	#define REG110	0x110
	#define REG111	0x111
	#define REG112	0x112
	#define REG113	0x113
	#define REG114	0x114
	#define REG115	0x115
	#define REG116	0x116
	#define REG117	0x117
	#define REG118	0x118
	#define REG119	0x119
	#define REG11A	0x11A
	#define REG11B	0x11B
	#define REG11C	0x11C
	#define REG11D	0x11D
	#define REG11E	0x11E
	#define REG11F	0x11F

	#define REG120	0x120
	#define REG121	0x121
	#define REG122	0x122
	#define REG123	0x123
	#define REG124	0x124
	#define REG125	0x125
	#define REG126	0x126
	#define REG127	0x127
	#define REG128	0x128
	#define REG129	0x129
	#define REG12A	0x12A
	#define REG12B	0x12B
	#define REG12C	0x12C
	#define REG12D	0x12D
	#define REG12E	0x12E
	#define REG12F	0x12F

	#define REG130	0x130
	#define REG131	0x131
	#define REG132	0x132
	#define REG133	0x133
	#define REG134	0x134
	#define REG135	0x135
	#define REG136	0x136
	#define REG137	0x137
	#define REG138	0x138
	#define REG139	0x139
	#define REG13A	0x13A
	#define REG13B	0x13B
	#define REG13C	0x13C
	#define REG13D	0x13D
	#define REG13E	0x13E
	#define REG13F	0x13F

	#define REG140	0x140
	#define REG141	0x141
	#define REG142	0x142
	#define REG143	0x143
	#define REG144	0x144
	#define REG145	0x145
	#define REG146	0x146
	#define REG147	0x147
	#define REG148	0x148
	#define REG149	0x149
	#define REG14A	0x14A
	#define REG14B	0x14B
	#define REG14C	0x14C
	#define REG14D	0x14D
	#define REG14E	0x14E
	#define REG14F	0x14F

	#define REG150	0x150
	#define REG151	0x151
	#define REG152	0x152
	#define REG153	0x153
	#define REG154	0x154
	#define REG155	0x155
	#define REG156	0x156
	#define REG157	0x157
	#define REG158	0x158
	#define REG159	0x159
	#define REG15A	0x15A
	#define REG15B	0x15B
	#define REG15C	0x15C
	#define REG15D	0x15D
	#define REG15E	0x15E
	#define REG15F	0x15F

	#define REG160	0x160
	#define REG161	0x161
	#define REG162	0x162
	#define REG163	0x163
	#define REG164	0x164
	#define REG165	0x165
	#define REG166	0x166
	#define REG167	0x167
	#define REG168	0x168
	#define REG169	0x169
	#define REG16A	0x16A
	#define REG16B	0x16B
	#define REG16C	0x16C
	#define REG16D	0x16D
	#define REG16E	0x16E
	#define REG16F	0x16F

	#define REG170	0x170
	#define REG171	0x171
	#define REG172	0x172
	#define REG173	0x173
	#define REG174	0x174
	#define REG175	0x175
	#define REG176	0x176
	#define REG177	0x177
	#define REG178	0x178
	#define REG179	0x179
	#define REG17A	0x17A
	#define REG17B	0x17B
	#define REG17C	0x17C
	#define REG17D	0x17D
	#define REG17E	0x17E
	#define REG17F	0x17F

	#define REG180	0x180
	#define REG181	0x181
	#define REG182	0x182
	#define REG183	0x183
	#define REG184	0x184
	#define REG185	0x185
	#define REG186	0x186
	#define REG187	0x187
	#define REG188	0x188
	#define REG189	0x189
	#define REG18A	0x18A
	#define REG18B	0x18B
	#define REG18C	0x18C
	#define REG18D	0x18D
	#define REG18E	0x18E
	#define REG18F	0x18F

	#define REG190	0x190
	#define REG191	0x191
	#define REG192	0x192
	#define REG193	0x193
	#define REG194	0x194
	#define REG195	0x195
	#define REG196	0x196
	#define REG197	0x197
	#define REG198	0x198
	#define REG199	0x199
	#define REG19A	0x19A
	#define REG19B	0x19B
	#define REG19C	0x19C
	#define REG19D	0x19D
	#define REG19E	0x19E
	#define REG19F	0x19F

	#define REG1A0	0x1A0
	#define REG1A1	0x1A1
	#define REG1A2	0x1A2
	#define REG1A3	0x1A3
	#define REG1A4	0x1A4
	#define REG1A5	0x1A5
	#define REG1A6	0x1A6
	#define REG1A7	0x1A7
	#define REG1A8	0x1A8
	#define REG1A9	0x1A9
	#define REG1AA	0x1AA
	#define REG1AB	0x1AB
	#define REG1AC	0x1AC
	#define REG1AD	0x1AD
	#define REG1AE	0x1AE
	#define REG1AF	0x1AF

	#define REG1B0	0x1B0
	#define REG1B1	0x1B1
	#define REG1B2	0x1B2
	#define REG1B3	0x1B3
	#define REG1B4	0x1B4
	#define REG1B5	0x1B5
	#define REG1B6	0x1B6
	#define REG1B7	0x1B7
	#define REG1B8	0x1B8
	#define REG1B9	0x1B9
	#define REG1BA	0x1BA
	#define REG1BB	0x1BB
	#define REG1BC	0x1BC
	#define REG1BD	0x1BD
	#define REG1BE	0x1BE
	#define REG1BF	0x1BF

	#define REG1C0	0x1C0
	#define REG1C1	0x1C1
	#define REG1C2	0x1C2
	#define REG1C3	0x1C3
	#define REG1C4	0x1C4
	#define REG1C5	0x1C5
	#define REG1C6	0x1C6
	#define REG1C7	0x1C7
	#define REG1C8	0x1C8
	#define REG1C9	0x1C9
	#define REG1CA	0x1CA
	#define REG1CB	0x1CB
	#define REG1CC	0x1CC
	#define REG1CD	0x1CD
	#define REG1CE	0x1CE
	#define REG1CF	0x1CF

	#define REG1D0	0x1D0
	#define REG1D1	0x1D1
	#define REG1D2	0x1D2
	#define REG1D3	0x1D3
	#define REG1D4	0x1D4
	#define REG1D5	0x1D5
	#define REG1D6	0x1D6
	#define REG1D7	0x1D7
	#define REG1D8	0x1D8
	#define REG1D9	0x1D9
	#define REG1DA	0x1DA
	#define REG1DB	0x1DB
	#define REG1DC	0x1DC
	#define REG1DD	0x1DD
	#define REG1DE	0x1DE
	#define REG1DF	0x1DF

	#define REG1E0	0x1E0
	#define REG1E1	0x1E1
	#define REG1E2	0x1E2
	#define REG1E3	0x1E3
	#define REG1E4	0x1E4
	#define REG1E5	0x1E5
	#define REG1E6	0x1E6
	#define REG1E7	0x1E7
	#define REG1E8	0x1E8
	#define REG1E9	0x1E9
	#define REG1EA	0x1EA
	#define REG1EB	0x1EB
	#define REG1EC	0x1EC
	#define REG1ED	0x1ED
	#define REG1EE	0x1EE
	#define REG1EF	0x1EF

	#define REG1F0	0x1F0
	#define REG1F1	0x1F1
	#define REG1F2	0x1F2
	#define REG1F3	0x1F3
	#define REG1F4	0x1F4
	#define REG1F5	0x1F5
	#define REG1F6	0x1F6
	#define REG1F7	0x1F7
	#define REG1F8	0x1F8
	#define REG1F9	0x1F9
	#define REG1FA	0x1FA
	#define REG1FB	0x1FB
	#define REG1FC	0x1FC
	#define REG1FD	0x1FD
	#define REG1FE	0x1FE
	//#define REG1FF	0x1FF
//===========================
// PAGE 2
//===========================
	#define REG200	0x200
	#define REG201	0x201
	#define REG202	0x202
	#define REG203	0x203
	#define REG204	0x204
	#define REG205	0x205
	#define REG206	0x206
	#define REG207	0x207
	#define REG208	0x208
	#define REG209	0x209
	#define REG20A	0x20A
	#define REG20B	0x20B
	#define REG20C	0x20C
	#define REG20D	0x20D
	#define REG20E	0x20E
	#define REG20F	0x20F

	#define REG210	0x210
	#define REG211	0x211
	#define REG212	0x212
	#define REG213	0x213
	#define REG214	0x214
	#define REG215	0x215
	#define REG216	0x216
	#define REG217	0x217
	#define REG218	0x218
	#define REG219	0x219
	#define REG21A	0x21A
	#define REG21B	0x21B
	#define REG21C	0x21C
	#define REG21D	0x21D
	#define REG21E	0x21E
	#define REG21F	0x21F

	#define REG220	0x220
	#define REG221	0x221
	#define REG222	0x222
	#define REG223	0x223
	#define REG224	0x224
	#define REG225	0x225
	#define REG226	0x226
	#define REG227	0x227
	#define REG228	0x228
	#define REG229	0x229
	#define REG22A	0x22A
	#define REG22B	0x22B
	#define REG22C	0x22C
	#define REG22D	0x22D
	#define REG22E	0x22E
	#define REG22F	0x22F

	#define REG230	0x230
	#define REG231	0x231
	#define REG232	0x232
	#define REG233	0x233
	#define REG234	0x234
	#define REG235	0x235
	#define REG236	0x236
	#define REG237	0x237
	#define REG238	0x238
	#define REG239	0x239
	#define REG23A	0x23A
	#define REG23B	0x23B
	#define REG23C	0x23C
	#define REG23D	0x23D
	#define REG23E	0x23E
	#define REG23F	0x23F

	#define REG240	0x240
	#define REG241	0x241
	#define REG242	0x242
	#define REG243	0x243
	#define REG244	0x244
	#define REG245	0x245
	#define REG246	0x246
	#define REG247	0x247
	#define REG248	0x248
	#define REG249	0x249
	#define REG24A	0x24A
	#define REG24B	0x24B
	#define REG24C	0x24C
	#define REG24D	0x24D
	#define REG24E	0x24E
	#define REG24F	0x24F

	#define REG250	0x250
	#define REG251	0x251
	#define REG252	0x252
	#define REG253	0x253
	#define REG254	0x254
	#define REG255	0x255
	#define REG256	0x256
	#define REG257	0x257
	#define REG258	0x258
	#define REG259	0x259
	#define REG25A	0x25A
	#define REG25B	0x25B
	#define REG25C	0x25C
	#define REG25D	0x25D
	#define REG25E	0x25E
	#define REG25F	0x25F

	#define REG260	0x260
	#define REG261	0x261
	#define REG262	0x262
	#define REG263	0x263
	#define REG264	0x264
	#define REG265	0x265
	#define REG266	0x266
	#define REG267	0x267
	#define REG268	0x268
	#define REG269	0x269
	#define REG26A	0x26A
	#define REG26B	0x26B
	#define REG26C	0x26C
	#define REG26D	0x26D
	#define REG26E	0x26E
	#define REG26F	0x26F

	#define REG270	0x270
	#define REG271	0x271
	#define REG272	0x272
	#define REG273	0x273
	#define REG274	0x274
	#define REG275	0x275
	#define REG276	0x276
	#define REG277	0x277
	#define REG278	0x278
	#define REG279	0x279
	#define REG27A	0x27A
	#define REG27B	0x27B
	#define REG27C	0x27C
	#define REG27D	0x27D
	#define REG27E	0x27E
	#define REG27F	0x27F

	#define REG280	0x280
	#define REG281	0x281
	#define REG282	0x282
	#define REG283	0x283
	#define REG284	0x284
	#define REG285	0x285
	#define REG286	0x286
	#define REG287	0x287
	#define REG288	0x288
	#define REG289	0x289
	#define REG28A	0x28A
	#define REG28B	0x28B
	#define REG28C	0x28C
	#define REG28D	0x28D
	#define REG28E	0x28E
	#define REG28F	0x28F

	#define REG290	0x290
	#define REG291	0x291
	#define REG292	0x292
	#define REG293	0x293
	#define REG294	0x294
	#define REG295	0x295
	#define REG296	0x296
	#define REG297	0x297
	#define REG298	0x298
	#define REG299	0x299
	#define REG29A	0x29A
	#define REG29B	0x29B
	#define REG29C	0x29C
	#define REG29D	0x29D
	#define REG29E	0x29E
	#define REG29F	0x29F

	#define REG2A0	0x2A0
	#define REG2A1	0x2A1
	#define REG2A2	0x2A2
	#define REG2A3	0x2A3
	#define REG2A4	0x2A4
	#define REG2A5	0x2A5
	#define REG2A6	0x2A6
	#define REG2A7	0x2A7
	#define REG2A8	0x2A8
	#define REG2A9	0x2A9
	#define REG2AA	0x2AA
	#define REG2AB	0x2AB
	#define REG2AC	0x2AC
	#define REG2AD	0x2AD
	#define REG2AE	0x2AE
	#define REG2AF	0x2AF

	#define REG2B0	0x2B0
	#define REG2B1	0x2B1
	#define REG2B2	0x2B2
	#define REG2B3	0x2B3
	#define REG2B4	0x2B4
	#define REG2B5	0x2B5
	#define REG2B6	0x2B6
	#define REG2B7	0x2B7
	#define REG2B8	0x2B8
	#define REG2B9	0x2B9
	#define REG2BA	0x2BA
	#define REG2BB	0x2BB
	#define REG2BC	0x2BC
	#define REG2BD	0x2BD
	#define REG2BE	0x2BE
	#define REG2BF	0x2BF

	#define REG2C0	0x2C0
	#define REG2C1	0x2C1
	#define REG2C2	0x2C2
	#define REG2C3	0x2C3
	#define REG2C4	0x2C4
	#define REG2C5	0x2C5
	#define REG2C6	0x2C6
	#define REG2C7	0x2C7
	#define REG2C8	0x2C8
	#define REG2C9	0x2C9
	#define REG2CA	0x2CA
	#define REG2CB	0x2CB
	#define REG2CC	0x2CC
	#define REG2CD	0x2CD
	#define REG2CE	0x2CE
	#define REG2CF	0x2CF

	#define REG2D0	0x2D0
	#define REG2D1	0x2D1
	#define REG2D2	0x2D2
	#define REG2D3	0x2D3
	#define REG2D4	0x2D4
	#define REG2D5	0x2D5
	#define REG2D6	0x2D6
	#define REG2D7	0x2D7
	#define REG2D8	0x2D8
	#define REG2D9	0x2D9
	#define REG2DA	0x2DA
	#define REG2DB	0x2DB
	#define REG2DC	0x2DC
	#define REG2DD	0x2DD
	#define REG2DE	0x2DE
	#define REG2DF	0x2DF

	#define REG2E0	0x2E0
	#define REG2E1	0x2E1
	#define REG2E2	0x2E2
	#define REG2E3	0x2E3
	#define REG2E4	0x2E4
	#define REG2E5	0x2E5
	#define REG2E6	0x2E6
	#define REG2E7	0x2E7
	#define REG2E8	0x2E8
	#define REG2E9	0x2E9
	#define REG2EA	0x2EA
	#define REG2EB	0x2EB
	#define REG2EC	0x2EC
	#define REG2ED	0x2ED
	#define REG2EE	0x2EE
	#define REG2EF	0x2EF

	#define REG2F0	0x2F0
	#define REG2F1	0x2F1
	#define REG2F2	0x2F2
	#define REG2F3	0x2F3
	#define REG2F4	0x2F4
	#define REG2F5	0x2F5
	#define REG2F6	0x2F6
	#define REG2F7	0x2F7
	#define REG2F8	0x2F8
	#define REG2F9	0x2F9
	#define REG2FA	0x2FA
	#define REG2FB	0x2FB
	#define REG2FC	0x2FC
	#define REG2FD	0x2FD
	#define REG2FE	0x2FE
	//#define REG2FF	0x2FF


//===========================
// PAGE 3
//===========================
	#define REG300	0x300
	#define REG301	0x301
	#define REG302	0x302
	#define REG303	0x303
	#define REG304	0x304
	#define REG305	0x305
	#define REG306	0x306
	#define REG307	0x307
	#define REG308	0x308
	#define REG309	0x309
	#define REG30A	0x30A
	#define REG30B	0x30B
	#define REG30C	0x30C
	#define REG30D	0x30D
	#define REG30E	0x30E
	#define REG30F	0x30F

	#define REG310	0x310
	#define REG311	0x311
	#define REG312	0x312
	#define REG313	0x313
	#define REG314	0x314
	#define REG315	0x315
	#define REG316	0x316
	#define REG317	0x317
	#define REG318	0x318
	#define REG319	0x319
	#define REG31A	0x31A
	#define REG31B	0x31B
	#define REG31C	0x31C
	#define REG31D	0x31D
	#define REG31E	0x31E
	#define REG31F	0x31F

	#define REG320	0x320
	#define REG321	0x321
	#define REG322	0x322
	#define REG323	0x323
	#define REG324	0x324
	#define REG325	0x325
	#define REG326	0x326
	#define REG327	0x327
	#define REG328	0x328
	#define REG329	0x329
	#define REG32A	0x32A
	#define REG32B	0x32B
	#define REG32C	0x32C
	#define REG32D	0x32D
	#define REG32E	0x32E
	#define REG32F	0x32F

	#define REG330	0x330
	#define REG331	0x331
	#define REG332	0x332
	#define REG333	0x333
	#define REG334	0x334
	#define REG335	0x335
	#define REG336	0x336
	#define REG337	0x337
	#define REG338	0x338
	#define REG339	0x339
	#define REG33A	0x33A
	#define REG33B	0x33B
	#define REG33C	0x33C
	#define REG33D	0x33D
	#define REG33E	0x33E
	#define REG33F	0x33F

	#define REG340	0x340
	#define REG341	0x341
	#define REG342	0x342
	#define REG343	0x343
	#define REG344	0x344
	#define REG345	0x345
	#define REG346	0x346
	#define REG347	0x347
	#define REG348	0x348
	#define REG349	0x349
	#define REG34A	0x34A
	#define REG34B	0x34B
	#define REG34C	0x34C
	#define REG34D	0x34D
	#define REG34E	0x34E
	#define REG34F	0x34F

	#define REG350	0x350
	#define REG351	0x351
	#define REG352	0x352
	#define REG353	0x353
	#define REG354	0x354
	#define REG355	0x355
	#define REG356	0x356
	#define REG357	0x357
	#define REG358	0x358
	#define REG359	0x359
	#define REG35A	0x35A
	#define REG35B	0x35B
	#define REG35C	0x35C
	#define REG35D	0x35D
	#define REG35E	0x35E
	#define REG35F	0x35F

	#define REG360	0x360
	#define REG361	0x361
	#define REG362	0x362
	#define REG363	0x363
	#define REG364	0x364
	#define REG365	0x365
	#define REG366	0x366
	#define REG367	0x367
	#define REG368	0x368
	#define REG369	0x369
	#define REG36A	0x36A
	#define REG36B	0x36B
	#define REG36C	0x36C
	#define REG36D	0x36D
	#define REG36E	0x36E
	#define REG36F	0x36F

	#define REG370	0x370
	#define REG371	0x371
	#define REG372	0x372
	#define REG373	0x373
	#define REG374	0x374
	#define REG375	0x375
	#define REG376	0x376
	#define REG377	0x377
	#define REG378	0x378
	#define REG379	0x379
	#define REG37A	0x37A
	#define REG37B	0x37B
	#define REG37C	0x37C
	#define REG37D	0x37D
	#define REG37E	0x37E
	#define REG37F	0x37F

	#define REG380	0x380
	#define REG381	0x381
	#define REG382	0x382
	#define REG383	0x383
	#define REG384	0x384
	#define REG385	0x385
	#define REG386	0x386
	#define REG387	0x387
	#define REG388	0x388
	#define REG389	0x389
	#define REG38A	0x38A
	#define REG38B	0x38B
	#define REG38C	0x38C
	#define REG38D	0x38D
	#define REG38E	0x38E
	#define REG38F	0x38F

	#define REG390	0x390
	#define REG391	0x391
	#define REG392	0x392
	#define REG393	0x393
	#define REG394	0x394
	#define REG395	0x395
	#define REG396	0x396
	#define REG397	0x397
	#define REG398	0x398
	#define REG399	0x399
	#define REG39A	0x39A
	#define REG39B	0x39B
	#define REG39C	0x39C
	#define REG39D	0x39D
	#define REG39E	0x39E
	#define REG39F	0x39F

	#define REG3A0	0x3A0
	#define REG3A1	0x3A1
	#define REG3A2	0x3A2
	#define REG3A3	0x3A3
	#define REG3A4	0x3A4
	#define REG3A5	0x3A5
	#define REG3A6	0x3A6
	#define REG3A7	0x3A7
	#define REG3A8	0x3A8
	#define REG3A9	0x3A9
	#define REG3AA	0x3AA
	#define REG3AB	0x3AB
	#define REG3AC	0x3AC
	#define REG3AD	0x3AD
	#define REG3AE	0x3AE
	#define REG3AF	0x3AF

	#define REG3B0	0x3B0
	#define REG3B1	0x3B1
	#define REG3B2	0x3B2
	#define REG3B3	0x3B3
	#define REG3B4	0x3B4
	#define REG3B5	0x3B5
	#define REG3B6	0x3B6
	#define REG3B7	0x3B7
	#define REG3B8	0x3B8
	#define REG3B9	0x3B9
	#define REG3BA	0x3BA
	#define REG3BB	0x3BB
	#define REG3BC	0x3BC
	#define REG3BD	0x3BD
	#define REG3BE	0x3BE
	#define REG3BF	0x3BF

	#define REG3C0	0x3C0
	#define REG3C1	0x3C1
	#define REG3C2	0x3C2
	#define REG3C3	0x3C3
	#define REG3C4	0x3C4
	#define REG3C5	0x3C5
	#define REG3C6	0x3C6
	#define REG3C7	0x3C7
	#define REG3C8	0x3C8
	#define REG3C9	0x3C9
	#define REG3CA	0x3CA
	#define REG3CB	0x3CB
	#define REG3CC	0x3CC
	#define REG3CD	0x3CD
	#define REG3CE	0x3CE
	#define REG3CF	0x3CF

	#define REG3D0	0x3D0
	#define REG3D1	0x3D1
	#define REG3D2	0x3D2
	#define REG3D3	0x3D3
	#define REG3D4	0x3D4
	#define REG3D5	0x3D5
	#define REG3D6	0x3D6
	#define REG3D7	0x3D7
	#define REG3D8	0x3D8
	#define REG3D9	0x3D9
	#define REG3DA	0x3DA
	#define REG3DB	0x3DB
	#define REG3DC	0x3DC
	#define REG3DD	0x3DD
	#define REG3DE	0x3DE
	#define REG3DF	0x3DF

	#define REG3E0	0x3E0
	#define REG3E1	0x3E1
	#define REG3E2	0x3E2
	#define REG3E3	0x3E3
	#define REG3E4	0x3E4
	#define REG3E5	0x3E5
	#define REG3E6	0x3E6
	#define REG3E7	0x3E7
	#define REG3E8	0x3E8
	#define REG3E9	0x3E9
	#define REG3EA	0x3EA
	#define REG3EB	0x3EB
	#define REG3EC	0x3EC
	#define REG3ED	0x3ED
	#define REG3EE	0x3EE
	#define REG3EF	0x3EF

	#define REG3F0	0x3F0
	#define REG3F1	0x3F1
	#define REG3F2	0x3F2
	#define REG3F3	0x3F3
	#define REG3F4	0x3F4
	#define REG3F5	0x3F5
	#define REG3F6	0x3F6
	#define REG3F7	0x3F7
	#define REG3F8	0x3F8
	#define REG3F9	0x3F9
	#define REG3FA	0x3FA
	#define REG3FB	0x3FB
	#define REG3FC	0x3FC
	#define REG3FD	0x3FD
	#define REG3FE	0x3FE
	//#define REG3FF	0x3FF

//===========================
// PAGE 4
//===========================
	#define REG400	0x400
	#define REG401	0x401
	#define REG402	0x402
	#define REG403	0x403
	#define REG404	0x404
	#define REG405	0x405
	#define REG406	0x406
	#define REG407	0x407
	#define REG408	0x408
	#define REG409	0x409
	#define REG40A	0x40A
	#define REG40B	0x40B
	#define REG40C	0x40C
	#define REG40D	0x40D
	#define REG40E	0x40E
	#define REG40F	0x40F

	#define REG410	0x410
	#define REG411	0x411
	#define REG412	0x412
	#define REG413	0x413
	#define REG414	0x414
	#define REG415	0x415
	#define REG416	0x416
	#define REG417	0x417
	#define REG418	0x418
	#define REG419	0x419
	#define REG41A	0x41A
	#define REG41B	0x41B
	#define REG41C	0x41C
	#define REG41D	0x41D
	#define REG41E	0x41E
	#define REG41F	0x41F

	#define REG420	0x420
	#define REG421	0x421
	#define REG422	0x422
	#define REG423	0x423
	#define REG424	0x424
	#define REG425	0x425
	#define REG426	0x426
	#define REG427	0x427
	#define REG428	0x428
	#define REG429	0x429
	#define REG42A	0x42A
	#define REG42B	0x42B
	#define REG42C	0x42C
	#define REG42D	0x42D
	#define REG42E	0x42E
	#define REG42F	0x42F

	#define REG430	0x430
	#define REG431	0x431
	#define REG432	0x432
	#define REG433	0x433
	#define REG434	0x434
	#define REG435	0x435
	#define REG436	0x436
	#define REG437	0x437
	#define REG438	0x438
	#define REG439	0x439
	#define REG43A	0x43A
	#define REG43B	0x43B
	#define REG43C	0x43C
	#define REG43D	0x43D
	#define REG43E	0x43E
	#define REG43F	0x43F

	#define REG440	0x440
	#define REG441	0x441
	#define REG442	0x442
	#define REG443	0x443
	#define REG444	0x444
	#define REG445	0x445
	#define REG446	0x446
	#define REG447	0x447
	#define REG448	0x448
	#define REG449	0x449
	#define REG44A	0x44A
	#define REG44B	0x44B
	#define REG44C	0x44C
	#define REG44D	0x44D
	#define REG44E	0x44E
	#define REG44F	0x44F

	#define REG450	0x450
	#define REG451	0x451
	#define REG452	0x452
	#define REG453	0x453
	#define REG454	0x454
	#define REG455	0x455
	#define REG456	0x456
	#define REG457	0x457
	#define REG458	0x458
	#define REG459	0x459
	#define REG45A	0x45A
	#define REG45B	0x45B
	#define REG45C	0x45C
	#define REG45D	0x45D
	#define REG45E	0x45E
	#define REG45F	0x45F

	#define REG460	0x460
	#define REG461	0x461
	#define REG462	0x462
	#define REG463	0x463
	#define REG464	0x464
	#define REG465	0x465
	#define REG466	0x466
	#define REG467	0x467
	#define REG468	0x468
	#define REG469	0x469
	#define REG46A	0x46A
	#define REG46B	0x46B
	#define REG46C	0x46C
	#define REG46D	0x46D
	#define REG46E	0x46E
	#define REG46F	0x46F

	#define REG470	0x470
	#define REG471	0x471
	#define REG472	0x472
	#define REG473	0x473
	#define REG474	0x474
	#define REG475	0x475
	#define REG476	0x476
	#define REG477	0x477
	#define REG478	0x478
	#define REG479	0x479
	#define REG47A	0x47A
	#define REG47B	0x47B
	#define REG47C	0x47C
	#define REG47D	0x47D
	#define REG47E	0x47E
	#define REG47F	0x47F

	#define REG480	0x480
	#define REG481	0x481
	#define REG482	0x482
	#define REG483	0x483
	#define REG484	0x484
	#define REG485	0x485
	#define REG486	0x486
	#define REG487	0x487
	#define REG488	0x488
	#define REG489	0x489
	#define REG48A	0x48A
	#define REG48B	0x48B
	#define REG48C	0x48C
	#define REG48D	0x48D
	#define REG48E	0x48E
	#define REG48F	0x48F

	#define REG490	0x490
	#define REG491	0x491
	#define REG492	0x492
	#define REG493	0x493
	#define REG494	0x494
	#define REG495	0x495
	#define REG496	0x496
	#define REG497	0x497
	#define REG498	0x498
	#define REG499	0x499
	#define REG49A	0x49A
	#define REG49B	0x49B
	#define REG49C	0x49C
	#define REG49D	0x49D
	#define REG49E	0x49E
	#define REG49F	0x49F

	#define REG4A0	0x4A0
	#define REG4A1	0x4A1
	#define REG4A2	0x4A2
	#define REG4A3	0x4A3
	#define REG4A4	0x4A4
	#define REG4A5	0x4A5
	#define REG4A6	0x4A6
	#define REG4A7	0x4A7
	#define REG4A8	0x4A8
	#define REG4A9	0x4A9
	#define REG4AA	0x4AA
	#define REG4AB	0x4AB
	#define REG4AC	0x4AC
	#define REG4AD	0x4AD
	#define REG4AE	0x4AE
	#define REG4AF	0x4AF

	#define REG4B0	0x4B0
	#define REG4B1	0x4B1
	#define REG4B2	0x4B2
	#define REG4B3	0x4B3
	#define REG4B4	0x4B4
	#define REG4B5	0x4B5
	#define REG4B6	0x4B6
	#define REG4B7	0x4B7
	#define REG4B8	0x4B8
	#define REG4B9	0x4B9
	#define REG4BA	0x4BA
	#define REG4BB	0x4BB
	#define REG4BC	0x4BC
	#define REG4BD	0x4BD
	#define REG4BE	0x4BE
	#define REG4BF	0x4BF

	#define REG4C0	0x4C0
	#define REG4C1	0x4C1
	#define REG4C2	0x4C2
	#define REG4C3	0x4C3
	#define REG4C4	0x4C4
	#define REG4C5	0x4C5
	#define REG4C6	0x4C6
	#define REG4C7	0x4C7
	#define REG4C8	0x4C8
	#define REG4C9	0x4C9
	#define REG4CA	0x4CA
	#define REG4CB	0x4CB
	#define REG4CC	0x4CC
	#define REG4CD	0x4CD
	#define REG4CE	0x4CE
	#define REG4CF	0x4CF

	#define REG4D0	0x4D0
	#define REG4D1	0x4D1
	#define REG4D2	0x4D2
	#define REG4D3	0x4D3
	#define REG4D4	0x4D4
	#define REG4D5	0x4D5
	#define REG4D6	0x4D6
	#define REG4D7	0x4D7
	#define REG4D8	0x4D8
	#define REG4D9	0x4D9
	#define REG4DA	0x4DA
	#define REG4DB	0x4DB
	#define REG4DC	0x4DC
	#define REG4DD	0x4DD
	#define REG4DE	0x4DE
	#define REG4DF	0x4DF

	#define REG4E0	0x4E0
	#define REG4E1	0x4E1
	#define REG4E2	0x4E2
	#define REG4E3	0x4E3
	#define REG4E4	0x4E4
	#define REG4E5	0x4E5
	#define REG4E6	0x4E6
	#define REG4E7	0x4E7
	#define REG4E8	0x4E8
	#define REG4E9	0x4E9
	#define REG4EA	0x4EA
	#define REG4EB	0x4EB
	#define REG4EC	0x4EC
	#define REG4ED	0x4ED
	#define REG4EE	0x4EE
	#define REG4EF	0x4EF

	#define REG4F0	0x4F0
	#define REG4F1	0x4F1
	#define REG4F2	0x4F2
	#define REG4F3	0x4F3
	#define REG4F4	0x4F4
	#define REG4F5	0x4F5
	#define REG4F6	0x4F6
	#define REG4F7	0x4F7
	#define REG4F8	0x4F8
	#define REG4F9	0x4F9
	#define REG4FA	0x4FA
	#define REG4FB	0x4FB
	#define REG4FC	0x4FC
	#define REG4FD	0x4FD
	#define REG4FE	0x4FE
	//#define REG4FF	0x4FF


//===========================
// PAGE 5
//===========================
	#define REG500	0x500
	#define REG501	0x501
	#define REG502	0x502
	#define REG503	0x503
	#define REG504	0x504
	#define REG505	0x505
	#define REG506	0x506
	#define REG507	0x507
	#define REG508	0x508
	#define REG509	0x509
	#define REG50A	0x50A
	#define REG50B	0x50B
	#define REG50C	0x50C
	#define REG50D	0x50D
	#define REG50E	0x50E
	#define REG50F	0x50F

	#define REG510	0x510
	#define REG511	0x511
	#define REG512	0x512
	#define REG513	0x513
	#define REG514	0x514
	#define REG515	0x515
	#define REG516	0x516
	#define REG517	0x517
	#define REG518	0x518
	#define REG519	0x519
	#define REG51A	0x51A
	#define REG51B	0x51B
	#define REG51C	0x51C
	#define REG51D	0x51D
	#define REG51E	0x51E
	#define REG51F	0x51F

	#define REG520	0x520
	#define REG521	0x521
	#define REG522	0x522
	#define REG523	0x523
	#define REG524	0x524
	#define REG525	0x525
	#define REG526	0x526
	#define REG527	0x527
	#define REG528	0x528
	#define REG529	0x529
	#define REG52A	0x52A
	#define REG52B	0x52B
	#define REG52C	0x52C
	#define REG52D	0x52D
	#define REG52E	0x52E
	#define REG52F	0x52F

	#define REG530	0x530
	#define REG531	0x531
	#define REG532	0x532
	#define REG533	0x533
	#define REG534	0x534
	#define REG535	0x535
	#define REG536	0x536
	#define REG537	0x537
	#define REG538	0x538
	#define REG539	0x539
	#define REG53A	0x53A
	#define REG53B	0x53B
	#define REG53C	0x53C
	#define REG53D	0x53D
	#define REG53E	0x53E
	#define REG53F	0x53F

	#define REG540	0x540
	#define REG541	0x541
	#define REG542	0x542
	#define REG543	0x543
	#define REG544	0x544
	#define REG545	0x545
	#define REG546	0x546
	#define REG547	0x547
	#define REG548	0x548
	#define REG549	0x549
	#define REG54A	0x54A
	#define REG54B	0x54B
	#define REG54C	0x54C
	#define REG54D	0x54D
	#define REG54E	0x54E
	#define REG54F	0x54F

	#define REG550	0x550
	#define REG551	0x551
	#define REG552	0x552
	#define REG553	0x553
	#define REG554	0x554
	#define REG555	0x555
	#define REG556	0x556
	#define REG557	0x557
	#define REG558	0x558
	#define REG559	0x559
	#define REG55A	0x55A
	#define REG55B	0x55B
	#define REG55C	0x55C
	#define REG55D	0x55D
	#define REG55E	0x55E
	#define REG55F	0x55F

	#define REG560	0x560
	#define REG561	0x561
	#define REG562	0x562
	#define REG563	0x563
	#define REG564	0x564
	#define REG565	0x565
	#define REG566	0x566
	#define REG567	0x567
	#define REG568	0x568
	#define REG569	0x569
	#define REG56A	0x56A
	#define REG56B	0x56B
	#define REG56C	0x56C
	#define REG56D	0x56D
	#define REG56E	0x56E
	#define REG56F	0x56F

	#define REG570	0x570
	#define REG571	0x571
	#define REG572	0x572
	#define REG573	0x573
	#define REG574	0x574
	#define REG575	0x575
	#define REG576	0x576
	#define REG577	0x577
	#define REG578	0x578
	#define REG579	0x579
	#define REG57A	0x57A
	#define REG57B	0x57B
	#define REG57C	0x57C
	#define REG57D	0x57D
	#define REG57E	0x57E
	#define REG57F	0x57F

	#define REG580	0x580
	#define REG581	0x581
	#define REG582	0x582
	#define REG583	0x583
	#define REG584	0x584
	#define REG585	0x585
	#define REG586	0x586
	#define REG587	0x587
	#define REG588	0x588
	#define REG589	0x589
	#define REG58A	0x58A
	#define REG58B	0x58B
	#define REG58C	0x58C
	#define REG58D	0x58D
	#define REG58E	0x58E
	#define REG58F	0x58F

	#define REG590	0x590
	#define REG591	0x591
	#define REG592	0x592
	#define REG593	0x593
	#define REG594	0x594
	#define REG595	0x595
	#define REG596	0x596
	#define REG597	0x597
	#define REG598	0x598
	#define REG599	0x599
	#define REG59A	0x59A
	#define REG59B	0x59B
	#define REG59C	0x59C
	#define REG59D	0x59D
	#define REG59E	0x59E
	#define REG59F	0x59F

	#define REG5A0	0x5A0
	#define REG5A1	0x5A1
	#define REG5A2	0x5A2
	#define REG5A3	0x5A3
	#define REG5A4	0x5A4
	#define REG5A5	0x5A5
	#define REG5A6	0x5A6
	#define REG5A7	0x5A7
	#define REG5A8	0x5A8
	#define REG5A9	0x5A9
	#define REG5AA	0x5AA
	#define REG5AB	0x5AB
	#define REG5AC	0x5AC
	#define REG5AD	0x5AD
	#define REG5AE	0x5AE
	#define REG5AF	0x5AF

	#define REG5B0	0x5B0
	#define REG5B1	0x5B1
	#define REG5B2	0x5B2
	#define REG5B3	0x5B3
	#define REG5B4	0x5B4
	#define REG5B5	0x5B5
	#define REG5B6	0x5B6
	#define REG5B7	0x5B7
	#define REG5B8	0x5B8
	#define REG5B9	0x5B9
	#define REG5BA	0x5BA
	#define REG5BB	0x5BB
	#define REG5BC	0x5BC
	#define REG5BD	0x5BD
	#define REG5BE	0x5BE
	#define REG5BF	0x5BF

	#define REG5C0	0x5C0
	#define REG5C1	0x5C1
	#define REG5C2	0x5C2
	#define REG5C3	0x5C3
	#define REG5C4	0x5C4
	#define REG5C5	0x5C5
	#define REG5C6	0x5C6
	#define REG5C7	0x5C7
	#define REG5C8	0x5C8
	#define REG5C9	0x5C9
	#define REG5CA	0x5CA
	#define REG5CB	0x5CB
	#define REG5CC	0x5CC
	#define REG5CD	0x5CD
	#define REG5CE	0x5CE
	#define REG5CF	0x5CF

	#define REG5D0	0x5D0
	#define REG5D1	0x5D1
	#define REG5D2	0x5D2
	#define REG5D3	0x5D3
	#define REG5D4	0x5D4
	#define REG5D5	0x5D5
	#define REG5D6	0x5D6
	#define REG5D7	0x5D7
	#define REG5D8	0x5D8
	#define REG5D9	0x5D9
	#define REG5DA	0x5DA
	#define REG5DB	0x5DB
	#define REG5DC	0x5DC
	#define REG5DD	0x5DD
	#define REG5DE	0x5DE
	#define REG5DF	0x5DF

	#define REG5E0	0x5E0
	#define REG5E1	0x5E1
	#define REG5E2	0x5E2
	#define REG5E3	0x5E3
	#define REG5E4	0x5E4
	#define REG5E5	0x5E5
	#define REG5E6	0x5E6
	#define REG5E7	0x5E7
	#define REG5E8	0x5E8
	#define REG5E9	0x5E9
	#define REG5EA	0x5EA
	#define REG5EB	0x5EB
	#define REG5EC	0x5EC
	#define REG5ED	0x5ED
	#define REG5EE	0x5EE
	#define REG5EF	0x5EF

	#define REG5F0	0x5F0
	#define REG5F1	0x5F1
	#define REG5F2	0x5F2
	#define REG5F3	0x5F3
	#define REG5F4	0x5F4
	#define REG5F5	0x5F5
	#define REG5F6	0x5F6
	#define REG5F7	0x5F7
	#define REG5F8	0x5F8
	#define REG5F9	0x5F9
	#define REG5FA	0x5FA
	#define REG5FB	0x5FB
	#define REG5FC	0x5FC
	#define REG5FD	0x5FD
	#define REG5FE	0x5FE
	//#define REG5FF	0x5FF
						 		//LVDS Tx
	#define REG640	0x640
	#define REG641	0x641
	#define REG642	0x642
								//LVDS Rx
	#define REG644	0x644
	#define REG645	0x645
	#define REG646	0x646
	#define REG647	0x647
	#define REG648	0x648
	#define REG649	0x649
	#define REG64A	0x64A
	#define REG64B	0x64B
	#define REG64C	0x64C
	#define REG64D	0x64D
	#define REG64E	0x64E




//============================================================
//
//============================================================
#define REG0F8_FPLL0					REG0F8
#define REG0F9_FPLL1					REG0F9
#define REG0FA_FPLL2					REG0FA
#define REG0FD_SSPLL_ANALOG				REG0FD

#define REG040_INPUT_CTRL_I             REG040
#define REG041_INPUT_CTRL_II            REG041

#define REG101_DEC_CSTATUS				REG101	// Chip Status
#define REG102_DEC_INFORM				REG102	// Input Format
#define REG106_DEC_ACNTL				REG106	// Analog Control
#define REG107_DEC_CROP_HI				REG107	// Cropping High
#define REG108_DEC_VDELAY_LO			REG108	// Vertical Delay Low
#define REG109_DEC_VACTIVE_LO			REG109	// Vertical Active Low
#define REG10A_DEC_HDELAY_LO			REG10A	// Horizontal Delay Low
#define REG10B_DEC_HACTIVE_LO			REG10B	// Horizontal Active Low
#define REG10C_DEC_CNTRL1				REG10C	// Control 1

#define REG110_DEC_BRIGHT				REG110	// Brightness Control
#define REG111_DEC_CONTRAST				REG111	// Contrast Control
#define REG112_DEC_SHARPNESS			REG112	// Sharpness Control
#define REG113_DEC_SAT_U				REG113	// Chroma(U) Gain
#define REG114_DEC_SAT_V				REG114	// Chroma(V) Gain
#define REG115_DEC_HUE					REG115	// Hue Conrol
#define REG117_DEC_V_PEAKING			REG117	// Vertical Peaking

#define REG11A_DEC_CC_STATUS			REG11a	// CC/EDS Status
#define REG11B_DEC_CC_DATA				REG11b	// CC/EDS Data
#define REG11C_DEC_SDT					REG11c	// Standard Selection
#define REG11D_DEC_SDTR					REG11d	// Standard Recognition

#define REG129_DEC_V_CONTROL2			REG129	// Vertical Control II
#define REG12C_DEC_HFILTER				REG12c
#define REG12D_DEC_MISC1				REG12d	// Miscellaneous Control 1
#define REG12E_DEC_MISC2				REG12e	// Miscellaneous Control 2
#define REG12F_DEC_MISC3				REG12f	// Miscellaneous Control 2

#define REG134_DEC_VBI_CNTL2			REG134	// VBI control 2 - WSSEN
#define REG135_DEC_CC_ODDLINE			REG135	// CC Odd Line


#define REG278_PNLSHARPNESS				REG278	// Sharpness Control  BUG...USe REG281

#define REG280_SHUE						REG280	// RGB Hue
#define REG281_SCONTRAST_R				REG281	// Red Contrast
#define REG282_SCONTRAST_G				REG282	// Green Contrast
#define REG283_SCONTRAST_B				REG283	// Blue Contrast
#define REG284_SCONTRAST_Y				REG284	// Y Contrast
#define REG285_SCONTRAST_Cb				REG285	// Cb Contrast
#define REG286_SCONTRAST_Cr				REG286	// Cr Contrast
#define REG287_SBRIGHT_R				REG287	// Red Brightness
#define REG288_SBRIGHT_G				REG288	// Green Brightness
#define REG289_SBRIGHT_B				REG289	// Blue Brightness
#define REG28A_SBRIGHT_Y				REG28A	// Y Brightness

#define REG280_IA_HUE_REG				REG280
#define REG281_IA_CONTRAST_R			REG281
#define REG282_IA_CONTRAST_G			REG282
#define REG283_IA_CONTRAST_B			REG283
#define REG284_IA_CONTRAST_Y			REG284
#define REG285_IA_CONTRAST_CB			REG285
#define REG286_IA_CONTRAST_CR			REG286
#define REG287_IA_BRIGHTNESS_R			REG287
#define REG288_IA_BRIGHTNESS_G			REG288
#define REG289_IA_BRIGHTNESS_B			REG289
#define REG28A_IA_BRIGHTNESS_Y			REG28A
#define REG28B_IA_SHARPNESS				REG28B

#define REG4C0_SPIBASE					REG4C0	//note:Not a page. it is a offset.

#define REG500_MEAS_HSTART_MSB      	REG500
#define REG501_MEAS_HSTART_LSB      	REG501
#define REG502_MEAS_HLEN_MSB        	REG502
#define REG503_MEAS_HLEN_LSB        	REG503
#define REG505_MEAS_VSTART_MSB      	REG504
#define REG505_MEAS_VSTART_LSB      	REG505
#define REG506_MEAS_VLEN_MSB        	REG506
#define REG507_MEAS_VLEN_LSB        	REG507

//===================================================================
// TW8835/TW8836 FOSD
//===================================================================

//----------------
// Font OSD
//----------------
//#ifdef MODEL_TW8835
//#define REG_FOSD_CHEIGHT			REG350
//#define REG_FOSD_MUL_CON			REG351
//#define REG_FOSD_ALPHA_SEL			REG352
//#define REG_FOSD_MADD3				REG353
//#define REG_FOSD_MADD4				REG354
//#else
#define REG_FOSD_CHEIGHT			REG390
#define REG_FOSD_MUL_CON			REG391
#define REG_FOSD_ALPHA_SEL			REG392
#define REG_FOSD_MADD3				REG393
#define REG_FOSD_MADD4				REG394
//#endif

//----------------
// I2CCMD buffer
//----------------
//#ifdef MODEL_TW8835
//#define I2CCMD_REG0					REG4DB
//#define I2CCMD_REG1					REG4DC
//#define I2CCMD_REG2					REG4DD
//#define I2CCMD_REG3					REG4DE
//#define I2CCMD_REG4					REG4DF
//#else
#define I2CCMD_REG0					REG4FA
#define I2CCMD_REG1					REG4FB
#define I2CCMD_REG2					REG4FC
#define I2CCMD_REG3					REG4FD
#define I2CCMD_REG4					REG4FE
//#endif
//===================================================================
// memory map register
//
// below two define is better, but have a problem on the high optimization option.
//	#define WriteTW88(a,b)	(*((unsigned char volatile xdata *) (REG_START_ADDRESS+a) )) = ((BYTE)(b))
//	#define	ReadTW88(a)		(*((unsigned char volatile xdata *) (REG_START_ADDRESS+a) ))
//===================================================================
extern volatile BYTE	XDATA *DATA regTW88;

#define WriteTW88(a, b)			regTW88[a] = ((BYTE)(b))
#define	ReadTW88(a)				regTW88[a]

//TW8836 uses only 16bit index. below is for backward compatibility.
#define DECLARE_LOCAL_page		
#define WriteTW88Page(p)    
#define ReadTW88Page(p)		

//wrong name. WriteTW88_8bit_index
#define WriteTW88Byte(a, b)		regTW88[(BYTE)(a)] = ((BYTE)(b))   
#define	ReadTW88Byte(a)			regTW88[(BYTE)(a)]
#define WriteTW88BytePage(p)	regTW88[(BYTE)0xff] = ((BYTE)(p))
#define ReadTW88BytePage(p)		p = ReadTW88(0xff)


//#if 0
//inline void TW88Reg16WriteMask(BYTE rhi, BYTE rlo, WORD value, BYTE rmask, BYTE vmask, BYTE vshift)
//{
//	WriteTW88(rhi, (ReadTW88(rhi) & ^rmask) | ((value & (vmask<<8)) >> vshift));
//	WriteTW88(rlo, (BYTE)value);
//}
//#else
/*
#define TW88Reg16WriteMask(rhi, rlo, value, rmask, vmask, vshift) \
	WriteTW88(rhi, (ReadTW88(rhi) & ^rmask) | ((value & (vmask<<8)) >> vshift)); \
	WriteTW88(rlo, (BYTE)value)
*/
//#endif
//#define TW88Reg16Write(rhi,rlo,value) WriteTW88(rhi, (value >> 8)); WriteTW88(rlo, (BYTE)value)
#define Write2TW88(rhi,rlo,value) WriteTW88(rhi, (value >> 8)); WriteTW88(rlo, (BYTE)value)
#define Read2TW88(rhi,rlo,value) value=ReadTW88(rhi); value <<= 8; value |= ReadTW88(rlo)
#define Read3TW88(rhi,rmid,rlo,value) value=ReadTW88(rhi); value <<= 8; value |= ReadTW88(rmid); value <<= 8; value |= ReadTW88(rlo)
#define Read4TW88(r3,r2,r1,r0,value) value=ReadTW88(r3); value <<= 8; value |= ReadTW88(r2); value <<= 8; value |= ReadTW88(r1); value <<= 8; value |= ReadTW88(r0)

void WriteBlockTW88(WORD index, BYTE *val, BYTE cnt);

/* status register */
#define TW8835_R002	(*((unsigned char volatile xdata *) (REG_START_ADDRESS+0x002) ))
#define TW8835_R101	(*((unsigned char volatile xdata *) (REG_START_ADDRESS+0x101) ))
#define TW8835_R11C	(*((unsigned char volatile xdata *) (REG_START_ADDRESS+0x11C) ))
#define TW8835_R130	(*((unsigned char volatile xdata *) (REG_START_ADDRESS+0x130) ))
#define TW8835_R1C1	(*((unsigned char volatile xdata *) (REG_START_ADDRESS+0x1C1) ))
#define TW8835_R1CD	(*((unsigned char volatile xdata *) (REG_START_ADDRESS+0x1CD) ))
#define TW8835_R412	(*((unsigned char volatile xdata *) (REG_START_ADDRESS+0x412) ))
#define TW8835_R508	(*((unsigned char volatile xdata *) (REG_START_ADDRESS+0x508) ))

/*
TODO: refine internal TW88 command.
compare example. 
================
r101 = ReadTW88(REG101);
r130 = ReadTW88(REG130);
------------------------
0005         L?0034:
0005 F582              MOV     DPL,A
0007 E500        E     MOV     A,regTW88
0009 3401              ADDC    A,#01H
000B F583              MOV     DPH,A
000D 22                RET     
..
00B5         L?0035:
00B5 900000      R     MOV     DPTR,#r130
00B8 F0                MOVX    @DPTR,A
..
003C E500        E     MOV     A,regTW88+01H
003E 2401              ADD     A,#01H
0040 120000      R     LCALL   L?0034
0043 E0                MOVX    A,@DPTR
0044 900000      R     MOV     DPTR,#r101
0047 F0                MOVX    @DPTR,A
                                           ; SOURCE LINE # 389
0048 E500        E     MOV     A,regTW88+01H
004A 2430              ADD     A,#030H
004C 120000      R     LCALL   L?0034
004F E0                MOVX    A,@DPTR
                                           ; SOURCE LINE # 390
                                           ; SOURCE LINE # 391
0050 120000      R     LCALL   L?0035


r101 = TW8835_R101;
r130 = TW8835_R130;
0076 90C101            MOV     DPTR,#0C101H
0079 E0                MOVX    A,@DPTR
007A 900000      R     MOV     DPTR,#r101
007D F0                MOVX    @DPTR,A
                                           ; SOURCE LINE # 402
007E 90C130            MOV     DPTR,#0C130H
0081 E0                MOVX    A,@DPTR
                                           ; SOURCE LINE # 403
                                           ; SOURCE LINE # 404
0082 120000      R     LCALL   L?0035

*/

//===================================================================
// special port
//===================================================================

#define PORT_I2C_SCL			P1_0	//I2C_SCL
#define PORT_I2C_SDA			P1_1	//I2C_SDA

#define PORT_I2CCMD_GPIO_SLAVE	P1_3	//INT10
#define PORT_REMO				P1_4	//TCPOLN uses M_GPO

#define PORT_POWER_SAVE			P1_5
#define PORT_NOINIT_MODE		P1_5
#define PORT_BACKDRIVE_MODE		P1_6	//RearCameraDisplay(RCD) MODE

#define PORT_I2CCMD_GPIO_MASTER	P1_7	//P3_1

#define PORT_CRYSTAL_OSC		P2_7
#define PORT_EXTMCU_ISP			P3_2
//#ifdef MODEL_TW8835
//#define PORT_DEBUG				P3_3	//use PORT_DEBUG = !PORT_DEBUG. 
//#else
//#define PORT_DEBUG				P1_4	//use PORT_DEBUG = !PORT_DEBUG. conflict with DTVDE. 
//#endif

#endif	//.._TW8836_H_




