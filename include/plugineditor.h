#pragma once

#include "plugincontroller.h"
#include "pluginterfaces/gui/iplugview.h"
#include "base/source/fobject.h"
#include "pluginids.h"
#include <vector>
#include <string>

#if SMTG_OS_WINDOWS
#include <windows.h>
#endif

namespace MyVSTPlugin {

// Forward declarations
class Knob;
class Label;

//-----------------------------------------------------------------------------
// PluginEditor: Custom GUI editor for the VST plugin
//-----------------------------------------------------------------------------
class PluginEditor : public Steinberg::IPlugView, public Steinberg::FObject
{
public:
    // Constructor
    PluginEditor(Steinberg::Vst::EditController* controller);
    
    // Destructor
    virtual ~PluginEditor();
    
    // IPlugView methods
    Steinberg::tresult PLUGIN_API isPlatformTypeSupported(Steinberg::FIDString type) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API attached(void* parent, Steinberg::FIDString type) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API removed() SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API onWheel(float distance) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API onKeyDown(Steinberg::char16 key, Steinberg::int16 keyCode,
                                          Steinberg::int16 modifiers) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API onKeyUp(Steinberg::char16 key, Steinberg::int16 keyCode,
                                        Steinberg::int16 modifiers) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API getSize(Steinberg::ViewRect* size) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API onSize(Steinberg::ViewRect* newSize) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API onFocus(Steinberg::TBool state) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setFrame(Steinberg::IPlugFrame* frame) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API canResize() SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API checkSizeConstraint(Steinberg::ViewRect* rect) SMTG_OVERRIDE;
    
    // FObject methods
    DECLARE_FUNKNOWN_METHODS
    
private:
    // Controller reference
    Steinberg::Vst::EditController* mController;
    
    // Platform-specific view handle
    void* mPlatformHandle;
    
    // Frame reference
    Steinberg::IPlugFrame* mPlugFrame;
    
    // View size
    Steinberg::ViewRect mSize;
    
    // Windows-specific members
#if SMTG_OS_WINDOWS
    HWND mWndHandle;
    HBRUSH mBackgroundBrush;
    HFONT mTitleFont;
    HFONT mLabelFont;
    
    // Window procedure
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    
    // Paint method
    void paint(HDC hdc);
    
    // Mouse handling
    void onMouseDown(int x, int y);
    void onMouseMove(int x, int y);
    void onMouseUp(int x, int y);
#endif
    
    // GUI elements
    struct KnobInfo {
        int x, y;                   // Position
        int paramId;                // Parameter ID
        std::string name;           // Parameter name
        Steinberg::Vst::ParamValue value; // Current value
    };
    
    std::vector<KnobInfo> mKnobs;
    int mDraggingKnob;              // Index of knob being dragged, -1 if none
    
    // Initialize GUI elements
    void initializeGUI();
    
    // Update parameter value
    void updateParameter(int knobIndex, float normalizedValue);
    
    // Get parameter value from controller
    float getParameterValue(int paramId);
};

} // namespace MyVSTPlugin