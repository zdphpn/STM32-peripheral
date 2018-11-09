
#include "RLED.h"





uint16_t RLED_Count = 0;                                    //RLED开时间计数


/*
功能：	RLED初始化
参数：	无
返回：	无
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
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;         //复用
#else
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;        //输出
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
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;//PWM1开始时为低,打开之后能马上见效
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
功能：	判断一个数是2的几次幂
参数1：	操作数
返回：	幂数
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
功能：	RLEDPWM使能开关
参数1：	1使能PWM,0关闭PWM
返回：	无
*/
void RLED_PWMEN(uint8_t en)
{
    uint32_t tmp;                                           //IO寄存器操作应为32位
    uint8_t count = RLED_log2(RLED_Pinx);                   //利用宏定义的IO关系
    if(count >= 8)count -= 8;
    count <<= 2;
    count += 3;
    tmp = 0x00000001 << count;                              //根据IO计算进行不同的移位
    if(!en)tmp = ~tmp;

    if(RLED_Pinx > GPIO_Pin_7)
    {
        if(en)
        {
            RLED_GPIOx->CRH |= tmp;                         //复用功能
        }
        else
        {
            RLED_GPIOx->CRH &= tmp;                         //普通输出
        }
    }
    else
    {
        if(en)
        {
            RLED_GPIOx->CRL |= tmp;                         //复用功能
        }
        else
        {
            RLED_GPIOx->CRL &= tmp;                         //普通输出
        }
    }
}

/*
功能：	RLED开
参数1：	频率0-500Hz
参数2：	占空比0-100
参数3：	打开时间ms,为0表示常开
返回：	无
*/
void RLED_Open(uint16_t hz, uint16_t dut, uint16_t t_c)
{
    RLED_Count = t_c;
    if(hz == 0)                                             //频率为0
    {
        RLED_PWMEN(0);
        GPIO_ResetBits(RLED_GPIOx, RLED_Pinx);              //拉低
    }
    else                                                    //频率不为0
    {
        uint32_t hz_t, dut_t;

        if(hz > 500)hz = 500;
        if(dut > 100)dut = 100;

        hz_t = 60000 / hz;
        if(hz_t < 120)hz_t = 120;
        dut_t = dut * hz_t / 100;                           //计算频率和占空比
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

        RLED_PWMEN(1);                                      //使能PWM
        TIM_SetCounter(RLED_TIMx, 0);
    }

}
#else
void RLED_Open(uint16_t ms)
{
    RLED_Count = ms;
    GPIO_ResetBits(RLED_GPIOx, RLED_Pinx);                  //拉低
}
#endif



/*
功能：	RLED关
参数：	无
返回：	无
*/
void RLED_Close(void)
{
#if RLED_USEPWM==1
    RLED_PWMEN(0);                                          //关闭PWM
#endif
    GPIO_SetBits(RLED_GPIOx, RLED_Pinx);                    //拉高
}

/*
功能：	RLED1ms定时中断调用
参数：	无
返回：	无
*/
void RLED_TIMxInt(void)
{
    if(RLED_Count > 0)                                      //计时>0
    {
        RLED_Count--;                                       //减
        if(RLED_Count == 0)                                 //减到0
        {
            RLED_Close();                                   //关闭
        }
    }
}


