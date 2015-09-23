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

#include "FXCache.h"
#include "Render/ShaderCache.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderPassNames.h"

#include "Utils/Utils.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"

namespace DAVA
{
namespace //for private members
{
Map<Vector<int32>, FXDescriptor> fxDescriptors;
FXDescriptor defaultFX;
bool initialized = false;
}

RenderPassDescriptor::RenderPassDescriptor()
    :cullMode(rhi::CULL_NONE)
    , renderLayer(RenderLayer::RENDER_LAYER_INVALID_ID)
    , shader(nullptr)
{

}

namespace FXCache
{
rhi::DepthStencilState::Descriptor LoadDepthStencilState(const YamlNode* stateNode);
const FXDescriptor& LoadFXFromOldTemplate(const FastName &fxName, HashMap<FastName, int32>& defines, const Vector<int32>& key, const FastName& quality);

void Initialize()
{
    DVASSERT(!initialized);
    initialized = true;
return;
    RenderPassDescriptor defaultPass;
    defaultPass.passName = PASS_FORWARD;
    defaultPass.renderLayer = RenderLayer::RENDER_LAYER_OPAQUE_ID;
    FastName defShaderName = FastName("~res:/Materials/Shaders/Default/materials");
    HashMap<FastName, int32> defFlags;
    defFlags[FastName("MATERIAL_TEXTURE")] = 1;
    defaultPass.shader = ShaderDescriptorCache::GetShaderDescriptor(defShaderName, defFlags);
    defaultFX.renderPassDescriptors.clear();
    defaultFX.renderPassDescriptors.push_back(defaultPass);
}

void Uninitialize()
{
    Clear();
    initialized = false;
}
void Clear()
{
    DVASSERT(initialized);
    //RHI_COMPLETE
}

const FXDescriptor& GetFXDescriptor(const FastName &fxName, HashMap<FastName, int32>& defines, const FastName& quality)
{
    DVASSERT(initialized);
    Vector<int32> key;
    ShaderDescriptorCache::BuildFlagsKey(fxName, defines, key);

    if (quality.IsValid()) //quality made as part of fx key
        key.push_back(quality.Index());

    Map<Vector<int32>, FXDescriptor>& localFx = fxDescriptors;
    auto it = fxDescriptors.find(key);
    if (it != fxDescriptors.end())
        return it->second;
    //not found - load new        
    return LoadFXFromOldTemplate(fxName, defines, key, quality);        
}
    

const FXDescriptor& LoadFXFromOldTemplate(const FastName &fxName, HashMap<FastName, int32>& defines, const Vector<int32>& key, const FastName& quality)
{
    //the stuff below is old old legacy carried from RenderTechnique and NMaterialTemplate

    FilePath fxPath(fxName.c_str());
    YamlParser* parser = YamlParser::Create(fxPath);
    YamlNode* rootNode = nullptr;
    if (parser)
    {
        rootNode = parser->GetRootNode();
    }
    if (!rootNode)
    {
        Logger::Error("Can't load requested old-material-template-into-fx: %s", fxPath.GetAbsolutePathname().c_str());
        SafeRelease(parser);
        return defaultFX;
    }

    const YamlNode* materialTemplateNode = rootNode->Get("MaterialTemplate");
    const YamlNode* renderTechniqueNode = nullptr;
    if (materialTemplateNode) //multy-quality material
    {
        /*int32 quality = 0;
        auto it = defines.find(NMaterialQualityName::QUALITY_FLAG_NAME);
        if (it != defines.end())
            quality = it->second;*/

        const YamlNode *qualityNode = nullptr;
        if(quality.IsValid())
        {
            qualityNode = materialTemplateNode->Get(quality.c_str());
        }
        if (qualityNode == nullptr)
        {
            Logger::Error("Template: %s do not support quality %s - loading first", fxPath.GetAbsolutePathname().c_str(), quality.c_str());
            qualityNode = materialTemplateNode->Get(materialTemplateNode->GetCount()-1);
        }
        YamlParser* parserTechnique = YamlParser::Create(qualityNode->AsString());
        if (parserTechnique)
        {
            renderTechniqueNode = parserTechnique->GetRootNode();
        }
        if (!renderTechniqueNode)
        {
            Logger::Error("Can't load technique from template: %s with quality %s", fxPath.GetAbsolutePathname().c_str(), quality.c_str());
            SafeRelease(parserTechnique);
            return defaultFX;
        }

        SafeRelease(parser);
        parser = parserTechnique;

    }
    else //technique
    {
        renderTechniqueNode = rootNode;
    }

    //now load render technique
    const YamlNode * stateNode = renderTechniqueNode->Get("RenderTechnique");
    if (!stateNode)
    {
        SafeRelease(parser);
        return defaultFX;
    }

    FXDescriptor target;
    target.fxName = fxName;
    target.defines = defines;
    RenderLayer::eRenderLayerID renderLayer = RenderLayer::RENDER_LAYER_OPAQUE_ID;

    const YamlNode * layersNode = stateNode->Get("Layers");
    if (layersNode)
    {
        int32 count = layersNode->GetCount();
        DVASSERT(count == 1);
        renderLayer = RenderLayer::GetLayerIDByName(FastName(layersNode->Get(0)->AsString().c_str()));
    }

    for (uint32 k = 0; k < stateNode->GetCount(); ++k)
    {
        if (stateNode->GetItemKeyName(k) == "RenderPass")
        {
            RenderPassDescriptor passDescriptor;
            passDescriptor.renderLayer = renderLayer;

            const YamlNode * renderPassNode = stateNode->Get(k);

            //name            
            const YamlNode * renderPassNameNode = renderPassNode->Get("Name");
            if (renderPassNameNode)
            {
                passDescriptor.passName = renderPassNameNode->AsFastName();
            }
            
            //shader
            const YamlNode * shaderNode = renderPassNode->Get("Shader");
            if (!shaderNode)
            {
                Logger::Error("RenderPass:%s does not have shader", passDescriptor.passName.c_str());
                break;
            }
            FastName shaderName = shaderNode->AsFastName();
            HashMap<FastName, int32> shaderDefines = defines;
            const YamlNode * definesNode = renderPassNode->Get("UniqueDefines");
            if (definesNode)
            {
                int32 count = definesNode->GetCount();
                for (int32 k = 0; k < count; ++k)
                {
                    const YamlNode * singleDefineNode = definesNode->Get(k);
                    shaderDefines[FastName(singleDefineNode->AsString().c_str())] = 1;
                }
            }

            //state
            const YamlNode * renderStateNode = renderPassNode->Get("RenderState");
            if (renderStateNode)
            {                
                const YamlNode * stateNode = renderStateNode->Get("state");
                if (stateNode)
                {
                    Vector<String> states;
                    Split(stateNode->AsString(), "| ", states);                    
                    passDescriptor.depthStateDescriptor.depthTestEnabled = false;
                    passDescriptor.depthStateDescriptor.depthWriteEnabled = false;
                    passDescriptor.cullMode = rhi::CULL_NONE;
                    bool hasBlend = false;
                    for (auto& state : states)
                    {
                        if (state == "STATE_BLEND")
                        {
                            hasBlend = true;
                            if (shaderDefines.find(NMaterialFlagName::FLAG_BLENDING) == shaderDefines.end())
                                shaderDefines[NMaterialFlagName::FLAG_BLENDING] = BLENDING_ALPHABLEND;
                        }
                        else if (state == "STATE_CULL")
                        {
                            passDescriptor.cullMode = rhi::CULL_CW; //default
                            const YamlNode * cullModeNode = renderStateNode->Get("cullMode");
                            if (cullModeNode)
                            {
                                if (cullModeNode->AsString() == "FACE_FRONT")
                                    passDescriptor.cullMode = rhi::CULL_CCW;
                            }
                        }
                        else if (state == "STATE_DEPTH_WRITE")
                        {
                            passDescriptor.depthStateDescriptor.depthWriteEnabled = true;
                        }
                        else if (state == "STATE_DEPTH_TEST")
                        {
                            passDescriptor.depthStateDescriptor.depthTestEnabled = true;
                            const YamlNode * depthFuncNode = renderStateNode->Get("depthFunc");
                            if (depthFuncNode)
                            {                                
                                passDescriptor.depthStateDescriptor.depthFunc = GetCmpFuncByName(depthFuncNode->AsString()); 
                            }
                        }
                        else if (state == "STATE_STENCIL_TEST")
                        {                            
                            passDescriptor.depthStateDescriptor.stencilEnabled = 1;
                            passDescriptor.depthStateDescriptor.stencilTwoSided = 1;

                            const YamlNode * stencilNode = renderStateNode->Get("stencil");
                            if (stencilNode)
                            {
                                const YamlNode * stencilRefNode = stencilNode->Get("ref");
                                if (stencilRefNode)
                                {
                                    uint8 refValue = (uint8) stencilRefNode->AsInt32();
                                    passDescriptor.depthStateDescriptor.stencilBack.refValue = refValue;
                                    passDescriptor.depthStateDescriptor.stencilFront.refValue = refValue;
                                }                                    

                                const YamlNode * stencilMaskNode = stencilNode->Get("mask");
                                if (stencilMaskNode)
                                {
                                    uint8 maskValue = (uint8)stencilMaskNode->AsInt32();
                                    passDescriptor.depthStateDescriptor.stencilBack.readMask = maskValue;
                                    passDescriptor.depthStateDescriptor.stencilBack.writeMask = maskValue;
                                    passDescriptor.depthStateDescriptor.stencilFront.readMask = maskValue;
                                    passDescriptor.depthStateDescriptor.stencilFront.writeMask = maskValue;                                    
                                }                                                                        

                                const YamlNode * stencilFuncNode = stencilNode->Get("funcFront");
                                if (stencilFuncNode)
                                {
                                    passDescriptor.depthStateDescriptor.stencilFront.func = GetCmpFuncByName(stencilFuncNode->AsString());
                                }

                                stencilFuncNode = stencilNode->Get("funcBack");
                                if (stencilFuncNode)
                                {
                                    passDescriptor.depthStateDescriptor.stencilBack.func = GetCmpFuncByName(stencilFuncNode->AsString());
                                }

                                const YamlNode * stencilPassNode = stencilNode->Get("passFront");
                                if (stencilPassNode)
                                {
                                    passDescriptor.depthStateDescriptor.stencilFront.depthStencilPassOperation = GetStencilOpByName(stencilPassNode->AsString());                                    
                                }

                                stencilPassNode = stencilNode->Get("passBack");
                                if (stencilPassNode)
                                {
                                    passDescriptor.depthStateDescriptor.stencilBack.depthStencilPassOperation = GetStencilOpByName(stencilPassNode->AsString());
                                }

                                const YamlNode * stencilFailNode = stencilNode->Get("failFront");
                                if (stencilFailNode)
                                {
                                    passDescriptor.depthStateDescriptor.stencilFront.failOperation = GetStencilOpByName(stencilFailNode->AsString());
                                }

                                stencilFailNode = stencilNode->Get("failBack");
                                if (stencilFailNode)
                                {
                                    passDescriptor.depthStateDescriptor.stencilBack.failOperation = GetStencilOpByName(stencilFailNode->AsString());
                                }

                                const YamlNode * stencilZFailNode = stencilNode->Get("zFailFront");
                                if (stencilZFailNode)
                                {
                                    passDescriptor.depthStateDescriptor.stencilFront.depthFailOperation = GetStencilOpByName(stencilZFailNode->AsString());
                                }

                                stencilZFailNode = stencilNode->Get("zFailBack");
                                if (stencilZFailNode)
                                {
                                    passDescriptor.depthStateDescriptor.stencilBack.depthFailOperation = GetStencilOpByName(stencilZFailNode->AsString());
                                }
                            }
                        }

                    }  
                    //it's temporary solution to allow runtime blend-mode configuration and still prevent blending for materials without corresponding state
                    if (!hasBlend)
                        shaderDefines.erase(NMaterialFlagName::FLAG_BLENDING); 
                }                
            }

            
            passDescriptor.shader = ShaderDescriptorCache::GetShaderDescriptor(shaderName, shaderDefines);

            target.renderPassDescriptors.push_back(passDescriptor);
        }
    }

    SafeRelease(parser);
    return fxDescriptors[key] = target;
}


}
}

