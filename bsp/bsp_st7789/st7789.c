#include "st7789.h"
#include "stm32f10x.h"                  // Device header

#include "bsp_delay.h"

#define TFT_SPI                   SPI1
#define TFT_SPI_RCC_APB2          RCC_APB2Periph_SPI1
#define TFT_GPIO_RCC_APB2         (RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO)

#define TFT_SCK_PIN               ST7789_PIN_SCK
#define TFT_MOSI_PIN              ST7789_PIN_MOSI
#define TFT_SPI_PORT              ST7789_PIN_SPI_PORT

#define TFT_CS_PIN                ST7789_PIN_CS
#define TFT_CS_PORT               ST7789_PIN_CS_PORT

#define TFT_DC_PIN                ST7789_PIN_DC
#define TFT_DC_PORT               ST7789_PIN_DC_PORT

#define TFT_RST_PIN               ST7789_PIN_RST
#define TFT_RST_PORT              ST7789_PIN_RST_PORT

#define TFT_BL_PIN                ST7789_PIN_BL
#define TFT_BL_PORT               ST7789_PIN_BL_PORT

#define RGB565_BLACK              0x0000
#define RGB565_WHITE              0xFFFF

#define CS_LOW()                  GPIO_ResetBits(TFT_CS_PORT, TFT_CS_PIN)
#define CS_HIGH()                 GPIO_SetBits(TFT_CS_PORT, TFT_CS_PIN)
#define DC_CMD()                  GPIO_ResetBits(TFT_DC_PORT, TFT_DC_PIN)
#define DC_DATA()                 GPIO_SetBits(TFT_DC_PORT, TFT_DC_PIN)

static void spi_write_u8(uint8_t data)
{
    while (SPI_I2S_GetFlagStatus(TFT_SPI, SPI_I2S_FLAG_TXE) == RESET)
    {
    }
    SPI_I2S_SendData(TFT_SPI, data);
    while (SPI_I2S_GetFlagStatus(TFT_SPI, SPI_I2S_FLAG_BSY) == SET)
    {
    }
}

static void write_cmd(uint8_t cmd)
{
    CS_LOW();
    DC_CMD();
    spi_write_u8(cmd);
    CS_HIGH();
}

static void write_data(const uint8_t *data, uint16_t len)
{
    uint16_t i;
    CS_LOW();
    DC_DATA();
    for (i = 0; i < len; i++)
    {
        spi_write_u8(data[i]);
    }
    CS_HIGH();
}

static void write_cmd_data(uint8_t cmd, const uint8_t *data, uint16_t len)
{
    write_cmd(cmd);
    if ((data != 0) && (len > 0U))
    {
        write_data(data, len);
    }
}

