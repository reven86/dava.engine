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
#include "Render/3D/PolygonGroup.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/StaticOcclusion.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/Systems/LodSystem.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/Highlevel/Landscape.h"
#include "Debug/Stats.h"

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
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::WORLD_TRANSFORM_CHANGED);
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::STATIC_OCCLUSION_COMPONENT_CHANGED); 
}

StaticOcclusionBuildSystem::~StaticOcclusionBuildSystem()
{
    GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::WORLD_TRANSFORM_CHANGED);
    GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::STATIC_OCCLUSION_COMPONENT_CHANGED);
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

void StaticOcclusionBuildSystem::ImmediateEvent(Component * _component, uint32 event)
{
    Entity * entity = _component->GetEntity();
    StaticOcclusionComponent *component = static_cast<StaticOcclusionComponent*>(entity->GetComponent(Component::STATIC_OCCLUSION_COMPONENT));
    DVASSERT(component);
    if (component->GetPlaceOnLandscape()&&((event == EventSystem::WORLD_TRANSFORM_CHANGED)||(event == EventSystem::STATIC_OCCLUSION_COMPONENT_CHANGED)))
    {
        component->cellHeightOffset.clear();
        component->cellHeightOffset.resize(component->GetSubdivisionsX()*component->GetSubdivisionsY(), 0);
        /*place on landscape*/
        Landscape *landscape = FindLandscape(GetScene());
        AABBox3 localBox = component->GetBoundingBox();
        Vector3 boxSize = localBox.GetSize();        
        AABBox3 bbox;
        localBox.GetTransformedBox(GetTransformComponent(entity)->GetWorldTransform(), bbox);
        uint32 xSubdivisions = component->GetSubdivisionsX();
        uint32 ySubdivisions = component->GetSubdivisionsY();
        boxSize.x /= xSubdivisions;
        boxSize.y /= ySubdivisions;
        
        if (landscape)
        {
            //place on landscape
            for (uint32 xs = 0; xs<xSubdivisions; ++xs)
                for (uint32 ys = 0; ys<ySubdivisions; ++ys)
                {                    
                    Vector3 v = bbox.min + Vector3(boxSize.x * (xs + 0.5f), boxSize.y * (ys + 0.5f), 0);
                    if (landscape->PlacePoint(v, v))
                       component->cellHeightOffset[xs+ys*xSubdivisions] = v.z - bbox.min.z;
                }
        }        
    }    
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
    
    StaticOcclusionSystem *sos = GetScene()->staticOcclusionSystem;
    sos->InvalidateOcclusion();
    SceneForceLod(LodComponent::INVALID_LOD_LAYER);
    RestoreOcclusionMaterials();
}

void StaticOcclusionBuildSystem::CollectEntitiesForOcclusionRecursively(Vector<Entity*>& dest, Entity *entity)
{
    if (GetAnimationComponent(entity)) //skip animated hierarchies
        return;
    if (GetRenderComponent(entity))
        dest.push_back(entity);

    for (int32 i=0, sz = entity->GetChildrenCount(); i<sz; ++i)
        CollectEntitiesForOcclusionRecursively(dest, entity->GetChild(i));
}
    
