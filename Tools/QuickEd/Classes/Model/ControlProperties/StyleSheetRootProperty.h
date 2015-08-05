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


#ifndef __QUICKED_STYLE_SHEETS_ROOT_PROPERTY_H__
#define __QUICKED_STYLE_SHEETS_ROOT_PROPERTY_H__

#include "AbstractProperty.h"
#include "SectionProperty.h"
#include "StyleSheetProperty.h"
#include "StyleSheetSelectorProperty.h"

#include "UI/Styles/UIStyleSheetSelectorChain.h"

class PropertyListener;
class ValueProperty;

class StyleSheetNode;

namespace DAVA
{
    class UIControl;
    class UIStyleSheetPropertyTable;
    class UIStyleSheet;
}

class StyleSheetPropertiesSection : public SectionProperty<StyleSheetProperty>
{
public:
    StyleSheetPropertiesSection(const DAVA::String &name) : SectionProperty<StyleSheetProperty>(name) { }
};

class StyleSheetSelectorsSection : public SectionProperty<StyleSheetSelectorProperty>
{
public:
    StyleSheetSelectorsSection(const DAVA::String &name) : SectionProperty<StyleSheetSelectorProperty>(name) { }
};

class StyleSheetRootProperty : public AbstractProperty
{
public:
    StyleSheetRootProperty(StyleSheetNode *styleSheet, const DAVA::Vector<DAVA::UIStyleSheetSelectorChain> &selectorChains, const DAVA::Vector<DAVA::UIStyleSheetProperty> &properties);
protected:
    virtual ~StyleSheetRootProperty();
    
public:
    int GetCount() const override;
    AbstractProperty *GetProperty(int index) const override;

    void Accept(PropertyVisitor *visitor) override;
    bool IsReadOnly() const override;
    
    const DAVA::String &GetName() const override;
    ePropertyType GetType() const override;

    void AddListener(PropertyListener *listener);
    void RemoveListener(PropertyListener *listener);
    
    void SetProperty(AbstractProperty *property, const DAVA::VariantType &newValue);
    bool CanAddProperty(DAVA::uint32 propertyIndex) const;
    bool CanRemoveProperty(DAVA::uint32 propertyIndex) const;
    void AddProperty(StyleSheetProperty *property);
    void RemoveProperty(StyleSheetProperty *property);
    bool CanAddSelector() const;
    bool CanRemoveSelector() const;
    void InsertSelector(StyleSheetSelectorProperty *property, int index);
    void RemoveSelector(StyleSheetSelectorProperty *property);
    
    StyleSheetSelectorsSection *GetSelectors() const;
    StyleSheetPropertiesSection *GetPropertiesSection() const;

    StyleSheetProperty *FindPropertyByPropertyIndex(DAVA::uint32 index) const;
    StyleSheetSelectorProperty *GetSelectorAtIndex(DAVA::int32 index) const;

    DAVA::String GetSelectorsAsString() const;
    
    DAVA::Vector<DAVA::UIStyleSheet*> CollectStyleSheets();

    DAVA::Vector<DAVA::UIStyleSheetSelectorChain> CollectStyleSheetSelectors() const;
    DAVA::Vector<DAVA::UIStyleSheetProperty> CollectStyleSheetProperties() const;
    
    DAVA::UIStyleSheetPropertyTable *GetStyleSheetPropertyTable() const;

private:
    void UpdateStyleSheetPropertyTable();
    
private:
    enum eSection
    {
        SECTION_SELECTORS = 0,
        SECTION_PROPERTIES = 1,
        SECTION_COUNT = 2
    };
    
private:
    StyleSheetNode *styleSheet = nullptr;
    DAVA::Vector<PropertyListener*> listeners;
    
    StyleSheetSelectorsSection *selectors = nullptr;
    StyleSheetPropertiesSection *propertiesSection = nullptr;
    
    DAVA::UIStyleSheetPropertyTable *propertyTable = nullptr;
};

#endif // __QUICKED_STYLE_SHEETS_ROOT_PROPERTY_H__
