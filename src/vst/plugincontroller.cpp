#include "plugincontroller.h"
#include "pluginids.h"
#include "plugineditor.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ustring.h"

using namespace Steinberg;
using namespace Steinberg::Vst;
using namespace MyVSTPlugin;

//-----------------------------------------------------------------------------
PluginController::PluginController()
: mAmpBypass(0.0f)
, mDistBypass(0.0f)
, mReverbBypass(0.0f)
, mDelayBypass(0.0f)
, mModBypass(0.0f)
, mGain(0.5f)
, mBass(0.5f)
, mMid(0.5f)
, mTreble(0.5f)
, mPresence(0.5f)
, mOutputLevel(0.7f)
, mDistType(kDistCrunch)
, mDistDrive(0.5f)
, mReverbMix(0.3f)
, mReverbSize(0.5f)
, mReverbReverse(0.0f)
, mReverbShimmer(0.0f)
, mDelayMix(0.3f)
, mDelayTime(0.5f)
, mDelayFeedback(0.3f)
, mDelayReverse(0.0f)
, mModType(kModChorus)
, mModRate(0.5f)
, mModDepth(0.5f)
{
    // Initialize parameters
}

//-----------------------------------------------------------------------------
PluginController::~PluginController()
{
    // Nothing to clean up
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginController::initialize(FUnknown* context)
{
    // First initialize the parent
    tresult result = EditControllerEx1::initialize(context);
    if (result != kResultOk)
    {
        return result;
    }

    // Setup parameters
    setupParameters();

    return kResultOk;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginController::terminate()
{
    // Clean up resources
    return EditControllerEx1::terminate();
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginController::setComponentState(IBStream* state)
{
    // Called when the controller should sync with processor state
    if (!state)
        return kResultFalse;

    IBStreamer streamer(state, kLittleEndian);
    
    // Read the parameter values
    float savedAmpBypass = 0.0f;
    float savedDistBypass = 0.0f;
    float savedReverbBypass = 0.0f;
    float savedDelayBypass = 0.0f;
    float savedModBypass = 0.0f;
    
    float savedGain = 0.5f;
    float savedBass = 0.5f;
    float savedMid = 0.5f;
    float savedTreble = 0.5f;
    float savedPresence = 0.5f;
    float savedOutputLevel = 0.7f;
    
    int32 savedDistType = kDistCrunch;
    float savedDistDrive = 0.5f;
    
    float savedReverbMix = 0.3f;
    float savedReverbSize = 0.5f;
    float savedReverbReverse = 0.0f;
    float savedReverbShimmer = 0.0f;
    
    float savedDelayMix = 0.3f;
    float savedDelayTime = 0.5f;
    float savedDelayFeedback = 0.3f;
    float savedDelayReverse = 0.0f;
    
    int32 savedModType = kModChorus;
    float savedModRate = 0.5f;
    float savedModDepth = 0.5f;
    
    // Read all parameters (with error checking)
    if (streamer.readFloat(savedAmpBypass) == false ||
        streamer.readFloat(savedDistBypass) == false ||
        streamer.readFloat(savedReverbBypass) == false ||
        streamer.readFloat(savedDelayBypass) == false ||
        streamer.readFloat(savedModBypass) == false ||
        
        streamer.readFloat(savedGain) == false ||
        streamer.readFloat(savedBass) == false ||
        streamer.readFloat(savedMid) == false ||
        streamer.readFloat(savedTreble) == false ||
        streamer.readFloat(savedPresence) == false ||
        streamer.readFloat(savedOutputLevel) == false ||
        
        streamer.readInt32(savedDistType) == false ||
        streamer.readFloat(savedDistDrive) == false ||
        
        streamer.readFloat(savedReverbMix) == false ||
        streamer.readFloat(savedReverbSize) == false ||
        streamer.readFloat(savedReverbReverse) == false ||
        streamer.readFloat(savedReverbShimmer) == false ||
        
        streamer.readFloat(savedDelayMix) == false ||
        streamer.readFloat(savedDelayTime) == false ||
        streamer.readFloat(savedDelayFeedback) == false ||
        streamer.readFloat(savedDelayReverse) == false ||
        
        streamer.readInt32(savedModType) == false ||
        streamer.readFloat(savedModRate) == false ||
        streamer.readFloat(savedModDepth) == false)
    {
        return kResultFalse;
    }
    
    // Update the parameters
    setParamNormalized(kParamAmpBypassId, savedAmpBypass);
    setParamNormalized(kParamDistBypassId, savedDistBypass);
    setParamNormalized(kParamReverbBypassId, savedReverbBypass);
    setParamNormalized(kParamDelayBypassId, savedDelayBypass);
    setParamNormalized(kParamModBypassId, savedModBypass);
    
    setParamNormalized(kParamGainId, savedGain);
    setParamNormalized(kParamBassId, savedBass);
    setParamNormalized(kParamMidId, savedMid);
    setParamNormalized(kParamTrebleId, savedTreble);
    setParamNormalized(kParamPresenceId, savedPresence);
    setParamNormalized(kParamOutputLevelId, savedOutputLevel);
    
    setParamNormalized(kParamDistTypeId, (float)savedDistType / (float)(kNumDistTypes - 1));
    setParamNormalized(kParamDistDriveId, savedDistDrive);
    
    setParamNormalized(kParamReverbMixId, savedReverbMix);
    setParamNormalized(kParamReverbSizeId, savedReverbSize);
    setParamNormalized(kParamReverbReverseId, savedReverbReverse);
    setParamNormalized(kParamReverbShimmerId, savedReverbShimmer);
    
    setParamNormalized(kParamDelayMixId, savedDelayMix);
    setParamNormalized(kParamDelayTimeId, savedDelayTime);
    setParamNormalized(kParamDelayFeedbackId, savedDelayFeedback);
    setParamNormalized(kParamDelayReverseId, savedDelayReverse);
    
    setParamNormalized(kParamModTypeId, (float)savedModType / (float)(kNumModTypes - 1));
    setParamNormalized(kParamModRateId, savedModRate);
    setParamNormalized(kParamModDepthId, savedModDepth);
    
    // Store values locally
    mAmpBypass = savedAmpBypass;
    mDistBypass = savedDistBypass;
    mReverbBypass = savedReverbBypass;
    mDelayBypass = savedDelayBypass;
    mModBypass = savedModBypass;
    
    mGain = savedGain;
    mBass = savedBass;
    mMid = savedMid;
    mTreble = savedTreble;
    mPresence = savedPresence;
    mOutputLevel = savedOutputLevel;
    
    mDistType = savedDistType;
    mDistDrive = savedDistDrive;
    
    mReverbMix = savedReverbMix;
    mReverbSize = savedReverbSize;
    mReverbReverse = savedReverbReverse;
    mReverbShimmer = savedReverbShimmer;
    
    mDelayMix = savedDelayMix;
    mDelayTime = savedDelayTime;
    mDelayFeedback = savedDelayFeedback;
    mDelayReverse = savedDelayReverse;
    
    mModType = savedModType;
    mModRate = savedModRate;
    mModDepth = savedModDepth;
    
    return kResultOk;
}

//-----------------------------------------------------------------------------
IPlugView* PLUGIN_API PluginController::createView(FIDString name)
{
    // Create the view
    if (FIDStringsEqual(name, Vst::ViewType::kEditor))
    {
        // Create our custom editor
        return new PluginEditor(this);
    }
    
    return nullptr;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginController::setParamNormalized(ParamID tag, ParamValue value)
{
    // Called when a parameter is changed
    tresult result = EditControllerEx1::setParamNormalized(tag, value);
    
    // Store the parameter value locally
    switch (tag)
    {
        // Bypass Parameters
        case kParamAmpBypassId:
            mAmpBypass = value;
            break;
        case kParamDistBypassId:
            mDistBypass = value;
            break;
        case kParamReverbBypassId:
            mReverbBypass = value;
            break;
        case kParamDelayBypassId:
            mDelayBypass = value;
            break;
        case kParamModBypassId:
            mModBypass = value;
            break;
            
        // Amp Section
        case kParamGainId:
            mGain = value;
            break;
        case kParamBassId:
            mBass = value;
            break;
        case kParamMidId:
            mMid = value;
            break;
        case kParamTrebleId:
            mTreble = value;
            break;
        case kParamPresenceId:
            mPresence = value;
            break;
        case kParamOutputLevelId:
            mOutputLevel = value;
            break;
            
        // Distortion Section
        case kParamDistTypeId:
            mDistType = (int)(value * (kNumDistTypes - 1) + 0.5f);
            break;
        case kParamDistDriveId:
            mDistDrive = value;
            break;
            
        // Reverb Section
        case kParamReverbMixId:
            mReverbMix = value;
            break;
        case kParamReverbSizeId:
            mReverbSize = value;
            break;
        case kParamReverbReverseId:
            mReverbReverse = value;
            break;
        case kParamReverbShimmerId:
            mReverbShimmer = value;
            break;
            
        // Delay Section
        case kParamDelayMixId:
            mDelayMix = value;
            break;
        case kParamDelayTimeId:
            mDelayTime = value;
            break;
        case kParamDelayFeedbackId:
            mDelayFeedback = value;
            break;
        case kParamDelayReverseId:
            mDelayReverse = value;
            break;
            
        // Modulation Section
        case kParamModTypeId:
            mModType = (int)(value * (kNumModTypes - 1) + 0.5f);
            break;
        case kParamModRateId:
            mModRate = value;
            break;
        case kParamModDepthId:
            mModDepth = value;
            break;
    }
    
    return result;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginController::getState(IBStream* state)
{
    // Called when saving controller state
    return kResultOk;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginController::setState(IBStream* state)
{
    // Called when loading controller state
    return kResultOk;
}

//-----------------------------------------------------------------------------
void PluginController::setupParameters()
{
    // Create units for parameter organization
    UnitInfo unitInfo;
    unitInfo.id = 1;
    unitInfo.parentUnitId = kRootUnitId;
    unitInfo.programListId = kNoProgramListId;
    UString(unitInfo.name, USTRINGSIZE(unitInfo.name)).assign(STR16("Amp"));
    addUnit(new Unit(unitInfo));
    
    unitInfo.id = 2;
    UString(unitInfo.name, USTRINGSIZE(unitInfo.name)).assign(STR16("Distortion"));
    addUnit(new Unit(unitInfo));
    
    unitInfo.id = 3;
    UString(unitInfo.name, USTRINGSIZE(unitInfo.name)).assign(STR16("Reverb"));
    addUnit(new Unit(unitInfo));
    
    unitInfo.id = 4;
    UString(unitInfo.name, USTRINGSIZE(unitInfo.name)).assign(STR16("Delay"));
    addUnit(new Unit(unitInfo));
    
    unitInfo.id = 5;
    UString(unitInfo.name, USTRINGSIZE(unitInfo.name)).assign(STR16("Modulation"));
    addUnit(new Unit(unitInfo));
    
    // Bypass Parameters
    parameters.addParameter(
        STR16("Amp Bypass"),      // Parameter title
        STR16(""),                // Parameter unit
        1,                        // Step count (1 = toggle)
        0.0,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamAmpBypassId,        // Parameter ID
        1,                        // Unit ID (Amp)
        STR16("Bypass")           // Parameter group
    );
    
    parameters.addParameter(
        STR16("Dist Bypass"),     // Parameter title
        STR16(""),                // Parameter unit
        1,                        // Step count (1 = toggle)
        0.0,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamDistBypassId,       // Parameter ID
        2,                        // Unit ID (Distortion)
        STR16("Bypass")           // Parameter group
    );
    
    parameters.addParameter(
        STR16("Reverb Bypass"),   // Parameter title
        STR16(""),                // Parameter unit
        1,                        // Step count (1 = toggle)
        0.0,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamReverbBypassId,     // Parameter ID
        3,                        // Unit ID (Reverb)
        STR16("Bypass")           // Parameter group
    );
    
    parameters.addParameter(
        STR16("Delay Bypass"),    // Parameter title
        STR16(""),                // Parameter unit
        1,                        // Step count (1 = toggle)
        0.0,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamDelayBypassId,      // Parameter ID
        4,                        // Unit ID (Delay)
        STR16("Bypass")           // Parameter group
    );
    
    parameters.addParameter(
        STR16("Mod Bypass"),      // Parameter title
        STR16(""),                // Parameter unit
        1,                        // Step count (1 = toggle)
        0.0,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamModBypassId,        // Parameter ID
        5,                        // Unit ID (Modulation)
        STR16("Bypass")           // Parameter group
    );
    
    // Amp Section Parameters
    parameters.addParameter(
        STR16("Gain"),            // Parameter title
        STR16("%"),               // Parameter unit
        0,                        // Step count (0 = continuous)
        0.5,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamGainId,             // Parameter ID
        1,                        // Unit ID (Amp)
        STR16("Preamp")           // Parameter group
    );
    
    parameters.addParameter(
        STR16("Bass"),            // Parameter title
        STR16("dB"),              // Parameter unit
        0,                        // Step count (0 = continuous)
        0.5,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamBassId,             // Parameter ID
        1,                        // Unit ID (Amp)
        STR16("EQ")               // Parameter group
    );
    
    parameters.addParameter(
        STR16("Mid"),             // Parameter title
        STR16("dB"),              // Parameter unit
        0,                        // Step count (0 = continuous)
        0.5,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamMidId,              // Parameter ID
        1,                        // Unit ID (Amp)
        STR16("EQ")               // Parameter group
    );
    
    parameters.addParameter(
        STR16("Treble"),          // Parameter title
        STR16("dB"),              // Parameter unit
        0,                        // Step count (0 = continuous)
        0.5,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamTrebleId,           // Parameter ID
        1,                        // Unit ID (Amp)
        STR16("EQ")               // Parameter group
    );
    
    parameters.addParameter(
        STR16("Presence"),        // Parameter title
        STR16("dB"),              // Parameter unit
        0,                        // Step count (0 = continuous)
        0.5,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamPresenceId,         // Parameter ID
        1,                        // Unit ID (Amp)
        STR16("EQ")               // Parameter group
    );
    
    parameters.addParameter(
        STR16("Output"),          // Parameter title
        STR16("dB"),              // Parameter unit
        0,                        // Step count (0 = continuous)
        0.7,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamOutputLevelId,      // Parameter ID
        1,                        // Unit ID (Amp)
        STR16("Master")           // Parameter group
    );
    
    // Distortion Section Parameters
    StringListParameter* distTypeParam = new StringListParameter(
        STR16("Type"),            // Parameter title
        kParamDistTypeId,         // Parameter ID
        nullptr,                  // Parameter title for displaying
        ParameterInfo::kCanAutomate | ParameterInfo::kIsList // Flags
    );
    distTypeParam->appendString(STR16("Clean"));
    distTypeParam->appendString(STR16("Crunch"));
    distTypeParam->appendString(STR16("Fuzz"));
    parameters.addParameter(distTypeParam);
    
    parameters.addParameter(
        STR16("Drive"),           // Parameter title
        STR16("%"),               // Parameter unit
        0,                        // Step count (0 = continuous)
        0.5,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamDistDriveId,        // Parameter ID
        2,                        // Unit ID (Distortion)
        STR16("Distortion")       // Parameter group
    );
    
    // Reverb Section Parameters
    parameters.addParameter(
        STR16("Mix"),             // Parameter title
        STR16("%"),               // Parameter unit
        0,                        // Step count (0 = continuous)
        0.3,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamReverbMixId,        // Parameter ID
        3,                        // Unit ID (Reverb)
        STR16("Reverb")           // Parameter group
    );
    
    parameters.addParameter(
        STR16("Size"),            // Parameter title
        STR16("%"),               // Parameter unit
        0,                        // Step count (0 = continuous)
        0.5,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamReverbSizeId,       // Parameter ID
        3,                        // Unit ID (Reverb)
        STR16("Reverb")           // Parameter group
    );
    
    parameters.addParameter(
        STR16("Reverse"),         // Parameter title
        STR16(""),                // Parameter unit
        1,                        // Step count (1 = toggle)
        0.0,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamReverbReverseId,    // Parameter ID
        3,                        // Unit ID (Reverb)
        STR16("Reverb")           // Parameter group
    );
    
    parameters.addParameter(
        STR16("Shimmer"),         // Parameter title
        STR16("%"),               // Parameter unit
        0,                        // Step count (0 = continuous)
        0.0,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamReverbShimmerId,    // Parameter ID
        3,                        // Unit ID (Reverb)
        STR16("Reverb")           // Parameter group
    );
    
    // Delay Section Parameters
    parameters.addParameter(
        STR16("Mix"),             // Parameter title
        STR16("%"),               // Parameter unit
        0,                        // Step count (0 = continuous)
        0.3,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamDelayMixId,         // Parameter ID
        4,                        // Unit ID (Delay)
        STR16("Delay")            // Parameter group
    );
    
    parameters.addParameter(
        STR16("Time"),            // Parameter title
        STR16("s"),               // Parameter unit
        0,                        // Step count (0 = continuous)
        0.25,                     // Default normalized value (now maps to 1s instead of 2s with 4s max)
        ParameterInfo::kCanAutomate, // Flags
        kParamDelayTimeId,        // Parameter ID
        4,                        // Unit ID (Delay)
        STR16("Delay")            // Parameter group
    );
    
    parameters.addParameter(
        STR16("Feedback"),        // Parameter title
        STR16("%"),               // Parameter unit
        0,                        // Step count (0 = continuous)
        0.3,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamDelayFeedbackId,    // Parameter ID
        4,                        // Unit ID (Delay)
        STR16("Delay")            // Parameter group
    );
    
    parameters.addParameter(
        STR16("Reverse"),         // Parameter title
        STR16(""),                // Parameter unit
        1,                        // Step count (1 = toggle)
        0.0,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamDelayReverseId,     // Parameter ID
        4,                        // Unit ID (Delay)
        STR16("Delay")            // Parameter group
    );
    
    // Modulation Section Parameters
    StringListParameter* modTypeParam = new StringListParameter(
        STR16("Type"),            // Parameter title
        kParamModTypeId,          // Parameter ID
        nullptr,                  // Parameter title for displaying
        ParameterInfo::kCanAutomate | ParameterInfo::kIsList // Flags
    );
    modTypeParam->appendString(STR16("Chorus"));
    modTypeParam->appendString(STR16("Flanger"));
    modTypeParam->appendString(STR16("Phaser"));
    parameters.addParameter(modTypeParam);
    
    parameters.addParameter(
        STR16("Rate"),            // Parameter title
        STR16("Hz"),              // Parameter unit
        0,                        // Step count (0 = continuous)
        0.5,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamModRateId,          // Parameter ID
        5,                        // Unit ID (Modulation)
        STR16("Modulation")       // Parameter group
    );
    
    parameters.addParameter(
        STR16("Depth"),           // Parameter title
        STR16("%"),               // Parameter unit
        0,                        // Step count (0 = continuous)
        0.5,                      // Default normalized value
        ParameterInfo::kCanAutomate, // Flags
        kParamModDepthId,         // Parameter ID
        5,                        // Unit ID (Modulation)
        STR16("Modulation")       // Parameter group
    );
}