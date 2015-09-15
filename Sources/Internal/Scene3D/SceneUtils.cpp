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

void CombineLods(Scene * scene)
{
    for (auto child : scene->children)
    {
        CombineEntityLods(child);
    }
}

String LodNameForIndex(const String & pattern, uint32 lodIndex)
{
    return Format(pattern.c_str(), lodIndex);
}

void CombineEntityLods(Entity * forRootNode)
{
    const String lodNamePattern("_lod%d");
    const String dummyLodNamePattern("_lod%ddummy");
    
    List<Entity *> lodNodes;
    
    // try to find nodes which have lodNamePattrn.
    const String lod0 = LodNameForIndex(lodNamePattern, 0);
    if (!forRootNode->FindNodesByNamePart(lod0, lodNodes))
    {
        // There is no lods.
        return;
    }
    
    // ok. We have some nodes with lod 0 in the name. Try to find other lods for same name.
    for (Entity * oneLodNode : lodNodes)
    {
        // node name which contains lods
        const String lodName(oneLodNode->GetName().c_str());
        const String nodeWithLodsName(lodName, 0, lodName.find(lod0));
        
        Entity * oldParent = oneLodNode->GetParent();
        
        ScopedPtr<Entity> newNodeWithLods(new Entity());
        newNodeWithLods->SetName(nodeWithLodsName.c_str());
        
        ScopedPtr<Mesh> newMesh(new Mesh());
        
        uint32 lodCount = 0;
        for (int lodNo = 0; lodNo < LodComponent::MAX_LOD_LAYERS; ++lodNo)
        {
            
            // Remove dummy nodes
            // Try to find node with same name but with other lod
            const FastName lodIName(nodeWithLodsName + LodNameForIndex(lodNamePattern, lodNo));
            Entity * ln = oldParent->FindByName(lodIName.c_str());
            
            if (nullptr == ln)
            {
                const FastName dummyLodName(nodeWithLodsName + LodNameForIndex(dummyLodNamePattern, lodNo));
                ln = oldParent->FindByName(dummyLodName.c_str());
                
                if (nullptr != ln)
                {
                    ln->SetVisible(false);
                    ln->RemoveAllChildren();
                }
            }
            
            if (nullptr != ln)
            {
                CollapseRenderBatchesRecursiveAsLod(ln, lodNo, newMesh);
                CollapseAnimationsUpToFarParent(ln, newNodeWithLods);
                
                oldParent->RemoveNode(ln);
                ++lodCount;
            }
            
        }
        
        if (0 < lodCount)
        {
            LodComponent *lc = new LodComponent();
            newNodeWithLods->AddComponent(lc);
            if (lodCount < LodComponent::MAX_LOD_LAYERS && lodCount > LodComponent::INVALID_LOD_LAYER)
            {
                // Fix max lod distance for max used lod index
                lc->SetLodLayerDistance(lodCount, LodComponent::MAX_LOD_DISTANCE);
            }
        }
        
        RenderComponent * rc = new RenderComponent();
        rc->SetRenderObject(newMesh);
        
        newNodeWithLods->AddComponent(rc);
        oldParent->AddNode(newNodeWithLods);
        
        DVASSERT(oldParent->GetScene());
        DVASSERT(newNodeWithLods->GetScene());
    }
}


void BakeTransformsUpToFarParent(Entity * parent, Entity * currentNode)
{
    for (auto child : currentNode->children)
    {
        BakeTransformsUpToFarParent(parent, child);
    }
    
    // Bake transforms to geometry
    RenderObject * ro = GetRenderObject(currentNode);
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

void CollapseRenderBatchesRecursiveAsLod(Entity * node, uint32 lod, RenderObject * ro)
{
    for (auto child : node->children)
    {
        CollapseRenderBatchesRecursiveAsLod(child, lod, ro);
    }
    
    RenderObject * lodRenderObject = GetRenderObject(node);
    if (nullptr != lodRenderObject)
    {
        uint32 batchNo = 0;
        while (lodRenderObject->GetRenderBatchCount() > 0)
        {
            RenderBatch * batch = lodRenderObject->GetRenderBatch(batchNo);
            batch->Retain();
            lodRenderObject->RemoveRenderBatch(batch);
            ro->AddRenderBatch(batch, lod, -1);
            batch->Release();
            ++batchNo;
        }
    }
}

void CollapseAnimationsUpToFarParent(Entity * node, Entity * parent)
{
    for (auto child : parent->children)
    {
        CollapseAnimationsUpToFarParent(child, parent);
    }
    
    Component * ac = GetAnimationComponent(node);
    if (ac)
    {
        node->DetachComponent(ac);
        parent->AddComponent(ac);
    }
}

} //namespace SceneUtils
    
} //namespace DAVA