#ifndef UI_GRAPHICS_H
#define UI_GRAPHICS_H

#include "stm32f10x.h"

/* 科幻主题配色 RGB565 */
#define COLOR_BG           0x0000   // 黑色背景
#define COLOR_CYAN         0x07FF   // 青色
#define COLOR_GREEN        0x07E0   // 绿色
#define COLOR_DARK_GREEN   0x0320   // 暗绿色
#define COLOR_ORANGE       0xFD20   // 橙色
#define COLOR_BLUE         0x001F   // 蓝色
#define COLOR_GRAY         0x8410   // 灰色
#define COLOR_WHITE        0xFFFF   // 白色

/* 基础图形绘制 */
void UI_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void UI_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void UI_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void UI_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void UI_DrawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color);
void UI_DrawCircle(uint16_t cx, uint16_t cy, uint16_t r, uint16_t color);
void UI_FillCircle(uint16_t cx, uint16_t cy, uint16_t r, uint16_t color);

/* 雷达扫描动画 */
void UI_RadarInit(uint16_t cx, uint16_t cy, uint16_t radius);
void UI_RadarUpdate(void);

/* 代码滚动效果 */
void UI_CodeScrollInit(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void UI_CodeScrollUpdate(void);

/* 日期显示 */
void UI_DateDisplayInit(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void UI_DateDisplayUpdate(uint16_t year, uint8_t month, uint8_t day);

#endif
