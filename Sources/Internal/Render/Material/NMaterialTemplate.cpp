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


#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "Scene3D/Systems/MaterialSystem.h"
#include "Render/Material/NMaterialTemplate.h"

namespace DAVA
{

const NMaterialTemplate* NMaterialTemplateCache::Get(const FastName& templateName)
{
    NMaterialTemplate* matTemplate = templateCache.at(templateName);
    if(nullptr == matTemplate)
    {
        FilePath path = templateName.c_str();
        matTemplate = Load(path);

        //VI: automatically create template with default quality level
        if (nullptr == matTemplate)
        {
            matTemplate = new NMaterialTemplate();
            matTemplate->techniqueStateMap.insert(NMaterial::DEFAULT_QUALITY_NAME, templateName);
        }

        matTemplate->name = templateName;
        templateCache.insert(templateName, matTemplate);
    }

    DVASSERT(matTemplate);
    return matTemplate;
}

void NMaterialTemplateCache::Clear()
{
    for (auto& x : templateCache)
    {
        NMaterialTemplate* objToDelete = x.second;
        delete objToDelete;
    }
    templateCache.clear();
}

NMaterialTemplate* NMaterialTemplateCache::Load(const FilePath& loadPath)
{
    RefPtr<YamlParser> parser(YamlParser::Create(loadPath));
    if (!parser.Valid())
    {
        Logger::Error("Cannot load requested material template: %s", loadPath.GetAbsolutePathname().c_str());
        return nullptr;
    }

    YamlNode* rootNode = parser->GetRootNode();
    if (rootNode != nullptr)
    {
        const YamlNode* materialTemplateNode = rootNode->Get("MaterialTemplate");
        //VI: if "MaterialTemplate" is NULL then it's probably a technique itsef.
        //VI: Fallback to default behavior.
        if (nullptr != materialTemplateNode)
        {
            NMaterialTemplate* result = new NMaterialTemplate;
            for (uint32 i = 0, n = materialTemplateNode->GetCount();i < n;++i)
            {
                const YamlNode* techniqueNode = materialTemplateNode->Get(i);
                result->techniqueStateMap.insert(FastName(materialTemplateNode->GetItemKeyName(i)),
                                                 FastName(techniqueNode->AsString()));
            }
            return result;
        }
    }
    return nullptr;
}

}   // namespace DAVA
