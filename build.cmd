@echo off
setlocal

if not exist build mkdir build

cl /nologo /std:c17 /W4 /O2 ^
    /I includes ^
    src\main.c ^
    src\stn_config.c ^
    src\stn_display.c ^
    src\stn_log.c ^
    src\stn_miner.c ^
    src\stn_protocol.c ^
    src\stn_hash.c ^
    src\stn_cpu.c ^
    platforms\windows\stn_platform_win32.c ^
    /Fe:build\stn-miner.exe ^
    /link ws2_32.lib user32.lib

if errorlevel 1 (
    echo.
    echo Build failed.
    exit /b 1
)

echo.
echo BUILD SUCCESSFUL
echo build\stn-miner.exe