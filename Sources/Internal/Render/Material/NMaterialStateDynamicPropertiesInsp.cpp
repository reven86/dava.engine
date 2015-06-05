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
#include "Render/Material/NMaterialStateDynamicPropertiesInsp.h"

namespace DAVA
{
///////////////////////////////////////////////////////////////////////////
///// NMaterialStateDynamicPropertiesInsp implementation

const FastNameMap<NMaterialStateDynamicPropertiesInsp::PropData>* NMaterialStateDynamicPropertiesInsp::FindMaterialProperties(NMaterial *state, bool global) const
{
    static FastNameMap<PropData> staticData;
    
    staticData.clear();
    NMaterial *parent = state;
    int source = PropData::SOURCE_SELF;
    
    // properties chain data
    while(NULL != parent)
    {
        HashMap<FastName, NMaterialProperty* >::iterator it = parent->materialProperties.begin();
        HashMap<FastName, NMaterialProperty* >::iterator end = parent->materialProperties.end();
        
        for(; it != end; ++it)
        {
            FastName propName = it->first;
            
            // don't add some properties with light settings, they should be hidden for user
            if(propName != NMaterial::PARAM_LIGHT_AMBIENT_COLOR &&
               propName != NMaterial::PARAM_LIGHT_DIFFUSE_COLOR &&
               propName != NMaterial::PARAM_LIGHT_SPECULAR_COLOR)
            {
                if(0 == staticData.count(it->first))
                {
                    PropData data;
                    data.data = it->second->data;
                    data.size = it->second->size;
                    data.type = it->second->type;
                    data.source |= source;
                    
                    staticData.Insert(propName, data);
                    //printf("insert %s, %d\n", propName.c_str(), source);
                }
                else
                {
                    staticData[propName].source |= source;
                    //printf("update %s, %d\n", propName.c_str(), source);
                }
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
                    eShaderSemantic shaderSemantic = uniform->shaderSemantic;
                    if( //!Shader::IsAutobindUniform(uniform->id) && // isn't auto-bind
                       // we can't use IsAutobindUniform, because we need color to change
                       // so this is copy from IsAutobindUniform with color excluded -->
                       !(shaderSemantic == PARAM_WORLD_VIEW_PROJ ||
                         shaderSemantic == PARAM_WORLD_VIEW ||
                         shaderSemantic == PARAM_PROJ ||
                         shaderSemantic == PARAM_WORLD_VIEW_INV_TRANSPOSE ||
                         shaderSemantic == PARAM_GLOBAL_TIME ||
                         shaderSemantic == PARAM_WORLD_SCALE)
                       // <--
                       &&
                       uniform->type != Shader::UT_SAMPLER_2D && uniform->type != Shader::UT_SAMPLER_CUBE) // isn't texture
                    {
                        FastName propName = uniform->name;
                        
                        // redefine some shader properties names
                        if(propName == NMaterial::PARAM_LIGHT_AMBIENT_COLOR) propName = NMaterial::PARAM_PROP_AMBIENT_COLOR;
                        else if(propName == NMaterial::PARAM_LIGHT_DIFFUSE_COLOR) propName = NMaterial::PARAM_PROP_DIFFUSE_COLOR;
                        else if(propName == NMaterial::PARAM_LIGHT_SPECULAR_COLOR) propName = NMaterial::PARAM_PROP_SPECULAR_COLOR;
                        
                        if(!staticData.count(propName))
                        {
                            PropData data;
                            data.data = NULL;
                            data.size = uniform->size;
                            data.type = uniform->type;
                            data.source |= source;
                            
                            staticData.Insert(propName, data);
                            //printf("insert %s, %d\n", propName.c_str(), source);
                        }
                        else
                        {
                            staticData[propName].source |= source;
                            //printf("update %s, %d\n", propName.c_str(), source);
                        }
                    }
                }
            }
        }
    }
    
    return &staticData;
}

