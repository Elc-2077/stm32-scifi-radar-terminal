#!/bin/bash
# STM32F103RCT6 模板快速编译脚本

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 显示帮助信息
show_help() {
    echo -e "${BLUE}STM32F103RCT6 模板编译工具${NC}"
    echo ""
    echo "用法: ./build.sh [选项] [环境名称]"
    echo ""
    echo "选项:"
    echo "  -h, --help          显示帮助信息"
    echo "  -l, --list          列出所有可用环境"
    echo "  -u, --upload        编译并上传"
    echo "  -c, --clean         清理编译输出"
    echo "  -v, --verbose       显示详细编译信息"
    echo ""
    echo "可用环境:"
    echo "  hello_world         Hello World 示例"
    echo "  pd_animation        PD 动画播放器"
    echo "  thermal_camera      热成像相机"
    echo ""
    echo "示例:"
    echo "  ./build.sh hello_world              # 编译 Hello World"
    echo "  ./build.sh -u pd_animation          # 编译并上传 PD 动画"
    echo "  ./build.sh -c                       # 清理所有编译输出"
}

# 列出所有环境
list_environments() {
    echo -e "${BLUE}可用的编译环境:${NC}"
    echo ""
    echo -e "${GREEN}1. hello_world${NC}     - Hello World 示例 (2.5KB)"
    echo -e "${GREEN}2. pd_animation${NC}    - PD 动画播放器 (130KB)"
    echo -e "${GREEN}3. thermal_camera${NC}  - 热成像相机 (模板)"
    echo ""
}

# PlatformIO 命令路径
PIO="/c/Users/HP/.platformio/penv/Scripts/platformio.exe"

# 解析参数
UPLOAD=false
CLEAN=false
VERBOSE=""
ENV=""

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -l|--list)
            list_environments
            exit 0
            ;;
        -u|--upload)
            UPLOAD=true
            shift
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -v|--verbose)
            VERBOSE="-v"
            shift
            ;;
        *)
            ENV=$1
            shift
            ;;
    esac
done

# 清理模式
if [ "$CLEAN" = true ]; then
    echo -e "${YELLOW}正在清理编译输出...${NC}"
    $PIO run -t clean
    echo -e "${GREEN}清理完成！${NC}"
    exit 0
fi

# 检查是否指定了环境
if [ -z "$ENV" ]; then
    echo -e "${RED}错误: 请指定编译环境${NC}"
    echo ""
    show_help
    exit 1
fi

# 验证环境名称
case $ENV in
    hello_world|pd_animation|thermal_camera)
        ;;
    *)
        echo -e "${RED}错误: 未知的环境 '$ENV'${NC}"
        echo ""
        list_environments
        exit 1
        ;;
esac

# 编译
echo -e "${BLUE}正在编译环境: ${GREEN}$ENV${NC}"
echo ""

if [ "$UPLOAD" = true ]; then
    echo -e "${YELLOW}编译并上传模式${NC}"
    $PIO run -e $ENV -t upload $VERBOSE
else
    $PIO run -e $ENV $VERBOSE
fi

# 检查编译结果
if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}✅ 编译成功！${NC}"
    if [ "$UPLOAD" = true ]; then
        echo -e "${GREEN}✅ 上传成功！${NC}"
    fi
else
    echo ""
    echo -e "${RED}❌ 编译失败！${NC}"
    exit 1
fi
