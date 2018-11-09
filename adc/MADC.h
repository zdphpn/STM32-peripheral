
#ifndef _MADC_H
#define _MADC_H



#include "stm32f10x.h"


void MADC_Init(void);
uint16_t MADC_GetValue(uint8_t index);



#endif


