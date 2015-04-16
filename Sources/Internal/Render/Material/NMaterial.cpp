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



#include "Scene3D/Systems/MaterialSystem.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Render/Material/NMaterial.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/RenderBase.h"
#include "FileSystem/YamlParser.h"
#include "Render/Shader.h"
#include "Render/Material/MaterialGraph.h"
#include "Render/Material/MaterialCompiler.h"
#include "Render/ShaderCache.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Light.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Utils/StringFormat.h"
#include "Render/Material/NMaterialTemplate.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/TextureDescriptor.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Material/NMaterialNames.h"

namespace DAVA
{

uint32 NMaterialProperty::globalPropertyUpdateSemanticCounter = 0;

NMaterial::NMaterial()
    : parent(nullptr)
    , localProperties(16, nullptr)
    , localConstBuffers(16, nullptr)
    , localTextures(8, nullptr)
    , needRebuildBindings(true)
    , needRebuildTextures(true)
    , needRebuildShader(true)
{

    /*for now we just init with one default render variant - later possible variants be read from .fx config and modified by engine flags*/
    RenderVariantInstance *renderVariant = new RenderVariantInstance();

    renderVariant->depthState = rhi::AcquireDepthStencilState(rhi::DepthStencilState::Descriptor());
    renderVariant->samplerState = rhi::AcquireSamplerState(rhi::SamplerState::Descriptor());   
    renderVariant->renderLayer = RenderLayerManager::GetLayerIDByName(LAYER_OPAQUE);

    activeVariantInstance = renderVariant;
    renderVariants[FastName("FORWARD_PASS")] = renderVariant;

    materialKey = (uint64)this;
}

NMaterial::~NMaterial()
{
    //RHI_COMPLETE
}

void NMaterial::BindParams(rhi::Packet& target)
{
    //Logger::Info( "bind-params" );
    DVASSERT(activeVariantInstance);       //trying to bind config-only material 

    //shader rebuild first - as it sets needRebuildBindings and needRebuildTextures
    if (needRebuildShader)
        RebuildShader();

    if (needRebuildBindings)
        RebuildBindings();
    if (needRebuildTextures)
        RebuildTextureBindings();

    /*set pipeline state*/
    target.renderPipelineState = activeVariantInstance->shader->GetPiplineState();
    target.depthStencilState = activeVariantInstance->depthState;
    target.samplerState = activeVariantInstance->samplerState;
    target.textureSet = activeVariantInstance->textureSet;

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
                    rhi::UpdateConstBuffer(materialBufferBinding->constBuffer, materialBinding.reg, materialBinding.source->data.get(), ShaderDescriptor::CalculateRegsCount(materialBinding.type, materialBinding.source->arraySize));
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

Texture* NMaterial::GetMaterialTexture(const FastName& slotName)
{
    Texture* res = localTextures.at(slotName);
    if ((res == nullptr) && (parent != nullptr))
    {
        res = parent->GetMaterialTexture(slotName);
    }
    return res;
}

void NMaterial::AddProperty(const FastName& propName, float32 *propData, rhi::ShaderProp::Type type, uint32 arraySize)
{
    DVASSERT(localProperties.at(propName) == nullptr);
    NMaterialProperty *prop = new NMaterialProperty();
    prop->name = propName;
    prop->type = type;
    prop->arraySize = arraySize;
    prop->data.reset(new float[sizeof(float32) * 4 * ShaderDescriptor::CalculateRegsCount(type, arraySize)]);
    prop->SetPropertyValue(propData);
    localProperties[propName] = prop;
}

void NMaterial::RemoveProperty(const FastName& propName)
{

    NMaterialProperty *prop = localProperties.at(propName);
    DVASSERT(prop != nullptr);
    localProperties.erase(propName);
    SafeDelete(prop);

    InvalidateBufferBindings();
}

void NMaterial::SetPropertyValue(const FastName& propName, float32 *propData)
{
    NMaterialProperty *prop = localProperties.at(propName);
    DVASSERT(prop != nullptr);
    prop->SetPropertyValue(propData);
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

void NMaterial::AddFlag(const FastName& flagName, int32 value)
{
    DVASSERT(localFlags.find(flagName) == localFlags.end());
    localFlags[flagName] = value;
    InvalidateShader();
}
void NMaterial::RemoveFlag(const FastName& flagName)
{
    DVASSERT(localFlags.find(flagName) != localFlags.end());
    localFlags.erase(flagName);
    InvalidateShader();
}
void NMaterial::SetFlag(const FastName& flagName, int32 value)
{
    DVASSERT(localFlags.find(flagName) != localFlags.end());
    localFlags[flagName] = value;
    InvalidateShader();
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
        parent->RemoveChildMaterial(this);

    parent = _parent;
    sortingKey = (uint32)parent;

    if (parent)
        parent->AddChildMaterial(this);
}

NMaterial* NMaterial::GetParent()
{
    return parent;
}

void NMaterial::AddChildMaterial(NMaterial *material)
{    
    DVASSERT(material);
    children.push_back(SafeRetain(material));
    material->InvalidateBufferBindings();
    material->InvalidateTextureBindings();
}

void NMaterial::RemoveChildMaterial(NMaterial *material)
{
    bool res = FindAndRemoveExchangingWithLast(children, material);
    DVASSERT(res);
    SafeRelease(material);
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

void NMaterial::InvalidateShader()
{
    //release existing descriptor?
    needRebuildShader = true;
    for (auto& child : children)
        child->InvalidateShader();
}

void NMaterial::RebuildShader()
{
    HashMap<FastName, int32> flags;
    CollectMaterialFlags(flags);
    for (auto& variant : renderVariants)
    {
        RenderVariantInstance* currRenderVariant = variant.second;
        currRenderVariant->shader = ShaderDescriptorCache::Instance()->GetShaderDescriptor(FastName("material.wgfx-blitz"), flags);
    }
    needRebuildShader = false;
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
            if (bufferDescr.updateType == ConstBufferDescriptor::UpdateType::Static)
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
                        if ((prop != nullptr) && (prop->type == propDescr.type)) //has property of the same type
                        {
                            //create property binding
                            MaterialPropertyBinding binding;
                            binding.type = propDescr.type;
                            binding.reg = propDescr.bufferReg;
                            binding.updateSemantic = 0; //mark as dirty - set it on next draw request
                            binding.source = prop;
                            bufferBinding->propBindings.push_back(binding);
                        }
                        else
                        {
                            //just set default property to const buffer
                            rhi::UpdateConstBuffer(bufferBinding->constBuffer, propDescr.bufferReg, propDescr.defaultValue, propDescr.bufferRegCount);
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
        ShaderDescriptor *currShader = currRenderVariant->shader;
        if (!currShader) //cant build for empty shader
            continue;
        rhi::TextureSetDescriptor descr;
        descr.count = currShader->fragmentSamplerList.size();
        for (size_t i = 0, sz = descr.count; i < sz; ++i)
            descr.texture[i] = GetMaterialTexture(currShader->fragmentSamplerList[i].uid)->handle;
        currRenderVariant->textureSet = rhi::AcquireTextureSet(descr);
    }

    needRebuildTextures = false;
}

void NMaterial::Load(KeyedArchive * archive, SerializationContext * serializationContext)
{
    //RHI_COMPLETE
    DataNode::Load(archive, serializationContext);
    if (archive->IsKeyExists("materialKey"))
    {
        materialKey = archive->GetUInt64("materialKey");
        pointer = materialKey;
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
}

};
