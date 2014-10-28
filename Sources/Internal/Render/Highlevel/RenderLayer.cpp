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
    ,   queryPending(false)
    ,   lastFragmentsRenderedValue(0)
    ,   occlusionQuery(NULL)
{
    
}
    
RenderLayer::~RenderLayer()
{
    SafeRelease(occlusionQuery);
}

void RenderLayer::Draw(const FastName & ownerRenderPass, Camera * camera, RenderLayerBatchArray * renderLayerBatchArray)
{
    TIME_PROFILE("RenderLayer::Draw");
    
    renderLayerBatchArray->Sort(camera);
            
    RenderOptions* options = RenderManager::Instance()->GetOptions();
    bool layerOcclustionStatsEnabled = options->IsOptionEnabled(RenderOptions::LAYER_OCCLUSION_STATS);
    
    if(layerOcclustionStatsEnabled)
    {
        if(NULL == occlusionQuery)
        {
            occlusionQuery = new OcclusionQuery();
            occlusionQuery->Init();
        }
    
        if(false == queryPending)
        {
            occlusionQuery->BeginQuery();
        }
    }
    
    DrawRenderBatchArray(ownerRenderPass, camera, renderLayerBatchArray);
    
    if(layerOcclustionStatsEnabled)
    {
        if(false == queryPending)
        {
            occlusionQuery->EndQuery();
            queryPending = true;
        }
        
        if((true == queryPending) &&
           occlusionQuery->IsResultAvailable())
        {
            occlusionQuery->GetQuery(&lastFragmentsRenderedValue);
            queryPending = false;
        }
    }   
}

void RenderLayer::DrawRenderBatchArray(const FastName & ownerRenderPass, Camera * camera, RenderLayerBatchArray * renderLayerBatchArray)
{    
    for (uint32 k = 0, size = (uint32)renderLayerBatchArray->GetRenderBatchCount(); k < size; ++k)
    {
        RenderBatch * batch = renderLayerBatchArray->Get(k);
        batch->Draw(ownerRenderPass, camera);
    }
}

void InstancedRenderLayer::StartInstancingGroup(RenderBatch *batch, const FastName & ownerRenderPass, Camera * camera)
{
    NMaterial *material = batch->GetMaterial();
    if (!material->IsInstancingSupported())//just draw and forget
    {        
        batch->Draw(ownerRenderPass, camera);            
        return;
    }    
    batch->GetRenderObject()->BindDynamicParameters(camera);
    material->SetActivePass(ownerRenderPass);
    Shader *shader = material->GetActivePassShader();
    shader->Bind();    
    shader->BindDynamicParameters();

    int32 instancedUniformesCount = shader->GetInstancingUniformCount();
    incomingUniformValues.clear();
    incomingUniformValues.resize(instancedUniformesCount);
        
   
    for (int32 i=0; i<instancedUniformesCount; i++)
    {
        Shader::Uniform* uniform = shader->GetInstancingUniform(i);
        incomingUniformValues[i].first = uniform;
        uint32 uniformDataSize = Shader::GetUniformTypeSize(uniform->type);
        incomingUniformValues[i].second.resize(uniformDataSize*MAX_INSTANCES_COUNT);
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
    if ((batch->GetPolygonGroup()!=incomingGroup->GetPolygonGroup())
        ||((!incomingGroup->GetPolygonGroup())&&(batch->GetRenderDataObject()!=incomingGroup->GetRenderDataObject())))
    {
        return false;
    }
    material->SetActivePass(ownerRenderPass);
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
    int32 instancedUniformesCount = incomingUniformValues.size();    
    for (int32 i=0; i<instancedUniformesCount; i++)
    {
        Shader::Uniform* uniform = incomingUniformValues[i].first;        
        uint32 uniformDataSize = Shader::GetUniformTypeSize(uniform->type);
        if (Shader::IsAutobindUniform(uniform->shaderSemantic))
        {
            const void *data = RenderManager::GetDynamicParam(uniform->shaderSemantic);
            DVASSERT(data);
            Memcpy(&incomingUniformValues[i].second[currInstancesCount], data, uniformDataSize);
        }
        else
        {
            NMaterialProperty *property = material->GetPropertyValue(uniform->name);
            DVASSERT(property);
            Memcpy(&incomingUniformValues[i].second[currInstancesCount], property->data, uniformDataSize);
        }
    }
}


void InstancedRenderLayer::CompleteInstancingGroup(const FastName & ownerRenderPass, Camera * camera)
{
    if (!incomingGroup)
        return;
    
    incomingGroup->Draw(ownerRenderPass, camera);        

    if (currInstancesCount>1)
        drawQue.push_back(std::make_pair(incomingGroup->GetMaterial()->GetParent()->GetMaterialName(), currInstancesCount));

    incomingGroup = NULL;
    currInstancesCount = 0;
}

void InstancedRenderLayer::DrawRenderBatchArray(const FastName & ownerRenderPass, Camera * camera, RenderLayerBatchArray * renderLayerBatchArray)
{
       
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

    int32 economy = 0;
    for (int32 i=0, sz = drawQue.size(); i<sz; ++i)
    {
        Logger::FrameworkDebug("instance - %s count %d", drawQue[i].first.c_str(), drawQue[i].second);
        economy+=drawQue[i].second - 1;
    }

    Logger::FrameworkDebug("Total economy: %d", economy);
    drawQue.clear();

};


};
