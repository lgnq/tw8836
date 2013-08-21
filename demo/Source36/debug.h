#ifndef __DEBUG__
#define __DEBUG__

#define DEBUG_ERR	1
#define DEBUG_WARN	2
#define DEBUG_INFO	3
#define DEBUG_BREAK	4

#define Pause(a)	{ Printf("\r\n"); Printf(a); while(!RS_ready()); RS_rx(); }
//#define assert(x)	
#define assert(x) if((x)==0) { Printf("\nAssertion failed: line:%d",__LINE__); }

#endif	// __DEBUG__
