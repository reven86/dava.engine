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
#include "Render/Highlevel/Landscape.h"
#include "Render/Material/NMaterialNames.h"
#include "Debug/Stats.h"

namespace DAVA
{
    
    
//
// Static Occlusion System
//
    
void StaticOcclusionSystem::UndoOcclusionVisibility()
{
    for (auto ro : indexedRenderObjects)
    {
        if (ro != nullptr)
        {
            ro->SetFlags(ro->GetFlags() | RenderObject::VISIBLE_STATIC_OCCLUSION);
        }
    }

    activeBlockIndex = 0;
    activePVSSet = nullptr;
}

void StaticOcclusionSystem::ProcessStaticOcclusionForOneDataSet(uint32 blockIndex, StaticOcclusionData * data)
{
//#define LOG_DEBUG_OCCLUSION_APPLY
#if defined(LOG_DEBUG_OCCLUSION_APPLY)
    uint32 visCount = 0;
    uint32 invisCount = 0;
#endif

    uint32* bitdata = data->GetBlockVisibilityData(blockIndex);
    uint32 size = (uint32)indexedRenderObjects.size();
    for (uint32 k = 0; k < size; ++k)
    {
        uint32 index = k / 32; // number of bits in uint32
        uint32 shift = k & 31; // bitmask for uint32

        RenderObject* ro = indexedRenderObjects[k];
        if (nullptr == ro)
            continue;

        if (bitdata[index] & (1 << shift))
        {
            ro->SetFlags(ro->GetFlags() | RenderObject::VISIBLE_STATIC_OCCLUSION);

#if defined(LOG_DEBUG_OCCLUSION_APPLY)
            visCount++;
#endif
        }
        else
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
        indexedRenderObjects[k] = nullptr;
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

    bool updateNotInPVS = true;
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
                        updateNotInPVS = false;
                    }
                }
            }
        }
    }

    if (updateNotInPVS && isInPvs)
    {
        UndoOcclusionVisibility();
        isInPvs = false;
    }
    else
    {
        isInPvs = true;
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
        DVASSERT(indexedRenderObjects[renderObject->GetStaticOcclusionIndex()] == nullptr);

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
        {
            indexedRenderObjects[i]->SetStaticOcclusionIndex(INVALID_STATIC_OCCLUSION_INDEX);
            indexedRenderObjects[i]->AddFlag(RenderObject::VISIBLE_STATIC_OCCLUSION);
        }
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
    RenderObject* renderObject = GetRenderObject(entity);

    if (renderObject != nullptr)
    {
        renderObject->SetStaticOcclusionIndex(INVALID_STATIC_OCCLUSION_INDEX);
        renderObject->AddFlag(RenderObject::VISIBLE_STATIC_OCCLUSION);
    }

    for (int32 i = 0, sz = entity->GetChildrenCount(); i < sz; ++i)
    {
        InvalidateOcclusionIndicesRecursively(entity->GetChild(i));
    }
}

StaticOcclusionDebugDrawSystem::StaticOcclusionDebugDrawSystem(Scene *scene):SceneSystem(scene)
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::WORLD_TRANSFORM_CHANGED);
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::STATIC_OCCLUSION_COMPONENT_CHANGED);

    Color gridColor(0.0f, 0.3f, 0.1f, 0.2f);
    Color coverColor(0.1f, 0.5f, 0.1f, 0.3f);

    gridMaterial = new NMaterial();
    gridMaterial->SetMaterialName(FastName("DebugQcclusionGridMaterial"));
    gridMaterial->SetFXName(NMaterialName::DEBUG_DRAW_ALPHABLEND);
    gridMaterial->AddProperty(FastName("color"), gridColor.color, rhi::ShaderProp::TYPE_FLOAT4);

    coverMaterial = new NMaterial();
    coverMaterial->SetMaterialName(FastName("DebugQcclusionCoverMaterial"));
    coverMaterial->SetFXName(NMaterialName::DEBUG_DRAW_ALPHABLEND);    
    coverMaterial->AddProperty(FastName("color"), coverColor.color, rhi::ShaderProp::TYPE_FLOAT4);

    rhi::VertexLayout vertexLayout;
    vertexLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    vertexLayoutId = rhi::VertexLayout::UniqueId(vertexLayout);
}

StaticOcclusionDebugDrawSystem::~StaticOcclusionDebugDrawSystem()
{
    GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::WORLD_TRANSFORM_CHANGED);
    GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::STATIC_OCCLUSION_COMPONENT_CHANGED);

    SafeRelease(gridMaterial);
    SafeRelease(coverMaterial);

}

