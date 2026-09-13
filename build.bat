@echo off
REM STM32F103RCT6 模板快速编译脚本 (Windows版本)

setlocal enabledelayedexpansion

set PIO=C:\Users\HP\.platformio\penv\Scripts\platformio.exe

REM 显示帮助
if "%1"=="-h" goto :help
if "%1"=="--help" goto :help
if "%1"=="/?" goto :help

REM 列出环境
if "%1"=="-l" goto :list
if "%1"=="--list" goto :list

REM 清理
if "%1"=="-c" goto :clean
if "%1"=="--clean" goto :clean

REM 检查参数
if "%1"=="" goto :noenv

REM 检查是否上传
set UPLOAD=0
set ENV=%1
if "%1"=="-u" (
    set UPLOAD=1
    set ENV=%2
)
if "%1"=="--upload" (
    set UPLOAD=1
    set ENV=%2
)

REM 验证环境名称
if "%ENV%"=="hello_world" goto :build
if "%ENV%"=="pd_animation" goto :build
if "%ENV%"=="thermal_camera" goto :build

echo [错误] 未知的环境 '%ENV%'
echo.
goto :list

:build
echo [编译] 环境: %ENV%
echo.
if %UPLOAD%==1 (
    echo [模式] 编译并上传
    "%PIO%" run -e %ENV% -t upload
) else (
    "%PIO%" run -e %ENV%
)

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [成功] 编译完成！
    if %UPLOAD%==1 echo [成功] 上传完成！
) else (
    echo.
    echo [失败] 编译失败！
    exit /b 1
)
goto :end

:help
echo STM32F103RCT6 模板编译工具
echo.
echo 用法: build.bat [选项] [环境名称]
echo.
echo 选项:
echo   -h, --help          显示帮助信息
echo   -l, --list          列出所有可用环境
echo   -u, --upload        编译并上传
echo   -c, --clean         清理编译输出
echo.
echo 可用环境:
echo   hello_world         Hello World 示例
echo   pd_animation        PD 动画播放器
echo   thermal_camera      热成像相机
echo.
echo 示例:
echo   build.bat hello_world              # 编译 Hello World
echo   build.bat -u pd_animation          # 编译并上传 PD 动画
echo   build.bat -c                       # 清理所有编译输出
goto :end

:list
echo 可用的编译环境:
echo.
echo 1. hello_world     - Hello World 示例 (2.5KB)
echo 2. pd_animation    - PD 动画播放器 (130KB)
echo 3. thermal_camera  - 热成像相机 (模板)
echo.
goto :end

:clean
echo [清理] 正在清理编译输出...
"%PIO%" run -t clean
echo [完成] 清理完成！
goto :end

:noenv
echo [错误] 请指定编译环境
echo.
goto :help

:end
endlocal
