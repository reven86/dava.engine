#include "FXCache.h"
#include "Render/ShaderCache.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderPassNames.h"

#include "Logger/Logger.h"
#include "Utils/Utils.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"

namespace DAVA
{
namespace FXCacheDetails
{
Map<Vector<int32>, FXDescriptor> fxDescriptors;
Map<std::pair<FastName, FastName>, FXDescriptor> oldTemplateMap;

FXDescriptor defaultFX;
bool initialized = false;
}

namespace FXCache
{
const FXDescriptor& LoadFXFromOldTemplate(const FastName& fxName, HashMap<FastName, int32>& defines, const Vector<int32>& key, const FastName& quality);

void Initialize()
{
    using namespace FXCacheDetails;

    DVASSERT(!initialized);
    initialized = true;

    HashMap<FastName, int32> defFlags;
    defFlags[FastName("MATERIAL_TEXTURE")] = 1;

    RenderPassDescriptor defaultPass;
    defaultPass.passName = PASS_FORWARD;
    defaultPass.renderLayer = RenderLayer::RENDER_LAYER_OPAQUE_ID;
    defaultPass.shaderFileName = FastName("~res:/Materials/Shaders/Default/materials");
    defaultPass.shader = ShaderDescriptorCache::GetShaderDescriptor(defaultPass.shaderFileName, defFlags);
    defaultPass.templateDefines[FastName("MATERIAL_TEXTURE")] = 1;

    defaultFX.renderPassDescriptors.clear();
    defaultFX.renderPassDescriptors.push_back(defaultPass);
}

void Uninitialize()
{
    Clear();
    FXCacheDetails::initialized = false;
}
void Clear()
{
    DVASSERT(FXCacheDetails::initialized);
    //RHI_COMPLETE
}

const FXDescriptor& GetFXDescriptor(const FastName& fxName, HashMap<FastName, int32>& defines, const FastName& quality)
{
    using namespace FXCacheDetails;

    DVASSERT(initialized);

    if (!fxName.IsValid())
    {
        return FXCacheDetails::defaultFX;
    }

    Vector<int32> key = ShaderDescriptorCache::BuildFlagsKey(fxName, defines);

    if (quality.IsValid()) //quality made as part of fx key
        key.push_back(quality.Index());

    //[METAL_COMPLETE] to be able to switch fx depending on metal/non-metal option
    key.push_back(QualitySettingsSystem::Instance()->GetAllowMetalFeatures() ? 1 : 0);

    auto it = fxDescriptors.find(key);
    if (it != fxDescriptors.end())
        return it->second;

    //not found - load new
    return LoadFXFromOldTemplate(fxName, defines, key, quality);
}

const FXDescriptor& LoadOldTempalte(const FastName& fxName, const FastName& quality)
{
    using namespace FXCacheDetails;

    auto oldTemplate = oldTemplateMap.find(std::make_pair(fxName, quality));
    if (oldTemplate != oldTemplateMap.end())
    {
        return oldTemplate->second;
    }

    FilePath fxPath(fxName.c_str());
    ScopedPtr<YamlParser> parser(YamlParser::Create(fxPath));
    YamlNode* rootNode = nullptr;
    if (parser)
    {
        rootNode = parser->GetRootNode();
    }
    if (!rootNode)
    {
        Logger::Error("Can't load requested old-material-template-into-fx: %s", fxPath.GetAbsolutePathname().c_str());
        return FXCacheDetails::defaultFX;
    }

    const YamlNode* materialTemplateNode = rootNode->Get("MaterialTemplate");
    const YamlNode* renderTechniqueNode = nullptr;
    if (materialTemplateNode) //multy-quality material
    {
        const YamlNode* qualityNode = nullptr;
        if (quality.IsValid())
        {
            qualityNode = materialTemplateNode->Get(quality.c_str());
        }
        if (qualityNode == nullptr)
        {
            if ((quality.c_str() == nullptr) || (strlen(quality.c_str()) == 0))
            {
                Logger::Warning("Quality not defined for template: %s, loading default one.",
                                fxPath.GetAbsolutePathname().c_str(), quality.c_str());
            }
            else
            {
                Logger::Error("Template: %s do not support quality %s - loading first one.",
                              fxPath.GetAbsolutePathname().c_str(), quality.c_str());
            }
            qualityNode = materialTemplateNode->Get(materialTemplateNode->GetCount() - 1);
        }
        ScopedPtr<YamlParser> parserTechnique(YamlParser::Create(qualityNode->AsString()));
        if (parserTechnique)
        {
            renderTechniqueNode = parserTechnique->GetRootNode();
        }
        if (!renderTechniqueNode)
        {
            Logger::Error("Can't load technique from template: %s with quality %s", fxPath.GetAbsolutePathname().c_str(), quality.c_str());
            return FXCacheDetails::defaultFX;
        }

        parser = parserTechnique;
    }
    else //technique
    {
        renderTechniqueNode = rootNode;
    }

    //now load render technique
    const YamlNode* stateNode = renderTechniqueNode->Get("RenderTechnique");
    if (stateNode == nullptr)
    {
        return FXCacheDetails::defaultFX;
    }

    FXDescriptor target;
    target.fxName = fxName;

    RenderLayer::eRenderLayerID renderLayer = RenderLayer::RENDER_LAYER_OPAQUE_ID;

    const YamlNode* layersNode = stateNode->Get("Layers");
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

            const YamlNode* renderPassNode = stateNode->Get(k);

            //name
            const YamlNode* renderPassNameNode = renderPassNode->Get("Name");
            if (renderPassNameNode)
            {
                passDescriptor.passName = renderPassNameNode->AsFastName();
            }

            //shader
            const YamlNode* shaderNode = renderPassNode->Get("Shader");
            if (!shaderNode)
            {
                Logger::Error("RenderPass:%s does not have shader", passDescriptor.passName.c_str());
                break;
            }
            passDescriptor.shaderFileName = shaderNode->AsFastName();

            const YamlNode* definesNode = renderPassNode->Get("UniqueDefines");
            if (definesNode)
            {
                int32 count = definesNode->GetCount();
                for (int32 k = 0; k < count; ++k)
                {
                    const YamlNode* singleDefineNode = definesNode->Get(k);
                    passDescriptor.templateDefines[FastName(singleDefineNode->AsString().c_str())] = 1;
                }
            }

            //state
            const YamlNode* renderStateNode = renderPassNode->Get("RenderState");
            if (renderStateNode)
            {
                const YamlNode* stateNode = renderStateNode->Get("state");
                if (stateNode)
                {
                    Vector<String> states;
                    Split(stateNode->AsString(), "| ", states);
                    passDescriptor.depthStateDescriptor.depthTestEnabled = false;
                    passDescriptor.depthStateDescriptor.depthWriteEnabled = false;
                    passDescriptor.cullMode = rhi::CULL_NONE;
                    bool hasBlend = false;
                    const YamlNode* fillMode = renderStateNode->Get("fillMode");
                    if (fillMode)
                    {
                        if (fillMode->AsString() == "FILLMODE_WIREFRAME")
                            passDescriptor.wireframe = true;
                    }
                    for (auto& state : states)
                    {
                        if (state == "STATE_BLEND")
                        {
                            passDescriptor.hasBlend = true;
                        }
                        else if (state == "STATE_CULL")
                        {
                            passDescriptor.cullMode = rhi::CULL_CW; //default
                            const YamlNode* cullModeNode = renderStateNode->Get("cullMode");
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
                            const YamlNode* depthFuncNode = renderStateNode->Get("depthFunc");
                            if (depthFuncNode)
                            {
                                passDescriptor.depthStateDescriptor.depthFunc = GetCmpFuncByName(depthFuncNode->AsString());
                            }
                        }
                        else if (state == "STATE_STENCIL_TEST")
                        {
                            passDescriptor.depthStateDescriptor.stencilEnabled = 1;

                            const YamlNode* stencilNode = renderStateNode->Get("stencil");
                            if (stencilNode)
                            {
                                const YamlNode* stencilRefNode = stencilNode->Get("ref");
                                if (stencilRefNode)
                                {
                                    uint8 refValue = static_cast<uint8>(stencilRefNode->AsInt32());
                                    passDescriptor.depthStateDescriptor.stencilBack.refValue = refValue;
                                    passDescriptor.depthStateDescriptor.stencilFront.refValue = refValue;
                                }

                                const YamlNode* stencilMaskNode = stencilNode->Get("mask");
                                if (stencilMaskNode)
                                {
                                    uint8 maskValue = static_cast<uint8>(stencilMaskNode->AsInt32());
                                    passDescriptor.depthStateDescriptor.stencilBack.readMask = maskValue;
                                    passDescriptor.depthStateDescriptor.stencilBack.writeMask = maskValue;
                                    passDescriptor.depthStateDescriptor.stencilFront.readMask = maskValue;
                                    passDescriptor.depthStateDescriptor.stencilFront.writeMask = maskValue;
                                }

                                const YamlNode* stencilFuncNode = stencilNode->Get("funcFront");
                                if (stencilFuncNode)
                                {
                                    passDescriptor.depthStateDescriptor.stencilFront.func = GetCmpFuncByName(stencilFuncNode->AsString());
                                }

                                stencilFuncNode = stencilNode->Get("funcBack");
                                if (stencilFuncNode)
                                {
                                    passDescriptor.depthStateDescriptor.stencilBack.func = GetCmpFuncByName(stencilFuncNode->AsString());
                                }

                                const YamlNode* stencilPassNode = stencilNode->Get("passFront");
                                if (stencilPassNode)
                                {
                                    passDescriptor.depthStateDescriptor.stencilFront.depthStencilPassOperation = GetStencilOpByName(stencilPassNode->AsString());
                                }

                                stencilPassNode = stencilNode->Get("passBack");
                                if (stencilPassNode)
                                {
                                    passDescriptor.depthStateDescriptor.stencilBack.depthStencilPassOperation = GetStencilOpByName(stencilPassNode->AsString());
                                }

                                const YamlNode* stencilFailNode = stencilNode->Get("failFront");
                                if (stencilFailNode)
                                {
                                    passDescriptor.depthStateDescriptor.stencilFront.failOperation = GetStencilOpByName(stencilFailNode->AsString());
                                }

                                stencilFailNode = stencilNode->Get("failBack");
                                if (stencilFailNode)
                                {
                                    passDescriptor.depthStateDescriptor.stencilBack.failOperation = GetStencilOpByName(stencilFailNode->AsString());
                                }

                                const YamlNode* stencilZFailNode = stencilNode->Get("zFailFront");
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

                            passDescriptor.depthStateDescriptor.stencilTwoSided = (memcmp(&passDescriptor.depthStateDescriptor.stencilBack,
                                                                                          &passDescriptor.depthStateDescriptor.stencilFront,
                                                                                          sizeof(rhi::DepthStencilState::Descriptor::StencilDescriptor)
                                                                                          ) != 0);
                        }
                    }
                }
            }

            target.renderPassDescriptors.push_back(passDescriptor);
        }
    }

