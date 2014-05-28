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
#include "Scene3D/Systems/StaticOcclusionSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/StaticOcclusionComponent.h"
#include "Sound/SoundEvent.h"
#include "Sound/SoundSystem.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/StaticOcclusion.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/Systems/LodSystem.h"

namespace DAVA
{
    
MessageQueue::MessageQueue()
{
    
}

void MessageQueue::DispatchMessages()
{
    if (!messageQueue.empty())
    {
        const Message & message = messageQueue.front();
        message(0);
        messageQueue.pop();
    }
}
    
void MessageQueue::AddMessage(const Message & message)
{
    messageQueue.push(message);
}

    
    
StaticOcclusionBuildSystem::StaticOcclusionBuildSystem(Scene * scene)
:	SceneSystem(scene)
{
    staticOcclusion = 0;
    activeIndex = -1;
    componentInProgress = 0;
}

StaticOcclusionBuildSystem::~StaticOcclusionBuildSystem()
{
    SafeDelete(staticOcclusion);
}

void StaticOcclusionBuildSystem::AddEntity(Entity * entity)
{
    entities.push_back(entity);
}
    
void StaticOcclusionBuildSystem::RemoveEntity(Entity * entity)
{
    entities.erase( std::remove( entities.begin(), entities.end(), entity ), entities.end() );
}
    
void StaticOcclusionBuildSystem::Build()
{
    if (entities.size() == 0)return;
    activeIndex = 0;
    buildStepsCount = 0;
    buildStepRemains = 0;
    if (!staticOcclusion)
        staticOcclusion = new StaticOcclusion();
    renewIndex = StaticOcclusion::RENEW_OCCLUSION_INDICES;
    
    messageQueue.AddMessage(Message(this, &StaticOcclusionBuildSystem::StartBuildOcclusion));
}
    
void StaticOcclusionBuildSystem::RebuildCurrentCell()
{
    if (entities.size() == 0)return;
    activeIndex = 0;
    buildStepsCount = 0;
    buildStepRemains = 0;
    if (!staticOcclusion)
        staticOcclusion = new StaticOcclusion();
    renewIndex = StaticOcclusion::LEAVE_OLD_INDICES;
    
    messageQueue.AddMessage(Message(this, &StaticOcclusionBuildSystem::StartBuildOcclusion));
}

void StaticOcclusionBuildSystem::Cancel()
{
    activeIndex = -1;
    SafeDelete(staticOcclusion);
}
    
void StaticOcclusionBuildSystem::StartBuildOcclusion(BaseObject * bo, void * messageData, void * callerData)
{
    if (activeIndex == -1)return; // System inactive
    
    SetCamera(GetScene()->GetCurrentCamera());

    Entity * entity = entities[activeIndex];
    StaticOcclusionComponent * occlusionComponent = (StaticOcclusionComponent*)entity->GetComponent(Component::STATIC_OCCLUSION_COMPONENT);
    TransformComponent * transformComponent = (TransformComponent*)entity->GetComponent(Component::TRANSFORM_COMPONENT);
 
    AABBox3 localBox = occlusionComponent->GetBoundingBox();
    AABBox3 worldBox;
    localBox.GetTransformedBox(transformComponent->GetWorldTransform(), worldBox);
    
    // Prepare render objects
    Vector<Entity*> entities;
    Vector<RenderObject*> renderObjectsArray;
    GetScene()->GetChildEntitiesWithComponent(entities, Component::RENDER_COMPONENT);
    
    uint32 size = (uint32)entities.size();
    renderObjectsArray.reserve(size);
    DVASSERT(renderObjectsArray.size() == 0);
    for(uint32 k = 0; k < size; ++k)
    {
        RenderObject * renderObject = GetRenderObject(entities[k]);
        if (   (RenderObject::TYPE_MESH == renderObject->GetType())
            || (RenderObject::TYPE_LANDSCAPE == renderObject->GetType()))
        {
            renderObjectsArray.push_back(renderObject);
            renderObject->SetFlags(renderObject->GetFlags() | RenderObject::VISIBLE_STATIC_OCCLUSION);
        }
    }
    
    // Prepare occlusion
    componentInProgress = (StaticOcclusionDataComponent*)entity->GetOrCreateComponent(Component::STATIC_OCCLUSION_DATA_COMPONENT);
    
    StaticOcclusionData & data = componentInProgress->GetData();
    
    if (StaticOcclusion::RENEW_OCCLUSION_INDICES == renewIndex)
    {
        data.Init(occlusionComponent->GetSubdivisionsX(),
                  occlusionComponent->GetSubdivisionsY(),
                  occlusionComponent->GetSubdivisionsZ(),
                  size,
                  worldBox);
    }
    
    staticOcclusion->SetScene(GetScene());
    staticOcclusion->SetRenderSystem(GetScene()->GetRenderSystem());
    staticOcclusion->BuildOcclusionInParallel(renderObjectsArray, &data, (StaticOcclusion::eIndexRenew)renewIndex);
    
    SceneForceLod(0);
    
    Map<RenderObject*, Vector<RenderObject*> > equalRenderObjects;
    staticOcclusion->SetEqualVisibilityVector(equalRenderObjects);
    
    messageQueue.AddMessage(Message(this, &StaticOcclusionBuildSystem::OcclusionBuildStep, 0));
}
    
void StaticOcclusionBuildSystem::OcclusionBuildStep(BaseObject * bo, void * messageData, void * callerData)
{
    StaticOcclusionDataComponent * componentInProgress = (StaticOcclusionDataComponent *)messageData;

    if (StaticOcclusion::LEAVE_OLD_INDICES == renewIndex)
    {
        const Vector3 & position = camera->GetPosition();
        
        StaticOcclusionData & data = componentInProgress->GetData();
        
        if (data.bbox.IsInside(position))
        {
            uint32 x = (uint32)((position.x - data.bbox.min.x) / (data.bbox.max.x - data.bbox.min.x) * (float32)data.sizeX);
            uint32 y = (uint32)((position.y - data.bbox.min.y) / (data.bbox.max.y - data.bbox.min.y) * (float32)data.sizeY);
            uint32 z = (uint32)((position.z - data.bbox.min.z) / (data.bbox.max.z - data.bbox.min.z) * (float32)data.sizeZ);
            
            staticOcclusion->RenderFrame(x, y, z);
        }
        
        activeIndex = -1;
        
        SceneForceLod(LodComponent::INVALID_LOD_LAYER);
    }
    else
    {
        buildStepRemains = staticOcclusion->RenderFrame();
        if(buildStepRemains > buildStepsCount)
        {
            buildStepsCount = buildStepRemains + 1;
        }
        
        if (buildStepRemains == 0)
        {
            messageQueue.AddMessage(Message(this, &StaticOcclusionBuildSystem::FinishBuildOcclusion, 0));
        }
        else
        {
            messageQueue.AddMessage(Message(this, &StaticOcclusionBuildSystem::OcclusionBuildStep, 0));
        }
    }
}
    
void StaticOcclusionBuildSystem::FinishBuildOcclusion(DAVA::BaseObject *bo, void *messageData, void *callerData)
{
    Component * prevComponent = entities[activeIndex]->GetComponent(Component::STATIC_OCCLUSION_DATA_COMPONENT);
    if (prevComponent != componentInProgress)
    {
        // Replace component, only if it's new
        entities[activeIndex]->RemoveComponent(Component::STATIC_OCCLUSION_DATA_COMPONENT);
        entities[activeIndex]->AddComponent(componentInProgress);
    }
    componentInProgress = 0;
    
    activeIndex++;
    if (activeIndex == entities.size())
    {
        activeIndex = -1;
    }else
    {
        // not final index add more occlusion build cycle
        messageQueue.AddMessage(Message(this, &StaticOcclusionBuildSystem::StartBuildOcclusion));
    }
    
    SceneForceLod(LodComponent::INVALID_LOD_LAYER);
}
    
bool StaticOcclusionBuildSystem::IsInBuild() const
{
    return (-1 != activeIndex);
}

uint32 StaticOcclusionBuildSystem::GetBuildStatus() const
{
    uint32 ret = 0;
    
    if(0 != buildStepsCount && buildStepsCount >= buildStepRemains)
    {
        ret = ((buildStepsCount - buildStepRemains) * 100) / buildStepsCount;
    }
    
    return ret;
}

void StaticOcclusionBuildSystem::SceneForceLod(int32 forceLodIndex)
{
    Vector<Entity*> lodEntities;
    GetScene()->GetChildEntitiesWithComponent(lodEntities, Component::LOD_COMPONENT);
    uint32 size = (uint32)lodEntities.size();
    for(uint32 k = 0; k < size; ++k)
    {
        LodComponent * lodComponent = (LodComponent*)lodEntities[k]->GetComponent(Component::LOD_COMPONENT);
        lodComponent->SetForceLodLayer(forceLodIndex);
    }
    GetScene()->lodSystem->SetForceUpdateAll();
    GetScene()->lodSystem->Process(0.0f);
}

void StaticOcclusionBuildSystem::Process(float32 timeElapsed)
{
    messageQueue.DispatchMessages();
}
    
    
//
// Static Occlusion System
//
    
void StaticOcclusionSystem::UndoOcclusionVisibility()
{
    uint32 size = (uint32)indexedRenderObjects.size();
    for (uint32 k = 0; k < size; ++k)
    {
        RenderObject * ro = indexedRenderObjects[k];
        if (!ro)continue;
        ro->SetFlags(ro->GetFlags() | RenderObject::VISIBLE_STATIC_OCCLUSION);
    }

}

void StaticOcclusionSystem::ProcessStaticOcclusionForOneDataSet(uint32 blockIndex, StaticOcclusionData * data)
{
    uint32 * bitdata = data->GetBlockVisibilityData(blockIndex);
    uint32 size = (uint32)indexedRenderObjects.size();
    for (uint32 k = 0; k < size; ++k)
    {
        uint32 index = k / 32; // number of bits in uint32
        uint32 shift = k & 31; // bitmask for uint32
        RenderObject * ro = indexedRenderObjects[k];
        if (!ro)continue;
        if (bitdata[index] & (1 << shift))
        {
            ro->SetFlags(ro->GetFlags() | RenderObject::VISIBLE_STATIC_OCCLUSION);
        }else
        {
            ro->SetFlags(ro->GetFlags() & ~RenderObject::VISIBLE_STATIC_OCCLUSION);
        }
    }
}

    
StaticOcclusionSystem::StaticOcclusionSystem(Scene * scene)
:	SceneSystem(scene)
{
    indexedRenderObjects.reserve(2000);
    for (uint32 k = 0; k < indexedRenderObjects.size(); ++k)
        indexedRenderObjects[k] = 0;
    
    activePVSSet = 0;
    activeBlockIndex = 0;
}

StaticOcclusionSystem::~StaticOcclusionSystem()
{
}

void StaticOcclusionSystem::Process(float32 timeElapsed)
{
    SetCamera(GetScene()->GetCurrentCamera());

    // Verify that system is initialized
    if (!camera)return;

    uint32 size = (uint32)staticOcclusionComponents.size();
    if (size == 0)return;
    
    
    bool notInPVS = true;
    bool needUpdatePVS = false;
    
    const Vector3 & position = camera->GetPosition();
    
    for (uint32 k = 0; k < size; ++k)
    {
        StaticOcclusionData * data = &staticOcclusionComponents[k]->GetData();
        if (!data)return;
        
        if (data->bbox.IsInside(position))
        {
            uint32 x = (uint32)((position.x - data->bbox.min.x) / (data->bbox.max.x - data->bbox.min.x) * (float32)data->sizeX);
            uint32 y = (uint32)((position.y - data->bbox.min.y) / (data->bbox.max.y - data->bbox.min.y) * (float32)data->sizeY);
            uint32 z = (uint32)((position.z - data->bbox.min.z) / (data->bbox.max.z - data->bbox.min.z) * (float32)data->sizeZ);
            
            // IsInside function take borders into account.
            
            if ((x < data->sizeX) && (y < data->sizeY) && (z < data->sizeZ))
            {
                uint32 blockIndex = z * (data->sizeX * data->sizeY) + y * (data->sizeX) + (x);

                if ((activePVSSet != data) || (activeBlockIndex != blockIndex))
                {
                    activePVSSet = data;
                    activeBlockIndex = blockIndex;
                    needUpdatePVS = true;
                }
                notInPVS = false;
            }
        }
    }
    
    if (notInPVS)
    {
        UndoOcclusionVisibility();
        activePVSSet = 0;
        activeBlockIndex = 0;
    }
    
    if (needUpdatePVS)
    {
        ProcessStaticOcclusionForOneDataSet(activeBlockIndex, activePVSSet);
    }
    
    
    
}
    
void StaticOcclusionSystem::AddEntity(Entity * entity)
{
    staticOcclusionComponents.push_back((StaticOcclusionDataComponent*)entity->GetComponent(Component::STATIC_OCCLUSION_DATA_COMPONENT));
    SceneDidLoaded();
}
    
void StaticOcclusionSystem::SceneDidLoaded()
{
    // Recalc indices
    Vector<Entity*> entities;
    Vector<RenderObject*> renderObjectsArray;
    GetScene()->GetChildEntitiesWithComponent(entities, Component::RENDER_COMPONENT);
    
    uint32 size = (uint32)entities.size();
    renderObjectsArray.resize(size);
    for(uint32 k = 0; k < size; ++k)
        renderObjectsArray[k] = GetRenderObject(entities[k]);
    
    indexedRenderObjects.resize(size);
    for (uint32 k = 0; k < size; ++k)
    {
        if (renderObjectsArray[k]->GetStaticOcclusionIndex() != INVALID_STATIC_OCCLUSION_INDEX)
        {
            indexedRenderObjects[renderObjectsArray[k]->GetStaticOcclusionIndex()] = renderObjectsArray[k];
        }
    }
}
    
void StaticOcclusionSystem::RemoveEntity(Entity * entity)
{
    for (uint32 k = 0; k < (uint32)staticOcclusionComponents.size(); ++k)
    {
        StaticOcclusionDataComponent * component = staticOcclusionComponents[k];
        if (component == entity->GetComponent(Component::STATIC_OCCLUSION_DATA_COMPONENT))
        {
            staticOcclusionComponents[k] = staticOcclusionComponents[(uint32)staticOcclusionComponents.size() - 1];
            staticOcclusionComponents.pop_back();
            break;
        }
    }
}

    
};