void StaticOcclusionDebugDrawSystem::AddEntity(Entity * entity)
{

    StaticOcclusionComponent *staticOcclusionComponent = static_cast<StaticOcclusionComponent*>(entity->GetComponent(Component::STATIC_OCCLUSION_COMPONENT));
    Matrix4 * worldTransformPointer = GetTransformComponent(entity)->GetWorldTransformPtr();
    //create render object
    ScopedPtr<RenderObject> debugRenderObject(new RenderObject());
    debugRenderObject->SetWorldTransformPtr(worldTransformPointer);
    ScopedPtr<RenderBatch> gridBatch(new RenderBatch());
    ScopedPtr<RenderBatch> coverBatch(new RenderBatch());
    gridBatch->SetMaterial(gridMaterial);
    gridBatch->vertexLayoutId = vertexLayoutId;
    gridBatch->primitiveType = rhi::PRIMITIVE_LINELIST;
    coverBatch->SetMaterial(coverMaterial);
    coverBatch->vertexLayoutId = vertexLayoutId;                

    debugRenderObject->AddRenderBatch(coverBatch);
    debugRenderObject->AddRenderBatch(gridBatch);
    StaticOcclusionDebugDrawComponent *debugDrawComponent = new StaticOcclusionDebugDrawComponent(debugRenderObject);
    entity->AddComponent(debugDrawComponent);
    
    UpdateGeometry(debugDrawComponent);        
                            
    GetScene()->GetRenderSystem()->RenderPermanent(debugRenderObject);        

}

void StaticOcclusionDebugDrawSystem::RemoveEntity(Entity * entity)
{
    StaticOcclusionDebugDrawComponent* debugDrawComponent = GetStaticOcclusionDebugDrawComponent(entity);
    DVASSERT(debugDrawComponent != nullptr);
    GetScene()->GetRenderSystem()->RemoveFromRender(debugDrawComponent->GetRenderObject());
    entity->RemoveComponent(Component::STATIC_OCCLUSION_DEBUG_DRAW_COMPONENT);    
}


void StaticOcclusionDebugDrawSystem::ImmediateEvent(Component * component, uint32 event)
{
    Entity * entity = component->GetEntity();
    StaticOcclusionDebugDrawComponent* debugDrawComponent = GetStaticOcclusionDebugDrawComponent(entity);
    StaticOcclusionComponent* staticOcclusionComponent = GetStaticOcclusionComponent(entity);
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
        UpdateGeometry(debugDrawComponent);                
    }
}

void StaticOcclusionDebugDrawSystem::UpdateGeometry(StaticOcclusionDebugDrawComponent * component)
{
    Entity * entity = component->GetEntity();
    StaticOcclusionComponent *staticOcclusionComponent = static_cast<StaticOcclusionComponent*>(entity->GetComponent(Component::STATIC_OCCLUSION_COMPONENT));
    DVASSERT(staticOcclusionComponent);        

    CreateStaticOcclusionDebugDrawVertices(component, staticOcclusionComponent);
    CreateStaticOcclusionDebugDrawGridIndice(component, staticOcclusionComponent);
    CreateStaticOcclusionDebugDrawCoverIndice(component, staticOcclusionComponent);

    RenderObject *debugRenderObject = component->renderObject;
    debugRenderObject->GetRenderBatch(0)->vertexBuffer = component->vertices;
    debugRenderObject->GetRenderBatch(0)->vertexCount = component->vertexCount;
    debugRenderObject->GetRenderBatch(0)->indexBuffer = component->coverIndices;
    debugRenderObject->GetRenderBatch(0)->indexCount = component->coverIndexCount;

    debugRenderObject->GetRenderBatch(1)->vertexBuffer = component->vertices;
    debugRenderObject->GetRenderBatch(1)->vertexCount = component->vertexCount;
    debugRenderObject->GetRenderBatch(1)->indexBuffer = component->gridIndices;
    debugRenderObject->GetRenderBatch(1)->indexCount = component->gridIndexCount;

    debugRenderObject->SetAABBox(component->bbox);
    entity->GetScene()->renderSystem->MarkForUpdate(debugRenderObject);
}


#define IDX_BY_POS(xc, yc, zc) ((zc) + (zSubdivisions+1)*((yc) + (xc) * ySubdivisions))*4

