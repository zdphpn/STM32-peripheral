
#ifndef _UARTx_H
#define _UARTx_H


#include "stm32f10x.h"



#define cUARTx                   UART4
#define cUARTx_IRQn              UART4_IRQn
#define cUARTTx_GPIOx            GPIOC
#define cUARTTx_Pinx             GPIO_Pin_10
#define cUARTRx_GPIOx            GPIOC
#define cUARTRx_Pinx             GPIO_Pin_11


#define cTx_DMAx_Channelx        DMA2_Channel5
#define cRx_DMAx_Channelx        DMA2_Channel3

#define c_RCC_APBxPeriphClockCmd RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOC,ENABLE);\
                                 RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);\
                                 RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2,ENABLE)



extern uint8_t cUART_RecvData[256];                             //串口接收到的数据
extern uint16_t cUART_RecvDataLen;                              //数据长度


void cUART_Init(void);                                          //串口初始化
void cUART_SendData(uint32_t addr, uint16_t len);               //发送数据
void cUART_Int(void);                                           //接收数据



#endif


