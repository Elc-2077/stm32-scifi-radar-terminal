# STM32F103RCT6 科幻雷达终端

基于 STM32F103RCT6 和 ST7789 显示屏的科幻风格雷达终端，具有实时动态扫描效果和终端代码滚动显示。

![Demo](docs/demo.gif)

##  特性

-  **雷达扫描动画** - 平滑的顺时针旋转扫描线，同心圆和十字交叉线
-  **动态目标检测** - 5个固定目标点，扫描时高亮显示并产生回波残影效果
-  **实时代码滚动** - 终端风格的代码从底部向上流动，带淡入淡出效果
-  **高性能非阻塞动画** - 基于 SysTick 定时器，独立刷新周期，无闪烁
-  **科幻视觉风格** - 青色边框、绿色雷达、橙色目标、黑色背景

##  效果预览

- **雷达扫描**: 50fps 平滑旋转，扫描线经过时目标点亮起（橙色光晕）
- **回波效果**: 目标点在扫描后逐渐衰减，形成余辉效果
- **代码滚动**: 60fps 像素级平滑滚动，顶部和底部淡入淡出
- **日期显示**: 底部青色日期区域，每秒更新

##  快速开始

### 硬件要求

- STM32F103RCT6 开发板（ARM Cortex-M3，72MHz，48KB RAM，256KB Flash）
- ST7789 SPI TFT 显示屏（240x320 分辨率）
- ST-Link 调试器

### 接线说明

| ST7789 引脚 | STM32 引脚 | 说明 |
|------------|----------|------|
| SCK | PA5 | SPI 时钟 |
| MOSI | PA7 | SPI 数据 |
| CS | PB0 | 片选 |
| DC | PB1 | 数据/命令 |
| RST | PC13 | 复位 |
| VCC | 3.3V | 电源 |
| GND | GND | 地 |

### 编译上传

```bash
# 安装 PlatformIO
pip install platformio

# 克隆项目
git clone https://github.com/yourusername/stm32-scifi-radar.git
cd stm32-scifi-radar

# 编译科幻雷达终端
pio run -e scifi_ui

# 编译并上传到开发板
pio run -e scifi_ui -t upload
```

### 使用便捷脚本

```bash
# Windows
build.bat scifi_ui
build.bat -u scifi_ui

# Linux/Mac/Git Bash
./build.sh scifi_ui
./build.sh -u scifi_ui
```

##  资源使用

- **Flash**: 13.5 KB (5.1%)
- **RAM**: 15.6 KB (31.8%)
- **帧率**: 雷达 50fps / 代码滚动 60fps

##  技术实现

### 非阻塞动画架构

使用 SysTick 1ms 中断提供时基，主循环采用时间戳轮询方式：

```c
while (1)
{
    uint32_t current_time = Delay_GetTickMs();
    
    // 雷达更新 - 50fps (20ms)
    if (current_time - last_radar_update >= 20) {
        UI_RadarUpdate();
        last_radar_update = current_time;
    }
    
    // 代码滚动更新 - 60fps (17ms)
    if (current_time - last_code_update >= 17) {
        UI_CodeScrollUpdate();
        last_code_update = current_time;
    }
}
```

### 独立刷新周期

- 雷达: 20ms 周期
- 代码滚动: 17ms 周期
- 两者互质，避免同步造成的视觉卡顿

### 颜色混合算法

实现目标点的渐变回波效果：

```c
static uint16_t blend_color(uint16_t color1, uint16_t color2, uint8_t alpha)
{
    // RGB565 分离和线性插值
    uint8_t r = (r1 * (255 - alpha) + r2 * alpha) / 255;
    uint8_t g = (g1 * (255 - alpha) + g2 * alpha) / 255;
    uint8_t b = (b1 * (255 - alpha) + b2 * alpha) / 255;
    return (r << 11) | (g << 5) | b;
}
```

##  项目结构

```
stm32-scifi-radar/
├── platformio.ini          # PlatformIO 配置
├── build.sh / build.bat    # 快速编译脚本
│
├── applications/
│   ├── 04_scifi_ui/       # 科幻雷达终端（主项目）
│   ├── 01_hello_world/    # Hello World 示例
│   ├── 02_pd_animation/   # PD 动画播放器
│   └── 03_thermal_camera/ # 热成像相机
│
├── common/
│   ├── ui_graphics.c/h    # 雷达和代码滚动实现
│   ├── image_display.c/h  # 图像显示
│   ├── thermal_render.c/h # 热成像渲染
│   └── stm32f10x_it.c/h   # 中断处理（SysTick）
│
├── bsp/
│   ├── bsp_delay/         # 延时和时间戳功能
│   └── bsp_st7789/        # ST7789 SPI 显示驱动
│
├── drivers/               # 外设驱动
├── library/               # STM32 标准外设库
├── start/                 # CMSIS 和启动文件
└── include/               # 共享头文件
```

