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

#include "UI/Styles/UIStyleSheetSelectorChain.h"
#include "UI/UIControl.h"

namespace DAVA
{

namespace
{

class SelectorParser
{
public:
    enum SelectorParserState
    {
        SELECTOR_STATE_CONTROL_CLASS_NAME,
        SELECTOR_STATE_CLASS,
        SELECTOR_STATE_PSEUDO_CLASS,
        SELECTOR_STATE_NAME,
        SELECTOR_STATE_NONE,
    };

    SelectorParser(Vector<UIStyleSheetSelector>& aSelectorChain) :
        selectorChain(aSelectorChain)
    {

    }

    void Parse(const char* selectorStr)
    {
        currentSelector = UIStyleSheetSelector();
        currentToken = "";
        state = SELECTOR_STATE_CONTROL_CLASS_NAME;

        while (*selectorStr && *selectorStr == ' ') ++selectorStr;

        while (*selectorStr)
        {
            if ((*selectorStr) == ' ')
            {
                FinishProcessingCurrentSelector();
                while (*(selectorStr + 1) == ' ') ++selectorStr;
            }
            else if ((*selectorStr) == '?')
            {
                FinishProcessingCurrentSelector();
                while (*(selectorStr + 1) == ' ') ++selectorStr;
                selectorChain.push_back(UIStyleSheetSelector());
            }
            else if ((*selectorStr) == '.')
            {
                GoToState(SELECTOR_STATE_CLASS);
            }
            else if ((*selectorStr) == ':')
            {
                GoToState(SELECTOR_STATE_PSEUDO_CLASS);
            }
            else if ((*selectorStr) == '#')
            {
                GoToState(SELECTOR_STATE_NAME);
            }
            else
            {
                currentToken += *selectorStr;
            }

            ++selectorStr;
        }
        FinishProcessingCurrentSelector();
    }
private:
    String currentToken;
    SelectorParserState state;
    UIStyleSheetSelector currentSelector;

    Vector<UIStyleSheetSelector>& selectorChain;

    void FinishProcessingCurrentSelector()
    {
        GoToState(SELECTOR_STATE_CONTROL_CLASS_NAME);
        if (!currentSelector.className.empty() || currentSelector.name.IsValid() || !currentSelector.classes.empty())
            selectorChain.push_back(currentSelector);
        currentSelector = UIStyleSheetSelector();
    }

    void GoToState(SelectorParserState newState)
    {
        if (currentToken != "")
        {
            if (state == SELECTOR_STATE_CONTROL_CLASS_NAME)
            {
                currentSelector.className = currentToken;
            }
            else if (state == SELECTOR_STATE_NAME)
            {
                currentSelector.name = FastName(currentToken);
            }
            else if (state == SELECTOR_STATE_CLASS)
            {
                currentSelector.classes.push_back(FastName(currentToken));
            }
            else if (state == SELECTOR_STATE_PSEUDO_CLASS)
            {
                for (int32 stateIndex = 0; stateIndex < UIControl::STATE_COUNT; ++stateIndex)
                    if (currentToken == UIControl::STATE_NAMES[stateIndex])
                        currentSelector.stateMask |= 1 << stateIndex;
            }
        }
        currentToken = "";
        state = newState;
    }
};

}

UIStyleSheetSelectorChain::UIStyleSheetSelectorChain()
{

}

UIStyleSheetSelectorChain::UIStyleSheetSelectorChain(const String& string)
{
    SelectorParser parser(selectors);
    parser.Parse(string.c_str());
}

String UIStyleSheetSelectorChain::ToString() const
{
    String result = "";

    for (const UIStyleSheetSelector& selectorChainIter : selectors)
    {
        if (selectorChainIter.className.empty() &&
            !selectorChainIter.name.IsValid() &&
            selectorChainIter.classes.empty() &&
            selectorChainIter.stateMask == 0)
        {
            result += "?";
        }
        else
        {
            result += selectorChainIter.className;
            if (selectorChainIter.name.IsValid())
                result += String("#") + selectorChainIter.name.c_str();

            for (const FastName& clazz : selectorChainIter.classes)
                result += String(".") + clazz.c_str();

            for (int32 stateIndex = 0; stateIndex < UIControl::STATE_COUNT; ++stateIndex)
                if (selectorChainIter.stateMask & (1 << stateIndex))
                    result += String(":") + UIControl::STATE_NAMES[stateIndex];
        }

        result += " ";
    }

    if (!result.empty())
        result.resize(result.size() - 1);
    else
        result = "?";

    return result;
}

Vector<UIStyleSheetSelector>::const_iterator UIStyleSheetSelectorChain::begin() const
{
    return selectors.begin();
}

Vector<UIStyleSheetSelector>::const_iterator UIStyleSheetSelectorChain::end() const
{
    return selectors.end();
}

Vector<UIStyleSheetSelector>::const_reverse_iterator UIStyleSheetSelectorChain::rbegin() const
{
    return selectors.rbegin();
}

Vector<UIStyleSheetSelector>::const_reverse_iterator UIStyleSheetSelectorChain::rend() const
{
    return selectors.rend();
}

}
