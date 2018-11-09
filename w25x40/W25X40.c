
#include "W25X40.h"


/*
文件用途:           STM32驱动W25X40
作者:               张栋培
创建时间:           2018/07/04
更新时间:           2018/07/04
版本:               V1.0

历史版本:           V1.0:基于STM32 SPI/IO口驱动W25X40


*/



#define W25X_PageSize         256                                //页缓存


/*命令列表*/
#define W25X_WriteEnable      0x06
#define W25X_WriteDisable     0x04
#define W25X_ReadStatusReg    0x05
#define W25X_WriteStatusReg   0x01
#define W25X_ReadData         0x03
#define W25X_FastReadData     0x0B
#define W25X_FastReadDual     0x3B
#define W25X_PageProgram      0x02
#define W25X_BlockErase       0xD8
#define W25X_SectorErase      0x20
#define W25X_ChipErase        0xC7
#define W25X_PowerDown        0xB9
#define W25X_ReleasePowerDown 0xAB
#define W25X_DeviceID         0xAB
#define W25X_ManufactDeviceID 0x90
#define W25X_JedecDeviceID    0x9F


#if W25X_SPIMOD==0
/*模拟SPI引脚状态切换定义*/
#define W25X_CLK_H  GPIO_SetBits(W25X_CLK_GPIOx,W25X_CLK_Pinx)
#define W25X_CLK_L  GPIO_ResetBits(W25X_CLK_GPIOx,W25X_CLK_Pinx)
#define W25X_MOSI_H GPIO_SetBits(W25X_MOSI_GPIOx,W25X_MOSI_Pinx)
#define W25X_MOSI_L GPIO_ResetBits(W25X_MOSI_GPIOx,W25X_MOSI_Pinx)

#define W25X_MISO   GPIO_ReadInputDataBit(W25X_MISO_GPIOx,W25X_MISO_Pinx)

/*模拟SPI引脚状态切换定义*/
#endif

/*片选,L选择*/
#define W25X_NSS_H GPIO_SetBits(W25X_NSS_GPIOx,W25X_NSS_Pinx)
#define W25X_NSS_L GPIO_ResetBits(W25X_NSS_GPIOx,W25X_NSS_Pinx)
/*写保护,H可以写入*/
#define W25X_WP_H  GPIO_SetBits(W25X_WP_GPIOx,W25X_WP_Pinx)
#define W25X_WP_L  GPIO_ResetBits(W25X_WP_GPIOx,W25X_WP_Pinx)




static void SPI_FLASH_SendByte(uint8_t byte);
static uint8_t SPI_FLASH_ReadByte(void);


