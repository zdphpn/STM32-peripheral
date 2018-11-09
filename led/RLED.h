
#ifndef _RLED_H
#define _RLED_H


#include "stm32f10x.h"


#define RLED_USEPWM    1                               //是否使用PWM

#define RLED_GPIOx     GPIOB
#define RLED_Pinx      GPIO_Pin_0

#define RLED_TIMx      TIM3
#if RLED_USEPWM==1
#define RLED_CHx       3
#endif

#define RLED_RCC_APBxPeriphClockCmd	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOB,ENABLE);\
                                    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE)


void RLED_Init(void);                                   //RLED初始化
                                                        //RLED开
#if RLED_USEPWM==1
void RLED_Open(uint16_t hz, uint16_t dut, uint16_t t_c);
#else
void RLED_Open(uint16_t ms);
#endif
void RLED_Close(void);                                  //RLED关
void RLED_TIMxInt(void);                                //RLED定时中断



#endif


