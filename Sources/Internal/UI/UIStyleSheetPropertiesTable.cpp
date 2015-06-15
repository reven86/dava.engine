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

#include "UI/UIStyleSheetPropertiesTable.h"
#include "UI/UIControl.h"
#include "UI/UIStaticText.h"
#include "UI/UIControlBackground.h"
#include "UI/Components/UIFakeComponent.h"
#include "UI/Components/UIFakeMultiComponent.h"

namespace DAVA
{
    namespace
    {
        UIStyleSheetPropertyDescriptor STYLE_SHEET_PROPERTY_TABLE[STYLE_SHEET_PROPERTY_COUNT] = {
            { FastName("angle"), VariantType::TYPE_FLOAT },
            { FastName("scale"), VariantType::TYPE_VECTOR2 },
            { FastName("visible"), VariantType::TYPE_BOOLEAN },
            { FastName("enabled"), VariantType::TYPE_BOOLEAN },
            { FastName("leftAlignEnabled"), VariantType::TYPE_BOOLEAN },
            { FastName("rightAlignEnabled"), VariantType::TYPE_BOOLEAN },
            { FastName("bottomAlignEnabled"), VariantType::TYPE_BOOLEAN },
            { FastName("topAlignEnabled"), VariantType::TYPE_BOOLEAN },
            { FastName("hcenterAlignEnabled"), VariantType::TYPE_BOOLEAN },
            { FastName("vcenterAlignEnabled"), VariantType::TYPE_BOOLEAN },
            { FastName("leftAlign"), VariantType::TYPE_FLOAT },
            { FastName("rightAlign"), VariantType::TYPE_FLOAT },
            { FastName("bottomAlign"), VariantType::TYPE_FLOAT },
            { FastName("topAlign"), VariantType::TYPE_FLOAT },
            { FastName("hcenterAlign"), VariantType::TYPE_FLOAT },
            { FastName("vcenterAlign"), VariantType::TYPE_FLOAT },

            { FastName("drawType"), VariantType::TYPE_INT32 },
            { FastName("sprite"), VariantType::TYPE_FILEPATH },
            { FastName("frame"), VariantType::TYPE_INT32 },
            { FastName("color"), VariantType::TYPE_COLOR },
            { FastName("colorInherit"), VariantType::TYPE_INT32 },
            { FastName("align"), VariantType::TYPE_INT32 },
            { FastName("leftRightStretchCap"), VariantType::TYPE_FLOAT },
            { FastName("topBottomStretchCap"), VariantType::TYPE_FLOAT },

            { FastName("font"), VariantType::TYPE_STRING },
            { FastName("textColor"), VariantType::TYPE_COLOR },
            { FastName("textcolorInheritType"), VariantType::TYPE_INT32 },
            { FastName("shadowoffset"), VariantType::TYPE_VECTOR2 },
            { FastName("shadowcolor"), VariantType::TYPE_COLOR },
            { FastName("textalign"), VariantType::TYPE_INT32 },
        };

        UnorderedMap< FastName, uint32 > propertyNameToIndexMap;

        struct ComponentPropertyRegistrator
        {
            void operator () (uint32 index, const InspInfo* typeInfo, const InspMember* member) const
            {
                DVASSERT(STYLE_SHEET_PROPERTY_TABLE[index].owner == ePropertyOwner::COMPONENT || STYLE_SHEET_PROPERTY_TABLE[index].owner == ePropertyOwner::UNKNOWN);
                STYLE_SHEET_PROPERTY_TABLE[index].owner = ePropertyOwner::COMPONENT;
                STYLE_SHEET_PROPERTY_TABLE[index].targetComponents.push_back(std::make_pair(index, member));
            }

            uint32 componentType;
        };

        struct BackgroundPropertyRegistrator
        {
            void operator () (uint32 index, const InspInfo* typeInfo, const InspMember* member) const
            {
                DVASSERT(
                    STYLE_SHEET_PROPERTY_TABLE[index].owner == ePropertyOwner::UNKNOWN ||
                    (STYLE_SHEET_PROPERTY_TABLE[index].owner == ePropertyOwner::BACKGROUND && STYLE_SHEET_PROPERTY_TABLE[index].inspMember == member));
                STYLE_SHEET_PROPERTY_TABLE[index].owner = ePropertyOwner::BACKGROUND;
                STYLE_SHEET_PROPERTY_TABLE[index].inspMember = member;
                STYLE_SHEET_PROPERTY_TABLE[index].typeInfo = typeInfo;
            }
        };

        struct ControlPropertyRegistrator
        {
            void operator () (uint32 index, const InspInfo* typeInfo, const InspMember* member) const
            {
                DVASSERT(
                    STYLE_SHEET_PROPERTY_TABLE[index].owner == ePropertyOwner::UNKNOWN || 
                    (STYLE_SHEET_PROPERTY_TABLE[index].owner == ePropertyOwner::CONTROL && STYLE_SHEET_PROPERTY_TABLE[index].inspMember == member));
                STYLE_SHEET_PROPERTY_TABLE[index].owner = ePropertyOwner::CONTROL;
                STYLE_SHEET_PROPERTY_TABLE[index].inspMember = member;
                STYLE_SHEET_PROPERTY_TABLE[index].typeInfo = typeInfo;
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
                    callback(iter->second, typeInfo, member);
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
    }

    void InitializeStyleSheetPropertyTable()
    {
        for (uint32_t i = 0; i < COUNT_OF(STYLE_SHEET_PROPERTY_TABLE); ++i)
        {
            propertyNameToIndexMap[STYLE_SHEET_PROPERTY_TABLE[i].name] = i;
        }

        ProcessComponentIntrospection< UIFakeComponent >();
        ProcessComponentIntrospection< UIFakeMultiComponent >();
        ProcessControlIntrospection< UIControl >();
        ProcessControlIntrospection< UIStaticText >();
        ProcessObjectIntrospection(UIControlBackground::TypeInfo(), BackgroundPropertyRegistrator());
    }

    uint32 GetStyleSheetPropertyIndex(const FastName& name)
    {
        const auto& iter = propertyNameToIndexMap.find(name);

        DVASSERT(iter != propertyNameToIndexMap.end());

        return iter->second;
    }

    bool IsValidStyleSheetPropertyIndex(const FastName& name)
    {
        return propertyNameToIndexMap.find(name) != propertyNameToIndexMap.end();
    }

    const UIStyleSheetPropertyDescriptor& GetStyleSheetPropertyByIndex(uint32 index)
    {
        return STYLE_SHEET_PROPERTY_TABLE[index];
    }
}