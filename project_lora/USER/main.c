#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "usmart.h" 
#include "lcd.h"
#include "string.h"
#include "sdram.h"
#include "malloc.h"
#include "w25qxx.h"
#include "ff.h"
#include "exfuns.h"
#include "sdio_sdcard.h"
#include "fontupd.h"
#include "text.h"
#include "touch.h"	
#include "usart3.h"
#include "lora_app.h"
#include "ds18b20.h"
#include "pcf8574.h"
#include "adc.h"

int main(void)
{	
	u8 key,fontok=0;
	HAL_Init();                         //��ʼ��HAL��   
	Stm32_Clock_Init(360,25,2,8);       //����ʱ��,180Mhz
	delay_init(180);                    //��ʼ����ʱ����
	uart_init(115200);                  //��ʼ��USART
	usmart_init(90);                    //��ʼ��USMART
	LED_Init();                         //��ʼ��LED 
	KEY_Init();                         //��ʼ������
	SDRAM_Init();                       //SDRAM��ʼ��
	LCD_Init();                         //LCD��ʼ��
	W25QXX_Init();				         			//��ʼ��W25Q256
	tp_dev.init();				         			//��ʼ��������
	my_mem_init(SRAMIN);                //��ʼ���ڲ��ڴ��
	my_mem_init(SRAMEX);                //��ʼ���ⲿSDRAM�ڴ��
	my_mem_init(SRAMCCM);               //��ʼ���ڲ�CCM�ڴ��
	exfuns_init();		                  //Ϊfatfs��ر��������ڴ�
	MY_ADC_Init();                 			//��ʼ��ADC1ͨ��5 PA5  ͨ��8 PB0
	PCF8574_Init();
	f_mount(fs[0],"0:",1);              //����SD�� 
	f_mount(fs[1],"1:",1);              //����SPI FLASH.  
	key=KEY_Scan(0);  
	if(key==KEY0_PRES)		             	//ǿ��У׼
	{
		LCD_Clear(WHITE);	             		//����
		TP_Adjust();  		             		//��ĻУ׼ 
		TP_Save_Adjdata();	  
		LCD_Clear(WHITE);	             		//����
	}
	fontok=font_init();		              //����ֿ��Ƿ�OK
	if(fontok||key==KEY1_PRES)          //��Ҫ�����ֿ�				 
	{
		LCD_Clear(WHITE);		       //����
		POINT_COLOR=BLUE;			     //��������Ϊ��ɫ	   	   	  
		LCD_ShowString(60,50,200,16,16,"ALIENTEK STM32");
		while(SD_Init())			     //���SD��
		{
			LCD_ShowString(60,70,200,16,16,"SD Card Failed!");
			delay_ms(200);
			LCD_Fill(60,70,200+60,70+16,WHITE);
			delay_ms(200);		    
		}								 						    
		LCD_ShowString(60,70,200,16,16,"SD Card OK");
		LCD_ShowString(60,90,200,16,16,"Font Updating...");
		key=update_font(20,110,16,"0:"); //��SD������
		while(key)                       //����ʧ��		
		{			 		  
			LCD_ShowString(60,110,200,16,16,"Font Update Failed!");
			delay_ms(200);
			LCD_Fill(20,110,200+20,110+16,WHITE);
			delay_ms(200);		       
		} 		  
		LCD_ShowString(60,110,200,16,16,"Font Update Success!");
		delay_ms(1500);	
		LCD_Clear(WHITE);                //����	       
	} 
	Lora_Test();//lora����
}
