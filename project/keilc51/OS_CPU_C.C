/*********************************************************************************************************
**				                               Small RTOS 51
**                                   The Real-Time Kernel For Keil c51
**
**                                  (c) Copyright 2002-2003, chenmingji
**                                           All Rights Reserved
**
**                                                  V1.20.2
**
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: OS_CPU_C.C
**创   建   人: 陈明计
**最后修改日期:  2004年4月22日
**描        述: Small RTOS 51与CPU(既8051系列)相关的C语言代码
**
**--------------历史版本信息----------------------------------------------------------------------------
** 创建人: 陈明计
** 版  本: V0.50~v1.00
** 日　期: 2002年2月22日~2002年6月20日
** 描　述: 基本完成
**
**------------------------------------------------------------------------------------------------------
** 修改人: 陈明计
** 版  本: V1.10~V1.12.0
** 日　期: 2002年9月1日~002年12月30日
** 描　述: 完善代码
**
**------------------------------------------------------------------------------------------------------
** 修改人: 陈明计
** 版  本: V1.20.0
** 日　期: 2003年8月17日
** 描　述: 根据新版本需要增减了函数
**
**------------------------------------------------------------------------------------------------------
** 修改人: 陈明计
** 版  本: V1.20.2
** 日　期: 2004年4月22日
** 描　述: 修正任务删除时的一个BUG
**
**--------------当前版本修订------------------------------------------------------------------------------
** 修改人:
** 日　期:
** 描　述:
**
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/

#define  IN_OS_CPU_C
#include "CONFIG.h"

extern idata uint8 STACK[1];                    /* 堆栈起始位置,在OS_CPU_A定义 */
uint8 idata * data OSTsakStackBotton[OS_MAX_TASKS + 2];/* 任务堆栈底部位置            */

#if EN_SP2 > 0
uint8 idata  Sp2[Sp2Space];                     /* 高级中断堆栈           */
#endif

#if OS_MAX_TASKS < 9            
uint8 data OSFastSwap[1];                        /* 任务是否可以快速切换 */
#else
uint8 data OSFastSwap[2];
#endif

extern data uint16 C_XBP;

void TaskIdle(void)   small;
/*********************************************************************************************************
** 函数名称: OSCPUInit
** 功能描述: Small RTOS 与系统相关的初始化
** 输　入: 无
** 输　出 : 无
** 全局变量: OSTaskID,OSTsakStackBotton,SP
** 调用模块: LoadCtx
** 
** 作　者: 陈明计
** 日　期: 2002年2月22日
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**-------------------------------------------------------------------------------------------------------
********************************************************************************************************/
        void OSCPUInit(void)  small
{
    uint8 i;

    for (i = 0; i < OS_MAX_TASKS + 1; i++)
    {
        OSTsakStackBotton[i] = STACK;
    }
    OSTsakStackBotton[OS_MAX_TASKS + 1] = (uint8 idata *)(IDATA_RAM_SIZE % 256);
    
}

/*********************************************************************************************************
** 函数名称: OSTaskStkInit
** 功能描述: 任务堆栈初始化
** 输　入: 无
** 输　出 : 无
** 全局变量: OSTaskID,OSTsakStackBotton,SP
** 调用模块: LoadCtx
** 
** 作　者: 陈明计
** 日　期: 2002年2月22日
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**-------------------------------------------------------------------------------------------------------
********************************************************************************************************/
/*
#if EN_REENTRANT > 0
#define SP_ADD_BYTE  5
#else
#define SP_ADD_BYTE  3
#endif


        void OSTaskStkInit(void (code * task)(void), void xdata *ptos, uint8 TaskID) small
{
    uint8 i;
    uint8 idata * cp;
    uint8 idata * cp1;
    
#if OS_MAX_TASKS < 9
        OSFastSwap[1] |= OSMapTbl[TaskID];
#else
        if (TaskID < 8)
        {
            OSFastSwap[1] |= OSMapTbl[TaskID];
        }
        else
        {
            OSFastSwap[0] |= OSMapTbl[TaskID & 0x07];
        }
#endif
    
    if (TaskID < OSRunningTaskID())
    {
        i =  OSRunningTaskID() - TaskID;
        cp = (uint8 idata *)(&(OSTsakStackBotton[TaskID + 1]));
        
        do
        {
            *cp += SP_ADD_BYTE;
            cp++;
        } while (--i != 0);
        
        cp1 = (uint8 idata *)SP;
        SP = SP + SP_ADD_BYTE;
        i = SP - (uint8)(OSTsakStackBotton[TaskID + 1]);
        cp = (uint8 idata *)SP;
        do
        {
            *cp-- = *cp1--;
        } while (--i != 0);
    }
    else
    {
        cp1 = (uint8 idata *)(&(OSTsakStackBotton[OSRunningTaskID() + 1]));
        
        i = TaskID - OSRunningTaskID();
        
        do
        {
            *cp1 -= SP_ADD_BYTE;
            cp1++;
        } while (--i != 0);
        
        cp = OSTsakStackBotton[OSRunningTaskID() + 1];
        i = OSTsakStackBotton[TaskID] - cp - SP_ADD_BYTE;
        cp1 = cp - SP_ADD_BYTE;
        do
        {
            *cp1++ = *cp++;
        } while (--i != 0);
    }
    cp = OSTsakStackBotton[TaskID];
    *cp++ = (uint16)task % 256;
    *cp++ = (uint16)task / 256;
#if EN_REENTRANT > 0
    *cp++ = (uint16)ptos / 256;
    *cp++ = (uint16)ptos % 256;
#endif
    *cp = 0;
}
    */


