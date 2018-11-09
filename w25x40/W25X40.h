#ifndef _W25X40_H
#define _W25X40_H

/*
文件用途:           STM32驱动W25X40
作者:               张栋培
创建时间:           2018/07/04
更新时间:           2018/07/04
版本:               V1.0

*/



#include "stm32f10x.h"


#define W25X_SPIMOD         1                                    //1硬件SPI,0软件模拟SPI

#if W25X_SPIMOD==1
#define W25X_SPIx           SPI3
#endif
/*引脚定义*/
//SPI通信接口
#define W25X_CLK_GPIOx      GPIOB
#define W25X_CLK_Pinx       GPIO_Pin_3
#define W25X_MOSI_GPIOx     GPIOB
#define W25X_MOSI_Pinx      GPIO_Pin_5
#define W25X_MISO_GPIOx     GPIOB
#define W25X_MISO_Pinx      GPIO_Pin_4
#define W25X_NSS_GPIOx      GPIOA
#define W25X_NSS_Pinx       GPIO_Pin_15

//写保护
#define W25X_WP_GPIOx       GPIOB
#define W25X_WP_Pinx        GPIO_Pin_11

//时钟,记得把使用的外设时钟都写上
#define W25X_RCC_APBxPeriphClockCmd	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO,ENABLE);\
                                    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3,ENABLE);\
                                    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE)
/*引脚定义*/



uint8_t W25X_Init(void);                                         //初始化

void W25X_EraseChip(void);                                       //整片擦除
void W25X_EraseSector(uint32_t addr);                            //扇区擦除(W25X40为4K)

void W25X_Write(uint32_t addr, uint8_t *dat, uint16_t len);      //写
void W25X_Read(uint32_t addr, uint8_t *dat, uint16_t len);       //读




#endif

