#include "lora_app.h"
#include "lora_ui.h"
#include "lora_cfg.h"
#include "usart3.h"
#include "string.h"
#include "led.h"
#include "delay.h"
#include "lcd.h"
#include "stdio.h"
#include "text.h"
#include "key.h"
#include "usart.h"
#include "ds18b20.h"
#include "pcf8574.h"
#include "adc.h"


//�豸������ʼ��(�����豸������lora_cfg.h����)
_LoRa_CFG LoRa_CFG=
{
	.addr = LORA_ADDR,       //�豸��ַ
	.power = LORA_POWER,     //���书��
	.chn = LORA_CHN,         //�ŵ�
	.wlrate = LORA_RATE,     //��������
	.wltime = LORA_WLTIME,   //˯��ʱ��
	.mode = LORA_MODE,       //����ģʽ
	.mode_sta = LORA_STA,    //����״̬
	.bps = LORA_TTLBPS ,     //����������
	.parity = LORA_TTLPAR    //У��λ����
};


GPIO_InitTypeDef GPIO_Initure;

//�豸����ģʽ(���ڼ�¼�豸״̬)
u8 Lora_mode=0;// 0:����ģʽ 1:����ģʽ 2:����ģʽ

//��¼�ж�״̬
static u8 Int_mode=0;//0���ر� 1:������ 2:�½���

//AUX�ж����ã�mode:���õ�ģʽ 0:�ر� 1:������ 2:�½���
void Aux_Int(u8 mode)
{
	if(!mode)
	{
		HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);//ʧ���ж���11
	}
	else
	{
		if(mode==1)
		{
			 GPIO_Initure.Pin=GPIO_PIN_11; //PI11
       GPIO_Initure.Mode=GPIO_MODE_IT_RISING;//�����ش���
		}
			
	  else if(mode==2)
		{
			 GPIO_Initure.Pin=GPIO_PIN_11; //PI11
       GPIO_Initure.Mode=GPIO_MODE_IT_FALLING;//�½��ش���
		}
		
		 HAL_GPIO_Init(GPIOI,&GPIO_Initure);//���³�ʼ��
	   __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_11);//����жϱ�־λ
		 HAL_NVIC_SetPriority(EXTI15_10_IRQn,2,0); //��ռ���ȼ�Ϊ2�������ȼ�Ϊ0
		 HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);//ʹ��
	}
	Int_mode = mode;//��¼�ж�ģʽ
}

void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);//�����жϴ����ú���	
}

////�жϷ����������Ҫ��������
////��HAL�������е��ⲿ�жϷ�����������ô˺���
////GPIO_Pin:�ж����ź�
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin==GPIO_PIN_11)
	{
	   if(Int_mode==1)//������(����:��ʼ�������� ����:���ݿ�ʼ���)     
	   {
		  if(Lora_mode==1)//����ģʽ
		  {
			 USART3_RX_STA=0;//���ݼ�����0
		  }
		  Int_mode=2;//�����½���
		  LED0=0;//DS0��
	   }
       else if(Int_mode==2)//�½���(����:�����ѷ����� ����:�����������)	
	   {
		  if(Lora_mode==1)//����ģʽ
		  {
			 USART3_RX_STA|=1<<15;//���ݼ���������
		  }else if(Lora_mode==2)//����ģʽ(�������ݷ������)
		  {
			 Lora_mode=1;//�������ģʽ
		  }
		  Int_mode=1;//����������
          LED0=1;//DS0��		   
	   }
     Aux_Int(Int_mode);//���������жϱ���
	}
}

