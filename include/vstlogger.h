#pragma once

#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <mutex>

namespace MyVSTPlugin {

class VSTLogger {
public:
    enum LogLevel {
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        ERROR = 3
    };

    static VSTLogger& getInstance() {
        static VSTLogger instance;
        return instance;
    }

    void initialize(const std::string& logPath = "C:/temp/amneziagaze_log.txt") {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!isInitialized_) {
            logFile_.open(logPath, std::ios::out | std::ios::trunc);
            if (logFile_.is_open()) {
                isInitialized_ = true;
                logFile_ << "=== AMNEZIAGAZE VST Real-Time Log Started ===" << std::endl;
                logFile_ << "Timestamp,Level,Component,Parameter,Value,Additional_Info" << std::endl;
                logFile_.flush();
            }
        }
    }

    void log(LogLevel level, const std::string& component, const std::string& parameter, 
             float value, const std::string& additionalInfo = "") {
        if (!isInitialized_) return;
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Get current timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
        ss << "." << std::setfill('0') << std::setw(3) << ms.count();
        
        // Write log entry
        logFile_ << ss.str() << ","
                << getLevelString(level) << ","
                << component << ","
                << parameter << ","
                << std::fixed << std::setprecision(6) << value << ","
                << additionalInfo << std::endl;
        
        // Flush every few entries to ensure real-time logging
        if (++entryCount_ % 10 == 0) {
            logFile_.flush();
        }
    }

    void logAudioSample(const std::string& stage, float input, float output, 
                       const std::string& additionalInfo = "") {
        if (!isInitialized_) return;
        
        // Only log every Nth sample to avoid overwhelming the log
        static int sampleCounter = 0;
        if (++sampleCounter % 1000 == 0) { // Log every 1000th sample
            log(DEBUG, stage, "input", input, "sample_" + std::to_string(sampleCounter));
            log(DEBUG, stage, "output", output, "sample_" + std::to_string(sampleCounter) + "_" + additionalInfo);
        }
    }

    void logParameterChange(const std::string& paramName, float oldValue, float newValue) {
        if (!isInitialized_) return;
        
        std::stringstream info;
        info << "changed_from_" << std::fixed << std::setprecision(3) << oldValue;
        log(INFO, "Parameter", paramName, newValue, info.str());
    }

    void logEffectState(const std::string& effectName, bool bypassed, 
                       const std::string& settings = "") {
        if (!isInitialized_) return;
        
        log(INFO, effectName, "bypassed", bypassed ? 1.0f : 0.0f, settings);
    }

    void logClipping(const std::string& stage, float value, float threshold) {
        if (!isInitialized_) return;
        
        std::stringstream info;
        info << "clipped_at_threshold_" << std::fixed << std::setprecision(3) << threshold;
        log(WARNING, stage, "clipping", value, info.str());
    }

    void logFrequencyAnalysis(const std::string& stage, float rms, float peak, 
                             float spectralCentroid = 0.0f) {
        if (!isInitialized_) return;
        
        static int analysisCounter = 0;
        if (++analysisCounter % 4410 == 0) { // Log every ~100ms at 44.1kHz
            log(DEBUG, stage, "rms", rms, "frequency_analysis");
            log(DEBUG, stage, "peak", peak, "frequency_analysis");
            if (spectralCentroid > 0.0f) {
                log(DEBUG, stage, "spectral_centroid", spectralCentroid, "frequency_analysis");
            }
        }
    }

    ~VSTLogger() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (logFile_.is_open()) {
            logFile_ << "=== AMNEZIAGAZE VST Log Ended ===" << std::endl;
            logFile_.close();
        }
    }

private:
    VSTLogger() : isInitialized_(false), entryCount_(0) {}
    
    std::string getLevelString(LogLevel level) {
        switch (level) {
            case DEBUG: return "DEBUG";
            case INFO: return "INFO";
            case WARNING: return "WARNING";
            case ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

    std::ofstream logFile_;
    bool isInitialized_;
    int entryCount_;
    std::mutex mutex_;
};

// Convenience macros for logging
#define VST_LOG_DEBUG(component, param, value, info) \
    VSTLogger::getInstance().log(VSTLogger::DEBUG, component, param, value, info)

#define VST_LOG_INFO(component, param, value, info) \
    VSTLogger::getInstance().log(VSTLogger::INFO, component, param, value, info)

#define VST_LOG_WARNING(component, param, value, info) \
    VSTLogger::getInstance().log(VSTLogger::WARNING, component, param, value, info)

#define VST_LOG_ERROR(component, param, value, info) \
    VSTLogger::getInstance().log(VSTLogger::ERROR, component, param, value, info)

#define VST_LOG_AUDIO(stage, input, output, info) \
    VSTLogger::getInstance().logAudioSample(stage, input, output, info)

#define VST_LOG_PARAM_CHANGE(name, oldVal, newVal) \
    VSTLogger::getInstance().logParameterChange(name, oldVal, newVal)

#define VST_LOG_EFFECT_STATE(name, bypassed, settings) \
    VSTLogger::getInstance().logEffectState(name, bypassed, settings)

#define VST_LOG_CLIPPING(stage, value, threshold) \
    VSTLogger::getInstance().logClipping(stage, value, threshold)

#define VST_LOG_FREQUENCY(stage, rms, peak, centroid) \
    VSTLogger::getInstance().logFrequencyAnalysis(stage, rms, peak, centroid)

} // namespace MyVSTPlugin