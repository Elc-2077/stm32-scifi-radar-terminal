"""
STM32 RGB565图片转换工具
将PNG/JPG/GIF图片转换为RGB565格式的C头文件
支持静态图片和GIF动画
"""

from PIL import Image
import sys
import os
import numpy as np

def rgb888_to_rgb565_swapped(r, g, b):
    """将RGB888转换为RGB565格式（字节序交换以适配ST7789）"""
    rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
    # 交换高低字节
    return ((rgb565 & 0xFF) << 8) | ((rgb565 >> 8) & 0xFF)

def convert_static_image(input_path, output_path=None, var_name=None):
    """转换静态图片"""

    if not os.path.exists(input_path):
        print(f"错误: 文件不存在 {input_path}")
        return False

    try:
        img = Image.open(input_path).convert('RGB')
        print(f"读取图片: {input_path}")
        print(f"  尺寸: {img.size[0]}x{img.size[1]}")
    except Exception as e:
        print(f"错误: 无法读取图片 - {e}")
        return False

    width, height = img.size

    if output_path is None:
        base_name = os.path.splitext(os.path.basename(input_path))[0]
        output_path = f"{base_name}.h"

    if var_name is None:
        base_name = os.path.splitext(os.path.basename(input_path))[0]
        var_name = base_name.replace('-', '_').replace(' ', '_').lower()

    guard_name = var_name.upper() + "_H"

    print(f"\n生成头文件: {output_path}")

    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(f'#ifndef {guard_name}\n')
        f.write(f'#define {guard_name}\n\n')
        f.write('#include "stm32f10x.h"\n\n')
        f.write(f'#define {var_name.upper()}_WIDTH {width}U\n')
        f.write(f'#define {var_name.upper()}_HEIGHT {height}U\n\n')
        f.write(f'const uint16_t {var_name}_data[{height}][{width}] = {{\n')

        pixels = img.load()
        for y in range(height):
            f.write('{')
            for x in range(width):
                r, g, b = pixels[x, y]
                rgb565 = rgb888_to_rgb565_swapped(r, g, b)
                f.write(f'0x{rgb565:04X}')
                if x < width - 1:
                    f.write(',')
            f.write('}')
            if y < height - 1:
                f.write(',\n')
            else:
                f.write('\n')

            if (y + 1) % 20 == 0 or y == height - 1:
                progress = (y + 1) / height * 100
                print(f"  进度: {progress:.1f}%", end='\r')

        f.write('};\n\n')
        f.write(f'#endif // {guard_name}\n')

    size = width * height * 2
    print(f"\n✓ 转换完成! 大小: {size/1024:.1f}KB")
    return True

def convert_gif_animation(input_path, output_path=None, var_name=None, max_frames=None, resize=None):
    """转换GIF动画"""

    if not os.path.exists(input_path):
        print(f"错误: 文件不存在 {input_path}")
        return False

    try:
        gif = Image.open(input_path)
        print(f"读取GIF: {input_path}")

        # 提取所有帧
        frames = []
        frame_idx = 0
        try:
            while True:
                frame = gif.copy().convert('RGB')
                if resize:
                    frame = frame.resize(resize, Image.LANCZOS)
                frames.append(frame)
                gif.seek(gif.tell() + 1)
                frame_idx += 1
                if max_frames and len(frames) >= max_frames:
                    break
        except EOFError:
            pass

        print(f"  原始: {gif.size}, {len(frames)}帧")
        if resize:
            print(f"  缩放到: {resize}")

    except Exception as e:
        print(f"错误: 无法读取GIF - {e}")
        return False

    width, height = frames[0].size

    if output_path is None:
        base_name = os.path.splitext(os.path.basename(input_path))[0]
        output_path = f"{base_name}_anim.h"

    if var_name is None:
        base_name = os.path.splitext(os.path.basename(input_path))[0]
        var_name = base_name.replace('-', '_').replace(' ', '_').lower() + "_anim"

    guard_name = var_name.upper() + "_H"

    print(f"\n生成头文件: {output_path}")

    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(f'#ifndef {guard_name}\n')
        f.write(f'#define {guard_name}\n\n')
        f.write('#include "stm32f10x.h"\n\n')
        f.write(f'#define {var_name.upper()}_WIDTH {width}U\n')
        f.write(f'#define {var_name.upper()}_HEIGHT {height}U\n')
        f.write(f'#define {var_name.upper()}_FRAMES {len(frames)}U\n\n')
        f.write(f'const uint16_t {var_name}_data[{len(frames)}][{height}][{width}] = {{\n')

        for frame_idx, frame in enumerate(frames):
            pixels = np.array(frame)

            f.write('{\n')
            for y in range(height):
                f.write('{')
                for x in range(width):
                    r, g, b = pixels[y, x]
                    rgb565 = rgb888_to_rgb565_swapped(r, g, b)
                    f.write(f'0x{rgb565:04X}')
                    if x < width - 1:
                        f.write(',')
                f.write('}')
                if y < height - 1:
                    f.write(',\n')
                else:
                    f.write('\n')
            f.write('}')
            if frame_idx < len(frames) - 1:
                f.write(',\n')
            else:
                f.write('\n')

            print(f"  进度: {frame_idx+1}/{len(frames)}帧", end='\r')

        f.write('};\n\n')
        f.write(f'#endif // {guard_name}\n')

    size = width * height * len(frames) * 2
    print(f"\n✓ 转换完成! 大小: {size/1024:.1f}KB")
    return True

def main():
    print("=" * 60)
    print("STM32 RGB565图片转换工具")
    print("=" * 60)

    if len(sys.argv) < 2:
        print("\n用法:")
        print(f"  python {sys.argv[0]} <图片路径> [选项]")
        print(f"\n静态图片示例:")
        print(f"  python {sys.argv[0]} image.png")
        print(f"  python {sys.argv[0]} image.jpg output.h my_image")
        print(f"\nGIF动画示例:")
        print(f"  python {sys.argv[0]} animation.gif")
        print(f"  python {sys.argv[0]} animation.gif --frames 10")
        print(f"  python {sys.argv[0]} animation.gif --resize 80x80 --frames 15")
        return

    input_path = sys.argv[1]

    # 检查是否是GIF
    try:
        img = Image.open(input_path)
        is_gif = img.format == 'GIF' and hasattr(img, 'n_frames') and img.n_frames > 1
        img.close()
    except:
        print(f"错误: 无法打开文件 {input_path}")
        return

    if is_gif:
        # GIF动画处理
        max_frames = None
        resize = None

        for i, arg in enumerate(sys.argv[2:], 2):
            if arg == '--frames' and i+1 < len(sys.argv):
                max_frames = int(sys.argv[i+1])
            elif arg == '--resize' and i+1 < len(sys.argv):
                size_str = sys.argv[i+1]
                w, h = map(int, size_str.split('x'))
                resize = (w, h)

        print()
        convert_gif_animation(input_path, max_frames=max_frames, resize=resize)
    else:
        # 静态图片处理
        output_path = sys.argv[2] if len(sys.argv) > 2 else None
        var_name = sys.argv[3] if len(sys.argv) > 3 else None

        print()
        convert_static_image(input_path, output_path, var_name)

    print("\n" + "=" * 60)

if __name__ == "__main__":
    main()
