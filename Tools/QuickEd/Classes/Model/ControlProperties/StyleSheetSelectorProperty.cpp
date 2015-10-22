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


#include "StyleSheetSelectorProperty.h"

#include "PropertyVisitor.h"
#include "../PackageHierarchy/StyleSheetNode.h"

#include "UI/Styles/UIStyleSheet.h"
#include "Utils/Utils.h"

using namespace DAVA;

StyleSheetSelectorProperty::StyleSheetSelectorProperty(const UIStyleSheetSelectorChain &chain)
    : ValueProperty("Selector")
{
    styleSheet = new UIStyleSheet();
    styleSheet->SetSelectorChain(chain);

    SetOverridden(true);
    value = chain.ToString();
}

StyleSheetSelectorProperty::~StyleSheetSelectorProperty()
{
    SafeRelease(styleSheet);
}

uint32 StyleSheetSelectorProperty::GetCount() const
{
    return 0;
}

AbstractProperty *StyleSheetSelectorProperty::GetProperty(int index) const
{
    return nullptr;
}

void StyleSheetSelectorProperty::Accept(PropertyVisitor *visitor)
{
    visitor->VisitStyleSheetSelectorProperty(this);
}

AbstractProperty::ePropertyType StyleSheetSelectorProperty::GetType() const
{
    return TYPE_VARIANT;
}

uint32 StyleSheetSelectorProperty::GetFlags() const
{
    return EF_CAN_REMOVE;
}

VariantType StyleSheetSelectorProperty::GetValue() const
{
    return VariantType(value);
}

void StyleSheetSelectorProperty::ApplyValue(const DAVA::VariantType &aValue)
{
    Vector<String> selectorList;
    Split(aValue.AsString(), ",", selectorList);

    if (!selectorList.empty())
    {
        styleSheet->SetSelectorChain(UIStyleSheetSelectorChain(selectorList.front()));
    }
    else
    {
        styleSheet->SetSelectorChain(UIStyleSheetSelectorChain(""));
    }
    value = styleSheet->GetSelectorChain().ToString();
}

const DAVA::UIStyleSheetSelectorChain &StyleSheetSelectorProperty::GetSelectorChain() const
{
    return styleSheet->GetSelectorChain();
}

const DAVA::String &StyleSheetSelectorProperty::GetSelectorChainString() const
{
    return value;
}

UIStyleSheet *StyleSheetSelectorProperty::GetStyleSheet() const
{
    return styleSheet;
}

void StyleSheetSelectorProperty::SetStyleSheetPropertyTable(DAVA::UIStyleSheetPropertyTable *propertyTable)
{
    styleSheet->SetPropertyTable(propertyTable);
}