void StaticOcclusionBuildSystem::StartBuildOcclusion(BaseObject * bo, void * messageData, void * callerData)
{
    if (activeIndex == static_cast<uint32>(-1))return; // System inactive
    
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
    CollectEntitiesForOcclusionRecursively(entities, GetScene());    
    
    uint32 size = (uint32)entities.size();
    renderObjectsArray.reserve(size);
    DVASSERT(renderObjectsArray.size() == 0);
    for(uint32 k = 0; k < size; ++k)
    {
        RenderObject * renderObject = GetRenderObject(entities[k]);
        if (   (RenderObject::TYPE_MESH == renderObject->GetType())
            || (RenderObject::TYPE_LANDSCAPE == renderObject->GetType())
            || (RenderObject::TYPE_SPEED_TREE == renderObject->GetType()))
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
                  worldBox,
                  occlusionComponent->GetCellHeightOffsets());

        GetScene()->staticOcclusionSystem->ClearOcclusionObjects();
    }
    
    staticOcclusion->SetScene(GetScene());
    staticOcclusion->SetRenderSystem(GetScene()->GetRenderSystem());
    staticOcclusion->BuildOcclusionInParallel(renderObjectsArray, landscape, &data, (StaticOcclusion::eIndexRenew)renewIndex);
    
    SceneForceLod(0);
    UpdateMaterialsForOcclusionRecursively(GetScene());
    
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

        if ((position.x>=data.bbox.min.x)&&(position.x<=data.bbox.max.x)&&(position.y>=data.bbox.min.y)&&(position.y<=data.bbox.max.y))
        {        
            uint32 x = (uint32)((position.x - data.bbox.min.x) / (data.bbox.max.x - data.bbox.min.x) * (float32)data.sizeX);
            uint32 y = (uint32)((position.y - data.bbox.min.y) / (data.bbox.max.y - data.bbox.min.y) * (float32)data.sizeY);
            float32 dH = data.cellHeightOffset?data.cellHeightOffset[x+y*data.sizeX]:0;        
            if ((position.z>=(data.bbox.min.z+dH))&&(position.z<=(data.bbox.max.z+dH)))
            {        
                uint32 z = (uint32)((position.z - (data.bbox.min.z+dH)) / (data.bbox.max.z - data.bbox.min.z) * (float32)data.sizeZ);                                    
                
                if ((x < data.sizeX) && (y < data.sizeY) && (z < data.sizeZ))
                {                    
                    staticOcclusion->RenderFrame(x, y, z);
                }
            }
        }

        entities[activeIndex]->AddComponent(componentInProgress);
        componentInProgress = 0;
        
        activeIndex = -1;
        
        SceneForceLod(LodComponent::INVALID_LOD_LAYER);
        RestoreOcclusionMaterials();
    }
    else if(staticOcclusion)
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
    RestoreOcclusionMaterials();

    Scene *scene = GetScene();
    scene->staticOcclusionSystem->CollectOcclusionObjectsRecursively(scene);
}
    
bool StaticOcclusionBuildSystem::IsInBuild() const
{
    return (static_cast<uint32>(-1) != activeIndex);
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

void StaticOcclusionBuildSystem::UpdateMaterialsForOcclusionRecursively(Entity *entity)
{
    for (int32 i=0, sz = entity->GetChildrenCount(); i<sz; ++i)
        UpdateMaterialsForOcclusionRecursively(entity->GetChild(i));

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

        
        for (int32 i=0, sz = ro->GetRenderBatchCount(); i<sz; ++i)
        {
            NMaterial *mat = ro->GetRenderBatch(i)->GetMaterial();
            if (mat&&originalRenderStateData.find(mat)==originalRenderStateData.end())
            {
                RenderStateData data;
                mat->GetRenderState(PASS_FORWARD, data);
                originalRenderStateData[mat]=data;
                data.state = data.state & ~RenderStateData::STATE_CULL;
                if (isSwitch)                    
                    data.state = data.state & ~RenderStateData::STATE_DEPTH_WRITE;
                mat->SubclassRenderState(PASS_FORWARD, data);
            }
        }        
    }
    
}

