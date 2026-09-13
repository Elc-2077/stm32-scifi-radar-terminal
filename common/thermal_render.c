#include "thermal_render.h"
#include "stm32f10x.h"                  // Device header

#define TEMP_MIN_CENTI 0
#define TEMP_MAX_CENTI 8000

static uint16_t g_colormap_jet[256];
static int16_t g_range_min_centi = TEMP_MIN_CENTI;
static int16_t g_range_max_centi = TEMP_MAX_CENTI;

static int32_t clamp_i32(int32_t v, int32_t lo, int32_t hi)
{
    if (v < lo)
    {
        return lo;
    }
    if (v > hi)
    {
        return hi;
    }
    return v;
}

static uint16_t rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint16_t)(((uint16_t)(r & 0xF8U) << 8) | ((uint16_t)(g & 0xFCU) << 3) | (b >> 3));
}

void ThermalRenderer_Init(void)
{
    uint16_t i;
    for (i = 0; i < 256U; i++)
    {
        int32_t t_q10 = ((int32_t)i * 1024) / 255;
        int32_t x4 = t_q10 * 4;
        int32_t r_q10 = 1536 - ((x4 > 3072) ? (x4 - 3072) : (3072 - x4));
        int32_t g_q10 = 1536 - ((x4 > 2048) ? (x4 - 2048) : (2048 - x4));
        int32_t b_q10 = 1536 - ((x4 > 1024) ? (x4 - 1024) : (1024 - x4));

        uint8_t r = (uint8_t)((clamp_i32(r_q10, 0, 1024) * 255) / 1024);
        uint8_t g = (uint8_t)((clamp_i32(g_q10, 0, 1024) * 255) / 1024);
        uint8_t b = (uint8_t)((clamp_i32(b_q10, 0, 1024) * 255) / 1024);

        g_colormap_jet[i] = rgb888_to_rgb565(r, g, b);
    }
}

void ThermalRenderer_SetRange(int16_t t_min_centi, int16_t t_max_centi)
{
    int16_t span = (int16_t)(t_max_centi - t_min_centi);
    if (span < 50)
    {
        span = 50;
    }

    g_range_min_centi = t_min_centi;
    g_range_max_centi = (int16_t)(t_min_centi + span);
}

void ThermalRenderer_RenderLine(const int16_t src[THERMAL_SRC_H][THERMAL_SRC_W], uint16_t y, uint16_t out_line[THERMAL_DST_W])
{
    uint16_t x;
    int32_t y_fp;
    int32_t y0;
    int32_t y1;
    int32_t fy;

    y_fp = ((int32_t)y * (THERMAL_SRC_H - 1) << 16) / (THERMAL_DST_H - 1);
    y0 = y_fp >> 16;
    y1 = (y0 < (THERMAL_SRC_H - 1)) ? (y0 + 1) : y0;
    fy = y_fp & 0xFFFF;

    for (x = 0; x < THERMAL_DST_W; x++)
    {
        int32_t x_fp = ((int32_t)x * (THERMAL_SRC_W - 1) << 16) / (THERMAL_DST_W - 1);
        int32_t x0 = x_fp >> 16;
        int32_t x1 = (x0 < (THERMAL_SRC_W - 1)) ? (x0 + 1) : x0;
        int32_t fx = x_fp & 0xFFFF;

        int32_t t00 = src[y0][x0];
        int32_t t10 = src[y0][x1];
        int32_t t01 = src[y1][x0];
        int32_t t11 = src[y1][x1];

        int32_t top = t00 + (((t10 - t00) * fx) >> 16);
        int32_t bot = t01 + (((t11 - t01) * fx) >> 16);
        int32_t t = top + (((bot - top) * fy) >> 16);
        int32_t t_clamped = clamp_i32(t, g_range_min_centi, g_range_max_centi);
        uint16_t idx = (uint16_t)((t_clamped - g_range_min_centi) * 255 / (g_range_max_centi - g_range_min_centi));

        out_line[x] = g_colormap_jet[idx];
    }
}
