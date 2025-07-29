#include "pluginprocessor.h"
#include "plugincontroller.h"
#include "pluginids.h"
#include "vstlogger.h"

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
    
    // Initialize additional filter states to prevent "bee buzzing"
    for (int i = 0; i < 2; i++) {
        mAmpSmoothFilter[i] = 0.0f;
        mEqSmoothFilter1[i] = 0.0f;
        mEqSmoothFilter2[i] = 0.0f;
        mLowMidFilter[i] = 0.0f;
        mHighMidFilter[i] = 0.0f;
        mEqGateState[i] = 0.0f;
        mGateState[i] = 0.0f;
    }
    
    // Initialize distortion state variables
    mDistGateState = 0.0f;
    mDistortionSmoothFilter1 = 0.0f;
    mDistortionSmoothFilter2 = 0.0f;
    mFuzzSmoother = 0.0f;
    
    // Initialize modulation state variables
    mModGateState = 0.0f;
    mLfoSmoother = 0.5f;
    mModSmoothFilter1 = 0.0f;
    mModSmoothFilter2 = 0.0f;
    mFlangerFeedback = 0.0f;
    mPhaserStage1 = 0.0f;
    mPhaserStage2 = 0.0f;
    mPhaserStage3 = 0.0f;
    mPhaserStage4 = 0.0f;
    mPhaserFeedback = 0.0f;
    
    // Initialize new tube-style amp simulation state variables
    for (int i = 0; i < 2; i++) {
        mTubePreampState[i] = 0.0f;
        mToneStackLowpass[i] = 0.0f;
        mToneStackHighpass[i] = 0.0f;
        mToneStackMidband[i] = 0.0f;
        mCabinetFilter1[i] = 0.0f;
        mCabinetFilter2[i] = 0.0f;
        mCabinetFilter3[i] = 0.0f;
        mSpeakerResonance[i] = 0.0f;
        mTubeCompressionState[i] = 0.0f;
        mInputHighpass[i] = 0.0f;
        mOutputLowpass[i] = 0.0f;
        
        // Initialize NAM-inspired neural network state
        mHistoryIndex[i] = 0;
        mDynamicGain[i] = 1.0f;
        
        for (int j = 0; j < 8; j++) {
            mNeuralHistory[i][j] = 0.0f;
        }
        
        for (int j = 0; j < 3; j++) {
            mNeuralActivation[i][j] = 0.0f;
        }
        
        for (int j = 0; j < 4; j++) {
            mMemoryState[i][j] = 0.0f;
        }
    }
    
    // Initialize neural network weights and biases (simplified amp model)
    // Layer 1: Input processing
    mNeuralWeights[0][0] = 0.8f;  mNeuralWeights[0][1] = -0.3f; mNeuralWeights[0][2] = 0.6f;  mNeuralWeights[0][3] = -0.2f;
    mNeuralWeights[0][4] = 0.4f;  mNeuralWeights[0][5] = -0.7f; mNeuralWeights[0][6] = 0.5f;  mNeuralWeights[0][7] = -0.1f;
    
    // Layer 2: Nonlinear processing
    mNeuralWeights[1][0] = 1.2f;  mNeuralWeights[1][1] = -0.8f; mNeuralWeights[1][2] = 0.9f;  mNeuralWeights[1][3] = -0.4f;
    mNeuralWeights[1][4] = 0.7f;  mNeuralWeights[1][5] = -0.6f; mNeuralWeights[1][6] = 1.1f;  mNeuralWeights[1][7] = -0.3f;
    
    // Layer 3: Output shaping
    mNeuralWeights[2][0] = 0.9f;  mNeuralWeights[2][1] = -0.5f; mNeuralWeights[2][2] = 0.8f;  mNeuralWeights[2][3] = -0.2f;
    mNeuralWeights[2][4] = 0.6f;  mNeuralWeights[2][5] = -0.4f; mNeuralWeights[2][6] = 0.7f;  mNeuralWeights[2][7] = -0.1f;
    
    // Biases
    mNeuralBias[0] = 0.1f;
    mNeuralBias[1] = -0.05f;
    mNeuralBias[2] = 0.02f;
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

    // Initialize logging system
    VSTLogger::getInstance().initialize("C:/temp/amneziagaze_realtime_log.txt");
    VST_LOG_INFO("System", "plugin_initialized", 1.0f, "AMNEZIAGAZE v0.3.1 started");

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
    
    // Reset additional filter states to prevent "bee buzzing"
    for (int i = 0; i < 2; i++) {
        mAmpSmoothFilter[i] = 0.0f;
        mEqSmoothFilter1[i] = 0.0f;
        mEqSmoothFilter2[i] = 0.0f;
        mLowMidFilter[i] = 0.0f;
        mHighMidFilter[i] = 0.0f;
        mEqGateState[i] = 0.0f;
        mGateState[i] = 0.0f;
    }
    
    // Reset distortion state variables
    mDistGateState = 0.0f;
    mDistortionSmoothFilter1 = 0.0f;
    mDistortionSmoothFilter2 = 0.0f;
    mFuzzSmoother = 0.0f;
    
    // Reset modulation state variables
    mModGateState = 0.0f;
    mLfoSmoother = 0.5f;
    mModSmoothFilter1 = 0.0f;
    mModSmoothFilter2 = 0.0f;
    mFlangerFeedback = 0.0f;
    mPhaserStage1 = 0.0f;
    mPhaserStage2 = 0.0f;
    mPhaserStage3 = 0.0f;
    mPhaserStage4 = 0.0f;
    mPhaserFeedback = 0.0f;
    
    // Reset new tube-style amp simulation state variables
    for (int i = 0; i < 2; i++) {
        mTubePreampState[i] = 0.0f;
        mToneStackLowpass[i] = 0.0f;
        mToneStackHighpass[i] = 0.0f;
        mToneStackMidband[i] = 0.0f;
        mCabinetFilter1[i] = 0.0f;
        mCabinetFilter2[i] = 0.0f;
        mCabinetFilter3[i] = 0.0f;
        mSpeakerResonance[i] = 0.0f;
        mTubeCompressionState[i] = 0.0f;
        mInputHighpass[i] = 0.0f;
        mOutputLowpass[i] = 0.0f;
        
        // Reset NAM-inspired neural network state
        mHistoryIndex[i] = 0;
        mDynamicGain[i] = 1.0f;
        
        for (int j = 0; j < 8; j++) {
            mNeuralHistory[i][j] = 0.0f;
        }
        
        for (int j = 0; j < 3; j++) {
            mNeuralActivation[i][j] = 0.0f;
        }
        
        for (int j = 0; j < 4; j++) {
            mMemoryState[i][j] = 0.0f;
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
                        
                        // Handle parameter changes with logging
                        switch (paramQueue->getParameterId())
                        {
                            // Bypass Parameters
                            case kParamAmpBypassId:
                                VST_LOG_PARAM_CHANGE("AmpBypass", mAmpBypass, value);
                                mAmpBypass = value;
                                break;
                            case kParamDistBypassId:
                                VST_LOG_PARAM_CHANGE("DistBypass", mDistBypass, value);
                                mDistBypass = value;
                                break;
                            case kParamReverbBypassId:
                                VST_LOG_PARAM_CHANGE("ReverbBypass", mReverbBypass, value);
                                mReverbBypass = value;
                                break;
                            case kParamDelayBypassId:
                                VST_LOG_PARAM_CHANGE("DelayBypass", mDelayBypass, value);
                                mDelayBypass = value;
                                break;
                            case kParamModBypassId:
                                VST_LOG_PARAM_CHANGE("ModBypass", mModBypass, value);
                                mModBypass = value;
                                break;
                                
                            // Amp Section
                            case kParamGainId:
                                VST_LOG_PARAM_CHANGE("Gain", mGain, value);
                                mGain = value;
                                break;
                            case kParamBassId:
                                VST_LOG_PARAM_CHANGE("Bass", mBass, value);
                                mBass = value;
                                break;
                            case kParamMidId:
                                VST_LOG_PARAM_CHANGE("Mid", mMid, value);
                                mMid = value;
                                break;
                            case kParamTrebleId:
                                VST_LOG_PARAM_CHANGE("Treble", mTreble, value);
                                mTreble = value;
                                break;
                            case kParamPresenceId:
                                VST_LOG_PARAM_CHANGE("Presence", mPresence, value);
                                mPresence = value;
                                break;
                            case kParamOutputLevelId:
                                VST_LOG_PARAM_CHANGE("OutputLevel", mOutputLevel, value);
                                mOutputLevel = value;
                                break;
                                
                            // Distortion Section
                            case kParamDistTypeId:
                                {
                                    int oldType = mDistType;
                                    mDistType = (int)(value * (kNumDistTypes - 1) + 0.5f);
                                    VST_LOG_PARAM_CHANGE("DistType", (float)oldType, (float)mDistType);
                                }
                                break;
                            case kParamDistDriveId:
                                VST_LOG_PARAM_CHANGE("DistDrive", mDistDrive, value);
                                mDistDrive = value;
                                break;
                                
                            // Reverb Section
                            case kParamReverbMixId:
                                VST_LOG_PARAM_CHANGE("ReverbMix", mReverbMix, value);
                                mReverbMix = value;
                                break;
                            case kParamReverbSizeId:
                                VST_LOG_PARAM_CHANGE("ReverbSize", mReverbSize, value);
                                mReverbSize = value;
                                break;
                            case kParamReverbReverseId:
                                VST_LOG_PARAM_CHANGE("ReverbReverse", mReverbReverse, value);
                                mReverbReverse = value;
                                break;
                            case kParamReverbShimmerId:
                                VST_LOG_PARAM_CHANGE("ReverbShimmer", mReverbShimmer, value);
                                mReverbShimmer = value;
                                break;
                                
                            // Delay Section
                            case kParamDelayMixId:
                                VST_LOG_PARAM_CHANGE("DelayMix", mDelayMix, value);
                                mDelayMix = value;
                                break;
                            case kParamDelayTimeId:
                                VST_LOG_PARAM_CHANGE("DelayTime", mDelayTime, value);
                                mDelayTime = value;
                                break;
                            case kParamDelayFeedbackId:
                                VST_LOG_PARAM_CHANGE("DelayFeedback", mDelayFeedback, value);
                                mDelayFeedback = value;
                                break;
                            case kParamDelayReverseId:
                                VST_LOG_PARAM_CHANGE("DelayReverse", mDelayReverse, value);
                                mDelayReverse = value;
                                break;
                                
                            // Modulation Section
                            case kParamModTypeId:
                                {
                                    int oldType = mModType;
                                    mModType = (int)(value * (kNumModTypes - 1) + 0.5f);
                                    VST_LOG_PARAM_CHANGE("ModType", (float)oldType, (float)mModType);
                                }
                                break;
                            case kParamModRateId:
                                VST_LOG_PARAM_CHANGE("ModRate", mModRate, value);
                                mModRate = value;
                                break;
                            case kParamModDepthId:
                                VST_LOG_PARAM_CHANGE("ModDepth", mModDepth, value);
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
        
        // Process samples with logging
        for (int32 sample = 0; sample < data.numSamples; sample++)
        {
            float processed = ptrIn[sample];
            float originalInput = processed;
            
            // Log input level and detect clipping
            if (fabs(originalInput) > 0.95f) {
                VST_LOG_CLIPPING("Input", originalInput, 0.95f);
            }
            
            // Apply amp simulation and EQ (with bypass)
            if (mAmpBypass <= 0.5f) {
                float preAmp = processed;
                processed = processAmp(processed, channel);
                VST_LOG_AUDIO("Amp", preAmp, processed, "channel_" + std::to_string(channel));
                
                if (fabs(processed) > 0.95f) {
                    VST_LOG_CLIPPING("Amp", processed, 0.95f);
                }
            }
            
            // Apply distortion (with bypass)
            if (mDistBypass <= 0.5f) {
                float preDist = processed;
                processed = processDistortion(processed);
                VST_LOG_AUDIO("Distortion", preDist, processed, "type_" + std::to_string(mDistType));
                
                if (fabs(processed) > 0.95f) {
                    VST_LOG_CLIPPING("Distortion", processed, 0.95f);
                }
            }
            
            // Apply modulation (chorus/flanger/phaser) (with bypass)
            if (mModBypass <= 0.5f) {
                float preMod = processed;
                processed = processModulation(processed);
                VST_LOG_AUDIO("Modulation", preMod, processed, "type_" + std::to_string(mModType));
                
                if (fabs(processed) > 0.95f) {
                    VST_LOG_CLIPPING("Modulation", processed, 0.95f);
                }
            }
            
            // Apply delay (with bypass)
            if (mDelayBypass <= 0.5f) {
                float preDelay = processed;
                processed = processDelay(processed);
                VST_LOG_AUDIO("Delay", preDelay, processed, "reverse_" + std::to_string(mDelayReverse > 0.5f ? 1 : 0));
                
                if (fabs(processed) > 0.95f) {
                    VST_LOG_CLIPPING("Delay", processed, 0.95f);
                }
            }
            
            // Apply reverb (with bypass)
            if (mReverbBypass <= 0.5f) {
                float preReverb = processed;
                processed = processReverb(processed);
                VST_LOG_AUDIO("Reverb", preReverb, processed, "shimmer_" + std::to_string(mReverbShimmer));
                
                if (fabs(processed) > 0.95f) {
                    VST_LOG_CLIPPING("Reverb", processed, 0.95f);
                }
            }
            
            // Apply output level (always applied, even when amp is bypassed)
            float preOutput = processed;
            processed = processed * mOutputLevel;
            VST_LOG_AUDIO("Output", preOutput, processed, "level_" + std::to_string(mOutputLevel));
            
            // Final clipping check
            if (fabs(processed) > 0.98f) {
                VST_LOG_CLIPPING("FinalOutput", processed, 0.98f);
            }
            
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
    // NAM-inspired neural amp modeling approach
    
    // ===== STAGE 1: INPUT HISTORY COLLECTION =====
    // Store input history for neural network processing (like NAM's temporal modeling)
    mNeuralHistory[channel][mHistoryIndex[channel]] = input;
    mHistoryIndex[channel] = (mHistoryIndex[channel] + 1) % 8;
    
    // ===== STAGE 2: DYNAMIC GAIN ADAPTATION =====
    // Simulate amp's dynamic response to input level (like NAM's level-dependent modeling)
    float inputLevel = fabs(input);
    float targetGain = 1.0f + mGain * 2.0f; // Base gain from user control
    
    // Dynamic gain adjustment based on input level (amp compression/expansion)
    if (inputLevel > 0.1f) {
        targetGain *= (1.0f - (inputLevel - 0.1f) * 0.3f); // Compression at high levels
    } else {
        targetGain *= (1.0f + (0.1f - inputLevel) * 0.2f); // Slight expansion at low levels
    }
    
    // Smooth gain changes to avoid artifacts
    mDynamicGain[channel] += (targetGain - mDynamicGain[channel]) * 0.01f;
    
    // ===== STAGE 3: NEURAL NETWORK PROCESSING =====
    // Simplified 3-layer neural network inspired by NAM architecture
    
    // Layer 1: Input processing with history
    float layer1_sum = mNeuralBias[0];
    for (int i = 0; i < 8; i++) {
        int histIdx = (mHistoryIndex[channel] + i) % 8;
        layer1_sum += mNeuralHistory[channel][histIdx] * mNeuralWeights[0][i];
    }
    mNeuralActivation[channel][0] = tanh(layer1_sum); // Activation function
    
    // Layer 2: Nonlinear processing (main amp character)
    float layer2_input = mNeuralActivation[channel][0] * mDynamicGain[channel];
    float layer2_sum = mNeuralBias[1];
    
    // Use current and previous activations for temporal modeling
    layer2_sum += layer2_input * mNeuralWeights[1][0];
    layer2_sum += mMemoryState[channel][0] * mNeuralWeights[1][1]; // Memory feedback
    layer2_sum += mMemoryState[channel][1] * mNeuralWeights[1][2];
    layer2_sum += mMemoryState[channel][2] * mNeuralWeights[1][3];
    layer2_sum += layer2_input * layer2_input * mNeuralWeights[1][4]; // Nonlinear term
    layer2_sum += layer2_input * mMemoryState[channel][0] * mNeuralWeights[1][5]; // Cross term
    layer2_sum += sin(layer2_input * 3.14159f) * mNeuralWeights[1][6] * 0.1f; // Harmonic content
    layer2_sum += layer2_input * fabs(layer2_input) * mNeuralWeights[1][7] * 0.5f; // Asymmetric term
    
    mNeuralActivation[channel][1] = tanh(layer2_sum * 0.8f); // Prevent saturation
    
    // Layer 3: Output shaping and filtering
    float layer3_sum = mNeuralBias[2];
    layer3_sum += mNeuralActivation[channel][1] * mNeuralWeights[2][0];
    layer3_sum += mNeuralActivation[channel][0] * mNeuralWeights[2][1]; // Skip connection
    layer3_sum += mMemoryState[channel][3] * mNeuralWeights[2][2]; // Long-term memory
    layer3_sum += input * mNeuralWeights[2][3] * 0.1f; // Dry signal blend
    
    float neuralOutput = tanh(layer3_sum);
    
    // ===== STAGE 4: MEMORY STATE UPDATE =====
    // Update memory states for next sample (amp's temporal behavior)
    mMemoryState[channel][3] = mMemoryState[channel][2];
    mMemoryState[channel][2] = mMemoryState[channel][1];
    mMemoryState[channel][1] = mMemoryState[channel][0];
    mMemoryState[channel][0] = mNeuralActivation[channel][1];
    
    // ===== STAGE 5: FINAL PROCESSING =====
    // Apply EQ and final output scaling
    float eqOutput = processEQ(neuralOutput, channel);
    
    // Conservative output scaling
    return eqOutput * 0.8f;
}

//-----------------------------------------------------------------------------
// Tone Stack Simulation (Classic Guitar Amp EQ)
//-----------------------------------------------------------------------------
float PluginProcessor::processToneStack(float input, int channel)
{
    // Classic guitar amp tone stack (based on Fender/Marshall circuits)
    // This simulates the interactive bass/mid/treble controls found in tube amps
    
    // ===== BASS CONTROL (Low Shelf Filter) =====
    // Bass control affects low frequencies (80-300Hz)
    float bassFreq = 0.08f; // ~150Hz equivalent at 44.1kHz
    float bassGain = 0.3f + mBass * 1.4f; // 0.3x to 1.7x gain range
    
    // Low shelf filter implementation
    mToneStackLowpass[channel] = mToneStackLowpass[channel] * (1.0f - bassFreq) + input * bassFreq;
    float bassComponent = mToneStackLowpass[channel] * bassGain;
    float bassProcessed = input * (1.0f - bassFreq) + bassComponent * bassFreq;
    
    // ===== TREBLE CONTROL (High Shelf Filter) =====
    // Treble control affects high frequencies (3kHz+)
    float trebleFreq = 0.25f; // ~3kHz equivalent at 44.1kHz
    float trebleGain = 0.4f + mTreble * 1.2f; // 0.4x to 1.6x gain range
    
    // High shelf filter implementation
    mToneStackHighpass[channel] = mToneStackHighpass[channel] * (1.0f - trebleFreq) + bassProcessed * trebleFreq;
    float trebleComponent = (bassProcessed - mToneStackHighpass[channel]) * trebleGain;
    float trebleProcessed = bassProcessed * (1.0f - trebleFreq) + (mToneStackHighpass[channel] + trebleComponent) * trebleFreq;
    
    // ===== MID CONTROL (Bandpass/Notch Filter) =====
    // Mid control affects midrange frequencies (300Hz-3kHz)
    float midFreq = 0.15f; // ~1kHz equivalent at 44.1kHz
    float midGain = 0.5f + (mMid - 0.5f) * 1.0f; // 0.0x to 1.0x range (can cut or boost)
    
    // Bandpass filter for midrange
    mToneStackMidband[channel] = mToneStackMidband[channel] * (1.0f - midFreq) + trebleProcessed * midFreq;
    float midComponent = (trebleProcessed - mToneStackMidband[channel]) * midGain;
    float midProcessed = trebleProcessed * 0.7f + midComponent * 0.3f;
    
    // ===== PRESENCE CONTROL (High-Mid Boost) =====
    // Presence adds clarity and bite in the upper midrange (1-5kHz)
    float presenceAmount = mPresence * 0.3f; // Subtle effect
    float presenceFreq = 0.18f; // ~1.5kHz equivalent
    
    // Simple high-mid boost
    float presenceFiltered = midProcessed - mToneStackMidband[channel] * 0.5f;
    float presenceOutput = midProcessed + presenceFiltered * presenceAmount;
    
    // ===== TONE STACK INTERACTION =====
    // Real guitar amp tone stacks have interactive controls - when you turn up bass, it affects mids, etc.
    float interaction = (mBass + mTreble) * 0.1f; // Subtle interaction effect
    float interactionGain = 1.0f - interaction * 0.2f; // Slight mid scoop when bass/treble are high
    
    float finalToneStack = presenceOutput * interactionGain;
    
    // Gentle saturation to simulate tube tone stack loading
    finalToneStack = tanh(finalToneStack * 1.1f) * 0.95f;
    
    return finalToneStack;
}

//-----------------------------------------------------------------------------
// Cabinet and Speaker Simulation
//-----------------------------------------------------------------------------
float PluginProcessor::processCabinetSimulation(float input, int channel)
{
    // Simulate a 4x12 guitar cabinet with Celestion-style speakers
    // This includes cabinet resonance, speaker frequency response, and room acoustics
    
    // ===== CABINET RESONANCE =====
    // Guitar cabinets have resonant frequencies that color the sound
    float cabinetResonanceFreq = 0.06f; // ~100Hz cabinet resonance
    mSpeakerResonance[channel] = mSpeakerResonance[channel] * (1.0f - cabinetResonanceFreq) + input * cabinetResonanceFreq;
    float resonanceBoost = mSpeakerResonance[channel] * 0.15f; // Subtle low-end thump
    float resonanceProcessed = input + resonanceBoost;
    
    // ===== SPEAKER FREQUENCY RESPONSE =====
    // Multi-stage filtering to simulate speaker characteristics
    
    // Stage 1: Low-frequency rolloff (speakers don't reproduce very low frequencies well)
    float lowRolloffFreq = 0.04f; // ~80Hz rolloff
    mCabinetFilter1[channel] = mCabinetFilter1[channel] * (1.0f - lowRolloffFreq) + resonanceProcessed * lowRolloffFreq;
    float stage1 = resonanceProcessed - mCabinetFilter1[channel] * 0.3f;
    
    // Stage 2: Mid-frequency presence (speakers have a presence peak around 2-4kHz)
    float presencePeakFreq = 0.20f; // ~2.5kHz presence peak
    mCabinetFilter2[channel] = mCabinetFilter2[channel] * (1.0f - presencePeakFreq) + stage1 * presencePeakFreq;
    float presencePeak = (stage1 - mCabinetFilter2[channel]) * 0.2f; // Subtle presence boost
    float stage2 = stage1 + presencePeak;
    
    // Stage 3: High-frequency rolloff (speakers naturally roll off high frequencies)
    float highRolloffFreq = 0.35f; // ~5kHz rolloff starts
    mCabinetFilter3[channel] = mCabinetFilter3[channel] * (1.0f - highRolloffFreq) + stage2 * highRolloffFreq;
    float stage3 = mCabinetFilter3[channel]; // Smooth high-frequency rolloff
    
    // ===== SPEAKER SATURATION =====
    // Speakers add subtle compression and harmonic distortion at high levels
    float speakerLevel = fabs(stage3);
    float speakerSaturation = 1.0f / (1.0f + speakerLevel * 0.5f); // Gentle compression
    float speakerOutput = stage3 * speakerSaturation;
    
    // Add subtle speaker cone resonance (very subtle harmonic content)
    float coneResonance = sin(speakerOutput * 2.5f) * 0.008f * speakerLevel;
    speakerOutput += coneResonance;
    
    // ===== CABINET AIR MOVEMENT =====
    // Simulate the air movement and room acoustics of a guitar cabinet
    float airMovement = tanh(speakerOutput * 0.9f) * 1.05f; // Subtle air compression
    
    return airMovement * 0.85f; // Conservative output level
}

//-----------------------------------------------------------------------------
// EQ processing
//-----------------------------------------------------------------------------
float PluginProcessor::processEQ(float input, int channel)
{
    // Simple, clean EQ without complex filtering or mixing
    
    // Simple bass adjustment (just multiply by gain)
    float bassGain = 0.5f + mBass * 1.0f; // 0.5x to 1.5x
    
    // Simple treble adjustment (just multiply by gain)
    float trebleGain = 0.5f + mTreble * 1.0f; // 0.5x to 1.5x
    
    // Simple mid adjustment
    float midGain = 0.5f + mMid * 1.0f; // 0.5x to 1.5x
    
    // Apply simple gain adjustments without complex filtering
    float output = input * bassGain * midGain * trebleGain;
    
    // Simple limiting to prevent clipping
    if (output > 1.0f) output = 1.0f;
    if (output < -1.0f) output = -1.0f;
    
    return output;
}

//-----------------------------------------------------------------------------
// Distortion processing
//-----------------------------------------------------------------------------
float PluginProcessor::processDistortion(float input)
{
    // Simple, clean distortion without noise gates or complex processing
    
    // Simple drive control
    float drive = 1.0f + mDistDrive * 4.0f; // 1x to 5x drive
    float driven = input * drive;
    
    float distorted = 0.0f;
    
    // Simple, clean distortion algorithms
    switch (mDistType)
    {
        case kDistClean:
            // Clean: just gentle tanh saturation
            distorted = tanh(driven * 0.8f);
            break;
            
        case kDistCrunch:
            // Crunch: moderate tanh saturation
            distorted = tanh(driven * 1.2f) * 0.9f;
            break;
            
        case kDistFuzz:
            // Fuzz: harder tanh saturation
            distorted = tanh(driven * 1.8f) * 0.8f;
            break;
    }
    
    // Simple output limiting
    if (distorted > 1.0f) distorted = 1.0f;
    if (distorted < -1.0f) distorted = -1.0f;
    
    return distorted * 0.7f; // Conservative output level
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
    
    // Apply noise gate with hysteresis - using member variables
    if (fabs(input) < 0.0003f && mModGateState == 0.0f) {
        return input;
    } else if (fabs(input) > 0.0005f) {
        mModGateState = 1.0f;
    } else if (fabs(input) < 0.0003f) {
        mModGateState = 0.0f;
        return input;
    }
    
    // More conservative modulation rate
    float rate = 0.1f + mModRate * 1.0f; // Further reduced max rate
    
    // Update phase smoothly
    mModPhase += rate / mSampleRate;
    if (mModPhase >= 1.0f) {
        mModPhase -= 1.0f;
    }
    
    // Calculate LFO value with better waveform
    float lfo = 0.5f + 0.5f * sin(TWO_PI * mModPhase);
    
    // Better LFO smoothing to prevent zipper noise - using member variables
    mLfoSmoother = mLfoSmoother * 0.95f + lfo * 0.05f; // More smoothing
    lfo = mLfoSmoother;
    
    // Apply different modulation types with much better algorithms
    float modulated = input;
    
    switch (mModType)
    {
        case kModChorus:
            // Improved chorus with interpolation and feedback control
            {
                // Calculate delay time (3-8ms) - better range for chorus
                float delayMs = 3.0f + lfo * 5.0f;
                float delaySamplesFloat = delayMs * mSampleRate / 1000.0f;
                int delaySamples = (int)delaySamplesFloat;
                float fraction = delaySamplesFloat - delaySamples;
                
                // Ensure delay samples is within bounds
                delaySamples = std::max(1, std::min(delaySamples, mDelayBufferLength - 2));
                
                // Read from delay buffer with linear interpolation
                int readPos = mDelayBufferPos - delaySamples;
                if (readPos < 0) readPos += mDelayBufferLength;
                int nextPos = (readPos + 1) % mDelayBufferLength;
                
                float delayed = mDelayBuffer[readPos] * (1.0f - fraction) +
                               mDelayBuffer[nextPos] * fraction;
                
                // Conservative mixing with proper balance
                float mixAmount = mModDepth * 0.15f; // Slightly increased but still conservative
                modulated = input * (1.0f - mixAmount * 0.5f) + delayed * mixAmount;
            }
            break;
            
        case kModFlanger:
            // Improved flanger with feedback and better delay range
            {
                // Calculate delay time (0.7-2.5ms) - better flanger range
                float delayMs = 0.7f + lfo * 1.8f;
                float delaySamplesFloat = delayMs * mSampleRate / 1000.0f;
                int delaySamples = (int)delaySamplesFloat;
                float fraction = delaySamplesFloat - delaySamples;
                
                // Ensure delay samples is within bounds
                delaySamples = std::max(1, std::min(delaySamples, mDelayBufferLength - 2));
                
                // Read from delay buffer with interpolation
                int readPos = mDelayBufferPos - delaySamples;
                if (readPos < 0) readPos += mDelayBufferLength;
                int nextPos = (readPos + 1) % mDelayBufferLength;
                
                float delayed = mDelayBuffer[readPos] * (1.0f - fraction) +
                               mDelayBuffer[nextPos] * fraction;
                
                // Add feedback for more pronounced flanger effect - using member variables
                mFlangerFeedback = mFlangerFeedback * 0.3f + delayed * 0.7f;
                delayed = delayed + mFlangerFeedback * 0.2f;
                
                // Conservative flanging with inverted phase for sweep effect
                float mixAmount = mModDepth * 0.12f;
                modulated = input + delayed * mixAmount * (lfo > 0.5f ? 1.0f : -1.0f);
            }
            break;
            
        case kModPhaser:
            // Much improved phaser with multiple allpass stages - using member variables
            {
                // Modulated allpass frequency
                float modAmount = mModDepth * 0.08f * lfo;
                float alpha = 0.06f + modAmount * 0.04f; // Better range
                
                // Clamp alpha for stability
                alpha = std::max(0.03f, std::min(0.12f, alpha));
                
                // Four-stage allpass filter cascade - using member variables
                mPhaserStage1 = mPhaserStage1 * (1.0f - alpha) + input * alpha;
                float stage1Out = input - mPhaserStage1;
                
                mPhaserStage2 = mPhaserStage2 * (1.0f - alpha) + stage1Out * alpha;
                float stage2Out = stage1Out - mPhaserStage2;
                
                mPhaserStage3 = mPhaserStage3 * (1.0f - alpha) + stage2Out * alpha;
                float stage3Out = stage2Out - mPhaserStage3;
                
                mPhaserStage4 = mPhaserStage4 * (1.0f - alpha) + stage3Out * alpha;
                float stage4Out = stage3Out - mPhaserStage4;
                
                // Mix with feedback for more pronounced effect - using member variables
                mPhaserFeedback = mPhaserFeedback * 0.7f + stage4Out * 0.3f;
                
                float mixAmount = mModDepth * 0.08f;
                modulated = input * (1.0f - mixAmount) +
                           (stage4Out + mPhaserFeedback * 0.15f) * mixAmount;
            }
            break;
    }
    
    // Two-stage smoothing for better quality - using member variables
    mModSmoothFilter1 = mModSmoothFilter1 * 0.85f + modulated * 0.15f;
    mModSmoothFilter2 = mModSmoothFilter2 * 0.9f + mModSmoothFilter1 * 0.1f;
    
    // Gentle tanh limiting for musical saturation
    float finalOutput = tanh(mModSmoothFilter2 * 0.95f) * 1.02f;
    
    return finalOutput;
}
//-----------------------------------------------------------------------------
// Improved Complex Reverb Algorithm - Fixed Issues
//-----------------------------------------------------------------------------
float PluginProcessor::processComplexReverbSample(float input)
{
    // ===== STAGE 1: SIMPLIFIED INPUT DIFFUSION =====
    // Reduced pre-delay buffer size for less memory usage and latency
    static std::vector<float> preDelayBuffer(8820, 0.0f); // 200ms at 44.1kHz (reduced)
    static int preDelayPos = 0;
    
    int preDelayTime = (int)(mSampleRate * 0.01f * (1.0f + mReverbSize * 1.5f)); // 10-25ms pre-delay (reduced)
    if (preDelayTime >= preDelayBuffer.size()) preDelayTime = preDelayBuffer.size() - 1;
    
    int preDelayReadPos = (preDelayPos + preDelayBuffer.size() - preDelayTime) % preDelayBuffer.size();
    float preDelayed = preDelayBuffer[preDelayReadPos];
    preDelayBuffer[preDelayPos] = input;
    preDelayPos = (preDelayPos + 1) % preDelayBuffer.size();
    
    // Simplified input diffusion with only two allpass filters
    static std::vector<float> allpass1(223, 0.0f);  // Reduced complexity
    static std::vector<float> allpass2(149, 0.0f);
    static int ap1pos = 0, ap2pos = 0;
    
    // Simplified allpass filter function
    auto processAllpass = [&](float in, std::vector<float>& buffer, int& pos, float feedback) -> float {
        float delayed = buffer[pos];
        float output = -in + delayed;
        buffer[pos] = in + delayed * feedback;
        pos = (pos + 1) % buffer.size();
        return output;
    };
    
    float diffused = preDelayed;
    diffused = processAllpass(diffused, allpass1, ap1pos, 0.4f);  // Further reduced feedback
    diffused = processAllpass(diffused, allpass2, ap2pos, 0.4f);
    
    // ===== STAGE 2: SIMPLIFIED COMB FILTERS =====
    // Reduced to 4 comb filters for less CPU usage and cleaner sound
    static std::vector<std::vector<float>> combFilters(4);
    static std::vector<int> combPositions(4, 0);
    static std::vector<float> combOutputs(4, 0.0f);
    static bool combInitialized = false;
    
    if (!combInitialized) {
        // Initialize comb filters with moderate delay times
        int combLengths[] = {1116, 1188, 1277, 1356}; // Classic Schroeder lengths (reduced)
        for (int i = 0; i < 4; i++) {
            float sizeMultiplier = 1.0f + mReverbSize * 8.0f; // Reduced scaling: 1x to 9x
            int length = (int)(combLengths[i] * sizeMultiplier);
            combFilters[i].resize(length, 0.0f);
        }
        combInitialized = true;
    }
    
    // Process through comb filters with moderate sustain
    float combSum = 0.0f;
    float feedback = 0.5f + mReverbSize * 0.3f; // 0.5 to 0.8 feedback (reduced)
    
    for (int i = 0; i < 4; i++) {
        if (!combFilters[i].empty()) {
            // Read delayed sample
            float delayed = combFilters[i][combPositions[i]];
            combOutputs[i] = delayed;
            
            // Apply gentler damping filter
            static float dampingFilters[4] = {0};
            float dampingAmount = 0.15f + (1.0f - mReverbSize) * 0.2f; // Reduced damping
            dampingFilters[i] = dampingFilters[i] * (1.0f - dampingAmount) + delayed * dampingAmount;
            float damped = delayed * (1.0f - dampingAmount) + dampingFilters[i] * dampingAmount;
            
            // Write input + feedback
            combFilters[i][combPositions[i]] = diffused + damped * feedback;
            combPositions[i] = (combPositions[i] + 1) % combFilters[i].size();
            
            // Sum outputs with equal weights for cleaner sound
            combSum += combOutputs[i] * 0.25f; // Equal weighting
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
    
    // ===== STAGE 4: SIMPLIFIED SHIMMER EFFECT =====
    float shimmerOutput = finalDiffused;
    if (mReverbShimmer > 0.01f) {
        // Much simpler shimmer effect to reduce CPU and aliasing
        static std::vector<float> shimmerBuffer(11025, 0.0f); // 250ms buffer (reduced)
        static int shimmerWritePos = 0;
        static float shimmerReadPos = 0.0f;
        static float shimmerPhase = 0.0f;
        
        // Store input in buffer
        shimmerBuffer[shimmerWritePos] = finalDiffused;
        shimmerWritePos = (shimmerWritePos + 1) % shimmerBuffer.size();
        
        // Simple octave-up shimmer (12 semitones)
        float pitchRatio = 2.0f; // Fixed octave up, no modulation
        float shimmerVol = 0.4f * mReverbShimmer; // Reduced volume
        
        // Simple pitch shifting without complex interpolation
        shimmerReadPos += pitchRatio;
        if (shimmerReadPos >= shimmerBuffer.size()) {
            shimmerReadPos -= shimmerBuffer.size();
        }
        
        // Simple linear interpolation only
        int readIndex = (int)shimmerReadPos;
        float fraction = shimmerReadPos - readIndex;
        int nextIndex = (readIndex + 1) % shimmerBuffer.size();
        
        float pitchShifted = shimmerBuffer[readIndex] * (1.0f - fraction) +
                            shimmerBuffer[nextIndex] * fraction;
        
        // Simple low-pass filter to reduce aliasing
        static float shimmerFilter = 0.0f;
        shimmerFilter = shimmerFilter * 0.7f + pitchShifted * 0.3f;
        
        // Mix with original reverb
        float shimmerAmount = mReverbShimmer * 0.6f; // Reduced intensity
        shimmerOutput = finalDiffused * (1.0f - shimmerAmount) + shimmerFilter * shimmerAmount;
    }
    
    // ===== STAGE 5: SIMPLIFIED FINAL OUTPUT =====
    float finalReverb = shimmerOutput;
    
    // Single-stage gentle anti-aliasing
    static float antiAliasingFilter = 0.0f;
    
    // Gentle low-pass filter
    float cutoffFreq = 0.15f; // Much less aggressive
    antiAliasingFilter = antiAliasingFilter * (1.0f - cutoffFreq) + finalReverb * cutoffFreq;
    
    finalReverb = antiAliasingFilter;
    
    // Gentle tanh saturation for musical character
    finalReverb = tanh(finalReverb * 0.8f) * 1.1f;
    
    // Conservative gain scaling
    finalReverb *= 0.4f; // Increased from 0.2f for better level
    
    return finalReverb;
}