static const uint8_t *font_bitmap_5x7(char ch)
{
    /* 完整的5x7 ASCII字体库 */
    static const uint8_t font_data[][5] = {
        {0x00, 0x00, 0x00, 0x00, 0x00}, // 32: space
        {0x00, 0x00, 0x5F, 0x00, 0x00}, // 33: !
        {0x00, 0x07, 0x00, 0x07, 0x00}, // 34: "
        {0x14, 0x7F, 0x14, 0x7F, 0x14}, // 35: #
        {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // 36: $
        {0x23, 0x13, 0x08, 0x64, 0x62}, // 37: %
        {0x36, 0x49, 0x55, 0x22, 0x50}, // 38: &
        {0x00, 0x05, 0x03, 0x00, 0x00}, // 39: '
        {0x00, 0x1C, 0x22, 0x41, 0x00}, // 40: (
        {0x00, 0x41, 0x22, 0x1C, 0x00}, // 41: )
        {0x14, 0x08, 0x3E, 0x08, 0x14}, // 42: *
        {0x08, 0x08, 0x3E, 0x08, 0x08}, // 43: +
        {0x00, 0x50, 0x30, 0x00, 0x00}, // 44: ,
        {0x08, 0x08, 0x08, 0x08, 0x08}, // 45: -
        {0x00, 0x60, 0x60, 0x00, 0x00}, // 46: .
        {0x20, 0x10, 0x08, 0x04, 0x02}, // 47: /
        {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 48: 0
        {0x00, 0x42, 0x7F, 0x40, 0x00}, // 49: 1
        {0x42, 0x61, 0x51, 0x49, 0x46}, // 50: 2
        {0x21, 0x41, 0x45, 0x4B, 0x31}, // 51: 3
        {0x18, 0x14, 0x12, 0x7F, 0x10}, // 52: 4
        {0x27, 0x45, 0x45, 0x45, 0x39}, // 53: 5
        {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 54: 6
        {0x01, 0x71, 0x09, 0x05, 0x03}, // 55: 7
        {0x36, 0x49, 0x49, 0x49, 0x36}, // 56: 8
        {0x06, 0x49, 0x49, 0x29, 0x1E}, // 57: 9
        {0x00, 0x36, 0x36, 0x00, 0x00}, // 58: :
        {0x00, 0x56, 0x36, 0x00, 0x00}, // 59: ;
        {0x08, 0x14, 0x22, 0x41, 0x00}, // 60: <
        {0x14, 0x14, 0x14, 0x14, 0x14}, // 61: =
        {0x00, 0x41, 0x22, 0x14, 0x08}, // 62: >
        {0x02, 0x01, 0x51, 0x09, 0x06}, // 63: ?
        {0x32, 0x49, 0x79, 0x41, 0x3E}, // 64: @
        {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 65: A
        {0x7F, 0x49, 0x49, 0x49, 0x36}, // 66: B
        {0x3E, 0x41, 0x41, 0x41, 0x22}, // 67: C
        {0x7F, 0x41, 0x41, 0x22, 0x1C}, // 68: D
        {0x7F, 0x49, 0x49, 0x49, 0x41}, // 69: E
        {0x7F, 0x09, 0x09, 0x09, 0x01}, // 70: F
        {0x3E, 0x41, 0x49, 0x49, 0x7A}, // 71: G
        {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 72: H
        {0x00, 0x41, 0x7F, 0x41, 0x00}, // 73: I
        {0x20, 0x40, 0x41, 0x3F, 0x01}, // 74: J
        {0x7F, 0x08, 0x14, 0x22, 0x41}, // 75: K
        {0x7F, 0x40, 0x40, 0x40, 0x40}, // 76: L
        {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // 77: M
        {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 78: N
        {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 79: O
        {0x7F, 0x09, 0x09, 0x09, 0x06}, // 80: P
        {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 81: Q
        {0x7F, 0x09, 0x19, 0x29, 0x46}, // 82: R
        {0x46, 0x49, 0x49, 0x49, 0x31}, // 83: S
        {0x01, 0x01, 0x7F, 0x01, 0x01}, // 84: T
        {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 85: U
        {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 86: V
        {0x3F, 0x40, 0x38, 0x40, 0x3F}, // 87: W
        {0x63, 0x14, 0x08, 0x14, 0x63}, // 88: X
        {0x07, 0x08, 0x70, 0x08, 0x07}, // 89: Y
        {0x61, 0x51, 0x49, 0x45, 0x43}, // 90: Z
        {0x00, 0x7F, 0x41, 0x41, 0x00}, // 91: [
        {0x02, 0x04, 0x08, 0x10, 0x20}, // 92: backslash
        {0x00, 0x41, 0x41, 0x7F, 0x00}, // 93: ]
        {0x04, 0x02, 0x01, 0x02, 0x04}, // 94: ^
        {0x40, 0x40, 0x40, 0x40, 0x40}, // 95: _
        {0x00, 0x01, 0x02, 0x04, 0x00}, // 96: `
        {0x20, 0x54, 0x54, 0x54, 0x78}, // 97: a
        {0x7F, 0x48, 0x44, 0x44, 0x38}, // 98: b
        {0x38, 0x44, 0x44, 0x44, 0x20}, // 99: c
        {0x38, 0x44, 0x44, 0x48, 0x7F}, // 100: d
        {0x38, 0x54, 0x54, 0x54, 0x18}, // 101: e
        {0x08, 0x7E, 0x09, 0x01, 0x02}, // 102: f
        {0x0C, 0x52, 0x52, 0x52, 0x3E}, // 103: g
        {0x7F, 0x08, 0x04, 0x04, 0x78}, // 104: h
        {0x00, 0x44, 0x7D, 0x40, 0x00}, // 105: i
        {0x20, 0x40, 0x44, 0x3D, 0x00}, // 106: j
        {0x7F, 0x10, 0x28, 0x44, 0x00}, // 107: k
        {0x00, 0x41, 0x7F, 0x40, 0x00}, // 108: l
        {0x7C, 0x04, 0x18, 0x04, 0x78}, // 109: m
        {0x7C, 0x08, 0x04, 0x04, 0x78}, // 110: n
        {0x38, 0x44, 0x44, 0x44, 0x38}, // 111: o
        {0x7C, 0x14, 0x14, 0x14, 0x08}, // 112: p
        {0x08, 0x14, 0x14, 0x18, 0x7C}, // 113: q
        {0x7C, 0x08, 0x04, 0x04, 0x08}, // 114: r
        {0x48, 0x54, 0x54, 0x54, 0x20}, // 115: s
        {0x04, 0x3F, 0x44, 0x40, 0x20}, // 116: t
        {0x3C, 0x40, 0x40, 0x20, 0x7C}, // 117: u
        {0x1C, 0x20, 0x40, 0x20, 0x1C}, // 118: v
        {0x3C, 0x40, 0x30, 0x40, 0x3C}, // 119: w
        {0x44, 0x28, 0x10, 0x28, 0x44}, // 120: x
        {0x0C, 0x50, 0x50, 0x50, 0x3C}, // 121: y
        {0x44, 0x64, 0x54, 0x4C, 0x44}, // 122: z
        {0x00, 0x08, 0x36, 0x41, 0x00}, // 123: {
        {0x00, 0x00, 0x7F, 0x00, 0x00}, // 124: |
        {0x00, 0x41, 0x36, 0x08, 0x00}, // 125: }
        {0x10, 0x08, 0x08, 0x10, 0x08}, // 126: ~
    };

    if (ch >= 32 && ch <= 126) {
        return font_data[ch - 32];
    }

    // 返回空白字符
    static const uint8_t blank[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
    return blank;
}

void ST7789_Init(void)
{
    GPIO_InitTypeDef gpio;
    SPI_InitTypeDef spi;

    RCC_APB2PeriphClockCmd(TFT_GPIO_RCC_APB2 | TFT_SPI_RCC_APB2, ENABLE);

    gpio.GPIO_Pin = TFT_SCK_PIN | TFT_MOSI_PIN;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TFT_SPI_PORT, &gpio);

    gpio.GPIO_Pin = TFT_CS_PIN;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TFT_CS_PORT, &gpio);

    gpio.GPIO_Pin = TFT_DC_PIN;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TFT_DC_PORT, &gpio);

    gpio.GPIO_Pin = TFT_RST_PIN;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(TFT_RST_PORT, &gpio);

#if (ST7789_USE_BACKLIGHT_GPIO == 1U)
    gpio.GPIO_Pin = TFT_BL_PIN;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TFT_BL_PORT, &gpio);
#endif

    SPI_I2S_DeInit(TFT_SPI);
    spi.SPI_Direction = SPI_Direction_1Line_Tx;
    spi.SPI_Mode = SPI_Mode_Master;
    spi.SPI_DataSize = SPI_DataSize_8b;
    spi.SPI_CPOL = SPI_CPOL_Low;
    spi.SPI_CPHA = SPI_CPHA_1Edge;
    spi.SPI_NSS = SPI_NSS_Soft;
    spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    spi.SPI_FirstBit = SPI_FirstBit_MSB;
    spi.SPI_CRCPolynomial = 7;
    SPI_Init(TFT_SPI, &spi);
    SPI_Cmd(TFT_SPI, ENABLE);

    CS_HIGH();

    GPIO_ResetBits(TFT_RST_PORT, TFT_RST_PIN);
    Delay_Ms(20);
    GPIO_SetBits(TFT_RST_PORT, TFT_RST_PIN);
    Delay_Ms(120);

    {
        const uint8_t madctl = ST7789_MADCTL_VALUE;
        const uint8_t colmod = 0x55;
        const uint8_t porctrl[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
        const uint8_t gctrl = 0x35;
        const uint8_t vcoms = 0x19;
        const uint8_t lcmctrl = 0x2C;
        const uint8_t vdvvrhen = 0x01;
        const uint8_t vrhs = 0x12;
        const uint8_t vdvs = 0x20;
        const uint8_t frctrl2 = 0x0F;
        const uint8_t pwctrl1[] = {0xA4, 0xA1};
        const uint8_t pvgamctrl[] = {0xD0,0x04,0x0D,0x11,0x13,0x2B,0x3F,0x54,0x4C,0x18,0x0D,0x0B,0x1F,0x23};
        const uint8_t nvgamctrl[] = {0xD0,0x04,0x0C,0x11,0x13,0x2C,0x3F,0x44,0x51,0x2F,0x1F,0x1F,0x20,0x23};

        write_cmd(0x01);
        Delay_Ms(10);
        write_cmd_data(0x36, &madctl, 1);
        write_cmd_data(0x3A, &colmod, 1);
        write_cmd_data(0xB2, porctrl, 5);
        write_cmd_data(0xB7, &gctrl, 1);
        write_cmd_data(0xBB, &vcoms, 1);
        write_cmd_data(0xC0, &lcmctrl, 1);
        write_cmd_data(0xC2, &vdvvrhen, 1);
        write_cmd_data(0xC3, &vrhs, 1);
        write_cmd_data(0xC4, &vdvs, 1);
        write_cmd_data(0xC6, &frctrl2, 1);
        write_cmd_data(0xD0, pwctrl1, 2);
        write_cmd_data(0xE0, pvgamctrl, 14);
        write_cmd_data(0xE1, nvgamctrl, 14);

        write_cmd(0x21);
        write_cmd(0x11);
        Delay_Ms(120);
        write_cmd(0x29);
    }

#if (ST7789_USE_BACKLIGHT_GPIO == 1U)
    #if (ST7789_BACKLIGHT_ACTIVE_H == 1U)
    GPIO_SetBits(TFT_BL_PORT, TFT_BL_PIN);
    #else
    GPIO_ResetBits(TFT_BL_PORT, TFT_BL_PIN);
    #endif
#endif
    ST7789_FillColor(RGB565_BLACK);
}

void ST7789_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint8_t data[4];

    x0 = (uint16_t)(x0 + ST7789_X_OFFSET);
    x1 = (uint16_t)(x1 + ST7789_X_OFFSET);
    y0 = (uint16_t)(y0 + ST7789_Y_OFFSET);
    y1 = (uint16_t)(y1 + ST7789_Y_OFFSET);

    data[0] = (uint8_t)(x0 >> 8);
    data[1] = (uint8_t)(x0 & 0xFF);
    data[2] = (uint8_t)(x1 >> 8);
    data[3] = (uint8_t)(x1 & 0xFF);
    write_cmd_data(0x2A, data, 4);

    data[0] = (uint8_t)(y0 >> 8);
    data[1] = (uint8_t)(y0 & 0xFF);
    data[2] = (uint8_t)(y1 >> 8);
    data[3] = (uint8_t)(y1 & 0xFF);
    write_cmd_data(0x2B, data, 4);

    write_cmd(0x2C);
}

void ST7789_WritePixels(const uint16_t *pixels, uint16_t count)
{
    uint16_t i;

    CS_LOW();
    DC_DATA();
    for (i = 0; i < count; i++)
    {
        uint16_t p = pixels[i];
        spi_write_u8((uint8_t)(p >> 8));
        spi_write_u8((uint8_t)(p & 0xFF));
    }
    CS_HIGH();
}

void ST7789_FillColor(uint16_t color)
{
    uint16_t y;
    static uint16_t line[ST7789_WIDTH];

    for (y = 0; y < ST7789_WIDTH; y++)
    {
        line[y] = color;
    }

    for (y = 0; y < ST7789_HEIGHT; y++)
    {
        ST7789_SetAddressWindow(0, y, ST7789_WIDTH - 1, y);
        ST7789_WritePixels(line, ST7789_WIDTH);
    }
}

void ST7789_DrawChar5x7(uint16_t x, uint16_t y, char ch, uint16_t fg, uint16_t bg)
{
    uint8_t col;
    const uint8_t *bmp = font_bitmap_5x7(ch);

    for (col = 0; col < 5U; col++)
    {
        uint8_t bits = bmp[col];
        uint8_t row;
        for (row = 0; row < 7U; row++)
        {
            uint16_t c = ((bits & (1U << row)) != 0U) ? fg : bg;
            ST7789_SetAddressWindow((uint16_t)(x + col), (uint16_t)(y + row), (uint16_t)(x + col), (uint16_t)(y + row));
            ST7789_WritePixels(&c, 1);
        }
    }
}

void ST7789_DrawString5x7(uint16_t x, uint16_t y, const char *str, uint16_t fg, uint16_t bg)
{
    while (*str != '\0')
    {
        ST7789_DrawChar5x7(x, y, *str, fg, bg);
        x = (uint16_t)(x + 6U);
        str++;
    }
}
