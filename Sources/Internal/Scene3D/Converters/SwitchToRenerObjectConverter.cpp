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


#include "SwitchToRenerObjectConverter.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Entity.h"
#include "DAVAEngine.h"

namespace DAVA {

void SwitchToRenerObjectConverter::ConsumeSwitchedRenderObjects(Entity * scene)
{
    SerachForSwitch(scene);
}

void SwitchToRenerObjectConverter::SerachForSwitch(Entity * currentNode)
{
    DVASSERT(currentNode);
    for (int32 c = 0; c < currentNode->GetChildrenCount(); ++c)
    {
        Entity * childNode = currentNode->GetChild(c);
        SerachForSwitch(childNode);
        bool wasReplace = MergeSwitch(childNode);
        if (wasReplace)
        {
            c--;
        }
    }
}

bool SwitchToRenerObjectConverter::MergeSwitch(Entity * entity)
{
    Vector<Entity*> entitiesToRemove;

    SwitchComponent * sw = GetSwitchComponent(entity);
    if (nullptr != sw)
    {
        RenderComponent * rc = GetRenderComponent(entity);
        RenderObject * ro = 0;
        if (nullptr == rc)
        {
            ro = new Mesh();
            rc = new RenderComponent(ro);
            ro->Release();

            ro->SetAABBox(AABBox3(Vector3(0, 0, 0), Vector3(0, 0, 0)));
            entity->AddComponent(rc);
        }
        else
        {
            ro = rc->GetRenderObject();
        }

        DVASSERT(ro);

        int32 size = entity->GetChildrenCount();
        for (int32 i = 0; i < size; ++i)
        {
            Entity * sourceEntity = entity->GetChild(i);
            RenderObject * sourceRenderObject = GetRenderObject(sourceEntity);

            //workaround for custom properties for crashed model
            if (1 == i) // crash model
            {
                KeyedArchive *childProps = GetCustomPropertiesArchieve(sourceEntity);
                if (nullptr != childProps && childProps->IsKeyExists("CollisionType"))
                {
                    KeyedArchive *entityProps = GetOrCreateCustomProperties(entity)->GetArchive();
                    entityProps->SetInt32("CollisionTypeCrashed", childProps->GetInt32("CollisionType", 0));
                }
            }
            //end of custom properties

            Vector<std::pair<Entity*, RenderObject*> > renderPairs;
            if (nullptr != sourceRenderObject)
            {
                renderPairs.push_back(std::make_pair(sourceEntity, sourceRenderObject));
            }
            else
            {
                FindRenderObjectsRecursive(sourceEntity, renderPairs);
                DVASSERT(renderPairs.size() == 1);
                sourceRenderObject = renderPairs[0].second;
            }

            if (nullptr != sourceRenderObject)
            {
                TransformComponent * sourceTransform = GetTransformComponent(sourceEntity);
                if (sourceTransform->GetLocalTransform() != Matrix4::IDENTITY)
                {
                    PolygonGroup * pg = sourceRenderObject->GetRenderBatchCount() > 0 ? sourceRenderObject->GetRenderBatch(0)->GetPolygonGroup() : nullptr;
                    if (nullptr != pg && bakedPolygonGroups.end() == bakedPolygonGroups.find(pg))
                    {
                        sourceRenderObject->BakeGeometry(sourceTransform->GetLocalTransform());
                        bakedPolygonGroups.insert(pg);
                    }
                }

                uint32 sourceSize = sourceRenderObject->GetRenderBatchCount();
                while (0 != sourceSize)
                {
                    int32 lodIndex, switchIndex;
                    RenderBatch * sourceRenderBatch = sourceRenderObject->GetRenderBatch(0, lodIndex, switchIndex);
                    sourceRenderBatch->Retain();
                    sourceRenderObject->RemoveRenderBatch(sourceRenderBatch);
                    ro->AddRenderBatch(sourceRenderBatch, lodIndex, i);
                    sourceRenderBatch->Release();
                    sourceSize--;
                }
            }

            renderPairs[0].first->RemoveComponent(Component::RENDER_COMPONENT);

            LodComponent * lc = GetLodComponent(sourceEntity);
            if ((nullptr != lc) && (nullptr == GetLodComponent(entity)))
            {
                LodComponent * newLod = (LodComponent*)lc->Clone(entity);
                entity->AddComponent(newLod);
            }

            renderPairs[0].first->RemoveComponent(Component::LOD_COMPONENT);

            if (sourceEntity->GetChildrenCount() == 0)
            {
                entitiesToRemove.push_back(sourceEntity);
            }
        }
    }

    uint32 entitiesToRemoveCount = static_cast<uint32>(entitiesToRemove.size());
    for (uint32 i = 0; i < entitiesToRemoveCount; ++i)
    {
        entitiesToRemove[i]->GetParent()->RemoveNode(entitiesToRemove[i]);
    }

    return false;
}

void SwitchToRenerObjectConverter::FindRenderObjectsRecursive(Entity * fromEntity, Vector<std::pair<Entity*, RenderObject*> > & entityAndObjectPairs)
{
    RenderObject * ro = GetRenderObject(fromEntity);
    if (nullptr != ro && ro->GetType() == RenderObject::TYPE_MESH)
    {
        entityAndObjectPairs.push_back(std::make_pair(fromEntity, ro));
    }

    int32 size = fromEntity->GetChildrenCount();
    for (int32 i = 0; i < size; ++i)
    {
        Entity * child = fromEntity->GetChild(i);
        FindRenderObjectsRecursive(child, entityAndObjectPairs);
    }
}

};