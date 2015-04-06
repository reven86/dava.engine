/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __DAVAENGINE_RENDERER_H__
#define __DAVAENGINE_RENDERER_H__

#include "Core/Core.h"
#include "RenderBase.h"
#include "RenderOptions.h"
#include "RHI/rhi_Public.h"
#include "RHI/rhi_Type.h"

namespace DAVA
{
class Image;
struct ScreenShotCallbackDelegate
{
    void operator()(Image *image)
    {
        return OnScreenShot(image);
    }

protected:
    virtual void OnScreenShot(Image *image) = 0;

};

struct RenderCaps
{
    RenderCaps()
    {
        isHardwareCursorSupported = false;

        isPVRTC2Supported = false;
        isOpenGLES3Supported = false;

        isFramebufferFetchSupported = isPVRTCSupported = isETCSupported = isDXTSupported = isATCSupported = false;
        isVertexTextureUnitsSupported = isBGRA8888Supported = isFloat16Supported = isFloat32Supported = false;

#if defined(__DAVAENGINE_ANDROID__)
        isGlDepth24Stencil8Supported = isGlDepthNvNonLinearSupported = false;
#endif
    }    
    bool isHardwareCursorSupported;
    bool isPVRTCSupported;
    bool isPVRTC2Supported;
    bool isETCSupported;
    bool isOpenGLES3Supported;
    bool isBGRA8888Supported;
    bool isFloat16Supported;
    bool isFloat32Supported;
    bool isDXTSupported;
    bool isATCSupported;
    bool isVertexTextureUnitsSupported;
    bool isFramebufferFetchSupported;

#if defined(__DAVAENGINE_ANDROID__)
    bool isGlDepth24Stencil8Supported;
    bool isGlDepthNvNonLinearSupported;
#endif

};

namespace Renderer
{

    //init
    void Initialize(Core::eRenderer renderer);
    void Uninitialize();

    bool IsDeviceLost();
    void SetDesiredFPS(int32 fps);
    int32 GetDesiredFPS();

    //frame management
    void BeginFrame();
    void EndFrame();

    //caps
    const RenderCaps & GetCaps();

    //misc
    void RequestGLScreenShot(ScreenShotCallbackDelegate *screenShotCallback);

    //options
    RenderOptions *GetOptions();    

    //dynamic params
    //color will just use internal storage for dynamic PARAM_COLOR and bind it to dynamic params - it's here only to simplify legacy code moving to new renderer
    const Color& GetColor();
    void SetColor(const Color& color);
    void SetColor(float32 r, float32 g, float32 b, float32 a); 

    void SetDynamicParam(eShaderSemantic shaderSemantic, const void * value, pointer_size updateSemantic);
    const void * GetDynamicParam(eShaderSemantic shaderSemantic);
    pointer_size GetDynamicParamUpdateSemantic(eShaderSemantic shaderSemantic);        
}


}

#endif