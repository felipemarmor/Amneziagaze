#!/bin/bash

echo "==================================================="
echo "FuzzEffect VST Plugin Build Script"
echo "==================================================="
echo

# Create lib directory if it doesn't exist
mkdir -p lib
cd lib

# Check if VST3 SDK exists
if [ -d "vst3sdk" ]; then
    echo "VST3 SDK found in lib/vst3sdk"
else
    echo "VST3 SDK not found in lib/vst3sdk"
    echo
    echo "You need to download the Steinberg VST3 SDK manually:"
    echo "1. Visit https://www.steinberg.net/developers/"
    echo "2. Download the latest VST3 SDK"
    echo "3. Extract the contents to lib/vst3sdk"
    echo
    
    read -p "Do you want to open the Steinberg developer website now? (Y/N): " OPEN_BROWSER
    if [[ $OPEN_BROWSER == "Y" || $OPEN_BROWSER == "y" ]]; then
        if [[ "$OSTYPE" == "darwin"* ]]; then
            # macOS
            open "https://www.steinberg.net/developers/"
        elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
            # Linux
            xdg-open "https://www.steinberg.net/developers/"
        else
            echo "Please open https://www.steinberg.net/developers/ in your browser"
        fi
    fi
    
    echo
    echo "Please download and extract the SDK, then run this script again."
    read -p "Press Enter to exit..."
    exit 1
fi

# Return to project root
cd ..

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo
echo "Configuring project with CMake..."
cmake ..

# Build the plugin
echo
echo "Building the plugin..."
cmake --build . --config Release

# Check if build was successful
if [ $? -ne 0 ]; then
    echo
    echo "Build failed. Please check the error messages above."
    read -p "Press Enter to exit..."
    exit 1
fi

# Copy the plugin to the VST3 directory
echo
echo "Copying plugin to VST3 directory..."

# Determine VST3 directory based on OS
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    VST3_DIR="/Library/Audio/Plug-Ins/VST3"
    sudo mkdir -p "$VST3_DIR"
    sudo cp -r "VST3/Release/FuzzEffect.vst3" "$VST3_DIR/"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    VST3_DIR="/usr/local/lib/vst3"
    sudo mkdir -p "$VST3_DIR"
    sudo cp -r "VST3/FuzzEffect.vst3" "$VST3_DIR/"
else
    echo "Unsupported operating system. Please manually copy the plugin to your VST3 directory."
    VST3_DIR="unknown"
fi

echo
echo "Plugin has been built and installed to $VST3_DIR"
echo
echo "To use the plugin:"
echo "1. Open your DAW (Reaper, Cubase, FL Studio, etc.)"
echo "2. Scan for new plugins or refresh plugin list"
echo "3. Add \"FuzzEffect\" as an insert effect on an audio track"
echo

read -p "Press Enter to exit..."