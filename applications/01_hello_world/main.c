#include "stm32f10x.h"
#include "bsp_delay.h"
#include "st7789.h"

int main(void)
{
    /* 系统初始化 */
    SystemCoreClockUpdate();
    Delay_Init();
    ST7789_Init();

    /* 清屏为黑色 */
    ST7789_FillColor(0x0000);

    /* 显示测试文本 */
    ST7789_DrawString5x7(60, 150, "STM32F103RCT6", 0xFFFF, 0x0000);
    ST7789_DrawString5x7(70, 165, "Hello World!", 0x07E0, 0x0000);

    /* 主循环 */
    while (1)
    {
        Delay_Ms(1000);
    }
}
