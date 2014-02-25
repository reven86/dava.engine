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
    
void StaticOcclusionBuildSystem::BuildOcclusionInformation()
{
    if (entities.size() == 0)return;
    activeIndex = 0;
    needSetupNextOcclusion = true;
    if (!staticOcclusion)
        staticOcclusion = new StaticOcclusion();
}
    
void StaticOcclusionBuildSystem::Process(float32 timeElapsed)
{
    //ProcessStaticOcclusion(camera);
    
    if (activeIndex == -1)return; // System inactive
    
    Entity * entity = entities[activeIndex];
    StaticOcclusionComponent * occlusionComponent = (StaticOcclusionComponent*)entity->GetComponent(Component::STATIC_OCCLUSION_COMPONENT);
    TransformComponent * transformComponent = (TransformComponent*)entity->GetComponent(Component::TRANSFORM_COMPONENT);
    
    if (needSetupNextOcclusion)
    {
        AABBox3 localBox = occlusionComponent->GetBoundingBox();
        AABBox3 worldBox;
        localBox.GetTransformedBox(transformComponent->GetWorldTransform(), worldBox);
        
        // Prepare render objects
        Vector<Entity*> entities;
        Vector<RenderObject*> renderObjectsArray;
        GetScene()->GetChildEntitiesWithComponent(entities, Component::RENDER_COMPONENT);
        
        uint32 size = (uint32)entities.size();
        renderObjectsArray.resize(size);
        for(uint32 k = 0; k < size; ++k)
            renderObjectsArray[k] = GetRenderObject(entities[k]);
        
        // Prepare occlusion
        componentInProgress = (StaticOcclusionDataComponent*)Component::CreateByType(Component::STATIC_OCCLUSION_DATA_COMPONENT);
        
        StaticOcclusionData & data = componentInProgress->GetData();
        
        data.Init(occlusionComponent->GetSubdivisionsX(),
                  occlusionComponent->GetSubdivisionsY(),
                  occlusionComponent->GetSubdivisionsZ(),
                  size,
                  worldBox);
        
        staticOcclusion->SetScene(GetScene());
        staticOcclusion->SetRenderSystem(GetScene()->GetRenderSystem());
        staticOcclusion->BuildOcclusionInParallel(renderObjectsArray,
                                                  &data,
                                                  GetScene()->GetRenderSystem()->GetRenderHierarchy());
        
        
        
        Map<RenderObject*, Vector<RenderObject*> > equalRenderObjects;
        Vector<Entity*> lodEntities;
        GetScene()->GetChildEntitiesWithComponent(lodEntities, Component::LOD_COMPONENT);
        
        // VB: This code will require changes )))
        size = (uint32)lodEntities.size();
        for(uint32 k = 0; k < size; ++k)
        {
            LodComponent * lodComponent = (LodComponent*)lodEntities[k]->GetComponent(Component::LOD_COMPONENT);
            
            // VB: Why I hate current lod system starts here.
//            Vector<DAVA::LodComponent::LodData *> retLodLayers;
//            lodComponent->GetLodData(retLodLayers);
//            
//            RenderObject * lod0Object = 0;
//            Vector<RenderObject*> others;
//            for (uint32 lodLayer = 0; lodLayer < (uint32)retLodLayers.size(); ++lodLayer)
//            {
//                if (retLodLayers[lodLayer]->layer == 0)
//                {
//                    DVASSERT(retLodLayers[lodLayer]->nodes.size() == 1);
//                    lod0Object = GetRenderObject(retLodLayers[lodLayer]->nodes[0]);
//                    DVASSERT(lod0Object != 0);
//                }else
//                {
//                    for (uint32 p = 0; p < retLodLayers[lodLayer]->nodes.size(); ++p)
//                    {
//                        RenderObject * ro = GetRenderObject(retLodLayers[lodLayer]->nodes[p]);
//                        DVASSERT(ro != 0);
//                        others.push_back(ro);
//                    }
//                }
//            }
//            if (lod0Object)
//                equalRenderObjects[lod0Object] = others;
        }
        // VB: This code will require changes )))
        size = (uint32)lodEntities.size();
        for(uint32 k = 0; k < size; ++k)
        {
            LodComponent * lodComponent = (LodComponent*)lodEntities[k]->GetComponent(Component::LOD_COMPONENT);
            lodComponent->SetForceLodLayer(0);
        }
        GetScene()->lodSystem->SetForceUpdateAll();
        GetScene()->lodSystem->Process(timeElapsed);

        
        staticOcclusion->SetEqualVisibilityVector(equalRenderObjects);
        
        needSetupNextOcclusion = false;
    }else
    {
        //Logger::FrameworkDebug("start");
        uint32 result = 0;
        while(1)
        {
            result = staticOcclusion->RenderFrame();
            if (result == 0)break;
        }
        //Logger::FrameworkDebug("end");
        if (result == 0)
        {
            
            //occlusionComponent->renderPositions = staticOcclusion->renderPositions;
            
            // Remove old component
            entities[activeIndex]->RemoveComponent(Component::STATIC_OCCLUSION_DATA_COMPONENT);
            // Add new component
            entities[activeIndex]->AddComponent(componentInProgress);
            componentInProgress = 0;
            
            needSetupNextOcclusion = true;
            activeIndex++;
            if (activeIndex == entities.size())
            {
                activeIndex = -1;
            }
            
            Vector<Entity*> lodEntities;
            GetScene()->GetChildEntitiesWithComponent(lodEntities, Component::LOD_COMPONENT);
            // VB: This code will require changes )))
            uint32 size = (uint32)lodEntities.size();
            for(uint32 k = 0; k < size; ++k)
            {
                LodComponent * lodComponent = (LodComponent*)lodEntities[k]->GetComponent(Component::LOD_COMPONENT);
                lodComponent->SetForceLodLayer(LodComponent::INVALID_LOD_LAYER);
            }
            GetScene()->lodSystem->Process(timeElapsed);
        }
    }
}
    
    

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
        uint32 index = k / 32;
        uint32 shift = k & 31;
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
    // Verify that system is initialized
    if (!camera)return;

    uint32 size = (uint32)staticOcclusionComponents.size();
    if (size == 0)return;
    
    
    bool notInPVS = true;
    bool needUpdatePVS = false;
    
    for (uint32 k = 0; k < size; ++k)
    {
        StaticOcclusionData * data = &staticOcclusionComponents[k]->GetData();
        if (!data)return;
        
        const Vector3 & position = camera->GetPosition();
        if (data->bbox.IsInside(position))
        {
            uint32 x = (uint32)((position.x - data->bbox.min.x) / (data->bbox.max.x - data->bbox.min.x) * (float32)data->sizeX);
            uint32 y = (uint32)((position.y - data->bbox.min.y) / (data->bbox.max.y - data->bbox.min.y) * (float32)data->sizeY);
            uint32 z = (uint32)((position.z - data->bbox.min.z) / (data->bbox.max.z - data->bbox.min.z) * (float32)data->sizeZ);
            
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
    
}

    
};