void StaticOcclusionDebugDrawSystem::CreateStaticOcclusionDebugDrawVertices(StaticOcclusionDebugDrawComponent *target, StaticOcclusionComponent *source)
{
    rhi::DeleteVertexBuffer(target->vertices);

    uint32 xSubdivisions = source->GetSubdivisionsX();
    uint32 ySubdivisions = source->GetSubdivisionsY();
    uint32 zSubdivisions = source->GetSubdivisionsZ();
    
    int32 vertexCount = xSubdivisions * ySubdivisions * 4 * (zSubdivisions + 1);    
    target->vertexCount = vertexCount;
    target->vertices = rhi::CreateVertexBuffer(vertexCount * 4 * 3);

    const AABBox3& srcBBox = source->GetBoundingBox();
    Vector3 boxSize = srcBBox.GetSize();
    boxSize.x /= xSubdivisions;
    boxSize.y /= ySubdivisions;
    boxSize.z /= zSubdivisions;

    const float32* cellHeightOffset = source->GetCellHeightOffsets();

    std::unique_ptr<Vector3[]> mesh(new Vector3[vertexCount]);
    AABBox3 resBBox;
    //vertices
    //as we are going to place blocks on landscape we are to treat each column as independent - not sharing vertices between columns. we can still share vertices within 1 column
    for (uint32 xs = 0; xs < xSubdivisions; ++xs)
        for (uint32 ys = 0; ys < ySubdivisions; ++ys)
            for (uint32 zs = 0; zs < (zSubdivisions + 1); ++zs)
            {
                int32 vBase = IDX_BY_POS(xs, ys, zs);
                float32 hOffset = cellHeightOffset ? cellHeightOffset[xs + ys*xSubdivisions] : 0;
                mesh[vBase + 0] = srcBBox.min + Vector3(boxSize.x * xs, boxSize.y * ys, boxSize.z * zs + hOffset);              resBBox.AddPoint(mesh[vBase + 0]);
                mesh[vBase + 1] = srcBBox.min + Vector3(boxSize.x * (xs + 1), boxSize.y * ys, boxSize.z * zs + hOffset);        resBBox.AddPoint(mesh[vBase + 1]);
                mesh[vBase + 2] = srcBBox.min + Vector3(boxSize.x * (xs + 1), boxSize.y * (ys + 1), boxSize.z * zs + hOffset);  resBBox.AddPoint(mesh[vBase + 2]);
                mesh[vBase + 3] = srcBBox.min + Vector3(boxSize.x * xs, boxSize.y * (ys + 1), boxSize.z * zs + hOffset);        resBBox.AddPoint(mesh[vBase + 3]);                
            }
    rhi::UpdateVertexBuffer(target->vertices, mesh.get(), 0, vertexCount * 4 * 3);
    target->bbox = resBBox;
}

void StaticOcclusionDebugDrawSystem::CreateStaticOcclusionDebugDrawGridIndice(StaticOcclusionDebugDrawComponent *target, StaticOcclusionComponent *source)
{
    rhi::DeleteIndexBuffer(target->gridIndices);
    uint32 xSubdivisions = source->GetSubdivisionsX();
    uint32 ySubdivisions = source->GetSubdivisionsY();
    uint32 zSubdivisions = source->GetSubdivisionsZ();
    uint32 indexCount = xSubdivisions * ySubdivisions * zSubdivisions * 12 * 2; //12 lines per box 2 indices per line
    target->gridIndexCount = indexCount;

    target->gridIndices = rhi::CreateIndexBuffer(indexCount * 2);
    std::unique_ptr<uint16[]> meshIndices(new uint16[indexCount]);
    //in pair indexOffset, z
    const static int32 indexOffsets[]={0,0, 1,0, 1,0, 2,0, 2,0, 3,0, 3,0, 0,0,  //bot
                                       0,0, 0,1, 1,0, 1,1, 2,0, 2,1, 3,0, 3,1,  //mid
                                       0,1, 1,1, 1,1, 2,1, 2,1, 3,1, 3,1, 0,1}; //top

    for (uint32 xs = 0; xs < xSubdivisions; ++xs)
        for (uint32 ys = 0; ys < ySubdivisions; ++ys)
            for (uint32 zs = 0; zs < zSubdivisions; ++zs)
            {
                int32 iBase = (zs + zSubdivisions*(ys + xs * ySubdivisions)) * 24;
                int32 vBase[2] = { static_cast<int32>(IDX_BY_POS(xs, ys, zs)), static_cast<int32>(IDX_BY_POS(xs, ys, zs + 1)) };
                for (int32 i = 0; i < 24; i++)
                    meshIndices[iBase + i] = indexOffsets[i * 2] + vBase[indexOffsets[i * 2 + 1]];

            }

    rhi::UpdateIndexBuffer(target->gridIndices, meshIndices.get(), 0, indexCount * 2);
}