void StaticOcclusionBuildSystem::RestoreOcclusionMaterials()
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
    TIME_PROFILE("StaticOcclusionSystem::Process")

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

        if ((position.x>=data->bbox.min.x)&&(position.x<=data->bbox.max.x)&&(position.y>=data->bbox.min.y)&&(position.y<=data->bbox.max.y))
        {        
            uint32 x = (uint32)((position.x - data->bbox.min.x) / (data->bbox.max.x - data->bbox.min.x) * (float32)data->sizeX);
            uint32 y = (uint32)((position.y - data->bbox.min.y) / (data->bbox.max.y - data->bbox.min.y) * (float32)data->sizeY);   
            if ((x < data->sizeX) && (y < data->sizeY)) //
            {
                float32 dH = data->cellHeightOffset?data->cellHeightOffset[x+y*data->sizeX]:0;  
                if ((position.z>=(data->bbox.min.z+dH))&&(position.z<=(data->bbox.max.z+dH)))
                {

                    uint32 z = (uint32)((position.z - (data->bbox.min.z+dH)) / (data->bbox.max.z - data->bbox.min.z) * (float32)data->sizeZ);                    
            
                    if (z < data->sizeZ)
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
    
void StaticOcclusionSystem::UnregisterComponent(Entity *entity, Component * component)
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
    for (size_t i=0, sz = indexedRenderObjects.size(); i<sz; ++i)
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



StaticOcclusionDebugDrawSystem::StaticOcclusionDebugDrawSystem(Scene *scene):SceneSystem(scene)
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::WORLD_TRANSFORM_CHANGED);
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::STATIC_OCCLUSION_COMPONENT_CHANGED);

    debugOpaqueMaterial = NMaterial::CreateMaterial(FastName("Debug_Opaque_Material"),  NMaterialName::DEBUG_DRAW_OPAQUE, NMaterial::DEFAULT_QUALITY_NAME);		
    debugAlphablendMaterial = NMaterial::CreateMaterial(FastName("Debug_Alphablend_Material"),  NMaterialName::DEBUG_DRAW_ALPHABLEND, NMaterial::DEFAULT_QUALITY_NAME);	
}

StaticOcclusionDebugDrawSystem::~StaticOcclusionDebugDrawSystem()
{
    GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::WORLD_TRANSFORM_CHANGED);
    GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::STATIC_OCCLUSION_COMPONENT_CHANGED);
    SafeRelease(debugOpaqueMaterial);
    SafeRelease(debugAlphablendMaterial);
}

void StaticOcclusionDebugDrawSystem::AddEntity(Entity * entity)
{
    StaticOcclusionComponent *staticOcclusionComponent = static_cast<StaticOcclusionComponent*>(entity->GetComponent(Component::STATIC_OCCLUSION_COMPONENT));
    Matrix4 * worldTransformPointer = GetTransformComponent(entity)->GetWorldTransformPtr();
    //create render object
    ScopedPtr<RenderObject> debugRenderObject(new RenderObject());
    ScopedPtr<RenderBatch> gridBatch(new RenderBatch());
    ScopedPtr<PolygonGroup> gridPolygonGroup(CreateStaticOcclusionDebugDrawGrid(staticOcclusionComponent->GetBoundingBox(), staticOcclusionComponent->GetSubdivisionsX(), staticOcclusionComponent->GetSubdivisionsY(), staticOcclusionComponent->GetSubdivisionsZ(), staticOcclusionComponent->GetCellHeightOffsets()));
    ScopedPtr<NMaterial> gridMaterialInstance(NMaterial::CreateMaterialInstance());
    gridMaterialInstance->SetParent(debugAlphablendMaterial);
    Color col(0.0f, 0.3f, 0.1f, 0.2f);
    gridMaterialInstance->SetPropertyValue(NMaterial::PARAM_FLAT_COLOR, Shader::UT_FLOAT_VEC4, 1, &col);
    gridBatch->SetPolygonGroup(gridPolygonGroup);
    gridBatch->SetMaterial(gridMaterialInstance);
    


    ScopedPtr<RenderBatch> coverBatch(new RenderBatch());
    ScopedPtr<PolygonGroup> coverPolygonGroup(CreateStaticOcclusionDebugDrawCover(staticOcclusionComponent->GetBoundingBox(), staticOcclusionComponent->GetSubdivisionsX(), staticOcclusionComponent->GetSubdivisionsY(), staticOcclusionComponent->GetSubdivisionsZ(), gridPolygonGroup));
        
    ScopedPtr<NMaterial> coverMaterialInstance(NMaterial::CreateMaterialInstance());
    coverMaterialInstance->SetParent(debugAlphablendMaterial);
    Color colCover(0.1f, 0.5f, 0.1f, 0.3f);
    coverMaterialInstance->SetPropertyValue(NMaterial::PARAM_FLAT_COLOR, Shader::UT_FLOAT_VEC4, 1, &colCover);
    coverBatch->SetPolygonGroup(coverPolygonGroup);
    coverBatch->SetMaterial(coverMaterialInstance);
    
    
    debugRenderObject->AddRenderBatch(coverBatch);
    debugRenderObject->AddRenderBatch(gridBatch);
    
    debugRenderObject->SetWorldTransformPtr(worldTransformPointer);
    GetScene()->renderSystem->MarkForUpdate(debugRenderObject);

    entity->AddComponent(new StaticOcclusionDebugDrawComponent(debugRenderObject));    
    GetScene()->GetRenderSystem()->RenderPermanent(debugRenderObject);        
}

