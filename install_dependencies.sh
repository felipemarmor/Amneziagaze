#!/bin/bash

echo "==================================================="
echo "FuzzEffect VST Plugin - Dependency Installer"
echo "==================================================="
echo
echo "This script will help you install the required dependencies:"
echo "1. CMake - Required for building the plugin"
echo "2. C++ Compiler - Required for compiling C++ code"
echo "3. Other build tools - Required for the build process"
echo
echo "Press Ctrl+C at any time to cancel."
echo
read -p "Press Enter to continue..."

# Detect OS
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    echo
    echo "Detected macOS system."
    echo
    
    # Check if Homebrew is installed
    if ! command -v brew &> /dev/null; then
        echo "Homebrew is not installed. Installing Homebrew..."
        echo
        
        # Install Homebrew
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
        
        if [ $? -ne 0 ]; then
            echo
            echo "Failed to install Homebrew. Please install it manually:"
            echo "https://brew.sh/"
            echo
            echo "After installing Homebrew, run this script again."
            read -p "Press Enter to exit..."
            exit 1
        fi
        
        echo
        echo "Homebrew installed successfully."
        echo
    else
        echo "Homebrew is already installed."
    fi
    
    echo
    echo "Installing CMake..."
    brew install cmake
    
    if [ $? -ne 0 ]; then
        echo
        echo "Failed to install CMake. Please install it manually:"
        echo "brew install cmake"
        echo
        echo "After installing CMake, run the setup_and_build_all.sh script again."
        read -p "Press Enter to exit..."
        exit 1
    fi
    
    echo
    echo "Installing Xcode Command Line Tools..."
    xcode-select --install
    
    # This might fail if already installed, so we don't check the return code
    
    echo
    echo "Installing other build tools..."
    brew install pkg-config
    
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    echo
    echo "Detected Linux system."
    echo
    
    # Check for package manager
    if command -v apt-get &> /dev/null; then
        # Debian/Ubuntu
        echo "Using apt package manager."
        
        echo
        echo "Updating package lists..."
        sudo apt-get update
        
        echo
        echo "Installing CMake and build tools..."
        sudo apt-get install -y cmake build-essential pkg-config
        
        if [ $? -ne 0 ]; then
            echo
            echo "Failed to install dependencies. Please install them manually:"
            echo "sudo apt-get install cmake build-essential pkg-config"
            echo
            echo "After installing the dependencies, run the setup_and_build_all.sh script again."
            read -p "Press Enter to exit..."
            exit 1
        fi
        
    elif command -v dnf &> /dev/null; then
        # Fedora/RHEL
        echo "Using dnf package manager."
        
        echo
        echo "Installing CMake and build tools..."
        sudo dnf install -y cmake gcc-c++ make pkg-config
        
        if [ $? -ne 0 ]; then
            echo
            echo "Failed to install dependencies. Please install them manually:"
            echo "sudo dnf install cmake gcc-c++ make pkg-config"
            echo
            echo "After installing the dependencies, run the setup_and_build_all.sh script again."
            read -p "Press Enter to exit..."
            exit 1
        fi
        
    elif command -v pacman &> /dev/null; then
        # Arch Linux
        echo "Using pacman package manager."
        
        echo
        echo "Installing CMake and build tools..."
        sudo pacman -S --noconfirm cmake base-devel pkg-config
        
        if [ $? -ne 0 ]; then
            echo
            echo "Failed to install dependencies. Please install them manually:"
            echo "sudo pacman -S cmake base-devel pkg-config"
            echo
            echo "After installing the dependencies, run the setup_and_build_all.sh script again."
            read -p "Press Enter to exit..."
            exit 1
        fi
        
    else
        echo
        echo "Unsupported Linux distribution. Please install the following packages manually:"
        echo "- CMake"
        echo "- C++ compiler (GCC or Clang)"
        echo "- Make"
        echo "- pkg-config"
        echo
        echo "After installing the dependencies, run the setup_and_build_all.sh script again."
        read -p "Press Enter to exit..."
        exit 1
    fi
    
else
    echo
    echo "Unsupported operating system. Please install the following packages manually:"
    echo "- CMake"
    echo "- C++ compiler"
    echo "- Make"
    echo "- pkg-config"
    echo
    echo "After installing the dependencies, run the setup_and_build_all.sh script again."
    read -p "Press Enter to exit..."
    exit 1
fi

echo
echo "==================================================="
echo "Dependencies installed successfully!"
echo "==================================================="
echo
echo "Now you can run the setup_and_build_all.sh script to build the plugin."
echo

read -p "Press Enter to exit..."