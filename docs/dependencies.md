# VST Plugin Development Dependencies

This document outlines the dependencies required for VST plugin development.

## Core Dependencies

1. **Steinberg VST3 SDK**
   - The official SDK from Steinberg for developing VST3 plugins
   - Download from: https://www.steinberg.net/developers/
   - License: Proprietary (requires agreement acceptance)
   - Version: Latest (3.7.4 or newer recommended)

2. **C++ Compiler**
   - Windows: Visual Studio 2019 or newer (with C++17 support)
   - macOS: Xcode 12 or newer (with C++17 support)
   - Linux: GCC 9 or newer or Clang 10 or newer (with C++17 support)

3. **CMake**
   - Version 3.14 or newer
   - Used for cross-platform build configuration

## Optional Dependencies

1. **JUCE Framework**
   - A C++ application framework that simplifies VST development
   - Website: https://juce.com/
   - License: Commercial or GPL
   - Provides higher-level abstractions over the VST SDK

2. **Git**
   - For version control
   - Recommended for managing project changes

3. **IDE**
   - Visual Studio Code with C/C++ extensions
   - Visual Studio (Windows)
   - Xcode (macOS)
   - CLion (cross-platform)

## Platform-Specific Dependencies

### Windows
- Windows 10 or newer
- Windows SDK 10.0.18362.0 or newer
- Visual Studio with "Desktop development with C++" workload

### macOS
- macOS 10.15 (Catalina) or newer
- Xcode Command Line Tools
- CoreAudio framework

### Linux
- ALSA development libraries
- JACK development libraries
- X11 development libraries
- GTK development libraries (for UI)

## Build System Dependencies

1. **pkg-config** (Linux/macOS)
   - For finding installed libraries

2. **ninja** (optional)
   - A faster alternative build system to make

## Testing Dependencies

1. **VST3 Plugin Test Host**
   - Included in the VST3 SDK
   - Used for basic plugin validation

2. **DAW for Testing**
   - Any VST3-compatible DAW such as:
     - Cubase
     - Reaper
     - Ableton Live
     - FL Studio
     - Logic Pro (macOS only)

## Installation Instructions

### Windows

```powershell
# Install Chocolatey (Windows package manager)
Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))

# Install CMake
choco install cmake -y

# Install Git
choco install git -y
```

### macOS

```bash
# Install Homebrew (macOS package manager)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install CMake
brew install cmake

# Install Git
brew install git
```

### Linux (Ubuntu/Debian)

```bash
# Update package lists
sudo apt update

# Install build essentials
sudo apt install build-essential

# Install CMake
sudo apt install cmake

# Install Git
sudo apt install git

# Install audio development libraries
sudo apt install libasound2-dev libjack-jackd2-dev

# Install X11 and GTK development libraries
sudo apt install libx11-dev libgtk-3-dev