void StaticOcclusionDebugDrawSystem::RemoveEntity(Entity * entity)
{
    StaticOcclusionDebugDrawComponent *debugDrawComponent = static_cast<StaticOcclusionDebugDrawComponent *>(entity->GetComponent(Component::STATIC_OCCLUSION_DEBUG_DRAW_COMPONENT));
    DVASSERT(debugDrawComponent);    
    GetScene()->GetRenderSystem()->RemoveFromRender(debugDrawComponent->GetRenderObject());
    entity->RemoveComponent(Component::STATIC_OCCLUSION_DEBUG_DRAW_COMPONENT);    
}
void StaticOcclusionDebugDrawSystem::ImmediateEvent(Component * component, uint32 event)
{
    Entity * entity = component->GetEntity();
    StaticOcclusionDebugDrawComponent *debugDrawComponent = static_cast<StaticOcclusionDebugDrawComponent*>(entity->GetComponent(Component::STATIC_OCCLUSION_DEBUG_DRAW_COMPONENT));
    StaticOcclusionComponent *staticOcclusionComponent = static_cast<StaticOcclusionComponent*>(entity->GetComponent(Component::STATIC_OCCLUSION_COMPONENT));
    if (event == EventSystem::WORLD_TRANSFORM_CHANGED)
    {
        // Update new transform pointer, and mark that transform is changed
        Matrix4 * worldTransformPointer = GetTransformComponent(entity)->GetWorldTransformPtr();
        RenderObject * object = debugDrawComponent->GetRenderObject();
        if(NULL != object)
        {
            object->SetWorldTransformPtr(worldTransformPointer);
            entity->GetScene()->renderSystem->MarkForUpdate(object);
        }        
    }

    if ((event == EventSystem::STATIC_OCCLUSION_COMPONENT_CHANGED) || (staticOcclusionComponent->GetPlaceOnLandscape()))
    {                
        ScopedPtr<PolygonGroup> gridPolygonGroup(CreateStaticOcclusionDebugDrawGrid(staticOcclusionComponent->GetBoundingBox(), staticOcclusionComponent->GetSubdivisionsX(), staticOcclusionComponent->GetSubdivisionsY(), staticOcclusionComponent->GetSubdivisionsZ(), staticOcclusionComponent->GetCellHeightOffsets()));
        ScopedPtr<PolygonGroup> coverPolygonGroup(CreateStaticOcclusionDebugDrawCover(staticOcclusionComponent->GetBoundingBox(), staticOcclusionComponent->GetSubdivisionsX(), staticOcclusionComponent->GetSubdivisionsY(), staticOcclusionComponent->GetSubdivisionsZ(), gridPolygonGroup));
        RenderObject *debugRenderObject = debugDrawComponent->GetRenderObject();        
        debugRenderObject->GetRenderBatch(0)->SetPolygonGroup(coverPolygonGroup);
        debugRenderObject->GetRenderBatch(1)->SetPolygonGroup(gridPolygonGroup);
        debugRenderObject->RecalcBoundingBox();
        entity->GetScene()->renderSystem->MarkForUpdate(debugRenderObject);
    }
}

