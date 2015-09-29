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


#include "Render/Material/NMaterial.h"

#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Scene3D/SceneFile/SerializationContext.h"

#include "Render/Material/NMaterialNames.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Material/FXCache.h"
#include "Render/Shader.h"
#include "Render/Texture.h"

#include "Utils/Utils.h"
#include "Utils/StringFormat.h"
#include "FileSystem/YamlParser.h"

namespace DAVA
{
namespace MaterialSerializationKey
{
const DAVA::String MaterialKey = "materialKey";
const DAVA::String ParentMaterialKey = "parentMaterialKey";
const DAVA::String FXName = "fxName";
const DAVA::String QualityGroup = "qualityGroup";
const DAVA::String MaterialName = "materialName";
}

struct MaterialPropertyBinding
{
	rhi::ShaderProp::Type type;
	uint32 reg;
	uint32 regCount;
	uint32 updateSemantic;
	NMaterialProperty* source;
	MaterialPropertyBinding(rhi::ShaderProp::Type type_, uint32 reg_, uint32 regCount_, uint32 updateSemantic_, NMaterialProperty* source_) : 
			type(type_), reg(reg_), regCount(regCount_), updateSemantic(updateSemantic_), source(source_) 
	{
	}
};

struct MaterialBufferBinding
{
	rhi::HConstBuffer constBuffer;
	Vector<MaterialPropertyBinding> propBindings;
	uint32 lastValidPropertySemantic = 0;
};

uint32 NMaterialProperty::globalPropertyUpdateSemanticCounter = 0;

RenderVariantInstance::RenderVariantInstance() :shader(nullptr)
{
}

RenderVariantInstance::~RenderVariantInstance()
{    
    rhi::ReleaseDepthStencilState(depthState);
    rhi::ReleaseTextureSet(textureSet);
    rhi::ReleaseSamplerState(samplerState);    
}

NMaterial::NMaterial()
    : localProperties(16, nullptr)
    , localTextures(8, nullptr)
    , localFlags(16, 0)
    , localConstBuffers(16, nullptr)
    , renderVariants(4, nullptr)
{
}

NMaterial::~NMaterial()
{
    SetParent(nullptr);
    DVASSERT(children.size() == 0); //as children refernce parent in our material scheme, this should not be released while it has children
    for (auto& prop : localProperties)
        SafeDelete(prop.second);
    for (auto& texInfo : localTextures)
    {
        SafeRelease(texInfo.second->texture);
        SafeDelete(texInfo.second);
    }
       
    for (auto& buffer : localConstBuffers)
    {
        rhi::DeleteConstBuffer(buffer.second->constBuffer);
        SafeDelete(buffer.second);
    }
    for (auto& variant : renderVariants)
        delete variant.second;
}

void NMaterial::BindParams(rhi::Packet& target)
{
    //Logger::Info( "bind-params" );
    DVASSERT(activeVariantInstance);       //trying to bind material that was not staged to render
    DVASSERT(activeVariantInstance->shader); //should have returned false on PreBuild!
    DVASSERT(activeVariantInstance->shader->IsValid()); //should have returned false on PreBuild!
    /*set pipeline state*/
    target.renderPipelineState = activeVariantInstance->shader->GetPiplineState();
    target.depthStencilState = activeVariantInstance->depthState;
    target.samplerState = activeVariantInstance->samplerState;
    target.textureSet = activeVariantInstance->textureSet;
    target.cullMode = activeVariantInstance->cullMode; //rhi::CULL_NONE;// 

    activeVariantInstance->shader->UpdateDynamicParams();
    /*update values in material const buffers*/
    for (auto& materialBufferBinding : activeVariantInstance->materialBufferBindings)
    {
        if (materialBufferBinding->lastValidPropertySemantic == NMaterialProperty::GetCurrentUpdateSemantic()) //prevent buffer update if nothing changed
            continue;
        //assume that if we have no property - we bind default value on buffer allocation step - no binding is created in that case
        for (auto& materialBinding : materialBufferBinding->propBindings)
        {
            DVASSERT(materialBinding.source)
            if (materialBinding.updateSemantic != materialBinding.source->updateSemantic)
            {
                //Logger::Info( " upd-prop " );
                if (materialBinding.type < rhi::ShaderProp::TYPE_FLOAT4)
                {
                    DVASSERT(materialBinding.source->arraySize == 1);
                    rhi::UpdateConstBuffer1fv(materialBufferBinding->constBuffer, materialBinding.reg, materialBinding.regCount, materialBinding.source->data.get(), ShaderDescriptor::CalculateDataSize(materialBinding.type, materialBinding.source->arraySize));
                }
                else
                {
                    DVASSERT(materialBinding.source->arraySize <= materialBinding.regCount);
                    rhi::UpdateConstBuffer4fv(materialBufferBinding->constBuffer, materialBinding.reg, materialBinding.source->data.get(), ShaderDescriptor::CalculateRegsCount(materialBinding.type, materialBinding.source->arraySize));
                }
                materialBinding.updateSemantic = materialBinding.source->updateSemantic;

#if defined(__DAVAENGINE_RENDERSTATS__)
                ++Renderer::GetRenderStats().materialParamBindCount;
#endif
            }
        }
        materialBufferBinding->lastValidPropertySemantic = NMaterialProperty::GetCurrentUpdateSemantic();
    }

    target.vertexConstCount = activeVariantInstance->vertexConstBuffers.size();
    target.fragmentConstCount = activeVariantInstance->fragmentConstBuffers.size();
    /*bind material const buffers*/
    for (size_t i = 0, sz = activeVariantInstance->vertexConstBuffers.size(); i < sz; ++i)
        target.vertexConst[i] = activeVariantInstance->vertexConstBuffers[i];
    for (size_t i = 0, sz = activeVariantInstance->fragmentConstBuffers.size(); i < sz; ++i)
        target.fragmentConst[i] = activeVariantInstance->fragmentConstBuffers[i];
}


uint32 NMaterial::GetRequiredVertexFormat()
{
    uint32 res = 0;
    for (auto& variant : renderVariants)
    {
        bool shaderValid = (nullptr != variant.second) && (variant.second->shader->IsValid());
        DVASSERT_MSG(shaderValid, "Shader is invalid. Check log for details.");

		if (shaderValid)
		{
			res |= variant.second->shader->GetRequiredVertexFormat();
		}
    }
    return res;
}

MaterialBufferBinding* NMaterial::GetConstBufferBinding(UniquePropertyLayout propertyLayout)
{
    MaterialBufferBinding* res = localConstBuffers.at(propertyLayout);
    if ((res == nullptr) && (parent != nullptr))
        res = parent->GetConstBufferBinding(propertyLayout);

    return res;
}

NMaterialProperty* NMaterial::GetMaterialProperty(const FastName& propName)
{
    NMaterialProperty *res = localProperties.at(propName);
    if ((res == nullptr) && (parent != nullptr))
    {
        res = parent->GetMaterialProperty(propName);
    }
    return res;
}

Texture* NMaterial::GetEffectiveTexture(const FastName& slotName)
{
    MaterialTextureInfo * localInfo = localTextures.at(slotName);
    if (localInfo)
    {
        if (localInfo->texture == nullptr)
            localInfo->texture = Texture::CreateFromFile(localInfo->path);
        return localInfo->texture;
    }
    
    if (parent != nullptr)
    {
        return parent->GetEffectiveTexture(slotName);
    }
    return nullptr;
}
    
void NMaterial::CollectLocalTextures(Set<MaterialTextureInfo *> &collection) const
{
    for(const auto &lc: localTextures)
    {
        const auto & path = lc.second->path;
        if(!path.IsEmpty())
        {
            collection.emplace(lc.second);
        }
    }
}

    

void NMaterial::SetFXName(const FastName & fx)
{
    fxName = fx;
    InvalidateRenderVariants();
}

const FastName& NMaterial::GetEffectiveFXName() const
{   
    if ((!fxName.IsValid()) && (parent != nullptr))
    {
        return parent->GetEffectiveFXName();
    }
    return fxName;
}

const FastName& NMaterial::GetLocalFXName() const
{
    return fxName;
}

bool NMaterial::HasLocalFXName() const
{
    return fxName.IsValid();
}

const FastName& NMaterial::GetQualityGroup()
{    
    if ((!qualityGroup.IsValid()) && (parent != nullptr))
    {
        return parent->GetQualityGroup();
    }
    return qualityGroup;
}

void NMaterial::SetQualityGroup(const FastName& quality)
{
    qualityGroup = quality;
}

void NMaterial::AddProperty(const FastName& propName, const float32 *propData, rhi::ShaderProp::Type type, uint32 arraySize)
{
    DVASSERT(localProperties.at(propName) == nullptr);
    NMaterialProperty *prop = new NMaterialProperty();
    prop->name = propName;
    prop->type = type;
    prop->arraySize = arraySize;
    prop->data.reset(new float[ShaderDescriptor::CalculateDataSize(type, arraySize)]);
    prop->SetPropertyValue(propData);
    localProperties[propName] = prop;
    ClearLocalBuffers(); //RHI_COMPLETE - as local buffers can have binding for this property set as default
    InvalidateBufferBindings();
}

void NMaterial::RemoveProperty(const FastName& propName)
{

    NMaterialProperty *prop = localProperties.at(propName);
    DVASSERT(prop != nullptr);
    localProperties.erase(propName);
    SafeDelete(prop);
    ClearLocalBuffers(); //RHI_COMPLETE - as local buffers can have binding for this property now just clear them all, later rethink to erease just buffers containing this propertyg

    InvalidateBufferBindings();
}

void NMaterial::SetPropertyValue(const FastName& propName, const float32 *propData)
{
    NMaterialProperty *prop = localProperties.at(propName);
    DVASSERT(prop != nullptr);
    prop->SetPropertyValue(propData);
}

bool NMaterial::HasLocalProperty(const FastName& propName)
{
    return localProperties.at(propName)!=nullptr;
}

rhi::ShaderProp::Type NMaterial::GetLocalPropType(const FastName& propName)
{
    NMaterialProperty *prop = localProperties.at(propName);
    DVASSERT(prop != nullptr);
    return prop->type;
}

const float32* NMaterial::GetLocalPropValue(const FastName& propName)
{
    NMaterialProperty *prop = localProperties.at(propName);
    DVASSERT(prop != nullptr);
    return prop->data.get();
}

uint32 NMaterial::GetLocalPropArraySize(const FastName& propName)
{
    NMaterialProperty *prop = localProperties.at(propName);
    DVASSERT(prop != nullptr);
    return prop->arraySize;
}

const float32* NMaterial::GetEffectivePropValue(const FastName& propName)
{
    NMaterialProperty *prop = localProperties.at(propName);
    if (prop)
        return prop->data.get();
    if (parent)
        return parent->GetEffectivePropValue(propName);
    return nullptr;
}

void NMaterial::AddTexture(const FastName& slotName, Texture* texture)
{
    DVASSERT(localTextures.at(slotName) == nullptr);
    MaterialTextureInfo *texInfo = new MaterialTextureInfo();
    texInfo->texture = SafeRetain(texture);
    texInfo->path = texture->GetPathname();
    localTextures[slotName] = texInfo;        
    InvalidateTextureBindings();

}
void NMaterial::RemoveTexture(const FastName& slotName)
{
    MaterialTextureInfo * texInfo = localTextures.at(slotName);
    DVASSERT(texInfo != nullptr);
    localTextures.erase(slotName);
    SafeRelease(texInfo->texture);
    SafeDelete(texInfo);
    InvalidateTextureBindings();
}
void NMaterial::SetTexture(const FastName& slotName, Texture* texture)
{    
    MaterialTextureInfo * texInfo = localTextures.at(slotName);
    DVASSERT(texture != nullptr);    //use RemoveTexture to remove texture!
    DVASSERT(texInfo != nullptr);   //use AddTexture to add texture!

    if (texInfo->texture != texture)
    {
        SafeRelease(texInfo->texture);
        texInfo->texture = SafeRetain(texture);
        texInfo->path = texture->GetPathname();
    }

    InvalidateTextureBindings();
}

bool NMaterial::HasLocalTexture(const FastName& slotName)
{
    return localTextures.find(slotName) != localTextures.end();
}
Texture* NMaterial::GetLocalTexture(const FastName& slotName)
{
    DVASSERT(HasLocalTexture(slotName));
    MaterialTextureInfo * texInfo = localTextures.at(slotName);    
    if (texInfo->texture == nullptr)
        texInfo->texture = Texture::CreateFromFile(texInfo->path);
    return texInfo->texture;
}

void NMaterial::AddFlag(const FastName& flagName, int32 value)
{
    DVASSERT(localFlags.find(flagName) == localFlags.end());
    localFlags[flagName] = value;
    InvalidateRenderVariants();
}
void NMaterial::RemoveFlag(const FastName& flagName)
{
    DVASSERT(localFlags.find(flagName) != localFlags.end());
    localFlags.erase(flagName);
    InvalidateRenderVariants();
}
void NMaterial::SetFlag(const FastName& flagName, int32 value)
{
    DVASSERT(localFlags.find(flagName) != localFlags.end());
    localFlags[flagName] = value;
    InvalidateRenderVariants();
}

int32 NMaterial::GetEffectiveFlagValue(const FastName& flagName)
{
    HashMap<FastName, int32>::iterator it = localFlags.find(flagName);
    if (it != localFlags.end())
        return it->second;
    else if (parent)
        return parent->GetEffectiveFlagValue(flagName);
    return 0;
}

int32 NMaterial::GetLocalFlagValue(const FastName& flagName)
{
    DVASSERT(localFlags.find(flagName) != localFlags.end());
    return localFlags[flagName];
}

bool NMaterial::HasLocalFlag(const FastName& flagName)
{
    return localFlags.find(flagName) != localFlags.end();
}


bool NMaterial::NeedLocalOverride(UniquePropertyLayout propertyLayout)
{
    for (auto& descr : ShaderDescriptor::GetProps(propertyLayout))
    {
        if (localProperties.at(descr.uid) != nullptr)
            return true;
    }
    return false;
}


void NMaterial::SetParent(NMaterial *_parent)
{
    DVASSERT(_parent != this);

    if (parent == _parent)
        return;

    if (parent)
    {
        parent->RemoveChildMaterial(this);
        SafeRelease(parent);
    }
        

    parent = _parent;
    sortingKey = (uint32)((uint64)parent);

    if (parent)
    {
        SafeRetain(parent);
        parent->AddChildMaterial(this);
    }

    InvalidateRenderVariants();
}

NMaterial* NMaterial::GetParent()
{
    return parent;
}

const Vector<NMaterial *>&  NMaterial::GetChildren() const
{
    return children;
}

void NMaterial::AddChildMaterial(NMaterial *material)
{    
    DVASSERT(material);
    children.push_back(material);
}

void NMaterial::RemoveChildMaterial(NMaterial *material)
{
    bool res = FindAndRemoveExchangingWithLast(children, material);
    DVASSERT(res);    
}

void NMaterial::InjectChildBuffer(UniquePropertyLayout propLayoutId, MaterialBufferBinding* buffer)
{
    if (parent&&!NeedLocalOverride(propLayoutId))
        parent->InjectChildBuffer(propLayoutId, buffer);
    else
    {
        DVASSERT(localConstBuffers.at(propLayoutId) == nullptr);
        localConstBuffers[propLayoutId] = buffer;
    }
}

void NMaterial::ClearLocalBuffers()
{

    for (auto& buffer : localConstBuffers)
    {
        rhi::DeleteConstBuffer(buffer.second->constBuffer);        
        SafeDelete(buffer.second);
    }
    for (auto& variant : renderVariants)
        variant.second->materialBufferBindings.clear();
    localConstBuffers.clear();
}

void NMaterial::InvalidateBufferBindings()
{    
    needRebuildBindings = true;
    for (auto& child : children)
        child->InvalidateBufferBindings();
}

void NMaterial::InvalidateTextureBindings()
{
    //reset existing handle?
    needRebuildTextures = true;
    for (auto& child : children)
        child->InvalidateTextureBindings();
}

void NMaterial::InvalidateRenderVariants()
{
    //release existing descriptor?
    needRebuildVariants = true;
    for (auto& child : children)
        child->InvalidateRenderVariants();
}

void NMaterial::RebuildRenderVariants()
{
    HashMap<FastName, int32> flags;
    CollectMaterialFlags(flags);

    const FXDescriptor& fxDescr = FXCache::GetFXDescriptor(GetEffectiveFXName(), flags, QualitySettingsSystem::Instance()->GetCurMaterialQuality(GetQualityGroup()));
    
    if( fxDescr.renderPassDescriptors.size() == 0)
    {
        // dragon: because I'm fucking sick and tired of Render2D-init crashing (when I don't even need it)
        return;
    }    

    /*at least in theory flag changes can lead to changes in number of render passes*/
    activeVariantInstance = nullptr;
    activeVariantName = FastName();
    for (auto& variant : renderVariants)
    {
        delete variant.second;
    }
    renderVariants.clear();

    for (auto& variantDescr : fxDescr.renderPassDescriptors)
    {
        RenderVariantInstance *variant = new RenderVariantInstance();                
        variant->renderLayer = variantDescr.renderLayer;
        variant->depthState = rhi::AcquireDepthStencilState(variantDescr.depthStateDescriptor);
        variant->shader = variantDescr.shader;
        variant->cullMode = variantDescr.cullMode;
        renderVariants[variantDescr.passName] = variant;
    }

    ClearLocalBuffers();
    activeVariantName = FastName();    
    activeVariantInstance = nullptr;
    needRebuildVariants = false;
    needRebuildBindings = true;
    needRebuildTextures = true;
}

void NMaterial::CollectMaterialFlags(HashMap<FastName, int32>& target)
{
    if (parent)
        parent->CollectMaterialFlags(target);
    for (auto &it : localFlags)
        target[it.first] = it.second;
}

void NMaterial::RebuildBindings()
{
    ClearLocalBuffers();
    for (auto& variant : renderVariants)
    {
        RenderVariantInstance* currRenderVariant = variant.second;
        ShaderDescriptor *currShader = currRenderVariant->shader;
        if (!currShader->IsValid()) //cant build for empty shader
            continue;
        currRenderVariant->vertexConstBuffers.resize(currShader->GetVertexConstBuffersCount());
        currRenderVariant->fragmentConstBuffers.resize(currShader->GetFragmentConstBuffersCount());

        for (auto& bufferDescr : currShader->GetConstBufferDescriptors())
        {
            rhi::HConstBuffer bufferHandle;
            MaterialBufferBinding* bufferBinding = nullptr;
            //for static buffers resolve sharing and bindings
            if (bufferDescr.updateType == rhi::ShaderProp::STORAGE_STATIC)
            {

                bufferBinding = GetConstBufferBinding(bufferDescr.propertyLayoutId);
                bool needLocalOverride = NeedLocalOverride(bufferDescr.propertyLayoutId);
                //Create local buffer and build it's bindings if required;
                if ((bufferBinding == nullptr) || needLocalOverride)
                {
                    //create buffer
                    bufferBinding = new MaterialBufferBinding();

                    //create handles
                    if (bufferDescr.type == ConstBufferDescriptor::Type::Vertex)
                        bufferBinding->constBuffer = rhi::CreateVertexConstBuffer(currShader->GetPiplineState(), bufferDescr.targetSlot);
                    else
                        bufferBinding->constBuffer = rhi::CreateFragmentConstBuffer(currShader->GetPiplineState(), bufferDescr.targetSlot);

                    //create bindings for this buffer
                    for (auto& propDescr : ShaderDescriptor::GetProps(bufferDescr.propertyLayoutId))
                    {
                        NMaterialProperty *prop = GetMaterialProperty(propDescr.uid);
                        if ((prop != nullptr)) //has property of the same type
                        {
                            DVASSERT(prop->type == propDescr.type);

                            // create property binding

                            bufferBinding->propBindings.emplace_back(propDescr.type, 
								propDescr.bufferReg, propDescr.bufferRegCount, 0, prop);
                        }
                        else
                        {
                            //just set default property to const buffer
                            if (propDescr.type < rhi::ShaderProp::TYPE_FLOAT4)
                            {                                
                                rhi::UpdateConstBuffer1fv(bufferBinding->constBuffer, propDescr.bufferReg, propDescr.bufferRegCount, propDescr.defaultValue, ShaderDescriptor::CalculateDataSize(propDescr.type, 1));
                            }
                            else
                            {
                                rhi::UpdateConstBuffer4fv(bufferBinding->constBuffer, propDescr.bufferReg, propDescr.defaultValue, propDescr.bufferRegCount);
                            }
                        }
                    }

                    //store it locally or at parent
                    if (needLocalOverride || (!parent))
                    {
                        //buffer should be handled locally
                        localConstBuffers[bufferDescr.propertyLayoutId] = bufferBinding;
                    }
                    else
                    {
                        //buffer can be propagated upward
                        parent->InjectChildBuffer(bufferDescr.propertyLayoutId, bufferBinding);
                    }
                }
                currRenderVariant->materialBufferBindings.push_back(bufferBinding);

                bufferHandle = bufferBinding->constBuffer;
            }

            else //if (bufferDescr.updateType == ConstBufferDescriptor::ConstBufferUpdateType::Static)
            {
                //for dynamic buffers just copy it's handle to corresponding slot
                bufferHandle = currShader->GetDynamicBuffer(bufferDescr.type, bufferDescr.targetSlot);
            }

            DVASSERT(bufferHandle.IsValid());
            if (bufferDescr.type == ConstBufferDescriptor::Type::Vertex)
                currRenderVariant->vertexConstBuffers[bufferDescr.targetSlot] = bufferHandle;
            else
                currRenderVariant->fragmentConstBuffers[bufferDescr.targetSlot] = bufferHandle;

        }
    }

    needRebuildBindings = false;

}

void NMaterial::RebuildTextureBindings()
{
    for (auto& variant : renderVariants)
    {
        RenderVariantInstance* currRenderVariant = variant.second;
        
        //release existing        
        rhi::ReleaseTextureSet(currRenderVariant->textureSet);        
        rhi::ReleaseSamplerState(currRenderVariant->samplerState);
        

        ShaderDescriptor *currShader = currRenderVariant->shader;
        if (!currShader->IsValid()) //cant build for empty shader
            continue;
        rhi::TextureSetDescriptor textureDescr;        
        rhi::SamplerState::Descriptor samplerDescr;
        const rhi::ShaderSamplerList& fragmentSamplerList = currShader->GetFragmentSamplerList();
        const rhi::ShaderSamplerList& vertexSamplerList = currShader->GetVertexSamplerList();

        textureDescr.fragmentTextureCount = fragmentSamplerList.size();       
        samplerDescr.fragmentSamplerCount = fragmentSamplerList.size();
        for (size_t i = 0, sz = textureDescr.fragmentTextureCount; i < sz; ++i)
        {       
            RuntimeTextures::eDynamicTextureSemantic textureSemantic = RuntimeTextures::GetDynamicTextureSemanticByName(currShader->GetFragmentSamplerList()[i].uid);
            if (textureSemantic == RuntimeTextures::TEXTURE_STATIC)
            {
                Texture *tex = GetEffectiveTexture(fragmentSamplerList[i].uid);
                if (tex)
                {
                    textureDescr.fragmentTexture[i] = tex->handle;
                    samplerDescr.fragmentSampler[i] = tex->samplerState;                  
                }
                else
                {
                    textureDescr.fragmentTexture[i] = Renderer::GetRuntimeTextures().GetPinkTexture(fragmentSamplerList[i].type);
                    samplerDescr.fragmentSampler[i] = Renderer::GetRuntimeTextures().GetPinkTextureSamplerState(fragmentSamplerList[i].type);

                    Logger::Debug(" no texture for slot : %s", fragmentSamplerList[i].uid.c_str());
                }
            }
            else
            {
                textureDescr.fragmentTexture[i] = Renderer::GetRuntimeTextures().GetDynamicTexture(textureSemantic);
                samplerDescr.fragmentSampler[i] = Renderer::GetRuntimeTextures().GetDynamicTextureSamplerState(textureSemantic);
            }
            DVASSERT(textureDescr.fragmentTexture[i].IsValid());                        
        }


        textureDescr.vertexTextureCount = vertexSamplerList.size();
        for (size_t i = 0, sz = textureDescr.vertexTextureCount; i < sz; ++i)
        {
            Texture *tex = GetEffectiveTexture(vertexSamplerList[i].uid);
            if (tex)
                textureDescr.vertexTexture[i] = tex->handle;
            else
                textureDescr.vertexTexture[i] = Renderer::GetRuntimeTextures().GetPinkTexture(vertexSamplerList[i].type);

        }                            


        currRenderVariant->textureSet = rhi::AcquireTextureSet(textureDescr);
        currRenderVariant->samplerState = rhi::AcquireSamplerState(samplerDescr);
    }

    needRebuildTextures = false;
}

bool NMaterial::PreBuildMaterial(const FastName& passName)
{
    
    //shader rebuild first - as it sets needRebuildBindings and needRebuildTextures
    if (needRebuildVariants)
        RebuildRenderVariants();
    if (needRebuildBindings)
        RebuildBindings();
    if (needRebuildTextures)
        RebuildTextureBindings();

    bool res = (activeVariantInstance != nullptr) && (activeVariantInstance->shader->IsValid());
    if (activeVariantName != passName)
    {
        RenderVariantInstance *targetVariant = renderVariants[passName];
        
        if (targetVariant!=nullptr)
        {
            activeVariantName = passName;
            activeVariantInstance = targetVariant;

            res = (activeVariantInstance->shader->IsValid());
        }
        else
        {
            res = false;
        }
        
    }
    return res;
}

NMaterial* NMaterial::Clone()
{
    NMaterial *clonedMaterial = new NMaterial();
    clonedMaterial->materialName = materialName;
    clonedMaterial->fxName = fxName;

    for (auto prop : localProperties)
        clonedMaterial->AddProperty(prop.first, prop.second->data.get(), prop.second->type, prop.second->arraySize);

    for (auto tex : localTextures)
    {
        MaterialTextureInfo *res = new MaterialTextureInfo();
        res->path = tex.second->path;
        res->texture = SafeRetain(tex.second->texture);
        clonedMaterial->localTextures[tex.first] = res;        
    }
        
    for (auto flag : localFlags)
        clonedMaterial->AddFlag(flag.first, flag.second);

    clonedMaterial->SetParent(parent);

    // DataNode properties
    clonedMaterial->id = 0;
    clonedMaterial->scene = scene;
    clonedMaterial->isRuntime = isRuntime;

    return clonedMaterial;
}

void NMaterial::Save(KeyedArchive * archive, SerializationContext * serializationContext)
{
    DataNode::Save(archive, serializationContext);

    if (parent)
        archive->SetUInt64(MaterialSerializationKey::ParentMaterialKey, parent->GetNodeID());

    if (materialName.IsValid())
        archive->SetString(MaterialSerializationKey::MaterialName, materialName.c_str());

    if (fxName.IsValid())
        archive->SetString(MaterialSerializationKey::FXName, fxName.c_str());

    if (qualityGroup.IsValid())
        archive->SetString(MaterialSerializationKey::QualityGroup, qualityGroup.c_str());

    ScopedPtr<KeyedArchive> propertiesArchive(new KeyedArchive());
    for (HashMap<FastName, NMaterialProperty*>::iterator it = localProperties.begin(), itEnd = localProperties.end(); it != itEnd; ++it)
    {
        NMaterialProperty* property = it->second;

        uint32 dataSize = ShaderDescriptor::CalculateDataSize(property->type, property->arraySize) * sizeof(float32);
        uint32 storageSize = sizeof(uint8) + sizeof(uint32) + dataSize;
        uint8* propertyStorage = new uint8[storageSize];

        memcpy(propertyStorage, &property->type, sizeof(uint8));
        memcpy(propertyStorage + sizeof(uint8), &property->arraySize, sizeof(uint32));
        memcpy(propertyStorage + sizeof(uint8) + sizeof(uint32), property->data.get(), dataSize);

        propertiesArchive->SetByteArray(it->first.c_str(), propertyStorage, storageSize);

        SafeDeleteArray(propertyStorage);
    }
    archive->SetArchive("properties", propertiesArchive);

    ScopedPtr<KeyedArchive> texturesArchive(new KeyedArchive());
    for (auto it = localTextures.begin(), itEnd = localTextures.end(); it != itEnd; ++it)
    {        
        if (!it->second->path.IsEmpty())
        {
            String textureRelativePath = it->second->path.GetRelativePathname(serializationContext->GetScenePath());
            if (textureRelativePath.size() > 0)
            {
                texturesArchive->SetString(it->first.c_str(), textureRelativePath);
            }
        }
    }
    archive->SetArchive("textures", texturesArchive);

    ScopedPtr<KeyedArchive> flagsArchive(new KeyedArchive());
    for (HashMap<FastName, int32>::iterator it = localFlags.begin(), itEnd = localFlags.end(); it != itEnd; ++it)
    {
        flagsArchive->SetInt32(it->first.c_str(), it->second);
    }
    archive->SetArchive("flags", flagsArchive);
}

void NMaterial::Load(KeyedArchive * archive, SerializationContext * serializationContext)
{
    DataNode::Load(archive, serializationContext);

    if (serializationContext->GetVersion() < RHI_SCENE_VERSION)
    {
        LoadOldNMaterial(archive, serializationContext);
        return;
    }

    if (archive->IsKeyExists(MaterialSerializationKey::MaterialName))
    {
        materialName = FastName(archive->GetString(MaterialSerializationKey::MaterialName));
    }

    if (archive->IsKeyExists(MaterialSerializationKey::MaterialKey))
    {
        uint64 materialKey = archive->GetUInt64(MaterialSerializationKey::MaterialKey);
        id = materialKey;
    }

    uint64 parentKey(0);
    if (archive->IsKeyExists(MaterialSerializationKey::ParentMaterialKey))
    {
        parentKey = archive->GetUInt64(MaterialSerializationKey::ParentMaterialKey);
    }
    serializationContext->AddBinding(parentKey, this); //parentKey == 0 is global material if it exists, or no-parent otherwise

    if (archive->IsKeyExists(MaterialSerializationKey::QualityGroup))
    {
        qualityGroup = FastName(archive->GetString(MaterialSerializationKey::QualityGroup).c_str());
    }

    if (archive->IsKeyExists(MaterialSerializationKey::FXName))
    {
        fxName = FastName(archive->GetString(MaterialSerializationKey::FXName).c_str());
    }

    if (archive->IsKeyExists("properties"))
    {
        const Map<String, VariantType*>& propsMap = archive->GetArchive("properties")->GetArchieveData();
        for (Map<String, VariantType*>::const_iterator it = propsMap.begin(); it != propsMap.end(); ++it)
        {
            const VariantType* propVariant = it->second;
            DVASSERT(VariantType::TYPE_BYTE_ARRAY == propVariant->type);
            DVASSERT(propVariant->AsByteArraySize() >= static_cast<int32>(sizeof(uint8) + sizeof(uint32)));

            const uint8* ptr = propVariant->AsByteArray();

            FastName propName = FastName(it->first);

            uint8 propType = *ptr; 
			ptr += sizeof(uint8);

            uint32 propSize = *(uint32*)ptr;
			ptr += sizeof(uint32);

            float32 *data = (float32*)ptr;
            AddProperty(propName, data, (rhi::ShaderProp::Type)propType, propSize);
        }
    }

    if (archive->IsKeyExists("textures"))
    {
        const Map<String, VariantType*>& texturesMap = archive->GetArchive("textures")->GetArchieveData();
        for (Map<String, VariantType*>::const_iterator it = texturesMap.begin(); it != texturesMap.end(); ++it)
        {
            String relativePathname = it->second->AsString();
            MaterialTextureInfo *texInfo = new MaterialTextureInfo();            
            texInfo->path = serializationContext->GetScenePath() + relativePathname;
            localTextures[FastName(it->first)] = texInfo;            
        }
    }

    if (archive->IsKeyExists("flags"))
    {
        const Map<String, VariantType*>& flagsMap = archive->GetArchive("flags")->GetArchieveData();
        for (Map<String, VariantType*>::const_iterator it = flagsMap.begin(); it != flagsMap.end(); ++it)
        {
			AddFlag(FastName(it->first), it->second->AsInt32());
        }
    }
}

void NMaterial::LoadOldNMaterial(KeyedArchive * archive, SerializationContext * serializationContext)
{    
    /*the following stuff is for importing old NMaterial stuff*/

    if (archive->IsKeyExists(MaterialSerializationKey::MaterialName))
    {
        materialName = FastName(archive->GetString(MaterialSerializationKey::MaterialName));
    }

    if (archive->IsKeyExists(MaterialSerializationKey::MaterialKey))
    {
        uint64 materialKey = archive->GetUInt64(MaterialSerializationKey::MaterialKey);
        id = materialKey;
    }

    int32 oldType = 0;
    if (archive->IsKeyExists("materialType"))
    {
        oldType = archive->GetInt32("materialType");
    }

    uint64 parentKey(0);
    if (archive->IsKeyExists(MaterialSerializationKey::ParentMaterialKey))
    {
        parentKey = archive->GetUInt64(MaterialSerializationKey::ParentMaterialKey);
    }
    serializationContext->AddBinding(parentKey, this); //parentKey == 0 is global material if it exists, or no-parent otherwise

    if (archive->IsKeyExists("materialGroup"))
    {
        qualityGroup = FastName(archive->GetString("materialGroup").c_str());
    }

    // don't load fxName from material instance (type = 2)
    if (archive->IsKeyExists("materialTemplate") && oldType != 2)
    {
        fxName = FastName(archive->GetString("materialTemplate").c_str());        
    }    

    if (archive->IsKeyExists("textures"))
    {
        const Map<String, VariantType*>& texturesMap = archive->GetArchive("textures")->GetArchieveData();
        for (Map<String, VariantType*>::const_iterator it = texturesMap.begin();
            it != texturesMap.end();
            ++it)
        {
            String relativePathname = it->second->AsString();
            MaterialTextureInfo *texInfo = new MaterialTextureInfo();
            texInfo->path = serializationContext->GetScenePath() + relativePathname;
            localTextures[FastName(it->first)] = texInfo;
        }
    }

    if (archive->IsKeyExists("setFlags"))
    {
        const Map<String, VariantType*>& flagsMap = archive->GetArchive("setFlags")->GetArchieveData();
        for (Map<String, VariantType*>::const_iterator it = flagsMap.begin(); it != flagsMap.end(); ++it)
        {
            AddFlag(FastName(it->first), it->second->AsInt32());
        }
    }
    //NMaterial hell - for some reason property types were saved as GL_XXX defines O_o
    const uint32 originalTypesCount = 5;
    struct { uint32 originalType; rhi::ShaderProp::Type newType;} propertyTypeRemapping[originalTypesCount] =
    {
        { 0x1406/*GL_FLOAT*/, rhi::ShaderProp::TYPE_FLOAT1},
        { 0x8B50/*GL_FLOAT_VEC2*/, rhi::ShaderProp::TYPE_FLOAT2},
        { 0x8B51/*GL_FLOAT_VEC3*/, rhi::ShaderProp::TYPE_FLOAT3},
        { 0x8B52/*GL_FLOAT_VEC4*/, rhi::ShaderProp::TYPE_FLOAT4},
        { 0x8B5C/*GL_FLOAT_MAT4*/, rhi::ShaderProp::TYPE_FLOAT4X4}
    };
           
    Array<FastName, 8> propertyFloat4toFloat3 =
    {{
        NMaterialParamName::PARAM_FOG_COLOR,
        NMaterialParamName::PARAM_FOG_ATMOSPHERE_COLOR_SKY,
        NMaterialParamName::PARAM_FOG_ATMOSPHERE_COLOR_SUN,
        NMaterialParamName::PARAM_DECAL_TILE_COLOR,
        Landscape::PARAM_TILE_COLOR0,
        Landscape::PARAM_TILE_COLOR1,
        Landscape::PARAM_TILE_COLOR2,
        Landscape::PARAM_TILE_COLOR3,
    }};
    Array<FastName, 1> propertyFloat3toFloat4 =
    {{
        NMaterialParamName::PARAM_FLAT_COLOR,
    }};

    if (archive->IsKeyExists("properties"))
    {
        const Map<String, VariantType*>& propsMap = archive->GetArchive("properties")->GetArchieveData();
        for (Map<String, VariantType*>::const_iterator it = propsMap.begin(); it != propsMap.end(); ++it)
        {            
            const VariantType* propVariant = it->second;
            DVASSERT(VariantType::TYPE_BYTE_ARRAY == propVariant->type);
            DVASSERT(propVariant->AsByteArraySize() >= static_cast<int32>(sizeof(uint32) + sizeof(uint32)));

            const uint8* ptr = propVariant->AsByteArray();
            
            FastName propName = FastName(it->first);
            uint32 propType = *(uint32*)ptr; ptr += sizeof(uint32);
            uint8 propSize = *(uint8*)ptr; ptr += sizeof(uint8);
            float32 *data = (float32*)ptr;            
            for (uint32 i = 0; i < originalTypesCount; i++)
            {
                if (propType == propertyTypeRemapping[i].originalType)
                {
                    if (propertyTypeRemapping[i].newType == rhi::ShaderProp::TYPE_FLOAT4)
                    {
                        if (std::find(propertyFloat4toFloat3.begin(), propertyFloat4toFloat3.end(), propName) != propertyFloat4toFloat3.end())
                        {
                            AddProperty(propName, data, rhi::ShaderProp::TYPE_FLOAT3, 1);
                            continue;
                        }
                    }
                    else if (propertyTypeRemapping[i].newType == rhi::ShaderProp::TYPE_FLOAT3)
                    {
                        if (std::find(propertyFloat3toFloat4.begin(), propertyFloat3toFloat4.end(), propName) != propertyFloat3toFloat4.end())
                        {
                            float32 data4[4];
                            Memcpy(data4, data, 3 * sizeof(float32));
                            data[3] = 1.f;

                            AddProperty(propName, data, rhi::ShaderProp::TYPE_FLOAT4, 1);
                            continue;
                        }
                    }

                    AddProperty(propName, data, propertyTypeRemapping[i].newType, 1);
                }
            }

        }
    }

    if (archive->IsKeyExists("illumination.isUsed"))
        AddFlag(NMaterialFlagName::FLAG_ILLUMINATION_USED, archive->GetBool("illumination.isUsed"));

    if (archive->IsKeyExists("illumination.castShadow"))
        AddFlag(NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_CASTER, archive->GetBool("illumination.castShadow"));

    if (archive->IsKeyExists("illumination.receiveShadow"))
        AddFlag(NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_RECEIVER, archive->GetBool("illumination.receiveShadow"));

    if (archive->IsKeyExists("illumination.lightmapSize"))
    {
        float32 lighmapSize = (float32)archive->GetInt32("illumination.lightmapSize");
        if (HasLocalProperty(NMaterialParamName::PARAM_LIGHTMAP_SIZE))
            SetPropertyValue(NMaterialParamName::PARAM_LIGHTMAP_SIZE, &lighmapSize);
        else
            AddProperty(NMaterialParamName::PARAM_LIGHTMAP_SIZE, &lighmapSize, rhi::ShaderProp::TYPE_FLOAT1, 1);
    }
}

const HashMap<FastName, int32>& NMaterial::GetLocalFlags() const
{
    return localFlags;
}
};
