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


#include "StyleSheetSelectorsSection.h"

#include "PropertyVisitor.h"
#include "StyleSheetSelectorProperty.h"
#include "../PackageHierarchy/StyleSheetNode.h"

#include "UI/Styles/UIStyleSheet.h"

using namespace DAVA;

StyleSheetSelectorsSection::StyleSheetSelectorsSection(StyleSheetNode *aStyleSheet, const Vector<UIStyleSheetSelectorChain> &selectorChains)
    : ValueProperty("Selectors")
    , styleSheet(aStyleSheet) // weak
{
    for (const UIStyleSheetSelectorChain &chain : selectorChains)
    {
        StyleSheetSelectorProperty *selector = new StyleSheetSelectorProperty(aStyleSheet, chain);
        selector->SetParent(this);
        selectors.push_back(selector);
    }
}

StyleSheetSelectorsSection::~StyleSheetSelectorsSection()
{
    styleSheet = nullptr; //weak
    
    for (StyleSheetSelectorProperty *selector : selectors)
    {
        selector->Release();
    }
}

int StyleSheetSelectorsSection::GetCount() const
{
    return static_cast<int>(selectors.size());
}

StyleSheetSelectorProperty *StyleSheetSelectorsSection::GetProperty(int index) const
{
    return selectors[index];
}

void StyleSheetSelectorsSection::Accept(PropertyVisitor *visitor)
{
    visitor->VisitStyleSheetSelectorsSection(this);
}

bool StyleSheetSelectorsSection::IsReadOnly() const
{
    return styleSheet->IsReadOnly();
}

AbstractProperty::ePropertyType StyleSheetSelectorsSection::GetType() const
{
    return TYPE_HEADER;
}

DAVA::VariantType StyleSheetSelectorsSection::GetValue() const
{
    return VariantType();
}

void StyleSheetSelectorsSection::ApplyValue(const DAVA::VariantType &value)
{
    
}
