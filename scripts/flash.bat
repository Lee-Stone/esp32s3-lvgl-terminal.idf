@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ========================================
echo    ESP32-S3 Firmware Flash Tool
echo ========================================
echo.

:: Scan for available COM ports
echo Scanning available COM ports...
echo.

:: Use PowerShell script to get COM ports with device names
set "tempFile=%temp%\comports_list.txt"
powershell -ExecutionPolicy Bypass -File "%~dp0get_ports.ps1" > "%tempFile%"

:: Read COM port list
set count=0
for /f "usebackq tokens=1,2 delims=|" %%a in ("%tempFile%") do (
    set /a count+=1
    set "port!count!=%%a"
    set "name!count!=%%b"
)

del "%tempFile%" 2>nul

if !count! equ 0 (
    echo [Error] No COM ports detected!
    echo.
    echo Please check:
    echo 1. ESP32-S3 is connected to the computer
    echo 2. USB driver is properly installed
    echo.
    pause
    exit /b 1
)

:: Display all available COM ports
echo Detected COM ports:
echo.
for /l %%i in (1,1,!count!) do (
    echo [%%i] !port%%i! - !name%%i!
)
echo.

:: User selects COM port
:select_port
set /p choice="Please enter the COM port number: "

:: Validate input
if "!choice!"=="" goto select_port
if !choice! lss 1 goto invalid_choice
if !choice! gtr !count! goto invalid_choice

set selected_port=!port%choice%!
set selected_name=!name%choice%!

echo.
echo Selected: !selected_port! - !selected_name!
echo.

:: Confirm flashing
set /p confirm="Confirm flash to this port? (Y/N): "
if /i not "!confirm!"=="Y" (
    if /i not "!confirm!"=="y" (
        echo Flash cancelled.
        pause
        exit /b 0
    )
)

echo.
echo ========================================
echo    Starting firmware flash...
echo ========================================
echo.
echo Port: !selected_port! - !selected_name!
echo Chip: ESP32-S3
echo Baud Rate: 921600
echo.

:: Execute flashing
"%~dp0esptool-windows-amd64\esptool.exe" --chip esp32s3 --port !selected_port! --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 16MB 0x0 "%~dp0bin\bootloader.bin" 0x8000 "%~dp0bin\partition-table.bin" 0xe000 "%~dp0bin\ota_data_initial.bin" 0x10000 "%~dp0bin\esp32s3-lvgl-terminal.idf.bin"

if errorlevel 1 (
    echo.
    echo ========================================
    echo    Flash Failed!
    echo ========================================
    echo.
    echo Possible reasons:
    echo 1. COM port is in use by another program
    echo 2. ESP32-S3 not in download mode
    echo 3. Unstable USB connection
    echo.
    pause
    exit /b 1
) else (
    echo.
    echo ========================================
    echo    Flash Successful!
    echo ========================================
    echo.
    echo Device will restart automatically...
    echo.
    pause
    exit /b 0
)

:invalid_choice
echo [Error] Invalid choice, please enter a number between 1 and !count!.
echo.
goto select_port
