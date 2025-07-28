@echo off
echo ===================================================
echo FuzzEffect VST Plugin - Dependency Installer
echo ===================================================
echo.
echo This script will help you install the required dependencies:
echo 1. CMake - Required for building the plugin
echo 2. Visual Studio Build Tools - Required for compiling C++ code
echo.
echo Press Ctrl+C at any time to cancel.
echo.
pause

REM Check if Chocolatey is installed
where choco >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Chocolatey is not installed. Installing Chocolatey...
    echo.
    
    REM Install Chocolatey
    @powershell -NoProfile -ExecutionPolicy Bypass -Command "iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))"
    
    if %ERRORLEVEL% NEQ 0 (
        echo.
        echo Failed to install Chocolatey. Please install it manually:
        echo https://chocolatey.org/install
        echo.
        echo After installing Chocolatey, run this script again.
        pause
        exit /b 1
    )
    
    echo.
    echo Chocolatey installed successfully.
    echo.
    
    REM Refresh environment variables
    call refreshenv
) else (
    echo Chocolatey is already installed.
)

echo.
echo Installing CMake...
choco install cmake -y

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Failed to install CMake. Please install it manually:
    echo https://cmake.org/download/
    echo.
    echo After installing CMake, run the setup_and_build_all.bat script again.
    pause
    exit /b 1
)

echo.
echo Installing Visual Studio Build Tools...
choco install visualstudio2019buildtools -y
choco install visualstudio2019-workload-vctools -y

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Failed to install Visual Studio Build Tools. Please install them manually:
    echo https://visualstudio.microsoft.com/downloads/
    echo.
    echo After installing Visual Studio Build Tools, run the setup_and_build_all.bat script again.
    pause
    exit /b 1
)

echo.
echo ===================================================
echo Dependencies installed successfully!
echo ===================================================
echo.
echo Now you can run the setup_and_build_all.bat script to build the plugin.
echo.

pause