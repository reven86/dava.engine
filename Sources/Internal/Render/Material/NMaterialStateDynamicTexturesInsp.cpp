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
#include "Render/Material/NMaterial.h"
#include "Render/Material/NMaterialStateDynamicTexturesInsp.h"
#include "Render/Material/FXCache.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"

namespace DAVA
{
///////////////////////////////////////////////////////////////////////////
///// NMaterialStateDynamicTexturesInsp implementation

NMaterialStateDynamicTexturesInsp::NMaterialStateDynamicTexturesInsp()
{
    defaultTexture = Texture::CreatePink();
}

NMaterialStateDynamicTexturesInsp::~NMaterialStateDynamicTexturesInsp()
{
    SafeRelease(defaultTexture);
}

void NMaterialStateDynamicTexturesInsp::FindMaterialTexturesRecursive(NMaterial *material, Set<FastName>& ret) const
{
	auto fxName = material->GetEffectiveFXName();
    if (fxName.IsValid())
    {
        HashMap<FastName, int32> flags;
        material->CollectMaterialFlags(flags);

        // shader data
        FXDescriptor fxDescriptor = FXCache::GetFXDescriptor(fxName, flags, QualitySettingsSystem::Instance()->GetCurMaterialQuality(material->qualityGroup));
        for (auto& descriptor : fxDescriptor.renderPassDescriptors)
        {
            if (!descriptor.shader->IsValid())
                continue;

            const rhi::ShaderSamplerList& fragmentSamplers = descriptor.shader->GetFragmentSamplerList();
            for (const auto& samp : fragmentSamplers)
            {
                ret.insert(samp.uid);
            }

            const rhi::ShaderSamplerList& vertexSamplers = descriptor.shader->GetVertexSamplerList();
            for (const auto& samp : vertexSamplers)
            {
                ret.insert(samp.uid);
            }
        }
    }
    else
    {
        // if fxName is not valid (e.g global material)
        // we just add all local textures
        for (const auto& t : material->localTextures)
            ret.insert(t.first);
    }

    if (nullptr != material->GetParent())
		FindMaterialTexturesRecursive(material->GetParent(), ret);
}

InspInfoDynamic::DynamicData NMaterialStateDynamicTexturesInsp::Prepare(void *object, int filter) const
{
    NMaterial *material = (NMaterial*)object;
    DVASSERT(material);

	Set<FastName>* data = new Set<FastName>();
    FindMaterialTexturesRecursive(material, *data);

    if (filter > 0)
    {
        auto checkAndAdd = [&data](const FastName &name) 
		{
            if (0 == data->count(name))
            {
                data->insert(name);
            }
        };

        checkAndAdd(NMaterialTextureName::TEXTURE_ALBEDO);
        checkAndAdd(NMaterialTextureName::TEXTURE_NORMAL);
        checkAndAdd(NMaterialTextureName::TEXTURE_DETAIL);
        checkAndAdd(NMaterialTextureName::TEXTURE_LIGHTMAP);
        checkAndAdd(NMaterialTextureName::TEXTURE_DECAL);
        checkAndAdd(NMaterialTextureName::TEXTURE_CUBEMAP);
        checkAndAdd(NMaterialTextureName::TEXTURE_HEIGHTMAP);
        checkAndAdd(NMaterialTextureName::TEXTURE_DECALMASK);
        checkAndAdd(NMaterialTextureName::TEXTURE_DECALTEXTURE);
    }

    DynamicData ddata;
    ddata.object = object;
    ddata.data = std::shared_ptr<void>(data);
    return ddata;
}

Vector<FastName> NMaterialStateDynamicTexturesInsp::MembersList(const DynamicData& ddata) const
{
    Vector<FastName> ret;
    
    Set<FastName>* textures = (Set<FastName>*) ddata.data.get();
    DVASSERT(textures);

    auto it = textures->begin();
    auto end = textures->end();
    
    ret.reserve(textures->size());
    while(it != end)
    {
        ret.push_back(*it);
        ++it;
    }
    
    return ret;
}

InspDesc NMaterialStateDynamicTexturesInsp::MemberDesc(const DynamicData& ddata, const FastName &textureName) const
{
    return InspDesc(textureName.c_str());
}

VariantType NMaterialStateDynamicTexturesInsp::MemberValueGet(const DynamicData& ddata, const FastName &textureName) const
{
    VariantType ret;
    
    Set<FastName>* textures = (Set<FastName>*) ddata.data.get();;
    DVASSERT(textures);

    NMaterial *material = (NMaterial*) ddata.object;
    DVASSERT(material);

    if (textures->count(textureName))
    {
        Texture *tex = material->GetEffectiveTexture(textureName);
        if (nullptr != tex)
        {
            ret.SetFilePath(tex->GetPathname());
        }
        else
        {
            ret.SetFilePath(FilePath());
        }
    }
    
    return ret;
}

void NMaterialStateDynamicTexturesInsp::MemberValueSet(const DynamicData& ddata, const FastName &textureName, const VariantType &value)
{
    VariantType ret;

    Set<FastName>* textures = (Set<FastName>*) ddata.data.get();;
    DVASSERT(textures);

    NMaterial *material = (NMaterial*)ddata.object;
    DVASSERT(material);

    if (textures->count(textureName))
    {
        if(value.type == VariantType::TYPE_NONE)
        {
            if (material->HasLocalTexture(textureName))
            {
                material->RemoveTexture(textureName);
            }
        }
        else
        {
            FilePath texPath = value.AsFilePath();
            Texture *texture = nullptr;
            
            if (texPath == FilePath())
            {
                texture = SafeRetain(defaultTexture);
            }
            else
            {
                texture = Texture::CreateFromFile(texPath);
            }

            if (material->HasLocalTexture(textureName))
            {
                material->SetTexture(textureName, texture);
            }
            else
            {
                material->AddTexture(textureName, texture);
            }

            SafeRelease(texture);
        }
    }
}

int NMaterialStateDynamicTexturesInsp::MemberFlags(const DynamicData& ddata, const FastName &textureName) const
{
    int flags = 0;
    
    NMaterial *material = (NMaterial*)ddata.object;
    DVASSERT(material);

    flags |= I_VIEW;

    if (material->HasLocalTexture(textureName))
    {
        flags |= I_EDIT;
    }
    
    return flags;
}

};
