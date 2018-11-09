
#include "MADC.h"




#define MADC_ChannelNum          2                          //ADC采集通道总数

GPIO_TypeDef *MADC_GPIOx[MADC_ChannelNum] =
{
    GPIOC,
    GPIOC
};                                                          //所用接口定义,数组方便扩展

uint16_t MADC_Pinx[MADC_ChannelNum] =
{
    GPIO_Pin_1,
    GPIO_Pin_1
};                                                          //所用接口定义,数组方便扩展

#define MADC_ADCx                ADC1
uint8_t MADC_CHx[MADC_ChannelNum] =
{
    ADC_Channel_11,
    ADC_Channel_11
};                                                          //ADC通道号,数组方便扩展

#define MADC_DR_Address  ((uint32_t)0x4001244c);            //ADC数据寄存器地址
#define MADC_DMAx_Channelx      DMA1_Channel1               //所用DMA通道号
//时钟
#define MADC_RCC_APBxPeriphClockCmd RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_ADC1,ENABLE);\
                                    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE)





static uint16_t MADCValue[MADC_ChannelNum];                 //AD测量值存放

/*
功能：	电压测量初始化
参数：	无
返回：	无
*/
void MADC_Init(void)
{
    uint8_t i;

    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;

    MADC_RCC_APBxPeriphClockCmd;

    for(i = 0; i < MADC_ChannelNum; i++)                    //IO接口
    {
        GPIO_InitStructure.GPIO_Pin = MADC_Pinx[i];
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;       //模拟输入
        GPIO_Init(MADC_GPIOx[i], &GPIO_InitStructure);
    }

    DMA_DeInit(MADC_DMAx_Channelx);                         //DMA参数
    DMA_InitStructure.DMA_PeripheralBaseAddr = MADC_DR_Address;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)MADCValue;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = MADC_ChannelNum;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(MADC_DMAx_Channelx, &DMA_InitStructure);

    DMA_Cmd(MADC_DMAx_Channelx, ENABLE);

    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
    ADC_DeInit(MADC_ADCx);                                  //AD参数
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = MADC_ChannelNum;
    ADC_Init(MADC_ADCx, &ADC_InitStructure);

    for(i = 0; i < MADC_ChannelNum; i++)                    //采样周期,分配虚拟通道号
    {
        ADC_RegularChannelConfig(MADC_ADCx, MADC_CHx[i], i + 1, ADC_SampleTime_239Cycles5);
    }

    ADC_DMACmd(MADC_ADCx, ENABLE);
    ADC_Cmd(MADC_ADCx, ENABLE);

    ADC_ResetCalibration(MADC_ADCx);                        //ADC校准
    while(ADC_GetResetCalibrationStatus(MADC_ADCx));
    ADC_StartCalibration(MADC_ADCx);
    while(ADC_GetCalibrationStatus(MADC_ADCx));

    ADC_SoftwareStartConvCmd(MADC_ADCx, ENABLE);            //启动转换
}

/*
功能：	获取测量的电压值
参数：	虚拟通道号
返回：	测量值
*/
uint16_t MADC_GetValue(uint8_t index)
{
    if(index >= MADC_ChannelNum)
    {
        return 0;
    }
    return MADCValue[index];
}








