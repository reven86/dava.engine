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

#include "StyleSheetRootProperty.h"

#include "SectionProperty.h"
#include "StyleSheetProperty.h"
#include "StyleSheetSelectorProperty.h"

#include "PropertyVisitor.h"
#include "PropertyListener.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "UI/Styles/UIStyleSheet.h"

using namespace DAVA;

StyleSheetRootProperty::StyleSheetRootProperty(StyleSheetNode *aStyleSheet, const DAVA::Vector<DAVA::UIStyleSheetSelectorChain> &selectorChains, const DAVA::Vector<DAVA::UIStyleSheetProperty> &properties)
    : styleSheet(aStyleSheet) // weak
{
    propertyTable = new UIStyleSheetPropertyTable();

    selectors = new StyleSheetSelectorsSection("Selectors");
    selectors->SetParent(this);
    for (const UIStyleSheetSelectorChain &chain : selectorChains)
    {
        ScopedPtr<StyleSheetSelectorProperty> selector(new StyleSheetSelectorProperty(chain));
        selector->SetStyleSheetPropertyTable(propertyTable);
        selectors->AddProperty(selector);
    }
    
    propertiesSection = new StyleSheetPropertiesSection("Properties");
    propertiesSection->SetParent(this);
    for (const UIStyleSheetProperty &p : properties)
    {
        ScopedPtr<StyleSheetProperty> prop(new StyleSheetProperty(p));
        propertiesSection->AddProperty(prop);
    }
    
    UpdateStyleSheetPropertyTable();
}

StyleSheetRootProperty::~StyleSheetRootProperty()
{
    styleSheet = nullptr; // weak
    
    selectors->SetParent(nullptr);
    SafeRelease(selectors);
    
    propertiesSection->SetParent(nullptr);
    SafeRelease(propertiesSection);

    SafeRelease(propertyTable);
}

uint32 StyleSheetRootProperty::GetCount() const
{
    return SECTION_COUNT;
}

AbstractProperty *StyleSheetRootProperty::GetProperty(int index) const
{
    switch (index)
    {
        case SECTION_SELECTORS:
            return selectors;
        case SECTION_PROPERTIES:
            return propertiesSection;
    }
    DVASSERT(false);
    return nullptr;
}

void StyleSheetRootProperty::Accept(PropertyVisitor *visitor)
{
    visitor->VisitStyleSheetRoot(this);
}

bool StyleSheetRootProperty::IsReadOnly() const
{
    return styleSheet->IsReadOnly();
}

const DAVA::String &StyleSheetRootProperty::GetName() const
{
    static String rootName = "Style Sheets Properties";
    return rootName;
}

AbstractProperty::ePropertyType StyleSheetRootProperty::GetType() const
{
    return TYPE_HEADER;
}

void StyleSheetRootProperty::AddListener(PropertyListener *listener)
{
    listeners.push_back(listener);
}

void StyleSheetRootProperty::RemoveListener(PropertyListener *listener)
{
    auto it = std::find(listeners.begin(), listeners.end(), listener);
    if (it != listeners.end())
    {
        listeners.erase(it);
    }
    else
    {
        DVASSERT(false);
    }
}

void StyleSheetRootProperty::SetProperty(AbstractProperty *property, const DAVA::VariantType &newValue)
{
    property->SetValue(newValue);
    UpdateStyleSheetPropertyTable();

    for (PropertyListener *listener : listeners)
        listener->PropertyChanged(property);
}

bool StyleSheetRootProperty::CanAddProperty(DAVA::uint32 propertyIndex) const
{
    return !IsReadOnly() && FindPropertyByPropertyIndex(propertyIndex) == nullptr;
}

bool StyleSheetRootProperty::CanRemoveProperty(DAVA::uint32 propertyIndex) const
{
    return !IsReadOnly() && FindPropertyByPropertyIndex(propertyIndex) != nullptr;
}

void StyleSheetRootProperty::AddProperty(StyleSheetProperty *property)
{
    if (CanAddProperty(property->GetPropertyIndex()) && property->GetParent() == nullptr)
    {
        for (PropertyListener *listener : listeners)
            listener->StylePropertyWillBeAdded(propertiesSection, property, property->GetPropertyIndex());

        uint32 index = 0;
        while (index < propertiesSection->GetCount())
        {
            StyleSheetProperty *p = propertiesSection->GetProperty(index);
            if (p->GetPropertyIndex() > property->GetPropertyIndex())
                break;
            index++;
        }
        propertiesSection->InsertProperty(property, index);
        UpdateStyleSheetPropertyTable();

        for (PropertyListener *listener : listeners)
            listener->StylePropertyWasAdded(propertiesSection, property, property->GetPropertyIndex());
    }
}

