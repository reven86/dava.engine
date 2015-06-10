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


#include "Render/Material/RenderTechnique.h"
#include "Render/ShaderCache.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "Base/HashMap.h"

namespace DAVA
{
    
uint16 RenderTechnique::techinqueSequenceId = 0;
	
RenderTechniquePass::RenderTechniquePass(const FastName & _shaderName,
										 const FastNameSet & _uniqueDefines,
										 RenderState * _renderState)
{
    shaderName = _shaderName;
    uniqueDefines = _uniqueDefines;
    renderState = _renderState;
}

RenderTechniquePass::~RenderTechniquePass()
{
    SafeDelete(renderState);
}

Shader * RenderTechniquePass::CompileShader(const FastNameSet & materialDefines)
{
    FastNameSet combinedDefines = materialDefines;
    if(uniqueDefines.size() > 0)
    {
        combinedDefines.Combine(uniqueDefines);
    }
    Shader * shader = SafeRetain(ShaderCache::Instance()->Get(shaderName, combinedDefines));
    return shader;
};
    
    
RenderTechnique::RenderTechnique(const FastName & _name)
    :   name(_name)
    ,   nameIndexMap(8)
{
	techinqueSequenceId++;
	techniqueId = techinqueSequenceId;
}
    
RenderTechnique::~RenderTechnique()
{
    size_t size = renderTechniquePassArray.size();
    for (size_t rt = 0; rt < size; ++rt)
    {
        SafeDelete(renderTechniquePassArray[rt]);
    }
    
    renderTechniquePassArray.clear();
    nameIndexMap.clear();
}
    
void RenderTechnique::AddRenderTechniquePass(const FastName& passName,
											 const DAVA::FastName &_shaderName,
                                             const DAVA::FastNameSet & _uniqueDefines,
                                             DAVA::RenderState *_renderState)
{
    RenderTechniquePass * technique = new RenderTechniquePass(_shaderName,
															  _uniqueDefines,
															  _renderState);
    nameIndexMap.insert(passName, (uint32)renderTechniquePassArray.size());
    renderTechniquePassArray.push_back(technique);
}

bool RenderTechniqueSingleton::LoadRenderTechniqueFromYamlNode(const YamlNode * rootNode, RenderTechnique * targetTechnique)
{
/*
    Move to shader level uniforms;
    const YamlNode * uniformsNode = stateNode->Get("Uniforms");
    if (uniformsNode)
    {
        uint32 count = uniformsNode->GetCount();
        for (uint32 k = 0; k < count; ++k)
        {
            const YamlNode * uniformNode = uniformsNode->Get(k);
            if (uniformNode)
            {
                //AddMaterialProperty(uniformsNode->GetItemKeyName(k), uniformNode);
            }
        }
    }
 */
    const YamlNode * stateNode = rootNode->Get("RenderTechnique");
    if (!stateNode)return false;
	
	const YamlNode * layersNode = stateNode->Get("Layers");
	if (layersNode)
	{
		int32 count = layersNode->GetCount();
		for (int32 k = 0; k < count; ++k)
		{
			const YamlNode * singleLayerNode = layersNode->Get(k);
			targetTechnique->layersSet.Insert(singleLayerNode->AsFastName());
        }
	}
    
    for (uint32 k = 0; k < stateNode->GetCount(); ++k)
    {
        if (stateNode->GetItemKeyName(k) == "RenderPass")
        {
            const YamlNode * renderStepNode = stateNode->Get(k);
            const YamlNode * renderPassNameNode = renderStepNode->Get("Name");
            FastName renderPassName;
            if (renderPassNameNode)
            {
                renderPassName = renderPassNameNode->AsFastName();
            }
            
            const YamlNode * shaderNode = renderStepNode->Get("Shader");
            const YamlNode * shaderGraphNode = renderStepNode->Get("ShaderGraph");
            
            if (!shaderNode && !shaderGraphNode)
            {
                Logger::Error("RenderPass:%s does not have shader or shader graph", renderStepNode->AsString().c_str());
                break;
            }
            
            FastName shaderName;
            if (shaderNode)
            {
                shaderName = shaderNode->AsFastName();
            }
            
            FastNameSet definesSet;
            const YamlNode * definesNode = renderStepNode->Get("UniqueDefines");
            if (definesNode)
            {
                int32 count = definesNode->GetCount();
                for (int32 k = 0; k < count; ++k)
                {
                    const YamlNode * singleDefineNode = definesNode->Get(k);
                    definesSet.Insert(FastName(singleDefineNode->AsString().c_str()));
                }
            }
            
            RenderState * renderState = new RenderState();
            if (renderStepNode)
            {
                renderState->LoadFromYamlNode(renderStepNode);
            }
            
            targetTechnique->AddRenderTechniquePass(renderPassName,
													shaderName,
													definesSet,
													renderState);
            //techniqueCount++;
        }
    }
    return true;
}

    
RenderTechnique* RenderTechniqueSingleton::CreateTechniqueByName(const FastName & renderTechniquePathInFastName)
{
    RenderTechnique* renderTechnique = renderTechniqueMap.at(renderTechniquePathInFastName);
    if (nullptr == renderTechnique)
    {
        FilePath renderTechniquePathname(renderTechniquePathInFastName.c_str());
        RefPtr<YamlParser> parser(YamlParser::Create(renderTechniquePathname));
        if (!parser.Valid())
        {
            Logger::Error("Cannot load requested material: %s", renderTechniquePathname.GetRelativePathname().c_str());
            return nullptr;
        }

        YamlNode* rootNode = parser->GetRootNode();
        if (rootNode != nullptr)
        {
            renderTechnique = new RenderTechnique(renderTechniquePathInFastName);
            LoadRenderTechniqueFromYamlNode(rootNode, renderTechnique);
            renderTechniqueMap.insert(renderTechniquePathInFastName, renderTechnique);
        }
        else
        {
            return nullptr;
        }
    }
    return SafeRetain(renderTechnique);
}

void RenderTechniqueSingleton::ReleaseRenderTechnique(RenderTechnique* renderTechnique)
{
    DVASSERT(renderTechnique != nullptr);
    DVASSERT(renderTechnique->GetRetainCount() > 1);    // If reference count is less than 2 then RenderTechnique
                                                        // has been released bypassing ReleaseRenderTechnique

    renderTechnique->Release();
    if (renderTechnique->GetRetainCount() == 1)
    {
        renderTechniqueMap.erase(renderTechnique->GetName());
        renderTechnique->Release();
    }
}

void RenderTechniqueSingleton::ClearRenderTechniques()
{
    // DAVA::HashMap's iterators are invalidated after map erasing operation
    // So ClearRenderTechniques does the following:
    //  - traverses over map and releases technique
    //  - if technique is referenced only by RenderTechniqueSingleton makes final release of technique
    //  - instead of deleted technique places nullptr
    for (auto& x : renderTechniqueMap)
    {
        RenderTechnique* technique = x.second;
        if (technique != nullptr)
        {
            DVASSERT(technique->GetRetainCount() > 1);  // If reference count is less than 2 then RenderTechnique
                                                        // has been released bypassing ReleaseRenderTechnique
            technique->Release();
            if (1 == technique->GetRetainCount())
            {
                technique->Release();
                renderTechniqueMap[x.first] = nullptr;
            }
        }
    }
}

};
