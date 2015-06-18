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

#include "UI/Styles/UIStyleSheetYamlWriter.h"
#include "UI/Styles/UIStyleSheet.h"
#include "UI/UIControl.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "Utils/Utils.h"

namespace DAVA
{
    UIStyleSheetYamlWriter::UIStyleSheetYamlWriter()
    {

    }

    RefPtr<YamlNode> UIStyleSheetYamlWriter::SaveToYaml(const Vector< UIStyleSheet* >& styleSheets)
    {
        const UIStyleSheetPropertyDataBase* propertyDB = UIStyleSheetPropertyDataBase::Instance();

        YamlNode* node(YamlNode::CreateMapNode());

        Map<const UIStyleSheetPropertyTable*, String> propertyTables;

        for (UIStyleSheet* styleSheet : styleSheets)
        {
            String& selector = propertyTables[styleSheet->GetPropertyTable()];
            if (!selector.empty())
            {
                selector += ", ";
            }

            selector += UIStyleSheetYamlWriter::GenerateSelectorString(styleSheet->GetSelectorChain());
        }
        
        for (const auto& table : propertyTables)
        {
            YamlNode* styleSheetNode(YamlNode::CreateMapNode(false));

            const UIStyleSheetPropertyTable* propertyTable = table.first;

            for (const auto& prop : propertyTable->GetProperties())
            {
                const UIStyleSheetPropertyDescriptor& propertyDescr = propertyDB->GetStyleSheetPropertyByIndex(prop.first);

                styleSheetNode->Add(propertyDescr.name.c_str(), prop.second);
            }

            node->Add(table.second, styleSheetNode);
        }

        return RefPtr<YamlNode>(node);
    }

    String UIStyleSheetYamlWriter::GenerateSelectorString(const Vector<UIStyleSheetSelector>& selectorChain)
    {
        String result = "";

        for (const UIStyleSheetSelector& selectorChainIter : selectorChain)
        {
            result += selectorChainIter.controlClassName;
            if (selectorChainIter.name.IsValid())
                result += String("#") + selectorChainIter.name.c_str();

            for (const FastName& clazz : selectorChainIter.classes)
                result += String(".") + clazz.c_str();

            for (int32 stateIndex = 0; stateIndex < UIControl::STATE_COUNT; ++stateIndex)
                if (selectorChainIter.controlStateMask & (1 << stateIndex))
                    result += String(":") + UIControl::STATE_NAMES[stateIndex];

            result += " ";
        }

        if (!result.empty())
            result.resize(result.size() - 1);

        return result;
    }
}
