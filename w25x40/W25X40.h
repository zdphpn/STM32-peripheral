#ifndef _W25X40_H
#define _W25X40_H

/*
�ļ���;:           STM32����W25X40
����:               �Ŷ���
����ʱ��:           2018/07/04
����ʱ��:           2018/07/04
�汾:               V1.0

*/



#include "stm32f10x.h"


#define W25X_SPIMOD         1                                    //1Ӳ��SPI,0���ģ��SPI

#if W25X_SPIMOD==1
#define W25X_SPIx           SPI3
#endif
/*���Ŷ���*/
//SPIͨ�Žӿ�
#define W25X_CLK_GPIOx      GPIOB
#define W25X_CLK_Pinx       GPIO_Pin_3
#define W25X_MOSI_GPIOx     GPIOB
#define W25X_MOSI_Pinx      GPIO_Pin_5
#define W25X_MISO_GPIOx     GPIOB
#define W25X_MISO_Pinx      GPIO_Pin_4
#define W25X_NSS_GPIOx      GPIOA
#define W25X_NSS_Pinx       GPIO_Pin_15

//д����
#define W25X_WP_GPIOx       GPIOB
#define W25X_WP_Pinx        GPIO_Pin_11

//ʱ��,�ǵð�ʹ�õ�����ʱ�Ӷ�д��
#define W25X_RCC_APBxPeriphClockCmd	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO,ENABLE);\
                                    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3,ENABLE);\
                                    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE)
/*���Ŷ���*/



uint8_t W25X_Init(void);                                         //��ʼ��

void W25X_EraseChip(void);                                       //��Ƭ����
void W25X_EraseSector(uint32_t addr);                            //��������(W25X40Ϊ4K)

void W25X_Write(uint32_t addr, uint8_t *dat, uint16_t len);      //д
void W25X_Read(uint32_t addr, uint8_t *dat, uint16_t len);       //��




#endif

