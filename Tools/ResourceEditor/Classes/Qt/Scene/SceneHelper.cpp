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


#include "SceneHelper.h"
#include "Deprecated/SceneValidator.h"

void SceneHelper::EnumerateSceneTextures(DAVA::Scene *forScene, DAVA::TexturesMap &textureCollection, TexturesEnumerateMode mode)
{
    EnumerateEntityTextures(forScene, forScene, textureCollection, mode);
}

void SceneHelper::BuildMaterialList(DAVA::Entity* forNode, Set<NMaterial*>& materialList, bool includeRuntime)
{
    if (nullptr == forNode)
        return;

    List<NMaterial*> materials;
    forNode->GetDataNodes(materials);

    for (auto& mat : materials)
    {
        if (!includeRuntime && mat->IsRuntime())
        {
            continue;
        }

        materialList.insert(mat);
    }
}

void SceneHelper::EnumerateEntityTextures(DAVA::Scene* forScene, DAVA::Entity* forNode, DAVA::TexturesMap& textureCollection, TexturesEnumerateMode mode)
{
    if (nullptr == forNode || nullptr == forScene)
    {
        return;
    }

    DAVA::Set<DAVA::NMaterial*> materials;
    BuildMaterialList(forNode, materials);

    Set<MaterialTextureInfo*> materialTextures;
    for (auto& mat : materials)
    {
        String materialName = mat->GetMaterialName().c_str();
        String parentName = mat->GetParent() ? mat->GetParent()->GetMaterialName().c_str() : String() ;
        
        if((parentName.find("Particle") != String::npos) || (materialName.find("Particle") != String::npos))
        {   //because particle materials has textures only after first start, so we have different result during scene life.
            continue;
        }

        mat->CollectLocalTextures(materialTextures);
    }

    for (auto const& matTex : materialTextures)
    {
        const DAVA::FilePath& texturePath = matTex->path;
        Texture* texture = matTex->texture;

        if (texturePath.IsEmpty() || !SceneValidator::Instance()->IsPathCorrectForProject(texturePath))
        {
            continue;
        }

        if ((TexturesEnumerateMode::EXCLUDE_NULL == mode) && (nullptr == texture || texture->isRenderTarget))
        {
            continue;
        }

        textureCollection[FILEPATH_MAP_KEY(texturePath)] = texture;
    }
}

int32 SceneHelper::EnumerateModifiedTextures(DAVA::Scene *forScene, DAVA::Map<DAVA::Texture *, DAVA::Vector< DAVA::eGPUFamily> > &textures)
{
	int32 retValue = 0;
	textures.clear();
	TexturesMap allTextures;
    EnumerateSceneTextures(forScene, allTextures, TexturesEnumerateMode::EXCLUDE_NULL);

    for (auto& it : allTextures)
    {
        DAVA::Texture* texture = it.second;
        if (nullptr == texture)
        {
            continue;
        }
		
		DAVA::TextureDescriptor *descriptor = texture->GetDescriptor();
        DVASSERT(descriptor);
        DVASSERT(descriptor->compression);

        DAVA::Vector<DAVA::eGPUFamily> markedGPUs;
        for(int i = 0; i < DAVA::GPU_DEVICE_COUNT; ++i)
		{
			eGPUFamily gpu = (eGPUFamily)i;
			if(GPUFamilyDescriptor::IsFormatSupported(gpu, (PixelFormat)descriptor->compression[gpu].format))
			{
				FilePath texPath = descriptor->GetSourceTexturePathname();
				if(texPath.Exists() && !descriptor->IsCompressedTextureActual(gpu))
				{
					markedGPUs.push_back(gpu);
					retValue++;
				}
			}
		}
		if(markedGPUs.size() > 0)
		{
			textures[texture] = markedGPUs;
		}
	}
	return retValue;
}

void SceneHelper::EnumerateMaterialInstances(DAVA::Entity *forNode, DAVA::Vector<DAVA::NMaterial *> &materials)
{
    uint32 childrenCount = forNode->GetChildrenCount();
    for(uint32 i = 0; i < childrenCount; ++i)
        EnumerateMaterialInstances(forNode->GetChild(i), materials);

    RenderObject * ro = GetRenderObject(forNode);
    if(!ro) return;
    
    uint32 batchCount = ro->GetRenderBatchCount();
    for(uint32 i = 0; i < batchCount; ++i)
        materials.push_back(ro->GetRenderBatch(i)->GetMaterial());

}

DAVA::Entity * SceneHelper::CloneEntityWithMaterials(DAVA::Entity *fromNode)
{
    Scene* scene = fromNode->GetScene();
    NMaterial* globalMaterial = (scene) ? scene->GetGlobalMaterial() : nullptr;

    Entity * newEntity = fromNode->Clone();

    Vector<NMaterial *> materialInstances;
    EnumerateMaterialInstances(newEntity, materialInstances);

    Set<NMaterial*> materialParentsSet;
    uint32 instancesCount = materialInstances.size();
    for (uint32 i = 0; i < instancesCount; ++i)
    {
        materialParentsSet.insert(materialInstances[i]->GetParent());
    }
    materialParentsSet.erase(globalMaterial);

    Map<NMaterial*, NMaterial*> clonedParents;
    for (auto& mp : materialParentsSet)
    {
        NMaterial* mat = mp ? mp->Clone() : nullptr;
        if (mat && mat->GetParent() == globalMaterial)
        {
            mat->SetParent(nullptr); //exclude material from scene
        }
        clonedParents[mp] = mat;
    }

    for(uint32 i = 0; i < instancesCount; ++i)
    {
        NMaterial* material = materialInstances[i];
        NMaterial* parent = material->GetParent();
        material->SetParent(clonedParents[parent]);
    }

    for (auto& cp : clonedParents)
    {
        SafeRelease(cp.second);
    }

    return newEntity;
}
