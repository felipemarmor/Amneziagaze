# Steinberg VST SDK Installation Guide

This guide provides step-by-step instructions for downloading and installing the Steinberg VST3 SDK for this project.

## Prerequisites

Before proceeding, ensure you have:
- Internet connection
- A web browser
- Basic knowledge of file extraction

## Download Steps

1. **Visit the Steinberg Developer Portal**
   - Go to [https://www.steinberg.net/developers/](https://www.steinberg.net/developers/)
   - Navigate to the VST 3 SDK section

2. **Create a Steinberg Account (if needed)**
   - If you don't already have a Steinberg account, you'll need to create one
   - Click on the "Register" or "Sign Up" button and follow the instructions

3. **Accept the License Agreement**
   - Read the Steinberg VST 3 Plug-In SDK Licensing Agreement
   - Accept the terms by checking the appropriate box

4. **Download the SDK**
   - Click the download button for the latest VST 3 SDK
   - Save the ZIP file to a known location on your computer

## Installation Steps

1. **Extract the SDK**
   - Locate the downloaded ZIP file
   - Extract its contents using your preferred extraction tool
   - You should see a folder named "VST3_SDK" after extraction

2. **Prepare the Project Directory**
   - Navigate to the project's `lib` directory
   - Create a new folder named `vst3sdk` inside the `lib` directory

3. **Copy SDK Files**
   - Copy all contents from the extracted "VST3_SDK" folder
   - Paste them into the `lib/vst3sdk` directory of this project

4. **Verify Installation**
   - Check that the following key files/directories exist:
     - `lib/vst3sdk/pluginterfaces/`
     - `lib/vst3sdk/public.sdk/`
     - `lib/vst3sdk/base/`
     - `lib/vst3sdk/CMakeLists.txt`

## Updating CMakeLists.txt (if needed)

If you installed the SDK in a different location than the default, you'll need to update the `CMakeLists.txt` file:

1. Open `CMakeLists.txt` in the root of the project
2. Locate the line: `set(VST3_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib/vst3sdk" CACHE PATH "Path to VST3 SDK")`
3. Update the path to match your SDK installation location
4. Save the file

## Troubleshooting

### Common Issues

1. **License Agreement Not Showing**
   - Try using a different browser
   - Ensure JavaScript is enabled

2. **Download Fails**
   - Check your internet connection
   - Try downloading again later
   - Use a different browser

3. **Missing Files After Extraction**
   - Ensure the ZIP file was completely downloaded
   - Try re-downloading the SDK
   - Use a different extraction tool

4. **Build Errors After Installation**
   - Verify all SDK files were copied correctly
   - Check that the SDK path in CMakeLists.txt is correct
   - Ensure you're using a compatible compiler version

### Getting Help

If you encounter issues not covered in this guide:
- Check the [Steinberg Developer Forum](https://forums.steinberg.net/c/developer/vst/78)
- Review the documentation included with the SDK
- Search for solutions on developer communities like Stack Overflow

## Next Steps

After successfully installing the SDK, you can proceed to:
1. Build the plugin using CMake
2. Test the plugin in a compatible DAW
3. Start customizing the plugin for your needs

## Additional Resources

- [VST 3 SDK Documentation](https://steinbergmedia.github.io/vst3_doc/)
- [VST 3 Plug-in Implementation Guide](https://steinbergmedia.github.io/vst3_dev_portal/pages/index.html)
- [Steinberg Developer Portal](https://www.steinberg.net/developers/)