/*********************************************************************************************************
** 函数名称: OSTaskStkDel
** 功能描述: 任务堆栈删除
** 输　入: 无
** 输　出 : 无
** 全局变量: OSTaskID,OSTsakStackBotton,SP
** 调用模块: LoadCtx
** 
** 作　者: 陈明计
** 日　期: 2003年8月17日
**-------------------------------------------------------------------------------------------------------
** 修改人: 陈明计
** 日　期: 2004年4月22日
**-------------------------------------------------------------------------------------------------------
********************************************************************************************************/

        void StkDelA(uint8 TaskID)
{
    uint8 idata *cp;
    uint8 idata *cp1;
    uint8 temp, i;
    
    cp = OSTsakStackBotton[TaskID];
    cp1 = OSTsakStackBotton[TaskID + 1];
    temp = cp1 - cp;
    for (i = TaskID + 1; i <= OSRunningTaskID(); i++)
    {
        OSTsakStackBotton[i] -= temp;
    }
    i = SP - (uint8)(cp1) + 1;
    do
    {
        *cp++ = *cp1++;
    } while (--i != 0);
    SP = SP - temp;
}


        void StkDelB(uint8 TaskID)
{
    uint8 idata *cp;
    uint8 idata *cp1;
    uint8 temp, i;

    cp = OSTsakStackBotton[TaskID] - 1;
    cp1 = OSTsakStackBotton[TaskID + 1] - 1;
    i = OSTsakStackBotton[TaskID] - OSTsakStackBotton[OSRunningTaskID() + 1];
    if (i != 0)
    {
        do
        {
            *cp1-- = *cp--;
        } while (--i != 0);
    }
    temp = cp1 - cp;
    for (i = OSRunningTaskID() + 1; i <= TaskID; i++)
    {
        OSTsakStackBotton[i] += temp;
    }
}

        void StkDelC(uint8 TaskID)
{
    uint8 idata *cp;
    extern void C_OSCtxSw(void);
    
    
    cp = OSTsakStackBotton[TaskID];
    *cp++ = (uint16)C_OSCtxSw % 256;
    *cp = (uint16)C_OSCtxSw / 256;
    SP = (uint8) cp;
    OSFindNextRunningTask();
}

        void OSTaskStkDel(uint8 TaskID)  small
{
    uint8 temp;

    temp = 1;
    if (TaskID == OSRunningTaskID())
    {
        temp = 2;
    }
    else if (TaskID < OSRunningTaskID())
    {
        temp = 0;
    }
    switch (temp)
    {
        case 0:
            StkDelA(TaskID);
            break;
        case 1:
            StkDelB(TaskID);
            break;
        case 2:
            StkDelC(TaskID);
            break;
        default:
            break;
    }
}

/*********************************************************************************************************
** 函数名称: OSTickISR
** 功能描述: 系统时钟中断服务函数
** 输　入: 无
** 输　出 : 无
** 全局变量: 无
** 调用模块: OS_IBT_ENTER,(UserTickTimer),OSTimeTick,OSIntExit
** 
** 作　者: 陈明计
** 日　期: 2002年2月22日
**-------------------------------------------------------------------------------------------------------
** 修改人: 陈明计
** 日　期: 2002年10月23日
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**-------------------------------------------------------------------------------------------------------
********************************************************************************************************/
#pragma disable                                        /* 除非最高优先级中断，否则，必须加上这一句 */

void OSTickISR(void) interrupt OS_TIME_ISR
{
#if TICK_TIMER_SHARING > 1 
    static uint8 TickSum = 0;
#endif

#if EN_USER_TICK_TIMER > 0 
    UserTickTimer();                                    /* 用户函数 */
#endif

#if TICK_TIMER_SHARING > 1 
    TickSum = (TickSum + 1) % TICK_TIMER_SHARING;
    if (TickSum != 0)
    {
        return;
    }
#endif

    OS_INT_ENTER();                                     /* 中断开始处理 */

#if EN_TIMER_SHARING > 0
    OSTimeTick();                                       /* 调用系统时钟处理函数 */
#else
    OSIntSendSignal(TIME_ISR_TASK_ID);                  /* 唤醒ID为TIME_ISR_TASK_ID的任务 */
#endif
    
    OSIntExit();                                        /* 中断结束处理 */
}

/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/
