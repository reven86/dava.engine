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


#include "StyleSheetPropertiesSection.h"

#include "StyleSheetProperty.h"
#include "PropertyVisitor.h"
#include "../PackageHierarchy/StyleSheetNode.h"
#include "UI/Styles/UIStyleSheet.h"

using namespace DAVA;

StyleSheetPropertiesSection::StyleSheetPropertiesSection(StyleSheetNode *aStyleSheet, const DAVA::Vector<DAVA::UIStyleSheetProperty> &properties)
    : styleSheet(aStyleSheet) // weak
{
    for (const UIStyleSheetProperty &p : properties)
    {
        StyleSheetProperty *prop = new StyleSheetProperty(styleSheet, p);
        prop->SetParent(this);
        styleSheetProperties.push_back(prop);
    }
}

StyleSheetPropertiesSection::~StyleSheetPropertiesSection()
{
    styleSheet = nullptr; //weak
    for (StyleSheetProperty *prop : styleSheetProperties)
        SafeRelease(prop);
    styleSheetProperties.clear();
}

int StyleSheetPropertiesSection::GetCount() const
{
    return static_cast<int>(styleSheetProperties.size());
}

AbstractProperty *StyleSheetPropertiesSection::GetProperty(int index) const
{
    return styleSheetProperties[index];
}

void StyleSheetPropertiesSection::Accept(PropertyVisitor *visitor)
{
    visitor->VisitStyleSheetPropertiesSection(this);
}

bool StyleSheetPropertiesSection::IsReadOnly() const
{
    return styleSheet->IsReadOnly();
}

const DAVA::String &StyleSheetPropertiesSection::GetName() const
{
    static String name = "Properties";
    return name;
}

AbstractProperty::ePropertyType StyleSheetPropertiesSection::GetType() const
{
    return TYPE_HEADER;
}

bool StyleSheetPropertiesSection::CanAddProperty(DAVA::uint32 propertyIndex) const
{
    return !HasProperty(propertyIndex);
}

bool StyleSheetPropertiesSection::CanRemoveProperty(DAVA::uint32 propertyIndex) const
{
    return HasProperty(propertyIndex);
}

void StyleSheetPropertiesSection::AddProperty(StyleSheetProperty *property)
{
    if (CanAddProperty(property->GetPropertyIndex()) && property->GetParent() == nullptr)
    {
        property->SetParent(this);
        styleSheetProperties.push_back(property);
        std::sort(styleSheetProperties.begin(), styleSheetProperties.end(), [] (StyleSheetProperty *p1, StyleSheetProperty *p2) {
            return p1->GetPropertyIndex() < p2->GetPropertyIndex();
        });
    }
}

void StyleSheetPropertiesSection::RemoveProperty(StyleSheetProperty *property)
{
    auto it = std::find(styleSheetProperties.begin(), styleSheetProperties.end(), property);
    if (it != styleSheetProperties.end())
    {
        DVASSERT((*it)->GetParent() == this);
        (*it)->SetParent(nullptr);
        (*it)->Release();
        styleSheetProperties.erase(it);
    }
    else
    {
        DVASSERT(false); 
    }
}

StyleSheetProperty *StyleSheetPropertiesSection::FindPropertyByIndex(DAVA::uint32 propertyIndex) const
{
    auto it = std::find_if(styleSheetProperties.begin(), styleSheetProperties.end(), [propertyIndex](const StyleSheetProperty *p) {
        return p->GetPropertyIndex() == propertyIndex;
    });
    
    return it == styleSheetProperties.end() ? nullptr : *it;
}

bool StyleSheetPropertiesSection::HasProperty(DAVA::uint32 propertyIndex) const
{
    return FindPropertyByIndex(propertyIndex) != nullptr;
}
