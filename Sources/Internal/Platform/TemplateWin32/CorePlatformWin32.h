#ifndef __DAVAENGINE_CORE_PLATFORM_WIN32_H__
#define __DAVAENGINE_CORE_PLATFORM_WIN32_H__

#if !defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_WIN32__)

#include "Base/Platform.h"
#include "Core/Core.h"
#include "UI/UIEvent.h"
#include "Input/InputSystem.h"

namespace DAVA
{
class CoreWin32Platform : public Core
{
public:
    CoreWin32Platform();
    eScreenMode GetScreenMode() override;
    bool SetScreenMode(eScreenMode screenMode) override;
    void GetAvailableDisplayModes(List<DisplayMode>& availableModes) override;

    bool CreateWin32Window(HINSTANCE hInstance); //true if window created, if false, need to quit the app
    void Run();

    void SetIcon(int32 iconId) override;

    void SetWindowMinimumSize(float32 width, float32 height) override;
    Vector2 GetWindowMinimumSize() const override;

    void InitArgs();
    void Quit() override;

    void* GetNativeWindow() const override;

private:
    DisplayMode currentMode;
    DisplayMode fullscreenMode;
    DisplayMode windowedMode;
    bool isFullscreen;

    static const uint32 WINDOWED_STYLE = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX;
    static const uint32 FULLSCREEN_STYLE = WS_POPUP;

    void OnMouseMove(int32 x, int32 y);
    void OnMouseWheel(int32 wheelDeltaX, int32 wheelDeltaY, int32 x, int32 y);
    void OnMouseClick(UIEvent::Phase phase, eMouseButtons button, int32 x, int32 y);
    void OnTouchEvent(UIEvent::Phase phase, eInputDevices deviceId, uint32 fingerId, float32 x, float32 y, float presure);
    void OnGetMinMaxInfo(MINMAXINFO* minmaxInfo);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    bool ProcessMouseClickEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    bool ProcessMouseMoveEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    bool ProcessMouseWheelEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    bool ProcessMouseInputEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    RECT GetWindowedRectForDisplayMode(DisplayMode& dm);
    void LoadWindowMinimumSizeSettings();

    void ClearMouseButtons();

    bool willQuit;

    std::bitset<static_cast<size_t>(eMouseButtons::COUNT)> mouseButtonState;
    Vector<TOUCHINPUT> inputTouchBuffer;

    float32 minWindowWidth = 0.0f;
    float32 minWindowHeight = 0.0f;

    HWND hWindow = nullptr;
};

} // end namespace DAVA
#endif // #if defined(__DAVAENGINE_WIN32__)
#endif // !__DAVAENGINE_COREV2__
#endif // __DAVAENGINE_CORE_PLATFORM_WIN32_H__
