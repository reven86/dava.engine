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


#ifndef __DAVAENGINE_UI_STYLESHEET_PROPERTIES_TABLE_H__
#define __DAVAENGINE_UI_STYLESHEET_PROPERTIES_TABLE_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Base/StaticSingleton.h"
#include "FileSystem/VariantType.h"

namespace DAVA
{
    enum class ePropertyOwner
    {
        CONTROL,
        BACKGROUND,
        COMPONENT,
    };

    struct UIStyleSheetPropertyTargetMember
    {
        ePropertyOwner propertyOwner;
        uint32 componentType;
        const InspInfo* typeInfo;
        const InspMember* memberInfo;

        bool operator == (const UIStyleSheetPropertyTargetMember& other) const
        {
            return  propertyOwner == other.propertyOwner &&
                    componentType == other.componentType &&
                    typeInfo == other.typeInfo &&
                    memberInfo == other.memberInfo;
        }
    };

    struct UIStyleSheetPropertyDescriptor
    {
        FastName name;
        VariantType defaultValue;
        Vector<UIStyleSheetPropertyTargetMember> targetMembers;
    };

    class UIStyleSheetPropertyDataBase : 
        public StaticSingleton<UIStyleSheetPropertyDataBase >
    {
    public:
        enum { STYLE_SHEET_PROPERTY_COUNT = 30 };

        UIStyleSheetPropertyDataBase();

        uint32 GetStyleSheetPropertyIndex(const FastName& name) const;
        bool IsValidStyleSheetProperty(const FastName& name) const;
        const UIStyleSheetPropertyDescriptor& GetStyleSheetPropertyByIndex(uint32 index) const;
    private:
        UnorderedMap<FastName, uint32> propertyNameToIndexMap;
        Array<UIStyleSheetPropertyDescriptor, STYLE_SHEET_PROPERTY_COUNT> properties;

        struct ComponentPropertyRegistrator
        {
            UIStyleSheetPropertyTargetMember operator () (uint32 index, const InspInfo* typeInfo, const InspMember* member) const
            {
                return UIStyleSheetPropertyTargetMember{ ePropertyOwner::COMPONENT, componentType, typeInfo, member };
            }

            uint32 componentType;
        };

        struct BackgroundPropertyRegistrator
        {
            UIStyleSheetPropertyTargetMember operator () (uint32 index, const InspInfo* typeInfo, const InspMember* member) const
            {
                return UIStyleSheetPropertyTargetMember{ ePropertyOwner::BACKGROUND, 0, typeInfo, member };
            }
        };

        struct ControlPropertyRegistrator
        {
            UIStyleSheetPropertyTargetMember operator () (uint32 index, const InspInfo* typeInfo, const InspMember* member) const
            {
                return UIStyleSheetPropertyTargetMember{ ePropertyOwner::CONTROL, 0, typeInfo, member };
            }
        };

        template < typename CallbackType >
        void ProcessObjectIntrospection(const InspInfo* typeInfo, const CallbackType& callback)
        {
            const InspInfo *baseInfo = typeInfo->BaseInfo();
            if (baseInfo)
                ProcessObjectIntrospection(baseInfo, callback);

            for (int32 i = 0; i < typeInfo->MembersCount(); i++)
            {
                const InspMember *member = typeInfo->Member(i);

                const auto& iter = propertyNameToIndexMap.find(member->GetFastName());
                if (iter != propertyNameToIndexMap.end())
                {
                    DVASSERT(properties[iter->second].targetMembers.empty() ? true : member->Type() == properties[iter->second].targetMembers.back().memberInfo->Type());

                    Vector<UIStyleSheetPropertyTargetMember>& targetMembers = properties[iter->second].targetMembers;
                    const UIStyleSheetPropertyTargetMember& newMember = callback(iter->second, typeInfo, member);

                    if (std::find(targetMembers.begin(), targetMembers.end(), newMember) != targetMembers.end())
                        return;

                    targetMembers.push_back(newMember);
                }
            }
        }

        template < typename ComponentType >
        void ProcessComponentIntrospection()
        {
            ProcessObjectIntrospection(ComponentType::TypeInfo(), ComponentPropertyRegistrator{ ComponentType::C_TYPE });
        }

        template < typename ControlType >
        void ProcessControlIntrospection()
        {
            ProcessObjectIntrospection(ControlType::TypeInfo(), ControlPropertyRegistrator());
        }
    };

    typedef Bitset<UIStyleSheetPropertyDataBase::STYLE_SHEET_PROPERTY_COUNT> UIStyleSheetPropertySet;
};


#endif
