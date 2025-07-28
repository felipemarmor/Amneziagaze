#include "plugineditor.h"
#include "pluginids.h"
#include <algorithm>
#include <cmath>

#if SMTG_OS_WINDOWS
#include <windowsx.h> // For GET_X_LPARAM and GET_Y_LPARAM
#endif

using namespace Steinberg;
using namespace Steinberg::Vst;
using namespace MyVSTPlugin;

// Define the COM interface
DEF_CLASS_IID(IPlugView)
IMPLEMENT_FUNKNOWN_METHODS(PluginEditor, IPlugView, IPlugView::iid)

// Define colors
#define COLOR_BACKGROUND RGB(40, 40, 45)
#define COLOR_KNOB RGB(80, 80, 85)
#define COLOR_KNOB_HIGHLIGHT RGB(120, 120, 125)
#define COLOR_TEXT RGB(220, 220, 225)
#define COLOR_TITLE RGB(180, 180, 255)
#define COLOR_SECTION RGB(150, 150, 155)

// Define knob size
#define KNOB_RADIUS 25
#define KNOB_SPACING_X 100
#define KNOB_SPACING_Y 120
#define KNOB_LABEL_OFFSET 35
#define KNOB_VALUE_OFFSET 50

//-----------------------------------------------------------------------------
PluginEditor::PluginEditor(EditController* controller)
: mController(controller)
, mPlatformHandle(nullptr)
, mPlugFrame(nullptr)
, mDraggingKnob(-1)
#if SMTG_OS_WINDOWS
, mWndHandle(nullptr)
, mBackgroundBrush(nullptr)
, mTitleFont(nullptr)
, mLabelFont(nullptr)
#endif
{
    // Set default size
    mSize.left = 0;
    mSize.top = 0;
    mSize.right = 800;
    mSize.bottom = 600;
    
    // Initialize GUI elements
    initializeGUI();
}

//-----------------------------------------------------------------------------
PluginEditor::~PluginEditor()
{
    // Clean up resources
    if (mPlugFrame)
    {
        mPlugFrame->release();
        mPlugFrame = nullptr;
    }
    
#if SMTG_OS_WINDOWS
    if (mBackgroundBrush)
        DeleteObject(mBackgroundBrush);
    
    if (mTitleFont)
        DeleteObject(mTitleFont);
    
    if (mLabelFont)
        DeleteObject(mLabelFont);
    
    if (mWndHandle)
        DestroyWindow(mWndHandle);
#endif
}

//-----------------------------------------------------------------------------
void PluginEditor::initializeGUI()
{
    // Clear any existing knobs
    mKnobs.clear();
    
    // Create knobs for all parameters with completely redesigned layout
    // Bypass buttons - positioned far right with no overlaps
    mKnobs.push_back({750, 120, kParamAmpBypassId, "Bypass", 0.0f});
    mKnobs.push_back({300, 240, kParamDistBypassId, "Bypass", 0.0f});
    mKnobs.push_back({420, 360, kParamReverbBypassId, "Bypass", 0.0f});
    mKnobs.push_back({420, 480, kParamDelayBypassId, "Bypass", 0.0f});
    mKnobs.push_back({750, 420, kParamModBypassId, "Bypass", 0.0f});
    
    // Amp section - horizontal layout with proper spacing
    mKnobs.push_back({60, 120, kParamGainId, "Gain", 0.5f});
    mKnobs.push_back({140, 120, kParamBassId, "Bass", 0.5f});
    mKnobs.push_back({220, 120, kParamMidId, "Mid", 0.5f});
    mKnobs.push_back({300, 120, kParamTrebleId, "Treble", 0.5f});
    mKnobs.push_back({380, 120, kParamPresenceId, "Presence", 0.5f});
    mKnobs.push_back({620, 120, kParamOutputLevelId, "Level", 0.7f});
    
    // Distortion section - horizontal layout
    mKnobs.push_back({60, 240, kParamDistTypeId, "Type", 0.5f});
    mKnobs.push_back({180, 240, kParamDistDriveId, "Drive", 0.5f});
    
    // Reverb section - horizontal layout
    mKnobs.push_back({60, 360, kParamReverbMixId, "Mix", 0.3f});
    mKnobs.push_back({140, 360, kParamReverbSizeId, "Size", 0.5f});
    mKnobs.push_back({220, 360, kParamReverbReverseId, "Reverse", 0.0f});
    mKnobs.push_back({300, 360, kParamReverbShimmerId, "Shimmer", 0.0f});
    
    // Delay section - horizontal layout
    mKnobs.push_back({60, 480, kParamDelayMixId, "Mix", 0.3f});
    mKnobs.push_back({140, 480, kParamDelayTimeId, "Time", 0.5f});
    mKnobs.push_back({220, 480, kParamDelayFeedbackId, "Feedback", 0.3f});
    mKnobs.push_back({300, 480, kParamDelayReverseId, "Reverse", 0.0f});
    
    // Modulation section - vertical layout on right side
    mKnobs.push_back({620, 360, kParamModTypeId, "Type", 0.0f});
    mKnobs.push_back({620, 420, kParamModRateId, "Rate", 0.5f});
    mKnobs.push_back({620, 480, kParamModDepthId, "Depth", 0.5f});
    
    // Update values from controller
    for (size_t i = 0; i < mKnobs.size(); i++)
    {
        mKnobs[i].value = getParameterValue(mKnobs[i].paramId);
    }
}

