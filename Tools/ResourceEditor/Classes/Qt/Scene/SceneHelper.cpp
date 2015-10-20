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

#include "Scene3D/Systems/MaterialSystem.h"

void SceneHelper::EnumerateSceneTextures(DAVA::Scene *forScene, DAVA::TexturesMap &textureCollection, TexturesEnumerateMode mode)
{
    EnumerateEntityTextures(forScene, forScene, textureCollection, mode);
}

void SceneHelper::EnumerateEntityTextures(DAVA::Scene *forScene, DAVA::Entity *forNode, DAVA::TexturesMap &textureCollection, TexturesEnumerateMode mode)
{
    if(!forNode || !forScene) return;
    
    DAVA::MaterialSystem *matSystem = forScene->GetMaterialSystem();
    
    DAVA::Set<DAVA::NMaterial *> materials;
    matSystem->BuildMaterialList(forNode, materials);
    
    Set<NMaterial *>::const_iterator endIt = materials.end();
    for(Set<NMaterial *>::const_iterator it = materials.begin(); it != endIt; ++it)
    {
        DAVA::NMaterial *mat = *it;
        
        String materialName = mat->GetMaterialName().c_str();
        String parentName = mat->GetParent() ? mat->GetParent()->GetMaterialName().c_str() : String() ;
        
        if((parentName.find("Particle") != String::npos) || (materialName.find("Particle") != String::npos))
        {   //because particle materials has textures only after first start, so we have different result during scene life.
            continue;
        }
        
        CollectTextures(*it, textureCollection, mode);
    }
}

int32 SceneHelper::EnumerateModifiedTextures(DAVA::Scene *forScene, DAVA::Map<DAVA::Texture *, DAVA::Vector< DAVA::eGPUFamily> > &textures)
{
	int32 retValue = 0;
	textures.clear();
	TexturesMap allTextures;
	EnumerateSceneTextures(forScene, allTextures, EXCLUDE_NULL);
	for(TexturesMap::iterator it = allTextures.begin(); it != allTextures.end(); ++it)
	{
		DAVA::Texture * texture = it->second;
		if(NULL == texture)
		{
			continue;
		}
		
		DAVA::TextureDescriptor *descriptor = texture->GetDescriptor();
		if(NULL == descriptor)
		{
			continue;
		}

		DVASSERT(descriptor->compression);

		DAVA::Vector< DAVA::eGPUFamily> markedGPUs;
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

void SceneHelper::CollectTextures(const DAVA::NMaterial *material, DAVA::TexturesMap &textures, TexturesEnumerateMode mode)
{
    DAVA::uint32 texCount = material->GetTextureCount();
    for(DAVA::uint32 t = 0; t < texCount; ++t)
    {
        DAVA::FilePath texturePath = material->GetTexturePath(material->GetTextureName(t));
        if(!texturePath.IsEmpty() && SceneValidator::Instance()->IsPathCorrectForProject(texturePath)&&!NMaterial::IsRuntimeTexture(material->GetTextureName(t)))
        {
            if(mode == EXCLUDE_NULL)
            {
                DAVA::Texture *texture = material->GetTexture(t);
                if(texture && !texture->isRenderTarget)
                {
                    const DAVA::FilePath & path = texture->texDescriptor->pathname;

                    if(path != texturePath)
                    {
                        DAVA::Logger::Error("texture path: \"%s\"\n material (%s) path: \"%s\"\n", 
							path.GetAbsolutePathname().c_str(), material->GetMaterialName().c_str(),
							texturePath.GetAbsolutePathname().c_str());
                        DVASSERT(path == texturePath);
                    }

                    textures[FILEPATH_MAP_KEY(path)] = texture;
                }
            }
            else if(mode == INCLUDE_NULL)
            {
                textures[FILEPATH_MAP_KEY(texturePath)] = material->GetTexture(t);
            }
            else
            {
                DVASSERT(0 && "Unknown enumeration mode");
            }
        }
    }
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
    Entity * newEntity = fromNode->Clone();

    Vector<NMaterial *> materialInstances;
    EnumerateMaterialInstances(newEntity, materialInstances);

    Set<NMaterial *> materialParentsSet;
    uint32 instancesCount = materialInstances.size();
    for(uint32 i = 0; i < instancesCount; ++i)
        materialParentsSet.insert(materialInstances[i]->GetParent());

    Map<NMaterial *, NMaterial *> clonedParents;

    Set<NMaterial *>::const_iterator it = materialParentsSet.begin();
    Set<NMaterial *>::const_iterator itEnd = materialParentsSet.end();
    for(; it != itEnd; ++it)
        clonedParents[(*it)] = (*it)->Clone();

    for(uint32 i = 0; i < instancesCount; ++i)
    {
        NMaterial * material = materialInstances[i];
        NMaterial * parent = material->GetParent();
        material->SetParent(clonedParents[parent]);
    }

    return newEntity;
}