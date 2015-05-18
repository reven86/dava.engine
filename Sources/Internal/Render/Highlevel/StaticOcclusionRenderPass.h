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
#ifndef __DAVAENGINE_STATIC_OCCLUSION_RENDER_PASS__
#define __DAVAENGINE_STATIC_OCCLUSION_RENDER_PASS__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/BaseMath.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/RenderBase.h"
#include "Render/Texture.h"

namespace DAVA
{

class StaticOcclusion;
class StaticOcclusionRenderPass : public RenderPass
{
public:
    StaticOcclusionRenderPass(const FastName & name, StaticOcclusion * occlusion);
    ~StaticOcclusionRenderPass();

    inline void SetIndex(int32 side, int32 i, int32 j, int32 k);
    inline void SetOcclusionCamera(Camera * camera);
    void Draw(RenderSystem * renderSystem);
    static bool CompareFunction(const RenderBatch * a, const RenderBatch *  b);
    
private:
    int32 debugSide;
    int32 debugI;
    int32 debugJ;
    int32 debugK;
    
    Camera * occlusionCamera;
    StaticOcclusion * occlusion;
    Set<RenderObject*> visibleObjectSet;
};
    
inline void StaticOcclusionRenderPass::SetOcclusionCamera(Camera * _camera)
{
    occlusionCamera = _camera;
}

inline void StaticOcclusionRenderPass::SetIndex(int32 side, int32 i, int32 j, int32 k)
{
    debugSide = side;
    debugI = i;
    debugJ = j;
    debugK = k;
}


};

#endif //__DAVAENGINE_STATIC_OCCLUSION_RENDER_PASS__
