#include "reg.h"
#include "uart.h"
#include "typedefs.h"

void init_uart(void)
{
	SFR_ES = 0;			
	SFR_UART0FIFO = 0x80;	
}

void putch(unsigned char c)
{
    unsigned char i,j;

		SFR_SBUF = c;
    i = 2048 / 256;
    j = 2048 % 256;
    do
    {
        do
        {
        } while (--j !=0);
    } while (--i != 0);    
}

/*
void uart0_isr(void) interrupt 4 using 1
{
	uint8	count;
	uint8 data temp;

	SFR_EA = 0;
	
	if (RI == 1)
	{
		OS_INT_ENTER();
		RI = 0;
		if (SFR_UART0FIFO & 0x80)
		{		// use fifo?
			count = SFR_UART0FIFO & 0x1F;
			if (count & 0x10)
			{
				SFR_UART0FIFO = 0x90;		// overflowed, buffer clear
			}
			else
			{
				while (count)
				{
					OSQIntPost(SerialInData, SBUF);
					count--;
				};
			}
		}
		else 
		{
			OSQIntPost(SerialInData, SBUF);
		}
		OSIntExit();
		
		return;
	}
	
	if (TI == 1)
	{
		TI = 0;
		if (OSQAccept(&temp, SerialOutData) == OS_Q_OK)
		{
		    SBUF = temp;
		}
		else
		{
		    SerialCanSend = 1;
		}
	}
	
	SFR_EA = 1;
}
*/