void StyleSheetRootProperty::RemoveProperty(StyleSheetProperty *property)
{
    uint32 index = propertiesSection->GetIndex(property);
    if (!IsReadOnly() && index != -1)
    {
        for (PropertyListener *listener : listeners)
            listener->StylePropertyWillBeRemoved(propertiesSection, property, index);
        
        propertiesSection->RemoveProperty(property);
        UpdateStyleSheetPropertyTable();

        for (PropertyListener *listener : listeners)
            listener->StylePropertyWasRemoved(propertiesSection, property, index);
    }
}

bool StyleSheetRootProperty::CanAddSelector() const
{
    return !IsReadOnly();
}

bool StyleSheetRootProperty::CanRemoveSelector() const
{
    return !IsReadOnly() && selectors->GetCount() > 1;
}

void StyleSheetRootProperty::InsertSelector(StyleSheetSelectorProperty *property, int index)
{
    if (CanAddSelector() && selectors->GetIndex(property) == -1)
    {
        for (PropertyListener *listener : listeners)
            listener->StyleSelectorWillBeAdded(selectors, property, index);

        selectors->InsertProperty(property, index);
        property->SetStyleSheetPropertyTable(propertyTable);

        for (PropertyListener *listener : listeners)
            listener->StyleSelectorWasAdded(selectors, property, index);
    }
    else
    {
        DVASSERT(false);
    }
}

void StyleSheetRootProperty::RemoveSelector(StyleSheetSelectorProperty *property)
{
    int32 index = selectors->GetIndex(property);
    if (CanRemoveSelector() && index != -1)
    {
        for (PropertyListener *listener : listeners)
            listener->StyleSelectorWillBeRemoved(selectors, property, index);
        
        selectors->RemoveProperty(property);
        property->SetStyleSheetPropertyTable(nullptr);

        for (PropertyListener *listener : listeners)
            listener->StyleSelectorWasRemoved(selectors, property, index);
    }
    else
    {
        DVASSERT(false);
    }
}

StyleSheetSelectorsSection *StyleSheetRootProperty::GetSelectors() const
{
    return selectors;
}

StyleSheetPropertiesSection *StyleSheetRootProperty::GetPropertiesSection() const
{
    return propertiesSection;
}

StyleSheetProperty *StyleSheetRootProperty::FindPropertyByPropertyIndex(DAVA::uint32 propertyIndex) const
{
    for (uint32 i = 0; i < propertiesSection->GetCount(); i++)
    {
        StyleSheetProperty *p = static_cast<StyleSheetProperty*>(propertiesSection->GetProperty(i));
        if (p->GetPropertyIndex() == propertyIndex)
            return p;
    }
    return nullptr;
}

StyleSheetSelectorProperty *StyleSheetRootProperty::GetSelectorAtIndex(DAVA::int32 index) const
{
    if (0 <= index && index < (int32)selectors->GetCount())
    {
        return static_cast<StyleSheetSelectorProperty*>(selectors->GetProperty(index));
    }
    else
    {
        DVASSERT(false);
        return nullptr;
    }
}

String StyleSheetRootProperty::GetSelectorsAsString() const
{
    StringStream stream;
    for (uint32 i = 0; i < selectors->GetCount(); i++)
    {
        if (i > 0)
            stream << ", ";
        stream << selectors->GetProperty(i)->GetValue().AsString();
    }
    return stream.str();
}

Vector<UIStyleSheet*> StyleSheetRootProperty::CollectStyleSheets()
{
    Vector<UIStyleSheet*> result;
    for (uint32 i = 0; i < selectors->GetCount(); i++)
    {
        result.push_back(selectors->GetProperty(i)->GetStyleSheet());
    }
    return result;
}

DAVA::Vector<DAVA::UIStyleSheetSelectorChain> StyleSheetRootProperty::CollectStyleSheetSelectors() const
{
    Vector<UIStyleSheetSelectorChain> result;
    for (uint32 i = 0; i < selectors->GetCount(); i++)
    {
        result.push_back(selectors->GetProperty(i)->GetSelectorChain());
    }
    return result;
}

DAVA::Vector<DAVA::UIStyleSheetProperty> StyleSheetRootProperty::CollectStyleSheetProperties() const
{
    Vector<UIStyleSheetProperty> properties;
    for (uint32 i = 0; i < propertiesSection->GetCount(); i++)
    {
        properties.push_back(propertiesSection->GetProperty(i)->GetProperty());
    }
    return properties;
}

DAVA::UIStyleSheetPropertyTable *StyleSheetRootProperty::GetStyleSheetPropertyTable() const
{
    return propertyTable;
}

void StyleSheetRootProperty::UpdateStyleSheetPropertyTable()
{
    propertyTable->SetProperties(CollectStyleSheetProperties());
}
