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
#include "Render/Highlevel/RenderFastNames.h"
#include "Render/Material/NMaterialNames.h"

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
    , renderLayer(RENDER_LAYER_INVALID_ID)
    , shader(nullptr)
{

}

namespace FXCache
{
rhi::DepthStencilState::Descriptor LoadDepthStencilState(const YamlNode* stateNode);
const FXDescriptor& LoadFXFromOldTemplate(const FastName &fxName, HashMap<FastName, int32>& defines, Vector<int32>& key);

void Initialize()
{
    DVASSERT(!initialized);
    initialized = true;
    RenderPassDescriptor defaultPass;
    defaultPass.passName = PASS_FORWARD;
    defaultPass.renderLayer = RENDER_LAYER_OPAQUE_ID;    
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

const FXDescriptor& GetFXDescriptor(const FastName &fxName, HashMap<FastName, int32>& defines)
{
    DVASSERT(initialized);
    Vector<int32> key;
    ShaderDescriptorCache::BuildFlagsKey(fxName, defines, key);
    auto it = fxDescriptors.find(key);
    if (it != fxDescriptors.end())
        return it->second;
    //not found - load new        
    return LoadFXFromOldTemplate(fxName, defines, key);        
}
    

const FXDescriptor& LoadFXFromOldTemplate(const FastName &fxName, HashMap<FastName, int32>& defines, Vector<int32>& key)
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
        int32 quality = 0;
        auto it = defines.find(NMaterialQualityName::QUALITY_FLAG_NAME);
        if (it != defines.end())
            quality = it->second;

        YamlParser* parserTechnique = YamlParser::Create(materialTemplateNode->Get(quality)->AsString());
        if (parserTechnique)
        {
            renderTechniqueNode = parserTechnique->GetRootNode();
        }
        if (!renderTechniqueNode)
        {
            Logger::Error("Can't load technique from template: %s with quality %d", fxPath.GetAbsolutePathname().c_str(), quality);
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
    eRenderLayerID renderLayer = RENDER_LAYER_OPAQUE_ID;

    const YamlNode * layersNode = stateNode->Get("Layers");
    if (layersNode)
    {
        int32 count = layersNode->GetCount();
        DVASSERT(count == 1);
        renderLayer = GetLayerIdByName(layersNode->Get(0)->AsString().c_str());
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
            const YamlNode * renderStateNode = rootNode->Get("RenderState");
            if (renderStateNode)
            {                
                const YamlNode * stateNode = renderStateNode->Get("state");
                if (stateNode)
                {
                    Vector<String> states;
                    Split(stateNode->AsString(), "| ", states);                    
                    passDescriptor.depthStateDescriptor.depthTestEnabled = false;
                    passDescriptor.depthStateDescriptor.depthWriteEnabled = false;
                    for (auto& state : states)
                    {
                        if (state == "STATE_BLEND")
                        {
                            shaderDefines[FastName("BLENDING")] = 1; //ALPHA
                        }
                        else if (state == "STATE_CULL")
                        {
                            passDescriptor.cullMode = rhi::CULL_CW; //default
                            const YamlNode * cullModeNode = renderStateNode->Get("cullMode");
                            if (cullModeNode)
                            {
                                if (cullModeNode->AsString() == "FACE_BACK")
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
                            const YamlNode * stencilNode = renderStateNode->Get("stencil");
                            if (stencilNode)
                            {
                                const YamlNode * stencilRefNode = stencilNode->Get("ref");
                                if (stencilRefNode)
                                    passDescriptor.depthStateDescriptor.stencilRefValue = stencilRefNode->AsInt32();

                                const YamlNode * stencilMaskNode = stencilNode->Get("mask");
                                if (stencilMaskNode)
                                {
                                    passDescriptor.depthStateDescriptor.stencilWriteMask = stencilMaskNode->AsUInt32();
                                }                                                                        
#if RHI_COMPLETE
                                const YamlNode * stencilFuncNode = stencilNode->Get("funcFront");
                                if (stencilFuncNode)
                                {
                                    passDescriptor.depthStateDescriptor.stencilFunc
                                    stateData.stencilFunc[FACE_FRONT] = GetCmpFuncByName(stencilFuncNode->AsString());
                                }

                                stencilFuncNode = stencilNode->Get("funcBack");
                                if (stencilFuncNode)
                                {
                                    stateData.stencilFunc[FACE_BACK] = GetCmpFuncByName(stencilFuncNode->AsString());
                                }

                                const YamlNode * stencilPassNode = stencilNode->Get("passFront");
                                if (stencilPassNode)
                                {
                                    stateData.stencilPass[FACE_FRONT] = GetStencilOpByName(stencilPassNode->AsString());
                                }

                                stencilPassNode = stencilNode->Get("passBack");
                                if (stencilPassNode)
                                {
                                    stateData.stencilPass[FACE_BACK] = GetStencilOpByName(stencilPassNode->AsString());
                                }

                                const YamlNode * stencilFailNode = stencilNode->Get("failFront");
                                if (stencilFailNode)
                                {
                                    stateData.stencilFail[FACE_FRONT] = GetStencilOpByName(stencilFailNode->AsString());
                                }

                                stencilFailNode = stencilNode->Get("failBack");
                                if (stencilFailNode)
                                {
                                    stateData.stencilFail[FACE_BACK] = GetStencilOpByName(stencilFailNode->AsString());
                                }

                                const YamlNode * stencilZFailNode = stencilNode->Get("zFailFront");
                                if (stencilZFailNode)
                                {
                                    stateData.stencilZFail[FACE_FRONT] = GetStencilOpByName(stencilZFailNode->AsString());
                                }

                                stencilZFailNode = stencilNode->Get("zFailBack");
                                if (stencilZFailNode)
                                {
                                    stateData.stencilZFail[FACE_BACK] = GetStencilOpByName(stencilZFailNode->AsString());
                                }
#endif // RHI_COMPLETE
                            }
                        }

                    }                        
                }
                passDescriptor.depthStateDescriptor = LoadDepthStencilState(renderStateNode);
            }

            
            passDescriptor.shader = ShaderDescriptorCache::GetShaderDescriptor(shaderName, shaderDefines);

            target.renderPassDescriptors.push_back(passDescriptor);
        }
    }

    SafeRelease(parser);
    return fxDescriptors[key] = target;
}


rhi::DepthStencilState::Descriptor LoadDepthStencilState(const YamlNode* stateNode)
{
    rhi::DepthStencilState::Descriptor descr;
    return descr;
    /*const YamlNode * stateNode = renderStateNode->Get("state");
    if (stateNode)
    {
        Vector<String> states;
        Split(stateNode->AsString(), "| ", states);
        uint32 currentState = 0;
        for (Vector<String>::const_iterator it = states.begin(); it != states.end(); it++)
            currentState |= GetRenderStateByName((*it));

        stateData.state = currentState;
    }

    const YamlNode * blendSrcNode = renderStateNode->Get("blendSrc");
    const YamlNode * blendDestNode = renderStateNode->Get("blendDest");
    if (blendSrcNode && blendDestNode)
    {
        eBlendMode newBlendScr = GetBlendModeByName(blendSrcNode->AsString());
        eBlendMode newBlendDest = GetBlendModeByName(blendDestNode->AsString());

        stateData.sourceFactor = newBlendScr;
        stateData.destFactor = newBlendDest;
    }

    const YamlNode * cullModeNode = renderStateNode->Get("cullMode");
    if (cullModeNode)
    {
        int32 newCullMode = (int32)GetFaceByName(cullModeNode->AsString());
        stateData.cullMode = (eFace)newCullMode;
    }

    const YamlNode * depthFuncNode = renderStateNode->Get("depthFunc");
    if (depthFuncNode)
    {
        eCmpFunc newDepthFunc = GetCmpFuncByName(depthFuncNode->AsString());
        stateData.depthFunc = newDepthFunc;
    }

    const YamlNode * fillModeNode = renderStateNode->Get("fillMode");
    if (fillModeNode)
    {
        eFillMode newFillMode = GetFillModeByName(fillModeNode->AsString());
        stateData.fillMode = newFillMode;
    }    

    const YamlNode * stencilNode = renderStateNode->Get("stencil");
    if (stencilNode)
    {
        const YamlNode * stencilRefNode = stencilNode->Get("ref");
        if (stencilRefNode)
            stateData.stencilRef = stencilRefNode->AsInt32();

        const YamlNode * stencilMaskNode = stencilNode->Get("mask");
        if (stencilMaskNode)
            stateData.stencilMask = stencilMaskNode->AsUInt32();

        const YamlNode * stencilFuncNode = stencilNode->Get("funcFront");
        if (stencilFuncNode)
        {
            stateData.stencilFunc[FACE_FRONT] = GetCmpFuncByName(stencilFuncNode->AsString());
        }

        stencilFuncNode = stencilNode->Get("funcBack");
        if (stencilFuncNode)
        {
            stateData.stencilFunc[FACE_BACK] = GetCmpFuncByName(stencilFuncNode->AsString());
        }

        const YamlNode * stencilPassNode = stencilNode->Get("passFront");
        if (stencilPassNode)
        {
            stateData.stencilPass[FACE_FRONT] = GetStencilOpByName(stencilPassNode->AsString());
        }

        stencilPassNode = stencilNode->Get("passBack");
        if (stencilPassNode)
        {
            stateData.stencilPass[FACE_BACK] = GetStencilOpByName(stencilPassNode->AsString());
        }

        const YamlNode * stencilFailNode = stencilNode->Get("failFront");
        if (stencilFailNode)
        {
            stateData.stencilFail[FACE_FRONT] = GetStencilOpByName(stencilFailNode->AsString());
        }

        stencilFailNode = stencilNode->Get("failBack");
        if (stencilFailNode)
        {
            stateData.stencilFail[FACE_BACK] = GetStencilOpByName(stencilFailNode->AsString());
        }

        const YamlNode * stencilZFailNode = stencilNode->Get("zFailFront");
        if (stencilZFailNode)
        {
            stateData.stencilZFail[FACE_FRONT] = GetStencilOpByName(stencilZFailNode->AsString());
        }

        stencilZFailNode = stencilNode->Get("zFailBack");
        if (stencilZFailNode)
        {
            stateData.stencilZFail[FACE_BACK] = GetStencilOpByName(stencilZFailNode->AsString());
        }
    }*/
}

}
}

