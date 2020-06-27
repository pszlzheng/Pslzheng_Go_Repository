#ifndef _LED_H
#define _LED_H
#include "sys.h"

#define LED0 			PBout(1)   //LED0
#define LED1 			PBout(0)   //LED1

#define LED_OXY 	PDout(13)  //LED_OXY
#define LED_PH 		PHout(9)   //LED_PH
#define LED_TEMP 	PGout(6)   //LED_TEMP

#define FUN_Food 	PHout(10)  //FUN_OXY
#define FUN_Fert	PEout(5)   //FUN_PH
#define FUN_Oxyg 	PEout(3)   //FUN_TEMP

void LED_Init(void);
#endif
