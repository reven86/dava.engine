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

UIStyleSheetYamlLoader::UIStyleSheetYamlLoader()
{

}

void UIStyleSheetYamlLoader::LoadFromYaml(const YamlNode* rootNode, Vector<UIStyleSheet*>* styleSheets)
{
    DVASSERT(styleSheets);

    const Vector<YamlNode*> &styleSheetMap = rootNode->AsVector();
    const UIStyleSheetPropertyDataBase* propertyDB = UIStyleSheetPropertyDataBase::Instance();

    for (auto styleSheetIter = styleSheetMap.begin(); styleSheetIter != styleSheetMap.end(); ++styleSheetIter)
    {
        const MultiMap<String, YamlNode*> &styleSheet = (*styleSheetIter)->AsMap();

        auto propertiesSectionIter = styleSheet.find("properties");

        if (propertiesSectionIter != styleSheet.end())
        {
            Vector<UIStyleSheetProperty> propertiesToSet;
            ScopedPtr<UIStyleSheetPropertyTable> propertyTable(new UIStyleSheetPropertyTable());

            for (const auto& propertyIter : propertiesSectionIter->second->AsMap())
            {
                uint32 index = propertyDB->GetStyleSheetPropertyIndex(FastName(propertyIter.first));
                const UIStyleSheetPropertyDescriptor& propertyDescr = propertyDB->GetStyleSheetPropertyByIndex(index);
                if (propertyDescr.memberInfo != nullptr)
                {
                    propertiesToSet.push_back(UIStyleSheetProperty{ index, propertyIter.second->AsVariantType(propertyDescr.memberInfo) });
                }
            }

            auto transitionSectionIter = styleSheet.find("transition");

            if (transitionSectionIter != styleSheet.end())
            {
                for (const auto& propertyTransitionIter : transitionSectionIter->second->AsMap())
                {
                    uint32 index = propertyDB->GetStyleSheetPropertyIndex(FastName(propertyTransitionIter.first));
                    for (UIStyleSheetProperty& prop : propertiesToSet)
                    {
                        if (prop.propertyIndex == index)
                        {
                            const Vector<YamlNode*>& transitionProps = propertyTransitionIter.second->AsVector();
                            int32 transitionFunctionType = Interpolation::LINEAR;
                            if (transitionProps.size() > 1)
                                GlobalEnumMap<Interpolation::FuncType>::Instance()->ToValue(transitionProps[1]->AsString().c_str(), transitionFunctionType);

                            prop.transition = true;
                            prop.transitionFunction = (Interpolation::FuncType)transitionFunctionType;
                            prop.transitionTime = transitionProps[0]->AsFloat();

                            break;
                        }
                    }
                }
            }
            propertyTable->SetProperties(propertiesToSet);

            Vector<String> selectorList;
            Split(styleSheet.find("selector")->second->AsString(), ",", selectorList);

            for (const String& selectorString : selectorList)
            {
                UIStyleSheet* styleSheet = new UIStyleSheet();

                styleSheet->SetSelectorChain(UIStyleSheetSelectorChain(selectorString));
                styleSheet->SetPropertyTable(propertyTable);

                styleSheets->push_back(styleSheet);
            }
        }
    }
}

}
