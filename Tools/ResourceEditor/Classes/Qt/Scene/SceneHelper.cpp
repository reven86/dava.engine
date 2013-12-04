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
#include "SceneEditor/SceneValidator.h"
#include "CubemapEditor/MaterialHelper.h"

using namespace DAVA;

void SceneHelper::EnumerateTextures(Entity *forNode, Map<String, Texture *> &textures)
{
	if(!forNode) return;

	Vector<Entity *> nodes;
	forNode->GetChildNodes(nodes);

	nodes.push_back(forNode);

	for(int32 n = 0; n < (int32)nodes.size(); ++n)
	{
		RenderObject *ro = GetRenderObject(nodes[n]);
		if(!ro) continue;

		uint32 count = ro->GetRenderBatchCount();
		for(uint32 b = 0; b < count; ++b)
		{
			RenderBatch *renderBatch = ro->GetRenderBatch(b);

			Material *material = renderBatch->GetMaterial();
			if(material)
			{
				for(int32 t = 0; t < Material::TEXTURE_COUNT; ++t)
				{
					CollectTexture(textures, material->GetTextureName((DAVA::Material::eTextureLevel)t).GetAbsolutePathname(), material->GetTexture((DAVA::Material::eTextureLevel)t));
				}
			}

			InstanceMaterialState *instanceMaterial = renderBatch->GetMaterialInstance();
			if(instanceMaterial)
			{
				CollectTexture(textures, instanceMaterial->GetLightmapName().GetAbsolutePathname(), instanceMaterial->GetLightmap());
			}
		}

		Landscape *land = dynamic_cast<Landscape *>(ro);
		if(land)
		{
			CollectLandscapeTextures(textures, land);
		}
	}
}

int32 SceneHelper::EnumerateModifiedTextures(DAVA::Entity *forNode, DAVA::Map<DAVA::Texture *, DAVA::Vector< DAVA::eGPUFamily> > &textures)
{
	int32 retValue = 0;
	textures.clear();
	Map<String, Texture *> allTextures;
	EnumerateTextures(forNode, allTextures);
	for(DAVA::Map<DAVA::String, DAVA::Texture *>::iterator it = allTextures.begin(); it != allTextures.end(); ++it)
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
				
		DAVA::Vector< DAVA::eGPUFamily> markedGPUs;
		for(int i = DAVA::GPU_UNKNOWN + 1; i < DAVA::GPU_FAMILY_COUNT; ++i)
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

void SceneHelper::CollectLandscapeTextures(DAVA::Map<DAVA::String, DAVA::Texture *> &textures, Landscape *forNode)
{
	for(int32 t = 0; t < Landscape::TEXTURE_COUNT; t++)
	{
		CollectTexture(textures, 
			forNode->GetTextureName((Landscape::eTextureLevel)t).GetAbsolutePathname(), 
			forNode->GetTexture((Landscape::eTextureLevel)t));
	}
}



void SceneHelper::CollectTexture(Map<String, Texture *> &textures, const String &name, Texture *tex)
{
	if(!name.empty() && SceneValidator::Instance()->IsPathCorrectForProject(name))
	{
		textures[name] = tex;
	}
}

void SceneHelper::EnumerateDescriptors(DAVA::Entity *forNode, DAVA::Set<DAVA::FilePath> &descriptors)
{
	if(!forNode)  return;

	Vector<Entity *> nodes;
	forNode->GetChildNodes(nodes);

	nodes.push_back(forNode);

	for(int32 n = 0; n < (int32)nodes.size(); ++n)
	{
		RenderComponent *rc = static_cast<RenderComponent *>(nodes[n]->GetComponent(Component::RENDER_COMPONENT));
		if(!rc) continue;

		RenderObject *ro = rc->GetRenderObject();
		if(!ro) continue;

		uint32 count = ro->GetRenderBatchCount();
		for(uint32 b = 0; b < count; ++b)
		{
			RenderBatch *renderBatch = ro->GetRenderBatch(b);

			Material *material = renderBatch->GetMaterial();
			if(material)
			{
				for(int32 t = 0; t < Material::TEXTURE_COUNT; ++t)
				{
					CollectDescriptors(descriptors, material->GetTextureName((DAVA::Material::eTextureLevel)t));
				}
			}

			InstanceMaterialState *instanceMaterial = renderBatch->GetMaterialInstance();
			if(instanceMaterial)
			{
				CollectDescriptors(descriptors, instanceMaterial->GetLightmapName());
			}
		}

		Landscape *land = dynamic_cast<Landscape *>(ro);
		if(land)
		{
			CollectLandscapeDescriptors(descriptors, land);
		}
	}
}

void SceneHelper::CollectLandscapeDescriptors(DAVA::Set<DAVA::FilePath> &descriptors, DAVA::Landscape *forNode)
{
	for(int32 t = 0; t < Landscape::TEXTURE_COUNT; t++)
	{
		CollectDescriptors(descriptors, forNode->GetTextureName((Landscape::eTextureLevel)t));
	}
}

void SceneHelper::CollectDescriptors(DAVA::Set<DAVA::FilePath> &descriptors, const DAVA::FilePath &pathname)
{
	if(pathname.GetType() == FilePath::PATH_EMPTY)
		return;

	DVASSERT(pathname.IsEqualToExtension(TextureDescriptor::GetDescriptorExtension()));

	if(!pathname.IsEmpty() && SceneValidator::Instance()->IsPathCorrectForProject(pathname))
	{
		descriptors.insert(pathname);
	}
}

void SceneHelper::EnumerateMaterials(DAVA::Entity *forNode, Vector<Material *> &materials)
{
	if(forNode)
	{
		forNode->GetDataNodes(materials);
		//VI: remove skybox materials so they not to appear in the lists
		MaterialHelper::FilterMaterialsByType(materials, DAVA::Material::MATERIAL_SKYBOX);
	}
}
