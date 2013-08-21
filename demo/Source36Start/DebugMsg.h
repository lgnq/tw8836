/* 
DebugMsg.h
*/
#if defined(SUPPORT_HDMI_EP9351) || defined(SUPPORT_HDMI_EP907M)

void DBG_PrintAviInfoFrame(void);
void DBG_PrintTimingRegister(void);
void DBG_DumpControlRegister(void);

#else 
void Dummy_DebugMsg_func(void);
#endif //..SUPPORT_HDMI_EP9351

void DumpDviTable(WORD hActive,WORD vActive);

void DbgMsg_EEP_Corruptted(void);