//-----------------------------------------------------------------------------
float PluginEditor::getParameterValue(int paramId)
{
    if (mController)
    {
        Parameter* param = mController->getParameterObject(paramId);
        if (param)
            return param->getNormalized();
    }
    return 0.0f;
}

//-----------------------------------------------------------------------------
void PluginEditor::updateParameter(int knobIndex, float normalizedValue)
{
    if (knobIndex >= 0 && knobIndex < (int)mKnobs.size() && mController)
    {
        // Clamp value between 0 and 1
        normalizedValue = (normalizedValue < 0.0f) ? 0.0f : ((normalizedValue > 1.0f) ? 1.0f : normalizedValue);
        
        // Update knob value
        mKnobs[knobIndex].value = normalizedValue;
        
        // Update controller
        mController->setParamNormalized(mKnobs[knobIndex].paramId, normalizedValue);
        mController->performEdit(mKnobs[knobIndex].paramId, normalizedValue);
        
#if SMTG_OS_WINDOWS
        // Redraw
        if (mWndHandle)
            InvalidateRect(mWndHandle, NULL, FALSE);
#endif
    }
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginEditor::isPlatformTypeSupported(FIDString type)
{
    // Check if the platform type is supported
    // For simplicity, we'll support the default types
#if SMTG_OS_WINDOWS
    if (strcmp(type, kPlatformTypeHWND) == 0)
        return kResultTrue;
#elif SMTG_OS_MACOS
    if (strcmp(type, kPlatformTypeNSView) == 0)
        return kResultTrue;
#elif SMTG_OS_LINUX
    if (strcmp(type, kPlatformTypeX11EmbedWindowID) == 0)
        return kResultTrue;
#endif
    
    return kResultFalse;
}

//-----------------------------------------------------------------------------
#if SMTG_OS_WINDOWS
LRESULT CALLBACK PluginEditor::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PluginEditor* editor = (PluginEditor*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    
    if (editor)
    {
        switch (message)
        {
            case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hWnd, &ps);
                editor->paint(hdc);
                EndPaint(hWnd, &ps);
                return 0;
            }
            
            case WM_LBUTTONDOWN:
            {
                int xPos = GET_X_LPARAM(lParam);
                int yPos = GET_Y_LPARAM(lParam);
                editor->onMouseDown(xPos, yPos);
                return 0;
            }
            
            case WM_MOUSEMOVE:
            {
                if (wParam & MK_LBUTTON)
                {
                    int xPos = GET_X_LPARAM(lParam);
                    int yPos = GET_Y_LPARAM(lParam);
                    editor->onMouseMove(xPos, yPos);
                }
                return 0;
            }
            
            case WM_LBUTTONUP:
            {
                int xPos = GET_X_LPARAM(lParam);
                int yPos = GET_Y_LPARAM(lParam);
                editor->onMouseUp(xPos, yPos);
                return 0;
            }
            
            case WM_ERASEBKGND:
                return 1; // Skip background erasing
        }
    }
    
    return DefWindowProc(hWnd, message, wParam, lParam);
}
#endif

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginEditor::attached(void* parent, FIDString type)
{
    // Store the platform handle
    mPlatformHandle = parent;
    
    // Create the platform-specific view
#if SMTG_OS_WINDOWS
    // Windows implementation
    HWND parentWindow = (HWND)parent;
    
    // Register window class
    static bool windowClassRegistered = false;
    static WNDCLASSEX windowClass;
    
    if (!windowClassRegistered)
    {
        windowClass.cbSize = sizeof(WNDCLASSEX);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = WndProc;
        windowClass.cbClsExtra = 0;
        windowClass.cbWndExtra = 0;
        windowClass.hInstance = GetModuleHandle(NULL);
        windowClass.hIcon = NULL;
        windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        windowClass.hbrBackground = NULL;
        windowClass.lpszMenuName = NULL;
        windowClass.lpszClassName = L"AmneziaGazePluginView";
        windowClass.hIconSm = NULL;
        
        RegisterClassEx(&windowClass);
        windowClassRegistered = true;
    }
    
    // Create window
    RECT rect;
    GetClientRect(parentWindow, &rect);
    
    mWndHandle = CreateWindowEx(
        0,
        L"AmneziaGazePluginView",
        L"AMNEZIAGAZE",
        WS_CHILD | WS_VISIBLE,
        0, 0, rect.right - rect.left, rect.bottom - rect.top,
        parentWindow,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    
    // Store this pointer with the window
    SetWindowLongPtr(mWndHandle, GWLP_USERDATA, (LONG_PTR)this);
    
    // Create resources
    mBackgroundBrush = CreateSolidBrush(COLOR_BACKGROUND);
    mTitleFont = CreateFont(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                           OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                           DEFAULT_PITCH | FF_DONTCARE, L"Arial");
    mLabelFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                           OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                           DEFAULT_PITCH | FF_DONTCARE, L"Arial");
    
    // Show window
    ShowWindow(mWndHandle, SW_SHOW);
    UpdateWindow(mWndHandle);
#elif SMTG_OS_MACOS
    // macOS implementation would go here
#elif SMTG_OS_LINUX
    // Linux implementation would go here
#endif
    
    return kResultTrue;
}

