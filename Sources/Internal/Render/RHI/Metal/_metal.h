#if !defined __METAL_H__
#define __METAL_H__

#include "Platform/TemplateiOS/DVMetalLayer.h"

extern id<MTLDevice>                _Metal_Device;
extern id<MTLCommandQueue>          _Metal_DefCmdQueue;
extern id<MTLDepthStencilState>     _Metal_DefDepthState;
extern DVMetalLayer*                _Metal_Layer;


#endif // __METAL_H__
