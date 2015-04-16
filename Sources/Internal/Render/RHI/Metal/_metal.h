#if !defined __METAL_H__
#define __METAL_H__

    #include <Metal/Metal.h>
//    #include "../iOSSpecific/ios_gl.h"

extern id<MTLDevice>                _Metal_Device;
extern id<MTLCommandQueue>          _Metal_DefCmdQueue;
extern id<MTLRenderCommandEncoder>  _Metal_RenderCmdEncoder;
extern MTLRenderPassDescriptor*     _Metal_DefRenderPassDescriptor;
extern id<MTLTexture>               _Metal_DefFrameBuf;
extern id<MTLTexture>               _Metal_DefDepthBuf;
extern id<MTLDepthStencilState>     _Metal_DefDepthState;


#endif // __METAL_H__
