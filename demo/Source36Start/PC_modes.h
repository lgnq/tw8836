#ifndef __PC_MODE_DATA__
#define __PC_MODE_DATA__

extern code struct _PCMODEDATA PCMDATA[];
extern CONST struct _PCMODEDATA DVIMDATA[] ;

DWORD sizeof_PCMDATA(void);
DWORD sizeof_DVIMDATA(void);

void PC_PrepareInfoString(BYTE mode);
void YUV_PrepareInfoString(BYTE mode);
void DVI_PrepareInfoString(WORD han, WORD van, BYTE vfreq);

#endif

