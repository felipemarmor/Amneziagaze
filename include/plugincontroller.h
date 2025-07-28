#pragma once

#include "pluginids.h"
#include "public.sdk/source/vst/vsteditcontroller.h"

namespace MyVSTPlugin {

//-----------------------------------------------------------------------------
// PluginController: UI controller class for the VST plugin
//-----------------------------------------------------------------------------
class PluginController : public Steinberg::Vst::EditControllerEx1
{
public:
    // Constructor and destructor
    PluginController();
    ~PluginController() SMTG_OVERRIDE;

    // Create function (factory method)
    static Steinberg::FUnknown* createInstance(void* context)
    {
        return (Steinberg::Vst::IEditController*)new PluginController;
    }

    // EditController overrides
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream* state) SMTG_OVERRIDE;
    
    // UI related methods
    Steinberg::IPlugView* PLUGIN_API createView(Steinberg::FIDString name) SMTG_OVERRIDE;
    
    // Parameter update handling
    Steinberg::tresult PLUGIN_API setParamNormalized(Steinberg::Vst::ParamID tag,
                                                   Steinberg::Vst::ParamValue value) SMTG_OVERRIDE;
    
    // State handling
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) SMTG_OVERRIDE;

private:
    // Helper methods
    void setupParameters();
    
    // Bypass Parameters
    float mAmpBypass;
    float mDistBypass;
    float mReverbBypass;
    float mDelayBypass;
    float mModBypass;
    
    // Amp Parameters
    float mGain;
    float mBass;
    float mMid;
    float mTreble;
    float mPresence;
    float mOutputLevel;
    
    // Distortion Parameters
    int mDistType;
    float mDistDrive;
    
    // Reverb Parameters
    float mReverbMix;
    float mReverbSize;
    float mReverbReverse;
    float mReverbShimmer;
    
    // Delay Parameters
    float mDelayMix;
    float mDelayTime;
    float mDelayFeedback;
    float mDelayReverse;
    
    // Modulation Parameters
    int mModType;
    float mModRate;
    float mModDepth;
};

} // namespace MyVSTPlugin