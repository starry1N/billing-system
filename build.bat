@echo off
chcp 65001 > nul
cd /d "%~dp0"

echo ============================================================
echo   计费管理系统  —  编译脚本
echo ============================================================
echo.

set GCC=D:\mingw64\bin\gcc.exe

echo 正在编译...

%GCC% ^
  src/main.c ^
  src/dao/tool.c ^
  src/dao/card_file.c ^
  src/dao/billing_file.c ^
  src/dao/record_file.c ^
  src/dao/rate_file.c ^
  src/service/card_service.c ^
  src/service/billing_service.c ^
  src/service/record_service.c ^
  src/service/rate_service.c ^
  src/menu/menu.c ^
  -o AMS.exe ^
  -finput-charset=UTF-8 ^
  -fexec-charset=GBK ^
  -Wall -w

if %errorlevel% == 0 (
    echo 编译成功！可执行文件: AMS.exe
    echo.
    echo 正在启动程序...
    echo ============================================================
    AMS.exe
) else (
    echo.
    echo 编译失败，请检查以上错误信息。
    pause
)
