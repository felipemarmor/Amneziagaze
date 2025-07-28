#pragma once

#include "pluginids.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include <vector>
#include <cmath>

namespace MyVSTPlugin {

//-----------------------------------------------------------------------------
// PluginProcessor: Main audio processing class for the VST plugin
//-----------------------------------------------------------------------------
class PluginProcessor : public Steinberg::Vst::AudioEffect
{
public:
    // Constructor and destructor
    PluginProcessor();
    ~PluginProcessor() SMTG_OVERRIDE;

    // Create function (factory method)
    static Steinberg::FUnknown* createInstance(void* context)
    {
        return (Steinberg::Vst::IAudioProcessor*)new PluginProcessor;
    }

    // AudioEffect overrides
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup& setup) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API canProcessSampleSize(Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;
    Steinberg::uint32 PLUGIN_API getLatencySamples() SMTG_OVERRIDE;

private:
    // Bypass Parameters
    float mAmpBypass;     // Amp bypass (0.0 to 1.0, where >0.5 is bypassed)
    float mDistBypass;    // Distortion bypass (0.0 to 1.0, where >0.5 is bypassed)
    float mReverbBypass;  // Reverb bypass (0.0 to 1.0, where >0.5 is bypassed)
    float mDelayBypass;   // Delay bypass (0.0 to 1.0, where >0.5 is bypassed)
    float mModBypass;     // Modulation bypass (0.0 to 1.0, where >0.5 is bypassed)
    
    // Amp Parameters
    float mGain;          // Preamp gain (0.0 to 1.0)
    float mBass;          // Bass EQ (0.0 to 1.0)
    float mMid;           // Mid EQ (0.0 to 1.0)
    float mTreble;        // Treble EQ (0.0 to 1.0)
    float mPresence;      // Presence (0.0 to 1.0)
    float mOutputLevel;   // Output level (0.0 to 1.0)
    
    // Distortion Parameters
    int mDistType;        // Distortion type (0=clean, 1=crunch, 2=fuzz)
    float mDistDrive;     // Distortion amount (0.0 to 1.0)
    
    // Reverb Parameters
    float mReverbMix;     // Reverb mix (0.0 to 1.0)
    float mReverbSize;    // Reverb size/decay (0.0 to 1.0)
    float mReverbReverse; // Reverse reverb (0.0 to 1.0, where >0.5 is on)
    float mReverbShimmer; // Shimmer effect amount (0.0 to 1.0)
    
    // Delay Parameters
    float mDelayMix;      // Delay mix (0.0 to 1.0)
    float mDelayTime;     // Delay time in seconds (0.0 to 2.0)
    float mDelayFeedback; // Delay feedback (0.0 to 1.0)
    float mDelayReverse;  // Reverse delay (0.0 to 1.0, where >0.5 is on)
    
    // Modulation Parameters
    int mModType;         // Modulation type (0=chorus, 1=flanger, 2=phaser)
    float mModRate;       // Modulation rate (0.0 to 1.0)
    float mModDepth;      // Modulation depth (0.0 to 1.0)

    // Processing state
    Steinberg::Vst::SampleRate mSampleRate;
    Steinberg::int32 mBypassed;
    
    // Delay line for echo effect
    std::vector<float> mDelayBuffer;
    std::vector<float> mDelayInputBuffer;  // For reverse delay
    std::vector<float> mReverseDelayProcessedBuffer; // For processed reverse delay output
    int mDelayBufferPos;
    int mDelayInputBufferPos;
    int mDelayBufferLength;
    
    // Reverse delay state variables
    int mReverseDelayBufferCounter;
    int mReverseDelayProcessedPos;
    bool mIsReverseDelayActive;
    
    // Reverb buffers and state
    std::vector<float> mReverbBuffer;
    std::vector<float> mReverbInputBuffer;  // For reverse reverb
    std::vector<float> mReverseReverbProcessedBuffer;  // For processed reverse reverb output
    int mReverbBufferPos;
    int mReverbInputBufferPos;
    float mReverbFeedback;
    
    // Reverse reverb state variables
    int mReverseBufferCounter;
    int mProcessedBufferPos;
    bool mIsReverseReverbActive;
    
    // Modulation state
    float mModPhase;
    
    // Filter state variables for EQ
    float mBassFilter[2][2];    // [channels][state]
    float mMidFilter[2][2];     // [channels][state]
    float mTrebleFilter[2][2];  // [channels][state]
    float mPresenceFilter[2][2];// [channels][state]
    
    // Helper methods for audio processing
    float processAmp(float input, int channel);
    float processDistortion(float input);
    float processEQ(float input, int channel);
    float processReverb(float input);
    float processComplexReverbSample(float input); // Advanced reverb algorithm
    float processDelay(float input);
    float processModulation(float input);
    
    // Reset all processing state
    void resetProcessingBuffers();
};

} // namespace MyVSTPlugin