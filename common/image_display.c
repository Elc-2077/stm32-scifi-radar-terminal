#include "image_display.h"
#include "st7789.h"
#include "test_image.h"

void ImageDisplay_ShowTestImage(void)
{
    static uint8_t current_frame = 0;
    uint16_t y;
    uint16_t x_offset;
    uint16_t y_offset;

    /* 计算居中位置 */
    x_offset = (ST7789_WIDTH - TEST_IMAGE_WIDTH) / 2;
    y_offset = (ST7789_HEIGHT - TEST_IMAGE_HEIGHT) / 2;

    /* 逐行显示当前帧 */
    for (y = 0; y < TEST_IMAGE_HEIGHT; y++)
    {
        ST7789_SetAddressWindow(x_offset, y_offset + y,
                               x_offset + TEST_IMAGE_WIDTH - 1,
                               y_offset + y);
        ST7789_WritePixels(test_image_data[current_frame][y], TEST_IMAGE_WIDTH);
    }

    /* 切换到下一帧 */
    current_frame++;
    if (current_frame >= TEST_IMAGE_FRAMES)
    {
        current_frame = 0;
    }
}
