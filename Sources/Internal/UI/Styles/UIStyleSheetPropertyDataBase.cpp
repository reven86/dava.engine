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

#include "UI/Styles/UIStyleSheetPropertyDataBase.h"
#include "UI/UIControl.h"
#include "UI/UIButton.h"
#include "UI/UIStaticText.h"
#include "UI/UITextField.h"
#include "UI/UIControlBackground.h"
#include "UI/Layouts/UIAnchorComponent.h"
#include "UI/Layouts/UILinearLayoutComponent.h"
#include "UI/Layouts/UISizePolicyComponent.h"

namespace DAVA
{

UIStyleSheetPropertyDataBase::UIStyleSheetPropertyDataBase() :
    properties({ {
        { FastName("angle"), VariantType(0.0f) },
        { FastName("scale"), VariantType(Vector2(1.0f, 1.0f)) },
        { FastName("visible"), VariantType(true) },

        { FastName("drawType"), VariantType(UIControlBackground::DRAW_ALIGNED) },
        { FastName("sprite"), VariantType(FilePath()) },
        { FastName("frame"), VariantType(0) },
        { FastName("color"), VariantType(Color::White) },
        { FastName("colorInherit"), VariantType(UIControlBackground::COLOR_IGNORE_PARENT) },
        { FastName("align"), VariantType(ALIGN_HCENTER | ALIGN_VCENTER) },
        { FastName("leftRightStretchCap"), VariantType(0.0f) },
        { FastName("topBottomStretchCap"), VariantType(0.0f) },

        { FastName("font"), VariantType(String("")) },
        { FastName("textColor"), VariantType(Color::White) },
        { FastName("textcolorInheritType"), VariantType(UIControlBackground::COLOR_MULTIPLY_ON_PARENT) },
        { FastName("shadowoffset"), VariantType(Vector2(0.0f, 0.0f)) },
        { FastName("shadowcolor"), VariantType(Color::White) },
        { FastName("textalign"), VariantType(ALIGN_HCENTER | ALIGN_VCENTER) },
    
        { FastName("leftAnchorEnabled"), VariantType(false) },
        { FastName("leftAnchor"), VariantType(0.0f) },

        { FastName("rightAnchorEnabled"), VariantType(false) },
        { FastName("rightAnchor"), VariantType(0.0f) },

        { FastName("bottomAnchorEnabled"), VariantType(false) },
        { FastName("bottomAnchor"), VariantType(0.0f) },

        { FastName("topAnchorEnabled"), VariantType(false) },
        { FastName("topAnchor"), VariantType(0.0f) },

        { FastName("hCenterAnchorEnabled"), VariantType(false) },
        { FastName("hCenterAnchor"), VariantType(0.0f) },

        { FastName("vCenterAnchorEnabled"), VariantType(false) },
        { FastName("vCenterAnchor"), VariantType(0.0f) },
        
    } })
{

    for (uint32_t i = 0; i < STYLE_SHEET_PROPERTY_COUNT; ++i)
    {
        propertyNameToIndexMap[properties[i].name] = i;
    }

    ProcessComponentIntrospection<UIAnchorComponent>();
    ProcessComponentIntrospection<UILinearLayoutComponent>();
    ProcessComponentIntrospection<UISizePolicyComponent>();
    ProcessControlIntrospection<UIControl>();
    ProcessControlIntrospection<UIButton>();
    ProcessControlIntrospection<UIStaticText>();
    ProcessControlIntrospection<UITextField>();
    ProcessObjectIntrospection(UIControlBackground::TypeInfo(), BackgroundPropertyRegistrator());
}

uint32 UIStyleSheetPropertyDataBase::GetStyleSheetPropertyIndex(const FastName& name) const
{
    const auto& iter = propertyNameToIndexMap.find(name);

    DVASSERT(iter != propertyNameToIndexMap.end());

    return iter->second;
}

bool UIStyleSheetPropertyDataBase::IsValidStyleSheetProperty(const FastName& name) const
{
    return propertyNameToIndexMap.find(name) != propertyNameToIndexMap.end();
}

const UIStyleSheetPropertyDescriptor& UIStyleSheetPropertyDataBase::GetStyleSheetPropertyByIndex(uint32 index) const
{
    return properties[index];
}
    
template < typename CallbackType >
void UIStyleSheetPropertyDataBase::ProcessObjectIntrospection(const InspInfo* typeInfo, const CallbackType& callback)
{
    const InspInfo *baseInfo = typeInfo->BaseInfo();
    if (baseInfo)
        ProcessObjectIntrospection(baseInfo, callback);
        
    for (int32 i = 0; i < typeInfo->MembersCount(); i++)
    {
        const InspMember *member = typeInfo->Member(i);
            
        const auto& iter = propertyNameToIndexMap.find(member->Name());
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
void UIStyleSheetPropertyDataBase::ProcessComponentIntrospection()
{
    ProcessObjectIntrospection(ComponentType::TypeInfo(), ComponentPropertyRegistrator{ ComponentType::C_TYPE });
}
    
template < typename ControlType >
void UIStyleSheetPropertyDataBase::ProcessControlIntrospection()
{
    ProcessObjectIntrospection(ControlType::TypeInfo(), ControlPropertyRegistrator());
}
    
UIStyleSheetPropertyTargetMember UIStyleSheetPropertyDataBase::ComponentPropertyRegistrator::operator () (uint32 index, const InspInfo* typeInfo, const InspMember* member) const
{
    return UIStyleSheetPropertyTargetMember{ ePropertyOwner::COMPONENT, componentType, typeInfo, member };
}
    
UIStyleSheetPropertyTargetMember UIStyleSheetPropertyDataBase::BackgroundPropertyRegistrator::operator () (uint32 index, const InspInfo* typeInfo, const InspMember* member) const
{
    return UIStyleSheetPropertyTargetMember{ ePropertyOwner::BACKGROUND, 0, typeInfo, member };
}
    
UIStyleSheetPropertyTargetMember UIStyleSheetPropertyDataBase::ControlPropertyRegistrator::operator () (uint32 index, const InspInfo* typeInfo, const InspMember* member) const
{
    return UIStyleSheetPropertyTargetMember{ ePropertyOwner::CONTROL, 0, typeInfo, member };
}

}
