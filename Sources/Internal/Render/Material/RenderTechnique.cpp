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
#include "Utils/Utils.h"

namespace DAVA
{
    
uint16 RenderTechnique::techinqueSequenceId = 0;
	
RenderTechniquePass::RenderTechniquePass(const FastName & _shaderName,
										 const FastNameSet & _uniqueDefines,
                                         const rhi::DepthStencilState::Descriptor& _depthStencilState)
{
    shaderName = _shaderName;
    uniqueDefines = _uniqueDefines;
    depthStencilState = _depthStencilState;
}

RenderTechniquePass::~RenderTechniquePass()
{    
}
    
    
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
											 const FastName& shaderName,
                                             const FastNameSet& uniqueDefines,
                                             const rhi::DepthStencilState::Descriptor& depthStencilState)
{
    RenderTechniquePass * technique = new RenderTechniquePass(shaderName, uniqueDefines, depthStencilState);
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
            
            rhi::DepthStencilState::Descriptor stateDescriptor;
            if (renderStepNode)
            {
                stateDescriptor = LoadDepthStencilState(renderStepNode);
            }
            
            targetTechnique->AddRenderTechniquePass(renderPassName, shaderName, definesSet, stateDescriptor);
            //techniqueCount++;
        }
    }
    return true;
}

