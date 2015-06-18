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

#include "UI/Styles/UIStyleSheetYamlLoader.h"
#include "UI/Styles/UIStyleSheet.h"
#include "UI/UIControl.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "Utils/Utils.h"

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
                    else if ((*selectorStr) == '*')
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
                if (!currentSelector.controlClassName.empty() || currentSelector.name.IsValid() || !currentSelector.classes.empty())
                    selectorChain.push_back(currentSelector);
                currentSelector = UIStyleSheetSelector();
            }

            void GoToState(SelectorParserState newState)
            {
                if (currentToken != "")
                {
                    if (state == SELECTOR_STATE_CONTROL_CLASS_NAME)
                    {
                        currentSelector.controlClassName = currentToken;
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
                                currentSelector.controlStateMask |= 1 << stateIndex;
                    }
                }
                currentToken = "";
                state = newState;
            }
        };
    }

    UIStyleSheetYamlLoader::UIStyleSheetYamlLoader()
    {

    }

    void UIStyleSheetYamlLoader::LoadFromYaml(const FilePath& path, Vector<UIStyleSheet*>* styleSheets)
    {
        RefPtr<YamlParser> parser(YamlParser::Create(path));

        if (parser.Get() == nullptr)
            return;

        YamlNode* rootNode = parser->GetRootNode();
        if (rootNode)
            LoadFromYaml(rootNode, styleSheets);
    }

    void UIStyleSheetYamlLoader::LoadFromYaml(const YamlNode* rootNode, Vector<UIStyleSheet*>* styleSheets)
    {
        DVASSERT(styleSheets);

        const MultiMap<String, YamlNode*> &styleSheetMap = rootNode->AsMap();
        const UIStyleSheetPropertyDataBase* propertyDB = UIStyleSheetPropertyDataBase::Instance();

        for (auto styleSheetIter = styleSheetMap.begin(); styleSheetIter != styleSheetMap.end(); ++styleSheetIter)
        {
            Vector<std::pair<uint32, VariantType>> propertiesToSet;
            ScopedPtr<UIStyleSheetPropertyTable> propertyTable(new UIStyleSheetPropertyTable());
            const MultiMap<String, YamlNode*> &propertiesMap = styleSheetIter->second->AsMap();
            for (const auto& propertyIter : propertiesMap)
            {
                uint32 index = propertyDB->GetStyleSheetPropertyIndex(FastName(propertyIter.first));
                const UIStyleSheetPropertyDescriptor& propertyDescr = propertyDB->GetStyleSheetPropertyByIndex(index);
                propertiesToSet.push_back(std::make_pair(index, propertyIter.second->AsVariantType(propertyDescr.targetMembers[0].memberInfo)));
            }
            propertyTable->SetProperties(propertiesToSet);

            Vector<String> selectorList;
            Split(styleSheetIter->first, ",", selectorList);

            for (const String& selectorString : selectorList)
            {
                UIStyleSheet* styleSheet = new UIStyleSheet();

                Vector<UIStyleSheetSelector> selectorChain;
                SelectorParser parser(selectorChain);
                parser.Parse(selectorString.c_str());

                styleSheet->SetSelectorChain(selectorChain);
                styleSheet->SetPropertyTable(propertyTable);

                styleSheets->push_back(styleSheet);
            }
        }
    }
}
