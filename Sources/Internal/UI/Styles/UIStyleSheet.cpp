#include "UI/Styles/UIStyleSheet.h"
#include "UI/UIControl.h"

namespace DAVA
{
UIStyleSheet::~UIStyleSheet()
{
}

UIStyleSheet::UIStyleSheet()
    :
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

const UIStyleSheetSelectorChain& UIStyleSheet::GetSelectorChain() const
{
    return selectorChain;
}

void UIStyleSheet::SetPropertyTable(UIStyleSheetPropertyTable* newProperties)
{
    properties = newProperties;
}

void UIStyleSheet::SetSelectorChain(const UIStyleSheetSelectorChain& newSelectorChain)
{
    selectorChain = newSelectorChain;

    RecalculateScore();
}

void UIStyleSheet::RecalculateScore()
{
    score = 0;
    for (const UIStyleSheetSelector& selector : selectorChain)
    {
        score += static_cast<uint32>(100000 + selector.classes.size());
        if (selector.name.IsValid())
            score += 100;
        if (!selector.className.empty())
            score += 100;
        for (int32 stateIndex = 0; stateIndex < UIControl::STATE_COUNT; ++stateIndex)
            if (selector.stateMask & (1 << stateIndex))
                score += 1;
    }
}
}
