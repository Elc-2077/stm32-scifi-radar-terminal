#ifndef MLX90640_H
#define MLX90640_H

#include "stm32f10x.h"

#define MLX90640_I2C_ADDR 0x33U
#define MLX90640_WIDTH 32
#define MLX90640_HEIGHT 24

/* PS pin for protocol selection (optional GPIO control) */
#define MLX90640_USE_PS_GPIO  0U
#define MLX90640_PS_PORT      GPIOB
#define MLX90640_PS_PIN       GPIO_Pin_8

void MLX90640_Init(void);
uint8_t MLX90640_Probe(uint16_t *device_id);
uint8_t MLX90640_ReadFrameCentiDeg(int16_t out[MLX90640_HEIGHT][MLX90640_WIDTH]);
uint8_t MLX90640_ScanAddress(uint8_t *addr_out);
uint8_t MLX90640_GetActiveAddress(void);
uint32_t MLX90640_GetProbeOkCount(void);
uint32_t MLX90640_GetProbeFailCount(void);
uint32_t MLX90640_GetReadOkCount(void);
uint32_t MLX90640_GetReadFailCount(void);
void MLX90640_GenerateTestPattern(int16_t out[MLX90640_HEIGHT][MLX90640_WIDTH], uint32_t tick_ms);

#endif
