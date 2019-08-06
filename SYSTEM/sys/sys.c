#include "sys.h"


void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;					//Priority7 USART1  调试口
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;					//Priority0 USART2  NB模块
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;						//Priority4 9600bps 定时器  USART、DALI
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); 
}


//看门狗初始化
void IWDG_Init(u8 prer,u16 rlr)
{	
 	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	
	IWDG_SetPrescaler(prer);	//设置IWDG预分频值
	
	IWDG_SetReload(rlr);		//设置IWDG重装载值
	
	IWDG_ReloadCounter();		//按照IWDG重装载寄存器的值重新装载IWDG计数器
	
	IWDG_Enable();
}
//喂狗
void IWDG_Feed(void)
{   
 	IWDG_ReloadCounter();									   
}

void WFI_SET(void)
{
	__ASM volatile("wfi");		  
}
//¹Ø±ÕËùÓÐÖÐ¶Ï
void INTX_DISABLE(void)
{		  
	__ASM volatile("cpsid i");
}
//¿ªÆôËùÓÐÖÐ¶Ï
void INTX_ENABLE(void)
{
	__ASM volatile("cpsie i");		  
}
//设置栈顶地址
//addr:栈顶地址
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}






