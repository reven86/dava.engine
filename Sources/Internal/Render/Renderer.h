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
#include "DynamicBindings.h"
#include "RuntimeTextures.h"
#include "RHI/rhi_Public.h"
#include "RHI/rhi_Type.h"

namespace DAVA
{

struct ScreenShotCallbackDelegate;

namespace Renderer
{
    //init
    void Initialize(rhi::Api api, const rhi::InitParam & params);
    void Uninitialize();

    void Reset(const rhi::ResetParam & params);

    rhi::Api GetAPI();

    bool IsDeviceLost();

    void SetDesiredFPS(int32 fps);
    int32 GetDesiredFPS();

    //frame management
    void BeginFrame();
    void EndFrame();

    //misc
    int32 GetFramebufferWidth();
    int32 GetFramebufferHeight();
    void RequestGLScreenShot(ScreenShotCallbackDelegate *screenShotCallback);

    //options
    RenderOptions *GetOptions();    

    //dynamic params
    DynamicBindings& GetDynamicBindings();

    //runtime textures
    RuntimeTextures& GetRuntimeTextures();
}


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

}

#endif