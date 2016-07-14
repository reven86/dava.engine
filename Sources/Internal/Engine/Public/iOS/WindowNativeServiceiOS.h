#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/EnginePrivateFwd.h"

@class UIView;
@class UIImage;

namespace DAVA
{
class Image;
class WindowNativeService final
{
public:
    UIView* CreateNativeControl(const char8* className);
    void ReleaseNativeControl(UIView* view);

    static UIImage* RenderUIViewToUIImage(UIView* view);
    static Image* ConvertUIImageToImage(UIImage* nativeImage);

private:
    WindowNativeService(Private::WindowNativeBridge* nativeBridge);

private:
    Private::WindowNativeBridge* bridge = nullptr;

    // Friends
    friend class Private::WindowBackend;
};

} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
