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

#if _MATERIAL_OFF

#include "Render/Material/NMaterial.h"
#include "Render/Material/NMaterialStateDynamicFlagsInsp.h"

namespace DAVA
{

///////////////////////////////////////////////////////////////////////////
///// NMaterialStateDynamicFlagsInsp implementation

Vector<FastName> NMaterialStateDynamicFlagsInsp::MembersList(void *object) const
{
    static Vector<FastName> ret;
    
    if(0 == ret.size())
    {
        ret.reserve(18);
        
        ret.push_back(NMaterial::FLAG_VERTEXFOG);
        ret.push_back(NMaterial::FLAG_FOG_LINEAR);
        ret.push_back(NMaterial::FLAG_FOG_HALFSPACE);
        ret.push_back(NMaterial::FLAG_FOG_HALFSPACE_LINEAR);
        ret.push_back(NMaterial::FLAG_FOG_ATMOSPHERE);

        ret.push_back(NMaterial::FLAG_FLATCOLOR);
        ret.push_back(NMaterial::FLAG_TEXTURESHIFT);
        ret.push_back(NMaterial::FLAG_TEXTURE0_ANIMATION_SHIFT);
        
        ret.push_back(NMaterial::FLAG_WAVE_ANIMATION);
        ret.push_back(NMaterial::FLAG_FAST_NORMALIZATION);
        
        ret.push_back(NMaterial::FLAG_SPECULAR);
        ret.push_back(NMaterial::FLAG_SEPARATE_NORMALMAPS);
        ret.push_back(NMaterial::FLAG_TANGENT_SPACE_WATER_REFLECTIONS);
        ret.push_back(NMaterial::FLAG_DEBUG_UNITY_Z_NORMAL);
        ret.push_back(NMaterial::FLAG_DEBUG_Z_NORMAL_SCALE);
        ret.push_back(NMaterial::FLAG_DEBUG_NORMAL_ROTATION);        
        ret.push_back(NMaterial::FLAG_SKINNING);
        ret.push_back(NMaterial::FLAG_TILED_DECAL_MASK);
    }
    return ret;
}

InspDesc NMaterialStateDynamicFlagsInsp::MemberDesc(void *object, const FastName &member) const
{
    return InspDesc(member.c_str());
}

VariantType NMaterialStateDynamicFlagsInsp::MemberValueGet(void *object, const FastName &member) const
{
    VariantType ret;
    NMaterial *state = (NMaterial*) object;
    DVASSERT(state);
    
    ret.SetBool(state->IsFlagEffective(member));
    return ret;
}

void NMaterialStateDynamicFlagsInsp::MemberValueSet(void *object, const FastName &member, const VariantType &value)
{
    NMaterial *state = (NMaterial*) object;
    DVASSERT(state);
    
    NMaterial::eFlagValue newValue = NMaterial::FlagOff;
    if(value.GetType() == VariantType::TYPE_BOOLEAN && value.AsBool())
    {
        newValue = NMaterial::FlagOn;
    }
    
    if(state->GetMaterialType() == NMaterial::MATERIALTYPE_GLOBAL)
    {
        // global material accepts only valid values
        if(value.GetType() == VariantType::TYPE_BOOLEAN)
        {
            state->SetFlag(member, newValue);
        }
    }
    else
    {
        // empty value is thread as flag remove
        if(value.GetType() == VariantType::TYPE_NONE)
        {
            state->ResetFlag(member);
        }
        else
        {
            state->SetFlag(member, newValue);
        }
    }
}

int NMaterialStateDynamicFlagsInsp::MemberFlags(void *object, const FastName &member) const
{
    int ret = I_VIEW;
    
    NMaterial *state = (NMaterial*) object;
    DVASSERT(state);
    
    if(!(NMaterial::FlagInherited & state->GetFlagValue(member)))
    {
        ret |= I_EDIT;
    }
    
    return ret;
}

};

#endif //_MATERIAL_OFF