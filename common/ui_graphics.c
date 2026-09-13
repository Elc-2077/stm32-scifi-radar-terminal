#include "ui_graphics.h"
#include "st7789.h"
#include <math.h>
#include <stdlib.h>

/* 数学常量 */
#define PI 3.14159265359f

/* ========== 基础图形绘制函数 ========== */

void UI_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return;
    ST7789_SetAddressWindow(x, y, x, y);
    ST7789_WritePixels(&color, 1);
}

void UI_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;
    int16_t e2;

    while (1)
    {
        UI_DrawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

void UI_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    UI_DrawLine(x, y, x + w - 1, y, color);
    UI_DrawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
    UI_DrawLine(x + w - 1, y + h - 1, x, y + h - 1, color);
    UI_DrawLine(x, y + h - 1, x, y, color);
}

void UI_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    uint16_t i, j;
    static uint16_t line_buf[ST7789_WIDTH];

    if (w > ST7789_WIDTH) w = ST7789_WIDTH;

    for (i = 0; i < w; i++) {
        line_buf[i] = color;
    }

    for (j = 0; j < h; j++) {
        if (y + j < ST7789_HEIGHT) {
            ST7789_SetAddressWindow(x, y + j, x + w - 1, y + j);
            ST7789_WritePixels(line_buf, w);
        }
    }
}

