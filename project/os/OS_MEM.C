/*********************************************************************************************************
**				                               Small RTOS(51)
**                                   The Real-Time Kernel(For Keil c51)
**
**                                  (c) Copyright 2002-2003, chenmingji
**                                           All Rights Reserved
**
**                                                  V1.20.0
**
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: OS_mem.c
**创   建   人: 陈明计
**最后修改日期:  2003年8月3日
**描　     述: 内存动态分配模块，可以在非Small RTOS(51)下使用。在Small RTOS(51)
**             下必须配置为支持重入栈。
**--------------历史版本信息----------------------------------------------------------------------------
** 创建人: 陈明计
** 版  本: V1.20.0
** 日　期: 2003年8月3日
** 描　述: 原始版本
**
**------------------------------------------------------------------------------------------------------
**--------------当前版本修订------------------------------------------------------------------------------
** 修改人: 
** 日　期:
** 描　述:
**
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
#define IN_OS_MEM
#include "config.h"

#ifndef EN_OS_MEM
#define EN_OS_MEM       0
#endif

#if EN_OS_MEM > 0
static struct _FreeMem xdata *OSFreeMem;                /* 自由内存块的双向链表的头 */


/*********************************************************************************************************
** 函数名称: UserMemChkErr
** 功能描述: 内存回收时出错处理函数，将EN_UserMemChkErr定义为1时,用户可以自己定义这个函数。
** 输　入: Addr: 应当回收的内存块
** 输　出: 无
** 全局变量: 无
** 调用模块: 无
**
** 作　者: 陈明计
** 日　期: 2003年8月3日
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
#if EN_OS_MEM_CHK > 0 && EN_UserMemChkErr == 0
        void UserMemChkErr(void xdata *Addr) reentrant
{
    Addr = Addr;
    while (1);
}
#endif

/*********************************************************************************************************
** 函数名称: OSMemInit
** 功能描述: 初始化动态内存分配模块。
** 输　入: Addr: 指向空闲空间起始位置的指针
**        MemSize: 空闲空间的大小 
** 输　出: TRUE: 成功
**        FALSE: 失败
** 全局变量: 无
** 调用模块: 无
**
** 作　者: 陈明计
** 日　期: 2003年8月3日
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
        uint8 OSMemInit(void xdata *Addr, unsigned int MemSize)
{
    struct _FreeMem xdata *MemFree;
    
    MemSize = MemSize & ~(sizeof(int) - 1);     /* 内存按照字(word)为单位分配 */
                                                /* 调整MemSize为字(word)的整数倍 */

    /* 判断自由空间是否有效 */
    if (Addr != NULL && MemSize > sizeof(struct _UsingMem))
    {
        /* 建立只有一个节点的双向队列 */
        OS_ENTER_CRITICAL();
        MemFree = (struct _FreeMem xdata *)Addr;
        OSFreeMem = MemFree;
        MemFree->Size = MemSize;
        MemFree->Next = NULL;
        MemFree->Last = NULL;
        OS_EXIT_CRITICAL();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*********************************************************************************************************
** 函数名称: OSMemNew
** 功能描述: 分配内存。
** 输　入: Size: 所需内存的大小
** 输　出: 指向已分配的内存开始的地址的指针，NULL为失败。
** 全局变量: 无
** 调用模块: 无
**
** 作　者: 陈明计
** 日　期: 2003年8月3日
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
        void xdata *OSMemNew(unsigned int Size) reentrant
{
    struct _FreeMem xdata *ThisFreeMem;
    struct _UsingMem xdata *Rt;
    
    OS_ENTER_CRITICAL();
    ThisFreeMem = OSFreeMem;
    
    /* 调整应当分配的大小（必须为字(word)）的整数倍，且要加入分配给头的空间 */
    Size = ((Size + sizeof(int) - 1) & ~(sizeof(int) - 1)) + 
           sizeof(struct _UsingMem);
    
    /* 查找足够大的自由内存块 */
    while(ThisFreeMem != NULL)
    {
        if ((ThisFreeMem->Size) >= Size)
        {
            break;
        }
        ThisFreeMem = ThisFreeMem->Next;
    }
    if (ThisFreeMem != NULL)
    {
        if ((ThisFreeMem->Size) < (Size + sizeof(struct _FreeMem)))
        {
            /* 分配后剩余内存太小则整块自由内存分配出去 */
            /* 即在双向链表中删除这个节点 */
            if (ThisFreeMem->Next != NULL)
            {
                (ThisFreeMem->Next)->Last = ThisFreeMem->Last;
            }
            if (ThisFreeMem->Last)
            {
                (ThisFreeMem->Last)->Next = ThisFreeMem->Next;
            }
            Size = ThisFreeMem->Size;
            Rt = (struct _UsingMem xdata *)(ThisFreeMem);
        }
        else
        {
            /* 否则从自由内存块的高地址端分配一块内存 */
            /* 即仅仅调整这个节点的内容 */
            ThisFreeMem->Size -= Size;
            Rt = (struct _UsingMem xdata *)((uint8 xdata *)ThisFreeMem + 
                                             ThisFreeMem->Size);
        }
#if EN_OS_MEM_CHK > 0
        Rt->HeadChk = 0xa55a;
        Rt->EndChk = 0x5aa5;
#endif
        Rt->Size = Size;
        OS_EXIT_CRITICAL();
        return ( void *)((uint8 xdata *)Rt + sizeof(struct _UsingMem));
    }
    else
    {
        OS_EXIT_CRITICAL();
        return NULL;
    }
    
}

/*********************************************************************************************************
** 函数名称: OSMemFree
** 功能描述: 把由OSMemNew分配的空间归还给系统
** 输　入: Addr: 由指OSMemNew返回的指针
** 输　出: 无
** 全局变量: 无
** 调用模块: 无
**
** 作　者: 陈明计
** 日　期: 2003年8月3日
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
#ifndef EN_OS_MEM_FREE
#define EN_OS_MEM_FREE      1
#endif

#if EN_OS_MEM_FREE > 0

        void OSMemFree(void xdata *Addr) reentrant
{
    struct _FreeMem xdata *ThisFreeMem;
    struct _UsingMem xdata *UserMem;
    unsigned int Size;

    OS_ENTER_CRITICAL();
    
    ThisFreeMem = OSFreeMem;
    /* 找到内存块的开始位置 */
    UserMem = (struct _UsingMem xdata *)((uint8 xdata *)Addr - 
                                          sizeof(struct _UsingMem));
#if EN_OS_MEM_CHK > 0
    if (UserMem->HeadChk != 0xa55a || UserMem->EndChk != 0x5aa5)
    {
        UserMemChkErr(Addr);
        OS_EXIT_CRITICAL();
        return;
    }
#endif
    /* 查找这块内存应当在双向链表中插入的位置 */    
    /* 双向链表以内存块首地址的大小顺序为节点的链接顺序 */
    while (1)
    {
        if (ThisFreeMem > (struct _FreeMem xdata *)UserMem)
        {
            ThisFreeMem = ThisFreeMem->Last;
            break;
        }
        if (ThisFreeMem->Next == NULL)
        {
            break;
        }
        
        ThisFreeMem = ThisFreeMem->Next;
    }

    if (((uint8 xdata *)ThisFreeMem + (ThisFreeMem->Size)) == (uint8 xdata *)UserMem)
    {
        /* 回收的内存块与这一个自由内存块相邻，则直接修改这个节点内容 */
        ThisFreeMem->Size += UserMem->Size;
    }
    else
    {
        /* 否则插入一个节点 */
        Size = UserMem->Size;
        ((struct _FreeMem xdata *)UserMem)->Last = ThisFreeMem;
        ((struct _FreeMem xdata *)UserMem)->Next = ThisFreeMem->Next;
        ((struct _FreeMem xdata *)UserMem)->Size = Size;
        ThisFreeMem->Next = (struct _FreeMem xdata *)UserMem;
        ThisFreeMem = (struct _FreeMem xdata *)UserMem;
        if (ThisFreeMem->Next != NULL)
        {
            (ThisFreeMem->Next)->Last = ThisFreeMem;
        }
    }
    if (((uint8 xdata *)ThisFreeMem + (ThisFreeMem->Size)) ==
         (uint8 xdata *)(ThisFreeMem->Next))
    {
        /* 如果这两个内存块相邻，则合并之 */
        ThisFreeMem->Size += (ThisFreeMem->Next)->Size;
        ThisFreeMem->Next = (ThisFreeMem->Next)->Next;
        if(ThisFreeMem->Next != NULL)
        {
            (ThisFreeMem->Next)->Last = ThisFreeMem;
        }
    }
    OS_EXIT_CRITICAL();
    return;
}
#endif
#endif
/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/
