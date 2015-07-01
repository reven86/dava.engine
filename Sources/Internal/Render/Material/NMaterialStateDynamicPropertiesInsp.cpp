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
#include "Render/Material/FXCache.h"
#include "Render/Material/NMaterialStateDynamicPropertiesInsp.h"

namespace DAVA
{
///////////////////////////////////////////////////////////////////////////
///// NMaterialStateDynamicPropertiesInsp implementation

const FastNameMap<NMaterialStateDynamicPropertiesInsp::PropData>* NMaterialStateDynamicPropertiesInsp::FindMaterialProperties(NMaterial *material) const
{
    static FastNameMap<PropData> staticData;
    staticData.clear();

    HashMap<FastName, int32> flags;
    material->CollectMaterialFlags(flags);

    // shader data
    if (material->fxName.IsValid())
    {
        FXDescriptor fxDescriptor = FXCache::GetFXDescriptor(material->fxName, flags, material->qualityGroup);
        for (auto& descriptor : fxDescriptor.renderPassDescriptors)
        {
            for (auto& buff : descriptor.shader->GetConstBufferDescriptors())
            {
                const rhi::ShaderPropList& props = ShaderDescriptor::GetProps(buff.propertyLayoutId);
                for (auto &prop : props)
                {
                    if (prop.storage != rhi::ShaderProp::STORAGE_DYNAMIC)
                    {
                        if (0 == staticData.count(prop.uid))
                        {
                            const float32 *value = material->GetEffectivePropValue(prop.uid);
                            if (nullptr == value)
                            {
                                value = prop.defaultValue;
                            }

                            PropData data;
                            data.size = prop.arraySize;
                            data.type = prop.type;
                            data.source = PropData::SOURCE_SHADER;
                            data.value = value;
                            staticData.insert(prop.uid, data);
                        }
                    }
                }
            }
        }
    }

    // local properties
    auto it = material->localProperties.begin();
    auto end = material->localProperties.end();

    for (; it != end; ++it)
    {
        FastName propName = it->first;
        if (0 == staticData.count(propName))
        {
            PropData data;
            data.size = it->second->arraySize;
            data.type = it->second->type;
            data.source = PropData::SOURCE_SELF;
            data.value = material->GetEffectivePropValue(propName);
            staticData.insert(propName, data);
        }
        else
        {
            staticData[it->first].source |= PropData::SOURCE_SELF;
        }
    }

    return &staticData;
}

Vector<FastName> NMaterialStateDynamicPropertiesInsp::MembersList(void *object) const
{
    Vector<FastName> ret;
    
    NMaterial *state = (NMaterial*) object;
    DVASSERT(state);
    
    const FastNameMap<NMaterialStateDynamicPropertiesInsp::PropData>* members = FindMaterialProperties(state);
    
    auto it = members->begin();
    auto end = members->end();
    
    ret.reserve(members->size());
    while(it != end)
    {
        ret.push_back(it->first);
        ++it;
    }
    
    return ret;
}

InspDesc NMaterialStateDynamicPropertiesInsp::MemberDesc(void *object, const FastName &member) const
{
    return InspDesc(member.c_str());
}

int NMaterialStateDynamicPropertiesInsp::MemberFlags(void *object, const FastName &member) const
{
    int flags = 0;
    
    NMaterial *state = (NMaterial*) object;
    DVASSERT(state);
    
    const FastNameMap<NMaterialStateDynamicPropertiesInsp::PropData>* members = FindMaterialProperties(state);
    if(members->count(member))
    {
        const PropData &propData = members->at(member);
        
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

VariantType NMaterialStateDynamicPropertiesInsp::MemberValueGet(void *object, const FastName &key) const
{
    VariantType ret;
    
    NMaterial *material = (NMaterial*) object;
    DVASSERT(material);
    
    const FastNameMap<NMaterialStateDynamicPropertiesInsp::PropData>* members = FindMaterialProperties(material);
    if(members->count(key))
    {
        const PropData &prop = members->at(key);

        // self or parent property
        if (nullptr != prop.value)
        {
            switch (prop.type)
            {
            case rhi::ShaderProp::TYPE_FLOAT1:
                ret.SetFloat(*(float32*)prop.value);
                break;

            case rhi::ShaderProp::TYPE_FLOAT2:
                ret.SetVector2(*(Vector2*)prop.value);
                break;

            case rhi::ShaderProp::TYPE_FLOAT3:
                if (IsColor(key))
                {
                    float32 *color = (float32*)prop.value;
                    ret.SetColor(Color(color[0], color[1], color[2], 1.0));
                }
                else
                {
                    ret.SetVector3(*(Vector3*)prop.value);
                }
                break;

            case rhi::ShaderProp::TYPE_FLOAT4:
                if (IsColor(key))
                {
                    float32 *color = (float32*)prop.value;
                    ret.SetColor(Color(color[0], color[1], color[2], color[3]));
                }
                else
                {
                    ret.SetVector4(*(Vector4*)prop.value);
                }
                break;

            case rhi::ShaderProp::TYPE_FLOAT4X4:
                ret.SetMatrix4(*(Matrix4*)prop.value);
                break;

            default:
                DVASSERT(false);
                break;
            }
        }
    }
    
    return ret;
}

bool NMaterialStateDynamicPropertiesInsp::IsColor(const FastName &propName) const
{
    return (NULL != strstr(propName.c_str(), "Color"));
}

void NMaterialStateDynamicPropertiesInsp::MemberValueSet(void *object, const FastName &key, const VariantType &value)
{
    NMaterial *material = (NMaterial*)object;
    DVASSERT(material);

    const FastNameMap<NMaterialStateDynamicPropertiesInsp::PropData>* members = FindMaterialProperties(material);
    if(members->count(key))
    {
        PropData prop = members->at(key);

        if(value.GetType() == VariantType::TYPE_NONE /* && state->GetMaterialType() != NMaterial::MATERIALTYPE_GLOBAL */)
        {
            // empty variant value should remove this property
            if (material->HasLocalProperty(key))
            {
                material->RemoveProperty(key);
            }
        }
        else
        {
            auto setValue = [&material, &key, &prop](float32* value) {
                if (material->HasLocalProperty(key))
                {
                    material->SetPropertyValue(key, value);
                }
                else
                {
                    material->AddProperty(key, value, prop.type, prop.size);
                }
            };

            switch(prop.type)
            {
            case rhi::ShaderProp::TYPE_FLOAT1:
                {
                    float32 val = value.AsFloat();
                    setValue(&val);
                }
                break;

            case rhi::ShaderProp::TYPE_FLOAT2:
                {
                    const Vector2& val = value.AsVector2();
                    setValue((float32 *) &val);
                }
                break;

            case rhi::ShaderProp::TYPE_FLOAT3:
                {
                    Vector3 val;
                    
                    if(IsColor(key))
                    {
                        Color c = value.AsColor();
                        val = Vector3(c.r, c.g, c.b);
                    }
                    else
                    {
                        val = value.AsVector3();
                    }
                    
                    setValue((float32 *) &val);
                }
                break;

            case rhi::ShaderProp::TYPE_FLOAT4:
                {
                    Vector4 val;
                    
                    if(IsColor(key))
                    {
                        Color c = value.AsColor();
                        val = Vector4(c.r, c.g, c.b, c.a);
                    }
                    else
                    {
                        val = value.AsVector4();
                    }
                    
                    setValue((float32 *) &val);
                }
                break;
                    
            case rhi::ShaderProp::TYPE_FLOAT4X4:
                {
                    const Matrix4& val = value.AsMatrix4();
                    setValue((float32 *) &val);
                }
                break;
                    
            default:
                DVASSERT(false);
                break;
            }
        }
    }
}

};
