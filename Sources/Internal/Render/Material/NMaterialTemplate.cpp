
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
	if(NULL == matTemplate)
	{
		FilePath path = templateName.c_str();
		matTemplate = Load(path);
		
		//VI: automatically create template with default quality level
		if(NULL == matTemplate)
		{
			matTemplate = new NMaterialTemplate();
			matTemplate->techniqueStateMap.insert(NMaterialQualityName::DEFAULT_QUALITY_NAME,
												  templateName);
		}
		
		matTemplate->name = templateName;
		templateCache.insert(templateName, matTemplate);
	}
	
	DVASSERT(matTemplate);
	return matTemplate;
}
	
NMaterialTemplate* NMaterialTemplateCache::Load(const FilePath& loadPath)
{
	NMaterialTemplate* result = NULL;
	
	YamlParser * parser = YamlParser::Create(loadPath);
	if (!parser)
	{
		Logger::Error("Can't load requested material template: %s", loadPath.GetAbsolutePathname().c_str());
		return result;
	}
	
	YamlNode * rootNode = parser->GetRootNode();
	
	if (!rootNode)
	{
		SafeRelease(parser);
		return result;
	}
	
	const YamlNode * materialTemplateNode = rootNode->Get("MaterialTemplate");
	
	//VI: if "MaterialTemplate" is NULL then it's probably a technique itsef.
	//VI: Fallback to default behavior.
	if(NULL != materialTemplateNode)
	{
		result = new NMaterialTemplate();
		
		for(uint32 i = 0; i < materialTemplateNode->GetCount(); ++i)
		{
			const YamlNode* techniqueNode = materialTemplateNode->Get(i);
			
			result->techniqueStateMap.insert(FastName(materialTemplateNode->GetItemKeyName(i)),
											 FastName(techniqueNode->AsString()));
		}
	}
	
	SafeRelease(parser);
	return result;
}
};