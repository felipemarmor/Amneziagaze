@echo off
echo ===================================================
echo VST3 SDK Automatic Download Script
echo ===================================================
echo.
echo This script will attempt to download the VST3 SDK from GitHub.
echo Note: The official SDK should be downloaded from Steinberg's website
echo       after accepting their license agreement.
echo.

REM Create lib directory if it doesn't exist
if not exist lib mkdir lib
cd lib

REM Check if VST3 SDK already exists
if exist vst3sdk (
    echo VST3 SDK already exists in lib/vst3sdk
    echo.
    set /p OVERWRITE="Do you want to overwrite it? (Y/N): "
    if /i not "%OVERWRITE%"=="Y" (
        echo.
        echo Download cancelled.
        cd ..
        pause
        exit /b 0
    )
    
    echo.
    echo Removing existing SDK...
    rmdir /s /q vst3sdk
)

echo.
echo Creating vst3sdk directory...
mkdir vst3sdk
cd ..

REM Download the SDK from GitHub
echo.
echo Downloading VST3 SDK from GitHub...
echo This may take a few minutes depending on your internet connection.

REM Use PowerShell to download the SDK
powershell -Command "& {Invoke-WebRequest -Uri 'https://github.com/steinbergmedia/vst3sdk/archive/refs/heads/master.zip' -OutFile 'sdk_temp.zip'}"

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Failed to download the SDK.
    echo.
    echo Please download the SDK manually from:
    echo https://www.steinberg.net/developers/
    echo.
    echo After downloading, extract the contents to lib/vst3sdk
    pause
    exit /b 1
)

echo.
echo Extracting SDK...
powershell -Command "& {Expand-Archive -Path 'sdk_temp.zip' -DestinationPath '.' -Force}"

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Failed to extract the SDK.
    echo.
    echo Please download the SDK manually from:
    echo https://www.steinberg.net/developers/
    echo.
    echo After downloading, extract the contents to lib/vst3sdk
    del /q sdk_temp.zip
    pause
    exit /b 1
)

REM Move the extracted files to the correct location
echo.
echo Moving files to lib/vst3sdk...
xcopy /E /I /Y "vst3sdk-master\*" "lib\vst3sdk\"

REM Clean up temporary files
echo.
echo Cleaning up temporary files...
rmdir /s /q vst3sdk-master
del /q sdk_temp.zip

echo.
echo VST3 SDK has been downloaded and installed to lib/vst3sdk
echo.
echo You can now build the plugin using the build_plugin script.
echo.

pause