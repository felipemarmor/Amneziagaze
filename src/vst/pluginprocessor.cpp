#include "pluginprocessor.h"
#include "plugincontroller.h"
#include "pluginids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include <cmath>
#include <algorithm>

using namespace Steinberg;
using namespace Steinberg::Vst;
using namespace MyVSTPlugin;

// Constants for effects
const float PI = 3.14159265358979323846f;
const float TWO_PI = 2.0f * PI;

//-----------------------------------------------------------------------------
PluginProcessor::PluginProcessor()
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
, mSampleRate(44100.0)
, mBypassed(0)
, mDelayBufferPos(0)
, mReverbBufferPos(0)
, mModPhase(0.0f)
, mReverseBufferCounter(0)
, mProcessedBufferPos(0)
, mIsReverseReverbActive(false)
, mReverseDelayBufferCounter(0)
, mReverseDelayProcessedPos(0)
, mIsReverseDelayActive(false)
{
    // Register VST3 interfaces
    setControllerClass(kPluginControllerUID);
    
    // Initialize filter states
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            mBassFilter[i][j] = 0.0f;
            mMidFilter[i][j] = 0.0f;
            mTrebleFilter[i][j] = 0.0f;
            mPresenceFilter[i][j] = 0.0f;
        }
    }
}

//-----------------------------------------------------------------------------
PluginProcessor::~PluginProcessor()
{
    // Nothing to clean up
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginProcessor::initialize(FUnknown* context)
{
    // First initialize the parent
    tresult result = AudioEffect::initialize(context);
    if (result != kResultOk)
    {
        return result;
    }

    // Set up audio bus arrangements
    // For this simple plugin, we'll use stereo in and stereo out
    addAudioInput(STR16("Audio Input"), SpeakerArr::kStereo);
    addAudioOutput(STR16("Audio Output"), SpeakerArr::kStereo);

    return kResultOk;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginProcessor::terminate()
{
    // Clean up resources
    return AudioEffect::terminate();
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginProcessor::setActive(TBool state)
{
    // Called when the plugin is enabled/disabled
    if (state)
    {
        // Initialize processing
        resetProcessingBuffers();
    }
    else
    {
        // Cleanup processing
    }
    return AudioEffect::setActive(state);
}

//-----------------------------------------------------------------------------
void PluginProcessor::resetProcessingBuffers()
{
    // Reset all processing state
    
    // Initialize delay buffer with longer duration for better reversed samples
    int delayBufferSize = (int)(mSampleRate * 4.0); // Max 4 seconds delay (increased from 2)
    mDelayBuffer.resize(delayBufferSize, 0.0f);
    mDelayBufferPos = 0;
    mDelayBufferLength = delayBufferSize;
    
    // Initialize reverse delay buffer with longer duration
    int reverseDelaySize = (int)(mSampleRate * 4.0); // 4 seconds for reverse buffer (increased from 2)
    mDelayInputBuffer.resize(reverseDelaySize, 0.0f);
    std::fill(mDelayInputBuffer.begin(), mDelayInputBuffer.end(), 0.0f);
    mDelayInputBufferPos = 0;
    
    // Initialize reverse delay processed buffer
    int reverseDelayProcessedSize = (int)(mSampleRate * 4.0); // 4 seconds (increased from 2)
    mReverseDelayProcessedBuffer.resize(reverseDelayProcessedSize, 0.0f);
    std::fill(mReverseDelayProcessedBuffer.begin(), mReverseDelayProcessedBuffer.end(), 0.0f);
    
    // Reset reverse delay state
    mReverseDelayBufferCounter = 0;
    mReverseDelayProcessedPos = 0;
    mIsReverseDelayActive = false;
    
    // Initialize reverb buffer
    int reverbBufferSize = (int)(mSampleRate * 3.0); // 3 seconds for reverb tail
    mReverbBuffer.resize(reverbBufferSize, 0.0f);
    mReverbBufferPos = 0;
    mReverbFeedback = 0.7f;
    
    // Initialize reverse reverb buffer
    int reverseBufferSize = (int)(mSampleRate * 2.0); // 2 seconds for reverse buffer
    mReverbInputBuffer.resize(reverseBufferSize, 0.0f);
    std::fill(mReverbInputBuffer.begin(), mReverbInputBuffer.end(), 0.0f);
    mReverbInputBufferPos = 0;
    
    // Initialize reverse reverb processed buffer
    int processedBufferSize = (int)(mSampleRate * 2.0); // 2 seconds for processed buffer
    mReverseReverbProcessedBuffer.resize(processedBufferSize, 0.0f);
    std::fill(mReverseReverbProcessedBuffer.begin(), mReverseReverbProcessedBuffer.end(), 0.0f);
    
    // Reset reverse reverb state
    mReverseBufferCounter = 0;
    mProcessedBufferPos = 0;
    mIsReverseReverbActive = false;
    
    // Reset modulation phase
    mModPhase = 0.0f;
    
    // Reset filter states
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            mBassFilter[i][j] = 0.0f;
            mMidFilter[i][j] = 0.0f;
            mTrebleFilter[i][j] = 0.0f;
            mPresenceFilter[i][j] = 0.0f;
        }
    }
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginProcessor::process(ProcessData& data)
{
    // Check if processing context is available
    if (data.processContext)
    {
        // Process parameter changes
        if (data.inputParameterChanges)
        {
            int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
            for (int32 i = 0; i < numParamsChanged; i++)
            {
                IParamValueQueue* paramQueue = data.inputParameterChanges->getParameterData(i);
                if (paramQueue)
                {
                    ParamValue value;
                    int32 sampleOffset;
                    int32 numPoints = paramQueue->getPointCount();
                    
                    if (numPoints > 0)
                    {
                        // Get the last point in the queue
                        paramQueue->getPoint(numPoints - 1, sampleOffset, value);
                        
                        // Handle parameter changes
                        switch (paramQueue->getParameterId())
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
                    }
                }
            }
        }
    }

    // Process audio
    if (data.numInputs == 0 || data.numOutputs == 0)
    {
        return kResultOk;
    }

    // Get audio buffers
    if (data.inputs[0].silenceFlags != 0)
    {
        // Input is silent, so output is silent too
        data.outputs[0].silenceFlags = data.inputs[0].silenceFlags;
        return kResultOk;
    }

    // For each channel
    for (int32 channel = 0; channel < data.inputs[0].numChannels; channel++)
    {
        // Get input and output buffers for this channel
        float* ptrIn = data.inputs[0].channelBuffers32[channel];
        float* ptrOut = data.outputs[0].channelBuffers32[channel];
        
        // Process samples
        for (int32 sample = 0; sample < data.numSamples; sample++)
        {
            float processed = ptrIn[sample];
            
            // Apply amp simulation and EQ (with bypass)
            if (mAmpBypass <= 0.5f) {
                processed = processAmp(processed, channel);
            }
            
            // Apply distortion (with bypass)
            if (mDistBypass <= 0.5f) {
                processed = processDistortion(processed);
            }
            
            // Apply modulation (chorus/flanger/phaser) (with bypass)
            if (mModBypass <= 0.5f) {
                processed = processModulation(processed);
            }
            
            // Apply delay (with bypass)
            if (mDelayBypass <= 0.5f) {
                processed = processDelay(processed);
            }
            
            // Apply reverb (with bypass)
            if (mReverbBypass <= 0.5f) {
                processed = processReverb(processed);
            }
            
            // Apply output level (always applied, even when amp is bypassed)
            processed = processed * mOutputLevel;
            
            ptrOut[sample] = processed;
        }
    }

    return kResultOk;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginProcessor::setState(IBStream* state)
{
    // Called when loading a preset or project
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
    
    // Store values
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
tresult PLUGIN_API PluginProcessor::getState(IBStream* state)
{
    // Called when saving a preset or project
    if (!state)
        return kResultFalse;

    IBStreamer streamer(state, kLittleEndian);
    
    // Write the parameter values
    streamer.writeFloat(mAmpBypass);
    streamer.writeFloat(mDistBypass);
    streamer.writeFloat(mReverbBypass);
    streamer.writeFloat(mDelayBypass);
    streamer.writeFloat(mModBypass);
    
    streamer.writeFloat(mGain);
    streamer.writeFloat(mBass);
    streamer.writeFloat(mMid);
    streamer.writeFloat(mTreble);
    streamer.writeFloat(mPresence);
    streamer.writeFloat(mOutputLevel);
    
    streamer.writeInt32(mDistType);
    streamer.writeFloat(mDistDrive);
    
    streamer.writeFloat(mReverbMix);
    streamer.writeFloat(mReverbSize);
    streamer.writeFloat(mReverbReverse);
    streamer.writeFloat(mReverbShimmer);
    
    streamer.writeFloat(mDelayMix);
    streamer.writeFloat(mDelayTime);
    streamer.writeFloat(mDelayFeedback);
    streamer.writeFloat(mDelayReverse);
    
    streamer.writeInt32(mModType);
    streamer.writeFloat(mModRate);
    streamer.writeFloat(mModDepth);
    
    return kResultOk;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginProcessor::setupProcessing(ProcessSetup& setup)
{
    // Called before processing starts
    mSampleRate = setup.sampleRate;
    
    // Reset processing buffers for the new sample rate
    resetProcessingBuffers();
    
    return AudioEffect::setupProcessing(setup);
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginProcessor::canProcessSampleSize(int32 symbolicSampleSize)
{
    // We support 32-bit float processing
    if (symbolicSampleSize == kSample32)
        return kResultOk;
    
    return kResultFalse;
}

//-----------------------------------------------------------------------------
uint32 PLUGIN_API PluginProcessor::getLatencySamples()
{
    // This plugin introduces some latency due to the delay effect
    return 0; // For simplicity, we're not reporting latency
}

//-----------------------------------------------------------------------------
// Amp simulation and EQ processing
//-----------------------------------------------------------------------------
float PluginProcessor::processAmp(float input, int channel)
{
    // EXTREME noise gate to eliminate all low-level interference
    float noiseGateThreshold = 0.005f; // Much higher threshold
    if (fabs(input) < noiseGateThreshold) {
        input = 0.0f;
    }
    
    // Apply gain with MUCH reduced range to prevent ANY harsh artifacts
    float gain = 1.0f + mGain * 6.0f; // Drastically reduced from 12x to 6x gain range
    float amplified = input * gain;
    
    // ELIMINATE all possible sources of digital artifacts
    // Clamp to prevent any overflow that could cause buzzing
    if (amplified > 0.8f) amplified = 0.8f;
    if (amplified < -0.8f) amplified = -0.8f;
    
    // Apply EXTREME 7-stage anti-aliasing cascade
    static float lpFilter[2] = {0.0f, 0.0f};
    static float secondaryLPF[2] = {0.0f, 0.0f};
    static float tertiaryLPF[2] = {0.0f, 0.0f};
    static float quaternaryLPF[2] = {0.0f, 0.0f};
    static float quinaryLPF[2] = {0.0f, 0.0f};
    static float senaryLPF[2] = {0.0f, 0.0f};
    static float septenaryLPF[2] = {0.0f, 0.0f};
    
    // Stage 1: Ultra-aggressive initial filtering
    float cutoff1 = 0.8f; // Extremely aggressive
    lpFilter[channel] = lpFilter[channel] * (1.0f - cutoff1) + amplified * cutoff1;
    
    // Stage 2: Continue aggressive filtering
    float cutoff2 = 0.7f;
    secondaryLPF[channel] = secondaryLPF[channel] * (1.0f - cutoff2) + lpFilter[channel] * cutoff2;
    
    // Stage 3: More filtering
    float cutoff3 = 0.6f;
    tertiaryLPF[channel] = tertiaryLPF[channel] * (1.0f - cutoff3) + secondaryLPF[channel] * cutoff3;
    
    // Stage 4: Even more filtering
    float cutoff4 = 0.5f;
    quaternaryLPF[channel] = quaternaryLPF[channel] * (1.0f - cutoff4) + tertiaryLPF[channel] * cutoff4;
    
    // Stage 5: Continue the cascade
    float cutoff5 = 0.4f;
    quinaryLPF[channel] = quinaryLPF[channel] * (1.0f - cutoff5) + quaternaryLPF[channel] * cutoff5;
    
    // Stage 6: More smoothing
    float cutoff6 = 0.3f;
    senaryLPF[channel] = senaryLPF[channel] * (1.0f - cutoff6) + quinaryLPF[channel] * cutoff6;
    
    // Stage 7: Final ultra-smooth stage
    float cutoff7 = 0.2f;
    septenaryLPF[channel] = septenaryLPF[channel] * (1.0f - cutoff7) + senaryLPF[channel] * cutoff7;
    
    // Apply EQ to the ultra-filtered signal
    return processEQ(septenaryLPF[channel], channel);
}

//-----------------------------------------------------------------------------
// EQ processing
//-----------------------------------------------------------------------------
float PluginProcessor::processEQ(float input, int channel)
{
    // REDESIGNED EQ with reduced bass and enhanced high/mid frequencies
    
    // Apply noise gate to EQ input
    if (fabs(input) < 0.001f) {
        input = 0.0f;
    }
    
    // Bass (low shelf around 80Hz) - REDUCED bass response
    float bassFreq = 80.0f / mSampleRate; // Moved down from 100Hz to 80Hz
    float bassAlpha = bassFreq / (bassFreq + 0.5f);
    float bassGain = 0.7f + (mBass * 2.0f - 1.0f) * 2.0f; // Reduced base gain and range
    
    // Clamp bass gain to reduce bass buildup
    if (bassGain > 1.5f) bassGain = 1.5f;
    if (bassGain < 0.3f) bassGain = 0.3f; // Allow more bass cut
    
    mBassFilter[channel][0] = mBassFilter[channel][0] + bassAlpha * (input - mBassFilter[channel][0]);
    float bassOut = mBassFilter[channel][0] * bassGain;
    
    // Apply anti-aliasing to bass output
    static float bassAAFilter[2] = {0.0f, 0.0f};
    bassAAFilter[channel] = bassAAFilter[channel] * 0.7f + bassOut * 0.3f;
    bassOut = bassAAFilter[channel];
    
    // Mid (peak around 1.5kHz) - ENHANCED mid response
    float midFreq = 1500.0f / mSampleRate; // Moved up from 1kHz to 1.5kHz for more presence
    float midAlpha = midFreq / (midFreq + 0.5f);
    float midGain = 1.2f + (mMid * 2.0f - 1.0f) * 4.0f; // Increased base gain and range
    
    // Allow more mid boost
    if (midGain > 3.0f) midGain = 3.0f;
    if (midGain < 0.5f) midGain = 0.5f;
    
    mMidFilter[channel][0] = mMidFilter[channel][0] + midAlpha * (input - mMidFilter[channel][0]);
    float midOut = (input - mMidFilter[channel][0]) * midGain;
    
    // Apply anti-aliasing to mid output
    static float midAAFilter[2] = {0.0f, 0.0f};
    midAAFilter[channel] = midAAFilter[channel] * 0.7f + midOut * 0.3f;
    midOut = midAAFilter[channel];
    
    // Treble (high shelf around 3kHz) - ENHANCED treble response
    float trebleFreq = 3000.0f / mSampleRate; // Back to 3kHz for more brightness
    float trebleAlpha = trebleFreq / (trebleFreq + 0.5f);
    float trebleGain = 1.3f + (mTreble * 2.0f - 1.0f) * 4.0f; // Increased base gain and range
    
    // Allow more treble boost
    if (trebleGain > 3.5f) trebleGain = 3.5f;
    if (trebleGain < 0.6f) trebleGain = 0.6f;
    
    mTrebleFilter[channel][0] = mTrebleFilter[channel][0] + trebleAlpha * (input - mTrebleFilter[channel][0]);
    float trebleOut = (input - mTrebleFilter[channel][0]) * trebleGain;
    
    // Apply controlled anti-aliasing to treble output
    static float trebleAAFilter1[2] = {0.0f, 0.0f};
    static float trebleAAFilter2[2] = {0.0f, 0.0f};
    trebleAAFilter1[channel] = trebleAAFilter1[channel] * 0.8f + trebleOut * 0.2f;
    trebleAAFilter2[channel] = trebleAAFilter2[channel] * 0.7f + trebleAAFilter1[channel] * 0.3f;
    trebleOut = trebleAAFilter2[channel];
    
    // Presence (high shelf around 5kHz) - ENHANCED presence
    float presenceFreq = 5000.0f / mSampleRate; // Moved up from 4kHz to 5kHz
    float presenceAlpha = presenceFreq / (presenceFreq + 0.5f);
    float presenceGain = 1.4f + (mPresence * 2.0f - 1.0f) * 3.0f; // Increased base gain and range
    
    // Allow more presence boost
    if (presenceGain > 3.0f) presenceGain = 3.0f;
    if (presenceGain < 0.7f) presenceGain = 0.7f;
    
    mPresenceFilter[channel][0] = mPresenceFilter[channel][0] + presenceAlpha * (input - mPresenceFilter[channel][0]);
    float presenceOut = (input - mPresenceFilter[channel][0]) * presenceGain;
    
    // Apply controlled anti-aliasing to presence output
    static float presenceAAFilter1[2] = {0.0f, 0.0f};
    static float presenceAAFilter2[2] = {0.0f, 0.0f};
    presenceAAFilter1[channel] = presenceAAFilter1[channel] * 0.8f + presenceOut * 0.2f;
    presenceAAFilter2[channel] = presenceAAFilter2[channel] * 0.7f + presenceAAFilter1[channel] * 0.3f;
    presenceOut = presenceAAFilter2[channel];
    
    // Sum all bands with enhanced high/mid frequencies
    float eqOutput = (bassOut * 0.6f + midOut * 1.2f + trebleOut * 1.3f + presenceOut * 1.1f) * 0.7f;
    
    // Final anti-aliasing stage for the entire EQ output
    static float finalEQFilter[2] = {0.0f, 0.0f};
    finalEQFilter[channel] = finalEQFilter[channel] * 0.8f + eqOutput * 0.2f;
    
    // Clamp final output to prevent any overflow
    float finalOutput = finalEQFilter[channel];
    if (finalOutput > 0.8f) finalOutput = 0.8f;
    if (finalOutput < -0.8f) finalOutput = -0.8f;
    
    return finalOutput;
}

//-----------------------------------------------------------------------------
// Distortion processing
//-----------------------------------------------------------------------------
float PluginProcessor::processDistortion(float input)
{
    // EXTREME noise gate for distortion input
    if (fabs(input) < 0.003f) {
        input = 0.0f;
    }
    
    // DRASTICALLY reduced drive range to eliminate ALL buzzing sources
    float drive = mDistDrive * 3.0f + 1.0f;  // Reduced from 12x to 3x maximum drive
    
    // Pre-gain with HARD limiting to prevent ANY overflow
    float preGain = input * drive;
    if (preGain > 0.5f) preGain = 0.5f;
    if (preGain < -0.5f) preGain = -0.5f;
    
    float distorted = 0.0f;
    
    // COMPLETELY REDESIGNED distortion algorithms to eliminate buzzing
    switch (mDistType)
    {
        case kDistClean:
            // Ultra-gentle soft clipping
            distorted = tanh(preGain * 0.1f); // Extremely reduced gain
            break;
            
        case kDistCrunch:
            // Very gentle saturation
            distorted = tanh(preGain * 0.2f); // Much more gentle
            // Remove ALL asymmetry that could cause artifacts
            break;
            
        case kDistFuzz:
            // AGGRESSIVE FUZZ with controlled aliasing
            // Hard clipping for aggressive fuzz character
            if (preGain > 0.4f) {
                distorted = 0.4f + tanh((preGain - 0.4f) * 8.0f) * 0.3f; // Aggressive clipping above threshold
            } else if (preGain < -0.4f) {
                distorted = -0.4f + tanh((preGain + 0.4f) * 8.0f) * 0.3f; // Aggressive clipping below threshold
            } else {
                // Aggressive cubic distortion in the middle range
                distorted = preGain + (preGain * preGain * preGain) * 2.0f;
            }
            
            // Add controlled octave-up effect for fuzz character
            static float fuzzOctavePhase = 0.0f;
            fuzzOctavePhase += TWO_PI * 2.0f / mSampleRate; // Double frequency
            if (fuzzOctavePhase >= TWO_PI) fuzzOctavePhase -= TWO_PI;
            
            float octaveEffect = sin(fuzzOctavePhase) * 0.15f * fabs(distorted); // Controlled octave-up
            distorted += octaveEffect;
            
            // Apply controlled smoothing (less than before for more aggression)
            static float fuzzSmoother1 = 0.0f;
            static float fuzzSmoother2 = 0.0f;
            
            fuzzSmoother1 = fuzzSmoother1 * 0.7f + distorted * 0.3f; // Less smoothing
            fuzzSmoother2 = fuzzSmoother2 * 0.6f + fuzzSmoother1 * 0.4f; // Less smoothing
            
            distorted = fuzzSmoother2;
            break;
    }
    
    // ELIMINATE post-gain that could cause artifacts
    float postGain = 0.8f; // Fixed, gentle post-gain
    
    // EXTREME 8-stage anti-aliasing cascade to eliminate ALL possible buzzing
    static float antiAliasingLPF1 = 0.0f;
    static float antiAliasingLPF2 = 0.0f;
    static float antiAliasingLPF3 = 0.0f;
    static float antiAliasingLPF4 = 0.0f;
    static float antiAliasingLPF5 = 0.0f;
    static float antiAliasingLPF6 = 0.0f;
    static float antiAliasingLPF7 = 0.0f;
    static float antiAliasingLPF8 = 0.0f;
    
    // Eight-stage EXTREME filtering cascade
    float lpfCutoff1 = 0.95f; // Ultra-aggressive
    antiAliasingLPF1 = antiAliasingLPF1 * (1.0f - lpfCutoff1) + distorted * lpfCutoff1;
    
    float lpfCutoff2 = 0.9f;
    antiAliasingLPF2 = antiAliasingLPF2 * (1.0f - lpfCutoff2) + antiAliasingLPF1 * lpfCutoff2;
    
    float lpfCutoff3 = 0.8f;
    antiAliasingLPF3 = antiAliasingLPF3 * (1.0f - lpfCutoff3) + antiAliasingLPF2 * lpfCutoff3;
    
    float lpfCutoff4 = 0.7f;
    antiAliasingLPF4 = antiAliasingLPF4 * (1.0f - lpfCutoff4) + antiAliasingLPF3 * lpfCutoff4;
    
    float lpfCutoff5 = 0.6f;
    antiAliasingLPF5 = antiAliasingLPF5 * (1.0f - lpfCutoff5) + antiAliasingLPF4 * lpfCutoff5;
    
    float lpfCutoff6 = 0.5f;
    antiAliasingLPF6 = antiAliasingLPF6 * (1.0f - lpfCutoff6) + antiAliasingLPF5 * lpfCutoff6;
    
    float lpfCutoff7 = 0.4f;
    antiAliasingLPF7 = antiAliasingLPF7 * (1.0f - lpfCutoff7) + antiAliasingLPF6 * lpfCutoff7;
    
    float lpfCutoff8 = 0.3f; // Final ultra-smooth stage
    antiAliasingLPF8 = antiAliasingLPF8 * (1.0f - lpfCutoff8) + antiAliasingLPF7 * lpfCutoff8;
    
    // Final output with hard limiting to prevent ANY overflow
    float finalOutput = antiAliasingLPF8 * postGain;
    if (finalOutput > 0.6f) finalOutput = 0.6f;
    if (finalOutput < -0.6f) finalOutput = -0.6f;
    
    return finalOutput;
}

//-----------------------------------------------------------------------------
// Reverb processing
//-----------------------------------------------------------------------------
float PluginProcessor::processReverb(float input)
{
    if (mReverbMix <= 0.01f) {
        return input; // Skip processing if reverb is off
    }
    
    float dryInput = input; // Store the original input
    float reverbInput = input; // Input to be processed by reverb
    float output = 0.0f;
    
    // Handle reverse reverb if enabled - COMPLETELY REDESIGNED FOR NO STUTTERING
    if (mReverbReverse > 0.5f) {
        // Continuous streaming approach - no chunks, no stuttering
        static std::vector<float> continuousReverseBuffer(176400, 0.0f); // 4 seconds at 44.1kHz
        static int continuousWritePos = 0;
        static float continuousReadPos = 0.0f;
        static bool reverseInitialized = false;
        static float reverseSmoothing = 0.0f;
        
        // Store input continuously
        continuousReverseBuffer[continuousWritePos] = input;
        continuousWritePos = (continuousWritePos + 1) % continuousReverseBuffer.size();
        
        // Initialize reverse reading position
        if (!reverseInitialized) {
            continuousReadPos = continuousWritePos - (mSampleRate * 2.0f); // Start 2 seconds back
            if (continuousReadPos < 0) continuousReadPos += continuousReverseBuffer.size();
            reverseInitialized = true;
        }
        
        // Continuous reverse reading with smooth interpolation
        continuousReadPos -= 1.0f; // Read backwards
        if (continuousReadPos < 0) continuousReadPos += continuousReverseBuffer.size();
        
        // High-quality interpolation for smooth reading
        int readIndex = (int)continuousReadPos;
        float fraction = continuousReadPos - readIndex;
        int nextIndex = (readIndex + 1) % continuousReverseBuffer.size();
        
        float reverseSample = continuousReverseBuffer[readIndex] * (1.0f - fraction) +
                             continuousReverseBuffer[nextIndex] * fraction;
        
        // Apply reverb to the reverse sample
        float reverseReverb = processComplexReverbSample(reverseSample);
        
        // Ultra-smooth envelope to eliminate any attack artifacts
        float targetLevel = 1.0f;
        float smoothingRate = 0.001f; // Very slow attack
        reverseSmoothing += (targetLevel - reverseSmoothing) * smoothingRate;
        reverseReverb *= reverseSmoothing;
        
        // Extreme anti-aliasing for reverse reverb
        static float reverseAAFilter1 = 0.0f;
        static float reverseAAFilter2 = 0.0f;
        static float reverseAAFilter3 = 0.0f;
        static float reverseAAFilter4 = 0.0f;
        static float reverseAAFilter5 = 0.0f;
        
        float aaCutoff1 = 0.9f; // Extremely aggressive
        reverseAAFilter1 = reverseAAFilter1 * (1.0f - aaCutoff1) + reverseReverb * aaCutoff1;
        
        float aaCutoff2 = 0.8f;
        reverseAAFilter2 = reverseAAFilter2 * (1.0f - aaCutoff2) + reverseAAFilter1 * aaCutoff2;
        
        float aaCutoff3 = 0.6f;
        reverseAAFilter3 = reverseAAFilter3 * (1.0f - aaCutoff3) + reverseAAFilter2 * aaCutoff3;
        
        float aaCutoff4 = 0.4f;
        reverseAAFilter4 = reverseAAFilter4 * (1.0f - aaCutoff4) + reverseAAFilter3 * aaCutoff4;
        
        float aaCutoff5 = 0.2f; // Final smoothing
        reverseAAFilter5 = reverseAAFilter5 * (1.0f - aaCutoff5) + reverseAAFilter4 * aaCutoff5;
        
        float ultraCleanReverseReverb = reverseAAFilter5;
        
        // Smooth additive mixing - no signal cutting
        float wetMix = mReverbMix * 0.6f; // Reduced level
        output = dryInput + ultraCleanReverseReverb * wetMix;
        return output;
    }
    
    // Process with the advanced complex reverb algorithm
    float complexReverb = processComplexReverbSample(reverbInput);
    
    // Mix dry and wet signals with proper balance
    float wetLevel = mReverbMix;
    float dryLevel = 1.0f - mReverbMix;
    
    // Apply some gain compensation to maintain perceived loudness
    float totalGain = sqrt(dryLevel * dryLevel + wetLevel * wetLevel);
    if (totalGain > 0.0f) {
        dryLevel /= totalGain;
        wetLevel /= totalGain;
    }
    
    return dryInput * dryLevel + complexReverb * wetLevel;
}

//-----------------------------------------------------------------------------
// Delay processing
//-----------------------------------------------------------------------------
float PluginProcessor::processDelay(float input)
{
    if (mDelayMix <= 0.01f) {
        return input; // Skip processing if delay is off
    }
    
    float dryInput = input; // Store the original input
    float delayInput = input; // Input to be processed by delay
    float output = 0.0f;
    
    // Handle reverse delay if enabled
    if (mDelayReverse > 0.5f) {
        // Always store input in the circular buffer
        mDelayInputBuffer[mDelayInputBufferPos] = input;
        mDelayInputBufferPos = (mDelayInputBufferPos + 1) % mDelayInputBuffer.size();
        
        // Calculate delay time in samples
        float delayTimeInSeconds = 0.1f + mDelayTime * 3.9f; // 0.1 to 4.0 seconds (increased range)
        int chunkSize = (int)(delayTimeInSeconds * mSampleRate);
        
        // Count samples for the current block
        mReverseDelayBufferCounter++;
        
        // When we've collected enough samples, process them with the reverse delay technique
        if (mReverseDelayBufferCounter >= chunkSize && !mIsReverseDelayActive) {
            // Extract a block from the circular buffer
            std::vector<float> blockToProcess(chunkSize);
            for (int i = 0; i < chunkSize; i++) {
                int readPos = (mDelayInputBufferPos - chunkSize + i + mDelayInputBuffer.size()) % mDelayInputBuffer.size();
                blockToProcess[i] = mDelayInputBuffer[readPos];
            }
            
            // Reverse the block
            for (int i = 0; i < chunkSize; i++) {
                mReverseDelayProcessedBuffer[i] = blockToProcess[chunkSize - 1 - i];
            }
            
            // Reset position and activate reverse delay playback
            mReverseDelayProcessedPos = 0;
            mIsReverseDelayActive = true;
        }
        
        // If we're in active reverse delay mode, output the processed data
        if (mIsReverseDelayActive) {
            if (mReverseDelayProcessedPos < mReverseDelayProcessedBuffer.size()) {
                // Mix the processed delay with the dry signal
                // Use a higher mix ratio for the reverse delay to make it more noticeable
                float wetMix = mDelayMix * 1.5f;
                if (wetMix > 1.0f) wetMix = 1.0f;
                
                // Apply feedback to the reversed signal
                float delayedSample = mReverseDelayProcessedBuffer[mReverseDelayProcessedPos];
                
                // Write the delayed sample with feedback to the main delay buffer for echo effects
                mDelayBuffer[mDelayBufferPos] = delayedSample * mDelayFeedback;
                mDelayBufferPos = (mDelayBufferPos + 1) % mDelayBufferLength;
                
                output = dryInput * (1.0f - wetMix) + delayedSample * wetMix;
                mReverseDelayProcessedPos++;
            } else {
                // We've played through the entire processed buffer, reset and start collecting again
                mIsReverseDelayActive = false;
                mReverseDelayBufferCounter = 0;
                output = dryInput; // Pass through dry signal until next block is processed
            }
            
            return output; // Return the reverse delay output directly
        }
    }
    
    // Calculate delay time in samples
    float delayTimeInSeconds = 0.1f + mDelayTime * 3.9f; // 0.1 to 4.0 seconds (increased range)
    int delaySamples = (int)(delayTimeInSeconds * mSampleRate);
    
    // Make sure delay time doesn't exceed buffer size
    delaySamples = std::min(delaySamples, mDelayBufferLength - 1);
    
    // Get current position
    int readPos = mDelayBufferPos - delaySamples;
    if (readPos < 0) {
        readPos += mDelayBufferLength;
    }
    
    // Read from delay buffer
    float delayedSample = mDelayBuffer[readPos];
    
    // Write to delay buffer with feedback
    mDelayBuffer[mDelayBufferPos] = delayInput + delayedSample * mDelayFeedback;
    
    // Update position
    mDelayBufferPos = (mDelayBufferPos + 1) % mDelayBufferLength;
    
    // Mix dry and wet signals
    return dryInput * (1.0f - mDelayMix) + delayedSample * mDelayMix;
}

//-----------------------------------------------------------------------------
// Modulation processing (chorus/flanger/phaser)
//-----------------------------------------------------------------------------
float PluginProcessor::processModulation(float input)
{
    if (mModDepth <= 0.01f) {
        return input; // Skip processing if depth is too low
    }
    
    // Apply noise gate to modulation input
    if (fabs(input) < 0.002f) {
        input = 0.0f;
    }
    
    // Calculate modulation rate with smoother transitions
    float rate = 0.1f + mModRate * 4.9f; // Reduced from 10Hz to 5Hz max to avoid artifacts
    float phase = mModPhase;
    
    // Update phase with smoother increment
    mModPhase += rate / mSampleRate;
    if (mModPhase >= 1.0f) {
        mModPhase -= 1.0f;
    }
    
    // Calculate LFO value with smoother sine wave
    float lfo = 0.5f + 0.5f * sin(TWO_PI * phase);
    
    // Apply EXTREME smoothing to LFO to prevent artifacts
    static float lfoSmoother = 0.5f;
    lfoSmoother = lfoSmoother * 0.95f + lfo * 0.05f; // Very smooth LFO
    lfo = lfoSmoother;
    
    // Apply different modulation types with MUCH reduced intensity
    float modulated = 0.0f;
    
    switch (mModType)
    {
        case kModChorus:
            // Chorus: MUCH gentler delay modulation
            {
                // Calculate delay time (3-8ms) - reduced range
                float delayMs = 3.0f + lfo * 5.0f;
                int delaySamples = (int)(delayMs * mSampleRate / 1000.0f);
                
                // Ensure delay samples is within bounds
                if (delaySamples >= mDelayBufferLength) {
                    delaySamples = mDelayBufferLength - 1;
                }
                
                // Read from delay buffer with bounds checking
                int readPos = mDelayBufferPos - delaySamples;
                if (readPos < 0) {
                    readPos += mDelayBufferLength;
                }
                
                float delayed = mDelayBuffer[readPos];
                
                // Apply anti-aliasing to delayed signal
                static float chorusAAFilter = 0.0f;
                chorusAAFilter = chorusAAFilter * 0.8f + delayed * 0.2f;
                delayed = chorusAAFilter;
                
                // Mix with original - MUCH gentler mixing
                float mixAmount = mModDepth * 0.3f; // Reduced from 0.5f to 0.3f
                modulated = input * (1.0f - mixAmount) + delayed * mixAmount;
            }
            break;
            
        case kModFlanger:
            // Flanger: MUCH gentler delay modulation
            {
                // Calculate delay time (0.5-3ms) - reduced range
                float delayMs = 0.5f + lfo * 2.5f;
                int delaySamples = (int)(delayMs * mSampleRate / 1000.0f);
                
                // Ensure delay samples is within bounds
                if (delaySamples >= mDelayBufferLength) {
                    delaySamples = mDelayBufferLength - 1;
                }
                if (delaySamples < 1) {
                    delaySamples = 1;
                }
                
                // Read from delay buffer with bounds checking
                int readPos = mDelayBufferPos - delaySamples;
                if (readPos < 0) {
                    readPos += mDelayBufferLength;
                }
                
                float delayed = mDelayBuffer[readPos];
                
                // Apply anti-aliasing to delayed signal
                static float flangerAAFilter = 0.0f;
                flangerAAFilter = flangerAAFilter * 0.8f + delayed * 0.2f;
                delayed = flangerAAFilter;
                
                // Mix with original - MUCH gentler flanging
                float mixAmount = mModDepth * 0.2f; // Much reduced intensity
                modulated = input - delayed * mixAmount;
            }
            break;
            
        case kModPhaser:
            // Phaser: COMPLETELY REDESIGNED to eliminate buzzing
            {
                // Use much lower frequency range to avoid buzz frequencies
                float allpassFreq = 100.0f + lfo * 1000.0f; // Reduced from 200-4200Hz to 100-1100Hz
                float allpassAlpha = allpassFreq / (allpassFreq + mSampleRate);
                
                // Clamp alpha to prevent instability
                if (allpassAlpha > 0.9f) allpassAlpha = 0.9f;
                if (allpassAlpha < 0.1f) allpassAlpha = 0.1f;
                
                // Apply ONLY 2 stages instead of 4 to reduce artifacts
                float phased = input;
                static float phaserState1 = 0.0f;
                static float phaserState2 = 0.0f;
                
                // Stage 1 with extreme smoothing
                float delayed1 = phaserState1;
                phased = allpassAlpha * (phased + delayed1) - delayed1;
                phaserState1 = phaserState1 * 0.9f + phased * 0.1f; // Extreme smoothing
                
                // Stage 2 with extreme smoothing
                float delayed2 = phaserState2;
                phased = allpassAlpha * (phased + delayed2) - delayed2;
                phaserState2 = phaserState2 * 0.9f + phased * 0.1f; // Extreme smoothing
                
                // Apply final anti-aliasing to phaser output
                static float phaserAAFilter = 0.0f;
                phaserAAFilter = phaserAAFilter * 0.8f + phased * 0.2f;
                phased = phaserAAFilter;
                
                // Mix with original - MUCH gentler phasing
                float mixAmount = mModDepth * 0.25f; // Reduced intensity
                modulated = input * (1.0f - mixAmount) + phased * mixAmount;
            }
            break;
    }
    
    // Apply final anti-aliasing to entire modulation output
    static float modAAFilter = 0.0f;
    modAAFilter = modAAFilter * 0.85f + modulated * 0.15f;
    
    // Hard limit to prevent any overflow
    float finalOutput = modAAFilter;
    if (finalOutput > 0.8f) finalOutput = 0.8f;
    if (finalOutput < -0.8f) finalOutput = -0.8f;
    
    return finalOutput;
}
//-----------------------------------------------------------------------------
// Improved Complex Reverb Algorithm - Fixed Issues
//-----------------------------------------------------------------------------
float PluginProcessor::processComplexReverbSample(float input)
{
    // ===== STAGE 1: INPUT DIFFUSION =====
    // Pre-delay for early reflection separation
    static std::vector<float> preDelayBuffer(8820, 0.0f); // 200ms at 44.1kHz
    static int preDelayPos = 0;
    
    int preDelayTime = (int)(mSampleRate * 0.01f * mReverbSize); // 0-20ms pre-delay (reduced)
    if (preDelayTime >= preDelayBuffer.size()) preDelayTime = preDelayBuffer.size() - 1;
    
    int preDelayReadPos = (preDelayPos + preDelayBuffer.size() - preDelayTime) % preDelayBuffer.size();
    float preDelayed = preDelayBuffer[preDelayReadPos];
    preDelayBuffer[preDelayPos] = input;
    preDelayPos = (preDelayPos + 1) % preDelayBuffer.size();
    
    // Input diffusion using cascaded allpass filters with smaller delays
    static std::vector<float> allpass1(347, 0.0f);  // Smaller, prime-number delays
    static std::vector<float> allpass2(113, 0.0f);
    static std::vector<float> allpass3(37, 0.0f);
    static std::vector<float> allpass4(59, 0.0f);
    static int ap1pos = 0, ap2pos = 0, ap3pos = 0, ap4pos = 0;
    
    // Allpass filter function with reduced feedback for less delay-like sound
    auto processAllpass = [&](float in, std::vector<float>& buffer, int& pos, float feedback) -> float {
        float delayed = buffer[pos];
        float output = -in + delayed;
        buffer[pos] = in + delayed * feedback;
        pos = (pos + 1) % buffer.size();
        return output;
    };
    
    float diffused = preDelayed;
    diffused = processAllpass(diffused, allpass1, ap1pos, 0.5f);  // Reduced feedback
    diffused = processAllpass(diffused, allpass2, ap2pos, 0.5f);
    diffused = processAllpass(diffused, allpass3, ap3pos, 0.5f);
    diffused = processAllpass(diffused, allpass4, ap4pos, 0.5f);
    
    // ===== STAGE 2: COMB FILTERS FOR DENSE REVERB =====
    // Multiple comb filters with different delay times
    static std::vector<std::vector<float>> combFilters(6);
    static std::vector<int> combPositions(6, 0);
    static std::vector<float> combOutputs(6, 0.0f);
    static bool combInitialized = false;
    
    if (!combInitialized) {
        // Initialize comb filters with carefully chosen delay times
        int combLengths[] = {1116, 1188, 1277, 1356, 1422, 1491}; // Classic Schroeder lengths
        for (int i = 0; i < 6; i++) {
            float sizeMultiplier = 1.0f + mReverbSize * 15.0f; // Massive scaling: 1x to 16x for huge sustain
            int length = (int)(combLengths[i] * sizeMultiplier);
            combFilters[i].resize(length, 0.0f);
        }
        combInitialized = true;
    }
    
    // Process through comb filters with much higher sustain
    float combSum = 0.0f;
    float feedback = 0.6f + mReverbSize * 0.35f; // 0.6 to 0.95 feedback for massive sustain
    
    for (int i = 0; i < 6; i++) {
        if (!combFilters[i].empty()) {
            // Read delayed sample
            float delayed = combFilters[i][combPositions[i]];
            combOutputs[i] = delayed;
            
            // Apply damping filter to reduce high frequencies over time
            static float dampingFilters[6] = {0};
            float dampingAmount = 0.2f + (1.0f - mReverbSize) * 0.3f;
            dampingFilters[i] = dampingFilters[i] * (1.0f - dampingAmount) + delayed * dampingAmount;
            float damped = delayed * (1.0f - dampingAmount) + dampingFilters[i] * dampingAmount;
            
            // Write input + feedback
            combFilters[i][combPositions[i]] = diffused + damped * feedback;
            combPositions[i] = (combPositions[i] + 1) % combFilters[i].size();
            
            // Sum outputs with different weights
            float weight = 1.0f / (6.0f + i * 0.1f); // Slightly different weights
            combSum += combOutputs[i] * weight;
        }
    }
    
    // ===== STAGE 3: FINAL ALLPASS CHAIN FOR DIFFUSION =====
    // Final allpass filters to break up remaining echoes
    static std::vector<float> finalAP1(225, 0.0f);
    static std::vector<float> finalAP2(341, 0.0f);
    static int finalAP1pos = 0, finalAP2pos = 0;
    
    float finalDiffused = combSum;
    finalDiffused = processAllpass(finalDiffused, finalAP1, finalAP1pos, 0.5f);
    finalDiffused = processAllpass(finalDiffused, finalAP2, finalAP2pos, 0.5f);
    
    // ===== STAGE 4: ELYSIERA-STYLE ADVANCED SHIMMER =====
    float shimmerOutput = finalDiffused;
    if (mReverbShimmer > 0.01f) {
        // Elysiera-style dual pitch shifter shimmer with modulation
        static std::vector<float> shimmerBufferA(22050, 0.0f); // 500ms buffer for pitch A
        static std::vector<float> shimmerBufferB(22050, 0.0f); // 500ms buffer for pitch B
        static int shimmerWritePos = 0;
        static float shimmerReadPosA = 0.0f;
        static float shimmerReadPosB = 0.0f;
        static float modPhaseA = 0.0f;
        static float modPhaseB = 0.0f;
        static float crossfadePhase = 0.0f;
        
        // Store input in both buffers
        shimmerBufferA[shimmerWritePos] = finalDiffused;
        shimmerBufferB[shimmerWritePos] = finalDiffused;
        shimmerWritePos = (shimmerWritePos + 1) % shimmerBufferA.size();
        
        // Elysiera pitch configuration
        float pitchA = 12.0f; // +1 octave (12 semitones)
        float pitchB = 5.0f;  // +perfect 4th (5 semitones)
        float pitchAVol = 0.6f * mReverbShimmer;
        float pitchBVol = 0.6f * mReverbShimmer;
        
        // Modulation oscillators (like Elysiera)
        float modRate = 3.1f; // Hz
        modPhaseA += (modRate / mSampleRate) * TWO_PI;
        modPhaseB += (modRate / mSampleRate) * TWO_PI;
        crossfadePhase += (1024.0f / mSampleRate) * TWO_PI; // Crossfade rate
        
        if (modPhaseA >= TWO_PI) modPhaseA -= TWO_PI;
        if (modPhaseB >= TWO_PI) modPhaseB -= TWO_PI;
        if (crossfadePhase >= TWO_PI) crossfadePhase -= TWO_PI;
        
        // Modulated pitch ratios
        float pitchModA = sin(modPhaseA) * 0.1f; // Small modulation
        float pitchModB = cos(modPhaseB) * 0.1f; // Small modulation
        
        // Convert semitones to pitch ratios with modulation
        float pitchRatioA = pow(2.0f, (pitchA + pitchModA) / 12.0f);
        float pitchRatioB = pow(2.0f, (pitchB + pitchModB) / 12.0f);
        
        // Pitch shifter A (like Elysiera ef.transpose)
        shimmerReadPosA += pitchRatioA;
        if (shimmerReadPosA >= shimmerBufferA.size()) {
            shimmerReadPosA -= shimmerBufferA.size();
        }
        
        // High-quality interpolation for pitch A
        int readIndexA = (int)shimmerReadPosA;
        float fractionA = shimmerReadPosA - readIndexA;
        int nextIndexA = (readIndexA + 1) % shimmerBufferA.size();
        int prevIndexA = (readIndexA - 1 + shimmerBufferA.size()) % shimmerBufferA.size();
        int nextNextIndexA = (readIndexA + 2) % shimmerBufferA.size();
        
        // Cubic interpolation for pitch A
        float aA = shimmerBufferA[prevIndexA];
        float bA = shimmerBufferA[readIndexA];
        float cA = shimmerBufferA[nextIndexA];
        float dA = shimmerBufferA[nextNextIndexA];
        
        float pitchShiftedA = bA + 0.5f * fractionA * (cA - aA + fractionA * (2.0f * aA - 5.0f * bA + 4.0f * cA - dA + fractionA * (3.0f * (bA - cA) + dA - aA)));
        
        // Pitch shifter B (like Elysiera ef.transpose)
        shimmerReadPosB += pitchRatioB;
        if (shimmerReadPosB >= shimmerBufferB.size()) {
            shimmerReadPosB -= shimmerBufferB.size();
        }
        
        // High-quality interpolation for pitch B
        int readIndexB = (int)shimmerReadPosB;
        float fractionB = shimmerReadPosB - readIndexB;
        int nextIndexB = (readIndexB + 1) % shimmerBufferB.size();
        int prevIndexB = (readIndexB - 1 + shimmerBufferB.size()) % shimmerBufferB.size();
        int nextNextIndexB = (readIndexB + 2) % shimmerBufferB.size();
        
        // Cubic interpolation for pitch B
        float aB = shimmerBufferB[prevIndexB];
        float bB = shimmerBufferB[readIndexB];
        float cB = shimmerBufferB[nextIndexB];
        float dB = shimmerBufferB[nextNextIndexB];
        
        float pitchShiftedB = bB + 0.5f * fractionB * (cB - aB + fractionB * (2.0f * aB - 5.0f * bB + 4.0f * cB - dB + fractionB * (3.0f * (bB - cB) + dB - aB)));
        
        // Elysiera-style crossfading and modulation
        float crossfade = (sin(crossfadePhase) + 1.0f) * 0.5f; // 0 to 1
        
        // Mix the two pitch shifters with crossfading (like Elysiera)
        float shimmerMixA = pitchShiftedA * pitchAVol * (1.0f - crossfade);
        float shimmerMixB = pitchShiftedB * pitchBVol * crossfade;
        
        // Apply anti-aliasing to shimmer output
        static float shimmerAAFilter1 = 0.0f;
        static float shimmerAAFilter2 = 0.0f;
        
        float shimmerMixed = shimmerMixA + shimmerMixB;
        
        // Two-stage anti-aliasing
        shimmerAAFilter1 = shimmerAAFilter1 * 0.8f + shimmerMixed * 0.2f;
        shimmerAAFilter2 = shimmerAAFilter2 * 0.7f + shimmerAAFilter1 * 0.3f;
        
        float cleanShimmer = shimmerAAFilter2;
        
        // Mix with original reverb (Elysiera style)
        float shimmerAmount = mReverbShimmer;
        shimmerOutput = finalDiffused * (1.0f - shimmerAmount) + cleanShimmer * shimmerAmount;
    }
    
    // ===== STAGE 5: FINAL OUTPUT WITH ENHANCED ANTI-ALIASING =====
    float finalReverb = shimmerOutput;
    
    // Multi-stage anti-aliasing to eliminate remaining buzz
    static float antiAliasingFilter1 = 0.0f;
    static float antiAliasingFilter2 = 0.0f;
    static float antiAliasingFilter3 = 0.0f;
    
    // First stage: aggressive low-pass
    float cutoffFreq1 = 0.6f; // Very aggressive
    antiAliasingFilter1 = antiAliasingFilter1 * (1.0f - cutoffFreq1) + finalReverb * cutoffFreq1;
    
    // Second stage: medium low-pass
    float cutoffFreq2 = 0.4f;
    antiAliasingFilter2 = antiAliasingFilter2 * (1.0f - cutoffFreq2) + antiAliasingFilter1 * cutoffFreq2;
    
    // Third stage: gentle low-pass for smoothness
    float cutoffFreq3 = 0.2f;
    antiAliasingFilter3 = antiAliasingFilter3 * (1.0f - cutoffFreq3) + antiAliasingFilter2 * cutoffFreq3;
    
    finalReverb = antiAliasingFilter3;
    
    // Very gentle saturation to avoid harsh artifacts
    if (fabs(finalReverb) > 0.2f) {
        finalReverb = tanh(finalReverb * 0.2f) * 5.0f; // Even gentler compression
    }
    
    // Proper gain scaling
    finalReverb *= 0.2f; // Further reduced to prevent loudness issues
    
    return finalReverb;
}