#include "LodToLod2Converter.h"
#include "DAVAEngine.h"

static DAVA::int32 emptyEntities = 0;

namespace DAVA
{
void LodToLod2Converter::ConvertLodToV2(Entity* scene)
{
    emptyEntities = 0;
    LodSystem::UpdateEntitiesAfterLoad(scene);
    SearchForLod(scene);
}

void LodToLod2Converter::SearchForLod(Entity* currentNode)
{
    for (int32 c = 0; c < currentNode->GetChildrenCount(); ++c)
    {
        Entity* childNode = currentNode->GetChild(c);
        SearchForLod(childNode);
        bool wasReplace = MergeLod(childNode);
        if (wasReplace)
        {
            c--;
        }
    }
}

bool LodToLod2Converter::MergeLod(Entity* entity)
{
    if (GetEffectComponent(entity))
    {
        return false;
    }

    bool res = false;

    Set<Entity*> entitiesToRemove;

    LodComponent* lod = GetLodComponent(entity);
    if (nullptr != lod)
    {
        RenderComponent* rc = GetRenderComponent(entity);
        RenderObject* ro = nullptr;
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
            if (ro->GetType() != RenderObject::TYPE_MESH)
            {
                return false;
            }
        }

        DVASSERT(ro);

        Vector<LodComponent::LodData*> lodData;
        lod->GetLodData(lodData);
        uint32 size = static_cast<uint32>(lodData.size());

        //search for same entity in different lods
        Set<Entity*> uniqueLodEntities;
        for (uint32 i = 0; i < size; ++i)
        {
            LodComponent::LodData* data = lodData[i];
            uint32 entitiesCount = static_cast<uint32>(data->nodes.size());
            for (uint32 j = 0; j < entitiesCount; ++j)
            {
                Entity* sourceEntity = data->nodes[j];
                if (uniqueLodEntities.end() != uniqueLodEntities.find(sourceEntity))
                {
                    Entity* cloned = sourceEntity->Clone();
                    sourceEntity->GetParent()->AddNode(cloned);
                    data->nodes.pop_back();
                    data->nodes.push_back(cloned);
                }
                else
                {
                    uniqueLodEntities.insert(sourceEntity);
                }
            }
        }

        for (uint32 i = 0; i < size; ++i)
        {
            LodComponent::LodData* data = lodData[i];
            uint32 entitiesCount = static_cast<uint32>(data->nodes.size());
            for (uint32 j = 0; j < entitiesCount; ++j)
            {
                emptyEntities++;
                Entity* sourceEntity = data->nodes[j];
                TransformComponent* sourceTransform = GetTransformComponent(sourceEntity);
                RenderObject* sourceRenderObject = GetRenderObject(sourceEntity);

                Vector<std::pair<Entity*, RenderObject*>> sourceRenderObjects;
                if (nullptr != sourceRenderObject)
                {
                    sourceRenderObjects.push_back(std::make_pair(sourceEntity, sourceRenderObject));
                    sourceRenderObject->Retain();
                }
                else
                {
                    FindAndEraseRenderObjectsRecursive(sourceEntity, sourceRenderObjects);
                }

                uint32 sourceRenderObjectsCount = static_cast<uint32>(sourceRenderObjects.size());
                for (uint32 n = 0; n < sourceRenderObjectsCount; ++n)
                {
                    sourceRenderObject = sourceRenderObjects[n].second;
                    if (sourceTransform->GetLocalTransform() != Matrix4::IDENTITY)
                    {
                        PolygonGroup* pg = sourceRenderObject->GetRenderBatchCount() > 0 ? sourceRenderObject->GetRenderBatch(0)->GetPolygonGroup() : 0;
                        if (nullptr != pg && bakedPolygonGroups.end() == bakedPolygonGroups.find(pg))
                        {
                            Matrix4 totalTransform = sourceRenderObjects[n].first->AccamulateTransformUptoFarParent(entity);
                            sourceRenderObject->BakeGeometry(totalTransform);
                            bakedPolygonGroups.insert(pg);
                        }
                    }

                    uint32 sourceRenderBatchCount = sourceRenderObject->GetRenderBatchCount();
                    while (0 != sourceRenderBatchCount)
                    {
                        RenderBatch* sourceRenderBatch = sourceRenderObject->GetRenderBatch(0);
                        sourceRenderBatch->Retain();
                        sourceRenderObject->RemoveRenderBatch(sourceRenderBatch);
                        ro->AddRenderBatch(sourceRenderBatch, data->layer, -1);
                        sourceRenderBatch->Release();
                        sourceRenderBatchCount--;
                    }
                    sourceRenderObject->Release();
                }

                sourceEntity->RemoveComponent(Component::RENDER_COMPONENT);
                if (sourceEntity->GetChildrenCount() == 0)
                {
                    entitiesToRemove.insert(sourceEntity);
                }

                //remove!!!
                data->nodes.clear();
            }
        }
    }

    Set<Entity*>::iterator itEnd = entitiesToRemove.end();
    for (Set<Entity*>::iterator it = entitiesToRemove.begin(); it != itEnd; ++it)
    {
        (*it)->GetParent()->RemoveNode(*it);
    }

    return res;
}

void LodToLod2Converter::FindAndEraseRenderObjectsRecursive(Entity* fromEntity, Vector<std::pair<Entity*, RenderObject*>>& entitiesAndRenderObjects)
{
    RenderObject* ro = GetRenderObject(fromEntity);
    if (nullptr != ro && ro->GetType() == RenderObject::TYPE_MESH)
    {
        ro->Retain();
        entitiesAndRenderObjects.push_back(std::make_pair(fromEntity, ro));
        fromEntity->RemoveComponent(Component::RENDER_COMPONENT);
    }

    int32 size = fromEntity->GetChildrenCount();
    for (int32 i = 0; i < size; ++i)
    {
        Entity* child = fromEntity->GetChild(i);
        FindAndEraseRenderObjectsRecursive(child, entitiesAndRenderObjects);
    }
}
};