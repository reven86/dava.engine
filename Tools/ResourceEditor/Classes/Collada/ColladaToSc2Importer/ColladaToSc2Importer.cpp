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

#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneUtils.h"
#include "Scene3D/AnimationData.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Render/Highlevel/Mesh.h"
#include "Render/3D/MeshUtils.h"
#include "Render/3D/PolygonGroup.h"
#include "Collada/ColladaMeshInstance.h"
#include "Collada/ColladaSceneNode.h"
#include "Collada/ColladaScene.h"

#include "Collada/ColladaToSc2Importer/ColladaToSc2Importer.h"

#include "Collada/ColladaToSc2Importer/ImportSettings.h"

namespace DAVA
{

// Creates Dava::Mesh from ColladaMeshInstance and puts it
Mesh * ColladaToSc2Importer::GetMeshFromCollada(ColladaMeshInstance * mesh, const bool isShadow)
{
    Mesh * davaMesh = new Mesh();
    for (auto polygonGroupInstance : mesh->polyGroupInstances)
    {
        PolygonGroup * davaPolygon = library.GetOrCreatePolygon(polygonGroupInstance);

        if (isShadow)
        {
            davaPolygon = DAVA::MeshUtils::CreateShadowPolygonGroup(davaPolygon);
        }

        NMaterial * davaMaterial = library.GetOrCreateMaterial(polygonGroupInstance, isShadow);
        davaMesh->AddPolygonGroup(davaPolygon, davaMaterial);
    }
    // TO VERIFY?
    DVASSERT(0 < davaMesh->GetPolygonGroupCount() && "Empty mesh");
    return davaMesh;
}

void ColladaToSc2Importer::ImportMeshes(const Vector<ColladaMeshInstance *> & meshInstances, Entity * node)
{
    DVASSERT(1 >= meshInstances.size() && "Should be only one meshInstance in one collada node");
    for (auto meshInstance : meshInstances)
    {
        bool isShadowNode = String::npos != node->GetName().find(ImportSettings::shadowNamePattern);
        
        ScopedPtr<RenderObject> davaMesh(GetMeshFromCollada(meshInstance, isShadowNode));
        RenderComponent * davaRenderComponent = GetRenderComponent(node);
        if (nullptr == davaRenderComponent)
        {
            davaRenderComponent = new RenderComponent();
            node->AddComponent(davaRenderComponent);
        }
        davaRenderComponent->SetRenderObject(davaMesh);
    }
}

void ColladaToSc2Importer::ImportAnimation(ColladaSceneNode * colladaNode, Entity * nodeEntity)
{
    if (nullptr != colladaNode->animation)
    {
        auto * animationComponent = new AnimationComponent();
        animationComponent->SetEntity(nodeEntity);
        nodeEntity->AddComponent(animationComponent);
        
        // Calculate actual transform and bake it into animation keys.
        // NOTE: for now usage of the same animation more than once is bad idea
        AnimationData * animation = library.GetOrCreateAnimation(colladaNode->animation);
        Matrix4 totalTransform = colladaNode->AccumulateTransformUptoFarParent(colladaNode->scene->rootNode);
        animation->BakeTransform(totalTransform);
        animationComponent->SetAnimation(animation);
    }
}

void ColladaToSc2Importer::BuildSceneAsCollada(Entity * root, ColladaSceneNode * colladaNode)
{
    String name(colladaNode->originalNode->GetName());
    if (name.empty())
    {
        static uint32 num = 0;
        name = Format("Node-%d", num++);
    }
    
    ScopedPtr<Entity> nodeEntity(new Entity());
    nodeEntity->SetName(FastName(name));
    
    // take mesh from node and put it into entity's render component
    ImportMeshes(colladaNode->meshInstances, nodeEntity);

    // Import animation
    ImportAnimation(colladaNode, nodeEntity);
    
    auto * transformComponent = GetTransformComponent(nodeEntity);
    transformComponent->SetLocalTransform(&colladaNode->localTransform);
    
    root->AddNode(nodeEntity);

    for (auto childNode : colladaNode->childs)
    {
        BuildSceneAsCollada(nodeEntity, childNode);
    }
}

void ColladaToSc2Importer::LoadMaterialParents(ColladaScene * colladaScene)
{
    for (auto cmaterial : colladaScene->colladaMaterials)
    {
        NMaterial * globalMaterial = library.GetOrCreateMaterialParent(cmaterial, false);
        DVASSERT(nullptr != globalMaterial);
    }
}
    
void ColladaToSc2Importer::LoadAnimations(ColladaScene * colladaScene)
{
    for (auto canimation : colladaScene->colladaAnimations)
    {
        for (auto & pair : canimation->animations)
        {
            SceneNodeAnimation * colladaAnimation = pair.second;
            AnimationData * animation = library.GetOrCreateAnimation(colladaAnimation);
            DVASSERT(nullptr != animation);
        }
    }
}
    
    
SceneFileV2::eError ColladaToSc2Importer::SaveSC2(ColladaScene * colladaScene, const FilePath & scenePath, const String & sceneName)
{
    ScopedPtr<Scene> scene(new Scene());

    // Load scene global materials.
    LoadMaterialParents(colladaScene);
    
    // Load scene global animations
    LoadAnimations(colladaScene);
    
    // Iterate recursive over collada scene and build Dava Scene with same ierarchy
    BuildSceneAsCollada(scene, colladaScene->rootNode);
    
    // Apply transforms to render batches and use identity local transforms
    SceneUtils::BakeTransformsUpToFarParent(scene, scene);
    
    // post process Entities and create Lod nodes.
    SceneUtils::CombineLods(scene);
    
    return scene->SaveScene(scenePath + sceneName);
}

};