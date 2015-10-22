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


#include "Scene3D/Systems/StaticOcclusionBuildSystem.h"
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
    occlusionEntities.push_back(entity);
}
    
void StaticOcclusionBuildSystem::RemoveEntity(Entity * entity)
{
    occlusionEntities.erase(std::remove(occlusionEntities.begin(), occlusionEntities.end(), entity), occlusionEntities.end());
}

void StaticOcclusionBuildSystem::ImmediateEvent(Component * _component, uint32 event)
{
    Entity * entity = _component->GetEntity();
    StaticOcclusionComponent *component = static_cast<StaticOcclusionComponent*>(entity->GetComponent(Component::STATIC_OCCLUSION_COMPONENT));
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
    if (occlusionEntities.empty())
        return;

    activeIndex = 0;

    if (nullptr == staticOcclusion)
        staticOcclusion = new StaticOcclusion();

    StartBuildOcclusion();
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

    for (int32 i = 0, sz = entity->GetChildrenCount(); i < sz; ++i)
        CollectEntitiesForOcclusionRecursively(dest, entity->GetChild(i));
}
    
void StaticOcclusionBuildSystem::StartBuildOcclusion()
{
    if (activeIndex == static_cast<uint32>(-1))
        return; // System inactive

    //global preparations
    SetCamera(GetScene()->GetCurrentCamera());

    // Prepare render objects
    Vector<Entity*> sceneEntities;
    Vector<RenderObject*> renderObjectsArray;
    Landscape* landscape = nullptr;
    CollectEntitiesForOcclusionRecursively(sceneEntities, GetScene());

    uint32 size = (uint32)sceneEntities.size();
    renderObjectsArray.reserve(size);
    DVASSERT(renderObjectsArray.size() == 0);
    for (uint32 k = 0; k < size; ++k)
    {
        RenderObject* renderObject = GetRenderObject(sceneEntities[k]);
        auto renderObjectType = renderObject->GetType();
        if ((RenderObject::TYPE_MESH == renderObjectType) || (RenderObject::TYPE_SPEED_TREE == renderObjectType))
        {
            renderObjectsArray.push_back(renderObject);
            renderObject->AddFlag(RenderObject::VISIBLE_STATIC_OCCLUSION);
        }
        if (RenderObject::TYPE_LANDSCAPE == renderObjectType)
        {
            landscape = static_cast<Landscape*>(renderObject);
        }
    }

    GetScene()->staticOcclusionSystem->ClearOcclusionObjects();

    uint16_t index = 0;
    for (auto& ro : renderObjectsArray)
    {
        // if we are going to renew indices they should be cleared prior to it
        DVASSERT(ro->GetStaticOcclusionIndex() == INVALID_STATIC_OCCLUSION_INDEX);
        ro->SetStaticOcclusionIndex(index++);
    }

    SceneForceLod(0);
    UpdateMaterialsForOcclusionRecursively(GetScene());

    // Prepare occlusion per component
    Entity* entity = occlusionEntities[activeIndex];

    componentInProgress = (StaticOcclusionDataComponent*)entity->GetComponent(Component::STATIC_OCCLUSION_DATA_COMPONENT);
    if (componentInProgress)
    {        
        // We detach component from system, to let system know that this data is not valid right now. Entity will be removed from system that apply occlusion information.         
        entity->DetachComponent(componentInProgress);
    }else
    {
        componentInProgress = new StaticOcclusionDataComponent();
    }
    StaticOcclusionData & data = componentInProgress->GetData();

    StaticOcclusionComponent* occlusionComponent = (StaticOcclusionComponent*)entity->GetComponent(Component::STATIC_OCCLUSION_COMPONENT);
    TransformComponent* transformComponent = (TransformComponent*)entity->GetComponent(Component::TRANSFORM_COMPONENT);
    AABBox3 localBox = occlusionComponent->GetBoundingBox();
    AABBox3 worldBox;
    localBox.GetTransformedBox(transformComponent->GetWorldTransform(), worldBox);

    data.Init(occlusionComponent->GetSubdivisionsX(), occlusionComponent->GetSubdivisionsY(),
              occlusionComponent->GetSubdivisionsZ(), size, worldBox, occlusionComponent->GetCellHeightOffsets());

    if (nullptr == staticOcclusion)
        staticOcclusion = new StaticOcclusion();

    staticOcclusion->StartBuildOcclusion(&data, GetScene()->GetRenderSystem(), landscape);       
}
    
void StaticOcclusionBuildSystem::FinishBuildOcclusion()
{
    Component* prevComponent = occlusionEntities[activeIndex]->GetComponent(Component::STATIC_OCCLUSION_DATA_COMPONENT);

    // We've detached component so we verify that here we still do not have this component.
    DVASSERT(prevComponent == 0);

    occlusionEntities[activeIndex]->AddComponent(componentInProgress);
    componentInProgress = 0;
    
    activeIndex++;
    if (activeIndex == occlusionEntities.size())
    {
        activeIndex = -1;
    }
    else
    {
        // not final index add more occlusion build cycle
        StartBuildOcclusion();
        return;
    }
 
    SceneForceLod(LodComponent::INVALID_LOD_LAYER);
    RestoreOcclusionMaterials();

    Scene* scene = GetScene();
    scene->staticOcclusionSystem->CollectOcclusionObjectsRecursively(scene);
    // SafeDelete(staticOcclusion);
}
    
bool StaticOcclusionBuildSystem::IsInBuild() const
{
    return (static_cast<uint32>(-1) != activeIndex);
}

uint32 StaticOcclusionBuildSystem::GetBuildStatus() const
{
    uint32 ret = 0;

    if (staticOcclusion)
    {
        uint32 currentStepsCount = staticOcclusion->GetCurrentStepsCount();
        uint32 totalStepsCount = staticOcclusion->GetTotalStepsCount();
        ret = (currentStepsCount * 100) / totalStepsCount;
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
#if RHI_COMPLETE
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
#endif //RHI_COMPLETE    
}

void StaticOcclusionBuildSystem::RestoreOcclusionMaterials()
{
#if RHI_COMPLETE
    for (Map<NMaterial*, RenderStateData>::iterator it = originalRenderStateData.begin(), e = originalRenderStateData.end(); it!=e; ++it)
        it->first->SubclassRenderState(PASS_FORWARD, it->second);
    
    originalRenderStateData.clear();
#endif //RHI_COMPLETE
}

void StaticOcclusionBuildSystem::Process(float32 timeElapsed)
{
    if (activeIndex == (uint32)(-1))
        return;

    bool finished = staticOcclusion->ProccessBlock();
    if (finished)
    {
        FinishBuildOcclusion();
    }
}   

    
};
