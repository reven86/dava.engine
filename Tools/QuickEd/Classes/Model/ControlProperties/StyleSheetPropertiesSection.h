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


#ifndef __QUICKED_STYLE_SHEET_PROPERTIES_SECTION_H__
#define __QUICKED_STYLE_SHEET_PROPERTIES_SECTION_H__

#include "Model/ControlProperties/AbstractProperty.h"

#include "UI/Styles/UIStyleSheetStructs.h"

class StyleSheetProperty;

class StyleSheetNode;

namespace DAVA
{
    class UIControl;
}

class StyleSheetPropertiesSection : public AbstractProperty
{
public:
    StyleSheetPropertiesSection(StyleSheetNode *styleSheet, const DAVA::Vector<DAVA::UIStyleSheetProperty> &properties);
protected:
    virtual ~StyleSheetPropertiesSection();
    
public:
    int GetCount() const override;
    AbstractProperty *GetProperty(int index) const override;
    
    void Accept(PropertyVisitor *visitor) override;
    bool IsReadOnly() const override;
    
    const DAVA::String &GetName() const override;
    ePropertyType GetType() const override;
    
    bool CanAddProperty(DAVA::uint32 propertyIndex) const;
    bool CanRemoveProperty(DAVA::uint32 propertyIndex) const;
    void AddProperty(StyleSheetProperty *property);
    void RemoveProperty(StyleSheetProperty *property);
    StyleSheetProperty *FindPropertyByIndex(DAVA::uint32 index) const;

private:
    bool HasProperty(DAVA::uint32 propertyIndex) const;
    
private:
    StyleSheetNode *styleSheet; // weak
    DAVA::Vector<StyleSheetProperty*> styleSheetProperties;
};

#endif // __QUICKED_STYLE_SHEET_PROPERTIES_SECTION_H__
