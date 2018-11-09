
#include "cUART.h"




uint8_t cUART_RecvData[256] = {0x00};                            //串口接收到的数据
uint16_t cUART_RecvDataLen = 0;                                  //数据长度


/*
功能：	所用串口初始化
参数：	无
返回：	无
*/
void cUART_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;

    c_RCC_APBxPeriphClockCmd;                                    //时钟

    GPIO_InitStructure.GPIO_Pin = cUARTRx_Pinx;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;                //上拉输入
    GPIO_Init(cUARTRx_GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = cUARTTx_Pinx;                  //复用输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(cUARTTx_GPIOx, &GPIO_InitStructure);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    NVIC_InitStructure.NVIC_IRQChannel = cUARTx_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;    //优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;                 //串口参数
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(cUARTx, &USART_InitStructure);
    USART_Cmd(cUARTx, ENABLE);

    USART_SendData(cUARTx, 0x00);
    while(USART_GetFlagStatus(cUARTx, USART_FLAG_TC) != SET);

    USART_ITConfig(cUARTx, USART_IT_IDLE, ENABLE);

    DMA_DeInit(cRx_DMAx_Channelx);                               //接收DMA参数
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(cUARTx->DR));
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(cUART_RecvData);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = sizeof(cUART_RecvData);
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(cRx_DMAx_Channelx, &DMA_InitStructure);

    DMA_DeInit(cTx_DMAx_Channelx);                               //发送DMA参数
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(cUARTx->DR));

    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(cUART_RecvData);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = 0;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(cTx_DMAx_Channelx, &DMA_InitStructure);

                                                                 //启动DMA接收
    USART_DMACmd(cUARTx, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);
    DMA_Cmd(cRx_DMAx_Channelx, ENABLE);
    DMA_Cmd(cTx_DMAx_Channelx, DISABLE);

}

/*
功能：  发送数据
参数1： 数据地址
参数2： 数据长度
返回：  无
*/
void cUART_SendData(uint32_t addr, uint16_t len)
{
    if(len == 0)return;
    while(DMA_GetCurrDataCounter(cTx_DMAx_Channelx));            //DMA空闲

    DMA_Cmd(cTx_DMAx_Channelx, DISABLE);
    cTx_DMAx_Channelx->CMAR = addr;                              //设置源地址
    cTx_DMAx_Channelx->CNDTR = len;                              //设置长度
    DMA_Cmd(cTx_DMAx_Channelx, ENABLE);                          //启动发送

}

/*
功能：  所用串口中断调用
参数：  无
返回：  无
*/
void cUART_Int(void)
{
    if(USART_GetITStatus(cUARTx, USART_IT_IDLE) != RESET)        //空闲中断,一帧数据接收完
    {
        cUARTx->DR;
        cUARTx->SR;                                              //清标识

        cUART_RecvDataLen = sizeof(cUART_RecvData) - DMA_GetCurrDataCounter(cRx_DMAx_Channelx); //计算接收到的数据长度

        DMA_Cmd(cRx_DMAx_Channelx, DISABLE);
        DMA_SetCurrDataCounter(cRx_DMAx_Channelx, sizeof(cUART_RecvData)); //恢复DMA
        DMA_Cmd(cRx_DMAx_Channelx, ENABLE);
    }
}


