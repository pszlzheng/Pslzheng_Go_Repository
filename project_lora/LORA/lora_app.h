#ifndef _LORA_APP_H_
#define _LORA_APP_H_

#include "sys.h"
#include "lora_cfg.h"

#define LORA_AUX  PIin(11)  //LORA模块状态引脚
#define LORA_MD0  PAout(4)  //LORA模块控制引脚

extern _LoRa_CFG LoRa_CFG;
extern u8 Lora_mode;

u8 LoRa_Init(void);
void Aux_Int(u8 mode);
void LoRa_Set(void);
void LoRa_SendData(int counter);
void LoRa_ReceData(void);
void LoRa_Process(void);
void Lora_Test(void);
void Lora_analyze(u8* bits);
void little_rooster_hen(void);
void big_rooster_hen(void);
void little_duck(void);
void big_duck(void);

#endif