rhi::DepthStencilState::Descriptor RenderTechniqueSingleton::LoadDepthStencilState(const YamlNode * rootNode)
{
    

    rhi::DepthStencilState::Descriptor resState;
    if (!rootNode)
        return resState;

    const YamlNode * renderStateNode = rootNode->Get("RenderState");
    /*if (renderStateNode)
    {
        const YamlNode * stateNode = renderStateNode->Get("state");
        if (stateNode)
        {
            Vector<String> states;
            Split(stateNode->AsString(), "| ", states);
            uint32 currentState = 0;
            for (Vector<String>::const_iterator it = states.begin(); it != states.end(); it++)
                currentState |= GetRenderStateByName((*it));

            resState.state = currentState;
        }

        const YamlNode * blendSrcNode = renderStateNode->Get("blendSrc");
        const YamlNode * blendDestNode = renderStateNode->Get("blendDest");
        if (blendSrcNode && blendDestNode)
        {
            eBlendMode newBlendScr = GetBlendModeByName(blendSrcNode->AsString());
            eBlendMode newBlendDest = GetBlendModeByName(blendDestNode->AsString());

            resState.sourceFactor = newBlendScr;
            resState.destFactor = newBlendDest;
        }

        const YamlNode * cullModeNode = renderStateNode->Get("cullMode");
        if (cullModeNode)
        {
            int32 newCullMode = (int32)GetFaceByName(cullModeNode->AsString());
            resState.cullMode = (eFace)newCullMode;
        }

        const YamlNode * depthFuncNode = renderStateNode->Get("depthFunc");
        if (depthFuncNode)
        {
            eCmpFunc newDepthFunc = GetCmpFuncByName(depthFuncNode->AsString());
            resState.depthFunc = newDepthFunc;
        }

        const YamlNode * fillModeNode = renderStateNode->Get("fillMode");
        if (fillModeNode)
        {
            eFillMode newFillMode = GetFillModeByName(fillModeNode->AsString());
            resState.fillMode = newFillMode;
        }

        //		const YamlNode * alphaFuncNode = renderStateNode->Get("alphaFunc");
        //		const YamlNode * alphaFuncCmpValueNode = renderStateNode->Get("alphaFuncCmpValue");
        //		if(alphaFuncNode && alphaFuncCmpValueNode)
        //		{
        //			eCmpFunc newAlphaFunc = GetCmpFuncByName(alphaFuncNode->AsString());
        //			float32 newCmpValue = alphaFuncCmpValueNode->AsFloat();
        //		
        //			//DO NOTHING FOR NOW
        //		}

        const YamlNode * stencilNode = renderStateNode->Get("stencil");
        if (stencilNode)
        {
            const YamlNode * stencilRefNode = stencilNode->Get("ref");
            if (stencilRefNode)
                resState.stencilRef = stencilRefNode->AsInt32();

            const YamlNode * stencilMaskNode = stencilNode->Get("mask");
            if (stencilMaskNode)
                resState.stencilMask = stencilMaskNode->AsUInt32();

            const YamlNode * stencilFuncNode = stencilNode->Get("funcFront");
            if (stencilFuncNode)
            {
                resState.stencilFunc[FACE_FRONT] = GetCmpFuncByName(stencilFuncNode->AsString());
            }

            stencilFuncNode = stencilNode->Get("funcBack");
            if (stencilFuncNode)
            {
                resState.stencilFunc[FACE_BACK] = GetCmpFuncByName(stencilFuncNode->AsString());
            }

            const YamlNode * stencilPassNode = stencilNode->Get("passFront");
            if (stencilPassNode)
            {
                resState.stencilPass[FACE_FRONT] = GetStencilOpByName(stencilPassNode->AsString());
            }

            stencilPassNode = stencilNode->Get("passBack");
            if (stencilPassNode)
            {
                resState.stencilPass[FACE_BACK] = GetStencilOpByName(stencilPassNode->AsString());
            }

            const YamlNode * stencilFailNode = stencilNode->Get("failFront");
            if (stencilFailNode)
            {
                resState.stencilFail[FACE_FRONT] = GetStencilOpByName(stencilFailNode->AsString());
            }

            stencilFailNode = stencilNode->Get("failBack");
            if (stencilFailNode)
            {
                resState.stencilFail[FACE_BACK] = GetStencilOpByName(stencilFailNode->AsString());
            }

            const YamlNode * stencilZFailNode = stencilNode->Get("zFailFront");
            if (stencilZFailNode)
            {
                resState.stencilZFail[FACE_FRONT] = GetStencilOpByName(stencilZFailNode->AsString());
            }

            stencilZFailNode = stencilNode->Get("zFailBack");
            if (stencilZFailNode)
            {
                resState.stencilZFail[FACE_BACK] = GetStencilOpByName(stencilZFailNode->AsString());
            }
        }        
    }*/
}

    
RenderTechnique * RenderTechniqueSingleton::CreateTechniqueByName(const FastName & renderTechniquePathInFastName)
{
    FilePath renderTechniquePathname(renderTechniquePathInFastName.c_str());
    //FastName renderTechniqueFastName(renderTechniquePathname.GetRelativePathname().c_str());
    //Logger::Debug("Get render technique: %s %d", renderTechniquePathname.GetRelativePathname().c_str(), renderTechniqueFastName.Index());
    
    RenderTechnique * renderTechnique = renderTechniqueMap.at(renderTechniquePathInFastName);
    if (!renderTechnique)
    {
		YamlParser * parser = YamlParser::Create(renderTechniquePathname);
		if (!parser)
		{
			Logger::Error("Can't load requested material: %s", renderTechniquePathname.GetRelativePathname().c_str());
			return 0;
		}

        YamlNode * rootNode = parser->GetRootNode();
		if (!rootNode)
		{
			SafeRelease(parser);
			return 0;
		}
        
        renderTechnique = new RenderTechnique(renderTechniquePathInFastName);
        LoadRenderTechniqueFromYamlNode(rootNode, renderTechnique);
        renderTechniqueMap.insert(renderTechniquePathInFastName, renderTechnique);
     
        SafeRelease(parser);
    }
	//else
    //{
    //    Logger::Debug("Get render technique: %s", renderTechnique->GetName().c_str());
    //}
	
    return SafeRetain(renderTechnique);
}
    
void RenderTechniqueSingleton::ReleaseRenderTechnique(RenderTechnique * renderTechnique)
{
    renderTechnique->Release();
    if (renderTechnique->GetRetainCount() == 1)
    {
        renderTechniqueMap.erase(renderTechnique->GetName());
        renderTechnique->Release();
    }
}

};
