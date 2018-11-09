
#include "RLED.h"





uint16_t RLED_Count = 0;                                    //RLED��ʱ�����


/*
���ܣ�	RLED��ʼ��
������	��
���أ�	��
*/
void RLED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
#if RLED_USEPWM==1
    TIM_OCInitTypeDef TIM_OCInitStructure;
#endif

    RLED_RCC_APBxPeriphClockCmd;

    GPIO_InitStructure.GPIO_Pin = RLED_Pinx;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#if RLED_USEPWM==1
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;         //����
#else
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;        //���
#endif
    GPIO_Init(RLED_GPIOx, &GPIO_InitStructure);

    TIM_TimeBaseInitStructure.TIM_Prescaler = 1200 - 1;     //60K
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 120 - 1;         //500Hz
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(RLED_TIMx, &TIM_TimeBaseInitStructure);

#if RLED_USEPWM==1
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;       //PWM
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 120 - 1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;//PWM1��ʼʱΪ��,��֮�������ϼ�Ч
#if RLED_CHx==1
    TIM_OC1Init(RLED_TIMx, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(RLED_TIMx, TIM_OCPreload_Enable);
#elif RLED_CHx==2
    TIM_OC2Init(RLED_TIMx, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(RLED_TIMx, TIM_OCPreload_Enable);
#elif RLED_CHx==3
    TIM_OC3Init(RLED_TIMx, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(RLED_TIMx, TIM_OCPreload_Enable);
#elif RLED_CHx==4
    TIM_OC4Init(RLED_TIMx, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(RLED_TIMx, TIM_OCPreload_Enable);
#else
#error "RLED_CHx isn't support"
#endif

    TIM_ARRPreloadConfig(RLED_TIMx, ENABLE);
#endif

    TIM_Cmd(RLED_TIMx, ENABLE);

    RLED_Close();
}

#if RLED_USEPWM==1

/*
���ܣ�	�ж�һ������2�ļ�����
����1��	������
���أ�	����
*/
uint8_t RLED_log2(uint16_t val)
{
    if(val <= 1)
    {
        return 0;
    }
    else
    {
        return 1 + RLED_log2(val >> 1);
    }
}

/*
���ܣ�	RLEDPWMʹ�ܿ���
����1��	1ʹ��PWM,0�ر�PWM
���أ�	��
*/
void RLED_PWMEN(uint8_t en)
{
    uint32_t tmp;                                           //IO�Ĵ�������ӦΪ32λ
    uint8_t count = RLED_log2(RLED_Pinx);                   //���ú궨���IO��ϵ
    if(count >= 8)count -= 8;
    count <<= 2;
    count += 3;
    tmp = 0x00000001 << count;                              //����IO������в�ͬ����λ
    if(!en)tmp = ~tmp;

    if(RLED_Pinx > GPIO_Pin_7)
    {
        if(en)
        {
            RLED_GPIOx->CRH |= tmp;                         //���ù���
        }
        else
        {
            RLED_GPIOx->CRH &= tmp;                         //��ͨ���
        }
    }
    else
    {
        if(en)
        {
            RLED_GPIOx->CRL |= tmp;                         //���ù���
        }
        else
        {
            RLED_GPIOx->CRL &= tmp;                         //��ͨ���
        }
    }
}

/*
���ܣ�	RLED��
����1��	Ƶ��0-500Hz
����2��	ռ�ձ�0-100
����3��	��ʱ��ms,Ϊ0��ʾ����
���أ�	��
*/
void RLED_Open(uint16_t hz, uint16_t dut, uint16_t t_c)
{
    RLED_Count = t_c;
    if(hz == 0)                                             //Ƶ��Ϊ0
    {
        RLED_PWMEN(0);
        GPIO_ResetBits(RLED_GPIOx, RLED_Pinx);              //����
    }
    else                                                    //Ƶ�ʲ�Ϊ0
    {
        uint32_t hz_t, dut_t;

        if(hz > 500)hz = 500;
        if(dut > 100)dut = 100;

        hz_t = 60000 / hz;
        if(hz_t < 120)hz_t = 120;
        dut_t = dut * hz_t / 100;                           //����Ƶ�ʺ�ռ�ձ�
        //		dut_t=hz_t-dut_t;
        if(dut_t == 0)dut_t = 1;
        dut_t--;
        hz_t--;

#if RLED_CHx==1
        TIM_SetCompare1(RLED_TIMx, dut_t);
        TIM_SetAutoreload(RLED_TIMx, hz_t);
#elif RLED_CHx==2
        TIM_SetCompare2(RLED_TIMx, dut_t);
        TIM_SetAutoreload(RLED_TIMx, hz_t);
#elif RLED_CHx==3
        TIM_SetCompare3(RLED_TIMx, dut_t);
        TIM_SetAutoreload(RLED_TIMx, hz_t);
#elif RLED_CHx==4
        TIM_SetCompare4(RLED_TIMx, dut_t);
        TIM_SetAutoreload(RLED_TIMx, hz_t);
#endif

        RLED_PWMEN(1);                                      //ʹ��PWM
        TIM_SetCounter(RLED_TIMx, 0);
    }

}
#else
void RLED_Open(uint16_t ms)
{
    RLED_Count = ms;
    GPIO_ResetBits(RLED_GPIOx, RLED_Pinx);                  //����
}
#endif



/*
���ܣ�	RLED��
������	��
���أ�	��
*/
void RLED_Close(void)
{
#if RLED_USEPWM==1
    RLED_PWMEN(0);                                          //�ر�PWM
#endif
    GPIO_SetBits(RLED_GPIOx, RLED_Pinx);                    //����
}

/*
���ܣ�	RLED1ms��ʱ�жϵ���
������	��
���أ�	��
*/
void RLED_TIMxInt(void)
{
    if(RLED_Count > 0)                                      //��ʱ>0
    {
        RLED_Count--;                                       //��
        if(RLED_Count == 0)                                 //����0
        {
            RLED_Close();                                   //�ر�
        }
    }
}


