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

namespace DAVA
{
///////////////////////////////////////////////////////////////////////////
///// NMaterialStateDynamicTexturesInsp implementation

const FastNameMap<NMaterialStateDynamicTexturesInsp::PropData>* NMaterialStateDynamicTexturesInsp::FindMaterialTextures(NMaterial *state, bool global) const
{
    static FastNameMap<PropData> staticData;
    staticData.clear();

    // don't enumerate textures for global material
    // this is temporary solution and is done because there is no 
    // code in framework that can load textures, that are be set into global material
    //
    // TODO: before removing this check we should implement some kind of code to load global material textures
    //
    if(state->materialType != NMaterial::MATERIALTYPE_GLOBAL)
    {
        NMaterial *parent = state;
        int source = PropData::SOURCE_SELF;
    
        // properties chain data
        while(NULL != parent)
        {
            HashMap<FastName, NMaterial::TextureBucket*>::iterator it = parent->textures.begin();
            HashMap<FastName, NMaterial::TextureBucket*>::iterator end = parent->textures.end();
        
            for(; it != end; ++it)
            {
                if(0 == staticData.count(it->first))
                {
                    PropData data;
                    NMaterial::TextureBucket *bucket = it->second;
                
                    data.source |= source;
                    data.path = bucket->GetPath();
                
                    staticData.Insert(it->first, data);
                }
            }
        
            parent = parent->GetParent();
            source = PropData::SOURCE_PARENT;
        
            if(!global && NULL != parent)
            {
                if(parent->GetMaterialType() == NMaterial::MATERIALTYPE_GLOBAL)
                {
                    // don't extract properties from globalMaterial
                    parent = NULL;
                }
            }
        }
    
    
        // shader data
        source = PropData::SOURCE_SHADER;
        if(state->instancePasses.size() > 0)
        {
            HashMap<FastName, NMaterial::RenderPassInstance*>::iterator it = state->instancePasses.begin();
            HashMap<FastName, NMaterial::RenderPassInstance*>::iterator end = state->instancePasses.end();
        
            for(; it != end; ++it)
            {
                Shader *shader = it->second->GetShader();
                if(NULL != shader)
                {
                    int32 uniformCount = shader->GetUniformCount();
                    for(int32 i = 0; i < uniformCount; ++i)
                    {
                        Shader::Uniform *uniform = shader->GetUniform(i);
                        if( uniform->type == Shader::UT_SAMPLER_2D ||
                           uniform->type == Shader::UT_SAMPLER_CUBE) // is texture
                        {
                            FastName propName = uniform->name;
                        
                            if(!staticData.count(propName))
                            {
                                PropData data;
                            
                                data.path = FilePath();
                                data.source |= source;
                            
                                staticData.Insert(propName, data);
                            }
                            else
                            {
                                staticData[propName].source |= source;
                            }
                        }
                    }
                }
            }
        }
    }

    return &staticData;
}

Vector<FastName> NMaterialStateDynamicTexturesInsp::MembersList(void *object) const
{
    Vector<FastName> ret;
    
    NMaterial *state = (NMaterial*) object;
    DVASSERT(state);
    
    const FastNameMap<NMaterialStateDynamicTexturesInsp::PropData>* textures = FindMaterialTextures(state, false);
    
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
    
    NMaterial *state = (NMaterial*) object;
    DVASSERT(state);
    
    const FastNameMap<NMaterialStateDynamicTexturesInsp::PropData>* textures = FindMaterialTextures(state, true);
    if(textures->count(texture))
    {
        ret.SetFilePath(textures->at(texture).path);
    }
    
    return ret;
}

void NMaterialStateDynamicTexturesInsp::MemberValueSet(void *object, const FastName &texture, const VariantType &value)
{
    VariantType ret;
    NMaterial *state = (NMaterial*) object;
    DVASSERT(state);
    
    const FastNameMap<NMaterialStateDynamicTexturesInsp::PropData>* textures = FindMaterialTextures(state, true);
    if(textures->count(texture))
    {
        if(value.type == VariantType::TYPE_NONE && state->GetMaterialType() != NMaterial::MATERIALTYPE_GLOBAL)
        {
            if(state->textures.count(texture) > 0)
            {
                state->RemoveTexture(texture);
            }
        }
        else
        {
            state->SetTexture(texture, value.AsFilePath());
        }
    }
}

int NMaterialStateDynamicTexturesInsp::MemberFlags(void *object, const FastName &texture) const
{
    int flags = 0;
    
    NMaterial *state = (NMaterial*) object;
    DVASSERT(state);
    
    const FastNameMap<NMaterialStateDynamicTexturesInsp::PropData>* textures = FindMaterialTextures(state, true);
    if(textures->count(texture))
    {
        const PropData &propData = textures->at(texture);
        
        if(propData.source & PropData::SOURCE_SELF)
        {
            flags |= I_EDIT;
        }
        
        if(propData.source & PropData::SOURCE_PARENT)
        {
            flags |= I_VIEW;
        }
        
        if(propData.source & PropData::SOURCE_SHADER)
        {
            flags |= I_SAVE;
        }
    }
    
    return flags;
}

};
