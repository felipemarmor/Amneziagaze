# AMNEZIAGAZE v0.2.7

A professional VST3 shoegaze guitar effect plugin with advanced reverb algorithms, shimmer effects, and comprehensive audio processing capabilities.

## Features

### 🎛️ Effect Sections
- **Amp Simulator**: Gain, Bass, Mid, Treble, Presence, Output Level
- **Distortion**: Clean/Crunch/Fuzz modes with Drive control
- **Reverb**: Advanced algorithms with Mix, Size, Reverse, and Shimmer effects
- **Delay**: Mix, Time, Feedback, and Reverse effects
- **Modulation**: Chorus/Flanger/Phaser with Rate and Depth controls

### ✨ Advanced Features
- **Bypass Buttons**: Individual bypass for all effect sections
- **Elysiera-Style Shimmer**: Dual pitch shifters (+12 semitones octave, +5 semitones perfect 4th) with modulation and crossfading
- **Smooth Reverse Reverb**: Continuous streaming approach with no stuttering or tremolo artifacts
- **Professional Reverb Algorithm**: Complex comb filters, allpass chains, and input diffusion
- **Enhanced EQ**: Reduced bass response, enhanced mid/high frequencies for better guitar tone
- **Aggressive Fuzz Distortion**: Hard clipping with controlled octave-up effect
- **Extended Delay Buffers**: 4-second buffers for longer, better-quality reversed samples
- **Anti-Aliasing**: Comprehensive filtering throughout all effects to eliminate buzzing and artifacts

### 🔧 Technical Specifications
- **Format**: VST3
- **Channels**: Mono/Stereo input and output
- **Sample Rate**: 22.05kHz - 384kHz support
- **Bit Depth**: 32-bit floating point processing
- **Latency**: Low-latency real-time processing
- **Validation**: Passes all 47 VST3 SDK validation tests

## Installation

### Windows
1. Download the latest release from the [Releases](../../releases) page
2. Copy `AMNEZIAGAZE.vst3` to your VST3 directory:
   - System: `C:\Program Files\Common Files\VST3\`
   - User: `%USERPROFILE%\AppData\Local\Programs\Common\VST3\`
3. Restart your DAW and scan for new plugins
4. Load "AMNEZIAGAZE" as an insert effect

### Building from Source

#### Prerequisites
- CMake 3.14 or higher
- Visual Studio 2019/2022 (Windows)
- VST3 SDK (automatically downloaded)

#### Build Instructions
```bash
# Clone the repository
git clone https://github.com/yourusername/AMNEZIAGAZE.git
cd AMNEZIAGAZE

# Run the automated build script
setup_and_build_all.bat

# Or build manually
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Usage

### Basic Setup
1. Load AMNEZIAGAZE on a guitar track
2. Start with the **Amp** section for basic tone shaping
3. Add **Distortion** for drive and character
4. Use **Reverb** for space and atmosphere
5. Apply **Delay** for rhythmic effects
6. Add **Modulation** for movement and texture

### Shoegaze Sound Tips
- Set **Reverb Size** to 80-100% for massive spaces
- Enable **Shimmer** at 30-50% for ethereal octave effects
- Use **Reverse Reverb** for swelling textures
- Combine **Fuzz** distortion with high **Reverb Mix**
- Apply **Chorus** or **Flanger** modulation for width

### Effect Chain Order
The plugin processes effects in this order:
1. Amp Simulation
2. Distortion
3. Modulation
4. Delay
5. Reverb

## Development

### Project Structure
```
AMNEZIAGAZE/
├── src/vst/              # VST3 plugin source code
├── include/              # Header files
├── docs/                 # Documentation
├── cmake/                # CMake modules
└── build/                # Build output (generated)
```

### Key Files
- [`src/vst/pluginprocessor.cpp`](src/vst/pluginprocessor.cpp) - Main audio processing
- [`src/vst/plugincontroller.cpp`](src/vst/plugincontroller.cpp) - Parameter management
- [`src/vst/plugineditor.cpp`](src/vst/plugineditor.cpp) - User interface
- [`include/pluginids.h`](include/pluginids.h) - Plugin identification and parameters

### Version History
- **v0.2.7** - Complete restoration with all advanced features
- **v0.2.6** - Enhanced audio quality and artifact elimination
- **v0.2.5** - Elysiera-style shimmer implementation
- **v0.2.4** - Advanced reverb algorithms
- **v0.2.3** - Smooth reverse reverb
- **v0.2.2** - Enhanced EQ and aggressive fuzz
- **v0.2.1** - Bypass functionality and UI improvements
- **v0.2.0** - Major feature additions and improvements

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Credits

- **Developer**: Plugins Legais
- **VST3 SDK**: Steinberg Media Technologies
- **Inspiration**: Classic shoegaze and ambient guitar effects

## Support

For issues, feature requests, or questions:
- Open an [Issue](../../issues)
- Check the [Documentation](docs/)
- Review the [Build Guide](docs/build_and_test.md)

---

*AMNEZIAGAZE - Professional shoegaze guitar effects for the modern producer*