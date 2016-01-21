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

#include "Scene/System/EditorStatisticsSystem.h"

#include "Scene3D/Components/LodComponent.h"

#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/ComponentHelpers.h"


using namespace DAVA;

// void LODComponentHolder::SummarizeValues()
// {
//     Array<float32, LodComponent::MAX_LOD_LAYERS> lodDistances;
//     lodDistances.fill(0.f);
//     trianglesCount.fill(0u);
// 
//     maxLodLayerIndex = LodComponent::INVALID_LOD_LAYER;
// 
//     uint32 count = static_cast<uint32> (lodComponents.size());
//     if (count > 0)
//     {
//         UnorderedSet<RenderObject*> renderObjects;
//         renderObjects.reserve(count);
// 
//         for (auto & lc : lodComponents)
//         {
//             maxLodLayerIndex = Max(maxLodLayerIndex, static_cast<int32>(GetLodLayersCount(lc)) - 1);
// 
//             for (uint32 i = 0; i < LodComponent::MAX_LOD_LAYERS; ++i)
//             {
//                 lodDistances[i] += lc->GetLodLayerDistance(i);
//             }
// 
//             InsertObjectWithTriangles(lc->GetEntity(), renderObjects);
//         }
// 
//         for (uint32 i = 0; i < LodComponent::MAX_LOD_LAYERS; ++i)
//         {
//             lodDistances[i] /= count;
//         }
// 
//         std::sort(lodDistances.begin(), lodDistances.end());
// 
//         CalculateTriangles(renderObjects);
//     }
// 
//     for (uint32 i = 0; i < LodComponent::MAX_LOD_LAYERS; ++i)
//     {
//         mergedComponent.SetLodLayerDistance(i, lodDistances[i]);
//     }
// }

// void LODComponentHolder::InsertObjectWithTriangles(Entity *entity, UnorderedSet<RenderObject *> &renderObjects)
// {
//     RenderObject * ro = GetRenderObject(entity);
//     if (ro && (ro->GetType() == RenderObject::TYPE_MESH || ro->GetType() == RenderObject::TYPE_SPEED_TREE || ro->GetType() == RenderObject::TYPE_SKINNED_MESH))
//     {
//         renderObjects.insert(ro);
//     }
// 
//     int32 count = entity->GetChildrenCount();
//     for (int32 i = 0; i < count; ++i)
//     {
//         Entity *child = entity->GetChild(i);
//         if (! HasComponent(child, Component::LOD_COMPONENT))
//         {
//             InsertObjectWithTriangles(child, renderObjects);
//         }
//     }
// }
// 
// void LODComponentHolder::CalculateTriangles(const UnorderedSet<RenderObject*> &renderObjects)
// {
//     bool onlyVisibleBatches = false; // to save leagcy code
// 
//     for (auto & ro : renderObjects)
//     {
//         uint32 batchCount = ro->GetRenderBatchCount();
//         for (uint32 b = 0; b < batchCount; ++b)
//         {
//             int32 lodIndex = 0;
//             int32 switchIndex = 0;
// 
//             RenderBatch *rb = ro->GetRenderBatch(b, lodIndex, switchIndex);
//             if (lodIndex < 0)
//             {
//                 continue;
//             }
// 
//             if (IsPointerToExactClass<RenderBatch>(rb))
//             {
//                 if (onlyVisibleBatches)
//                 { //check batch visibility
// 
//                     bool batchIsVisible = false;
//                     uint32 activeBatchCount = ro->GetActiveRenderBatchCount();
//                     for (uint32 a = 0; a < activeBatchCount; ++a)
//                     {
//                         RenderBatch *visibleBatch = ro->GetActiveRenderBatch(a);
//                         if (visibleBatch == rb)
//                         {
//                             batchIsVisible = true;
//                             break;
//                         }
//                     }
// 
//                     if (batchIsVisible == false) // need to skip this render batch
//                     {
//                         continue;
//                     }
//                 }
// 
//                 PolygonGroup *pg = rb->GetPolygonGroup();
//                 if (nullptr != pg)
//                 {
//                     DVASSERT(lodIndex < LodComponent::MAX_LOD_LAYERS);
//                     DVASSERT(lodIndex >= 0);
//                     trianglesCount[lodIndex] += pg->GetIndexCount() / 3;
//                 }
//             }
//         }
//     }
// }


EditorStatisticsSystem::EditorStatisticsSystem(Scene* scene)
    : SceneSystem(scene)
{
    uint32 a = 0;
    for (uint32 m = 0; m < eEditorMode::MODE_COUNT; ++m)
    {
        triangles[m].resize(LodComponent::MAX_LOD_LAYERS + 1);
        for (auto & tr : triangles[m])
        {
            tr = ++a;
        }
    }
}



void EditorStatisticsSystem::Process(float32 timeElapsed)
{
}

const DAVA::Vector<DAVA::uint32> &EditorStatisticsSystem::GetTriangles(eEditorMode mode) const
{
    return triangles[mode];
}
