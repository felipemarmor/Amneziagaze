#include "pluginprocessor.h"
#include "plugincontroller.h"
#include "pluginids.h"

#include "public.sdk/source/main/pluginfactory.h"

// VST3 Plugin factory implementation
// This is the entry point for the VST3 plugin

using namespace Steinberg;
using namespace Steinberg::Vst;
using namespace MyVSTPlugin;

// Define controller name
static const char* kControllerName = "AMNEZIAGAZE Controller";

// Define the VST3 Plugin Factory
BEGIN_FACTORY_DEF(kPluginVendor,
                 "https://www.pluginslegais.com",
                 "info@pluginslegais.com")

    // Register the processor component
    DEF_CLASS1(kPluginProcessorUID,
              PClassInfo::kManyInstances,
              kVstAudioEffectClass,
              kPluginName,
              MyVSTPlugin::PluginProcessor::createInstance)

    // Register the controller component
    DEF_CLASS1(kPluginControllerUID,
              PClassInfo::kManyInstances,
              kVstComponentControllerClass,
              kControllerName,
              MyVSTPlugin::PluginController::createInstance)

END_FACTORY