//LoRaģ���ʼ��
//����ֵ:0,���ɹ�
//       1,���ʧ��
u8 LoRa_Init(void)
{
	u8 retry=0;
	u8 temp=1;

	__HAL_RCC_GPIOA_CLK_ENABLE();          //����GPIOAʱ��
	__HAL_RCC_GPIOI_CLK_ENABLE();          //����GPIOIʱ��
	 
	GPIO_Initure.Pin=GPIO_PIN_4;           //PA4
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP; //�������
	GPIO_Initure.Pull=GPIO_PULLDOWN;       //����
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;    //����
	HAL_GPIO_Init(GPIOA,&GPIO_Initure);

	GPIO_Initure.Pin=GPIO_PIN_11;          //PI11
	GPIO_Initure.Mode=GPIO_MODE_IT_RISING; //�����ش���
	GPIO_Initure.Pull=GPIO_PULLDOWN;       //����
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;    //����
	HAL_GPIO_Init(GPIOI,&GPIO_Initure);
	
	LORA_MD0=0;				//MD0,AUX��Ϊ0ʱ��lora����һ��ģʽ������͸��
	LORA_AUX=0;
	
	while(LORA_AUX)//ȷ��LORAģ���ڿ���״̬��(LORA_AUX=0)
	{
		 Show_Str(40+30,50+20,200,16,"BUSYING NOW!!!",16,0); 	
		 delay_ms(500);
		 Show_Str(40+30,50+20,200,16,"______________",16,0);
     delay_ms(100);		 
	}
	usart3_init(115200);//��ʼ������3
	LORA_MD0=1;//����AT�����ã�ģʽ
	delay_ms(40);
	retry=3;
	while(retry--)
	{
	 if(!lora_send_cmd("AT","OK",70))
	 {
		 temp=0;//���ɹ�
		 break;
	 }	
	}
	if(retry==0) temp=1;//���ʧ��
	return temp;
}

//Loraģ���������
void LoRa_Set(void)
{
	u8 sendbuf[20];
	u8 lora_addrh,lora_addrl=0;
	
	usart3_set(LORA_TTLBPS_115200,LORA_TTLPAR_8N1);//��������ģʽǰ����ͨ�Ų����ʺ�У��λ(115200 8λ���� 1λֹͣ ������У�飩
	usart3_rx(1);//��������3����
	
	while(LORA_AUX);//�ȴ�ģ�����
	LORA_MD0=1; 		//��������ģʽ
	delay_ms(40);
	Lora_mode=0;//���"����ģʽ"
	
	lora_addrh = (LoRa_CFG.addr>>8)&0xff;//ͨ�ŵ�ַ�ĸ߰�λ
	lora_addrl = LoRa_CFG.addr&0xff;//ͨ�ŵ�ַ�Ͱ�λ
	sprintf((char*)sendbuf,"AT+ADDR=%02x,%02x",lora_addrh,lora_addrl);//�����豸��ַ
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+WLRATE=%d,%d",LoRa_CFG.chn,LoRa_CFG.wlrate);//�����ŵ��Ϳ�������
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+TPOWER=%d",LoRa_CFG.power);//���÷��书��
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+CWMODE=%d",LoRa_CFG.mode);//���ù���ģʽ
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+TMODE=%d",LoRa_CFG.mode_sta);//���÷���״̬
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+WLTIME=%d",LoRa_CFG.wltime);//����˯��ʱ��
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+UART=%d,%d",LoRa_CFG.bps,LoRa_CFG.parity);//���ô��ڲ����ʡ�����У��λ
	lora_send_cmd(sendbuf,"OK",50);

	LORA_MD0=0;//�˳�����,����ͨ��
	delay_ms(40);
	while(LORA_AUX);//�ж��Ƿ����(ģ����������ò���)
	USART3_RX_STA=0;
	Lora_mode=1;//���"����ģʽ"
	usart3_set(LoRa_CFG.bps,LoRa_CFG.parity);//����ͨ��,����ͨ�Ŵ�������(�����ʡ�����У��λ)
	Aux_Int(1);//����LORA_AUX�������ж�
}

float data[18]={18,6.8,6.0,22,6.9,6.0,26,7.0,6.0,20,7.2,8.0,24,7.3,8.0,27,7.4,8.0};
char flag='Y';

void ST_MON_1(char check,float tm1,float ph1,float oxy1)//��һ�׶ε�һ��ˮ��Ʒ��1���䣩
{
	float ph;
	float oxy;
	float temp;
	if (check=='1')
	
	{
		data[0]=tm1;
		data[1]=ph1;
		data[2]=oxy1;
	}
	PCF8574_ReadBit(BEEP_IO);
	temp=DS18B20_Get_Temp();
	oxy=Get_Adc_Average(ADC_CHANNEL_5,10)*0.0032;
	ph=Get_Adc_Average(ADC_CHANNEL_6,10)*0.0024;
	PCF8574_ReadBit(BEEP_IO);
	temp=DS18B20_Get_Temp();
	if((temp/10)>data[0]-2 && (temp/10)<=data[0]+2)		LED_TEMP=0;
	else
	{
		LED_TEMP=1;
		flag='N';
	}
	if(ph>data[1]-0.8 && ph<data[1]+0.8)	LED_PH=0;
	else
	{
		LED_PH=1;
		flag='N';
	}
	if(oxy>data[2]-2)	LED_OXY=0;
	else
	{
		LED_OXY=1;
		FUN_Oxyg=0;
		flag='N';
	}
}

