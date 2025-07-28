#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/base/fplatform.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace MyVSTPlugin {

// Unique identifiers for the plugin
static const char* kPluginName = "AMNEZIAGAZE v0.2.7";
static const char* kPluginVendor = "Plugins Legais";
static const char* kPluginVersion = "0.2.7";

// Plugin unique IDs - these should be unique for your plugin
// Define processor UID
static const Steinberg::FUID kPluginProcessorUID(0x12345678, 0x9ABCDEF0, 0x12345678, 0xABCDEF01);

// Define controller UID
static const Steinberg::FUID kPluginControllerUID(0x87654321, 0xCBA90FED, 0x10FEDCBA, 0x87654321);

// Parameter IDs
enum ParameterIDs {
    // Amp Section
    kParamAmpBypassId = 0,// Amp bypass (on/off)
    kParamGainId,         // Preamp gain
    kParamBassId,         // Bass EQ
    kParamMidId,          // Mid EQ
    kParamTrebleId,       // Treble EQ
    kParamPresenceId,     // Presence (high frequency emphasis)
    kParamOutputLevelId,  // Output level
    
    // Distortion Section
    kParamDistBypassId,   // Distortion bypass (on/off)
    kParamDistTypeId,     // Distortion type (clean, crunch, fuzz)
    kParamDistDriveId,    // Distortion amount
    
    // Reverb Section
    kParamReverbBypassId, // Reverb bypass (on/off)
    kParamReverbMixId,    // Reverb mix (dry/wet)
    kParamReverbSizeId,   // Reverb size/decay
    kParamReverbReverseId,// Reverse reverb (on/off)
    kParamReverbShimmerId,// Shimmer effect (amount)
    
    // Delay Section
    kParamDelayBypassId,  // Delay bypass (on/off)
    kParamDelayMixId,     // Delay mix (dry/wet)
    kParamDelayTimeId,    // Delay time
    kParamDelayFeedbackId,// Delay feedback
    kParamDelayReverseId, // Reverse delay (on/off)
    
    // Modulation Section
    kParamModBypassId,    // Modulation bypass (on/off)
    kParamModTypeId,      // Modulation type (chorus, flanger, phaser)
    kParamModRateId,      // Modulation rate
    kParamModDepthId,     // Modulation depth
    
    kNumParams
};

// Distortion Type Values
enum DistortionType {
    kDistClean = 0,
    kDistCrunch,
    kDistFuzz,
    kNumDistTypes
};

// Modulation Type Values
enum ModulationType {
    kModChorus = 0,
    kModFlanger,
    kModPhaser,
    kNumModTypes
};

} // namespace MyVSTPlugin