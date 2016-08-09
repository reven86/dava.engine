#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/iOS/WindowNativeServiceiOS.h"

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/iOS/Window/WindowBackendiOS.h"
#include "Engine/Private/iOS/Window/WindowNativeBridgeiOS.h"

#include "Render/Image/Image.h"

#import <UIKit/UIKit.h>

namespace DAVA
{
WindowNativeService::WindowNativeService(Private::WindowNativeBridge* nativeBridge)
    : bridge(nativeBridge)
{
}

void WindowNativeService::InitRenderParams(rhi::InitParam& params)
{
}

void WindowNativeService::AddUIView(UIView* uiview)
{
    bridge->AddUIView(uiview);
}

void WindowNativeService::RemoveUIView(UIView* uiview)
{
    bridge->RemoveUIView(uiview);
}

UIView* WindowNativeService::GetUIViewFromPool(const char8* className)
{
    return bridge->GetUIViewFromPool(className);
}

void WindowNativeService::ReturnUIViewToPool(UIView* view)
{
    bridge->ReturnUIViewToPool(view);
}

UIImage* WindowNativeService::RenderUIViewToUIImage(UIView* view)
{
    DVASSERT(view != nullptr);

    UIImage* image = nullptr;
    CGSize size = view.frame.size;
    if (size.width > 0 && size.height > 0)
    {
        UIGraphicsBeginImageContextWithOptions(size, NO, 0);
        // Workaround! iOS bug see http://stackoverflow.com/questions/23157653/drawviewhierarchyinrectafterscreenupdates-delays-other-animations
        [view.layer renderInContext:UIGraphicsGetCurrentContext()];

        image = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
    }
    return image;
}

Image* WindowNativeService::ConvertUIImageToImage(UIImage* nativeImage)
{
    DVASSERT(nativeImage != nullptr);

    Image* image = nullptr;
    CGImageRef imageRef = [nativeImage CGImage];
    if (imageRef != nullptr)
    {
        uint32 width = static_cast<uint32>(CGImageGetWidth(imageRef));
        uint32 height = static_cast<uint32>(CGImageGetHeight(imageRef));

        image = Image::Create(width, height, DAVA::FORMAT_RGBA8888);
        if (image != nullptr)
        {
            uint8* rawData = image->GetData();

            const uint32 bytesPerPixel = 4;
            const uint32 bitsPerComponent = 8;
            const uint32 bytesPerRow = bytesPerPixel * width;

            //Memset(rawData, 0, width * height * bytesPerPixel);

            CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
            CGContextRef context = CGBitmapContextCreate(rawData,
                                                         width,
                                                         height,
                                                         bitsPerComponent,
                                                         bytesPerRow,
                                                         colorSpace,
                                                         kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);

            CGContextDrawImage(context, CGRectMake(0, 0, width, height), imageRef);
            CGContextRelease(context);
            CGColorSpaceRelease(colorSpace);
        }
        //CGImageRelease(imageRef);
    }
    return image;
}

} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