void ND_MON_1(char check,float tm1,float ph1,float oxy1)//�ڶ��׶ε�һ��ˮ��Ʒ��3���䣩
{
	float ph;
	float oxy;
	float temp;
	if (check=='1')
	{
		data[3]=tm1;
		data[4]=ph1;
		data[5]=oxy1;
	}
	PCF8574_ReadBit(BEEP_IO);
	temp=DS18B20_Get_Temp();
	oxy=Get_Adc_Average(ADC_CHANNEL_5,10)*0.0032;
	ph=Get_Adc_Average(ADC_CHANNEL_6,10)*0.0024;
	PCF8574_ReadBit(BEEP_IO);
	temp=DS18B20_Get_Temp();
	if((temp/10)>data[3]-2 && (temp/10)<=data[3]+2)		LED_TEMP=0;
	else
	{
		LED_TEMP=1;
		flag='N';
	}
	if(ph>data[4]-0.8 && ph<data[4]+0.8)	LED_PH=0;
	else
	{
		LED_PH=1;
		flag='N';
	}
	if(oxy>data[5]-3)
	{
		LED_OXY=0;
		FUN_Oxyg=1;
	}
	else
	{
		LED_OXY=1;
		FUN_Oxyg=0;
		flag='N';
	}
}

void RD_MON_1(char check,float tm1,float ph1,float oxy1)//�����׶ε�һ��ˮ��Ʒ��7���䣩
{
	float ph;
	float oxy;
	float temp;
	if (check=='1')
	{
		data[6]=tm1;
		data[7]=ph1;
		data[8]=oxy1;
	}
	PCF8574_ReadBit(BEEP_IO);
	temp=DS18B20_Get_Temp();
	oxy=Get_Adc_Average(ADC_CHANNEL_5,10)*0.0032;
	ph=Get_Adc_Average(ADC_CHANNEL_6,10)*0.0024;
	if((temp/10)>data[6]-2 && (temp/10)<=data[6]+2)		LED_TEMP=0;
	else
	{
		LED_TEMP=1;
		flag='N';
	}
	if(ph>data[7]-0.8 && ph<data[7]+0.8)	LED_PH=0;
	else
	{
		LED_PH=1;
		flag='N';
	}
	if(oxy>data[8]-4)	LED_OXY=0;
	else
	{
		LED_OXY=1;
		FUN_Oxyg=0;
		flag='N';
	}
}

void ST_MON_2(char check,float tm1,float ph1,float oxy1)//��һ�׶εĶ���ˮ��Ʒ��1���䣩
{
	float ph;
	float oxy;
	float temp;
	if (check=='1')
	{
		data[6]=tm1;
		data[7]=ph1;
		data[8]=oxy1;
	}
	PCF8574_ReadBit(BEEP_IO);
	temp=DS18B20_Get_Temp();
	oxy=Get_Adc_Average(ADC_CHANNEL_5,10)*0.0032;
	ph=Get_Adc_Average(ADC_CHANNEL_6,10)*0.0024;
	PCF8574_ReadBit(BEEP_IO);
	temp=DS18B20_Get_Temp();
	if((temp/10)>data[9]-2 && (temp/10)<=data[9]+2)		LED_TEMP=0;
	else
	{
		LED_TEMP=1;
		flag='N';
	}
	if(ph>data[10]-0.8 && ph<data[10]+0.8)	LED_PH=0;
	else
	{
		LED_PH=1;
		flag='N';
	}
	if(oxy>data[11]-4)	LED_OXY=0;
	else
	{
		LED_OXY=1;
		FUN_Oxyg=0;
		flag='N';
	}
}