void UI_DrawCircle(uint16_t cx, uint16_t cy, uint16_t r, uint16_t color)
{
    int16_t x = 0;
    int16_t y = r;
    int16_t d = 3 - 2 * r;

    while (x <= y)
    {
        UI_DrawPixel(cx + x, cy + y, color);
        UI_DrawPixel(cx - x, cy + y, color);
        UI_DrawPixel(cx + x, cy - y, color);
        UI_DrawPixel(cx - x, cy - y, color);
        UI_DrawPixel(cx + y, cy + x, color);
        UI_DrawPixel(cx - y, cy + x, color);
        UI_DrawPixel(cx + y, cy - x, color);
        UI_DrawPixel(cx - y, cy - x, color);

        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

void UI_FillCircle(uint16_t cx, uint16_t cy, uint16_t r, uint16_t color)
{
    int16_t x = 0;
    int16_t y = r;
    int16_t d = 3 - 2 * r;

    while (x <= y)
    {
        UI_DrawLine(cx - x, cy + y, cx + x, cy + y, color);
        UI_DrawLine(cx - x, cy - y, cx + x, cy - y, color);
        UI_DrawLine(cx - y, cy + x, cx + y, cy + x, color);
        UI_DrawLine(cx - y, cy - x, cx + y, cy - x, color);

        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

void UI_DrawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color)
{
    /* 绘制四条边 */
    UI_DrawLine(x + r, y, x + w - r - 1, y, color);
    UI_DrawLine(x + r, y + h - 1, x + w - r - 1, y + h - 1, color);
    UI_DrawLine(x, y + r, x, y + h - r - 1, color);
    UI_DrawLine(x + w - 1, y + r, x + w - 1, y + h - r - 1, color);

    /* 绘制四个圆角 */
    int16_t fx = 0, fy = r;
    int16_t d = 3 - 2 * r;

    while (fx <= fy)
    {
        UI_DrawPixel(x + r - fx, y + r - fy, color);
        UI_DrawPixel(x + w - r - 1 + fx, y + r - fy, color);
        UI_DrawPixel(x + r - fx, y + h - r - 1 + fy, color);
        UI_DrawPixel(x + w - r - 1 + fx, y + h - r - 1 + fy, color);

        UI_DrawPixel(x + r - fy, y + r - fx, color);
        UI_DrawPixel(x + w - r - 1 + fy, y + r - fx, color);
        UI_DrawPixel(x + r - fy, y + h - r - 1 + fx, color);
        UI_DrawPixel(x + w - r - 1 + fy, y + h - r - 1 + fx, color);

        if (d < 0) {
            d = d + 4 * fx + 6;
        } else {
            d = d + 4 * (fx - fy) + 10;
            fy--;
        }
        fx++;
    }
}

/* ========== 雷达扫描动画 ========== */

/* 雷达目标点结构 */
typedef struct {
    int16_t x;           // 相对于雷达中心的坐标
    int16_t y;
    uint8_t echo_brightness;  // 回波亮度 0-255
    uint8_t active;      // 是否激活
} RadarTarget;

static struct {
    uint16_t center_x;
    uint16_t center_y;
    uint16_t radius;
    float angle;         // 当前扫描角度（度）
    RadarTarget targets[5];  // 5个固定目标
    uint8_t sweep_buffer[120][120];  // 扫描扇形亮度缓冲区
} radar;

void UI_RadarInit(uint16_t cx, uint16_t cy, uint16_t radius)
{
    radar.center_x = cx;
    radar.center_y = cy;
    radar.radius = radius;
    radar.angle = 0.0f;

    /* 初始化扫描缓冲区 */
    for (int i = 0; i < 120; i++) {
        for (int j = 0; j < 120; j++) {
            radar.sweep_buffer[i][j] = 0;
        }
    }

    /* 初始化固定目标点（5个） */
    for (int i = 0; i < 5; i++) {
        float target_angle = (i * 72 + 30) * PI / 180.0f;  // 均匀分布
        uint16_t target_dist = 25 + (i * 8);  // 不同距离
        radar.targets[i].x = (int16_t)(target_dist * cosf(target_angle));
        radar.targets[i].y = (int16_t)(target_dist * sinf(target_angle));
        radar.targets[i].echo_brightness = 0;
        radar.targets[i].active = 1;
    }

    /* 绘制边框 */
    UI_DrawRoundRect(cx - radius - 5, cy - radius - 5, radius * 2 + 10, radius * 2 + 10, 8, COLOR_CYAN);

    /* 绘制同心圆 */
    UI_DrawCircle(cx, cy, radius, COLOR_DARK_GREEN);
    UI_DrawCircle(cx, cy, radius * 2 / 3, COLOR_DARK_GREEN);
    UI_DrawCircle(cx, cy, radius / 3, COLOR_DARK_GREEN);

    /* 绘制十字线 */
    UI_DrawLine(cx - radius, cy, cx + radius, cy, COLOR_DARK_GREEN);
    UI_DrawLine(cx, cy - radius, cx, cy + radius, COLOR_DARK_GREEN);
}

/* 颜色混合函数 - 用于实现渐变扫描效果 */
static uint16_t blend_color(uint16_t color1, uint16_t color2, uint8_t alpha)
{
    if (alpha == 0) return color1;
    if (alpha >= 255) return color2;

    uint8_t r1 = (color1 >> 11) & 0x1F;
    uint8_t g1 = (color1 >> 5) & 0x3F;
    uint8_t b1 = color1 & 0x1F;

    uint8_t r2 = (color2 >> 11) & 0x1F;
    uint8_t g2 = (color2 >> 5) & 0x3F;
    uint8_t b2 = color2 & 0x1F;

    uint8_t r = (r1 * (255 - alpha) + r2 * alpha) / 255;
    uint8_t g = (g1 * (255 - alpha) + g2 * alpha) / 255;
    uint8_t b = (b1 * (255 - alpha) + b2 * alpha) / 255;

    return (r << 11) | (g << 5) | b;
}

void UI_RadarUpdate(void)
{
    const float angle_step = 4.0f;  // 60fps下每帧4度，保持平滑旋转
    static float last_angle = -10.0f;  // 记录上一次的角度

    /* 步骤1: 只清除上一帧的扫描线（避免清除十字线和同心圆） */
    if (last_angle >= 0) {
        float last_rad = last_angle * PI / 180.0f;
        for (uint16_t r = 0; r <= radar.radius; r++) {
            int16_t dx = (int16_t)(r * cosf(last_rad));
            int16_t dy = (int16_t)(r * sinf(last_rad));

            /* 检查是否在十字线上 */
            uint8_t is_crosshair = 0;
            if ((dx >= -1 && dx <= 1 && dy >= -radar.radius && dy <= radar.radius) ||  // 竖线
                (dy >= -1 && dy <= 1 && dx >= -radar.radius && dx <= radar.radius)) {  // 横线
                is_crosshair = 1;
            }

            /* 检查是否在同心圆上 */
            uint8_t is_circle = 0;
            int dist_sq = dx * dx + dy * dy;
            int r1_sq = radar.radius * radar.radius;
            int r2_sq = (radar.radius * 2 / 3) * (radar.radius * 2 / 3);
            int r3_sq = (radar.radius / 3) * (radar.radius / 3);
            if ((dist_sq >= r1_sq - radar.radius && dist_sq <= r1_sq + radar.radius) ||
                (dist_sq >= r2_sq - 5 && dist_sq <= r2_sq + 5) ||
                (dist_sq >= r3_sq - 5 && dist_sq <= r3_sq + 5)) {
                is_circle = 1;
            }

            /* 检查是否是目标点位置 */
            uint8_t is_target = 0;
            for (int i = 0; i < 5; i++) {
                if (radar.targets[i].active) {
                    int dist = (dx - radar.targets[i].x) * (dx - radar.targets[i].x) +
                              (dy - radar.targets[i].y) * (dy - radar.targets[i].y);
                    if (dist <= 16) {  // 目标点附近4像素
                        is_target = 1;
                        break;
                    }
                }
            }

            /* 只清除空白区域 */
            if (!is_crosshair && !is_circle && !is_target) {
                UI_DrawPixel(radar.center_x + dx, radar.center_y + dy, COLOR_BG);
            } else if (is_crosshair) {
                /* 重绘十字线 */
                UI_DrawPixel(radar.center_x + dx, radar.center_y + dy, COLOR_DARK_GREEN);
            } else if (is_circle) {
                /* 重绘同心圆 */
                UI_DrawPixel(radar.center_x + dx, radar.center_y + dy, COLOR_DARK_GREEN);
            }
        }
    }

    /* 步骤2: 绘制新的扫描线 */
    float main_rad = radar.angle * PI / 180.0f;
    for (uint16_t r = 0; r <= radar.radius; r++) {
        int16_t dx = (int16_t)(r * cosf(main_rad));
        int16_t dy = (int16_t)(r * sinf(main_rad));
        UI_DrawPixel(radar.center_x + dx, radar.center_y + dy, COLOR_GREEN);
    }

    /* 步骤3: 更新目标点（独立于扫描线） */
    for (int i = 0; i < 5; i++) {
        if (!radar.targets[i].active) continue;

        float target_angle = atan2f(radar.targets[i].y, radar.targets[i].x) * 180.0f / PI;
        if (target_angle < 0) target_angle += 360.0f;

        float angle_diff = radar.angle - target_angle;
        if (angle_diff < 0) angle_diff += 360.0f;
        if (angle_diff > 180.0f) angle_diff = 360.0f - angle_diff;

        uint8_t old_brightness = radar.targets[i].echo_brightness;

        if (angle_diff < 4.0f) {
            radar.targets[i].echo_brightness = 255;
        } else if (radar.targets[i].echo_brightness > 0) {
            radar.targets[i].echo_brightness -= 12;
            if (radar.targets[i].echo_brightness < 12) {
                radar.targets[i].echo_brightness = 0;
            }
        }

        /* 只在亮度变化时重绘目标点 */
        if (old_brightness != radar.targets[i].echo_brightness) {
            uint8_t brightness = radar.targets[i].echo_brightness;

            if (brightness > 150) {
                /* 高亮 */
                for (int dy = -3; dy <= 3; dy++) {
                    for (int dx = -3; dx <= 3; dx++) {
                        int dist_sq = dx * dx + dy * dy;
                        if (dist_sq >= 5 && dist_sq <= 9) {
                            UI_DrawPixel(radar.center_x + radar.targets[i].x + dx,
                                       radar.center_y + radar.targets[i].y + dy,
                                       blend_color(COLOR_BG, COLOR_ORANGE, brightness / 3));
                        }
                    }
                }
                for (int dy = -2; dy <= 2; dy++) {
                    for (int dx = -2; dx <= 2; dx++) {
                        if (dx * dx + dy * dy <= 4) {
                            UI_DrawPixel(radar.center_x + radar.targets[i].x + dx,
                                       radar.center_y + radar.targets[i].y + dy, COLOR_ORANGE);
                        }
                    }
                }
            } else if (brightness > 50) {
                /* 衰减 */
                for (int dy = -2; dy <= 2; dy++) {
                    for (int dx = -2; dx <= 2; dx++) {
                        if (dx * dx + dy * dy <= 4) {
                            UI_DrawPixel(radar.center_x + radar.targets[i].x + dx,
                                       radar.center_y + radar.targets[i].y + dy,
                                       blend_color(COLOR_BG, COLOR_ORANGE, brightness));
                        }
                    }
                }
            } else if (brightness == 0 && old_brightness > 0) {
                /* 清除光晕，只留暗点 */
                for (int dy = -3; dy <= 3; dy++) {
                    for (int dx = -3; dx <= 3; dx++) {
                        int dist_sq = dx * dx + dy * dy;
                        if (dist_sq > 2) {
                            UI_DrawPixel(radar.center_x + radar.targets[i].x + dx,
                                       radar.center_y + radar.targets[i].y + dy, COLOR_BG);
                        }
                    }
                }
                /* 绘制暗点 */
                UI_DrawPixel(radar.center_x + radar.targets[i].x,
                           radar.center_y + radar.targets[i].y, 0x4208);
                UI_DrawPixel(radar.center_x + radar.targets[i].x + 1,
                           radar.center_y + radar.targets[i].y, 0x4208);
                UI_DrawPixel(radar.center_x + radar.targets[i].x,
                           radar.center_y + radar.targets[i].y + 1, 0x4208);
            }
        }
    }

    /* 步骤4: 更新角度 */
    last_angle = radar.angle;
    radar.angle += angle_step;
    if (radar.angle >= 360.0f) {
        radar.angle -= 360.0f;
    }
}

/* ========== 代码滚动效果 ========== */

static struct {
    uint16_t x, y, w, h;
    int16_t pixel_offset;  // 像素级滚动偏移
    char lines[5][30];
} code_panel;

static const char* code_snippets[] = {
    "void init_system(void) {",
    "  SystemInit();",
    "  GPIO_Init(GPIOA);",
    "  SPI_Init(SPI1);",
    "  TIM_Config(TIM2);",
    "  NVIC_Config();",
    "}",
    "",
    "void radar_scan(void) {",
    "  uint16_t angle = 0;",
    "  while(angle < 360) {",
    "    update_targets();",
    "    angle += 3;",
    "  }",
    "}",
    "",
    "void target_update(void) {",
    "  for(int i=0; i<5; i++){",
    "    if(detect_target(i)){",
    "      targets[i].echo++;",
    "      trigger_alert();",
    "    }",
    "  }",
    "}",
    "",
    "void display_update(void) {",
    "  ST7789_SetWindow();",
    "  draw_radar_sweep();",
    "  draw_targets();",
    "  refresh_display();",
    "}",
    "",
    "int main(void) {",
    "  init_system();",
    "  while(1) {",
    "    radar_scan();",
    "    target_update();",
    "    display_update();",
    "    delay_nonblock();",
    "  }",
    "}",
    "",
    "// STM32F103RCT6",
    "// SCI-FI RADAR",
    "// SYSTEM ACTIVE",
};

void UI_CodeScrollInit(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    code_panel.x = x;
    code_panel.y = y;
    code_panel.w = w;
    code_panel.h = h;
    code_panel.pixel_offset = 0;

    /* 绘制边框 */
    UI_DrawRoundRect(x, y, w, h, 6, COLOR_CYAN);
}

void UI_CodeScrollUpdate(void)
{
    const uint8_t line_height = 9;
    const uint8_t num_snippets = sizeof(code_snippets) / sizeof(code_snippets[0]);
    static uint16_t frame_counter = 0;

    /* 像素级平滑滚动 - 提升500%速度 */
    code_panel.pixel_offset += 36;  // 原来6像素，现在36像素（6倍速度）

    /* 计算显示参数 */
    uint16_t total_lines_scrolled = code_panel.pixel_offset / line_height;
    int16_t pixel_shift = -(code_panel.pixel_offset % line_height);

    /* 完全清除整个内容区域（解决拖影问题） */
    UI_FillRect(code_panel.x + 3, code_panel.y + 3,
                code_panel.w - 6, code_panel.h - 6, COLOR_BG);

    /* 绘制所有可见的代码行 */
    uint8_t visible_lines = (code_panel.h / line_height) + 3;
    for (uint8_t i = 0; i < visible_lines; i++) {
        uint16_t line_idx = (total_lines_scrolled + i) % num_snippets;
        int16_t y_pos = code_panel.y + 5 + pixel_shift + (i * line_height);

        if (y_pos >= code_panel.y - line_height && y_pos <= code_panel.y + code_panel.h) {
            uint16_t text_color = COLOR_GREEN;
            int16_t relative_y = y_pos - code_panel.y;

            /* 顶部淡出 */
            if (relative_y < 12) {
                uint8_t fade = (relative_y > 0) ? (relative_y * 255) / 12 : 0;
                text_color = blend_color(COLOR_BG, COLOR_GREEN, fade);
            }
            /* 底部淡入 */
            else if (relative_y > code_panel.h - 15) {
                int16_t dist = code_panel.h - relative_y;
                uint8_t fade = (dist > 0) ? (dist * 255) / 15 : 0;
                text_color = blend_color(COLOR_BG, COLOR_GREEN, fade);
            }

            /* 绘制代码文本 */
            ST7789_DrawString5x7(code_panel.x + 6, y_pos,
                               code_snippets[line_idx], text_color, COLOR_BG);
        }
    }

    /* 防止溢出 */
    if (code_panel.pixel_offset >= line_height * num_snippets) {
        code_panel.pixel_offset = 0;
    }

    /* 闪烁光标 */
    frame_counter++;
    if ((frame_counter / 20) % 2 == 0) {
        UI_FillRect(code_panel.x + code_panel.w - 12, code_panel.y + code_panel.h - 12,
                   3, 7, COLOR_GREEN);
    }
}

/* ========== 日期显示 ========== */

static struct {
    uint16_t x, y, w, h;
} date_panel;

void UI_DateDisplayInit(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    date_panel.x = x;
    date_panel.y = y;
    date_panel.w = w;
    date_panel.h = h;

    /* 绘制边框 */
    UI_DrawRoundRect(x, y, w, h, 6, COLOR_CYAN);
}

void UI_DateDisplayUpdate(uint16_t year, uint8_t month, uint8_t day)
{
    char date_str[20];

    /* 清除内容区域 */
    UI_FillRect(date_panel.x + 2, date_panel.y + 2, date_panel.w - 4, date_panel.h - 4, COLOR_BG);

    /* 格式化日期字符串 */
    date_str[0] = '0' + (year / 1000);
    date_str[1] = '0' + ((year / 100) % 10);
    date_str[2] = '0' + ((year / 10) % 10);
    date_str[3] = '0' + (year % 10);
    date_str[4] = '-';
    date_str[5] = '0' + (month / 10);
    date_str[6] = '0' + (month % 10);
    date_str[7] = '-';
    date_str[8] = '0' + (day / 10);
    date_str[9] = '0' + (day % 10);
    date_str[10] = '\0';

    /* 居中显示日期 */
    ST7789_DrawString5x7(date_panel.x + 40, date_panel.y + 20, date_str, COLOR_CYAN, COLOR_BG);
}