##  自定义配置

### 修改雷达参数

编辑 `applications/04_scifi_ui/main.c`:

```c
// 雷达位置和大小
UI_RadarInit(120, 80, 60);  // (中心X, 中心Y, 半径)

// 刷新率（主循环中）
if (current_time - last_radar_update >= 20) {  // 20ms = 50fps
```

### 修改代码滚动速度

编辑 `common/ui_graphics.c` 的 `UI_CodeScrollUpdate()`:

```c
code_panel.pixel_offset += 36;  // 增大数值加速，减小数值减速
```

### 修改颜色方案

编辑 `common/ui_graphics.h`:

```c
#define COLOR_BG          0x0000  // 背景色（黑色）
#define COLOR_CYAN        0x07FF  // 边框颜色（青色）
#define COLOR_GREEN       0x07E0  // 雷达扫描线（绿色）
#define COLOR_ORANGE      0xFD20  // 目标点（橙色）
#define COLOR_DARK_GREEN  0x0320  // 同心圆/十字线（暗绿）
```

##  核心 API

### UI 图形库

```c
// 雷达系统
void UI_RadarInit(uint16_t cx, uint16_t cy, uint16_t radius);
void UI_RadarUpdate(void);

// 代码滚动
void UI_CodeScrollInit(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void UI_CodeScrollUpdate(void);

// 日期显示
void UI_DateDisplayInit(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void UI_DateDisplayUpdate(uint16_t year, uint8_t month, uint8_t day);

// 基础图形
void UI_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void UI_DrawCircle(uint16_t cx, uint16_t cy, uint16_t r, uint16_t color);
void UI_DrawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color);
void UI_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
```

### ST7789 显示驱动

```c
void ST7789_Init(void);
void ST7789_FillColor(uint16_t color);
void ST7789_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ST7789_WritePixels(const uint16_t *pixels, uint16_t count);
void ST7789_DrawString5x7(uint16_t x, uint16_t y, const char *str, uint16_t fg, uint16_t bg);
```

### 延时和时间戳

```c
void Delay_Init(void);              // 初始化（配置 SysTick 1ms 中断）
uint32_t Delay_GetTickMs(void);     // 获取系统运行毫秒数
void Delay_Ms(uint32_t ms);         // 阻塞延时（仅用于初始化）
```

## 🚀 其他示例应用

项目包含多个示例程序：

```bash
# Hello World - 基础显示测试
pio run -e hello_world -t upload

# PD 动画播放器 - 图像动画
pio run -e pd_animation -t upload

# 热成像相机 - MLX90640 传感器（需要硬件）
pio run -e thermal_camera -t upload
```

##  开发文档

- [CHEAT_SHEET.txt](CHEAT_SHEET.txt) - 快速参考卡片
- [STM32F103 数据手册](https://www.st.com/resource/en/datasheet/stm32f103rc.pdf)
- [ST7789 数据手册](https://www.displayfuture.com/Display/datasheet/controller/ST7789.pdf)
- [PlatformIO 文档](https://docs.platformio.org/)

##  常见问题

### Q: 显示屏显示偏移或颜色异常
**A**: 不同模块可能需要调整 `st7789.h` 中的配置：
```c
#define ST7789_X_OFFSET 0
#define ST7789_Y_OFFSET 0
#define ST7789_MADCTL_VALUE 0x00U  // 调整显示方向
```

### Q: 雷达扫描不流畅
**A**: 确保 SysTick 中断正常工作，检查 `stm32f10x_it.c` 中 `SysTick_Handler()` 是否调用 `Delay_IncTick()`。

### Q: 代码滚动有残影
**A**: 这是正常的部分刷新策略，如果残影严重可以降低滚动速度。

### Q: 内存不足
**A**: 当前配置使用 31.8% RAM，如果添加功能导致内存不足，可以减少雷达缓冲区大小或代码行数。

##  贡献

欢迎提交 Issue 和 Pull Request！

##  许可证

MIT License - 详见 [LICENSE](LICENSE) 文件

##  致谢

- STM32 标准外设库
- PlatformIO 开发平台
- ST7789 显示驱动

---

**Made with ❤️ for embedded developers**