void ND_MON_2(char check,float tm1,float ph1,float oxy1)//�ڶ��׶εĶ���ˮ��Ʒ��3���䣩
{
	float ph;
	float oxy;
	float temp;
	if (check=='1')
	{
		data[12]=tm1;
		data[13]=ph1;
		data[14]=oxy1;
	}
	PCF8574_ReadBit(BEEP_IO);
	temp=DS18B20_Get_Temp();
	oxy=Get_Adc_Average(ADC_CHANNEL_5,10)*0.0032;
	ph=Get_Adc_Average(ADC_CHANNEL_6,10)*0.0024;
	PCF8574_ReadBit(BEEP_IO);
	temp=DS18B20_Get_Temp();
	if((temp/10)>data[12]-2 && (temp/10)<=data[12]+2)		LED_TEMP=0;
	else
	{
		LED_TEMP=1;
		flag='N';
	}
	if(ph>data[13]-0.8 && ph<data[7]+0.8)	LED_PH=0;
	else
	{
		LED_PH=1;
		flag='N';
	}
	if(oxy>data[14]-3)	LED_OXY=0;
	else
	{
		LED_OXY=1;
		FUN_Oxyg=0;
		flag='N';
	}
}

void RD_MON_2(char check,float tm1,float ph1,float oxy1)//�����׶εĶ���ˮ��Ʒ��7���䣩
{
	float ph;
	float oxy;
	float temp;
	if (check=='1')
	{
		data[15]=tm1;
		data[16]=ph1;
		data[17]=oxy1;
	}
	PCF8574_ReadBit(BEEP_IO);
	temp=DS18B20_Get_Temp();
	oxy=Get_Adc_Average(ADC_CHANNEL_5,10)*0.0032;
	ph=Get_Adc_Average(ADC_CHANNEL_6,10)*0.0024;
	if((temp/10)>data[15]-2 && (temp/10)<=data[15]+2)		LED_TEMP=0;
	else
	{
		LED_TEMP=1;
		flag='N';
	}
	if(ph>data[16]-0.8 && ph<data[16]+0.8)	LED_PH=0;
	else
	{
		LED_PH=1;
		flag='N';
	}
	if(oxy>data[17]-4)	LED_OXY=0;
	else
	{
		LED_OXY=1;
		FUN_Oxyg=0;
		flag='N';
	}
}

void Function(void)//ÿ��һ��ʱ���Զ�Ͷιʳ�ʩ����������������������
{
	FUN_Fert=0;
	FUN_Food=0;
	delay_ms(5000);
	FUN_Food=1;
	FUN_Fert=1;
}

u8 Dire_Date[]={0x11,0x22,0x33,0x44,0x55};//����������
u8 date[30]={0};//��������
u8 Tran_Data[30]={0};//͸������

#define Dire_DateLen sizeof(Dire_Date)/sizeof(Dire_Date[0])
extern u32 obj_addr;//��¼�û�����Ŀ���ַ
extern u8 obj_chn;//��¼�û�����Ŀ���ŵ�

u8 wlcd_buff[10]={0}; //LCD��ʾ�ַ���������

void LoRa_SendData(int counter)//Loraģ�鷢������
{      
	float temperature=20;
	float ph=7.5;
	float oxy=5.2;
  u16 addr;
	u16 adcx;
	u16 adcy;
	u8 chn;
	u16 i=0;
	PCF8574_ReadBit(BEEP_IO);
	if(LoRa_CFG.mode_sta == LORA_STA_Tran)//͸������
	{
		temperature=DS18B20_Get_Temp();
		adcx=Get_Adc_Average(ADC_CHANNEL_5,10);//��ȡͨ��5��20��ȡƽ��������ת��ֵ
		adcy=Get_Adc_Average(ADC_CHANNEL_6,10);//��ȡͨ��6��20��ȡƽ������ת��ֵ
		oxy=adcx*0.012;
		ph=adcy*0.0050;
		sprintf((char*)Tran_Data,"%s-%.2f-%.2f-%.2f-%s-%d","STM_TWO",ph,temperature/10,oxy,"1",counter);
		u3_printf("%s",Tran_Data);
		LCD_Fill(0,195,240,220,WHITE);//�����ʾ
		Show_Str_Mid(10,195,Tran_Data,16,240);
	}
	else if(LoRa_CFG.mode_sta == LORA_STA_Dire)//������
	{
		addr = (u16)obj_addr;//Ŀ���ַ
		chn = obj_chn;//Ŀ���ŵ�
		date[i++] =(addr>>8)&0xff;//��λ��ַ
		date[i++] = addr&0xff;//��λ��ַ
		date[i] = chn;  //�����ŵ�
		for(i=0;i<Dire_DateLen;i++)//����д������BUFF
		{
			date[3+i] = Dire_Date[i];
		}
		for(i=0;i<(Dire_DateLen+3);i++)
		{
			while(__HAL_UART_GET_FLAG(&UART3_Handler,UART_FLAG_TXE)== RESET);//ѭ������,ֱ���������   
			HAL_UART_Transmit(&UART3_Handler,&date[i],1,1000);	
		}	
    //��ʮ�����Ƶ�����ת��Ϊ�ַ�����ӡ��lcd_buff����
		sprintf((char*)wlcd_buff,"%x %x %x %x %x %x %x %x",
		date[0],date[1],date[2],date[3],date[4],date[5],date[6],date[7]);	
		LCD_Fill(0,200,240,230,WHITE);//�����ʾ
		Show_Str_Mid(10,200,wlcd_buff,16,240);//��ʾ���͵�����		
	  Dire_Date[4]++;//Dire_Date[4]���ݸ���	
	}
}

