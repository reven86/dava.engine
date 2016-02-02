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

#include "Qt/Main/QtUtils.h"

namespace DAVA
{
// Creates Dava::Mesh from ColladaMeshInstance and puts it
Mesh* ColladaToSc2Importer::GetMeshFromCollada(ColladaMeshInstance* mesh, const bool isShadow)
{
    Mesh* davaMesh = new Mesh();
    for (auto polygonGroupInstance : mesh->polyGroupInstances)
    {
        PolygonGroup* davaPolygon = library.GetOrCreatePolygon(polygonGroupInstance);

        if (isShadow)
        {
            davaPolygon = DAVA::MeshUtils::CreateShadowPolygonGroup(davaPolygon);
        }

        ScopedPtr<NMaterial> davaMaterial(library.CreateMaterialInstance(polygonGroupInstance, isShadow));
        davaMesh->AddPolygonGroup(davaPolygon, davaMaterial);
    }
    // TO VERIFY?
    DVASSERT(0 < davaMesh->GetPolygonGroupCount() && "Empty mesh");
    return davaMesh;
}

eColladaErrorCodes ColladaToSc2Importer::VerifyDavaMesh(RenderObject* mesh, const FastName name)
{
    eColladaErrorCodes retValue = eColladaErrorCodes::COLLADA_OK;

    uint32 batchesCount = mesh->GetRenderBatchCount();
    if (0 == batchesCount)
    {
        ReportError(Format("[DAE to SC2] %s has no render batches.", name.c_str()));
        retValue = eColladaErrorCodes::COLLADA_ERROR;
    }
    else
    {
        for (uint32 i = 0; i < batchesCount; ++i)
        {
            auto batch = mesh->GetRenderBatch(i);
            if (nullptr == batch)
            {
                ReportError(Format("[DAE to SC2] Node %s has no %i render batch", i));
                retValue = eColladaErrorCodes::COLLADA_ERROR;
            }

            auto polygon = batch->GetPolygonGroup();
            if (nullptr == polygon)
            {
                ReportError(Format("[DAE to SC2] Node %s has no polygon in render batch %i ", i));
                retValue = eColladaErrorCodes::COLLADA_ERROR;
            }

            if (0 >= polygon->GetVertexCount())
            {
                ReportError(Format("[DAE to SC2] Node %s has no geometric data", name.c_str()));
                retValue = eColladaErrorCodes::COLLADA_ERROR;
            }
        }
    }

    return retValue;
}

eColladaErrorCodes ColladaToSc2Importer::ImportMeshes(const Vector<ColladaMeshInstance*>& meshInstances, Entity* node)
{
    eColladaErrorCodes retValue = eColladaErrorCodes::COLLADA_OK;

    DVASSERT(1 >= meshInstances.size() && "Should be only one meshInstance in one collada node");
    for (auto meshInstance : meshInstances)
    {
        bool isShadowNode = String::npos != node->GetName().find(ImportSettings::shadowNamePattern);

        ScopedPtr<RenderObject> davaMesh(GetMeshFromCollada(meshInstance, isShadowNode));
        RenderComponent* davaRenderComponent = GetRenderComponent(node);
        if (nullptr == davaRenderComponent)
        {
            davaRenderComponent = new RenderComponent();
            node->AddComponent(davaRenderComponent);
        }
        davaRenderComponent->SetRenderObject(davaMesh);

        // Verification!
        eColladaErrorCodes iterationRet = VerifyDavaMesh(davaMesh.get(), node->GetName());
        retValue = Max(iterationRet, retValue);
    }

    return retValue;
}

void ColladaToSc2Importer::ImportAnimation(ColladaSceneNode* colladaNode, Entity* nodeEntity)
{
    if (nullptr != colladaNode->animation)
    {
        auto* animationComponent = new AnimationComponent();
        animationComponent->SetEntity(nodeEntity);
        nodeEntity->AddComponent(animationComponent);

        // Calculate actual transform and bake it into animation keys.
        // NOTE: for now usage of the same animation more than once is bad idea
        AnimationData* animation = library.GetOrCreateAnimation(colladaNode->animation);
        Matrix4 totalTransform = colladaNode->AccumulateTransformUptoFarParent(colladaNode->scene->rootNode);
        animation->BakeTransform(totalTransform);
        animationComponent->SetAnimation(animation);
    }
}

eColladaErrorCodes ColladaToSc2Importer::BuildSceneAsCollada(Entity* root, ColladaSceneNode* colladaNode)
{
    eColladaErrorCodes res = eColladaErrorCodes::COLLADA_OK;

    ScopedPtr<Entity> nodeEntity(new Entity());

    String name(colladaNode->originalNode->GetName());
    if (name.empty())
    {
        name = Format("UNNAMED");

        res = eColladaErrorCodes::COLLADA_ERROR;
        ReportError(Format("[DAE to SC2] Unnamed node found as a child of %s", root->GetName().c_str()));
        if (0 < colladaNode->childs.size())
        {
            ReportError(Format("[DAE to SC2] It's childs:"));
            for (auto child : colladaNode->childs)
            {
                ReportError(Format("[DAE to SC2] %s", child->originalNode->GetName().c_str()));
            }
        }
    }

    nodeEntity->SetName(FastName(name));

    // take mesh from node and put it into entity's render component
    const eColladaErrorCodes importMeshesResult = ImportMeshes(colladaNode->meshInstances, nodeEntity);
    res = Max(importMeshesResult, res);

    // Import animation
    ImportAnimation(colladaNode, nodeEntity);

    auto* transformComponent = GetTransformComponent(nodeEntity);
    transformComponent->SetLocalTransform(&colladaNode->localTransform);

    root->AddNode(nodeEntity);

    for (auto childNode : colladaNode->childs)
    {
        const eColladaErrorCodes childRes = BuildSceneAsCollada(nodeEntity, childNode);
        res = Max(res, childRes);
    }

    return res;
}

void ColladaToSc2Importer::LoadMaterialParents(ColladaScene* colladaScene)
{
    for (auto cmaterial : colladaScene->colladaMaterials)
    {
        NMaterial* globalMaterial = library.GetOrCreateMaterialParent(cmaterial, false);
        DVASSERT(nullptr != globalMaterial);
    }
}

void ColladaToSc2Importer::LoadAnimations(ColladaScene* colladaScene)
{
    for (auto canimation : colladaScene->colladaAnimations)
    {
        for (auto& pair : canimation->animations)
        {
            SceneNodeAnimation* colladaAnimation = pair.second;
            AnimationData* animation = library.GetOrCreateAnimation(colladaAnimation);
            DVASSERT(nullptr != animation);
        }
    }
}

eColladaErrorCodes ColladaToSc2Importer::SaveSC2(ColladaScene* colladaScene, const FilePath& scenePath)
{
    ScopedPtr<Scene> scene(new Scene());

    // Load scene global materials.
    LoadMaterialParents(colladaScene);

    // Load scene global animations
    LoadAnimations(colladaScene);

    // Iterate recursive over collada scene and build Dava Scene with same ierarchy

    eColladaErrorCodes convertRes = BuildSceneAsCollada(scene, colladaScene->rootNode);
    if (eColladaErrorCodes::COLLADA_OK == convertRes)
    {
        // Apply transforms to render batches and use identity local transforms
        SceneUtils::BakeTransformsUpToFarParent(scene, scene);

        // post process Entities and create Lod nodes.
        SceneUtils::CombineLods(scene);

        SceneFileV2::eError saveRes = scene->SaveScene(scenePath);

        if (saveRes > SceneFileV2::eError::ERROR_NO_ERROR)
        {
            ReportError(Format("[DAE to SC2] Cannot save SC2. Error %d", saveRes));
            convertRes = eColladaErrorCodes::COLLADA_ERROR;
        }
    }

    if (0 < errorLogs.size())
    {
        ShowErrorDialog(errorLogs, "Conversion DAE to SC2 failed.");
    }

    return convertRes;
}

void ColladaToSc2Importer::ReportError(const String& errMessage)
{
    errorLogs.insert(errMessage);
    Logger::Error("%s", errMessage.c_str());
}
};