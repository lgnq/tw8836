/*********************************************************************************************************
**				                               Small RTOS(51)
**                                   The Real-Time Kernel(For Keil c51)
**
**                                  (c) Copyright 2002-2003, chenmingji
**                                           All Rights Reserved
**
**                                                  V1.12.0
**
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: serial.c
**创   建   人: 陈明计
**最后修改日期: 2002年12月30日
**描        述: 为信号量编制的测试代码。
**
**--------------历史版本信息----------------------------------------------------------------------------
** 创建人: 陈明计
** 版  本: V1.10
** 日　期: 2002年9月1日
** 描　述: 原始版本
**
**------------------------------------------------------------------------------------------------------
** 修改人: 陈明计
** 版  本: V1.11.0
** 日　期: 2003年12月2日
** 描　述: 删除TaskE代码
**
**------------------------------------------------------------------------------------------------------
** 修改人: 陈明计
** 版  本: V1.12.0
** 日　期: 2003年12月30日
** 描　述: 无变化
**
**--------------当前版本修订------------------------------------------------------------------------------
** 修改人: 
** 日　期:
** 描　述:
**
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
#include "config.h"
#include "cpu.h"

void TaskA(void);
void TaskB(void);
void TaskC(void);
void TaskD(void);

void putch(uint8 Data)
{
    uint8 i,j;

    SBUF = Data;
    i = 2048 / 256;
    j = 2048 % 256;
    do
    {
        do
        {
        } while (--j !=0);
    } while (--i != 0);    
}

void init_task(void)
{
	init_cpu();
	EA = 1;
	
	while (1)
	{
		putch('x');
		OSWait(K_TMO, 100);		
	}
}

void main(void)
{
    OSInit();
	
		OSTaskCreate(init_task, NULL, 0);
//    OSTaskCreate(TaskA, NULL, 0);
//    OSTaskCreate(TaskB, NULL, 1);
//    OSTaskCreate(TaskC, NULL, 2);
//    OSTaskCreate(TaskD, NULL, 3);

    while(1)
    {
			;
//        PCON = PCON | 0x01;                     /* CPU进入休眠状态 */
    }
}

void TaskA(void)
{
	init_cpu();
/*
	TMOD = (TMOD & 0XF0) | 0X01;
	TL0 = 0x0;
	TH0 = 0x0;
	TR0 = 1;
	ET0 = 1;
	TF0 = 0;
*/	
  EA = 1;
    
	OSSemCreate(0,1);
    while (1)
    {
        OSSemPend(0,0);
        putch('A');
        putch('0');
        putch('1');
        putch('2');
        putch('3');
        putch('\n');
        OSSemPost(0);
        OSWait(K_TMO,TL0);
    } 
}

void TaskB(void)
{
    while (1)
    {
        OSSemPend(0,0);
        putch('B');
        putch('0');
        putch('1');
        putch('2');
        putch('3');
        putch('\n');
        OSSemPost(0);
        OSWait(K_TMO,TL0);
    } 
}

void TaskC(void)
{
    while (1)
    {
        OSSemPend(0,0);
        putch('C');
        putch('0');
        putch('1');
        putch('2');
        putch('3');
        putch('\n');
        OSSemPost(0);
        OSWait(K_TMO,TL0);
    } 
}

void TaskD(void)
{
    uint8 i;
    
    while (1)
    {
        OSSemPend(0,0);
        for (i = 0; i < 100; i++)
        {
            putch('D');
            putch('E');
            putch('F');
            putch('G');
            putch(' ');
        }
        OSSemPost(0);
    } 
}
