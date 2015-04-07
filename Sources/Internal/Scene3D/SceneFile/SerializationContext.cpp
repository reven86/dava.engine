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

#include "Scene3D/SceneFile/SerializationContext.h"
#include "Scene3D/DataNode.h"

#include "Scene3D/Scene.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Scene3D/Systems/MaterialSystem.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Render/Material/NMaterial.h"
#include "Render/Material.h"

#include "Utils/StringFormat.h"

#include "Render/Material/NMaterialNames.h"

namespace DAVA
{
	class MaterialNameMapper
	{
	public:

		static FastName MapName(Material* mat)
		{
			FastName name;
			
			switch(mat->type)
			{
				case Material::MATERIAL_UNLIT_TEXTURE:
				{
					if(mat->GetAlphablend())
                    {
                        name = NMaterialName::TEXTURED_ALPHABLEND;
                    }
                    else if(mat->GetAlphatest())
                    {
                        name = NMaterialName::TEXTURED_ALPHATEST;
                    }
                    else
                    {
                        name = NMaterialName::TEXTURED_OPAQUE;
                    }
                    
					break;
				}
					
				case Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP:
				{
					if(mat->GetAlphatest())
                    {
                        name = NMaterialName::TEXTURE_LIGHTMAP_ALPHATEST;
                    }
                    else
                    {
                        name = NMaterialName::TEXTURE_LIGHTMAP_OPAQUE;
                    }

					break;
				}
					
				case Material::MATERIAL_UNLIT_TEXTURE_DECAL:
				{
					if(mat->GetAlphablend())
                    {
                        name = NMaterialName::DECAL_ALPHABLEND;
                    }
                    else if(mat->GetAlphatest())
                    {
                        name = NMaterialName::DECAL_ALPHATEST;
                    }
					else
					{
						name = NMaterialName::DECAL_OPAQUE;
					}

					break;
				}
					
				case Material::MATERIAL_UNLIT_TEXTURE_DETAIL:
				{
					if(mat->GetAlphablend())
                    {
                        name = NMaterialName::DETAIL_ALPHABLEND;
                    }
                    else if(mat->GetAlphatest())
                    {
                        name = NMaterialName::DETAIL_ALPHATEST;
                    }
					else
					{
						name = NMaterialName::DETAIL_OPAQUE;
					}
                    
					break;
				}
					
				case Material::MATERIAL_VERTEX_LIT_TEXTURE:
				{
					if(mat->GetAlphatest())
                    {
                        name = NMaterialName::VERTEXLIT_ALPHATEST;
                    }
                    else
                    {
                        name = NMaterialName::VERTEXLIT_OPAQUE;
                    }
					break;
				}
					
				case Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE:
				{
					if(mat->GetAlphatest())
                    {
                        name = NMaterialName::PIXELLIT_ALPHATEST;
                    }
                    else
                    {
                        name = NMaterialName::PIXELLIT_OPAQUE;
                    }

					break;
				}
					
				case Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR:
				{
					if(mat->GetAlphatest())
                    {
                        name = NMaterialName::PIXELLIT_SPECULAR_ALPHATEST;
                    }
                    else
                    {
                        name = NMaterialName::PIXELLIT_SPECULAR_OPAQUE;
                    }

					break;
				}
					
				case Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP:
				{
					if(mat->GetAlphatest())
                    {
                        name = NMaterialName::PIXELLIT_SPECULARMAP_ALPHATEST;
                    }
                    else
                    {
                        name = NMaterialName::PIXELLIT_SPECULARMAP_OPAQUE;
                    }

					break;
				}
					
				case Material::MATERIAL_VERTEX_COLOR_ALPHABLENDED:
				{
					if(mat->GetAlphablend())
                    {
                        name = NMaterialName::VERTEXCOLOR_ALPHABLEND;
                    }
                    else
                    {
                        name = NMaterialName::VERTEXCOLOR_OPAQUE;
                    }

					break;
				}
					
				case Material::MATERIAL_VERTEX_COLOR_ALPHABLENDED_FRAME_BLEND:
				{
					if(mat->GetAlphablend())
                    {
                        name = NMaterialName::VERTEXCOLOR_FRAMEBLEND_ALPHABLEND;
                    }
                    else
                    {
                        name = NMaterialName::VERTEXCOLOR_FRAMEBLEND_OPAQUE;
                    }
					
					break;

				}
					
				case Material::MATERIAL_SPEED_TREE_LEAF:
				{
					name = NMaterialName::SPEEDTREE_LEAF;
					break;
				}
					
				case Material::MATERIAL_SKYBOX:
				{
					name = NMaterialName::SKYBOX;
					break;
				}
					
				default:
					break;
			};
			
			return name;
		}
        
	};

