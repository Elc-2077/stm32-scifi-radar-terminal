#include "stm32f10x.h"
#include "bsp_delay.h"
#include "st7789.h"
#include "mlx90640.h"
#include "thermal_render.h"
#include "image_display.h"

/* 热成像数据缓冲区 */
static float mlx90640_data[768];  // 32x24 像素的温度数据

int main(void)
{
    /* 系统初始化 */
    SystemCoreClockUpdate();
    Delay_Init();
    ST7789_Init();

    /* 清屏为黑色 */
    ST7789_FillColor(0x0000);

    /* 显示启动信息 */
    ST7789_DrawString5x7(60, 100, "Thermal Camera", 0xFFFF, 0x0000);
    ST7789_DrawString5x7(70, 120, "Initializing...", 0x07E0, 0x0000);

    Delay_Ms(1000);

    /* TODO: 初始化 MLX90640 传感器 */
    // MLX90640_Init();

    /* 清屏，准备显示热成像画面 */
    ST7789_FillColor(0x0000);

    /* 主循环 */
    while (1)
    {
        /* TODO: 读取热成像数据 */
        // MLX90640_GetFrameData(mlx90640_data);

        /* TODO: 渲染热成像画面 */
        // ThermalRender_Draw(mlx90640_data, 32, 24);

        /* TODO: 显示温度信息 */
        // float temp_min, temp_max;
        // ThermalRender_GetTempRange(mlx90640_data, 768, &temp_min, &temp_max);
        // 显示温度范围

        Delay_Ms(100);  // 约10fps
    }
}
