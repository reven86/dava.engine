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
#include "Scene3D/AnimationData.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Render/Material/NMaterial.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/Mesh.h"
#include "Render/3D/MeshUtils.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/TextureDescriptor.h"
#include "Collada/ColladaPolygonGroup.h"
#include "Collada/ColladaMeshInstance.h"
#include "Collada/ColladaSceneNode.h"
#include "Collada/ColladaScene.h"
#include "FileSystem/FileSystem.h"

#include "Collada/ColladaToSc2Importer/ColladaToSc2Importer.h"

namespace DAVA
{
    
namespace ImportSettings
{
    static const String lodNamePattern("_lod%d");
    static const String dummyLodNamePattern("_lod%ddummy");
    static const String shadowNamePattern("_shadow");
    static const String normalMapPattern("_NM");
    static const FastName shadowMaterialName("Shadow_Material");
}
    
namespace
{
void FlipTexCoords(Vector2 & v)
{
    v.y = 1.0f - v.y;
}

bool IsShadowNode(const String & nodeName)
{
    size_t fp = nodeName.find(ImportSettings::shadowNamePattern);
    return fp != String::npos;
}
} // unnamed namespace

class ImportLibrary
{
public:
    ~ImportLibrary();
    
    PolygonGroup * GetOrCreatePolygon(ColladaPolygonGroupInstance * colladaPGI);
    NMaterial * GetOrCreateMaterial(ColladaPolygonGroupInstance * colladaPolyGroupInst, const bool isShadow);
    NMaterial * GetOrCreateMaterialParent(ColladaMaterial * colladaMaterial, const bool isShadow);
    AnimationData * GetOrCreateAnimation(SceneNodeAnimation * colladaSceneNode);
    
private:
    Map<ColladaPolygonGroupInstance *, PolygonGroup *> polygons;
    Map<FastName, NMaterial *> materialParents;
    Map<FastName, NMaterial *> materials;
    Map<SceneNodeAnimation *, AnimationData *> animations;
};
    
ImportLibrary::~ImportLibrary()
{
    for (auto & pair : polygons)
    {
        uint32 refCount = pair.second->Release();
        DVASSERT(0 == refCount);
    }
    polygons.clear();
 
    for (auto & pair : materials)
    {
        uint32 refCount = pair.second->Release();
        DVASSERT(0 == refCount);
    }
    materials.clear();

    for (auto & pair : materialParents)
    {
        uint32 refCount = pair.second->Release();
        DVASSERT(0 == refCount);
    }
    materialParents.clear();
    
    for (auto & pair : animations)
    {
        uint32 refCount = pair.second->Release();
        DVASSERT(0 == refCount);
    }
    animations.clear();
}

void InitPolygon(PolygonGroup * davaPolygon, uint32 vertexFormat, Vector<ColladaVertex> &vertices)
{
    uint32 vertexCount = static_cast<uint32>(vertices.size());
    for (uint32 vertexNo = 0; vertexNo < vertexCount; ++vertexNo)
    {
        const auto & vertex = vertices[vertexNo];
        
        if (vertexFormat & EVF_VERTEX)
        {
            davaPolygon->SetCoord(vertexNo, vertex.position);
        }
        if (vertexFormat & EVF_NORMAL)
        {
            davaPolygon->SetNormal(vertexNo, vertex.normal);
        }
        if (vertexFormat & EVF_TANGENT)
        {
            davaPolygon->SetTangent(vertexNo, vertex.tangent);
        }
        if (vertexFormat & EVF_BINORMAL)
        {
            davaPolygon->SetBinormal(vertexNo, vertex.binormal);
        }
        if (vertexFormat & EVF_TEXCOORD0)
        {
            Vector2 coord = vertex.texCoords[0];
            FlipTexCoords(coord);
            davaPolygon->SetTexcoord(0, vertexNo, coord);
        }
        if (vertexFormat & EVF_TEXCOORD1)
        {
            Vector2 coord = vertex.texCoords[1];
            FlipTexCoords(coord);
            davaPolygon->SetTexcoord(1, vertexNo, coord);
        }
        if (vertexFormat & EVF_TEXCOORD2)
        {
            Vector2 coord = vertex.texCoords[2];
            FlipTexCoords(coord);
            davaPolygon->SetTexcoord(2, vertexNo, coord);
        }
        if (vertexFormat & EVF_TEXCOORD3)
        {
            Vector2 coord = vertex.texCoords[3];
            FlipTexCoords(coord);
            davaPolygon->SetTexcoord(3, vertexNo, coord);
        }
    }
}

PolygonGroup * ImportLibrary::GetOrCreatePolygon(ColladaPolygonGroupInstance * colladaPGI)
{
    // Try to take polygon from library
    PolygonGroup * davaPolygon = polygons[colladaPGI];
    
    // there is no polygon, so create new one
    if (nullptr == davaPolygon)
    {
        davaPolygon = new PolygonGroup();
        
        ColladaPolygonGroup *colladaPolygon = colladaPGI->polyGroup;
        DVASSERT(nullptr != colladaPolygon && "Empty collada polyton group instance.");

        auto vertices = colladaPolygon->GetVertices();
        uint32 vertexCount = static_cast<uint32>(vertices.size());
        auto vertexFormat = colladaPolygon->GetVertexFormat();
        auto indecies = colladaPolygon->GetIndices();
        uint32 indexCount = static_cast<uint32>(indecies.size());

        // Allocate data buffers before fill them
        davaPolygon->AllocateData(vertexFormat, vertexCount, indexCount);
        davaPolygon->triangleCount = colladaPolygon->GetTriangleCount();
        
        // Fill index array
        for(uint32 indexNo = 0; indexNo < indexCount; ++indexNo)
        {
            davaPolygon->indexArray[indexNo] = indecies[indexNo];
        }
        
        // Take collada vertices and set to polygon group
        InitPolygon(davaPolygon, vertexFormat, vertices);

        bool rebuildTangentSpace = false;
#ifdef REBUILD_TANGENT_SPACE_ON_IMPORT
        rebuildTangentSpace = true;
#endif
        const int32 prerequiredFormat = EVF_TANGENT | EVF_BINORMAL | EVF_NORMAL;
        if (rebuildTangentSpace && (davaPolygon->GetFormat() & prerequiredFormat) == prerequiredFormat)
        {
            MeshUtils::RebuildMeshTangentSpace(davaPolygon, true);
        }
        else
        {
            davaPolygon->BuildBuffers();
        }
        
        // Put polygon to the library
        polygons[colladaPGI] = davaPolygon;
    }
    
    // TO VERIFY: polygon
        
    return davaPolygon;
}

AnimationData * ImportLibrary::GetOrCreateAnimation(SceneNodeAnimation * colladaAnimation)
{
    AnimationData * animation = animations[colladaAnimation];
    if (nullptr == animation)
    {
        animation = new AnimationData();
        
        animation->SetInvPose(colladaAnimation->invPose);
        animation->SetDuration(colladaAnimation->duration);
        if (nullptr != colladaAnimation->keys)
        {
            for (uint32 keyNo = 0; keyNo < colladaAnimation->keyCount; ++keyNo)
            {
                SceneNodeAnimationKey key = colladaAnimation->keys[keyNo];
                animation->AddKey(key);
            }
        }
        
        animations[colladaAnimation] = animation;
    }
    
    return animation;
}

namespace
{
bool GetTextureTypeAndPathFromCollada(ColladaMaterial * material, FastName & type, FilePath & path)
{
    ColladaTexture * diffuse = material->diffuseTexture;
    bool useDiffuseTexture = nullptr != diffuse && material->hasDiffuseTexture;
    if (useDiffuseTexture)
    {
        type = NMaterial::TEXTURE_ALBEDO;
        path = diffuse->texturePathName.c_str();
        return true;
    }
    return false;
}
    
FilePath GetNormalMapTexturePath(const FilePath & originalTexturePath)
{
    FilePath path = originalTexturePath;
    path.ReplaceBasename(path.GetBasename() + ImportSettings::normalMapPattern);
    return path;
}

} // unnamed namespace

NMaterial * ImportLibrary::GetOrCreateMaterialParent(ColladaMaterial * colladaMaterial, const bool isShadow)
{
    FastName parentMaterialTemplate;
    FastName parentMaterialName;

    if (isShadow)
    {
        parentMaterialName = ImportSettings::shadowMaterialName;
        parentMaterialTemplate = NMaterialName::SHADOW_VOLUME;
    }
    else
    {
        parentMaterialName = FastName(colladaMaterial->material->GetDaeId().c_str());
        parentMaterialTemplate = NMaterialName::TEXTURED_OPAQUE;
    }
    
    NMaterial * davaMaterialParent = materialParents[parentMaterialName];
    if (nullptr == davaMaterialParent)
    {
        davaMaterialParent = NMaterial::CreateMaterial(parentMaterialName, parentMaterialTemplate, NMaterial::DEFAULT_QUALITY_NAME);
        materialParents[parentMaterialName] = davaMaterialParent;
    }
    
    FastName textureType;
    FilePath texturePath;
    bool hasTexture = GetTextureTypeAndPathFromCollada(colladaMaterial, textureType, texturePath);
    if (hasTexture)
    {
        FilePath descriptorPathname = TextureDescriptor::GetDescriptorPathname(texturePath);
        
        TextureDescriptor * descr = TextureDescriptor::CreateFromFile(descriptorPathname);
        if (nullptr != descr)
        {
            descr->Save();
            texturePath = descr->pathname;
            SafeDelete(descr);
        }
        davaMaterialParent->SetTexture(textureType, descriptorPathname);
    
        FilePath normalMap = GetNormalMapTexturePath(descriptorPathname);
        if (FileSystem::Instance()->IsFile(normalMap))
        {
            davaMaterialParent->SetTexture(NMaterial::TEXTURE_NORMAL, normalMap);
        }
    }
    return davaMaterialParent;
}
    
NMaterial * ImportLibrary::GetOrCreateMaterial(ColladaPolygonGroupInstance * colladaPolyGroupInst, const bool isShadow)
{
    ColladaMaterial * colladaMaterial = colladaPolyGroupInst->material;
    DVASSERT(nullptr != colladaMaterial && "Empty material");

    NMaterial * davaMaterialParent = GetOrCreateMaterialParent(colladaMaterial, isShadow);

    // Use daeId + parentMaterialName to make unique key for material in the library
    String daeId = colladaMaterial->material->GetDaeId().c_str();
    String parentMaterialName(davaMaterialParent->GetMaterialName().c_str());
    FastName materialKey = FastName(daeId + parentMaterialName.c_str());

    // Try to get material from library
    NMaterial * material = materials[materialKey];
    
    // There is no material in the library, so create new one
    if (nullptr == material)
    {
        material = NMaterial::CreateMaterialInstance();
        material->SetMaterialName(FastName(daeId));
        material->SetParent(davaMaterialParent);

        materials[materialKey] = material;
    }
    
    return material;
}

namespace
{
void CollapseRenderBatchesRecursive(Entity * node, uint32 lod, RenderObject * ro)
{
    for (auto child : node->children)
    {
        CollapseRenderBatchesRecursive(child, lod, ro);
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

void CollapseAnimations(Entity * node, Entity * parent)
{
    for (auto child : parent->children)
    {
        CollapseAnimations(child, parent);
    }
    
    AnimationComponent * ac = GetAnimationComponent(node);
    if (ac)
    {
        node->DetachComponent(ac);
        parent->AddComponent(ac);
    }

}
    
String LodNameForIndex(const String & pattern, uint32 lodIndex)
{
    return Format(pattern.c_str(), lodIndex);
}

void CollapseLodsIntoOneEntity(Entity *forRootNode)
{
    List<Entity *> lodNodes;
    
    const String lod0 = LodNameForIndex(ImportSettings::lodNamePattern, 0);
    if (!forRootNode->FindNodesByNamePart(lod0, lodNodes))
    {
        // There is no lods.
        return;
    }
    
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
        for (int i = 0; i < LodComponent::MAX_LOD_LAYERS; ++i)
        {
            
            // Remove dummy nodes
            // Try to find node with same name but with other lod
            const FastName lodIName(nodeWithLodsName + LodNameForIndex(ImportSettings::lodNamePattern, i));
            Entity * ln = oldParent->FindByName(lodIName.c_str());
            
            if (nullptr == ln)
            {
                const FastName dummyLodName(nodeWithLodsName + LodNameForIndex(ImportSettings::dummyLodNamePattern, i));
                ln = oldParent->FindByName(dummyLodName.c_str());
                
                if (nullptr != ln)
                {
                    ln->SetVisible(false);
                    ln->RemoveAllChildren();
                }
            }
            
            if (nullptr != ln)
            {
                CollapseRenderBatchesRecursive(ln, i, newMesh);
                CollapseAnimations(ln, newNodeWithLods);
                
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
    
void CombineLods(Entity * root)
{
    for (auto child : root->children)
    {
        CollapseLodsIntoOneEntity(child);
    }
}

void BakeTransformsUpToParent(Entity * parent, Entity * currentNode)
{
    for (auto child : currentNode->children)
    {
        BakeTransformsUpToParent(parent, child);
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
} // unnamed namespace

// Creates Dava::Mesh from ColladaMeshInstance and puts it
Mesh * ColladaToSc2Importer::GetMeshFromCollada(ColladaMeshInstance * mesh, const bool isShadow)
{
    Mesh * davaMesh = new Mesh();
    for (auto polygonGroupInstance : mesh->polyGroupInstances)
    {
        PolygonGroup * davaPolygon = library->GetOrCreatePolygon(polygonGroupInstance);

        if (isShadow)
        {
            davaPolygon = DAVA::MeshUtils::CreateShadowPolygonGroup(davaPolygon);
        }

        NMaterial * davaMaterial = library->GetOrCreateMaterial(polygonGroupInstance, isShadow);
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
        bool isShadowNode = IsShadowNode(node->GetName().c_str());
        
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
        AnimationData * animation = library->GetOrCreateAnimation(colladaNode->animation);
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
        NMaterial * globalMaterial = library->GetOrCreateMaterialParent(cmaterial, false);
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
            AnimationData * animation = library->GetOrCreateAnimation(colladaAnimation);
            DVASSERT(nullptr != animation);
        }
    }
}

ColladaToSc2Importer::ColladaToSc2Importer()
{
    library = new ImportLibrary();
}

ColladaToSc2Importer::~ColladaToSc2Importer()
{
    SafeDelete(library);
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
    BakeTransformsUpToParent(scene, scene);
    
    // post process Entities and create Lod nodes.
    CombineLods(scene);
    
    return scene->SaveScene(scenePath + sceneName);
}

};