void StaticOcclusionDebugDrawSystem::CreateStaticOcclusionDebugDrawCoverIndice(StaticOcclusionDebugDrawComponent *target, StaticOcclusionComponent *source)
{
    rhi::DeleteIndexBuffer(target->coverIndices);

    uint32 xSubdivisions = source->GetSubdivisionsX();
    uint32 ySubdivisions = source->GetSubdivisionsY();
    uint32 zSubdivisions = source->GetSubdivisionsZ();

    int32 xSideIndexCount = xSubdivisions * 6 * 2;
    int32 ySideIndexCount = ySubdivisions * 6 * 2;
    int32 xySideIndexCount = xSideIndexCount + ySideIndexCount;
    int32 zSideIndexCount = xSubdivisions * ySubdivisions * 6 * 2;
    int32 totalSideIndexCount = xySideIndexCount + zSideIndexCount;
    int32 xExtraIndexCount = (xSubdivisions - 1) * (ySubdivisions)* 6 * 2;
    int32 yExtraIndexCount = (ySubdivisions - 1) * (xSubdivisions)* 6 * 2;
    int32 indexCount = totalSideIndexCount + xExtraIndexCount + yExtraIndexCount;

    target->coverIndexCount = indexCount;

    target->coverIndices = rhi::CreateIndexBuffer(indexCount * 2);
    std::unique_ptr<uint16[]> meshIndices(new uint16[indexCount]);

    //left and right
    for (uint32 xs = 0; xs < xSubdivisions; ++xs)
    {
        int32 iBase = xs * 6 * 2;

        meshIndices[iBase + 0] = IDX_BY_POS(xs, 0, 0);        
        meshIndices[iBase + 1] = IDX_BY_POS(xs, 0, 0) + 1;
        meshIndices[iBase + 2] = IDX_BY_POS(xs, 0, zSubdivisions) + 1;
        meshIndices[iBase + 3] = IDX_BY_POS(xs, 0, 0);
        meshIndices[iBase + 4] = IDX_BY_POS(xs, 0, zSubdivisions) + 1;
        meshIndices[iBase + 5] = IDX_BY_POS(xs, 0, zSubdivisions);

        iBase = xs * 6 * 2 + 6;

        meshIndices[iBase + 0] = IDX_BY_POS(xs, ySubdivisions - 1, 0) + 3;
        meshIndices[iBase + 1] = IDX_BY_POS(xs, ySubdivisions - 1, 0) + 2;
        meshIndices[iBase + 2] = IDX_BY_POS(xs, ySubdivisions - 1, zSubdivisions) + 2;
        meshIndices[iBase + 3] = IDX_BY_POS(xs, ySubdivisions - 1, 0) + 3;
        meshIndices[iBase + 4] = IDX_BY_POS(xs, ySubdivisions - 1, zSubdivisions) + 2;
        meshIndices[iBase + 5] = IDX_BY_POS(xs, ySubdivisions - 1, zSubdivisions) + 3;
    }

    //front and back
    for (uint32 ys = 0; ys < ySubdivisions; ++ys)
    {
        int32 iBase = xSideIndexCount + ys * 6 * 2;

        meshIndices[iBase + 0] = IDX_BY_POS(0, ys, 0);
        meshIndices[iBase + 1] = IDX_BY_POS(0, ys, 0) + 3;
        meshIndices[iBase + 2] = IDX_BY_POS(0, ys, zSubdivisions) + 3;
        meshIndices[iBase + 3] = IDX_BY_POS(0, ys, 0);
        meshIndices[iBase + 4] = IDX_BY_POS(0, ys, zSubdivisions) + 3;
        meshIndices[iBase + 5] = IDX_BY_POS(0, ys, zSubdivisions);

        iBase = xSideIndexCount + ys * 6 * 2 + 6;

        meshIndices[iBase + 0] = IDX_BY_POS(xSubdivisions - 1, ys, 0) + 1;
        meshIndices[iBase + 1] = IDX_BY_POS(xSubdivisions - 1, ys, 0) + 2;
        meshIndices[iBase + 2] = IDX_BY_POS(xSubdivisions - 1, ys, zSubdivisions) + 2;
        meshIndices[iBase + 3] = IDX_BY_POS(xSubdivisions - 1, ys, 0) + 1;
        meshIndices[iBase + 4] = IDX_BY_POS(xSubdivisions - 1, ys, zSubdivisions) + 2;
        meshIndices[iBase + 5] = IDX_BY_POS(xSubdivisions - 1, ys, zSubdivisions) + 1;
    }

    //bot and top
    for (uint32 xs = 0; xs < xSubdivisions; ++xs)
        for (uint32 ys = 0; ys < ySubdivisions; ++ys)
        {
            int32 iBase = xySideIndexCount + (ys*xSubdivisions + xs) * 6 * 2;
            int32 vBase = IDX_BY_POS(xs, ys, 0);
            meshIndices[iBase + 0] = vBase + 0;
            meshIndices[iBase + 1] = vBase + 1;
            meshIndices[iBase + 2] = vBase + 2;
            meshIndices[iBase + 3] = vBase + 0;
            meshIndices[iBase + 4] = vBase + 2;
            meshIndices[iBase + 5] = vBase + 3;

            iBase = xySideIndexCount + (ys*xSubdivisions + xs) * 6 * 2 + 6;
            vBase = IDX_BY_POS(xs, ys, zSubdivisions);
            meshIndices[iBase + 0] = vBase + 0;
            meshIndices[iBase + 1] = vBase + 1;
            meshIndices[iBase + 2] = vBase + 2;
            meshIndices[iBase + 3] = vBase + 0;
            meshIndices[iBase + 4] = vBase + 2;
            meshIndices[iBase + 5] = vBase + 3;
        }

    //extras across x axis
    for (uint32 xs = 0; xs < (xSubdivisions - 1); ++xs)
        for (uint32 ys = 0; ys < ySubdivisions; ++ys)
        {
            int32 iBase = totalSideIndexCount + (xs*ySubdivisions + ys) * 6 * 2;
            int32 vBase1 = IDX_BY_POS(xs, ys, 0);
            int32 vBase2 = IDX_BY_POS(xs + 1, ys, 0);
            meshIndices[iBase + 0] = vBase1 + 1;
            meshIndices[iBase + 1] = vBase1 + 2;
            meshIndices[iBase + 2] = vBase2 + 3;
            meshIndices[iBase + 3] = vBase1 + 1;
            meshIndices[iBase + 4] = vBase2 + 3;
            meshIndices[iBase + 5] = vBase2 + 0;

            iBase += 6;
            vBase1 = IDX_BY_POS(xs, ys, zSubdivisions);
            vBase2 = IDX_BY_POS(xs + 1, ys, zSubdivisions);
            meshIndices[iBase + 0] = vBase1 + 1;
            meshIndices[iBase + 1] = vBase1 + 2;
            meshIndices[iBase + 2] = vBase2 + 3;
            meshIndices[iBase + 3] = vBase1 + 1;
            meshIndices[iBase + 4] = vBase2 + 3;
            meshIndices[iBase + 5] = vBase2 + 0;
        }

    //extras across y axis
    for (uint32 xs = 0; xs < xSubdivisions; ++xs)
        for (uint32 ys = 0; ys < (ySubdivisions - 1); ++ys)
        {
            int32 iBase = totalSideIndexCount + xExtraIndexCount + (ys*xSubdivisions + xs) * 6 * 2;
            int32 vBase1 = IDX_BY_POS(xs, ys, 0);
            int32 vBase2 = IDX_BY_POS(xs, ys + 1, 0);
            meshIndices[iBase + 0] = vBase1 + 2;
            meshIndices[iBase + 1] = vBase1 + 3;
            meshIndices[iBase + 2] = vBase2 + 0;
            meshIndices[iBase + 3] = vBase1 + 2;
            meshIndices[iBase + 4] = vBase2 + 0;
            meshIndices[iBase + 5] = vBase2 + 1;

            iBase += 6;
            vBase1 = IDX_BY_POS(xs, ys, zSubdivisions);
            vBase2 = IDX_BY_POS(xs, ys + 1, zSubdivisions);
            meshIndices[iBase + 0] = vBase1 + 2;
            meshIndices[iBase + 1] = vBase1 + 3;
            meshIndices[iBase + 2] = vBase2 + 0;
            meshIndices[iBase + 3] = vBase1 + 2;
            meshIndices[iBase + 4] = vBase2 + 0;
            meshIndices[iBase + 5] = vBase2 + 1;
        }

    rhi::UpdateIndexBuffer(target->coverIndices, meshIndices.get(), 0, indexCount * 2);
}

#undef IDX_BY_POS

    
};