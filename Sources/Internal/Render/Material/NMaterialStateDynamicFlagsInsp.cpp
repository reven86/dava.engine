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
#include "Render/Material/NMaterialStateDynamicFlagsInsp.h"

namespace DAVA
{

///////////////////////////////////////////////////////////////////////////
///// NMaterialStateDynamicFlagsInsp implementation

InspInfoDynamic::DynamicData NMaterialStateDynamicFlagsInsp::Prepare(void *object, int filter) const
{
    InspInfoDynamic::DynamicData ddata;
    ddata.object = object;

    return ddata;
}

Vector<FastName> NMaterialStateDynamicFlagsInsp::MembersList(const DynamicData& ddata) const
{
    static Vector<FastName> ret;

    if (ret.empty())
    {
        ret.reserve(22);
        
        ret.push_back(NMaterialFlagName::FLAG_VERTEXFOG);
        ret.push_back(NMaterialFlagName::FLAG_FOG_LINEAR);
        ret.push_back(NMaterialFlagName::FLAG_FOG_HALFSPACE);
        ret.push_back(NMaterialFlagName::FLAG_FOG_HALFSPACE_LINEAR);
        ret.push_back(NMaterialFlagName::FLAG_FOG_ATMOSPHERE);

        ret.push_back(NMaterialFlagName::FLAG_FLATCOLOR);
        ret.push_back(NMaterialFlagName::FLAG_TEXTURESHIFT);
        ret.push_back(NMaterialFlagName::FLAG_TEXTURE0_ANIMATION_SHIFT);
        
        ret.push_back(NMaterialFlagName::FLAG_WAVE_ANIMATION);
        ret.push_back(NMaterialFlagName::FLAG_FAST_NORMALIZATION);
        
        ret.push_back(NMaterialFlagName::FLAG_SPECULAR);
        ret.push_back(NMaterialFlagName::FLAG_SEPARATE_NORMALMAPS);
        ret.push_back(NMaterialFlagName::FLAG_TANGENT_SPACE_WATER_REFLECTIONS);
        ret.push_back(NMaterialFlagName::FLAG_DEBUG_UNITY_Z_NORMAL);
        ret.push_back(NMaterialFlagName::FLAG_DEBUG_Z_NORMAL_SCALE);
        ret.push_back(NMaterialFlagName::FLAG_DEBUG_NORMAL_ROTATION);        
        ret.push_back(NMaterialFlagName::FLAG_SKINNING);
        ret.push_back(NMaterialFlagName::FLAG_TILED_DECAL_MASK);
		ret.push_back(NMaterialFlagName::FLAG_ALPHATESTVALUE);
        ret.push_back(NMaterialFlagName::FLAG_ILLUMINATION_USED);
        ret.push_back(NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_CASTER);
        ret.push_back(NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_RECEIVER);
    }

    return ret;
}

InspDesc NMaterialStateDynamicFlagsInsp::MemberDesc(const DynamicData& ddata, const FastName &member) const
{
    return InspDesc(member.c_str());
}

VariantType NMaterialStateDynamicFlagsInsp::MemberValueGet(const DynamicData& ddata, const FastName &member) const
{
    NMaterial *material = static_cast<NMaterial*>(ddata.object);
    DVASSERT(material);

    VariantType ret;
    ret.SetBool(0 != material->GetEffectiveFlagValue(member));
    return ret;
}

void NMaterialStateDynamicFlagsInsp::MemberValueSet(const DynamicData& ddata, const FastName &member, const VariantType &value)
{
    NMaterial *material = static_cast<NMaterial*>(ddata.object);
    DVASSERT(material);

    // empty value is thread as flag remove
    if (value.GetType() == VariantType::TYPE_NONE)
    {
        if (material->HasLocalFlag(member))
        {
            material->RemoveFlag(member);
        }
    }
    else
    {
        int32 newValue = 0;
        if ((value.GetType() == VariantType::TYPE_BOOLEAN) && value.AsBool())
            newValue = 1;

        if (material->HasLocalFlag(member))
        {
            material->SetFlag(member, newValue);
        }
        else
        {
            material->AddFlag(member, newValue);
        }
    }
}

int NMaterialStateDynamicFlagsInsp::MemberFlags(const DynamicData& ddata, const FastName &member) const
{
    NMaterial *material = static_cast<NMaterial*>(ddata.object);
    DVASSERT(material);

    return I_VIEW | (material->HasLocalFlag(member) ? I_EDIT : 0);
}

};

