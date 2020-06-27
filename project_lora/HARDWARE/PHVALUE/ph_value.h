#ifndef __PH_VALUE_H
#define __PH_VALUE_H
#include "sys.h"

//IO方向设置
#define PH_VALUE_IO_IN()  {GPIOB->MODER&=~(3<<(12*2));GPIOB->MODER|=0<<12*2;}
#define PH_VALUE_IO_OUT() {GPIOB->MODER&=~(3<<(12*2));GPIOB->MODER|=1<<12*2;}
//IO输入设置
#define	PH_VALUE_DQ_OUT PHout(10)
#define	PH_VALUE_DQ_IN  PHin(10)

u8 PH_VALUE_Init(void);			
short PH_VALUE_Get(void);
void PH_VALUE_Start(void);		
void PH_VALUE_Write_Byte(u8 dat);
u8 PH_VALUE_Read_Byte(void);		
u8 PH_VALUE_Read_Bit(void);
u8 PH_VALUE_Check(void);		
void PH_VALUE_Rst(void);		
#endif