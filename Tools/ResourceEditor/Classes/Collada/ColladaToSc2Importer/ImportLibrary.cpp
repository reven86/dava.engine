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

#include "Collada/ColladaToSc2Importer/ImportSettings.h"
#include "Collada/ColladaToSc2Importer/ImportLibrary.h"

namespace DAVA
{

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

void ImportLibrary::InitPolygon(PolygonGroup * davaPolygon, uint32 vertexFormat, Vector<ColladaVertex> & vertices)
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
    if (!isShadow && hasTexture)
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
    
bool ImportLibrary::GetTextureTypeAndPathFromCollada(ColladaMaterial * material, FastName & type, FilePath & path)
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

FilePath ImportLibrary::GetNormalMapTexturePath(const FilePath & originalTexturePath)
{
    FilePath path = originalTexturePath;
    path.ReplaceBasename(path.GetBasename() + ImportSettings::normalMapPattern);
    return path;
}
  

} // DAVA namespace