#include "bsp_delay.h"
#include "stm32f10x.h"                  // Device header


static volatile uint32_t g_ms_tick = 0;

void Delay_Init(void)
{
    SysTick_Config(SystemCoreClock / 1000U);
}

void Delay_IncTick(void)
{
    g_ms_tick++;
}

uint32_t Delay_GetTickMs(void)
{
    return g_ms_tick;
}

void Delay_Ms(uint32_t ms)
{
    uint32_t start = Delay_GetTickMs();
    while ((Delay_GetTickMs() - start) < ms)
    {
    }
}
