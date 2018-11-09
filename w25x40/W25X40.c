
#include "W25X40.h"


/*
�ļ���;:           STM32����W25X40
����:               �Ŷ���
����ʱ��:           2018/07/04
����ʱ��:           2018/07/04
�汾:               V1.0

��ʷ�汾:           V1.0:����STM32 SPI/IO������W25X40


*/



#define W25X_PageSize         256                                //ҳ����


/*�����б�*/
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
/*ģ��SPI����״̬�л�����*/
#define W25X_CLK_H  GPIO_SetBits(W25X_CLK_GPIOx,W25X_CLK_Pinx)
#define W25X_CLK_L  GPIO_ResetBits(W25X_CLK_GPIOx,W25X_CLK_Pinx)
#define W25X_MOSI_H GPIO_SetBits(W25X_MOSI_GPIOx,W25X_MOSI_Pinx)
#define W25X_MOSI_L GPIO_ResetBits(W25X_MOSI_GPIOx,W25X_MOSI_Pinx)

#define W25X_MISO   GPIO_ReadInputDataBit(W25X_MISO_GPIOx,W25X_MISO_Pinx)

/*ģ��SPI����״̬�л�����*/
#endif

/*Ƭѡ,Lѡ��*/
#define W25X_NSS_H GPIO_SetBits(W25X_NSS_GPIOx,W25X_NSS_Pinx)
#define W25X_NSS_L GPIO_ResetBits(W25X_NSS_GPIOx,W25X_NSS_Pinx)
/*д����,H����д��*/
#define W25X_WP_H  GPIO_SetBits(W25X_WP_GPIOx,W25X_WP_Pinx)
#define W25X_WP_L  GPIO_ResetBits(W25X_WP_GPIOx,W25X_WP_Pinx)




static void SPI_FLASH_SendByte(uint8_t byte);
static uint8_t SPI_FLASH_ReadByte(void);


/*
���ܣ�  W25X40оƬ��ʼ��
������  ��
���أ�  1�ɹ���0ʧ��
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
    GPIO_InitStructure.GPIO_Pin = W25X_CLK_Pinx;                 //CLK�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(W25X_CLK_GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = W25X_MOSI_Pinx;                //MOSI�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(W25X_MOSI_GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = W25X_MISO_Pinx;                //MISO��������
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(W25X_MISO_GPIOx, &GPIO_InitStructure);

#else
    GPIO_InitStructure.GPIO_Pin = W25X_CLK_Pinx;                 //CLK���Ÿ���
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(W25X_CLK_GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = W25X_MOSI_Pinx;                //MOSI���Ÿ���
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(W25X_MOSI_GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = W25X_MISO_Pinx;                //MISO���Ÿ���
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

    SPI_Cmd(W25X_SPIx, ENABLE);                                  //SPI��ʼ��
#endif

    GPIO_InitStructure.GPIO_Pin = W25X_NSS_Pinx;                 //NSS�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(W25X_NSS_GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = W25X_WP_Pinx;                  //WP�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(W25X_WP_GPIOx, &GPIO_InitStructure);

    W25X_NSS_H;                                                  //Ƭѡ����,Ĭ�ϲ�ʹ�ܶ�д
    W25X_WP_L;                                                   //д��������,Ĭ�ϲ���д��

    W25X_NSS_L;
    SPI_FLASH_SendByte(W25X_DeviceID);                           //��ȡID����
    SPI_FLASH_SendByte(0xFF);
    SPI_FLASH_SendByte(0xFF);
    SPI_FLASH_SendByte(0xFF);
    id = SPI_FLASH_ReadByte();                                   //��ȡ1��
    W25X_NSS_H;

    W25X_NSS_L;                                                  //Ƭѡ����
    SPI_FLASH_SendByte(W25X_JedecDeviceID);                      //��ȡID����
    id = SPI_FLASH_ReadByte();                                   //��ȡ3��
    id <<= 8;
    id |= SPI_FLASH_ReadByte();
    id <<= 8;
    id |= SPI_FLASH_ReadByte();
    W25X_NSS_H;                                                  //Ƭѡ����

    return id > 0 && id < 0x00FFFFFF;                            //ID=0xEF3013
}

/*
���ܣ�  SPI����һ���ֽ�
������  Ҫ���͵��ֽ�
���أ�  ��
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
���ܣ�  SPI��ȡһ���ֽ�
������  ��
���أ�  ��ȡ���ֽ�
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
���ܣ�  �ȴ�W25X40����ִ�����
������  ��
���أ�  ��
*/
static void SPI_FLASH_WaitForWriteEnd(void)
{
    uint8_t FLASH_Status = 0;
    uint32_t overTim = 0;

    W25X_NSS_L;
    SPI_FLASH_SendByte(W25X_ReadStatusReg);                      //��ȡ״̬
    do
    {
        FLASH_Status = SPI_FLASH_ReadByte();
        overTim++;
        if(overTim > 0xFFFF)                                     //��������
        {
            break;
        }
    }
    while((FLASH_Status & 0x01) == SET);

    W25X_NSS_H;

}

