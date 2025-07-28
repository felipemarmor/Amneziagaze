@echo off
echo ===================================================
echo FuzzEffect VST Plugin Build Script
echo ===================================================
echo.

REM Create lib directory if it doesn't exist
if not exist lib mkdir lib
cd lib

REM Check if VST3 SDK exists
if exist vst3sdk (
    echo VST3 SDK found in lib/vst3sdk
) else (
    echo VST3 SDK not found in lib/vst3sdk
    echo.
    echo You need to download the Steinberg VST3 SDK manually:
    echo 1. Visit https://www.steinberg.net/developers/
    echo 2. Download the latest VST3 SDK
    echo 3. Extract the contents to lib/vst3sdk
    echo.
    
    set /p OPEN_BROWSER="Do you want to open the Steinberg developer website now? (Y/N): "
    if /i "%OPEN_BROWSER%"=="Y" (
        start "" "https://www.steinberg.net/developers/"
    )
    
    echo.
    echo Please download and extract the SDK, then run this script again.
    pause
    exit /b 1
)

REM Return to project root
cd ..

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo.
echo Configuring project with CMake...
cmake ..

REM Build the plugin
echo.
echo Building the plugin...
cmake --build . --config Release

REM Check if build was successful
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Build failed. Please check the error messages above.
    pause
    exit /b 1
)

REM Copy the plugin to the VST3 directory
echo.
echo Copying plugin to VST3 directory...

REM Determine VST3 directory based on OS
set VST3_DIR="%PROGRAMFILES%\Common Files\VST3"
if not exist %VST3_DIR% mkdir %VST3_DIR%

REM Copy the plugin
copy /Y "VST3\Release\FuzzEffect.vst3" %VST3_DIR%

echo.
echo Plugin has been built and installed to %VST3_DIR%
echo.
echo To use the plugin:
echo 1. Open your DAW (Reaper, Cubase, FL Studio, etc.)
echo 2. Scan for new plugins or refresh plugin list
echo 3. Add "FuzzEffect" as an insert effect on an audio track
echo.

pause