#define IDX_BY_POS(xc, yc, zc) ((zc) + (zSubdivisions+1)*((yc) + (xc) * ySubdivisions))*4

PolygonGroup* StaticOcclusionDebugDrawSystem::CreateStaticOcclusionDebugDrawGrid(const AABBox3& boundingBox, uint32 xSubdivisions, uint32 ySubdivisions, uint32 zSubdivisions, const float32 *cellHeightOffset)
{
    int32 vertexCount = xSubdivisions * ySubdivisions * 4 * (zSubdivisions + 1);
    int32 indexCount = xSubdivisions * ySubdivisions * zSubdivisions * 12 * 2; //12 lines per box 2 indices per line
    
    PolygonGroup *res = new PolygonGroup();
    res->SetPrimitiveType(PRIMITIVETYPE_LINELIST);    
    res->AllocateData(EVF_VERTEX, vertexCount, indexCount);    

    Vector3 boxSize = boundingBox.GetSize();
    boxSize.x /= xSubdivisions;
    boxSize.y /= ySubdivisions;
    boxSize.z /= zSubdivisions;
    
    //vertices
    //as we are going to place blocks on landscape we are to treat each column as independent - not sharing vertices between columns. we can still share vertices within 1 column
    for (uint32 xs = 0; xs < xSubdivisions; ++xs)
        for (uint32 ys = 0; ys < ySubdivisions; ++ys)
            for (uint32 zs = 0; zs < (zSubdivisions+1); ++zs)  
            {                                
                int32 vBase = IDX_BY_POS(xs, ys, zs);                
                float32 hOffset = cellHeightOffset?cellHeightOffset[xs+ys*xSubdivisions] :0;
                res->SetCoord(vBase + 0, boundingBox.min + Vector3(boxSize.x * xs, boxSize.y * ys, boxSize.z * zs + hOffset));
                res->SetCoord(vBase + 1, boundingBox.min + Vector3(boxSize.x * (xs+1), boxSize.y * ys, boxSize.z * zs + hOffset));
                res->SetCoord(vBase + 2, boundingBox.min + Vector3(boxSize.x * (xs+1), boxSize.y * (ys+1), boxSize.z * zs + hOffset));
                res->SetCoord(vBase + 3, boundingBox.min + Vector3(boxSize.x * xs, boxSize.y * (ys+1), boxSize.z * zs + hOffset));
            }        
    //indices     
    //in pair indexOffset, z
    const static int32 indexOffsets[]={0,0, 1,0, 1,0, 2,0, 2,0, 3,0, 3,0, 0,0,  //bot
                                       0,0, 0,1, 1,0, 1,1, 2,0, 2,1, 3,0, 3,1,  //mid
                                       0,1, 1,1, 1,1, 2,1, 2,1, 3,1, 3,1, 0,1}; //top

    for (uint32 xs = 0; xs < xSubdivisions; ++xs)
        for (uint32 ys = 0; ys < ySubdivisions; ++ys)
            for (uint32 zs = 0; zs < zSubdivisions; ++zs)
            {
                int32 iBase = (zs + zSubdivisions*(ys + xs * ySubdivisions)) * 24;
                int32 vBase[2] = {static_cast<int32>(IDX_BY_POS(xs, ys, zs)), static_cast<int32>(IDX_BY_POS(xs, ys, zs+1))};
                for (int32 i=0; i<24; i++)
                    res->SetIndex(iBase + i, indexOffsets[i*2] + vBase[indexOffsets[i*2+1]]);

            }


    

    res->BuildBuffers();    
    return res;
}