u8 rlcd_buff[10]={0}; //LCD��ʾ�ַ���������

void LoRa_ReceData(void)//Loraģ���������
{
	u16 i=0;
	u16 len=0;
	//����������
	if(USART3_RX_STA&0x8000)
	{
		len = USART3_RX_STA&0X7FFF;
		USART3_RX_BUF[len]=0;//��ӽ�����
		USART3_RX_STA=0;

		for(i=0;i<len;i++)
		{
			while(__HAL_UART_GET_FLAG(&UART1_Handler,UART_FLAG_TXE)== RESET);//ѭ������,ֱ���������   
			HAL_UART_Transmit(&UART1_Handler,&USART3_RX_BUF[i],1,1000);	
		}
		LCD_Fill(10,260,240,320,WHITE);
		if(LoRa_CFG.mode_sta==LORA_STA_Tran)//͸������
		{
			if (USART3_RX_BUF[6]=='1')
			{
				Show_Str_Mid(10,270,USART3_RX_BUF,16,240);//��ʾ���յ�������
				Lora_analyze(USART3_RX_BUF);
			}
		}
		else if(LoRa_CFG.mode_sta==LORA_STA_Dire)//������
		{
			//��ʮ�����Ƶ�����ת��Ϊ�ַ�����ӡ��lcd_buff����
			sprintf((char*)rlcd_buff,"%x %x %x %x %x",USART3_RX_BUF[0],USART3_RX_BUF[1],USART3_RX_BUF[2],USART3_RX_BUF[3],USART3_RX_BUF[4]);
			Show_Str_Mid(10,270,rlcd_buff,16,240);//��ʾ���յ�������
			delay_ms(1000);
			Lora_analyze(rlcd_buff);
		}
		memset((char*)USART3_RX_BUF,0x00,len);//���ڽ��ջ�������0
	}
}


/*
�߼����£�
���ȵ�һλ���ֱ�ʾ��ǰ��Ҫ��ֳ���������ƣ��ڶ�λ��ʾ�����������׶Σ�����λ��ʾ�Ƿ����ˮ��ָ�꣬
ʣ����λ�ֱ��ʾ�µ�����PH���µ��������������µ�����ˮ���¶ȣ�����λ���ҽ�������λΪ��1��ʱ��Ч
*/
void Lora_analyze(u8* table)
{
	table[3]=table[3]-48;
	table[4]=table[4]-48;
	table[5]=table[5]-48;
	switch(table[0])
	{
		case '0':
			if(table[1]=='0')
			{
				Show_Str_Mid(10,290,"1���� �޷���",16,240);
				ST_MON_1(table[2],table[3],table[4],table[5]);
			}
			else if(table[1]=='1')
			{
				Show_Str_Mid(10,290,"3���� �޷���",16,240);
				ND_MON_1(table[2],table[3],table[4],table[5]);
			}
			else if(table[1]=='2')
			{
				Show_Str_Mid(10,290,"7���� �޷���",16,240);
				RD_MON_1(table[2],table[3],table[4],table[5]);
			}
			break;
		case '1':
			if(table[1]=='0')
			{
				Show_Str_Mid(10,290,"1���� ����",16,240);
				ST_MON_2(table[2],table[3],table[4],table[5]);
			}
			else if(table[1]=='1')
			{
				Show_Str_Mid(10,290,"3���� ����",16,240);
				ND_MON_2(table[2],table[3],table[4],table[5]);
			}
			else if(table[1]=='2')
			{
				Show_Str_Mid(10,290,"7���� ����",16,240);
				RD_MON_2(table[2],table[3],table[4],table[5]);
			}
			break;
		default:
			Show_Str_Mid(10,300,"ָ����󣡣���",18,100);
			break;
	}
}

