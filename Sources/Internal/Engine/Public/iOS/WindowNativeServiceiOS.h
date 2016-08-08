#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/EnginePrivateFwd.h"

@class UIView;
@class UIImage;

namespace rhi
{
struct InitParam;
}

namespace DAVA
{
class Image;
class WindowNativeService final
{
public:
    void InitRenderParams(rhi::InitParam& params);

    void AddUIView(UIView* uiview);
    void RemoveUIView(UIView* uiview);

    UIView* GetUIViewFromPool(const char8* className);
    void ReturnUIViewToPool(UIView* view);

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
