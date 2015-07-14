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

#include "StyleSheetSelectorsSection.h"
#include "StyleSheetPropertiesSection.h"
#include "StyleSheetProperty.h"

#include "PropertyVisitor.h"
#include "PropertyListener.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "UI/Styles/UIStyleSheet.h"

using namespace DAVA;

StyleSheetRootProperty::StyleSheetRootProperty(StyleSheetNode *aStyleSheet, const DAVA::Vector<DAVA::UIStyleSheetSelectorChain> &selectorChains, const DAVA::Vector<DAVA::UIStyleSheetProperty> &properties)
    : styleSheet(aStyleSheet) // weak
{
    selectors = new StyleSheetSelectorsSection(styleSheet, selectorChains);
    selectors->SetParent(this);
    
    propertiesSection = new StyleSheetPropertiesSection(styleSheet, properties);
    propertiesSection->SetParent(this);
}

StyleSheetRootProperty::~StyleSheetRootProperty()
{
    styleSheet = nullptr; // weak
    
    selectors->SetParent(nullptr);
    SafeRelease(selectors);
    
    propertiesSection->SetParent(nullptr);
    SafeRelease(propertiesSection);

}

int StyleSheetRootProperty::GetCount() const
{
    return 2;
}

AbstractProperty *StyleSheetRootProperty::GetProperty(int index) const
{
    switch (index)
    {
        case 0:
            return selectors;
        case 1:
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
    
    for (PropertyListener *listener : listeners)
        listener->PropertyChanged(property);
}

bool StyleSheetRootProperty::CanAddProperty(DAVA::uint32 propertyIndex) const
{
    return !IsReadOnly() && propertiesSection->CanAddProperty(propertyIndex);
}

void StyleSheetRootProperty::AddProperty(StyleSheetProperty *property)
{
    for (PropertyListener *listener : listeners)
        listener->StylePropertyWillBeAdded(propertiesSection, property, property->GetPropertyIndex());
    
    propertiesSection->AddProperty(property);

    for (PropertyListener *listener : listeners)
        listener->StylePropertyWasAdded(propertiesSection, property, property->GetPropertyIndex());
}

void StyleSheetRootProperty::RemoveProperty(StyleSheetProperty *property)
{
    uint32 index = property->GetPropertyIndex();
    for (PropertyListener *listener : listeners)
        listener->StylePropertyWillBeRemoved(propertiesSection, property, index);
    
    propertiesSection->RemoveProperty(property);
    
    for (PropertyListener *listener : listeners)
        listener->StylePropertyWasRemoved(propertiesSection, property, index);
}

StyleSheetSelectorsSection *StyleSheetRootProperty::GetSelectors() const
{
    return selectors;
}

StyleSheetPropertiesSection *StyleSheetRootProperty::GetPropertiesSection() const
{
    return propertiesSection;
}
