#ifndef ST7789_H
#define ST7789_H

#include "stm32f10x.h"

#define ST7789_WIDTH 240
#define ST7789_HEIGHT 320

/*
 * Module compatibility options:
 * - X/Y offset fixes panels with internal RAM shift.
 * - MADCTL controls scan orientation and RGB/BGR order.
 * - Backlight polarity differs between module boards.
 */
#define ST7789_X_OFFSET            0
#define ST7789_Y_OFFSET            0
#define ST7789_MADCTL_VALUE        0x00U
#define ST7789_BACKLIGHT_ACTIVE_H  1U

/* Pin mapping (from user wiring table) */
#define ST7789_PIN_SPI_PORT        GPIOA
#define ST7789_PIN_SCK             GPIO_Pin_5
#define ST7789_PIN_MOSI            GPIO_Pin_7

#define ST7789_PIN_CS_PORT         GPIOB
#define ST7789_PIN_CS              GPIO_Pin_0

#define ST7789_PIN_DC_PORT         GPIOB
#define ST7789_PIN_DC              GPIO_Pin_1

#define ST7789_PIN_RST_PORT        GPIOC
#define ST7789_PIN_RST             GPIO_Pin_13

/* LED pin on your module is tied to 3.3V, no GPIO control needed */
#define ST7789_USE_BACKLIGHT_GPIO  0U
#define ST7789_PIN_BL_PORT         GPIOB
#define ST7789_PIN_BL              GPIO_Pin_10

void ST7789_Init(void);
void ST7789_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ST7789_WritePixels(const uint16_t *pixels, uint16_t count);
void ST7789_FillColor(uint16_t color);
void ST7789_DrawChar5x7(uint16_t x, uint16_t y, char ch, uint16_t fg, uint16_t bg);
void ST7789_DrawString5x7(uint16_t x, uint16_t y, const char *str, uint16_t fg, uint16_t bg);

#endif