Vector<FastName> NMaterialStateDynamicPropertiesInsp::MembersList(void *object) const
{
    Vector<FastName> ret;
    
    NMaterial *state = (NMaterial*) object;
    DVASSERT(state);
    
    const FastNameMap<NMaterialStateDynamicPropertiesInsp::PropData>* members = FindMaterialProperties(state, false);
    
    FastNameMap<NMaterialStateDynamicPropertiesInsp::PropData>::iterator it = members->begin();
    FastNameMap<NMaterialStateDynamicPropertiesInsp::PropData>::iterator end = members->end();
    
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
    
    const FastNameMap<NMaterialStateDynamicPropertiesInsp::PropData>* members = FindMaterialProperties(state, true);
    if(members->count(member))
    {
        const PropData &propData = members->at(member);
        
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

VariantType NMaterialStateDynamicPropertiesInsp::MemberValueGet(void *object, const FastName &member) const
{
    VariantType ret;
    
    NMaterial *state = (NMaterial*) object;
    DVASSERT(state);
    
    const FastNameMap<NMaterialStateDynamicPropertiesInsp::PropData>* members = FindMaterialProperties(state, true);
    if(members->count(member))
    {
        const PropData &prop = members->at(member);
        ret = GetVariant(member, prop);
    }
    
    return ret;
}

VariantType NMaterialStateDynamicPropertiesInsp::GetVariant(const FastName &propName, const PropData &prop) const
{
    VariantType ret;
    
    // self or parent property
    if(NULL != prop.data)
    {
        switch(prop.type)
        {
            case Shader::UT_FLOAT:
                ret.SetFloat(*(float32*) prop.data);
                break;
                
            case Shader::UT_FLOAT_VEC2:
                ret.SetVector2(*(Vector2*) prop.data);
                break;
                
            case Shader::UT_FLOAT_VEC3:
                if(IsColor(propName))
                {
                    float32 *color = (float32*) prop.data;
                    ret.SetColor(Color(color[0], color[1], color[2], 1.0));
                }
                else
                {
                    ret.SetVector3(*(Vector3*) prop.data);
                }
                break;
                
            case Shader::UT_FLOAT_VEC4:
                if(IsColor(propName))
                {
                    float32 *color = (float32*) prop.data;
                    ret.SetColor(Color(color[0], color[1], color[2], color[3]));
                }
                else
                {
                    ret.SetVector4(*(Vector4*) prop.data);
                }
                break;
                
            case Shader::UT_INT:
                ret.SetInt32(*(int32*) prop.data);
                break;
                
            case Shader::UT_INT_VEC2:
            case Shader::UT_INT_VEC3:
            case Shader::UT_INT_VEC4:
                DVASSERT(false);
                //TODO: add a way to set int[]
                break;
                
            case Shader::UT_BOOL:
                ret.SetBool((*(int32*) prop.data) != 0);
                break;
                
            case Shader::UT_BOOL_VEC2:
            case Shader::UT_BOOL_VEC3:
            case Shader::UT_BOOL_VEC4:
                DVASSERT(false);
                //TODO: add a way to set bool[]
                break;
                
            case Shader::UT_FLOAT_MAT2:
                ret.SetMatrix2(*(Matrix2*) prop.data);
                break;
                
            case Shader::UT_FLOAT_MAT3:
                ret.SetMatrix3(*(Matrix3*) prop.data);
                break;
                
            case Shader::UT_FLOAT_MAT4:
                ret.SetMatrix4(*(Matrix4*) prop.data);
                break;
                
            case Shader::UT_SAMPLER_2D:
                ret.SetInt32(*(int32*) prop.data);
                break;
                
            case Shader::UT_SAMPLER_CUBE:
                ret.SetInt32(*(int32*) prop.data);
                break;
                
            default:
                DVASSERT(false);
                break;
        }
        
    }
    // shader property that is not set in self or parent properties
    else
    {
        switch(prop.type)
        {
            case Shader::UT_FLOAT: ret.SetFloat(0);	break;
            case Shader::UT_FLOAT_VEC2:	ret.SetVector2(DAVA::Vector2()); break;
            case Shader::UT_FLOAT_VEC3:	IsColor(propName) ? ret.SetColor(DAVA::Color()) : ret.SetVector3(DAVA::Vector3()); break;
            case Shader::UT_FLOAT_VEC4:	IsColor(propName) ? ret.SetColor(DAVA::Color()) : ret.SetVector4(DAVA::Vector4()); break;
            case Shader::UT_INT: ret.SetInt32(0); break;
            case Shader::UT_BOOL: ret.SetBool(false); break;
            case Shader::UT_FLOAT_MAT2:	ret.SetMatrix2(DAVA::Matrix2()); break;
            case Shader::UT_FLOAT_MAT3:	ret.SetMatrix3(DAVA::Matrix3()); break;
            case Shader::UT_FLOAT_MAT4:	ret.SetMatrix4(DAVA::Matrix4()); break;
            case Shader::UT_SAMPLER_2D:	ret.SetInt32(0); break;
            case Shader::UT_SAMPLER_CUBE: ret.SetInt32(0); break;
                
            case Shader::UT_INT_VEC2:
            case Shader::UT_INT_VEC3:
            case Shader::UT_INT_VEC4:
                DVASSERT(false);
                //TODO: add a way to set int[]
                break;
                
            case Shader::UT_BOOL_VEC2:
            case Shader::UT_BOOL_VEC3:
            case Shader::UT_BOOL_VEC4:
                DVASSERT(false);
                //TODO: add a way to set bool[]
                break;
                
            default:
                DVASSERT(false);
                break;
        }
    }
    
    return ret;
}

bool NMaterialStateDynamicPropertiesInsp::IsColor(const FastName &propName) const
{
    return (NULL != strstr(propName.c_str(), "Color"));
}

void NMaterialStateDynamicPropertiesInsp::MemberValueSet(void *object, const FastName &member, const VariantType &value)
{
    NMaterial *state = (NMaterial*) object;
    DVASSERT(state);
    
    const FastNameMap<NMaterialStateDynamicPropertiesInsp::PropData>* members = FindMaterialProperties(state, true);
    if(members->count(member))
    {
        PropData prop = members->at(member);
        int propSize = prop.size;
        Shader::eUniformType propType = prop.type;
        
        if(value.GetType() == VariantType::TYPE_NONE && state->GetMaterialType() != NMaterial::MATERIALTYPE_GLOBAL)
        {
            // empty variant value should remove this property
            state->RemoveMaterialProperty(member);
        }
        else
        {
            switch(prop.type)
            {
                case Shader::UT_FLOAT:
                {
                    float32 val = value.AsFloat();
                    state->SetPropertyValue(member, propType, propSize, &val);
                }
                    break;
                    
                case Shader::UT_FLOAT_VEC2:
                {
                    const Vector2& val = value.AsVector2();
                    state->SetPropertyValue(member, propType, propSize, &val);
                }
                    break;
                    
                case Shader::UT_FLOAT_VEC3:
                {
                    Vector3 val;
                    
                    if(IsColor(member))
                    {
                        Color c = value.AsColor();
                        val = Vector3(c.r, c.g, c.b);
                    }
                    else
                    {
                        val = value.AsVector3();
                    }
                    
                    state->SetPropertyValue(member, propType, propSize, &val);
                }
                    break;
                    
                case Shader::UT_FLOAT_VEC4:
                {
                    Vector4 val;
                    
                    if(IsColor(member))
                    {
                        Color c = value.AsColor();
                        val = Vector4(c.r, c.g, c.b, c.a);
                    }
                    else
                    {
                        val = value.AsVector4();
                    }
                    
                    state->SetPropertyValue(member, propType, propSize, &val);
                }
                    break;
                    
                case Shader::UT_INT:
                {
                    int32 val = value.AsInt32();
                    state->SetPropertyValue(member, propType, propSize, &val);
                }
                    break;
                    
                case Shader::UT_INT_VEC2:
                case Shader::UT_INT_VEC3:
                case Shader::UT_INT_VEC4:
                {
                    DVASSERT(false);
                    //TODO: add a way to set int[]
                }
                    break;
                    
                case Shader::UT_BOOL:
                {
                    bool val = value.AsBool();
                    state->SetPropertyValue(member, propType, propSize, &val);
                }
                    break;
                    
                case Shader::UT_BOOL_VEC2:
                case Shader::UT_BOOL_VEC3:
                case Shader::UT_BOOL_VEC4:
                {
                    DVASSERT(false);
                    //TODO: add a way to set bool[]
                }
                    break;
                    
                case Shader::UT_FLOAT_MAT2:
                {
                    const Matrix2& val = value.AsMatrix2();
                    state->SetPropertyValue(member, propType, propSize, &val);
                }
                    break;
                    
                case Shader::UT_FLOAT_MAT3:
                {
                    const Matrix3& val = value.AsMatrix3();
                    state->SetPropertyValue(member, propType, propSize, &val);
                }
                    break;
                    
                case Shader::UT_FLOAT_MAT4:
                {
                    const Matrix4& val = value.AsMatrix4();
                    state->SetPropertyValue(member, propType, propSize, &val);
                }
                    break;
                    
                case Shader::UT_SAMPLER_2D:
                    //VI: samplers are set by config materials
                    break;
                    
                case Shader::UT_SAMPLER_CUBE:
                    //VI: samplers are set by config materials
                    break;
                    
                default:
                    DVASSERT(false);
                    break;
            }
        }
    }
}

};