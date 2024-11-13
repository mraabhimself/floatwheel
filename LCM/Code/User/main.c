/**
  ******************************************************************************
  * @file       �� main.c
  * @author     :  FCZ
  * @version    �� V1.1.6
  * @date       �� 2022��03��28��
  * @brief      �� ������ - LCM�ƿ�
  * @mcu        :  HK32F030MF4P6
  ******************************************************************************/
/*----------------------------------------------------------------------------
  ������־:
  2022-03-28 V1.0.0:��ʼ�汾
  2022-05-16 ���ӿ�����ʾ�������ͳ����ʾ��ѹ
  2022-05-17 ���ĳ����ʾΪ������Ч��
  2022-06-09 �ػ�ʱ����5���Ӹ�Ϊ15���� 
             ռ�ձ��ɳ���70%��Ϊ����80%���������١����١�����
			 �Զ��ػ�������ԭ���Ĳ��Ƚ�̤������һ����ת�ٵ���1000�ſ�ʼ��ʱ�ػ�
  2022-07-19 �����12����Ϊ20��
             �ػ�ʱ���Ϊ30����
			 �������ѹ����ƽ��ֵ�˲�����ѹ�жϼӻ�ִ��ѹ��Χ
  2023-01-16 WS2812������Ӳ��SPIģ���ΪIO�ڼ���ʱģ��
  ----------------------------------------------------------------------------*/
#include "hk32f030m.h"
#include "led.h"
#include "time.h"
#include "ws2812.h"
#include "power.h"
#include "power_button.h"
#include "usart.h"
#include "adc.h"
#include "buzzer.h"
#include "led_pwm.h"
#include "task.h"

/**************************************************
 * @brie   :main()
 * @note   :������
 * @param  :��
 * @retval :��
 **************************************************/
int main(void)
{	
	//RCC_GetClocksFreq(&RCC_Clock);
	LED_Init();
#ifdef USE_BUZZER		
	Buzzer_Init();
#endif
	ADC1_Init();
	WS2812_Init();
	Power_Init();
	initializePowerButton();
	USART1_Init(115200);
	LED_PWM_Init();
	Time6_Init();

	if(POWER_BUTTON ==  BUTTON_PRESSED) {
		powerButtonState = SINGLE_PRESS;
	}

	while(1)
	{
		PowerButtonTask();
		
		if(WS2812_Counter >= 20) // 20ms refresh period
		{
			WS2812_Task();
			WS2812_Counter = 0;
		}

		Power_Task();

#ifdef ADV
		Charge_Task();
#endif
		Headlights_Task();
#ifdef USE_BUZZER		
		Buzzer_Task();
#endif
		Usart_Task();
		ADC_Task();
		VESC_State_Task();
	}
	return 0;
}
