#include "timer.h"
#include "usmart.h"

extern vu16 USART3_RX_STA;

TIM_HandleTypeDef TIM7_Handler; //��ʱ����� 

//��ʱ��7����Ԥװ������ֵ
void TIM7_SetARR(u16 period)
{
	 TIM7->CNT = 0;     //���������
	 TIM7->ARR&=0x00;   //����Ԥװ������ֵΪ0
	 TIM7->ARR|= period;//����Ԥװ������ֵ 
}

//ͨ�ö�ʱ��7�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��7!(��ʱ��7����APB1�ϣ�ʱ��ΪHCLK/2)
void TIM7_Init(u16 arr,u16 psc)
{  
	__HAL_RCC_TIM7_CLK_ENABLE();                           //ʹ��TIM7ʱ��
    TIM7_Handler.Instance=TIM7;                            //������ʱ��7
    TIM7_Handler.Init.Prescaler=psc;                       //��Ƶϵ��
    TIM7_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;      //���ϼ�����
    TIM7_Handler.Init.Period=arr;                          //�Զ�װ��ֵ
    TIM7_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//ʱ�ӷ�Ƶ����
    HAL_TIM_Base_Init(&TIM7_Handler);
    
    HAL_TIM_Base_Start_IT(&TIM7_Handler);                  //ʹ�ܶ�ʱ��7�Ͷ�ʱ��7�����жϣ�TIM_IT_UPDATE  

	HAL_NVIC_SetPriority(TIM7_IRQn,0,1);                   //�����ж����ȼ�����ռ���ȼ�0�������ȼ�1
	HAL_NVIC_EnableIRQ(TIM7_IRQn);                         //����ITM7�ж�  	
}

//��ʱ��7�жϷ�����
void TIM7_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM7_Handler);
}

//�ص���������ʱ���жϷ���������
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{ 

    if(htim==(&TIM7_Handler))//��ʱ��7�жϷ���������(GSM����)
    {
	   USART3_RX_STA|=1<<15;	           //��ǽ������
	   HAL_TIM_Base_Stop(&TIM7_Handler);//�رն�ʱ��7
		
    }
	if(htim==(&TIM4_Handler))//��ʱ��4�жϷ���������(UAMART����)
    {
        usmart_dev.scan();	                        //ִ��usmartɨ��
        __HAL_TIM_SET_COUNTER(&TIM4_Handler,0);;    //��ն�ʱ����CNT
        __HAL_TIM_SET_AUTORELOAD(&TIM4_Handler,100);//�ָ�ԭ��������
    }
}

