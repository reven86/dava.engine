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


#ifndef __DAVAENGINE_UI_STYLESHEET_STRUCTS_H__
#define __DAVAENGINE_UI_STYLESHEET_STRUCTS_H__

#include "Base/IntrospectionBase.h"
#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "FileSystem/VariantType.h"
#include "Animation/Interpolation.h"

namespace DAVA
{

enum class ePropertyOwner
{
    CONTROL,
    BACKGROUND,
    COMPONENT,
};

struct UIStyleSheetPropertyGroup
{
    String prefix;
    ePropertyOwner propertyOwner;
    uint32 componentType;
    const InspInfo* typeInfo;

    UIStyleSheetPropertyGroup(const String& prefix_, ePropertyOwner owner_, uint32 componentType_, const InspInfo* typeInfo_)
        : prefix(prefix_)
        , propertyOwner(owner_)
        , componentType(componentType_)
        , typeInfo(typeInfo_)
    {
    }
};

struct UIStyleSheetPropertyDescriptor
{
    UIStyleSheetPropertyGroup* group;

    FastName name;
    VariantType defaultValue;
    const InspMember* memberInfo;

    UIStyleSheetPropertyDescriptor(UIStyleSheetPropertyGroup* group_, const FastName& name_, const VariantType& defaultValue_)
        : group(group_)
        , name(name_)
        , defaultValue(defaultValue_)
        , memberInfo(nullptr)
    {
    }

    String GetFullName() const
    {
        if (group->prefix.empty())
            return String(name.c_str());

        return group->prefix + String("-") + String(name.c_str());
    }
};

struct UIStyleSheetSelector
{
    UIStyleSheetSelector() :
        className(""),
        name(),
        stateMask(0)
    {

    }

    String className;
    FastName name;
    int32 stateMask;
    Vector<FastName> classes;
};

struct UIStyleSheetProperty
{
    UIStyleSheetProperty(uint32 aPropertyIndex, const VariantType& aValue, bool aTransition = false, Interpolation::FuncType aTransitionFunction = Interpolation::LINEAR, float32 aTransitionTime = 0.0f) :
        propertyIndex(aPropertyIndex),
        value(aValue),
        transitionFunction(aTransitionFunction),
        transitionTime(aTransitionTime), 
        transition(aTransition)
    {

    }

    uint32 propertyIndex;
    VariantType value;

    Interpolation::FuncType transitionFunction;
    float32 transitionTime;
    bool transition;
};

struct UIStyleSheetClass
{
    UIStyleSheetClass(const FastName& tag_, const FastName& clazz_)
        : tag(tag_)
        , clazz(clazz_)
    {
    }

    FastName tag;
    FastName clazz;
};
};


#endif