//���ͺͽ��մ���
void LoRa_Process(void)
{
	u8 key=0;
	u8 t=0;
	int counter=1;
	short num=0;
	DATA:
	Process_ui();//������ʾ
	LoRa_Set();//LoRa����(�������������ô��ڲ�����Ϊ115200,) 
	while(1)
	{
		key = KEY_Scan(0);
		if(key==KEY0_PRES)
		{
			if(LoRa_CFG.mode_sta==LORA_STA_Dire)//���Ƕ�����,���������Ŀ���ַ���ŵ�����
			{
				usart3_rx(0);//�رմ��ڽ���
				Aux_Int(0);//�ر��ж�
				Dire_Set();//��������Ŀ���ַ���ŵ�
				goto DATA;
			}
		}
		else if(key==WKUP_PRES)//�������˵�ҳ��
		{
			LORA_MD0=1; //��������ģʽ
	    delay_ms(40);
			usart3_rx(0);//�رմ��ڽ���
			Aux_Int(0);//�ر��ж�
			break;
		}
		else if(key==KEY1_PRES)//��������
		{
			if(!LORA_AUX&&(LoRa_CFG.mode!=LORA_MODE_SLEEP))//�����ҷ�ʡ��ģʽ
			{
				while(1)
				{
					LoRa_ReceData();
					while(num<10)
					{
						Lora_mode=2;//���"����״̬"
						LoRa_SendData(counter);//��������
						delay_ms(1000);
						counter+=1;
						num+=1;
					}
					num=0;
				}
			}
		}
		t++;
		if(t==20)
		{
			t=0;
			LED1=~LED1;
		}
		delay_ms(10);
  }
}


//�����Ժ���
void Lora_Test(void)
{
	u8 t=0;
	u8 key=0;
	u8 netpro=0;
	LCD_Clear(WHITE);
	POINT_COLOR=RED;
	Show_Str_Mid(0,30,"SMART FISH POOL SYSTEM",16,240); 
	while(LoRa_Init())//��ʼ��ATK-LORA-01ģ��
	{
		Show_Str(40+30,50+20,200,16,"NO DEVICE DETECTED!!!",16,0); 	
		delay_ms(300);
		Show_Str(40+30,50+20,200,16,"BLANK",16,0);
	}
	Show_Str(40+30,50+20,200,16,"DEVICE DETECTED!!!",16,0);
  delay_ms(500); 	
	Menu_ui();//�˵�
	while(1)
	{
		key = KEY_Scan(0);
		if(key)
		{
			Show_Str(30+10,95+45+netpro*25,200,16,"  ",16,0);//���֮ǰ����ʾ
			if(key==KEY0_PRES)//KEY0����
			{
				if(netpro<6)netpro++;
				else netpro=0;
			}
			else if(key==KEY1_PRES)//KEY1����
			{
				if(netpro>0)netpro--;
				else netpro=6; 
			}
			else if(key==WKUP_PRES)//KEY_UP����
			{
				if(netpro==0)//����ͨ��ѡ��
				{
				  LoRa_Process();//��ʼ���ݲ���
				  netpro=0;//�������ص�0
				  Menu_ui();
				}
				else
				{
					Show_Str(30+40,95+45+netpro*25+2,200,16,"________",16,1);//��ʾ�»���,��ʾѡ��
					Show_Str(30+10,95+45+netpro*25,200,16,"-->",16,0);//ָ������Ŀ
					Menu_cfg(netpro);//��������
					LCD_Fill(30+40,95+45+netpro*25+2+15,30+40+100,95+45+netpro*25+2+18,WHITE);//����»�����ʾ
				}
			}
			Show_Str(30+10,95+45+netpro*25,200,16,"-->",16,0);//ָ������Ŀ
		}
		t++;
		if(t==30)
		{
			t=0;
			LED1=~LED1;
		}
		delay_ms(10);
	}	
}
