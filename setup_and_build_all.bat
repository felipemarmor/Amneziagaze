@echo off
echo ===================================================
echo AMNEZIAGAZE VST Plugin - Complete Setup and Build
echo ===================================================
echo.
echo This script will:
echo 1. Download the VST3 SDK
echo 2. Build the plugin
echo 3. Install it to your VST3 directory
echo.
echo Press Ctrl+C at any time to cancel.
echo.
pause

REM Step 1: Download the SDK
echo.
echo ===================================================
echo Step 1: Downloading VST3 SDK
echo ===================================================
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
        echo Using existing SDK.
    ) else (
        echo.
        echo Removing existing SDK...
        rmdir /s /q vst3sdk
        
        echo.
        echo Creating vst3sdk directory...
        mkdir vst3sdk
        
        REM Download the SDK from GitHub
        echo.
        echo Downloading VST3 SDK from GitHub...
        echo This may take a few minutes depending on your internet connection.
        cd ..
        
        REM Use PowerShell to download the SDK
        powershell -Command "& {Invoke-WebRequest -Uri 'https://github.com/steinbergmedia/vst3sdk/archive/refs/heads/master.zip' -OutFile 'sdk_temp.zip'}"
        
        if %ERRORLEVEL% NEQ 0 (
            echo.
            echo Failed to download the SDK automatically.
            echo.
            echo Please download the SDK manually from:
            echo https://www.steinberg.net/developers/
            echo.
            echo After downloading, extract the contents to lib/vst3sdk
            echo Then run this script again.
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
            echo Then run this script again.
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
    )
) else (
    echo VST3 SDK not found. Downloading...
    
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
        echo Failed to download the SDK automatically.
        echo.
        echo Please download the SDK manually from:
        echo https://www.steinberg.net/developers/
        echo.
        echo After downloading, extract the contents to lib/vst3sdk
        echo Then run this script again.
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
        echo Then run this script again.
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
)

REM Make sure we're in the project root
cd %~dp0

REM Step 2: Build the plugin
echo.
echo ===================================================
echo Step 2: Building the plugin
echo ===================================================
echo.

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo.
echo Configuring project with CMake...
"C:\Program Files\CMake\bin\cmake.exe" ..

REM Build the plugin
echo.
echo Building the plugin...
"C:\Program Files\CMake\bin\cmake.exe" --build . --config Release

REM Check if build was successful
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Build failed. Please check the error messages above.
    cd ..
    pause
    exit /b 1
)

REM Step 3: Install the plugin
echo.
echo ===================================================
echo Step 3: Installing the plugin
echo ===================================================
echo.

REM Determine VST3 directory based on OS
set VST3_DIR="%PROGRAMFILES%\Common Files\VST3"
if not exist %VST3_DIR% mkdir %VST3_DIR%

REM Copy the plugin to standard VST3 directory
echo Copying plugin to %VST3_DIR%...
copy /Y "Release\AMNEZIAGAZE.vst3" %VST3_DIR%

REM Copy the plugin to custom VST folder
set CUSTOM_VST_DIR="D:\Custom VST Folder"
if not exist %CUSTOM_VST_DIR% mkdir %CUSTOM_VST_DIR%
echo Copying plugin to %CUSTOM_VST_DIR%...
copy /Y "Release\AMNEZIAGAZE.vst3" %CUSTOM_VST_DIR%

REM Return to project root
cd ..

echo.
echo ===================================================
echo Setup and build completed successfully!
echo ===================================================
echo.
echo The AMNEZIAGAZE VST plugin has been:
echo 1. Built successfully
echo 2. Installed to %VST3_DIR%
echo 3. Installed to %CUSTOM_VST_DIR%
echo.
echo To use the plugin:
echo 1. Open your DAW (Reaper, Cubase, FL Studio, etc.)
echo 2. Scan for new plugins or refresh plugin list
echo 3. Add "AMNEZIAGAZE" as an insert effect on an audio track
echo.
echo Enjoy your new shoegaze guitar effect plugin!
echo.

pause