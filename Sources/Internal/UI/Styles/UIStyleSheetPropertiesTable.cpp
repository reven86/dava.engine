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

#include "UI/Styles/UIStyleSheetPropertiesTable.h"
#include "UI/UIControl.h"
#include "UI/UIButton.h"
#include "UI/UIStaticText.h"
#include "UI/UITextField.h"
#include "UI/UIControlBackground.h"
#include "UI/Components/UIFakeComponent.h"
#include "UI/Components/UIFakeMultiComponent.h"

namespace DAVA
{
    UIStyleSheetPropertyDataBase::UIStyleSheetPropertyDataBase() :
        properties({ {
            { FastName("angle"), VariantType(0.0f) },
            { FastName("scale"), VariantType(Vector2(1.0f, 1.0f)) },
            { FastName("visible"), VariantType(true) },
            { FastName("leftAlignEnabled"), VariantType(false) },
            { FastName("rightAlignEnabled"), VariantType(false) },
            { FastName("bottomAlignEnabled"), VariantType(false) },
            { FastName("topAlignEnabled"), VariantType(false) },
            { FastName("hcenterAlignEnabled"), VariantType(false) },
            { FastName("vcenterAlignEnabled"), VariantType(false) },
            { FastName("leftAlign"), VariantType(0.0f) },
            { FastName("rightAlign"), VariantType(0.0f) },
            { FastName("bottomAlign"), VariantType(0.0f) },
            { FastName("topAlign"), VariantType(0.0f) },
            { FastName("hcenterAlign"), VariantType(0.0f) },
            { FastName("vcenterAlign"), VariantType(0.0f) },

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
            { FastName("textalign"), VariantType(ALIGN_HCENTER | ALIGN_VCENTER) } } })
    {

        for (uint32_t i = 0; i < STYLE_SHEET_PROPERTY_COUNT; ++i)
        {
            propertyNameToIndexMap[properties[i].name] = i;
        }

        ProcessComponentIntrospection<UIFakeComponent>();
        ProcessComponentIntrospection<UIFakeMultiComponent>();
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
}