//-----------------------------------------------------------------------------
#if SMTG_OS_WINDOWS
void PluginEditor::paint(HDC hdc)
{
    // Get client rect
    RECT rect;
    GetClientRect(mWndHandle, &rect);
    
    // Create double buffer
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
    
    // Fill background
    FillRect(memDC, &rect, mBackgroundBrush);
    
    // Set text color and mode
    SetTextColor(memDC, COLOR_TEXT);
    SetBkMode(memDC, TRANSPARENT);
    
    // Draw plugin title
    HFONT oldFont = (HFONT)SelectObject(memDC, mTitleFont);
    SetTextColor(memDC, COLOR_TITLE);
    
    RECT titleRect = {0, 10, rect.right, 60};
    DrawText(memDC, L"AMNEZIAGAZE", -1, &titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    // Draw company name
    RECT companyRect = {0, 60, rect.right, 90};
    DrawText(memDC, L"Plugins Legais", -1, &companyRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    // Draw section titles
    SetTextColor(memDC, COLOR_SECTION);
    
    RECT ampRect = {20, 90, 700, 115};
    DrawText(memDC, L"Amp", -1, &ampRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    
    RECT distRect = {20, 210, 700, 235};
    DrawText(memDC, L"Distortion", -1, &distRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    
    RECT reverbRect = {20, 330, 700, 355};
    DrawText(memDC, L"Reverb", -1, &reverbRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    
    RECT delayRect = {20, 450, 700, 475};
    DrawText(memDC, L"Delay", -1, &delayRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    
    RECT modRect = {520, 330, 700, 355};
    DrawText(memDC, L"Modulation", -1, &modRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    
    // Draw knobs
    SelectObject(memDC, mLabelFont);
    SetTextColor(memDC, COLOR_TEXT);
    
    for (size_t i = 0; i < mKnobs.size(); i++)
    {
        // Check if this is a switch-style parameter (bypass, toggle, or mode selection)
        bool isSwitch = (mKnobs[i].paramId == kParamAmpBypassId ||
                        mKnobs[i].paramId == kParamDistBypassId ||
                        mKnobs[i].paramId == kParamReverbBypassId ||
                        mKnobs[i].paramId == kParamDelayBypassId ||
                        mKnobs[i].paramId == kParamModBypassId ||
                        mKnobs[i].paramId == kParamReverbReverseId ||
                        mKnobs[i].paramId == kParamDelayReverseId ||
                        mKnobs[i].paramId == kParamDistTypeId ||
                        mKnobs[i].paramId == kParamModTypeId);
        
        HBRUSH controlBrush;
        HBRUSH oldBrush;
        
        if (isSwitch) {
            // Draw switch-style button
            COLORREF buttonColor = COLOR_KNOB;
            
            // For bypass buttons, show red when active (bypassed)
            if (mKnobs[i].paramId == kParamAmpBypassId ||
                mKnobs[i].paramId == kParamDistBypassId ||
                mKnobs[i].paramId == kParamReverbBypassId ||
                mKnobs[i].paramId == kParamDelayBypassId ||
                mKnobs[i].paramId == kParamModBypassId) {
                if (mKnobs[i].value > 0.5f) {
                    buttonColor = RGB(200, 50, 50); // Red when bypassed
                }
            }
            // For toggle buttons, show green when active
            else if (mKnobs[i].paramId == kParamReverbReverseId ||
                     mKnobs[i].paramId == kParamDelayReverseId) {
                if (mKnobs[i].value > 0.5f) {
                    buttonColor = RGB(50, 200, 50); // Green when active
                }
            }
            // For mode selection, highlight current selection
            else if (mKnobs[i].paramId == kParamDistTypeId ||
                     mKnobs[i].paramId == kParamModTypeId) {
                if (i == mDraggingKnob) {
                    buttonColor = COLOR_KNOB_HIGHLIGHT;
                }
            }
            
            controlBrush = CreateSolidBrush(buttonColor);
            oldBrush = (HBRUSH)SelectObject(memDC, controlBrush);
            
            // Draw much larger rectangular button for better clickability
            int buttonWidth = KNOB_RADIUS * 4;  // Much larger width (100px)
            int buttonHeight = KNOB_RADIUS * 2; // Much larger height (50px)
            Rectangle(memDC,
                     mKnobs[i].x - buttonWidth/2,
                     mKnobs[i].y - buttonHeight/2,
                     mKnobs[i].x + buttonWidth/2,
                     mKnobs[i].y + buttonHeight/2);
            
        } else {
            // Draw traditional knob
            controlBrush = CreateSolidBrush(i == mDraggingKnob ? COLOR_KNOB_HIGHLIGHT : COLOR_KNOB);
            oldBrush = (HBRUSH)SelectObject(memDC, controlBrush);
            
            Ellipse(memDC,
                    mKnobs[i].x - KNOB_RADIUS,
                    mKnobs[i].y - KNOB_RADIUS,
                    mKnobs[i].x + KNOB_RADIUS,
                    mKnobs[i].y + KNOB_RADIUS);
            
            // Draw indicator line
            float angle = (mKnobs[i].value * 270.0f - 135.0f) * 3.14159f / 180.0f;
            int lineX = mKnobs[i].x + (int)(cos(angle) * KNOB_RADIUS * 0.8f);
            int lineY = mKnobs[i].y + (int)(sin(angle) * KNOB_RADIUS * 0.8f);
            
            HPEN indicatorPen = CreatePen(PS_SOLID, 2, COLOR_TEXT);
            HPEN oldPen = (HPEN)SelectObject(memDC, indicatorPen);
            
            MoveToEx(memDC, mKnobs[i].x, mKnobs[i].y, NULL);
            LineTo(memDC, lineX, lineY);
            
            SelectObject(memDC, oldPen);
            DeleteObject(indicatorPen);
        }
        
        // Clean up brush
        SelectObject(memDC, oldBrush);
        DeleteObject(controlBrush);
        
        // Draw label
        RECT labelRect = {
            mKnobs[i].x - KNOB_RADIUS * 2,
            mKnobs[i].y + KNOB_LABEL_OFFSET,
            mKnobs[i].x + KNOB_RADIUS * 2,
            mKnobs[i].y + KNOB_LABEL_OFFSET + 20
        };
        
        std::wstring wideLabel(mKnobs[i].name.begin(), mKnobs[i].name.end());
        DrawText(memDC, wideLabel.c_str(), -1, &labelRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        // Draw value
        RECT valueRect = {
            mKnobs[i].x - KNOB_RADIUS * 2,
            mKnobs[i].y + KNOB_VALUE_OFFSET,
            mKnobs[i].x + KNOB_RADIUS * 2,
            mKnobs[i].y + KNOB_VALUE_OFFSET + 20
        };
        
        // Format value based on parameter type
        std::wstring valueText;
        
        // Special handling for enum parameters
        if (mKnobs[i].paramId == kParamDistTypeId)
        {
            int distType = (int)(mKnobs[i].value * 2.0f + 0.5f);
            switch (distType)
            {
                case 0: valueText = L"Clean"; break;
                case 1: valueText = L"Crunch"; break;
                case 2: valueText = L"Fuzz"; break;
                default: valueText = L"Unknown";
            }
        }
        else if (mKnobs[i].paramId == kParamModTypeId)
        {
            int modType = (int)(mKnobs[i].value * 2.0f + 0.5f);
            switch (modType)
            {
                case 0: valueText = L"Chorus"; break;
                case 1: valueText = L"Flanger"; break;
                case 2: valueText = L"Phaser"; break;
                default: valueText = L"Unknown";
            }
        }
        else if (mKnobs[i].paramId == kParamReverbReverseId || mKnobs[i].paramId == kParamDelayReverseId)
        {
            valueText = mKnobs[i].value > 0.5f ? L"On" : L"Off";
        }
        else if (mKnobs[i].paramId == kParamAmpBypassId ||
                 mKnobs[i].paramId == kParamDistBypassId ||
                 mKnobs[i].paramId == kParamReverbBypassId ||
                 mKnobs[i].paramId == kParamDelayBypassId ||
                 mKnobs[i].paramId == kParamModBypassId)
        {
            valueText = mKnobs[i].value > 0.5f ? L"Bypassed" : L"Active";
        }
        else
        {
            // Format as percentage
            wchar_t buffer[32];
            swprintf(buffer, 32, L"%.0f%%", mKnobs[i].value * 100.0f);
            valueText = buffer;
        }
        
        DrawText(memDC, valueText.c_str(), -1, &valueRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    
    // Copy to screen
    BitBlt(hdc, 0, 0, rect.right, rect.bottom, memDC, 0, 0, SRCCOPY);
    
    // Clean up
    SelectObject(memDC, oldFont);
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}

//-----------------------------------------------------------------------------
void PluginEditor::onMouseDown(int x, int y)
{
    // Check if a knob or button was clicked
    for (size_t i = 0; i < mKnobs.size(); i++)
    {
        // Check if this is a switch-style parameter (larger hitbox)
        bool isSwitch = (mKnobs[i].paramId == kParamAmpBypassId ||
                        mKnobs[i].paramId == kParamDistBypassId ||
                        mKnobs[i].paramId == kParamReverbBypassId ||
                        mKnobs[i].paramId == kParamDelayBypassId ||
                        mKnobs[i].paramId == kParamModBypassId ||
                        mKnobs[i].paramId == kParamReverbReverseId ||
                        mKnobs[i].paramId == kParamDelayReverseId ||
                        mKnobs[i].paramId == kParamDistTypeId ||
                        mKnobs[i].paramId == kParamModTypeId);
        
        bool clicked = false;
        
        if (isSwitch) {
            // Much larger rectangular hitbox for buttons - same as drawing
            int buttonWidth = KNOB_RADIUS * 4;  // 100px wide
            int buttonHeight = KNOB_RADIUS * 2; // 50px tall
            int left = mKnobs[i].x - buttonWidth/2;
            int right = mKnobs[i].x + buttonWidth/2;
            int top = mKnobs[i].y - buttonHeight/2;
            int bottom = mKnobs[i].y + buttonHeight/2;
            
            clicked = (x >= left && x <= right && y >= top && y <= bottom);
        } else {
            // Circular hitbox for knobs
            int dx = x - mKnobs[i].x;
            int dy = y - mKnobs[i].y;
            int distSquared = dx * dx + dy * dy;
            clicked = (distSquared <= KNOB_RADIUS * KNOB_RADIUS);
        }
        
        if (clicked)
        {
            mDraggingKnob = (int)i;
            
            // For switch-style buttons (bypass, toggle, mode), handle immediate toggle
            if (isSwitch) {
                // For bypass buttons, toggle between 0 and 1
                if (mKnobs[i].paramId == kParamAmpBypassId ||
                    mKnobs[i].paramId == kParamDistBypassId ||
                    mKnobs[i].paramId == kParamReverbBypassId ||
                    mKnobs[i].paramId == kParamDelayBypassId ||
                    mKnobs[i].paramId == kParamModBypassId ||
                    mKnobs[i].paramId == kParamReverbReverseId ||
                    mKnobs[i].paramId == kParamDelayReverseId) {
                    
                    float newValue = (mKnobs[i].value > 0.5f) ? 0.0f : 1.0f;
                    updateParameter(i, newValue);
                }
                // For mode selection buttons, cycle through values
                else if (mKnobs[i].paramId == kParamDistTypeId) {
                    int currentMode = (int)(mKnobs[i].value * 2.0f + 0.5f);
                    int nextMode = (currentMode + 1) % 3; // 3 distortion types
                    float newValue = (float)nextMode / 2.0f;
                    updateParameter(i, newValue);
                }
                else if (mKnobs[i].paramId == kParamModTypeId) {
                    int currentMode = (int)(mKnobs[i].value * 2.0f + 0.5f);
                    int nextMode = (currentMode + 1) % 3; // 3 modulation types
                    float newValue = (float)nextMode / 2.0f;
                    updateParameter(i, newValue);
                }
            }
            
            // Capture mouse
            SetCapture(mWndHandle);
            
            // Redraw
            InvalidateRect(mWndHandle, NULL, FALSE);
            break;
        }
    }
}

//-----------------------------------------------------------------------------
void PluginEditor::onMouseMove(int x, int y)
{
    if (mDraggingKnob >= 0 && mDraggingKnob < (int)mKnobs.size())
    {
        // Check if this is a switch-style parameter (should not respond to dragging)
        bool isSwitch = (mKnobs[mDraggingKnob].paramId == kParamAmpBypassId ||
                        mKnobs[mDraggingKnob].paramId == kParamDistBypassId ||
                        mKnobs[mDraggingKnob].paramId == kParamReverbBypassId ||
                        mKnobs[mDraggingKnob].paramId == kParamDelayBypassId ||
                        mKnobs[mDraggingKnob].paramId == kParamModBypassId ||
                        mKnobs[mDraggingKnob].paramId == kParamReverbReverseId ||
                        mKnobs[mDraggingKnob].paramId == kParamDelayReverseId ||
                        mKnobs[mDraggingKnob].paramId == kParamDistTypeId ||
                        mKnobs[mDraggingKnob].paramId == kParamModTypeId);
        
        // Only allow dragging for continuous parameters (knobs), not switches
        if (!isSwitch)
        {
            // Calculate new value based on vertical movement
            static int lastY = y;
            int deltaY = lastY - y;
            lastY = y;
            
            float newValue = mKnobs[mDraggingKnob].value + deltaY * 0.01f;
            updateParameter(mDraggingKnob, newValue);
        }
    }
}

//-----------------------------------------------------------------------------
void PluginEditor::onMouseUp(int x, int y)
{
    if (mDraggingKnob >= 0)
    {
        // Release mouse capture
        ReleaseCapture();
        
        mDraggingKnob = -1;
        
        // Redraw
        InvalidateRect(mWndHandle, NULL, FALSE);
    }
}
#endif

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginEditor::removed()
{
    // Clean up the platform-specific view
#if SMTG_OS_WINDOWS
    if (mWndHandle)
    {
        DestroyWindow(mWndHandle);
        mWndHandle = nullptr;
    }
#endif
    
    mPlatformHandle = nullptr;
    
    return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginEditor::onWheel(float distance)
{
    // Handle mouse wheel events
    return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginEditor::onKeyDown(char16 key, int16 keyCode, int16 modifiers)
{
    // Handle key down events
    return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginEditor::onKeyUp(char16 key, int16 keyCode, int16 modifiers)
{
    // Handle key up events
    return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginEditor::getSize(ViewRect* size)
{
    // Return the current size
    if (size)
    {
        *size = mSize;
        return kResultTrue;
    }
    
    return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginEditor::onSize(ViewRect* newSize)
{
    // Handle size changes
    if (newSize)
    {
        mSize = *newSize;
        
#if SMTG_OS_WINDOWS
        if (mWndHandle)
        {
            SetWindowPos(mWndHandle, HWND_TOP, 0, 0,
                         mSize.right - mSize.left,
                         mSize.bottom - mSize.top,
                         SWP_NOMOVE);
        }
#endif
        
        return kResultTrue;
    }
    
    return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginEditor::onFocus(TBool state)
{
    // Handle focus changes
    return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginEditor::setFrame(IPlugFrame* frame)
{
    // Store the frame reference
    if (mPlugFrame)
        mPlugFrame->release();
    
    mPlugFrame = frame;
    
    if (mPlugFrame)
        mPlugFrame->addRef();
    
    return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginEditor::canResize()
{
    // Allow resizing
    return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PluginEditor::checkSizeConstraint(ViewRect* rect)
{
    // Check if the size is valid
    if (rect)
    {
        // Enforce minimum size
        if (rect->right - rect->left < 400)
            rect->right = rect->left + 400;
        
        if (rect->bottom - rect->top < 300)
            rect->bottom = rect->top + 300;
        
        return kResultTrue;
    }
    
    return kResultFalse;
}