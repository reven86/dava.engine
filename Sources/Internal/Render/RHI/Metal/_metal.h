#if !defined __METAL_H__
#define __METAL_H__

#if !(TARGET_IPHONE_SIMULATOR == 1)
#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>

extern id<MTLDevice> _Metal_Device;
extern id<MTLCommandQueue> _Metal_DefCmdQueue;
extern id<MTLTexture> _Metal_DefFrameBuf;
extern id<MTLTexture> _Metal_DefDepthBuf;
extern id<MTLTexture> _Metal_DefStencilBuf;
extern id<MTLDepthStencilState> _Metal_DefDepthState;
extern CAMetalLayer* _Metal_Layer;
extern bool _Metal_Suspended;
extern DAVA::Mutex _Metal_SuspendedSync;

extern rhi::ScreenShotCallback _Metal_PendingScreenshotCallback;
extern void* _Metal_ScreenshotData;
extern DAVA::Mutex _Metal_ScreenshotCallbackSync;

#endif

#endif // __METAL_H__
