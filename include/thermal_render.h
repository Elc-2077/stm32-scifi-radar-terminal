#ifndef THERMAL_RENDER_H
#define THERMAL_RENDER_H

#include "stm32f10x.h"

#define THERMAL_SRC_W 32
#define THERMAL_SRC_H 24
#define THERMAL_DST_W 240
#define THERMAL_DST_H 320

void ThermalRenderer_Init(void);
void ThermalRenderer_SetRange(int16_t t_min_centi, int16_t t_max_centi);
void ThermalRenderer_RenderLine(const int16_t src[THERMAL_SRC_H][THERMAL_SRC_W], uint16_t y, uint16_t out_line[THERMAL_DST_W]);

#endif
