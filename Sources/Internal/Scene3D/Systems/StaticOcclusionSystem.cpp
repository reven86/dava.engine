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
#include "Render/Highlevel/Landscape.h"
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
    Landscape * landscape = 0;
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
        if (RenderObject::TYPE_LANDSCAPE == renderObject->GetType())
        {
            landscape = DynamicTypeCheck<Landscape*>(renderObject);
        }
    }
    
    // Prepare occlusion
    componentInProgress = (StaticOcclusionDataComponent*)entity->GetComponent(Component::STATIC_OCCLUSION_DATA_COMPONENT);
    if (componentInProgress)
    {
        /*
            We detach component from system, to let system know that this data is not valid right now.
            Entity will be removed from system that apply occlusion information.
         */
        entity->DetachComponent(componentInProgress);
    }else
    {
        componentInProgress = new StaticOcclusionDataComponent();
    }
    
    StaticOcclusionData & data = componentInProgress->GetData();
    
    if (StaticOcclusion::RENEW_OCCLUSION_INDICES == renewIndex)
    {
        data.Init(occlusionComponent->GetSubdivisionsX(),
                  occlusionComponent->GetSubdivisionsY(),
                  occlusionComponent->GetSubdivisionsZ(),
                  size,
                  worldBox);

        GetScene()->staticOcclusionSystem->ClearOcclusionObjects();
    }
    
    staticOcclusion->SetScene(GetScene());
    staticOcclusion->SetRenderSystem(GetScene()->GetRenderSystem());
    staticOcclusion->BuildOcclusionInParallel(renderObjectsArray, landscape, &data, (StaticOcclusion::eIndexRenew)renewIndex);
    
    SceneForceLod(0);
    UpdateSwitchMaterialRecursively(GetScene());
    
    Map<RenderObject*, Vector<RenderObject*> > equalRenderObjects;
    staticOcclusion->SetEqualVisibilityVector(equalRenderObjects);
    
    messageQueue.AddMessage(Message(this, &StaticOcclusionBuildSystem::OcclusionBuildStep, 0));
}
    
void StaticOcclusionBuildSystem::OcclusionBuildStep(BaseObject * bo, void * messageData, void * callerData)
{
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
        RestoreSwitchMaterials();
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

    // We've detached component so we verify that here we still do not have this component.
    DVASSERT(prevComponent == 0);

    entities[activeIndex]->AddComponent(componentInProgress);
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
    RestoreSwitchMaterials();

    Scene *scene = GetScene();
    scene->staticOcclusionSystem->CollectOcclusionObjectsRecursively(scene);
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

void StaticOcclusionBuildSystem::UpdateSwitchMaterialRecursively(Entity *entity)
{
    for (int32 i=0, sz = entity->GetChildrenCount(); i<sz; ++i)
        UpdateSwitchMaterialRecursively(entity->GetChild(i));

    RenderObject *ro = GetRenderObject(entity);
    if (ro)
    {
        bool isSwitch = false;        
        for (int32 i=0, sz = ro->GetRenderBatchCount(); i<sz; ++i)
        {
            int32 li = 0;
            int32 si = 0;
            ro->GetRenderBatch(i, li, si);
            if (si>0)
            {
                isSwitch = true;
                break;
            }
        }

        if (isSwitch)
        {
            for (int32 i=0, sz = ro->GetRenderBatchCount(); i<sz; ++i)
            {
                NMaterial *mat = ro->GetRenderBatch(i)->GetMaterial();
                if (mat&&originalRenderStateData.find(mat)==originalRenderStateData.end())
                {
                    RenderStateData data;
                    mat->GetRenderState(PASS_FORWARD, data);
                    originalRenderStateData[mat]=data;
                    data.state = data.state & ~RenderStateData::STATE_DEPTH_WRITE;
                    mat->SubclassRenderState(PASS_FORWARD, data);
                }
            }
        }

    }
    
}

void StaticOcclusionBuildSystem::RestoreSwitchMaterials()
{
    for (Map<NMaterial*, RenderStateData>::iterator it = originalRenderStateData.begin(), e = originalRenderStateData.end(); it!=e; ++it)
        it->first->SubclassRenderState(PASS_FORWARD, it->second);
    
    originalRenderStateData.clear();
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
    
    activePVSSet = 0;
    activeBlockIndex = 0;
}

void StaticOcclusionSystem::ProcessStaticOcclusionForOneDataSet(uint32 blockIndex, StaticOcclusionData * data)
{
//#define LOG_DEBUG_OCCLUSION_APPLY
#if defined(LOG_DEBUG_OCCLUSION_APPLY)
    uint32 visCount = 0;
    uint32 invisCount = 0;
#endif
    
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
#if defined(LOG_DEBUG_OCCLUSION_APPLY)
            visCount++;
#endif
        }else
        {
            ro->SetFlags(ro->GetFlags() & ~RenderObject::VISIBLE_STATIC_OCCLUSION);
#if defined(LOG_DEBUG_OCCLUSION_APPLY)
            invisCount++;
#endif
        }
    }
#if defined(LOG_DEBUG_OCCLUSION_APPLY)
    Logger::Debug("apply cell: %d vis:%d invis:%d", blockIndex, visCount, invisCount);
