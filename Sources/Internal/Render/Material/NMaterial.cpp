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
#include "Render/Material/NMaterialNames.h"

#include "Render/Highlevel/Landscape.h"

#include "Render/Material/FXCache.h"
#include "Render/Shader.h"
#include "Render/Texture.h"
#include "Scene3D/SceneFile/SerializationContext.h"

#include "Utils/Utils.h"
#include "Utils/StringFormat.h"
#include "FileSystem/YamlParser.h"



namespace DAVA
{

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
    : parent(nullptr)
    , localProperties(16, nullptr)
    , localConstBuffers(16, nullptr)
    , localTextures(8, nullptr)
    , localFlags(16, 0)
    , renderVariants(4, nullptr)
    , needRebuildBindings(true)
    , needRebuildTextures(true)
    , needRebuildVariants(true)
{    
    materialKey = (uint64)this;
}

NMaterial::~NMaterial()
{
    SetParent(nullptr);
    DVASSERT(children.size() == 0); //as children refernce parent in our material scheme, this should not be released while it has children
    for (auto& prop : localProperties)
        SafeDelete(prop.second);
    for (auto& tex : localTextures)
        SafeRelease(tex.second);
       
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

    /*set pipeline state*/
    target.renderPipelineState = activeVariantInstance->shader->GetPiplineState();
    target.depthStencilState = activeVariantInstance->depthState;
    target.samplerState = activeVariantInstance->samplerState;
    target.textureSet = activeVariantInstance->textureSet;
    target.cullMode = activeVariantInstance->cullMode;

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
    Texture* res = localTextures.at(slotName);
    if ((res == nullptr) && (parent != nullptr))
    {
        res = parent->GetEffectiveTexture(slotName);
    }
    return res;
}

void NMaterial::SetFXName(const FastName & fx)
{
    fxName = fx;
    InvalidateRenderVariants();
}

const FastName& NMaterial::GetFXName()
{   
    if ((!fxName.IsValid()) && (parent != nullptr))
    {
        return parent->GetFXName();
    }
    return fxName;
}

const FastName& NMaterial::GetQualityGroup()
{    
    if ((!qualityGroup.IsValid()) && (parent != nullptr))
    {
        return parent->GetQualityGroup();
    }
    return qualityGroup;
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
    
    InvalidateBufferBindings();
}

void NMaterial::RemoveProperty(const FastName& propName)
{

    NMaterialProperty *prop = localProperties.at(propName);
    DVASSERT(prop != nullptr);
    localProperties.erase(propName);
    SafeDelete(prop);

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
    localTextures[slotName] = SafeRetain(texture);
    InvalidateTextureBindings();

}
void NMaterial::RemoveTexture(const FastName& slotName)
{
    Texture * currTexture = localTextures.at(slotName);
    DVASSERT(currTexture != nullptr);
    localTextures.erase(slotName);
    SafeRelease(currTexture);    
    InvalidateTextureBindings();
}
void NMaterial::SetTexture(const FastName& slotName, Texture* texture)
{    
    Texture * currTexture = localTextures.at(slotName);
    DVASSERT(currTexture != nullptr);
    if (currTexture != texture)
    {
        SafeRelease(currTexture);
        localTextures[slotName] = SafeRetain(texture);
        InvalidateTextureBindings();
    }        
}

bool NMaterial::HasLocalTexture(const FastName& slotName)
{
    return localTextures.find(slotName) != localTextures.end();
}
Texture* NMaterial::GetLocalTexture(const FastName& slotName)
{
    DVASSERT(HasLocalTexture(slotName));
    return localTextures.at(slotName);
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
        
}

NMaterial* NMaterial::GetParent()
{
    return parent;
}

void NMaterial::AddChildMaterial(NMaterial *material)
{    
    DVASSERT(material);
    children.push_back(material);
    material->InvalidateRenderVariants();    
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
    ClearLocalBuffers();
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
    
    //RHI_COMPLETE - move quality to numbers, or flags to fastname
    flags[NMaterialQualityName::QUALITY_FLAG_NAME] = 0; // QualitySettingsSystem::Instance()->GetCurMaterialQuality(GetQualityGroup());        

    const FXDescriptor& fxDescr = FXCache::GetFXDescriptor(GetFXName(), flags);
    
    for (auto &sampler : fxDescr.renderPassDescriptors[0].shader->fragmentSamplerList)
    {
        if (sampler.uid == FastName("decal"))
            int ttt = 3;
    }

    /*at least in theory flag changes can lead to changes in number of render pa*/
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
    for (auto& variant : renderVariants)
    {
        RenderVariantInstance* currRenderVariant = variant.second;
        ShaderDescriptor *currShader = currRenderVariant->shader;
        if (!currShader) //cant build for empty shader
            continue;
        currRenderVariant->vertexConstBuffers.resize(currShader->GetVertexConstBuffersCount());
        currRenderVariant->fragmentConstBuffers.resize(currShader->GetFragmentConstBuffersCount());

        for (auto& bufferDescr : currShader->constBuffers)
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

                            //create property binding
                            MaterialPropertyBinding binding;
                            binding.type = propDescr.type;
                            binding.reg = propDescr.bufferReg;
                            binding.regCount = propDescr.bufferRegCount;
                            binding.updateSemantic = 0; //mark as dirty - set it on next draw request
                            binding.source = prop;
                            bufferBinding->propBindings.push_back(binding);
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
        if (!currShader) //cant build for empty shader
            continue;
        rhi::TextureSetDescriptor textureDescr;        
        rhi::SamplerState::Descriptor samplerDescr;
        textureDescr.fragmentTextureCount = currShader->fragmentSamplerList.size();       
        samplerDescr.fragmentSamplerCount = currShader->fragmentSamplerList.size();
        for (size_t i = 0, sz = textureDescr.fragmentTextureCount; i < sz; ++i)
        {       
            RuntimeTextures::eDynamicTextureSemantic textureSemantic = RuntimeTextures::GetDynamicTextureSemanticByName(currShader->fragmentSamplerList[i].uid);
            if (textureSemantic == RuntimeTextures::TEXTURE_STATIC)
            {
                Texture *tex = GetEffectiveTexture(currShader->fragmentSamplerList[i].uid);
                if (tex)
                {
                    textureDescr.fragmentTexture[i] = tex->handle;
                    samplerDescr.fragmentSampler[i] = tex->samplerState;                  
                }
                else
                {
                    textureDescr.fragmentTexture[i] = Renderer::GetRuntimeTextures().GetPinkTexture(currShader->fragmentSamplerList[i].type);
                    samplerDescr.fragmentSampler[i] = Renderer::GetRuntimeTextures().GetPinkTextureSamplerState(currShader->fragmentSamplerList[i].type);

                    Logger::Debug(" no texture for slot : %s", currShader->fragmentSamplerList[i].uid.c_str());
                }
            }
            else
            {
                textureDescr.fragmentTexture[i] = Renderer::GetRuntimeTextures().GetDynamicTexture(textureSemantic);
                samplerDescr.fragmentSampler[i] = Renderer::GetRuntimeTextures().GetDynamicTextureSamplerState(textureSemantic);
            }
            DVASSERT(textureDescr.fragmentTexture[i].IsValid());                        
        }


        textureDescr.vertexTextureCount = currShader->vertexSamplerList.size();
        for (size_t i = 0, sz = textureDescr.vertexTextureCount; i < sz; ++i)
        {
            Texture *tex = GetEffectiveTexture(currShader->vertexSamplerList[i].uid);
            DVASSERT(tex);
            textureDescr.vertexTexture[i] = tex->handle;
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
    
    bool res = (activeVariantInstance != nullptr);
    if (activeVariantName != passName)
    {
        RenderVariantInstance *targetVariant = renderVariants[passName];
        
        if (targetVariant!=nullptr)
        {
            activeVariantName = passName;
            activeVariantInstance = targetVariant;
            res = true;
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
        clonedMaterial->AddTexture(tex.first, tex.second);
    for (auto flag : localFlags)
        clonedMaterial->AddFlag(flag.first, flag.second);

    clonedMaterial->SetParent(parent);

    //DataNode properties
    clonedMaterial->pointer = pointer;
    clonedMaterial->scene = scene;
    clonedMaterial->index = index;
    clonedMaterial->nodeFlags = nodeFlags;

    return clonedMaterial;
}

void NMaterial::Save(KeyedArchive * archive, SerializationContext * serializationContext)
{
    DataNode::Save(archive, serializationContext);

    archive->SetUInt64("materialKey", GetMaterialKey());

    if (parent)
        archive->SetUInt64("parentMaterialKey", parent->GetMaterialKey());

    if (materialName.IsValid())
        archive->SetString("materialName", materialName.c_str());

    if (fxName.IsValid())
        archive->SetString("materialTemplate", fxName.c_str());

    if (qualityGroup.IsValid())
        archive->SetString("qualityGroup", qualityGroup.c_str());

    ScopedPtr<KeyedArchive> propertiesArchive(new KeyedArchive());
    for (HashMap<FastName, NMaterialProperty*>::iterator it = localProperties.begin(), itEnd = localProperties.end(); it != itEnd; ++it)
    {
        NMaterialProperty* property = it->second;

        uint32 dataSize = ShaderDescriptor::CalculateDataSize(property->type, property->arraySize);
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
    for (HashMap<FastName, Texture*>::iterator it = localTextures.begin(), itEnd = localTextures.end(); it != itEnd; ++it)
    {
        FilePath texturePath = it->second->GetPathname();
        if (!texturePath.IsEmpty())
        {
            String textureRelativePath = texturePath.GetRelativePathname(serializationContext->GetScenePath());
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


    if (archive->IsKeyExists("materialName"))
    {
        materialName = FastName(archive->GetString("materialName"));
    }

    if (archive->IsKeyExists("materialKey"))
    {
        materialKey = archive->GetUInt64("materialKey");
        pointer = materialKey;
    }

    uint64 parentKey(0);
    if (archive->IsKeyExists("parentMaterialKey"))
    {
        parentKey = archive->GetUInt64("parentMaterialKey");
    }
    serializationContext->AddBinding(parentKey, this); //parentKey == 0 is global material if it exists, or no-parent otherwise

    if (archive->IsKeyExists("materialGroup"))
    {
        qualityGroup = FastName(archive->GetString("materialGroup").c_str());
    }

    if (archive->IsKeyExists("materialTemplate"))
    {
        fxName = FastName(archive->GetString("materialTemplate").c_str());
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
            uint8 propType = *ptr; ptr += sizeof(uint8);
            uint32 propSize = *(uint32*)ptr; ptr += sizeof(uint32);
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
            Texture* tx = Texture::CreateFromFile(serializationContext->GetScenePath() + relativePathname, FastName(""));
            AddTexture(FastName(it->first), tx);
            SafeRelease(tx);
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
    
    if (archive->IsKeyExists("materialName"))
    {
        materialName = FastName(archive->GetString("materialName"));
    }

    if (archive->IsKeyExists("materialKey"))
    {
        materialKey = archive->GetUInt64("materialKey");
        pointer = materialKey;
    }

    uint64 parentKey(0);
    if (archive->IsKeyExists("parentMaterialKey"))
    {
        parentKey = archive->GetUInt64("parentMaterialKey");
    }
    serializationContext->AddBinding(parentKey, this); //parentKey == 0 is global material if it exists, or no-parent otherwise

    if (archive->IsKeyExists("materialGroup"))
    {
        qualityGroup = FastName(archive->GetString("materialGroup").c_str());
    }

    if (archive->IsKeyExists("materialTemplate"))
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
            Texture* tx = Texture::CreateFromFile(serializationContext->GetScenePath() + relativePathname, FastName(""));
            AddTexture(FastName(it->first), tx);
            SafeRelease(tx);
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
           
    Array<FastName, 8> prepertyFloat4toFloat3 =
    {
        NMaterialParamName::PARAM_FOG_COLOR,
        NMaterialParamName::PARAM_FOG_ATMOSPHERE_COLOR_SKY,
        NMaterialParamName::PARAM_FOG_ATMOSPHERE_COLOR_SUN,
        NMaterialParamName::PARAM_DECAL_TILE_COLOR,
        Landscape::PARAM_TILE_COLOR0,
        Landscape::PARAM_TILE_COLOR1,
        Landscape::PARAM_TILE_COLOR2,
        Landscape::PARAM_TILE_COLOR3,
    };
    Array<FastName, 1> prepertyFloat3toFloat4 =
    {
        NMaterialParamName::PARAM_FLAT_COLOR,
    };

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
                    bool float3toFloat4 = false, float4toFloat3 = false;

                    for (const FastName & name : prepertyFloat4toFloat3)
                    {
                        if (name == propName && propertyTypeRemapping[i].newType == rhi::ShaderProp::TYPE_FLOAT4)
                        {
                            float4toFloat3 = true;
                            break;
                        }
                    }

                    for (const FastName & name : prepertyFloat3toFloat4)
                    {
                        if (name == propName && propertyTypeRemapping[i].newType == rhi::ShaderProp::TYPE_FLOAT3)
                        {
                            float3toFloat4 = true;
                            break;
                        }
                    }

                    if (float3toFloat4)
                    {
                        float32 data4[4];
                        Memcpy(data4, data, 3 * sizeof(float32));
                        data[3] = 1.f;

                        AddProperty(propName, data, rhi::ShaderProp::TYPE_FLOAT4, 1);
                    }
                    else if (float4toFloat3)
                    {
                        AddProperty(propName, data, rhi::ShaderProp::TYPE_FLOAT3, 1);
                    }
                    else
                    {
                        AddProperty(propName, data, propertyTypeRemapping[i].newType, 1);
                    }
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

};
