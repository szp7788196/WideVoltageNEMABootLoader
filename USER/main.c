#include "common.h"
#include "24cxx.h"
#include "led.h"
#include "rtc.h"
#include "usart.h"
#include "pwm.h"
#include "dac.h"
#include "inventr.h"
#include "cd4051b.h"
#include "ota.h"

//u16 i = 0;
//u8 j = 0;
//u8 eepbuf[256];
//RCC_ClocksTypeDef RCC_Clocks;

int main(void)
{
	IWDG_Init(IWDG_Prescaler_128,1600);	//128分频 312.5HZ 625为2秒

//	RCC_GetClocksFreq(&RCC_Clocks);		//查看各个总线的时钟频率
	__set_PRIMASK(1);	//关闭全局中断

	NVIC_Configuration();
	RELAY_Init();
	delay_init();
//	RTC_Init();
	AT24CXX_Init();
	DAC1_Init();
	LED_Init();
	TIM2_Init(99,7199);
	USART1_Init(115200);
	USART2_Init(9600);

	__set_PRIMASK(0);	//开启全局中断

//	AT24CXX_WriteOneByte(EC1_ADD,0);

//	for(i = 0; i < 256; i ++)
//	{
//		AT24CXX_WriteOneByte(i,i);
//	}
//	for(i = 0; i < 256; i ++)
//	{
//		eepbuf[i] = AT24CXX_ReadOneByte(i);
//	}
//	AT24CXX_WriteOneByte(UU_ID_ADD,255);

//	FrameWareState.state 			= FIRMWARE_DOWNLOADED;
//	WriteFrameWareStateToEeprom();			//将默认值写入EEPROM

	IWDG_Feed();				//喂看门狗

	SetLightLevel(100);

	while(1)
	{
		CopyJumpToAppAddress();

		delay_ms(1000);
	}
}

























