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


#ifndef __QUICKED_STYLE_SHEET_SELECTOR_PROPERTY_H__
#define __QUICKED_STYLE_SHEET_SELECTOR_PROPERTY_H__

#include "Model/ControlProperties/ValueProperty.h"
#include "UI/Styles/UIStyleSheetSelectorChain.h"

class ValueProperty;

class StyleSheetNode;

namespace DAVA
{
    class UIControl;
    class UIStyleSheet;
    class UIStyleSheetPropertyTable;
}

class StyleSheetSelectorProperty : public ValueProperty
{
public:
    StyleSheetSelectorProperty(const DAVA::UIStyleSheetSelectorChain &chain);
protected:
    virtual ~StyleSheetSelectorProperty();
    
public:
    int GetCount() const override;
    AbstractProperty *GetProperty(int index) const override;
    
    void Accept(PropertyVisitor *visitor) override;
    
    ePropertyType GetType() const override;
    DAVA::uint32 GetFlags() const override;

    DAVA::VariantType GetValue() const;
    void ApplyValue(const DAVA::VariantType &value);
    
    const DAVA::UIStyleSheetSelectorChain &GetSelectorChain() const;
    const DAVA::String &GetSelectorChainString() const;
    DAVA::UIStyleSheet *GetStyleSheet() const;
    
    void SetStyleSheetPropertyTable(DAVA::UIStyleSheetPropertyTable *propertyTable);
    
private:
    DAVA::UIStyleSheet *styleSheet = nullptr;
    DAVA::String value;
};

#endif // __QUICKED_STYLE_SHEET_SELECTOR_PROPERTY_H__