PolygonGroup* StaticOcclusionDebugDrawSystem::CreateStaticOcclusionDebugDrawCover(const AABBox3& boundingBox, uint32 xSubdivisions, uint32 ySubdivisions, uint32 zSubdivisions, PolygonGroup *gridPolygonGroup)
{
    
    int32 xSideIndexCount = xSubdivisions * 6 * 2; 
    int32 ySideIndexCount = ySubdivisions * 6 * 2;
    int32 xySideIndexCount =  xSideIndexCount + ySideIndexCount;
    int32 zSideIndexCount = xSubdivisions * ySubdivisions * 6 * 2;
    int32 totalSideIndexCount = xySideIndexCount+zSideIndexCount;
    int32 xExtraIndexCount = (xSubdivisions-1) * (ySubdivisions) * 6 * 2;
    int32 yExtraIndexCount = (ySubdivisions-1) * (xSubdivisions) * 6 * 2;
    int32 indexCount = totalSideIndexCount+xExtraIndexCount+yExtraIndexCount;

    PolygonGroup *res = new PolygonGroup();    
    res->AllocateData(0, 0, indexCount);    

    Vector3 boxSize = boundingBox.GetSize();
    boxSize.x /= xSubdivisions;
    boxSize.y /= ySubdivisions;
    boxSize.z /= zSubdivisions;

    //left and right
    for (uint32 xs = 0; xs < xSubdivisions; ++xs)
    {
        int32 iBase = xs*6*2;

        res->SetIndex(iBase+0, IDX_BY_POS(xs, 0, 0));
        res->SetIndex(iBase+1, IDX_BY_POS(xs, 0, 0)+1);
        res->SetIndex(iBase+2, IDX_BY_POS(xs, 0, zSubdivisions)+1);
        res->SetIndex(iBase+3, IDX_BY_POS(xs, 0, 0));
        res->SetIndex(iBase+4, IDX_BY_POS(xs, 0, zSubdivisions)+1);
        res->SetIndex(iBase+5, IDX_BY_POS(xs, 0, zSubdivisions));

        iBase = xs*6*2+6;

        res->SetIndex(iBase+0, IDX_BY_POS(xs, ySubdivisions-1, 0)+3);
        res->SetIndex(iBase+1, IDX_BY_POS(xs, ySubdivisions-1, 0)+2);
        res->SetIndex(iBase+2, IDX_BY_POS(xs, ySubdivisions-1, zSubdivisions)+2);
        res->SetIndex(iBase+3, IDX_BY_POS(xs, ySubdivisions-1, 0)+3);
        res->SetIndex(iBase+4, IDX_BY_POS(xs, ySubdivisions-1, zSubdivisions)+2);
        res->SetIndex(iBase+5, IDX_BY_POS(xs, ySubdivisions-1, zSubdivisions)+3);
    }

    //front and back
    for (uint32 ys = 0; ys < ySubdivisions; ++ys)
    {
        int32 iBase = xSideIndexCount + ys*6*2;

        res->SetIndex(iBase+0, IDX_BY_POS(0, ys, 0));
        res->SetIndex(iBase+1, IDX_BY_POS(0, ys, 0)+3);
        res->SetIndex(iBase+2, IDX_BY_POS(0, ys, zSubdivisions)+3);
        res->SetIndex(iBase+3, IDX_BY_POS(0, ys, 0));
        res->SetIndex(iBase+4, IDX_BY_POS(0, ys, zSubdivisions)+3);
        res->SetIndex(iBase+5, IDX_BY_POS(0, ys, zSubdivisions));

        iBase = xSideIndexCount + ys*6*2+6;

        res->SetIndex(iBase+0, IDX_BY_POS(xSubdivisions-1, ys, 0)+1);
        res->SetIndex(iBase+1, IDX_BY_POS(xSubdivisions-1, ys, 0)+2);
        res->SetIndex(iBase+2, IDX_BY_POS(xSubdivisions-1, ys, zSubdivisions)+2);
        res->SetIndex(iBase+3, IDX_BY_POS(xSubdivisions-1, ys, 0)+1);
        res->SetIndex(iBase+4, IDX_BY_POS(xSubdivisions-1, ys, zSubdivisions)+2);
        res->SetIndex(iBase+5, IDX_BY_POS(xSubdivisions-1, ys, zSubdivisions)+1);
    }
    
    //bot and top
    for (uint32 xs = 0; xs < xSubdivisions; ++xs)
        for (uint32 ys = 0; ys < ySubdivisions; ++ys)
        {
            int32 iBase = xySideIndexCount + (ys*xSubdivisions+xs)*6*2;
            int32 vBase = IDX_BY_POS(xs, ys, 0);
            res->SetIndex(iBase+0, vBase+0);
            res->SetIndex(iBase+1, vBase+1);
            res->SetIndex(iBase+2, vBase+2);
            res->SetIndex(iBase+3, vBase+0);
            res->SetIndex(iBase+4, vBase+2);
            res->SetIndex(iBase+5, vBase+3);

            iBase = xySideIndexCount + (ys*xSubdivisions+xs)*6*2 + 6;
            vBase = IDX_BY_POS(xs, ys, zSubdivisions);
            res->SetIndex(iBase+0, vBase+0);
            res->SetIndex(iBase+1, vBase+1);
            res->SetIndex(iBase+2, vBase+2);
            res->SetIndex(iBase+3, vBase+0);
            res->SetIndex(iBase+4, vBase+2);
            res->SetIndex(iBase+5, vBase+3);
        }

    //extras across x axis
    for (uint32 xs = 0; xs < (xSubdivisions-1); ++xs)
        for (uint32 ys = 0; ys < ySubdivisions; ++ys)
        {
            int32 iBase = totalSideIndexCount + (xs*ySubdivisions+ys) * 6 * 2;
            int32 vBase1 = IDX_BY_POS(xs, ys, 0);
            int32 vBase2 = IDX_BY_POS(xs+1, ys, 0);
            res->SetIndex(iBase+0, vBase1+1);
            res->SetIndex(iBase+1, vBase1+2);
            res->SetIndex(iBase+2, vBase2+3);
            res->SetIndex(iBase+3, vBase1+1);
            res->SetIndex(iBase+4, vBase2+3);
            res->SetIndex(iBase+5, vBase2+0);

            iBase += 6;
            vBase1 = IDX_BY_POS(xs, ys, zSubdivisions);
            vBase2 = IDX_BY_POS(xs+1, ys, zSubdivisions);
            res->SetIndex(iBase+0, vBase1+1);
            res->SetIndex(iBase+1, vBase1+2);
            res->SetIndex(iBase+2, vBase2+3);
            res->SetIndex(iBase+3, vBase1+1);
            res->SetIndex(iBase+4, vBase2+3);
            res->SetIndex(iBase+5, vBase2+0);
        }

    //extras across y axis
    for (uint32 xs = 0; xs < xSubdivisions; ++xs)
        for (uint32 ys = 0; ys < (ySubdivisions-1); ++ys)
        {
            int32 iBase = totalSideIndexCount + xExtraIndexCount + (ys*xSubdivisions+xs) * 6 * 2;
            int32 vBase1 = IDX_BY_POS(xs, ys, 0);
            int32 vBase2 = IDX_BY_POS(xs, ys+1, 0);
            res->SetIndex(iBase+0, vBase1+2);
            res->SetIndex(iBase+1, vBase1+3);
            res->SetIndex(iBase+2, vBase2+0);
            res->SetIndex(iBase+3, vBase1+2);
            res->SetIndex(iBase+4, vBase2+0);
            res->SetIndex(iBase+5, vBase2+1);

            iBase += 6;
            vBase1 = IDX_BY_POS(xs, ys, zSubdivisions);
            vBase2 = IDX_BY_POS(xs, ys+1, zSubdivisions);
            res->SetIndex(iBase+0, vBase1+2);
            res->SetIndex(iBase+1, vBase1+3);
            res->SetIndex(iBase+2, vBase2+0);
            res->SetIndex(iBase+3, vBase1+2);
            res->SetIndex(iBase+4, vBase2+0);
            res->SetIndex(iBase+5, vBase2+1);
        }

    res->BuildBuffers();
    res->renderDataObject->AttachVertices(gridPolygonGroup->renderDataObject);
    res->aabbox=gridPolygonGroup->aabbox;
    return res;
}

#undef IDX_BY_POS

    
};