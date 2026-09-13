#include "stm32f10x.h"
#include "bsp_delay.h"
#include "st7789.h"
#include "pd_animation.h"

static uint16_t current_frame = 0;

/* 显示PD动画的单帧 */
void PD_Animation_ShowFrame(uint16_t frame_idx)
{
    uint16_t y;
    uint16_t x_offset = (ST7789_WIDTH - PD_ANIM_WIDTH) / 2;
    uint16_t y_offset = (ST7789_HEIGHT - PD_ANIM_HEIGHT) / 2;

    if (frame_idx >= PD_ANIM_FRAMES)
        return;

    for (y = 0; y < PD_ANIM_HEIGHT; y++)
    {
        ST7789_SetAddressWindow(x_offset, y_offset + y,
                               x_offset + PD_ANIM_WIDTH - 1,
                               y_offset + y);
        ST7789_WritePixels(pd_animation_data[frame_idx][y], PD_ANIM_WIDTH);
    }
}

int main(void)
{
    SystemCoreClockUpdate();
    Delay_Init();
    ST7789_Init();

    /* 清屏为黑色 */
    ST7789_FillColor(0x0000);

    /* 循环播放PD动画 */
    while (1)
    {
        PD_Animation_ShowFrame(current_frame);

        current_frame++;
        if (current_frame >= PD_ANIM_FRAMES)
            current_frame = 0;

        Delay_Ms(80);  /* 80ms/帧，约12.5fps */
    }
}
