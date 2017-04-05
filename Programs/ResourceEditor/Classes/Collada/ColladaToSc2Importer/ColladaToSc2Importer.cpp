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
#include "Utils/UTF8Utils.h"
#include "Qt/Main/QtUtils.h"

namespace DAVA
{
ColladaToSc2Importer::ColladaToSc2Importer()
{
}

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

bool ColladaToSc2Importer::VerifyColladaMesh(ColladaMeshInstance* mesh, const FastName& nodeName)
{
    for (auto polygonGroupInstance : mesh->polyGroupInstances)
    {
        if (polygonGroupInstance->material == nullptr)
        {
            Logger::Error("[DAE to SC2] Node %s has no material", nodeName.c_str());
            return false;
        }

        auto polyGroup = polygonGroupInstance->polyGroup;
        if ((polyGroup == nullptr) || polyGroup->GetVertices().empty())
        {
            Logger::Error("[DAE to SC2] Node %s has no geometric data", nodeName.c_str());
            return false;
        }
    }

    return true;
}

eColladaErrorCodes ColladaToSc2Importer::VerifyDavaMesh(RenderObject* mesh, const FastName name)
{
    eColladaErrorCodes retValue = eColladaErrorCodes::COLLADA_OK;

    uint32 batchesCount = mesh->GetRenderBatchCount();
    if (0 == batchesCount)
    {
        Logger::Error("[DAE to SC2] %s has no render batches.", name.c_str());
        retValue = eColladaErrorCodes::COLLADA_ERROR;
    }
    else
    {
        for (uint32 i = 0; i < batchesCount; ++i)
        {
            auto batch = mesh->GetRenderBatch(i);
            if (nullptr == batch)
            {
                Logger::Error("[DAE to SC2] Node %s has no %i render batch", i);
                retValue = eColladaErrorCodes::COLLADA_ERROR;
            }

            auto polygon = batch->GetPolygonGroup();
            if (nullptr == polygon)
            {
                Logger::Error("[DAE to SC2] Node %s has no polygon in render batch %i ", i);
                retValue = eColladaErrorCodes::COLLADA_ERROR;
            }

            if (0 >= polygon->GetVertexCount())
            {
                Logger::Error("[DAE to SC2] Node %s has no geometric data", name.c_str());
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
        if (VerifyColladaMesh(meshInstance, node->GetName()))
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
        else
        {
            retValue = eColladaErrorCodes::COLLADA_ERROR;
        }
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
        // NOTE: animation->BakeTransform(totalTransform); actually was removed because of poor rotation behavior
        // when pose baked into keys. So just set correct invPose to animation instead.
        AnimationData* animation = library.GetOrCreateAnimation(colladaNode->animation);
        Matrix4 totalTransform = colladaNode->AccumulateTransformUptoFarParent(colladaNode->scene->rootNode);

        Matrix4 invPose;
        totalTransform.GetInverse(invPose);
        animation->SetInvPose(invPose);
        animationComponent->SetAnimation(animation);
    }
}

eColladaErrorCodes ColladaToSc2Importer::BuildSceneAsCollada(Entity* root, ColladaSceneNode* colladaNode)
{
    eColladaErrorCodes res = eColladaErrorCodes::COLLADA_OK;

    ScopedPtr<Entity> nodeEntity(new Entity());

    String name = UTF8Utils::MakeUTF8String(colladaNode->originalNode->GetName().c_str());
    if (name.empty())
    {
        name = Format("UNNAMED");

        res = eColladaErrorCodes::COLLADA_ERROR;
        Logger::Error("[DAE to SC2] Unnamed node found as a child of %s", root->GetName().c_str());
        if (0 < colladaNode->childs.size())
        {
            Logger::Error("[DAE to SC2] It's childs:");
            for (auto child : colladaNode->childs)
            {
                Logger::Error("[DAE to SC2] %s", child->originalNode->GetName().c_str());
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
        bool combinedSuccessfull = SceneUtils::CombineLods(scene);
        if (combinedSuccessfull)
        {
            SceneFileV2::eError saveRes = scene->SaveScene(scenePath);
            if (saveRes > SceneFileV2::eError::ERROR_NO_ERROR)
            {
                Logger::Error("[DAE to SC2] Cannot save SC2. Error %d", saveRes);
                convertRes = eColladaErrorCodes::COLLADA_ERROR;
            }
        }
        else
        {
            convertRes = eColladaErrorCodes::COLLADA_ERROR;
        }
    }

    return convertRes;
}
};