/*
功能：  W25X40芯片初始化
参数：  无
返回：  1成功，0失败
*/
uint8_t W25X_Init(void)
{
    uint32_t id = 0;

#if W25X_SPIMOD==1
    SPI_InitTypeDef SPI_InitStructure;
#endif
    GPIO_InitTypeDef GPIO_InitStructure;

    W25X_RCC_APBxPeriphClockCmd;

#if W25X_SPIMOD==0
    GPIO_InitStructure.GPIO_Pin = W25X_CLK_Pinx;                 //CLK引脚输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(W25X_CLK_GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = W25X_MOSI_Pinx;                //MOSI引脚输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(W25X_MOSI_GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = W25X_MISO_Pinx;                //MISO引脚输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(W25X_MISO_GPIOx, &GPIO_InitStructure);

#else
    GPIO_InitStructure.GPIO_Pin = W25X_CLK_Pinx;                 //CLK引脚复用
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(W25X_CLK_GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = W25X_MOSI_Pinx;                //MOSI引脚复用
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(W25X_MOSI_GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = W25X_MISO_Pinx;                //MISO引脚复用
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(W25X_MISO_GPIOx, &GPIO_InitStructure);

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32 ;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(W25X_SPIx, &SPI_InitStructure);

    SPI_Cmd(W25X_SPIx, ENABLE);                                  //SPI初始化
#endif

    GPIO_InitStructure.GPIO_Pin = W25X_NSS_Pinx;                 //NSS引脚输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(W25X_NSS_GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = W25X_WP_Pinx;                  //WP引脚输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(W25X_WP_GPIOx, &GPIO_InitStructure);

    W25X_NSS_H;                                                  //片选拉高,默认不使能读写
    W25X_WP_L;                                                   //写保护拉低,默认不能写入

    W25X_NSS_L;
    SPI_FLASH_SendByte(W25X_DeviceID);                           //读取ID命令
    SPI_FLASH_SendByte(0xFF);
    SPI_FLASH_SendByte(0xFF);
    SPI_FLASH_SendByte(0xFF);
    id = SPI_FLASH_ReadByte();                                   //读取1次
    W25X_NSS_H;

    W25X_NSS_L;                                                  //片选拉低
    SPI_FLASH_SendByte(W25X_JedecDeviceID);                      //读取ID命令
    id = SPI_FLASH_ReadByte();                                   //读取3次
    id <<= 8;
    id |= SPI_FLASH_ReadByte();
    id <<= 8;
    id |= SPI_FLASH_ReadByte();
    W25X_NSS_H;                                                  //片选拉高

    return id > 0 && id < 0x00FFFFFF;                            //ID=0xEF3013
}

/*
功能：  SPI发送一个字节
参数：  要发送的字节
返回：  无
*/
static void SPI_FLASH_SendByte(uint8_t byte)
{
#if W25X_SPIMOD==0

    uint8_t i;
    for(i = 0; i < 8; i++)
    {
        if(byte & 0x80)
        {
            W25X_MOSI_H;
        }
        else
        {
            W25X_MOSI_L;
        }
        W25X_CLK_H;
        byte <<= 1;

        W25X_CLK_L;
    }

#else

    while (SPI_I2S_GetFlagStatus(W25X_SPIx, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(W25X_SPIx, byte);
    while (SPI_I2S_GetFlagStatus(W25X_SPIx, SPI_I2S_FLAG_RXNE) == RESET);
    SPI_I2S_ReceiveData(W25X_SPIx);

#endif
}

/*
功能：  SPI读取一个字节
参数：  无
返回：  读取的字节
*/
static uint8_t SPI_FLASH_ReadByte(void)
{
#if W25X_SPIMOD==0

    uint8_t i, byte;

    byte = 0;
    for(i = 0; i < 8; i++)
    {
        byte <<= 1;
        W25X_CLK_H;

        if(W25X_MISO)
        {
            byte |= 1;
        }
        W25X_CLK_L;
    }

    return byte;

#else

    while(SPI_I2S_GetFlagStatus(W25X_SPIx, SPI_I2S_FLAG_TXE) == RESET) {;}
    SPI_I2S_SendData(W25X_SPIx, 0xFF);
    while(SPI_I2S_GetFlagStatus(W25X_SPIx, SPI_I2S_FLAG_RXNE) == RESET) {;}
    return SPI_I2S_ReceiveData(W25X_SPIx);

#endif
}

/*
功能：  等待W25X40操作执行完毕
参数：  无
返回：  无
*/
static void SPI_FLASH_WaitForWriteEnd(void)
{
    uint8_t FLASH_Status = 0;
    uint32_t overTim = 0;

    W25X_NSS_L;
    SPI_FLASH_SendByte(W25X_ReadStatusReg);                      //读取状态
    do
    {
        FLASH_Status = SPI_FLASH_ReadByte();
        overTim++;
        if(overTim > 0xFFFF)                                     //避免死机
        {
            break;
        }
    }
    while((FLASH_Status & 0x01) == SET);

    W25X_NSS_H;

}

/*
功能：  W25X40写入一页
参数：  数据,地址,长度
返回：  无
*/
static void SPI_FLASH_PageWrite(u8 *pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
    W25X_NSS_L;
    SPI_FLASH_SendByte(W25X_WriteEnable);                        //写使能命令
    W25X_NSS_H;

    W25X_NSS_L;
    SPI_FLASH_SendByte(W25X_PageProgram);                        //写数据命令
    SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);            //地址
    SPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);
    SPI_FLASH_SendByte(WriteAddr & 0xFF);
    while(NumByteToWrite--)                                      //数据
    {
        SPI_FLASH_SendByte(*pBuffer);
        pBuffer++;
    }
    W25X_NSS_H;

    SPI_FLASH_WaitForWriteEnd();                                 //等待完成
}

/*
功能：  W25X40写入数据
参数：  地址,数据,长度
返回：  无
*/
void W25X_Write(uint32_t addr, uint8_t *dat, uint16_t len)
{
    uint8_t NumOfPage = 0, NumOfSingle = 0, AddrT = 0, count = 0, temp = 0;

    W25X_WP_H;                                                   //写保护关闭

    AddrT = addr % W25X_PageSize;                                //非对齐地址偏移
    count = W25X_PageSize - AddrT;                               //页剩余空间
    NumOfPage = len / W25X_PageSize;                             //存放数据要使用的页数
    NumOfSingle = len % W25X_PageSize;                           //多余的数据(不满一页)

    if(AddrT == 0)                                               //正好对齐
    {
        if(NumOfPage == 0)                                       //只用一页
        {
            SPI_FLASH_PageWrite(dat, addr, len);
        }
        else
        {
            while(NumOfPage--)                                   //循环写入
            {
                SPI_FLASH_PageWrite(dat, addr, W25X_PageSize);   //每次写一页
                addr += W25X_PageSize;
                dat += W25X_PageSize;
            }
            SPI_FLASH_PageWrite(dat, addr, NumOfSingle);         //最后把不满一页的数据写入
        }
    }
    else                                                         //没有对齐
    {
        if(NumOfPage == 0)                                       //数据<一页
        {
            if(NumOfSingle > count)                              //数据长度>页剩余空间
            {
                temp = NumOfSingle - count;

                SPI_FLASH_PageWrite(dat, addr, count);           //把剩余空间写满
                addr += count;
                dat += count;

                SPI_FLASH_PageWrite(dat, addr, temp);            //其他数据写入新的一页
            }
            else                                                 //数据长度!>页剩余空间
            {
                SPI_FLASH_PageWrite(dat, addr, len);             //直接写入
            }
        }
        else                                                     //数据>一页
        {
            len -= count;                                        //因为页不对齐,所以先减去前面要填满一页的数据
            NumOfPage = len / W25X_PageSize;                     //重新计算需要的页数
            NumOfSingle = len % W25X_PageSize;                   //多余的数据(不满一页)

            SPI_FLASH_PageWrite(dat, addr, count);               //将要填满一页的数据先写入
            addr += count;
            dat += count;

            while (NumOfPage--)                                  //循环写入各页
            {
                SPI_FLASH_PageWrite(dat, addr, W25X_PageSize);
                addr += W25X_PageSize;
                dat += W25X_PageSize;
            }

            if(NumOfSingle != 0)                                 //有多余的数据
            {
                SPI_FLASH_PageWrite(dat, addr, NumOfSingle);     //单独写入新的一页
            }
        }
    }

    W25X_WP_L;                                                   //打开写保护
}

/*
功能：  W25X40读取数据
参数：  地址,数据,长度
返回：  无
*/
void W25X_Read(uint32_t addr, uint8_t *dat, uint16_t len)
{
    W25X_NSS_L;

    SPI_FLASH_SendByte(W25X_ReadData);                           //读命令
    SPI_FLASH_SendByte((addr & 0xFF0000) >> 16);                 //地址
    SPI_FLASH_SendByte((addr & 0xFF00) >> 8);
    SPI_FLASH_SendByte(addr & 0xFF);
    while(len--)                                                 //数据
    {
        *dat = SPI_FLASH_ReadByte();
        dat++;
    }

    W25X_NSS_H;
}

/*
功能：	W25X40整片擦除
参数：  无
返回：  无
*/
void W25X_EraseChip(void)
{
    W25X_WP_H;

    W25X_NSS_L;
    SPI_FLASH_SendByte(W25X_WriteEnable);                        //写使能命令
    W25X_NSS_H;

    W25X_NSS_L;
    SPI_FLASH_SendByte(W25X_ChipErase);                          //整片擦除命令
    W25X_NSS_H;

    SPI_FLASH_WaitForWriteEnd();                                 //等待操作完成

    W25X_WP_L;
}

/*
功能：  W25X40擦除一个扇区(4K)
参数：  地址
返回：  无
*/
void W25X_EraseSector(uint32_t addr)
{
    W25X_WP_H;

    W25X_NSS_L;
    SPI_FLASH_SendByte(W25X_WriteEnable);                        //写使能命令
    W25X_NSS_H;

    W25X_NSS_L;
    SPI_FLASH_SendByte(W25X_SectorErase);                        //扇区擦除命令
    SPI_FLASH_SendByte((addr & 0xFF0000) >> 16);                 //地址
    SPI_FLASH_SendByte((addr & 0xFF00) >> 8);
    SPI_FLASH_SendByte(addr & 0xFF);
    W25X_NSS_H;

    SPI_FLASH_WaitForWriteEnd();                                 //等待操作完成

    W25X_WP_L;
}








