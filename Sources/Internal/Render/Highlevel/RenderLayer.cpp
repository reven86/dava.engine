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


#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/Camera.h"
#include "Base/Radix/Radix.h"
#include "Debug/Stats.h"
#include "Render/OcclusionQuery.h"

namespace DAVA
{
RenderLayer::RenderLayer(const FastName & _name, uint32 sortingFlags, RenderLayerID _id)
    :	name(_name)
    ,   flags(sortingFlags)
    ,   id(_id)
{
    
}
    
RenderLayer::~RenderLayer()
{
}

void RenderLayer::Draw(const FastName & ownerRenderPass, Camera * camera, RenderLayerBatchArray * renderLayerBatchArray)
{
    TIME_PROFILE("RenderLayer::Draw");
    
    renderLayerBatchArray->Sort(camera);

    FrameOcclusionQueryManager::Instance()->BeginQuery(name);    
    DrawRenderBatchArray(ownerRenderPass, camera, renderLayerBatchArray);
    FrameOcclusionQueryManager::Instance()->EndQuery(name);    
}

void RenderLayer::DrawRenderBatchArray(const FastName & ownerRenderPass, Camera * camera, RenderLayerBatchArray * renderLayerBatchArray)
{    
    for (uint32 k = 0, size = (uint32)renderLayerBatchArray->GetRenderBatchCount(); k < size; ++k)
    {
        RenderBatch * batch = renderLayerBatchArray->Get(k);
        batch->Draw(ownerRenderPass, camera);
    }
}


InstancedRenderLayer::InstancedRenderLayer(const FastName & name, uint32 sortingFlags, RenderLayerID id) : RenderLayer(name, sortingFlags, id)
{
    incomingUniformValues.resize(INSTANCE_PARAMETERS_COUNT);
    incomingAutobindUniforms.reserve(INSTANCE_PARAMETERS_COUNT);
}

void InstancedRenderLayer::StartInstancingGroup(RenderBatch *batch, const FastName & ownerRenderPass, Camera * camera)
{
    DVASSERT(currInstancesCount==0);
    NMaterial *material = batch->GetMaterial();    
    if ((!material->IsInstancingSupported())||(!(batch->GetPolygonGroup()||batch->GetRenderDataObject())))//just draw and forget
    {        
        batch->Draw(ownerRenderPass, camera);            
        return;
    }    
    batch->GetRenderObject()->BindDynamicParameters(camera);
    material->SetActiveMaterialTechnique(ownerRenderPass, NMaterial::EF_INSTANCING);
    material->BindActivePassRenderState();        
    material->BindActivePassMaterialProperties();

    incomingAutobindUniforms.clear();
    Shader *shader = material->GetActivePassShader();
    int32 instancedUniformesCount = shader->GetInstancingUniformCount();            
   
    for (int32 i=0; i<instancedUniformesCount; i++)    
    {
        Shader::Uniform* uniform = shader->GetInstancingUniform(i);
        if (Shader::IsAutobindUniform(INSTANCE_PARAM_DESCRIPTORS[uniform->instanceSemantic].originalSemantic))
            incomingAutobindUniforms.push_back(uniform);
        uint32 uniformDataSize = Shader::GetUniformTypeSize(uniform->type);
        incomingUniformValues[uniform->instanceSemantic].resize(uniformDataSize*MAX_INSTANCES_COUNT);
    }

    CollectInstanceParams(material);
    incomingGroup = batch;
    currInstancesCount = 1;

}

bool InstancedRenderLayer::AppendInstance(RenderBatch *batch, const FastName & ownerRenderPass, Camera * camera)
{
    if (currInstancesCount==MAX_INSTANCES_COUNT)
        return false;
    NMaterial *material = batch->GetMaterial();
    if (!material->IsInstancingSupported())
        return false;
    if (!(batch->GetPolygonGroup()||batch->GetRenderDataObject())) //no geometry - custom draw
        return false;
    if ((batch->GetPolygonGroup()!=incomingGroup->GetPolygonGroup())
        ||((!incomingGroup->GetPolygonGroup())&&(batch->GetRenderDataObject()!=incomingGroup->GetRenderDataObject())))
    {
        return false;
    }
    material->SetActiveMaterialTechnique(ownerRenderPass, NMaterial::EF_INSTANCING);
    NMaterial *incomingMaterial = incomingGroup->GetMaterial();
    if ((material->GetParent()!=incomingMaterial->GetParent())
      ||(material->GetActivePassRenderStateHandle()!=incomingMaterial->GetActivePassRenderStateHandle())
      ||(material->GetActivePassTextureStateHandle()!=incomingMaterial->GetActivePassTextureStateHandle()))
    {
        return false;
    }

    //try to bind dynamic params
    batch->GetRenderObject()->BindDynamicParameters(camera);
    Shader *shader = incomingMaterial->GetActivePassShader();    
    if (!shader->TestDynamicParamsInstancing())
        return false;

    //and finally here we know it can be append to instance group    
    CollectInstanceParams(material);
    currInstancesCount++;

    return true;
}

void InstancedRenderLayer::CollectInstanceParams(NMaterial *material)
{
    /*int32 instancedUniformesCount = incomingUniformValues.size();    
    for (int32 i=0; i<instancedUniformesCount; i++)
    {
        Shader::Uniform* uniform = incomingUniformValues[i].first;        
        uint32 uniformDataSize = Shader::GetUniformTypeSize(uniform->type);
        eShaderSemantic originalSemantic = INSTANCE_PARAM_DESCRIPTORS[uniform->instanceSemantic].originalSemantic;
        if (Shader::IsAutobindUniform(originalSemantic))
        {
            const void *data = RenderManager::GetDynamicParam(originalSemantic);
            DVASSERT(data);
            Memcpy(&incomingUniformValues[i].second[currInstancesCount*uniformDataSize], data, uniformDataSize);
        }
        else
        {
            NMaterialProperty *property = material->GetPropertyValue(INSTANCE_PARAM_DESCRIPTORS[uniform->instanceSemantic].originalName);
            DVASSERT(property);
            Memcpy(&incomingUniformValues[i].second[currInstancesCount*uniformDataSize], property->data, uniformDataSize);
        }
    }*/

    for (int32 i = 0, sz = incomingAutobindUniforms.size(); i<sz; ++i)
    {
        Shader::Uniform* uniform = incomingAutobindUniforms[i];
        uint32 uniformDataSize = Shader::GetUniformTypeSize(uniform->type);        
        const void *data = RenderManager::GetDynamicParam(INSTANCE_PARAM_DESCRIPTORS[uniform->instanceSemantic].originalSemantic);
        DVASSERT(data);
        Memcpy(&(incomingUniformValues[uniform->instanceSemantic][currInstancesCount*uniformDataSize]), data, uniformDataSize);
    }

    const Vector<UniformCacheEntry>& incomingMaterialUniforms = material->GetActivePassInstancedUniforms();
    for (int32 i = 0, sz = incomingMaterialUniforms.size(); i<sz; ++i)
    {
        if (incomingMaterialUniforms[i].prop) //if is to support uninitialized properties - at least it's done that way in material somewhy
        {
            Shader::Uniform* uniform = incomingMaterialUniforms[i].uniform;
            uint32 uniformDataSize = Shader::GetUniformTypeSize(uniform->type);        
            const void *data = incomingMaterialUniforms[i].prop->data;            
            Memcpy(&incomingUniformValues[uniform->instanceSemantic][currInstancesCount*uniformDataSize], data, uniformDataSize);
        }
        
    }
}


void InstancedRenderLayer::CompleteInstancingGroup(const FastName & ownerRenderPass, Camera * camera)
{
    if (!incomingGroup)
        return;

    //incomingGroup->Draw(ownerRenderPass, camera);
    NMaterial *incomingMaterial = incomingGroup->GetMaterial();
    Shader *shader = incomingMaterial->GetActivePassShader(); 
    for (int32 i=0, sz = shader->GetInstancingUniformCount(); i<sz; ++i)
    {
        Shader::Uniform *uniform = shader->GetInstancingUniform(i);
        void *data = &(incomingUniformValues[uniform->instanceSemantic][0]);
        DVASSERT(incomingUniformValues[uniform->instanceSemantic].size()>=(uint32)Shader::GetUniformTypeSize(uniform->type)*currInstancesCount);
        shader->SetUniformValueByUniform(uniform, uniform->type, currInstancesCount, data);
    }

    //TODO this is mostly copy from RenderBatch::draw - rethink this
    RenderDataObject *renderData = incomingGroup->GetRenderDataObject();
    ePrimitiveType primType = PRIMITIVETYPE_TRIANGLELIST;
    PolygonGroup *dataSource = incomingGroup->GetPolygonGroup();       
    if (dataSource)
    {
        primType = dataSource->primitiveType;
        renderData = dataSource->renderDataObject;    
    }            
    DVASSERT(renderData);    
    RenderManager::Instance()->SetRenderData(renderData);
    RenderManager::Instance()->AttachRenderData();    
    
    void * indices = 0;
    if (!renderData->GetIndexBufferID())
        indices = renderData->GetIndices();   
    if (RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::DEBUG_DRAW_STATIC_OCCLUSION))
        RenderManager::Instance()->HWDrawElements(primType, renderData->GetIndexCount(), renderData->GetIndexFormat(), indices);
    else
        RenderManager::Instance()->HWDrawElementsInstanced(primType, renderData->GetIndexCount(), renderData->GetIndexFormat(), indices, currInstancesCount);

    incomingGroup = NULL;
    currInstancesCount = 0;
}

void InstancedRenderLayer::DrawRenderBatchArray(const FastName & ownerRenderPass, Camera * camera, RenderLayerBatchArray * renderLayerBatchArray)
{
    if (!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::TEST_OPTION))
    {    
        RenderLayer::DrawRenderBatchArray(ownerRenderPass, camera, renderLayerBatchArray);
        return;
    }        

    incomingGroup = NULL;
    currInstancesCount = 0;
        
    uint32 size = (uint32)renderLayerBatchArray->GetRenderBatchCount();   
    for (uint32 k = 0; k < size; ++k)
    {
        RenderBatch * batch = renderLayerBatchArray->Get(k);
        if (!incomingGroup)
        {   
            StartInstancingGroup(batch, ownerRenderPass, camera);
            continue;
        }

        if (!AppendInstance(batch, ownerRenderPass, camera))
        {
            CompleteInstancingGroup(ownerRenderPass, camera);                        
            StartInstancingGroup(batch, ownerRenderPass, camera);            
        }
    }    

    CompleteInstancingGroup(ownerRenderPass, camera);    

};


};