	SerializationContext::SerializationContext()
	{ }

	SerializationContext::~SerializationContext()
	{
		for(Map<uint64, DataNode*>::iterator it = dataBlocks.begin();
			it != dataBlocks.end();
			++it)
		{
			SafeRelease(it->second);
		}
		
		for(Map<uint64, NMaterial*>::iterator it = importedMaterials.begin();
			it != importedMaterials.end();
			++it)
		{
			SafeRelease(it->second);
		}

		DVASSERT(materialBindings.size() == 0 && "Serialization context destroyed without resolving material bindings!");
		materialBindings.clear();
	}
	
	void SerializationContext::ResolveMaterialBindings()
	{
		size_t instanceCount = materialBindings.size();
		for(size_t i = 0; i < instanceCount; ++i)
		{
			MaterialBinding& binding = materialBindings[i];
			NMaterial* parentMat = static_cast<NMaterial*>(GetDataBlock(binding.parentKey));
			
			DVASSERT(parentMat);
			if(parentMat)
			{
				binding.instanceMaterial->SetParent(parentMat);
			}
		}
		
		materialBindings.clear();
	}

	NMaterial* SerializationContext::ConvertOldMaterialToNewMaterial(Material* oldMaterial,
																	 InstanceMaterialState* oldMaterialState,
																	 uint64 oldMaterialId)
	{
#if RHI_COMPLETE
        if(!oldMaterial) return NULL;
        
		//VI: need to build the following material structure:
		//VI:     INSTANCE_WITH_COMMON_PROPS_AND_TEXTURES
		//VI:     (this instance has
		//VI:     same name as old material) ->
		//VI:                                  OBJECT_INSTANCE
		//VI:                                  (this instance is
		//VI:                                  assigned to object
		//VI:                                  and has specific
		//VI:                                  properties set)
		
		NMaterial* material = static_cast<NMaterial*>(GetImportedMaterial(oldMaterialId));
		if(NULL == material)
		{
			FastName newMaterialName = MaterialNameMapper::MapName(oldMaterial);
			DVASSERT(newMaterialName.IsValid());
			
			material = NMaterial::CreateMaterial(FastName(oldMaterial->GetName()),
													  newMaterialName,
													  GetDefaultMaterialQuality());
			
						
			if(oldMaterial->IsTextureShiftEnabled())
			{
				material->SetFlag(NMaterialFlagName::FLAG_TEXTURESHIFT, NMaterial::FlagOn);
			}
			
			if(oldMaterial->IsFlatColorEnabled() &&
               Material::MATERIAL_SKYBOX != oldMaterial->type)
			{
				material->SetFlag(NMaterialFlagName::FLAG_FLATCOLOR, NMaterial::FlagOn);
			}
			
			Material::eViewOptions viewOptions = oldMaterial->GetViewOption();
			switch(viewOptions)
			{
				case Material::MATERIAL_VIEW_TEXTURE_ONLY:
				{
					material->SetFlag(NMaterialFlagName::FLAG_TEXTUREONLY, NMaterial::FlagOn);
					break;
				}
				case Material::MATERIAL_VIEW_LIGHTMAP_ONLY:
				{
					material->SetFlag(NMaterialFlagName::FLAG_LIGHTMAPONLY, NMaterial::FlagOn);
					break;
				}

				default:
					break;
			}
			
			if(oldMaterial->GetSetupLightmap())
			{
				material->SetFlag(NMaterialFlagName::FLAG_SETUPLIGHTMAP, NMaterial::FlagOn);
			}
			
			if (Material::MATERIAL_UNLIT_TEXTURE_DECAL == oldMaterial->type)
			{
				Texture* tex = PrepareTexture(Texture::TEXTURE_2D, oldMaterial->GetTexture(Material::TEXTURE_DECAL));
				material->SetTexture(NMaterialTextureName::TEXTURE_DECAL, tex);
				
				if(tex->isPink)
				{
					SafeRelease(tex);
				}
			}
			else if(Material::MATERIAL_UNLIT_TEXTURE_DETAIL == oldMaterial->type)
			{
				Texture* tex = PrepareTexture(Texture::TEXTURE_2D, oldMaterial->GetTexture(Material::TEXTURE_DETAIL));
				material->SetTexture(NMaterialTextureName::TEXTURE_DETAIL, tex);
				
				if(tex->isPink)
				{
					SafeRelease(tex);
				}
			}
			
			if (Material::MATERIAL_FLAT_COLOR != oldMaterial->type &&
				Material::MATERIAL_SKYBOX != oldMaterial->type)
			{
				Texture* tex = PrepareTexture(Texture::TEXTURE_2D, oldMaterial->GetTexture(Material::TEXTURE_DIFFUSE));
				material->SetTexture(NMaterialTextureName::TEXTURE_ALBEDO, tex);
				
				if(tex->isPink)
				{
					SafeRelease(tex);
				}
			}
			
			if(Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE == oldMaterial->type ||
			   Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR == oldMaterial->type ||
			   Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP == oldMaterial->type)
			{
				Texture* tex = PrepareTexture(Texture::TEXTURE_2D, oldMaterial->GetTexture(Material::TEXTURE_NORMALMAP));
				material->SetTexture(NMaterialTextureName::TEXTURE_NORMAL, tex);
				
				if(tex->isPink)
				{
					SafeRelease(tex);
				}
			}
            
            if(Material::MATERIAL_SKYBOX == oldMaterial->type)
            {
                Texture* tex = PrepareTexture(Texture::TEXTURE_CUBE, oldMaterial->GetTexture(Material::TEXTURE_DIFFUSE));
                material->SetTexture(NMaterialTextureName::TEXTURE_CUBEMAP, tex);
                
                if(tex->isPink)
                {
                    SafeRelease(tex);
                }
            }

			if(oldMaterial->IsFlatColorEnabled() &&
               Material::MATERIAL_SKYBOX != oldMaterial->type)
			{
				material->SetPropertyValue(NMaterialParamName::PARAM_FLAT_COLOR, Shader::UT_FLOAT_VEC4, 1, &oldMaterialState->GetFlatColor());
			}
			
			if(oldMaterial->IsTextureShiftEnabled())
			{
				material->SetPropertyValue(NMaterialParamName::PARAM_TEXTURE0_SHIFT, Shader::UT_FLOAT_VEC2, 1, &oldMaterialState->GetTextureShift());
			}
			
			if(Material::MATERIAL_VERTEX_LIT_TEXTURE == oldMaterial->type ||
			   Material::MATERIAL_VERTEX_LIT_DETAIL == oldMaterial->type ||
			   Material::MATERIAL_VERTEX_LIT_DECAL == oldMaterial->type ||
			   Material::MATERIAL_VERTEX_LIT_LIGHTMAP == oldMaterial->type ||
			   Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE == oldMaterial->type ||
			   Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR == oldMaterial->type ||
			   Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP == oldMaterial->type)
			{
				float32 shininess = oldMaterial->GetShininess();
				material->SetPropertyValue(NMaterialParamName::PARAM_MATERIAL_SPECULAR_SHININESS, Shader::UT_FLOAT, 1, &shininess);

				Color ambientColor = oldMaterial->GetAmbientColor();
				Color diffuseColor = oldMaterial->GetDiffuseColor();
				Color specularColor = oldMaterial->GetSpecularColor();
				
				material->SetPropertyValue(NMaterialParamName::PARAM_PROP_AMBIENT_COLOR, Shader::UT_FLOAT_VEC4, 1, &ambientColor);
				material->SetPropertyValue(NMaterialParamName::PARAM_PROP_DIFFUSE_COLOR, Shader::UT_FLOAT_VEC4, 1, &diffuseColor);
				material->SetPropertyValue(NMaterialParamName::PARAM_PROP_SPECULAR_COLOR, Shader::UT_FLOAT_VEC4, 1, &specularColor);
			}
			else //VI: copy fog settings for static lit materials only! For tanks fog propeties will be set from scene
			{
				if(oldMaterial->IsFogEnabled())
				{
					Color fogColor = oldMaterial->GetFogColor();
					float32 fogDensity = oldMaterial->GetFogDensity();
					
					material->SetPropertyValue(NMaterialParamName::PARAM_FOG_COLOR, Shader::UT_FLOAT_VEC4, 1, &fogColor);
					material->SetPropertyValue(NMaterialParamName::PARAM_FOG_DENSITY, Shader::UT_FLOAT, 1, &fogDensity);
				}
			}
			
			//VI: material will be released by ~SerializationContext
			SetImportedMaterial(oldMaterialId, material);
		}
		
		NMaterial* instanceMaterial = NMaterial::CreateMaterialInstance();
		
		if(Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP == oldMaterial->type)
		{
			if(oldMaterialState)
			{
				instanceMaterial->GetIlluminationParams()->lightmapSize = oldMaterialState->GetLightmapSize();
			}
			
			Texture* tex = PrepareTexture(Texture::TEXTURE_2D, oldMaterialState ? oldMaterialState->GetLightmap() : NULL);
			instanceMaterial->SetTexture(NMaterialTextureName::TEXTURE_LIGHTMAP, tex);
			
			if(tex->isPink)
			{
				SafeRelease(tex);
			}
		}

        if(Material::MATERIAL_SPEED_TREE_LEAF == oldMaterial->type)
        {
            instanceMaterial->SetPropertyValue(FastName("treeLeafColorMul"), Shader::UT_FLOAT_VEC4, 1, &(oldMaterial->treeLeafColor));
            instanceMaterial->SetPropertyValue(FastName("treeLeafOcclusionMul"), Shader::UT_FLOAT, 1, &(oldMaterial->treeLeafOcclusionMul));
            instanceMaterial->SetPropertyValue(FastName("treeLeafOcclusionOffset"), Shader::UT_FLOAT, 1, &(oldMaterial->treeLeafOcclusionOffset));
        }
		if(oldMaterialState)
		{
			if(Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP == oldMaterial->type)
			{
				instanceMaterial->SetPropertyValue(NMaterialParamName::PARAM_UV_OFFSET, Shader::UT_FLOAT_VEC2, 1, &oldMaterialState->GetUVOffset());
				instanceMaterial->SetPropertyValue(NMaterialParamName::PARAM_UV_SCALE, Shader::UT_FLOAT_VEC2, 1, &oldMaterialState->GetUVScale());
			}
		}
		
		instanceMaterial->SetParent(material);
						
		return instanceMaterial;
#endif //RHI_COMPLETE
        return NULL;
	}
			
