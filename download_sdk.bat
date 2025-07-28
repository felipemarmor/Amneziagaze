@echo off
echo ===================================================
echo VST3 SDK Download Helper
echo ===================================================
echo.
echo This script will help you download and set up the Steinberg VST3 SDK.
echo.
echo NOTE: You need to manually download the SDK from the Steinberg website
echo       as it requires accepting a license agreement.
echo.
echo Steps:
echo 1. Visit https://www.steinberg.net/developers/
echo 2. Download the latest VST3 SDK
echo 3. Extract the downloaded ZIP file
echo 4. Copy the "VST3_SDK" folder to the "lib" directory of this project
echo.
echo ===================================================
echo.

set /p CONTINUE=Do you want to open the Steinberg developer website now? (Y/N): 

if /i "%CONTINUE%"=="Y" (
    start "" "https://www.steinberg.net/developers/"
    echo.
    echo Browser opened to Steinberg developer website.
    echo.
    echo After downloading and extracting the SDK:
    echo 1. Create a folder named "vst3sdk" in the "lib" directory
    echo 2. Copy the contents of the extracted "VST3_SDK" folder into "lib/vst3sdk"
    echo 3. Update the VST3_SDK_ROOT path in CMakeLists.txt if needed
    echo.
) else (
    echo.
    echo Operation cancelled.
    echo.
)

echo ===================================================
echo.
echo After setting up the SDK, you can build the plugin using CMake:
echo.
echo mkdir build
echo cd build
echo cmake ..
echo cmake --build . --config Release
echo.
echo ===================================================

pause