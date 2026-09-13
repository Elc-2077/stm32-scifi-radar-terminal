#include "mlx90640.h"
#include "stm32f10x.h"                  // Device header

#include "bsp_delay.h"

#define MLX90640_FRAME_START_REG 0x0400U

#define MLX_GPIO_RCC_APB2        (RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO)
#define MLX_SCL_PIN              GPIO_Pin_6
#define MLX_SDA_PIN              GPIO_Pin_7
#define MLX_GPIO_PORT            GPIOB

#define I2C_DELAY_CYCLES         260U
#define I2C_STRETCH_TIMEOUT      2000U

#define SCL_H()                  GPIO_SetBits(MLX_GPIO_PORT, MLX_SCL_PIN)
#define SCL_L()                  GPIO_ResetBits(MLX_GPIO_PORT, MLX_SCL_PIN)
#define SDA_H()                  GPIO_SetBits(MLX_GPIO_PORT, MLX_SDA_PIN)
#define SDA_L()                  GPIO_ResetBits(MLX_GPIO_PORT, MLX_SDA_PIN)
#define SDA_READ()               GPIO_ReadInputDataBit(MLX_GPIO_PORT, MLX_SDA_PIN)
#define SCL_READ()               GPIO_ReadInputDataBit(MLX_GPIO_PORT, MLX_SCL_PIN)

static uint8_t g_mlx_addr = MLX90640_I2C_ADDR;
static uint32_t g_probe_ok_cnt = 0;
static uint32_t g_probe_fail_cnt = 0;
static uint32_t g_read_ok_cnt = 0;
static uint32_t g_read_fail_cnt = 0;
static uint8_t g_last_scan_addr = 0xFFU;

static void i2c_delay(void)
{
    volatile uint32_t i;
    for (i = 0; i < I2C_DELAY_CYCLES; i++)
    {
        __NOP();
    }
}

static uint8_t i2c_wait_scl_high(void)
{
    uint32_t timeout = I2C_STRETCH_TIMEOUT;
    while (SCL_READ() == Bit_RESET)
    {
        if (timeout-- == 0U)
        {
            return 0U;
        }
        i2c_delay();
    }
    return 1U;
}

static void i2c_start(void)
{
    SDA_H();
    SCL_H();
    i2c_wait_scl_high();
    i2c_delay();
    SDA_L();
    i2c_delay();
    SCL_L();
}

static void i2c_stop(void)
{
    SDA_L();
    i2c_delay();
    SCL_H();
    i2c_wait_scl_high();
    i2c_delay();
    SDA_H();
    i2c_delay();
}

static uint8_t i2c_write_byte(uint8_t data)
{
    uint8_t bit;

    for (bit = 0; bit < 8U; bit++)
    {
        if ((data & 0x80U) != 0U)
        {
            SDA_H();
        }
        else
        {
            SDA_L();
        }
        data <<= 1;

        i2c_delay();
        SCL_H();
        if (!i2c_wait_scl_high())
        {
            SCL_L();
            return 0U;
        }
        i2c_delay();
        SCL_L();
    }

    SDA_H();
    i2c_delay();
    SCL_H();
    if (!i2c_wait_scl_high())
    {
        SCL_L();
        return 0U;
    }
    i2c_delay();
    bit = (SDA_READ() == Bit_RESET) ? 1U : 0U;
    SCL_L();
    return bit;
}

static uint8_t i2c_read_byte(uint8_t ack)
{
    uint8_t i;
    uint8_t data = 0;

    SDA_H();
    for (i = 0; i < 8U; i++)
    {
        data <<= 1;
        SCL_H();
        if (!i2c_wait_scl_high())
        {
            SCL_L();
            return 0xFFU;
        }
        i2c_delay();
        if (SDA_READ() == Bit_SET)
        {
            data |= 0x01U;
        }
        SCL_L();
        i2c_delay();
    }

    if (ack != 0U)
    {
        SDA_L();
    }
    else
    {
        SDA_H();
    }

    i2c_delay();
    SCL_H();
    i2c_wait_scl_high();
    i2c_delay();
    SCL_L();
    SDA_H();

    return data;
}

static void i2c_bus_recover(void)
{
    uint8_t i;

    SDA_H();
    SCL_H();
    i2c_delay();

    for (i = 0; i < 9U; i++)
    {
        SCL_L();
        i2c_delay();
        SCL_H();
        i2c_delay();
    }

    i2c_stop();
}

static uint8_t mlx90640_read_words(uint16_t reg, uint16_t *buf, uint16_t words)
{
    uint16_t i;
    uint8_t msb;
    uint8_t lsb;

    i2c_start();
    if (!i2c_write_byte((uint8_t)(g_mlx_addr << 1)))
    {
        i2c_stop();
        return 0U;
    }
    if (!i2c_write_byte((uint8_t)(reg >> 8)))
    {
        i2c_stop();
        return 0U;
    }
    if (!i2c_write_byte((uint8_t)(reg & 0xFFU)))
    {
        i2c_stop();
        return 0U;
    }

    i2c_start();
    if (!i2c_write_byte((uint8_t)((g_mlx_addr << 1) | 0x01U)))
    {
        i2c_stop();
        return 0U;
    }

    for (i = 0; i < words; i++)
    {
        msb = i2c_read_byte(1U);
        lsb = i2c_read_byte((i + 1U < words) ? 1U : 0U);
        buf[i] = (uint16_t)(((uint16_t)msb << 8) | lsb);
    }

    i2c_stop();
    return 1U;
}

