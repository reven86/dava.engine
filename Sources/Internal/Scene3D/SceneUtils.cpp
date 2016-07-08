#include "Scene3D/SceneUtils.h"

#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/Mesh.h"
#include "Scene3D/Entity.h"
#include "Entity/Component.h"
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/RenderComponent.h"

namespace DAVA
{
namespace SceneUtils
{
void CombineLods(Scene* scene)
{
    for (auto child : scene->children)
    {
        CombineEntityLods(child);
    }
}

String LodNameForIndex(const String& pattern, uint32 lodIndex)
{
    return Format(pattern.c_str(), lodIndex);
}

void CombineEntityLods(Entity* forRootNode)
{
    const String lodNamePattern("_lod%d");
    const String dummyLodNamePattern("_lod%ddummy");

    List<Entity*> lodNodes;

    // try to find nodes which have lodNamePattrn.
    const String lod0 = LodNameForIndex(lodNamePattern, 0);
    if (!forRootNode->FindNodesByNamePart(lod0, lodNodes))
    {
        // There is no lods.
        return;
    }

    // ok. We have some nodes with lod 0 in the name. Try to find other lods for same name.
    for (Entity* oneLodNode : lodNodes)
    {
        // node name which contains lods
        const String lodName(oneLodNode->GetName().c_str());
        const String nodeWithLodsName(lodName, 0, lodName.find(lod0));

        Entity* oldParent = oneLodNode->GetParent();

        ScopedPtr<Entity> newNodeWithLods(new Entity());
        newNodeWithLods->SetName(nodeWithLodsName.c_str());

        ScopedPtr<Mesh> newMesh(new Mesh());

        uint32 lodCount = 0;
        for (int lodNo = 0; lodNo < LodComponent::MAX_LOD_LAYERS; ++lodNo)
        {
            // Try to find node with same name but with other lod
            const FastName lodIName(nodeWithLodsName + LodNameForIndex(lodNamePattern, lodNo));
            Entity* ln = oldParent->FindByName(lodIName.c_str());

            // Lod found. Move render batches from entity to NewMesh as lod.
            if (nullptr != ln)
            {
                CollapseRenderBatchesRecursiveAsLod(ln, lodNo, newMesh);
                CollapseAnimationsUpToFarParent(ln, newNodeWithLods);

                oldParent->RemoveNode(ln);

                ++lodCount;
            }

            // Try to find dummy lod node
            const FastName dummyLodName(nodeWithLodsName + LodNameForIndex(dummyLodNamePattern, lodNo));
            ln = oldParent->FindByName(dummyLodName.c_str());

            if (nullptr != ln)
            {
                // Remove dummy nodes
                ln->RemoveAllChildren();
                oldParent->RemoveNode(ln);
            }
        }

        if (0 < lodCount)
        {
            LodComponent* lc = new LodComponent();
            newNodeWithLods->AddComponent(lc);
            if (lodCount < LodComponent::MAX_LOD_LAYERS && lodCount > LodComponent::INVALID_LOD_LAYER)
            {
                // Fix max lod distance for max used lod index
                lc->SetLodLayerDistance(lodCount, LodComponent::MAX_LOD_DISTANCE);
            }
        }

        RenderComponent* rc = new RenderComponent();
        rc->SetRenderObject(newMesh);

        newNodeWithLods->AddComponent(rc);
        oldParent->AddNode(newNodeWithLods);

        DVASSERT(oldParent->GetScene());
        DVASSERT(newNodeWithLods->GetScene());
    }
}

void BakeTransformsUpToFarParent(Entity* parent, Entity* currentNode)
{
    for (auto child : currentNode->children)
    {
        BakeTransformsUpToFarParent(parent, child);
    }

    // Bake transforms to geometry
    RenderObject* ro = GetRenderObject(currentNode);
    if (ro)
    {
        // Get actual transformation for current entity
        Matrix4 totalTransform = currentNode->AccamulateTransformUptoFarParent(parent);
        ro->BakeGeometry(totalTransform);
    }

    // Set local transform as Ident because transform is already baked up into geometry
    auto transformComponent = GetTransformComponent(currentNode);
    transformComponent->SetLocalTransform(&Matrix4::IDENTITY);
}

void CollapseRenderBatchesRecursiveAsLod(Entity* node, uint32 lod, RenderObject* ro)
{
    for (auto child : node->children)
    {
        CollapseRenderBatchesRecursiveAsLod(child, lod, ro);
    }

    RenderObject* lodRenderObject = GetRenderObject(node);
    if (nullptr != lodRenderObject)
    {
        while (lodRenderObject->GetRenderBatchCount() > 0)
        {
            RenderBatch* batch = lodRenderObject->GetRenderBatch(0);
            batch->Retain();
            lodRenderObject->RemoveRenderBatch(batch);
            ro->AddRenderBatch(batch, lod, -1);
            batch->Release();
        };
    }
}

void CollapseAnimationsUpToFarParent(Entity* node, Entity* parent)
{
    for (auto child : parent->children)
    {
        CollapseAnimationsUpToFarParent(child, parent);
    }

    Component* ac = GetAnimationComponent(node);
    if (ac)
    {
        node->DetachComponent(ac);
        parent->AddComponent(ac);
    }
}

} //namespace SceneUtils

} //namespace DAVA