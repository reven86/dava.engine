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


#ifndef __DAVAENGINE_NMATERIALSTATEDYNAMICTEXTURESINSP_NAMES_H__
#define __DAVAENGINE_NMATERIALSTATEDYNAMICTEXTURESINSP_NAMES_H__

#include "Base/BaseTypes.h"
#include "Base/HashMap.h"
#include "Base/FastNameMap.h"

namespace DAVA
{
class NMaterial;

class NMaterialStateDynamicTexturesInsp : public InspInfoDynamic
{
public:
    Vector<FastName> MembersList(void *object) const;
    InspDesc MemberDesc(void *object, const FastName &member) const;
    int MemberFlags(void *object, const FastName &member) const;
    VariantType MemberValueGet(void *object, const FastName &member) const;
    void MemberValueSet(void *object, const FastName &member, const VariantType &value);
    
protected:
    struct PropData
    {
        enum PropSource
        {
            SOURCE_UNKNOWN = 0x0,
            SOURCE_SELF = 0x1,
            SOURCE_PARENT = 0x2,
            SOURCE_SHADER = 0x4
        };
        
        PropData() : source(SOURCE_UNKNOWN)
        { }
        
        int source;
        FilePath path;
    };
    
    const FastNameMap<PropData>* FindMaterialTextures(NMaterial *state, bool global) const;
};
};

#endif /* defined(__DAVAENGINE_NMATERIALSTATEDYNAMICTEXTURESINSP_NAMES_H__) */