    //add render pass for reflection/refraction - hard coded for now
    //TODO: rethink how to modify material template without full copy for all passes
    //do it only for editor or if metal features are allowed - performance issue
    if (QualitySettingsSystem::Instance()->GetAllowMetalFeatures() || QualitySettingsSystem::Instance()->GetRuntimeQualitySwitching())
    {
        for (RenderPassDescriptor& pass : target.renderPassDescriptors)
        {
            if (pass.passName == PASS_FORWARD)
            {
                RenderPassDescriptor noFog = pass;
                noFog.passName = PASS_REFLECTION_REFRACTION;
                noFog.templateDefines[NMaterialFlagName::FLAG_FOG_HALFSPACE] = 0;
                target.renderPassDescriptors.push_back(noFog);
                break;
            }
        }
    }

    return oldTemplateMap[std::make_pair(fxName, quality)] = target;
}

const FXDescriptor& LoadFXFromOldTemplate(const FastName& fxName, HashMap<FastName, int32>& defines, const Vector<int32>& key, const FastName& quality)
{
    //the stuff below is old old legacy carried from RenderTechnique and NMaterialTemplate

    FXDescriptor target = LoadOldTempalte(fxName, quality); //we copy it to new fxdescr as single template can be compiled to many descriptors
    target.defines = defines; //combine
    for (auto& pass : target.renderPassDescriptors)
    {
        HashMap<FastName, int32> shaderDefines = defines;
        for (auto& templateDefine : pass.templateDefines)
            shaderDefines[templateDefine.first] = templateDefine.second;
        if (pass.hasBlend)
        {
            if (shaderDefines.find(NMaterialFlagName::FLAG_BLENDING) == shaderDefines.end())
                shaderDefines[NMaterialFlagName::FLAG_BLENDING] = BLENDING_ALPHABLEND;
        }
        else
        {
            shaderDefines.erase(NMaterialFlagName::FLAG_BLENDING);
        }

        //[METAL_COMPLETE] THIS IS TEMPORARY SOLUTION TO ENNABLE IT FOR METAL ONLY
        if (!QualitySettingsSystem::Instance()->GetAllowMetalFeatures())
        {
            shaderDefines.erase(NMaterialFlagName::FLAG_SPECULAR);
            shaderDefines.erase(NMaterialFlagName::FLAG_FLOWMAP_SKY);
        }

        pass.shader = ShaderDescriptorCache::GetShaderDescriptor(pass.shaderFileName, shaderDefines);
        pass.depthStencilState = rhi::AcquireDepthStencilState(pass.depthStateDescriptor);
    }

    return FXCacheDetails::fxDescriptors[key] = target;
}
}
}
