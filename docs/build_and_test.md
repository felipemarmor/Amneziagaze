# Building and Testing the VST Plugin

This guide provides instructions for building and testing the VST plugin after you have set up the project and installed the Steinberg VST SDK.

## Prerequisites

Before proceeding, ensure you have:
- Completed the SDK installation as described in `docs/sdk_installation.md`
- Installed all required dependencies as listed in `docs/dependencies.md`
- A compatible C++ compiler
- CMake (version 3.14 or newer)
- A VST3-compatible DAW for testing

## Building the Plugin

### Windows

1. **Open Command Prompt or PowerShell**
   - Navigate to the project directory

2. **Create a Build Directory**
   ```cmd
   mkdir build
   cd build
   ```

3. **Configure with CMake**
   ```cmd
   cmake -G "Visual Studio 16 2019" -A x64 ..
   ```
   Note: Replace "Visual Studio 16 2019" with your installed Visual Studio version if different

4. **Build the Plugin**
   ```cmd
   cmake --build . --config Release
   ```

5. **Locate the Built Plugin**
   - The VST3 plugin will be in `build\VST3\Release\MyVSTPlugin.vst3`

### macOS

1. **Open Terminal**
   - Navigate to the project directory

2. **Create a Build Directory**
   ```bash
   mkdir build
   cd build
   ```

3. **Configure with CMake**
   ```bash
   cmake -G Xcode ..
   ```

4. **Build the Plugin**
   ```bash
   cmake --build . --config Release
   ```

5. **Locate the Built Plugin**
   - The VST3 plugin will be in `build/VST3/Release/MyVSTPlugin.vst3`

### Linux

1. **Open Terminal**
   - Navigate to the project directory

2. **Create a Build Directory**
   ```bash
   mkdir build
   cd build
   ```

3. **Configure with CMake**
   ```bash
   cmake ..
   ```

4. **Build the Plugin**
   ```bash
   cmake --build . --config Release
   ```
   or simply:
   ```bash
   make
   ```

5. **Locate the Built Plugin**
   - The VST3 plugin will be in `build/VST3/MyVSTPlugin.vst3`

## Installing the Plugin

To use the plugin in a DAW, you need to install it in the standard VST3 location:

### Windows
```cmd
copy "build\VST3\Release\MyVSTPlugin.vst3" "C:\Program Files\Common Files\VST3\"
```

### macOS
```bash
cp -r "build/VST3/Release/MyVSTPlugin.vst3" "/Library/Audio/Plug-Ins/VST3/"
```

### Linux
```bash
cp -r "build/VST3/MyVSTPlugin.vst3" "/usr/local/lib/vst3/"
```

## Testing the Plugin

### Using the VST3 Plugin Test Host

The VST3 SDK includes a test host application that can be used to validate your plugin:

1. Build the VST3 Plugin Test Host from the SDK
2. Launch the test host
3. Load your plugin
4. Test basic functionality

### Testing in a DAW

1. **Launch a VST3-compatible DAW** such as:
   - Cubase
   - Reaper
   - Ableton Live
   - FL Studio
   - Logic Pro (macOS only)

2. **Scan for New Plugins**
   - Most DAWs have an option to scan for new plugins
   - This may be in the preferences or plugin manager

3. **Create a New Project**
   - Create a new project in your DAW

4. **Add the Plugin**
   - Add an instrument or effect track
   - Insert your plugin on the track

5. **Test Basic Functionality**
   - Verify the plugin loads without errors
   - Test the gain parameter
   - Check audio processing

## Troubleshooting Build Issues

### Common Build Errors

1. **VST3 SDK Not Found**
   - Ensure the SDK is correctly installed in `lib/vst3sdk`
   - Check that the path in CMakeLists.txt is correct

2. **Compiler Errors**
   - Verify you have a compatible C++ compiler
   - Ensure the compiler supports C++17

3. **Missing Dependencies**
   - Check that all required dependencies are installed
   - See `docs/dependencies.md` for details

4. **CMake Configuration Errors**
   - Ensure you have CMake 3.14 or newer
   - Check for typos in CMake commands

### Common Runtime Errors

1. **Plugin Not Detected by DAW**
   - Verify the plugin was built successfully
   - Ensure it was copied to the correct VST3 directory
   - Restart the DAW and rescan for plugins

2. **Plugin Crashes on Load**
   - Check DAW logs for error messages
   - Try running the plugin in the VST3 Plugin Test Host for more detailed error information

3. **No Audio Processing**
   - Verify audio connections in the DAW
   - Check that the plugin is receiving audio input

## Next Steps

After successfully building and testing the plugin, you can:

1. Customize the plugin by modifying the source code
2. Add more parameters and controls
3. Implement a custom UI
4. Extend the audio processing capabilities

## Getting Help

If you encounter issues not covered in this guide:
- Check the [Steinberg Developer Forum](https://forums.steinberg.net/c/developer/vst/78)
- Review the documentation included with the SDK
- Search for solutions on developer communities like Stack Overflow