/*
���ܣ�  W25X40д��һҳ
������  ����,��ַ,����
���أ�  ��
*/
static void SPI_FLASH_PageWrite(u8 *pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
    W25X_NSS_L;
    SPI_FLASH_SendByte(W25X_WriteEnable);                        //дʹ������
    W25X_NSS_H;

    W25X_NSS_L;
    SPI_FLASH_SendByte(W25X_PageProgram);                        //д��������
    SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);            //��ַ
    SPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);
    SPI_FLASH_SendByte(WriteAddr & 0xFF);
    while(NumByteToWrite--)                                      //����
    {
        SPI_FLASH_SendByte(*pBuffer);
        pBuffer++;
    }
    W25X_NSS_H;

    SPI_FLASH_WaitForWriteEnd();                                 //�ȴ����
}

/*
���ܣ�  W25X40д������
������  ��ַ,����,����
���أ�  ��
*/
void W25X_Write(uint32_t addr, uint8_t *dat, uint16_t len)
{
    uint8_t NumOfPage = 0, NumOfSingle = 0, AddrT = 0, count = 0, temp = 0;

    W25X_WP_H;                                                   //д�����ر�

    AddrT = addr % W25X_PageSize;                                //�Ƕ����ַƫ��
    count = W25X_PageSize - AddrT;                               //ҳʣ��ռ�
    NumOfPage = len / W25X_PageSize;                             //�������Ҫʹ�õ�ҳ��
    NumOfSingle = len % W25X_PageSize;                           //���������(����һҳ)

    if(AddrT == 0)                                               //���ö���
    {
        if(NumOfPage == 0)                                       //ֻ��һҳ
        {
            SPI_FLASH_PageWrite(dat, addr, len);
        }
        else
        {
            while(NumOfPage--)                                   //ѭ��д��
            {
                SPI_FLASH_PageWrite(dat, addr, W25X_PageSize);   //ÿ��дһҳ
                addr += W25X_PageSize;
                dat += W25X_PageSize;
            }
            SPI_FLASH_PageWrite(dat, addr, NumOfSingle);         //���Ѳ���һҳ������д��
        }
    }
    else                                                         //û�ж���
    {
        if(NumOfPage == 0)                                       //����<һҳ
        {
            if(NumOfSingle > count)                              //���ݳ���>ҳʣ��ռ�
            {
                temp = NumOfSingle - count;

                SPI_FLASH_PageWrite(dat, addr, count);           //��ʣ��ռ�д��
                addr += count;
                dat += count;

                SPI_FLASH_PageWrite(dat, addr, temp);            //��������д���µ�һҳ
            }
            else                                                 //���ݳ���!>ҳʣ��ռ�
            {
                SPI_FLASH_PageWrite(dat, addr, len);             //ֱ��д��
            }
        }
        else                                                     //����>һҳ
        {
            len -= count;                                        //��Ϊҳ������,�����ȼ�ȥǰ��Ҫ����һҳ������
            NumOfPage = len / W25X_PageSize;                     //���¼�����Ҫ��ҳ��
            NumOfSingle = len % W25X_PageSize;                   //���������(����һҳ)

            SPI_FLASH_PageWrite(dat, addr, count);               //��Ҫ����һҳ��������д��
            addr += count;
            dat += count;

            while (NumOfPage--)                                  //ѭ��д���ҳ
            {
                SPI_FLASH_PageWrite(dat, addr, W25X_PageSize);
                addr += W25X_PageSize;
                dat += W25X_PageSize;
            }

            if(NumOfSingle != 0)                                 //�ж��������
            {
                SPI_FLASH_PageWrite(dat, addr, NumOfSingle);     //����д���µ�һҳ
            }
        }
    }

    W25X_WP_L;                                                   //��д����
}

/*
���ܣ�  W25X40��ȡ����
������  ��ַ,����,����
���أ�  ��
*/
void W25X_Read(uint32_t addr, uint8_t *dat, uint16_t len)
{
    W25X_NSS_L;

    SPI_FLASH_SendByte(W25X_ReadData);                           //������
    SPI_FLASH_SendByte((addr & 0xFF0000) >> 16);                 //��ַ
    SPI_FLASH_SendByte((addr & 0xFF00) >> 8);
    SPI_FLASH_SendByte(addr & 0xFF);
    while(len--)                                                 //����
    {
        *dat = SPI_FLASH_ReadByte();
        dat++;
    }

    W25X_NSS_H;
}

/*
���ܣ�	W25X40��Ƭ����
������  ��
���أ�  ��
*/
void W25X_EraseChip(void)
{
    W25X_WP_H;

    W25X_NSS_L;
    SPI_FLASH_SendByte(W25X_WriteEnable);                        //дʹ������
    W25X_NSS_H;

    W25X_NSS_L;
    SPI_FLASH_SendByte(W25X_ChipErase);                          //��Ƭ��������
    W25X_NSS_H;

    SPI_FLASH_WaitForWriteEnd();                                 //�ȴ��������

    W25X_WP_L;
}

/*
���ܣ�  W25X40����һ������(4K)
������  ��ַ
���أ�  ��
*/
void W25X_EraseSector(uint32_t addr)
{
    W25X_WP_H;

    W25X_NSS_L;
    SPI_FLASH_SendByte(W25X_WriteEnable);                        //дʹ������
    W25X_NSS_H;

    W25X_NSS_L;
    SPI_FLASH_SendByte(W25X_SectorErase);                        //������������
    SPI_FLASH_SendByte((addr & 0xFF0000) >> 16);                 //��ַ
    SPI_FLASH_SendByte((addr & 0xFF00) >> 8);
    SPI_FLASH_SendByte(addr & 0xFF);
    W25X_NSS_H;

    SPI_FLASH_WaitForWriteEnd();                                 //�ȴ��������

    W25X_WP_L;
}








