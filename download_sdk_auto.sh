#!/bin/bash

echo "==================================================="
echo "VST3 SDK Automatic Download Script"
echo "==================================================="
echo
echo "This script will attempt to download the VST3 SDK from GitHub."
echo "Note: The official SDK should be downloaded from Steinberg's website"
echo "      after accepting their license agreement."
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
        echo
        echo "Download cancelled."
        cd ..
        read -p "Press Enter to exit..."
        exit 0
    fi
    
    echo
    echo "Removing existing SDK..."
    rm -rf vst3sdk
fi

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
    read -p "Press Enter to exit..."
    exit 1
fi

# Download the SDK
$DOWNLOAD_CMD "https://github.com/steinbergmedia/vst3sdk/archive/refs/heads/master.zip"

if [ $? -ne 0 ]; then
    echo
    echo "Failed to download the SDK."
    echo
    echo "Please download the SDK manually from:"
    echo "https://www.steinberg.net/developers/"
    echo
    echo "After downloading, extract the contents to lib/vst3sdk"
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
echo
echo "You can now build the plugin using the build_plugin script."
echo

read -p "Press Enter to exit..."