#endif
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
    }
    
    if (needUpdatePVS)
    {
        ProcessStaticOcclusionForOneDataSet(activeBlockIndex, activePVSSet);
    }
}
    
void StaticOcclusionSystem::RegisterEntity(Entity *entity)
{
    SceneSystem::RegisterEntity(entity);
    
    RenderObject * renderObject = GetRenderObject(entity);
    if (renderObject)
    {
        AddRenderObjectToOcclusion(renderObject);
    }
}
    
void StaticOcclusionSystem::UnregisterEntity(Entity *entity)
{
    RenderObject * renderObject = GetRenderObject(entity);
    if (renderObject)
    {
        RemoveRenderObjectFromOcclusion(renderObject);
    }
    SceneSystem::UnregisterEntity(entity);
}
    
void StaticOcclusionSystem::RegisterComponent(Entity *entity, Component * component)
{
    SceneSystem::RegisterComponent(entity, component);
    
    if (component->GetType() == Component::RENDER_COMPONENT)
    {
        RenderObject * ro = GetRenderObject(entity);
        if (ro)
        {
            AddRenderObjectToOcclusion(ro);
        }
    }
}
    
void StaticOcclusionSystem::UnregisterEntity(Entity *entity, Component * component)
{
    if (component->GetType() == Component::RENDER_COMPONENT)
    {
        RenderObject * ro = GetRenderObject(entity);
        if (ro)
        {
            RemoveRenderObjectFromOcclusion(ro);
        }
    }
    SceneSystem::UnregisterComponent(entity, component);
}

    
void StaticOcclusionSystem::AddEntity(Entity * entity)
{
    staticOcclusionComponents.push_back((StaticOcclusionDataComponent*)entity->GetComponent(Component::STATIC_OCCLUSION_DATA_COMPONENT));
}
    
void StaticOcclusionSystem::AddRenderObjectToOcclusion(RenderObject * renderObject)
{
    /*
        registed all render objects in occlusion array, when they added to scene
     */
    if (renderObject->GetStaticOcclusionIndex() != INVALID_STATIC_OCCLUSION_INDEX)
    {
        indexedRenderObjects.resize(Max((uint32)indexedRenderObjects.size(), (uint32)(renderObject->GetStaticOcclusionIndex() + 1)));
        DVASSERT(indexedRenderObjects[renderObject->GetStaticOcclusionIndex()] == 0);
        indexedRenderObjects[renderObject->GetStaticOcclusionIndex()] = renderObject;
    }
}
    
void StaticOcclusionSystem::RemoveRenderObjectFromOcclusion(RenderObject * renderObject)
{
    /*
        If object removed from scene, remove it from occlusion array, for safety.
     */
    if (renderObject->GetStaticOcclusionIndex() != INVALID_STATIC_OCCLUSION_INDEX)
    {
        DVASSERT(renderObject->GetStaticOcclusionIndex() < indexedRenderObjects.size());
        indexedRenderObjects[renderObject->GetStaticOcclusionIndex()] = 0;
    }
}
    
void StaticOcclusionSystem::RemoveEntity(Entity * entity)
{
    for (uint32 k = 0; k < (uint32)staticOcclusionComponents.size(); ++k)
    {
        StaticOcclusionDataComponent * component = staticOcclusionComponents[k];
        if (component == entity->GetComponent(Component::STATIC_OCCLUSION_DATA_COMPONENT))
        {
            UndoOcclusionVisibility();
            
            staticOcclusionComponents[k] = staticOcclusionComponents[(uint32)staticOcclusionComponents.size() - 1];
            staticOcclusionComponents.pop_back();
            break;
        }
    }
}


void StaticOcclusionSystem::ClearOcclusionObjects()
{
    for (int32 i=0, sz = indexedRenderObjects.size(); i<sz; ++i)
    {
        if (indexedRenderObjects[i])
            indexedRenderObjects[i]->SetStaticOcclusionIndex(INVALID_STATIC_OCCLUSION_INDEX);
    }
    indexedRenderObjects.clear();
}
void StaticOcclusionSystem::CollectOcclusionObjectsRecursively(Entity *entity)
{
    RenderObject * renderObject = GetRenderObject(entity);
    if (renderObject)
    {
        AddRenderObjectToOcclusion(renderObject);
    }

    for (int32 i=0, sz = entity->GetChildrenCount(); i<sz; ++i)
        CollectOcclusionObjectsRecursively(entity->GetChild(i));
}

void StaticOcclusionSystem::InvalidateOcclusion()
{    
    InvalidateOcclusionIndicesRecursively(GetScene());
    indexedRenderObjects.clear();
}
void StaticOcclusionSystem::InvalidateOcclusionIndicesRecursively(Entity *entity)
{
    RenderObject * ro = GetRenderObject(entity);
    if (ro)
        ro->SetStaticOcclusionIndex(INVALID_STATIC_OCCLUSION_INDEX);
    for (int32 i=0, sz = entity->GetChildrenCount(); i<sz; ++i)
        InvalidateOcclusionIndicesRecursively(entity->GetChild(i));
}
    
};