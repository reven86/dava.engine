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

#include "UI/Styles/UIStyleSheet.h"

namespace DAVA
{
    void UIStyleSheetPropertyTable::SetProperties(const Vector<std::pair<uint32, VariantType>>& newProperties)
    {
        properties = newProperties;
        for (const auto& prop : properties)
            propertiesSet.set(prop.first);

        std::sort(properties.begin(), properties.end(),
            [](const std::pair<uint32, VariantType>& first, const std::pair<uint32, VariantType>& second) {
            return first.first < second.first;
        });
    }

    const Vector<std::pair<uint32, VariantType>>& UIStyleSheetPropertyTable::GetProperties() const
    {
        return properties;
    }

    const UIStyleSheetPropertySet& UIStyleSheetPropertyTable::GetPropertySet() const
    {
        return propertiesSet;
    }

    UIStyleSheet::~UIStyleSheet()
    {

    }

    UIStyleSheet::UIStyleSheet() :
        score(0)
    {

    }

    int32 UIStyleSheet::GetScore() const
    {
        return score;
    }

    const UIStyleSheetPropertyTable* UIStyleSheet::GetPropertyTable() const
    {
        return properties.Get();
    }

    const Vector< UIStyleSheetSelector >& UIStyleSheet::GetSelectorChain() const
    {
        return selectorChain;
    }

    void UIStyleSheet::SetPropertyTable(UIStyleSheetPropertyTable* newProperties)
    {
        properties = newProperties;
    }

    void UIStyleSheet::SetSelectorChain(const Vector<UIStyleSheetSelector>& newSelectorChain)
    {
        selectorChain = newSelectorChain;

        RecalculateScore();
    }

    void UIStyleSheet::RecalculateScore()
    {
        score = 0;
        for (const UIStyleSheetSelector& selector : selectorChain)
        {
            score += 100000 + selector.classes.size();
            if (selector.name.IsValid())
                score += 100;
            if (!selector.controlClassName.empty())
                score += 100;
        }
    }
}
