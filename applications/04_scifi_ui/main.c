#include "stm32f10x.h"
#include "bsp_delay.h"
#include "st7789.h"
#include "ui_graphics.h"

int main(void)
{
    uint32_t last_radar_update = 0;
    uint32_t last_code_update = 8;  // 初始化为8，错开雷达刷新
    uint32_t last_date_update = 0;

    /* 系统初始化 */
    SystemCoreClockUpdate();
    Delay_Init();  // 这会配置 SysTick 为 1ms 中断
    ST7789_Init();

    /* 清屏为黑色 */
    ST7789_FillColor(COLOR_BG);

    /* 显示标题 */
    ST7789_DrawString5x7(65, 5, "SCI-FI RADAR TERMINAL", COLOR_CYAN, COLOR_BG);

    /* 初始化三个UI板块 */

    /* 1. 雷达扫描区域 - 顶部中央 */
    UI_RadarInit(120, 80, 60);

    /* 2. 代码滚动区域 - 中部 */
    UI_CodeScrollInit(10, 160, 220, 85);

    /* 3. 日期显示区域 - 底部 */
    UI_DateDisplayInit(10, 260, 220, 50);
    UI_DateDisplayUpdate(2026, 9, 10);

    /* 主循环 - 非阻塞动画 */
    while (1)
    {
        uint32_t current_time = Delay_GetTickMs();

        /* 雷达更新 - 约 50fps (20ms) */
        if (current_time - last_radar_update >= 20) {
            UI_RadarUpdate();
            last_radar_update = current_time;
        }

        /* 代码滚动更新 - 约 60fps (17ms) 使用不同周期避免同步 */
        if (current_time - last_code_update >= 17) {
            UI_CodeScrollUpdate();
            last_code_update = current_time;
        }

        /* 日期显示更新 - 每秒一次 */
        if (current_time - last_date_update >= 1000) {
            UI_DateDisplayUpdate(2026, 9, 10);
            last_date_update = current_time;
        }
    }
}
