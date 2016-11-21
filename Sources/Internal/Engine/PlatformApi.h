#pragma once

#include "Base/BaseTypes.h"

/**
    \ingroup engine
    Suite of functions to access some platform specific facilities.
*/

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_QT__)

class QApplication;

namespace DAVA
{
class RenderWidget;
class Window;
namespace PlatformApi
{
void AcqureWindowContext(Window* targetWindow);
void ReleaseWindowContext(Window* targetWindow);

QApplication* GetApplication();
RenderWidget* GetRenderWidget();

} // namespace PlatformApi
} // namespace DAVA

#elif defined(__DAVAENGINE_WIN_UAP__)

namespace DAVA
{
class Window;
namespace PlatformApi
{
void AddXamlControl(Window* targetWindow, ::Windows::UI::Xaml::UIElement ^ xamlControl);
void RemoveXamlControl(Window* targetWindow, ::Windows::UI::Xaml::UIElement ^ xamlControl);
void PositionXamlControl(Window* targetWindow, ::Windows::UI::Xaml::UIElement ^ xamlControl, float32 x, float32 y);
void UnfocusXamlControl(Window* targetWindow, ::Windows::UI::Xaml::UIElement ^ xamlControl);

} // namespace PlatformApi
} // namespace DAVA

#elif defined(__DAVAENGINE_MACOS__)

DAVA_FORWARD_DECLARE_OBJC_CLASS(NSView);

namespace DAVA
{
class Window;
namespace PlatformApi
{
void AddNSView(Window* targetWindow, NSView* nsview);
void RemoveNSView(Window* targetWindow, NSView* nsview);

} // namespace PlatformApi
} // namespace DAVA

#elif defined(__DAVAENGINE_IPHONE__)

DAVA_FORWARD_DECLARE_OBJC_CLASS(UIView);
DAVA_FORWARD_DECLARE_OBJC_CLASS(UIImage);

namespace DAVA
{
class Image;
class Window;
namespace PlatformApi
{
void AddUIView(Window* targetWindow, UIView* uiview);
void RemoveUIView(Window* targetWindow, UIView* uiview);

UIView* GetUIViewFromPool(Window* targetWindow, const char8* className);
void ReturnUIViewToPool(Window* targetWindow, UIView* view);

UIImage* RenderUIViewToUIImage(UIView* view);
Image* ConvertUIImageToImage(UIImage* nativeImage);

} // namespace PlatformApi
} // namespace DAVA

#elif defined(__DAVAENGINE_ANDROID__)

#include "Engine/Android/JNIBridge.h"

namespace DAVA
{
class Window;
namespace PlatformApi
{
jobject CreateNativeControl(Window* targetWindow, const char8* controlClassName, void* backendPointer);

} // namespace PlatformApi
} //

#endif
#endif // defined(__DAVAENGINE_COREV2__)
