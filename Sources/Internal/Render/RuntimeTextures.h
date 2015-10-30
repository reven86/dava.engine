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

#ifndef __DAVAENGINE_RUNTIME_TEXTURES_H__
#define __DAVAENGINE_RUNTIME_TEXTURES_H__

#include "Math/Color.h"
#include "Math/Matrix4.h"
#include "Math/Vector.h"
#include "Base/FastName.h"
#include "Render/RHI/rhi_Public.h"
#include "Render/Texture.h"

namespace DAVA
{
class RuntimeTextures
{
public:
    const static int32 REFLECTION_TEX_SIZE = 512;
    const static int32 REFRACTION_TEX_SIZE = 512;

    enum eDynamicTextureSemantic
    {
        TEXTURE_STATIC = 0,
        TEXTURE_DYNAMIC_REFLECTION,
        TEXTURE_DYNAMIC_REFRACTION,
        TEXTURE_DYNAMIC_RR_DEPTHBUFFER, //depth buffer for reflection and refraction
        //later add here shadow maps, environment probes etc.

        DYNAMIC_TEXTURES_END,
        DYNAMIC_TEXTURES_COUNT = DYNAMIC_TEXTURES_END,
    };

    RuntimeTextures()
    {
        pinkTexture[0] = pinkTexture[1] = nullptr;
    }

public:
    static RuntimeTextures::eDynamicTextureSemantic GetDynamicTextureSemanticByName(const FastName& name);

    rhi::HTexture GetDynamicTexture(eDynamicTextureSemantic semantic);
    rhi::SamplerState::Descriptor::Sampler GetDynamicTextureSamplerState(eDynamicTextureSemantic semantic);

    rhi::HTexture GetPinkTexture(rhi::TextureType type);
    rhi::SamplerState::Descriptor::Sampler GetPinkTextureSamplerState(rhi::TextureType type);

    void ClearRuntimeTextures();

private:
    void InitDynamicTexture(eDynamicTextureSemantic semantic);

    rhi::HTexture dynamicTextures[DYNAMIC_TEXTURES_COUNT];
    Texture* pinkTexture[2]; //TEXTURE_2D & TEXTURE_CUBE
};
}
#endif // __DAVAENGINE_RUNTIME_TEXTURES_H__