static uint8_t i2c_probe_addr(uint8_t addr7)
{
    uint8_t ack;

    i2c_start();
    ack = i2c_write_byte((uint8_t)(addr7 << 1));
    i2c_stop();
    return ack;
}

void MLX90640_Init(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(MLX_GPIO_RCC_APB2, ENABLE);

    gpio.GPIO_Pin = MLX_SCL_PIN | MLX_SDA_PIN;
    gpio.GPIO_Mode = GPIO_Mode_Out_OD;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MLX_GPIO_PORT, &gpio);

#if (MLX90640_USE_PS_GPIO == 1U)
    /* Configure PS pin to select I2C mode (pull low) */
    gpio.GPIO_Pin = MLX90640_PS_PIN;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(MLX90640_PS_PORT, &gpio);
    GPIO_ResetBits(MLX90640_PS_PORT, MLX90640_PS_PIN);  /* PS=0 for I2C mode */
#endif

    i2c_bus_recover();
    Delay_Ms(5);
}

uint8_t MLX90640_Probe(uint16_t *device_id)
{
    uint16_t id;
    uint8_t retry;
    uint8_t addr_idx;
    const uint8_t addr_list[3] = {0x52U, MLX90640_I2C_ADDR, 0x32U};

    for (addr_idx = 0; addr_idx < 3U; addr_idx++)
    {
        g_mlx_addr = addr_list[addr_idx];
        for (retry = 0; retry < 3U; retry++)
        {
            if (mlx90640_read_words(0x2407U, &id, 1U))
            {
                g_probe_ok_cnt++;
                if (device_id != 0)
                {
                    *device_id = id;
                }
                return 1U;
            }
            i2c_bus_recover();
            Delay_Ms(2);
        }
    }

    g_probe_fail_cnt++;
    return 0U;
}

uint8_t MLX90640_ScanAddress(uint8_t *addr_out)
{
    uint8_t a;
    for (a = 0x08U; a <= 0x77U; a++)
    {
        if (i2c_probe_addr(a))
        {
            g_last_scan_addr = a;
            g_mlx_addr = a;
            if (addr_out != 0)
            {
                *addr_out = a;
            }
            return 1U;
        }
    }

    g_last_scan_addr = 0xFFU;
    if (addr_out != 0)
    {
        *addr_out = 0xFFU;
    }
    return 0U;
}

uint8_t MLX90640_GetActiveAddress(void)
{
    return g_mlx_addr;
}

uint8_t MLX90640_ReadFrameCentiDeg(int16_t out[MLX90640_HEIGHT][MLX90640_WIDTH])
{
    uint16_t row;
    uint16_t raw_line[MLX90640_WIDTH];

    for (row = 0; row < MLX90640_HEIGHT; row++)
    {
        uint16_t col;
        uint16_t reg = (uint16_t)(MLX90640_FRAME_START_REG + row * MLX90640_WIDTH);
        uint8_t chunk;

        for (chunk = 0; chunk < 4U; chunk++)
        {
            uint16_t sub_reg = (uint16_t)(reg + (uint16_t)chunk * 8U);
            uint16_t *dst = &raw_line[(uint16_t)chunk * 8U];

            if (!mlx90640_read_words(sub_reg, dst, 8U))
            {
                i2c_bus_recover();
                if (!mlx90640_read_words(sub_reg, dst, 8U))
                {
                    g_read_fail_cnt++;
                    return 0U;
                }
            }
        }

        for (col = 0; col < MLX90640_WIDTH; col++)
        {
            /*
             * Relative mode: keep raw pixel intensity as pseudo temperature.
             * We only need shape/contrast, not calibrated absolute Celsius.
             */
            out[row][col] = (int16_t)raw_line[col];
        }
    }

    g_read_ok_cnt++;
    return 1U;
}

uint32_t MLX90640_GetProbeOkCount(void)
{
    return g_probe_ok_cnt;
}

uint32_t MLX90640_GetProbeFailCount(void)
{
    return g_probe_fail_cnt;
}

uint32_t MLX90640_GetReadOkCount(void)
{
    return g_read_ok_cnt;
}

uint32_t MLX90640_GetReadFailCount(void)
{
    return g_read_fail_cnt;
}

void MLX90640_GenerateTestPattern(int16_t out[MLX90640_HEIGHT][MLX90640_WIDTH], uint32_t tick_ms)
{
    uint16_t y;
    uint16_t phase = (uint16_t)((tick_ms / 100U) % 64U);

    for (y = 0; y < MLX90640_HEIGHT; y++)
    {
        uint16_t x;
        for (x = 0; x < MLX90640_WIDTH; x++)
        {
            int32_t base = 1500 + (int32_t)y * 120;
            int32_t wave = (int32_t)((x + phase) % 32U) * 120;
            int32_t t = base + wave;
            if (t > 8000)
            {
                t = 8000;
            }
            out[y][x] = (int16_t)t;
        }
    }
}
