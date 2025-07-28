#!/bin/bash

echo "==================================================="
echo "FuzzEffect VST Plugin - Complete Setup and Build"
echo "==================================================="
echo
echo "This script will:"
echo "1. Download the VST3 SDK"
echo "2. Build the plugin"
echo "3. Install it to your VST3 directory"
echo
echo "Press Ctrl+C at any time to cancel."
echo
read -p "Press Enter to continue..."

# Step 1: Download the SDK
echo
echo "==================================================="
echo "Step 1: Downloading VST3 SDK"
echo "==================================================="
echo

# Create lib directory if it doesn't exist
mkdir -p lib
cd lib

# Check if VST3 SDK already exists
if [ -d "vst3sdk" ]; then
    echo "VST3 SDK already exists in lib/vst3sdk"
    echo
    read -p "Do you want to overwrite it? (Y/N): " OVERWRITE
    if [[ $OVERWRITE != "Y" && $OVERWRITE != "y" ]]; then
        echo "Using existing SDK."
    else
        echo
        echo "Removing existing SDK..."
        rm -rf vst3sdk
        
        echo
        echo "Creating vst3sdk directory..."
        mkdir -p vst3sdk
        
        # Download the SDK from GitHub
        echo
        echo "Downloading VST3 SDK from GitHub..."
        echo "This may take a few minutes depending on your internet connection."
        cd ..
        
        # Check if curl or wget is available
        DOWNLOAD_CMD=""
        if command -v curl &> /dev/null; then
            DOWNLOAD_CMD="curl -L -o sdk_temp.zip"
        elif command -v wget &> /dev/null; then
            DOWNLOAD_CMD="wget -O sdk_temp.zip"
        else
            echo
            echo "Error: Neither curl nor wget is available."
            echo "Please install one of these tools and try again, or download the SDK manually."
            echo
            echo "Please download the SDK manually from:"
            echo "https://www.steinberg.net/developers/"
            echo
            echo "After downloading, extract the contents to lib/vst3sdk"
            echo "Then run this script again."
            read -p "Press Enter to exit..."
            exit 1
        fi
        
        # Download the SDK
        $DOWNLOAD_CMD "https://github.com/steinbergmedia/vst3sdk/archive/refs/heads/master.zip"
        
        if [ $? -ne 0 ]; then
            echo
            echo "Failed to download the SDK automatically."
            echo
            echo "Please download the SDK manually from:"
            echo "https://www.steinberg.net/developers/"
            echo
            echo "After downloading, extract the contents to lib/vst3sdk"
            echo "Then run this script again."
            read -p "Press Enter to exit..."
            exit 1
        fi
        
        echo
        echo "Extracting SDK..."
        
        # Check if unzip is available
        if command -v unzip &> /dev/null; then
            unzip -q sdk_temp.zip
        else
            echo
            echo "Error: unzip is not available."
            echo "Please install unzip and try again, or extract the SDK manually."
            echo
            echo "Please download the SDK manually from:"
            echo "https://www.steinberg.net/developers/"
            echo
            echo "After downloading, extract the contents to lib/vst3sdk"
            echo "Then run this script again."
            rm -f sdk_temp.zip
            read -p "Press Enter to exit..."
            exit 1
        fi
        
        if [ $? -ne 0 ]; then
            echo
            echo "Failed to extract the SDK."
            echo
            echo "Please download the SDK manually from:"
            echo "https://www.steinberg.net/developers/"
            echo
            echo "After downloading, extract the contents to lib/vst3sdk"
            echo "Then run this script again."
            rm -f sdk_temp.zip
            read -p "Press Enter to exit..."
            exit 1
        fi
        
        # Move the extracted files to the correct location
        echo
        echo "Moving files to lib/vst3sdk..."
        cp -R vst3sdk-master/* lib/vst3sdk/
        
        # Clean up temporary files
        echo
        echo "Cleaning up temporary files..."
        rm -rf vst3sdk-master
        rm -f sdk_temp.zip
        
        echo
        echo "VST3 SDK has been downloaded and installed to lib/vst3sdk"
    fi
else
    echo "VST3 SDK not found. Downloading..."
    
    echo
    echo "Creating vst3sdk directory..."
    mkdir -p vst3sdk
    cd ..
    
    # Download the SDK from GitHub
    echo
    echo "Downloading VST3 SDK from GitHub..."
    echo "This may take a few minutes depending on your internet connection."
    
    # Check if curl or wget is available
    DOWNLOAD_CMD=""
    if command -v curl &> /dev/null; then
        DOWNLOAD_CMD="curl -L -o sdk_temp.zip"
    elif command -v wget &> /dev/null; then
        DOWNLOAD_CMD="wget -O sdk_temp.zip"
    else
        echo
        echo "Error: Neither curl nor wget is available."
        echo "Please install one of these tools and try again, or download the SDK manually."
        echo
        echo "Please download the SDK manually from:"
        echo "https://www.steinberg.net/developers/"
        echo
        echo "After downloading, extract the contents to lib/vst3sdk"
        echo "Then run this script again."
        read -p "Press Enter to exit..."
        exit 1
    fi
    
    # Download the SDK
    $DOWNLOAD_CMD "https://github.com/steinbergmedia/vst3sdk/archive/refs/heads/master.zip"
    
    if [ $? -ne 0 ]; then
        echo
        echo "Failed to download the SDK automatically."
        echo
        echo "Please download the SDK manually from:"
        echo "https://www.steinberg.net/developers/"
        echo
        echo "After downloading, extract the contents to lib/vst3sdk"
        echo "Then run this script again."
        read -p "Press Enter to exit..."
        exit 1
    fi
    
    echo
    echo "Extracting SDK..."
    
    # Check if unzip is available
    if command -v unzip &> /dev/null; then
        unzip -q sdk_temp.zip
    else
        echo
        echo "Error: unzip is not available."
        echo "Please install unzip and try again, or extract the SDK manually."
        echo
        echo "Please download the SDK manually from:"
        echo "https://www.steinberg.net/developers/"
        echo
        echo "After downloading, extract the contents to lib/vst3sdk"
        echo "Then run this script again."
        rm -f sdk_temp.zip
        read -p "Press Enter to exit..."
        exit 1
    fi
    
    if [ $? -ne 0 ]; then
        echo
        echo "Failed to extract the SDK."
        echo
        echo "Please download the SDK manually from:"
        echo "https://www.steinberg.net/developers/"
        echo
        echo "After downloading, extract the contents to lib/vst3sdk"
        echo "Then run this script again."
        rm -f sdk_temp.zip
        read -p "Press Enter to exit..."
        exit 1
    fi
    
    # Move the extracted files to the correct location
    echo
    echo "Moving files to lib/vst3sdk..."
    cp -R vst3sdk-master/* lib/vst3sdk/
    
    # Clean up temporary files
    echo
    echo "Cleaning up temporary files..."
    rm -rf vst3sdk-master
    rm -f sdk_temp.zip
    
    echo
    echo "VST3 SDK has been downloaded and installed to lib/vst3sdk"
fi

# Make sure we're in the project root
cd "$(dirname "$0")"

# Step 2: Build the plugin
echo
echo "==================================================="
echo "Step 2: Building the plugin"
echo "==================================================="
echo

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
    cd ..
    read -p "Press Enter to exit..."
    exit 1
fi

# Step 3: Install the plugin
echo
echo "==================================================="
echo "Step 3: Installing the plugin"
echo "==================================================="
echo

# Determine VST3 directory based on OS
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    VST3_DIR="/Library/Audio/Plug-Ins/VST3"
    sudo mkdir -p "$VST3_DIR"
    echo "Copying plugin to $VST3_DIR..."
    sudo cp -r "VST3/Release/FuzzEffect.vst3" "$VST3_DIR/"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    VST3_DIR="/usr/local/lib/vst3"
    sudo mkdir -p "$VST3_DIR"
    echo "Copying plugin to $VST3_DIR..."
    sudo cp -r "VST3/FuzzEffect.vst3" "$VST3_DIR/"
else
    echo "Unsupported operating system. Please manually copy the plugin to your VST3 directory."
    VST3_DIR="unknown"
fi

# Return to project root
cd ..

echo
echo "==================================================="
echo "Setup and build completed successfully!"
echo "==================================================="
echo
echo "The FuzzEffect VST plugin has been:"
echo "1. Built successfully"
echo "2. Installed to $VST3_DIR"
echo
echo "To use the plugin:"
echo "1. Open your DAW (Reaper, Cubase, FL Studio, etc.)"
echo "2. Scan for new plugins or refresh plugin list"
echo "3. Add \"FuzzEffect\" as an insert effect on an audio track"
echo
echo "Enjoy your new fuzz effect plugin!"
echo

read -p "Press Enter to exit..."