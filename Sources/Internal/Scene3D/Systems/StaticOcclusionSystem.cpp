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

namespace DAVA
{
StaticOcclusionBuildSystem::StaticOcclusionBuildSystem(Scene * scene)
:	SceneSystem(scene)
{
    staticOcclusion = 0;
    activeIndex = -1;
    currentDataInProcess = 0;
}

StaticOcclusionBuildSystem::~StaticOcclusionBuildSystem()
{
    SafeDelete(staticOcclusion);
}

void StaticOcclusionBuildSystem::AddEntity(Entity * entity)
{
    entities.push_back(entity);
    computedOcclusionInfo.resize(entities.size());
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
    
void StaticOcclusionBuildSystem::Process()
{
    ProcessStaticOcclusion(camera);
    
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
        currentDataInProcess = new StaticOcclusionData();
        currentDataInProcess->Init(occlusionComponent->GetSubdivisionsX(),
                                   occlusionComponent->GetSubdivisionsY(),
                                   occlusionComponent->GetSubdivisionsZ(), size);
        currentDataInProcess->bbox = worldBox;
        
        staticOcclusion->SetScene(GetScene());
        staticOcclusion->SetRenderSystem(GetScene()->GetRenderSystem());
        staticOcclusion->BuildOcclusionInParallel(worldBox,
                                                  occlusionComponent->GetSubdivisionsX(),
                                                  occlusionComponent->GetSubdivisionsY(),
                                                  occlusionComponent->GetSubdivisionsZ(),
                                                  renderObjectsArray,
                                                  currentDataInProcess,
                                                  GetScene()->GetRenderSystem()->GetRenderHierarchy());
        
        indexedRenderObjects.resize(size);
        for (uint32 k = 0; k < size; ++k)
        {
            indexedRenderObjects[renderObjectsArray[k]->GetStaticOcclusionIndex()] = renderObjectsArray[k];
        }
        
        needSetupNextOcclusion = false;
    }else
    {
        uint32 result = staticOcclusion->RenderFrame();
        if (result == 0)
        {
            computedOcclusionInfo[activeIndex] = currentDataInProcess;
            currentDataInProcess = 0;

            needSetupNextOcclusion = true;
            activeIndex++;
            if (activeIndex == entities.size())
                activeIndex = -1;
        }
    }
}

void StaticOcclusionBuildSystem::ProcessStaticOcclusion(Camera * camera)
{
    uint32 size = (uint32)computedOcclusionInfo.size();
    for (uint32 k = 0; k < size; ++k)
    {
        StaticOcclusionData * data = computedOcclusionInfo[k];
        if (!data)return;
        
        const Vector3 & position = camera->GetPosition();
        if (data->bbox.IsInside(position))
        {
            uint32 x = (uint32)((position.x - data->bbox.min.x) / (data->bbox.max.x - data->bbox.min.x) / (float32)data->sizeX);
            uint32 y = (uint32)((position.y - data->bbox.min.y) / (data->bbox.max.y - data->bbox.min.y) / (float32)data->sizeY);
            uint32 z = (uint32)((position.z - data->bbox.min.z) / (data->bbox.max.z - data->bbox.min.z) / (float32)data->sizeZ);

            uint32 blockIndex = z * (data->sizeX * data->sizeY) + y * (data->sizeX) + (x);
        
            ProcessStaticOcclusionForOneDataSet(blockIndex, data);
        }else
            UndoOcclusionVisibility();
    }
}
void StaticOcclusionBuildSystem::UndoOcclusionVisibility()
{
    uint32 size = (uint32)indexedRenderObjects.size();
    for (uint32 k = 0; k < size; ++k)
    {
        RenderObject * ro = indexedRenderObjects[k];
        ro->SetFlags(ro->GetFlags() | RenderObject::VISIBLE_STATIC_OCCLUSION);
    }

}

void StaticOcclusionBuildSystem::ProcessStaticOcclusionForOneDataSet(uint32 blockIndex, StaticOcclusionData * data)
{
    uint32 * bitdata = data->GetBlockVisibilityData(blockIndex);
    uint32 size = (uint32)indexedRenderObjects.size();
    for (uint32 k = 0; k < size; ++k)
    {
        uint32 index = k / 32;
        uint32 shift = k & 31;
        RenderObject * ro = indexedRenderObjects[k];
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
    renderObjectArray.resize(3000);
    for (uint32 k = 0; k < renderObjectArray.size(); ++k)
        renderObjectArray[k] = 0;
    occlusionRenderObjectSize = 0;
}

StaticOcclusionSystem::~StaticOcclusionSystem()
{
}

void StaticOcclusionSystem::Process()
{
    
    
   
}
    
void StaticOcclusionSystem::AddEntity(Entity * entity)
{
    RenderComponent * renderComponent = (RenderComponent*)entity->GetComponent(Component::RENDER_COMPONENT);
    RenderObject * renderObject = renderComponent->GetRenderObject();
    
    if (renderObject->GetStaticOcclusionIndex() != INVALID_STATIC_OCCLUSION_INDEX)
    {
        if (renderObject->GetStaticOcclusionIndex() > occlusionRenderObjectSize)
        {
            occlusionRenderObjectSize = renderObject->GetStaticOcclusionIndex();
            if (occlusionRenderObjectSize > (uint32)renderObjectArray.size())
            {
                renderObjectArray.resize(occlusionRenderObjectSize);
            }
        }
        renderObjectArray[renderObject->GetStaticOcclusionIndex()] = renderObject;
    }
}
    
void StaticOcclusionSystem::RemoveEntity(Entity * entity)
{
    
}

    
};