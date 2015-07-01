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

namespace DAVA
{
///////////////////////////////////////////////////////////////////////////
///// NMaterialStateDynamicTexturesInsp implementation

const FastNameMap<NMaterialStateDynamicTexturesInsp::PropData>* NMaterialStateDynamicTexturesInsp::FindMaterialTextures(NMaterial *material) const
{
    static FastNameMap<PropData> staticData;
    staticData.clear();

    if (material->fxName.IsValid())
    {
        HashMap<FastName, int32> flags;
        material->CollectMaterialFlags(flags);

        // shader data
        FXDescriptor fxDescriptor = FXCache::GetFXDescriptor(material->fxName, flags, material->qualityGroup);
        for (auto& descriptor : fxDescriptor.renderPassDescriptors)
        {
            const rhi::ShaderSamplerList& samplers = descriptor.shader->GetFragmentSamplerList();
            for (auto &samp : samplers)
            {
                if (0 == staticData.count(samp.uid))
                {
                    PropData data;
                    data.source = PropData::SOURCE_SHADER;

                    Texture* tex = material->GetEffectiveTexture(samp.uid);
                    if (nullptr != tex)
                    {
                        data.path = tex->GetPathname();
                    }

                    staticData.insert(samp.uid, data);
                }
            }
        }

        // local properties
        auto it = material->localTextures.begin();
        auto end = material->localTextures.end();

        for (; it != end; ++it)
        {
            FastName texName = it->first;
            if (0 == staticData.count(texName))
            {
                PropData data;
                data.path = it->second->path;
                data.source = PropData::SOURCE_SELF;
                staticData.insert(texName, data);
            }
            else
            {
                staticData[it->first].source |= PropData::SOURCE_SELF;
            }
        }
    }

    return &staticData;
}

Vector<FastName> NMaterialStateDynamicTexturesInsp::MembersList(void *object) const
{
    Vector<FastName> ret;
    
    NMaterial *material = (NMaterial*) object;
    DVASSERT(material);
    
    const FastNameMap<NMaterialStateDynamicTexturesInsp::PropData>* textures = FindMaterialTextures(material);
    
    FastNameMap<NMaterialStateDynamicTexturesInsp::PropData>::iterator it = textures->begin();
    FastNameMap<NMaterialStateDynamicTexturesInsp::PropData>::iterator end = textures->end();
    
    ret.reserve(textures->size());
    while(it != end)
    {
        ret.push_back(it->first);
        ++it;
    }
    
    return ret;
}

InspDesc NMaterialStateDynamicTexturesInsp::MemberDesc(void *object, const FastName &texture) const
{
    return InspDesc(texture.c_str());
}

VariantType NMaterialStateDynamicTexturesInsp::MemberValueGet(void *object, const FastName &texture) const
{
    VariantType ret;
    
    NMaterial *material = (NMaterial*) object;
    DVASSERT(material);
    
    const FastNameMap<NMaterialStateDynamicTexturesInsp::PropData>* textures = FindMaterialTextures(material);
    if(textures->count(texture))
    {
        ret.SetFilePath(textures->at(texture).path);
    }
    
    return ret;
}

void NMaterialStateDynamicTexturesInsp::MemberValueSet(void *object, const FastName &texName, const VariantType &value)
{
    VariantType ret;
    NMaterial *material = (NMaterial*) object;
    DVASSERT(material);
    
    const FastNameMap<NMaterialStateDynamicTexturesInsp::PropData>* textures = FindMaterialTextures(material);
    if (textures->count(texName))
    {
        if(value.type == VariantType::TYPE_NONE)
        {
            if (material->HasLocalTexture(texName))
            {
                material->RemoveTexture(texName);
            }
        }
        else
        {
            Texture *texture = Texture::CreateFromFile(value.AsFilePath());
            if (material->HasLocalTexture(texName))
            {
                material->SetTexture(texName, texture);
            }
            else
            {
                material->SetTexture(texName, texture);
            }

            SafeRelease(texture);
        }
    }
}

int NMaterialStateDynamicTexturesInsp::MemberFlags(void *object, const FastName &texture) const
{
    int flags = 0;
    
    NMaterial *material = (NMaterial*) object;
    DVASSERT(material);
    
    const FastNameMap<NMaterialStateDynamicTexturesInsp::PropData>* textures = FindMaterialTextures(material);
    if(textures->count(texture))
    {
        const PropData &propData = textures->at(texture);
        
        if(propData.source & PropData::SOURCE_SELF)
        {
            flags |= I_EDIT;
        }
        
        if(propData.source & PropData::SOURCE_SHADER)
        {
            flags |= I_VIEW;
        }
    }
    
    return flags;
}

};
