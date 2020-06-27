#include "usart3.h"
#include "delay.h"
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"	  
#include "timer.h"
#include "lora_cfg.h"
#include "lora_app.h"


//���ڷ��ͻ����� 	
__align(8) u8 USART3_TX_BUF[USART3_MAX_SEND_LEN]; 	      //���ͻ���,���USART3_MAX_SEND_LEN�ֽ�
#ifdef USART3_RX_EN   								      //���ʹ���˽���   	  
//���ڽ��ջ����� 	
u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; 				      //���ջ���,���USART3_MAX_RECV_LEN���ֽ�.

UART_HandleTypeDef UART3_Handler;                         //UART���

//��ʼ��IO,����3
//bound:������
void usart3_init(u32 bound)
{	
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_Initure;

	__HAL_RCC_GPIOB_CLK_ENABLE();			              //ʹ��GPIOBʱ��
	__HAL_RCC_USART3_CLK_ENABLE();			              //ʹ��USART1ʱ��

	//UART ��ʼ������
	UART3_Handler.Instance=USART3;					      //USART3
	UART3_Handler.Init.BaudRate=bound;				      //������
	UART3_Handler.Init.WordLength=UART_WORDLENGTH_8B;     //�ֳ�Ϊ8λ���ݸ�ʽ
	UART3_Handler.Init.StopBits=UART_STOPBITS_1;	      //һ��ֹͣλ
	UART3_Handler.Init.Parity=UART_PARITY_NONE;		      //����żУ��λ
	UART3_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;     //��Ӳ������
	UART3_Handler.Init.Mode=UART_MODE_TX_RX;		      //�շ�ģʽ
	HAL_UART_Init(&UART3_Handler);					      //HAL_UART_Init()��ʹ��UART3	

	GPIO_Initure.Pin=GPIO_PIN_10|GPIO_PIN_11;			  //PB10��PB11
	GPIO_Initure.Mode=GPIO_MODE_AF_PP;		              //�����������
	GPIO_Initure.Pull=GPIO_PULLUP;			              //����
	GPIO_Initure.Speed=GPIO_SPEED_FAST;		              //����
	GPIO_Initure.Alternate=GPIO_AF7_USART3;	              //����ΪUSART3
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);	            	  //��ʼ��PB10,��PB11
	
	__HAL_UART_DISABLE_IT(&UART3_Handler,UART_IT_TC);
#if USART3_RX_EN 
	__HAL_UART_ENABLE_IT(&UART3_Handler,UART_IT_RXNE);    //���������ж�
	HAL_NVIC_EnableIRQ(USART3_IRQn);				      //ʹ��USART1�ж�ͨ��
	HAL_NVIC_SetPriority(USART3_IRQn,2,3);			      //��ռ���ȼ�2�������ȼ�3
#endif	
	TIM7_Init(100-1,9000-1);                              //��������Ϊ10ms�ж�	
	HAL_TIM_Base_Stop(&TIM7_Handler);                     //�رն�ʱ��7
	USART3_RX_STA=0;				                      //���� 
	
  
}

//timer=10ms
//ͨ���жϽ�������2���ַ�֮���ʱ������timer�������ǲ���һ������������.
//���2���ַ����ռ������timer,����Ϊ����1����������.Ҳ���ǳ���timerû�н��յ�
//�κ�����,���ʾ�˴ν������.
//���յ�������״̬
//[15]:0,û�н��յ�����;1,���յ���һ������.
//[14:0]:���յ������ݳ���
u16 USART3_RX_STA=0;  