	Texture* SerializationContext::PrepareTexture(uint32 textureTypeHint,
												  Texture* tx)
	{
#if RHI_COMPLETE
		if(tx)
		{
			if(tx->isPink)
			{
				tx->Retain();
			}

			return tx;
		}

		return Texture::CreatePink((Texture::TextureType)textureTypeHint);
//		return (tx) ? tx : Texture::CreatePink((Texture::TextureType)textureTypeHint);
#else
        return tx;
#endif //RHI_COMPLETE

	}


void SerializationContext::AddLoadedPolygonGroup(PolygonGroup *group, uint32 dataFilePos)
{
    DVASSERT(loadedPolygonGroups.find(group)==loadedPolygonGroups.end());
    PolygonGroupLoadInfo loadInfo;
    loadInfo.filePos = dataFilePos;
    loadedPolygonGroups[group] = loadInfo;
}
void SerializationContext::AddRequestedPolygonGroupFormat(PolygonGroup *group, int32 format)
{
    DVASSERT(loadedPolygonGroups.find(group)!=loadedPolygonGroups.end());
    loadedPolygonGroups[group].requestedFormat|=format;
}

void SerializationContext::LoadPolygonGroupData(File *file)
{
    int32 prerequiredVertexFormat = QualitySettingsSystem::Instance()->GetPrerequiredVertexFormat();
    for (Map<PolygonGroup*, PolygonGroupLoadInfo>::iterator it = loadedPolygonGroups.begin(), e = loadedPolygonGroups.end(); it!=e; ++it)
    {
        file->Seek(it->second.filePos, File::SEEK_FROM_START);
        KeyedArchive * archive = new KeyedArchive();
        archive->Load(file);        
        it->first->LoadPolygonData(archive, this, it->second.requestedFormat | prerequiredVertexFormat);
        SafeRelease(archive);        
    }
}
}