//����3�жϷ������
void USART3_IRQHandler(void)
{
	  u8 Res;
	  if((__HAL_UART_GET_FLAG(&UART3_Handler,UART_FLAG_RXNE)!=RESET))
	  {
		  HAL_UART_Receive(&UART3_Handler,&Res,1,1000);
		  if((USART3_RX_STA&0x8000)==0)                //�������һ������,��û�б�����,���ٽ�����������
		  { 
			 if(USART3_RX_STA<USART3_MAX_RECV_LEN)     //�����Խ�������
			 {
				 
				 if(!Lora_mode)//����ģʽ��(������ʱ����ʱ)
				 {
					  TIM7->CNT = 0;                       //���������
					  if(USART3_RX_STA==0)                 //���û�н����κ�����
					  {
						 HAL_TIM_Base_Start(&TIM7_Handler);//������ʱ��7 
					  }
					 
				 }
				 USART3_RX_BUF[USART3_RX_STA++]=Res;  //��¼���յ���ֵ
				  
			 }
			 else
			 {
				  USART3_RX_STA|=1<<15;			       //ǿ�Ʊ�ǽ������
			 }
		  }
	  }
//	  HAL_UART_IRQHandler(&UART3_Handler);	
}

//����3,printf ����
//ȷ��һ�η������ݲ�����USART3_MAX_SEND_LEN�ֽ�
void u3_printf(char* fmt,...)  
{  
	u16 i,j;
	va_list ap;
	va_start(ap,fmt);
	vsprintf((char*)USART3_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)USART3_TX_BUF);//�˴η������ݵĳ���
	for(j=0;j<i;j++)                     //ѭ����������
	{
		while((USART3->SR&0X40)==0);     //ѭ������,ֱ���������   
		USART3->DR=USART3_TX_BUF[j];  
	}
}

//����3�����ʺ�У��λ����
//bps:�����ʣ�1200~115200��
//parity:У��λ���ޡ�ż���棩
void usart3_set(u8 bps,u8 parity)
{
  static u32 bound=0;
	switch(bps)
	{
		case LORA_TTLBPS_1200:   bound=1200;     break;
		case LORA_TTLBPS_2400:   bound=2400;     break;
		case LORA_TTLBPS_4800:   bound=4800;     break;
		case LORA_TTLBPS_9600:   bound=9600;     break;
		case LORA_TTLBPS_19200:  bound=19200;    break;
		case LORA_TTLBPS_38400:  bound=38400;    break;
		case LORA_TTLBPS_57600:  bound=57600;    break;
		case LORA_TTLBPS_115200: bound=115200;   break;
	}
    
   
	__HAL_UART_DISABLE(&UART3_Handler);//�رմ���	
	UART3_Handler.Init.BaudRate = bound;
	UART3_Handler.Init.StopBits = UART_STOPBITS_1; 
	
	if(parity==LORA_TTLPAR_8N1)//��У��
	{
		UART3_Handler.Init.WordLength= UART_WORDLENGTH_8B;    
		UART3_Handler.Init.Parity = UART_PARITY_NONE;
	}else if(parity==LORA_TTLPAR_8E1)//żУ��
	{
		UART3_Handler.Init.WordLength= UART_WORDLENGTH_9B;    
		UART3_Handler.Init.Parity = UART_PARITY_EVEN;
	}else if(parity==LORA_TTLPAR_8O1)//��У��
	{
		UART3_Handler.Init.WordLength = UART_WORDLENGTH_9B;    
		UART3_Handler.Init.Parity = UART_PARITY_ODD;
	}
    HAL_UART_Init(&UART3_Handler);	
	
} 

//���ڽ���ʹ�ܿ���
//enable:0,�ر� 1,��
void usart3_rx(u8 enable)
{
	
	 __HAL_UART_DISABLE(&UART3_Handler); //ʧ�ܴ��� 
	
	 if(enable)
	 {
		 UART3_Handler.Init.Mode=UART_MODE_TX_RX;//�շ�ģʽ
	 }else
	 {
		  UART3_Handler.Init.Mode = UART_MODE_TX;//ֻ����  
	 }
	 
	 HAL_UART_Init(&UART3_Handler); //HAL_UART_Init()��ʹ��UART3
	
}

#endif













