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


#include "Scene3D/Systems/MaterialSystem.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Render/Material/NMaterial.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/RenderBase.h"
#include "FileSystem/YamlParser.h"
#include "Render/Shader.h"
#include "Render/Material/MaterialGraph.h"
#include "Render/Material/MaterialCompiler.h"
#include "Render/ShaderCache.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Light.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Utils/StringFormat.h"
#include "Render/Material/NMaterialTemplate.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/TextureDescriptor.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Material/NMaterialNames.h"

namespace DAVA
{

static const FastName DEFINE_VERTEX_LIT("VERTEX_LIT");
static const FastName DEFINE_PIXEL_LIT("PIXEL_LIT");
static const FastName DEFINE_LAND_SPECULAR("SPECULAR_LAND");

const FastName NMaterial::TEXTURE_ALBEDO("albedo");
const FastName NMaterial::TEXTURE_NORMAL("normalmap");
const FastName NMaterial::TEXTURE_DETAIL("detail");
const FastName NMaterial::TEXTURE_LIGHTMAP("lightmap");
const FastName NMaterial::TEXTURE_DECAL("decal");
const FastName NMaterial::TEXTURE_CUBEMAP("cubemap");
const FastName NMaterial::TEXTURE_HEIGHTMAP("heightmap");
const FastName NMaterial::TEXTURE_DECALMASK("decalmask");
const FastName NMaterial::TEXTURE_DECALTEXTURE("decaltexture");

const FastName NMaterial::TEXTURE_DYNAMIC_REFLECTION("dynamicReflection");
const FastName NMaterial::TEXTURE_DYNAMIC_REFRACTION("dynamicRefraction");

const FastName NMaterial::PARAM_LIGHT_POSITION0("lightPosition0");
const FastName NMaterial::PARAM_PROP_AMBIENT_COLOR("ambientColor");
const FastName NMaterial::PARAM_PROP_DIFFUSE_COLOR("diffuseColor");
const FastName NMaterial::PARAM_PROP_SPECULAR_COLOR("specularColor");
const FastName NMaterial::PARAM_LIGHT_AMBIENT_COLOR("materialLightAmbientColor");
const FastName NMaterial::PARAM_LIGHT_DIFFUSE_COLOR("materialLightDiffuseColor");
const FastName NMaterial::PARAM_LIGHT_SPECULAR_COLOR("materialLightSpecularColor");
const FastName NMaterial::PARAM_LIGHT_INTENSITY0("lightIntensity0");
const FastName NMaterial::PARAM_MATERIAL_SPECULAR_SHININESS("materialSpecularShininess");
const FastName NMaterial::PARAM_FOG_LIMIT("fogLimit");
const FastName NMaterial::PARAM_FOG_COLOR("fogColor");
const FastName NMaterial::PARAM_FOG_DENSITY("fogDensity");
const FastName NMaterial::PARAM_FOG_START("fogStart");
const FastName NMaterial::PARAM_FOG_END("fogEnd");
const FastName NMaterial::PARAM_FOG_HALFSPACE_DENSITY("fogHalfspaceDensity");
const FastName NMaterial::PARAM_FOG_HALFSPACE_HEIGHT("fogHalfspaceHeight");
const FastName NMaterial::PARAM_FOG_HALFSPACE_FALLOFF("fogHalfspaceFalloff");
const FastName NMaterial::PARAM_FOG_HALFSPACE_LIMIT("fogHalfspaceLimit");
const FastName NMaterial::PARAM_FOG_ATMOSPHERE_COLOR_SUN("fogAtmosphereColorSun");
const FastName NMaterial::PARAM_FOG_ATMOSPHERE_COLOR_SKY("fogAtmosphereColorSky");
const FastName NMaterial::PARAM_FOG_ATMOSPHERE_SCATTERING("fogAtmosphereScattering");
const FastName NMaterial::PARAM_FOG_ATMOSPHERE_DISTANCE("fogAtmosphereDistance");
const FastName NMaterial::PARAM_FLAT_COLOR("flatColor");
const FastName NMaterial::PARAM_TEXTURE0_SHIFT("texture0Shift");
const FastName NMaterial::PARAM_UV_OFFSET("uvOffset");
const FastName NMaterial::PARAM_UV_SCALE("uvScale");
const FastName NMaterial::PARAM_LIGHTMAP_SIZE("lightmapSize");
const FastName NMaterial::PARAM_SHADOW_COLOR("shadowColor");
const FastName NMaterial::PARAM_DECAL_TILE_SCALE("decalTileCoordScale");
const FastName NMaterial::PARAM_DECAL_TILE_COLOR("decalTileColor");
const FastName NMaterial::PARAM_DETAIL_TILE_SCALE("detailTileCoordScale");
const FastName NMaterial::PARAM_RCP_SCREEN_SIZE("rcpScreenSize");
const FastName NMaterial::PARAM_SCREEN_OFFSET("screenOffset");
const FastName NMaterial::PARAM_NORMAL_SCALE("normalScale");
const FastName NMaterial::PARAM_ALPHATEST_THRESHOLD("alphatestThreshold");

const FastName NMaterial::FLAG_VERTEXFOG = FastName("VERTEX_FOG");
const FastName NMaterial::FLAG_FOG_LINEAR = FastName("FOG_LINEAR");
const FastName NMaterial::FLAG_FOG_HALFSPACE = FastName("FOG_HALFSPACE");
const FastName NMaterial::FLAG_FOG_HALFSPACE_LINEAR = FastName("FOG_HALFSPACE_LINEAR");
const FastName NMaterial::FLAG_FOG_ATMOSPHERE = FastName("FOG_ATMOSPHERE");
const FastName NMaterial::FLAG_TEXTURESHIFT = FastName("TEXTURE0_SHIFT_ENABLED");
const FastName NMaterial::FLAG_TEXTURE0_ANIMATION_SHIFT = FastName("TEXTURE0_ANIMATION_SHIFT");
const FastName NMaterial::FLAG_WAVE_ANIMATION = FastName("WAVE_ANIMATION");
const FastName NMaterial::FLAG_FAST_NORMALIZATION = FastName("FAST_NORMALIZATION");
const FastName NMaterial::FLAG_TILED_DECAL_MASK = FastName("TILED_DECAL_MASK");
const FastName NMaterial::FLAG_FLATCOLOR = FastName("FLATCOLOR");
const FastName NMaterial::FLAG_DISTANCEATTENUATION = FastName("DISTANCE_ATTENUATION");
const FastName NMaterial::FLAG_SPECULAR = FastName("SPECULAR");
const FastName NMaterial::FLAG_SEPARATE_NORMALMAPS = FastName("SEPARATE_NORMALMAPS");

const FastName NMaterial::FLAG_SPHERICAL_LIT = FastName("SPHERICAL_LIT");

const FastName NMaterial::FLAG_TANGENT_SPACE_WATER_REFLECTIONS = FastName("TANGENT_SPACE_WATER_REFLECTIONS");

const FastName NMaterial::FLAG_DEBUG_UNITY_Z_NORMAL = FastName("DEBUG_UNITY_Z_NORMAL");
const FastName NMaterial::FLAG_DEBUG_Z_NORMAL_SCALE = FastName("DEBUG_Z_NORMAL_SCALE");
const FastName NMaterial::FLAG_DEBUG_NORMAL_ROTATION = FastName("DEBUG_NORMAL_ROTATION");


const FastName NMaterial::FLAG_SKINNING = FastName("SKINNING");

const FastName NMaterial::FLAG_LIGHTMAPONLY = FastName("MATERIAL_VIEW_LIGHTMAP_ONLY");
const FastName NMaterial::FLAG_TEXTUREONLY = FastName("MATERIAL_VIEW_TEXTURE_ONLY");
const FastName NMaterial::FLAG_SETUPLIGHTMAP = FastName("SETUP_LIGHTMAP");
const FastName NMaterial::FLAG_VIEWALBEDO = FastName("VIEW_ALBEDO");
const FastName NMaterial::FLAG_VIEWAMBIENT = FastName("VIEW_AMBIENT");
const FastName NMaterial::FLAG_VIEWDIFFUSE = FastName("VIEW_DIFFUSE");
const FastName NMaterial::FLAG_VIEWSPECULAR = FastName("VIEW_SPECULAR");

const FastName NMaterial::FLAG_ALPHATESTVALUE = FastName("ALPHATESTVALUE");

static Vector<FastName> TEXTURE_NAME_PROPS =
{
	NMaterial::TEXTURE_ALBEDO,
	NMaterial::TEXTURE_NORMAL,
	NMaterial::TEXTURE_DETAIL,
	NMaterial::TEXTURE_LIGHTMAP,
	NMaterial::TEXTURE_DECAL
};

static Vector<FastName> RUNTIME_ONLY_FLAGS =
{
	NMaterial::FLAG_LIGHTMAPONLY,
	NMaterial::FLAG_TEXTUREONLY,
	NMaterial::FLAG_SETUPLIGHTMAP,

    NMaterial::FLAG_DEBUG_UNITY_Z_NORMAL,
    NMaterial::FLAG_DEBUG_Z_NORMAL_SCALE,
    NMaterial::FLAG_DEBUG_NORMAL_ROTATION,
	
	NMaterial::FLAG_VIEWALBEDO,
	NMaterial::FLAG_VIEWAMBIENT,
	NMaterial::FLAG_VIEWDIFFUSE,
	NMaterial::FLAG_VIEWSPECULAR
};

static Vector<FastName>  RUNTIME_ONLY_PROPERTIES =
{
    NMaterial::PARAM_LIGHTMAP_SIZE,
    NMaterial::PARAM_LIGHT_POSITION0,
    NMaterial::PARAM_LIGHT_INTENSITY0,
    NMaterial::PARAM_LIGHT_AMBIENT_COLOR,
    NMaterial::PARAM_LIGHT_DIFFUSE_COLOR,
    NMaterial::PARAM_LIGHT_SPECULAR_COLOR,
    NMaterial::PARAM_RCP_SCREEN_SIZE,
    NMaterial::PARAM_SCREEN_OFFSET
};

static Vector<FastName> RUNTIME_ONLY_TEXTURES =
{
    NMaterial::TEXTURE_DYNAMIC_REFLECTION,
    NMaterial::TEXTURE_DYNAMIC_REFRACTION,
    NMaterial::TEXTURE_HEIGHTMAP
};

const FastName NMaterial::DEFAULT_QUALITY_NAME = FastName("Normal");

void IlluminationParams::SetLightmapSize(const int32 &size)
{
	lightmapSize = size;
	
	if(parent)
	{
		float32 floatLightmapSize = (float32)lightmapSize;
		parent->SetPropertyValue(NMaterial::PARAM_LIGHTMAP_SIZE,
								 Shader::UT_FLOAT,
								 1,
								 &floatLightmapSize);
	}
}

void IlluminationParams::SetParent(NMaterial* parentMaterial)
{
	parent = parentMaterial;
	
	if(parent)
	{
		float32 floatLightmapSize = (float32)lightmapSize;
		parent->SetPropertyValue(NMaterial::PARAM_LIGHTMAP_SIZE,
								 Shader::UT_FLOAT,
								 1,
								 &floatLightmapSize);
	}
}

////////////////////////////////////////////////////////////////////////////////
    
NMaterial::NMaterial()
    : materialType(NMaterial::MATERIALTYPE_NONE)
    , materialProperties(16)
    , textures(8)
    , parent(NULL)
    , requiredVertexFormat(0)
    , lightCount(0)
    , dynamicBindFlags(0)
    , materialSortKey(0)
    , baseTechnique(NULL)
    , instancePasses(4)
    , instancePassRenderStates(4)
    , activePassInstance(NULL)
    , activeRenderPass(NULL)
    , illuminationParams(NULL)
    , materialTemplate(NULL)
    , materialSetFlags(8)
{
	memset(lights, 0, sizeof(lights));
}

NMaterial::~NMaterial()
{
	SetParentInternal(NULL);
	ReleaseInstancePasses();
	
	for(HashMap<FastName, UniqueHandle>::iterator it = instancePassRenderStates.begin();
		it != instancePassRenderStates.end();
		++it)
	{
		RenderManager::Instance()->ReleaseRenderState(it->second);
	}
	
	for(HashMap<FastName, NMaterialProperty*>::iterator it = materialProperties.begin();
		it != materialProperties.end();
		++it)
	{
		SafeDelete(it->second);
	}
	materialProperties.clear();
	
	for(HashMap<FastName, TextureBucket*>::iterator it = textures.begin();
		it != textures.end();
		++it)
	{
		SafeDelete(it->second);
	}
	textures.clear();
	
	SafeDelete(illuminationParams);
	
	SafeRelease(baseTechnique);
}

void NMaterial::SetParent(NMaterial* newParent, bool inheritTemplate)
{
	DVASSERT(this != newParent);
	
	if(newParent != parent &&
	   newParent != this)
	{
        SetParentInternal(newParent);
		OnParentChanged(newParent, inheritTemplate);
	}
}
    
void NMaterial::SetParentInternal(DAVA::NMaterial *newParent)
{
    if(parent)
    {
        Vector<NMaterial*>::iterator curMaterial = std::find(parent->children.begin(),
                                                             parent->children.end(),
                                                             this);
        
        DVASSERT(curMaterial != parent->children.end());
        if(curMaterial != parent->children.end())
        {
            parent->children.erase(curMaterial);
        }
        
        SafeRelease(parent);
    }
    
    if(newParent)
    {
        DVASSERT(std::find(newParent->children.begin(), newParent->children.end(), this) == newParent->children.end());
        newParent->children.push_back(this);
    }
    
    SafeRelease(parent);
    parent = SafeRetain(newParent);
}

void NMaterial::SetFlag(const FastName& flag, eFlagValue flagValue)
{
	materialSetFlags.insert(flag, flagValue);

    // TODO: #################
    // ....

    UpdateShaderWithFlags();
}

void NMaterial::ResetFlag(const FastName& flag)
{
	if(materialSetFlags.count(flag) > 0)
	{
		materialSetFlags.erase(flag);
		
		UpdateShaderWithFlags();
	}
}

int32 NMaterial::GetFlagValue(const FastName& flag) const
{
	int32 flagValue = NMaterial::FlagOff | NMaterial::FlagInherited;
	
	if(materialSetFlags.count(flag) > 0)
	{
		flagValue = materialSetFlags.at(flag);
	}
	else
	{
		if(parent)
		{
			flagValue |= parent->GetFlagValue(flag);
		}
	}
	
	return flagValue;
}

bool NMaterial::IsFlagEffective(const FastName& flag) const
{
	int32 flagValue = GetFlagValue(flag);
	
	return ((flagValue & NMaterial::FlagOn) == NMaterial::FlagOn);
}

void NMaterial::Save(KeyedArchive * archive,
					 SerializationContext* serializationContext)
{
	DataNode::Save(archive, serializationContext);
	
	archive->SetString("materialName", (materialName.IsValid()) ? materialName.c_str() : "");
	archive->SetInt32("materialType", (int32)materialType);
	
	if(NMaterial::MATERIALTYPE_INSTANCE == materialType && parent)
	{
		archive->SetUInt64("parentMaterialKey", parent->GetNodeID());
	}
	
	if(GetMaterialGroup().IsValid())
	{
		archive->SetString("materialGroup", GetMaterialGroup().c_str());
	}
	
	archive->SetString("materialTemplate", (materialTemplate) ? materialTemplate->name.c_str() : "");		
	
	KeyedArchive* materialTextures = new KeyedArchive();
	for(HashMap<FastName, TextureBucket*>::iterator it = textures.begin();
		it != textures.end();
		++it)
	{

		if (IsRuntimeTexture(it->first))
			continue;
		
        FilePath texturePath = it->second->GetPath();
        if(!texturePath.IsEmpty())
        {
            String textureRelativePath = texturePath.GetRelativePathname(serializationContext->GetScenePath());
            if(textureRelativePath.size() > 0)
            {
                materialTextures->SetString(it->first.c_str(), textureRelativePath);
            }
        }
	}
	archive->SetArchive("textures", materialTextures);
	SafeRelease(materialTextures);
	
	KeyedArchive* materialProps = new KeyedArchive();
	for(HashMap<FastName, NMaterialProperty*>::iterator it = materialProperties.begin();
		it != materialProperties.end();
		++it)
	{
		if(!IsRuntimeProperty(it->first))
		{
			NMaterialProperty* property = it->second;
			
			uint32 dataSize = Shader::GetUniformTypeSize(property->type) * property->size;
			uint8* propertyStorage = new uint8[dataSize + sizeof(property->type) + sizeof(property->size)];
			
			memcpy(propertyStorage, &property->type, sizeof(property->type));
			memcpy(propertyStorage + sizeof(property->type), &property->size, sizeof(property->size));
			memcpy(propertyStorage + sizeof(property->type) + sizeof(property->size), property->data, dataSize);
			
			materialProps->SetByteArray(it->first.c_str(), propertyStorage, dataSize + sizeof(property->type) + sizeof(property->size));
			
			SafeDeleteArray(propertyStorage);
		}
	}
	archive->SetArchive("properties", materialProps);
	SafeRelease(materialProps);
	
	KeyedArchive* materialSetFlagsArchive = new KeyedArchive();
	for(HashMap<FastName, int32>::iterator it = materialSetFlags.begin();
		it != materialSetFlags.end();
		++it)
	{
		if(!IsRuntimeFlag(it->first))
		{
			materialSetFlagsArchive->SetInt32(it->first.c_str(), it->second);
		}
	}
	archive->SetArchive("setFlags", materialSetFlagsArchive);
	SafeRelease(materialSetFlagsArchive);
	
	if(illuminationParams)
	{
		archive->SetBool("illumination.isUsed", illuminationParams->isUsed);
		archive->SetBool("illumination.castShadow", illuminationParams->castShadow);
		archive->SetBool("illumination.receiveShadow", illuminationParams->receiveShadow);
		archive->SetInt32("illumination.lightmapSize", illuminationParams->lightmapSize);
	}
}

void NMaterial::Load(KeyedArchive * archive,
					 SerializationContext* serializationContext)
{
	DataNode::Load(archive, serializationContext);

    if(archive->IsKeyExists("materialName"))
    {
        materialName = FastName(archive->GetString("materialName"));
    }

	if(archive->IsKeyExists("materialType"))
    {
        materialType = (NMaterial::eMaterialType)archive->GetInt32("materialType");
    }

    if (archive->IsKeyExists("materialKey"))
    {
        uint64 materialKey = (NMaterial::eMaterialType)archive->GetUInt64("materialKey");
        id = materialKey;
    }

	if(archive->IsKeyExists("materialGroup"))
	{
		SetMaterialGroup(FastName(archive->GetString("materialGroup").c_str()));
	}
	else
	{
		SetMaterialGroup(materialGroup);
	}
	
	// orderedQuality will be set, after SetMaterialGroup call
	// but we are loading now, so currentQuality should be set to orderedQuality
	// to process loading with exactly ordered quality
	currentQuality = orderedQuality;
	
    if(archive->IsKeyExists("materialTemplate"))
    {
	    String materialTemplateName = archive->GetString("materialTemplate");
	    if(materialTemplateName.size() > 0)
	    {
		    NMaterialHelper::SwitchTemplate(this, FastName(materialTemplateName.c_str()));
	    }
	    else
	    {
		    //VI: will inherit from parent probably
		    materialTemplate = NULL;
	    }
    }
	
    if(archive->IsKeyExists("properties"))
    {
	    const Map<String, VariantType*>& propsMap = archive->GetArchive("properties")->GetArchieveData();
	    for(Map<String, VariantType*>::const_iterator it = propsMap.begin();
		    it != propsMap.end();
		    ++it)
	    {
		    if (IsRuntimeProperty(FastName(it->first)))continue;

		    const VariantType* propVariant = it->second;
		    DVASSERT(VariantType::TYPE_BYTE_ARRAY == propVariant->type);
		    DVASSERT(propVariant->AsByteArraySize() >= static_cast<int32>(sizeof(uint32) +sizeof(uint32)));
		
		    const uint8* ptr = propVariant->AsByteArray();
		
		    SetPropertyValue(FastName(it->first),
						     *(Shader::eUniformType*)ptr,
						     *(ptr + sizeof(Shader::eUniformType)),
						     ptr + sizeof(Shader::eUniformType) + sizeof(uint8));
	    }
    }

    if(archive->IsKeyExists("textures"))
    {
	    const Map<String, VariantType*>& texturesMap = archive->GetArchive("textures")->GetArchieveData();
	    for(Map<String, VariantType*>::const_iterator it = texturesMap.begin();
		    it != texturesMap.end();
		    ++it)
	    {
		    String relativePathname = it->second->AsString();
		    SetTexture(FastName(it->first), serializationContext->GetScenePath() + relativePathname);
	    }
    }
	
	if(archive->IsKeyExists("illumination.isUsed"))
	{
		illuminationParams = new IlluminationParams(this);
		
		illuminationParams->isUsed = archive->GetBool("illumination.isUsed", illuminationParams->isUsed);
		illuminationParams->castShadow = archive->GetBool("illumination.castShadow", illuminationParams->castShadow);
		illuminationParams->receiveShadow = archive->GetBool("illumination.receiveShadow", illuminationParams->receiveShadow);
		illuminationParams->SetLightmapSize(archive->GetInt32("illumination.lightmapSize", illuminationParams->lightmapSize));
	}
	
    if(archive->IsKeyExists("setFlags"))
    {
	    const Map<String, VariantType*>& flagsMap = archive->GetArchive("setFlags")->GetArchieveData();
	    for(Map<String, VariantType*>::const_iterator it = flagsMap.begin();
		    it != flagsMap.end();
		    ++it)
	    {
		    SetFlag(FastName(it->first), (NMaterial::eFlagValue)it->second->AsInt32());
	    }
    }
	
	if(archive->IsKeyExists("parentMaterialKey") && NMaterial::MATERIALTYPE_INSTANCE == materialType)
	{
		uint64 parentKey = archive->GetUInt64("parentMaterialKey");
		serializationContext->AddBinding(parentKey, this);
	}
}

void NMaterial::SetQuality(const FastName& stateName)
{
	DVASSERT(stateName.IsValid());
	orderedQuality = stateName;
}

FastName NMaterial::GetEffectiveQuality() const
{
	FastName ret = orderedQuality;
	
	const NMaterial* parent = GetParent();
	while(!ret.IsValid() && NULL != parent)
	{
		ret = parent->orderedQuality;
		parent = parent->parent;
	}
	
	return ret;
}

bool NMaterial::ReloadQuality(bool force)
{
	bool ret = false;
	
	DVASSERT(materialTemplate);
	
	FastName effectiveQuality = GetEffectiveQuality();
	FastName curGroupQuality = QualitySettingsSystem::Instance()->GetCurMaterialQuality(GetMaterialGroup());
	if(curGroupQuality != currentQuality)
	{
		effectiveQuality = curGroupQuality;
	}
	
	bool hasQuality = (materialTemplate->techniqueStateMap.count(effectiveQuality) > 0);
	if(hasQuality && (effectiveQuality != currentQuality || force))
	{
		ret = true;
		currentQuality = effectiveQuality;
		        
		if(NMaterial::MATERIALTYPE_INSTANCE == materialType)
		{
			OnInstanceQualityChanged();
		}
		else if(NMaterial::MATERIALTYPE_MATERIAL == materialType)
		{
            SetQuality(currentQuality);

			UpdateMaterialTemplate();
			
			LoadActiveTextures();
			
			this->Retain();
			
			size_t childrenCount = children.size();
			for(size_t i = 0; i < childrenCount; ++i)
			{
				NMaterial* child = children[i];
				
				child->SetQuality(currentQuality);
				child->ReloadQuality(force);
			}
			
			//VI: TODO: review if this call is realy needed at this point
			//CleanupUnusedTextures();
			
			this->Release();
		}
		else
		{
			DVASSERT(false && "Material is not initialized properly!");
		}
	}
	
	return ret;
}

NMaterial* NMaterial::Clone()
{
	NMaterial* clonedMaterial = NULL;
	if(NMaterial::MATERIALTYPE_MATERIAL == materialType)
	{
		clonedMaterial = NMaterial::CreateMaterial(materialName,
												   materialTemplate->name,
												   currentQuality);
		
	}
	else if(NMaterial::MATERIALTYPE_INSTANCE == materialType)
	{
		clonedMaterial = NMaterial::CreateMaterialInstance();
	}
    else if(NMaterial::MATERIALTYPE_GLOBAL == materialType)
    {
        clonedMaterial = NMaterial::CreateGlobalMaterial(materialName);
    }
	else
	{
		DVASSERT(false && "Material is not initialized properly!");
		return clonedMaterial;
	}
	
	//clonedMaterial->SetName(GetName());
			
	for(HashMap<FastName, NMaterialProperty*>::iterator it = materialProperties.begin();
		it != materialProperties.end();
		++it)
	{
		clonedMaterial->materialProperties.insert(it->first, it->second->Clone());
	}
	
	for(HashMap<FastName, TextureBucket*>::iterator it = textures.begin();
		it != textures.end();
		++it)
	{
		clonedMaterial->SetTexture(it->first, it->second->GetPath());
	}
	
	if(illuminationParams)
	{
		clonedMaterial->illuminationParams = new IlluminationParams(*illuminationParams);
		clonedMaterial->illuminationParams->SetParent(clonedMaterial);
	}
	
	if(NMaterial::MATERIALTYPE_INSTANCE == materialType)
	{
		clonedMaterial->SetParent(parent);
	}
	
	for(HashMap<FastName, int32>::iterator it = materialSetFlags.begin();
		it != materialSetFlags.end();
		++it)
	{
		clonedMaterial->SetFlag(it->first, (NMaterial::eFlagValue)it->second);
	}
	
	for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
		it != instancePasses.end();
		++it)
	{
		RenderPassInstance* currentPass = it->second;
		
		if(currentPass->dirtyState)
		{
			RenderPassInstance* clonedPass = clonedMaterial->instancePasses.at(it->first);
			DVASSERT(clonedPass);
			
			clonedPass->dirtyState = true;
			clonedPass->SetRenderStateHandle(currentPass->GetRenderStateHandle());
		}
	}
	
	for(HashMap<FastName, UniqueHandle>::iterator it = instancePassRenderStates.begin();
		it != instancePassRenderStates.end();
		++it)
	{
		clonedMaterial->instancePassRenderStates.insert(it->first, it->second);
		RenderManager::Instance()->RetainRenderState(it->second);
	}
	
	//DataNode properties
	clonedMaterial->id = 0;
	clonedMaterial->scene = scene;
	clonedMaterial->isRuntime = isRuntime;
	
	return clonedMaterial;
}

NMaterial* NMaterial::Clone(const String& newName)
{
	NMaterial* clonedMaterial = Clone();
	clonedMaterial->SetMaterialName(FastName(newName));
	return clonedMaterial;
}

IlluminationParams * NMaterial::GetIlluminationParams(bool createIfNeeded /*= true*/)
{
	if(createIfNeeded && !illuminationParams)
		illuminationParams = new IlluminationParams(this);
	
	return illuminationParams;
}

void NMaterial::ReleaseIlluminationParams()
{
	SafeDelete(illuminationParams);
}

void NMaterial::RemoveTexture(const FastName& textureFastName)
{
	TextureBucket* bucket = textures.at(textureFastName);
	DVASSERT(bucket);
	
	if(bucket)
	{
		textures.erase(textureFastName);
		SafeDelete(bucket);
		
		SetTexturesDirty();
	}
}

void NMaterial::SetTexture(const FastName& textureFastName,
						   const FilePath& texturePath)
{
	TextureBucket* bucket = textures.at(textureFastName);
	if(NULL == bucket)
	{
		bucket = new TextureBucket();
		textures.insert(textureFastName, bucket);
	}
	
	if(bucket->GetPath() != texturePath)
	{
		bucket->SetTexture(NULL); //VI: texture WILL NOT BE RELOADED if it's not active in the current quality
		bucket->SetPath(texturePath);
		
		if(IsTextureActive(textureFastName))
		{
			Texture* tx = Texture::CreateFromFile(texturePath, textureFastName);
			bucket->SetTexture(tx);
			SafeRelease(tx);
		}
		
		SetTexturesDirty();
	}
}

void NMaterial::SetTexture(const FastName& textureFastName,
						   Texture* texture)
{
	TextureBucket* bucket = textures.at(textureFastName);
	if(NULL == bucket)
	{
		bucket = new TextureBucket();
		textures.insert(textureFastName, bucket);
	}
	
	if(texture != bucket->GetTexture())
	{
		bucket->SetTexture(texture);
		bucket->SetPath((texture) ? texture->texDescriptor->pathname : FilePath());
		
		SetTexturesDirty();
	}
}

void NMaterial::SetTexturePath(const FastName& textureFastName, const FilePath& texturePath)
{
	TextureBucket* bucket = textures.at(textureFastName);
	
	if(NULL == bucket)
	{
		bucket = new TextureBucket();
		textures.insert(textureFastName, bucket);
	}
	
	bucket->SetPath(texturePath);
}

Texture * NMaterial::GetTexture(const FastName& textureFastName) const
{
	TextureBucket* bucket = textures.at(textureFastName);
	return (NULL == bucket) ? NULL : bucket->GetTexture();
}

const FilePath& NMaterial::GetTexturePath(const FastName& textureFastName) const
{
	static FilePath invalidEmptyPath;
	TextureBucket* bucket = textures.at(textureFastName);
	return (NULL == bucket) ? invalidEmptyPath : bucket->GetPath();
}

Texture * NMaterial::GetEffectiveTexture(const FastName& textureFastName) const
{
	TextureBucket* bucket = GetEffectiveTextureBucket(textureFastName);
	return (NULL == bucket) ? NULL : bucket->GetTexture();
}

const FilePath& NMaterial::GetEffectiveTexturePath(const FastName& textureFastName) const
{
	static FilePath invalidEmptyPath;
	TextureBucket* bucket = GetEffectiveTextureBucket(textureFastName);
	return (NULL == bucket) ? invalidEmptyPath : bucket->GetPath();
}

Texture * NMaterial::GetTexture(uint32 index) const
{
	DVASSERT(index < textures.size());
	
	TextureBucket* bucket = textures.valueByIndex(index);
	return bucket->GetTexture();
}

const FilePath& NMaterial::GetTexturePath(uint32 index) const
{
	DVASSERT(index < textures.size());
	
	TextureBucket* bucket = textures.valueByIndex(index);
	return bucket->GetPath();
}

const FastName& NMaterial::GetTextureName(uint32 index) const
{
	DVASSERT(index < textures.size());
	
	return textures.keyByIndex(index);
}

uint32 NMaterial::GetTextureCount() const
{
	return static_cast<uint32>(textures.size());
}

void NMaterial::SetPropertyValue(const FastName & keyName,
								 Shader::eUniformType type,
								 uint32 size,
								 const void * data)
{
	DVASSERT(data);
	DVASSERT(size);
	DVASSERT(keyName.IsValid());
	
	size_t dataSize = Shader::GetUniformTypeSize(type) * size;
	NMaterialProperty * materialProperty = materialProperties.at(keyName);
	if (materialProperty)
	{
		if (materialProperty->type != type || materialProperty->size != size)
		{
			//VI: material property type or size chnage should never happen at runtime
			DVASSERT(false && "Runtime change of material property type!");
			
			NMaterialProperty* newProp = new NMaterialProperty();
			newProp->size = size;
			newProp->type = type;
			newProp->data = new uint8[dataSize];
			
			OnMaterialPropertyRemoved(keyName);
			
			SafeDelete(materialProperty);
			materialProperties.insert(keyName, newProp);
			
			OnMaterialPropertyAdded(keyName);
			
			materialProperty = newProp;
		}
	}
	else
	{
		materialProperty = new NMaterialProperty();
		materialProperty->size = size;
		materialProperty->type = type;
		materialProperty->data = new uint8[dataSize];
		materialProperties.insert(keyName, materialProperty);
		
		OnMaterialPropertyAdded(keyName);
	}
	
	memcpy(materialProperty->data, data, dataSize);
	
	//VI: this is temporary solution. It has to be removed once lighting system + autobind system is ready
//	if(IsDynamicLit() && IsLightingProperty(keyName))
//	{
//		UpdateLightingProperties(lights[0]);
//	}
}

NMaterialProperty* NMaterial::GetPropertyValue(const FastName & keyName) const
{
	const NMaterial* currentMaterial = this;
	NMaterialProperty * property = NULL;
	while(currentMaterial != NULL)
	{
		property = currentMaterial->materialProperties.at(keyName);
		if (property)
		{
			break;
		}
		
		currentMaterial = currentMaterial->parent;
	}
	
	return property;
}

NMaterialProperty* NMaterial::GetMaterialProperty(const FastName & keyName) const
{
	return materialProperties.at(keyName);
}

void NMaterial::RemoveMaterialProperty(const FastName & keyName)
{
	NMaterialProperty* prop = materialProperties.at(keyName);
	if(prop)
	{
		materialProperties.erase(keyName);
		SafeDelete(prop);
	}
	
	OnMaterialPropertyRemoved(keyName);
}

void NMaterial::SetMaterialName(const FastName& name)
{
	materialName = name;
}

FastName NMaterial::GetMaterialGroup() const
{
	FastName result = materialGroup;
	
	NMaterial* curMaterial = GetParent();
	while(!result.IsValid() && curMaterial != NULL)
	{
		result = curMaterial->materialGroup;
		
		curMaterial = curMaterial->GetParent();
	}
	
	return result;
}

void NMaterial::SetMaterialGroup(const FastName &group)
{
	if(group.IsValid())
	{
		materialGroup = group;
		const MaterialQuality* curQuality = QualitySettingsSystem::Instance()->GetMaterialQuality(group, QualitySettingsSystem::Instance()->GetCurMaterialQuality(group));
		
		if(NULL != curQuality)
		{
			SetQuality(curQuality->qualityName);
		}
        else
        {
            Logger::Error("Material \"%s\" uses quality group \"%s\", that isn't exist in quality system.", materialName.c_str(), group.c_str());
        }
	}
}

void NMaterial::SetMaterialTemplate(const NMaterialTemplate* matTemplate,
									const FastName& defaultQuality)
{
	materialTemplate = matTemplate;
	currentQuality = defaultQuality;
	
	OnMaterialTemplateChanged();
	
	LoadActiveTextures();
	
	this->Retain();		
	
	size_t childrenCount = children.size();
	for(size_t i = 0; i < childrenCount; ++i)
	{
		children[i]->SetMaterialTemplate(matTemplate, defaultQuality);
	}
	
	//VI: TODO: review if this call is realy needed at this point
	//CleanupUnusedTextures();
	
	this->Release();
}

void NMaterial::OnMaterialTemplateChanged()
{
	UpdateMaterialTemplate();
}

void NMaterial::OnParentChanged(NMaterial* newParent, bool inheritTemplate)
{
	materialSortKey = (uint16)((pointer_size) newParent);
	
	bool useParentTemplate = (inheritTemplate || NULL == materialTemplate);

    if(useParentTemplate)
    {
        if(newParent)
        {
            SetMaterialTemplate(newParent->materialTemplate, newParent->currentQuality);
        }
    }
    else
    {
        UpdateShaderWithFlags();
    }
	
	SetTexturesDirty();
    InvalidateProperties();
}

void NMaterial::OnInstanceQualityChanged()
{
	UpdateMaterialTemplate();
	
	LoadActiveTextures();
	
	//VI: TODO: review if this call is realy needed at this point
	//CleanupUnusedTextures();
}

void NMaterial::BuildEffectiveFlagSet(FastNameSet& effectiveFlagSet)
{
	effectiveFlagSet.clear();
	
	BuildEffectiveFlagSetInternal(effectiveFlagSet);
}

void NMaterial::BuildEffectiveFlagSetInternal(FastNameSet& effectiveFlagSet)
{
	if(parent)
	{
		parent->BuildEffectiveFlagSetInternal(effectiveFlagSet);
	}
	
	//VI: now when flags were set by parent let's set them from our collection or filter out overriden flags.
	for(HashMap<FastName, int32>::iterator it = materialSetFlags.begin();
		it != materialSetFlags.end();
		++it)
	{
		if(NMaterial::FlagOn == it->second)
		{
			effectiveFlagSet.Insert(it->first);
		}
		else
		{
			effectiveFlagSet.erase(it->first);
		}
	}
}

void NMaterial::ReleaseInstancePasses()
{
	for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
		it != instancePasses.end();
		++it)
	{
		SafeDelete(it->second);
	}
	instancePasses.clear();
	
	activePassInstance = NULL;
	activePassName = FastName();
	activeRenderPass = NULL;
}

void NMaterial::UpdateMaterialTemplate()
{
	DVASSERT(materialTemplate);
	
	ReleaseInstancePasses();
	
	FastNameSet effectiveFlags;
	BuildEffectiveFlagSet(effectiveFlags);
	
	FastName techniqueName;
	if(currentQuality.IsValid())
	{
		techniqueName = materialTemplate->techniqueStateMap.at(currentQuality);
	}
	
	if(!techniqueName.IsValid())
	{
		techniqueName = materialTemplate->techniqueStateMap.valueByIndex(0);
	}
	
	DVASSERT(techniqueName.IsValid());
	
	SafeRelease(baseTechnique);
	baseTechnique = RenderTechniqueSingleton::Instance()->CreateTechniqueByName(techniqueName);
	
	//DVASSERT(baseTechnique);
	//materialSortKey = baseTechnique->GetTechniqueId();
    
    if(NULL == baseTechnique)
    {
        baseTechnique = RenderTechniqueSingleton::Instance()->CreateTechniqueByName(NMaterialName::TEXTURED_OPAQUE);
        DVASSERT(baseTechnique);
    }
	
    requiredVertexFormat = 0;
	uint32 passCount = baseTechnique->GetPassCount();
	for(uint32 i = 0; i < passCount; ++i)
	{
		RenderTechniquePass* pass = baseTechnique->GetPassByIndex(i);
		UpdateRenderPass(baseTechnique->GetPassName(i), effectiveFlags, pass);
	}
	
	SetTexturesDirty();
	
	SetRenderLayers(RenderLayerManager::Instance()->GetLayerIDMaskBySet(baseTechnique->GetLayersSet()));

    //{VI: temporray code should be removed once lighting system is up
    dynamicBindFlags = (baseTechnique->GetLayersSet().count(LAYER_SHADOW_VOLUME) != 0) ? DYNAMIC_BIND_LIGHT : 0;
    for(uint32 i = 0; i < passCount; ++i)
    {
        RenderTechniquePass* pass = baseTechnique->GetPassByIndex(i);
        const FastNameSet& defines = pass->GetUniqueDefineSet();
        dynamicBindFlags |= (defines.count(DEFINE_VERTEX_LIT) || defines.count(DEFINE_PIXEL_LIT) || defines.count(FLAG_SPHERICAL_LIT)) ? DYNAMIC_BIND_LIGHT : 0;
        dynamicBindFlags |= defines.count(FLAG_SPHERICAL_LIT) ? DYNAMIC_BIND_OBJECT_CENTER : 0;
    }
}

void NMaterial::UpdateRenderPass(const FastName& passName,
								 const FastNameSet& instanceDefines,
								 RenderTechniquePass* pass)
{
	RenderState* parentRenderState = pass->GetRenderState();
	
	RenderPassInstance* passInstance = new RenderPassInstance();
	passInstance->dirtyState = (instancePassRenderStates.count(passName) != 0);
	
	if(passInstance->dirtyState)
	{
		passInstance->SetRenderStateHandle(instancePassRenderStates.at(passName));
	}
	else
	{
		passInstance->SetRenderStateHandle(parentRenderState->stateHandle);
	}
	
	passInstance->SetTextureStateHandle(InvalidUniqueHandle);
	passInstance->texturesDirty = false;
	
	Shader* shader = pass->CompileShader(instanceDefines);
	passInstance->SetShader(shader);
    requiredVertexFormat |= shader->GetRequiredVertexFormat();
	SafeRelease(shader);
	
	passInstance->SetRenderer(parentRenderState->renderer);
	passInstance->SetColor(parentRenderState->color);
	
	instancePasses.insert(passName, passInstance);
	
	BuildTextureParamsCache(passInstance);
	BuildActiveUniformsCacheParamsCache(passInstance);
}

void NMaterial::BuildTextureParamsCache(RenderPassInstance* passInstance)
{
	Shader* shader = passInstance->GetShader();
	
	uint32 uniformCount = shader->GetUniformCount();
	for(uint32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
	{
		Shader::Uniform * uniform = shader->GetUniform(uniformIndex);
		if(uniform->shaderSemantic == UNKNOWN_SEMANTIC)
		{
			if(uniform->type == Shader::UT_SAMPLER_2D ||
			   uniform->type == Shader::UT_SAMPLER_CUBE)
			{
				ShaderAsset* asset = ShaderCache::Instance()->Get(shader->GetAssetName());
				DVASSERT(asset);
				int32 textureIndex = asset->GetDefaultValue(uniform->name).int32Value;
				passInstance->textureIndexMap.insert(uniform->name, textureIndex);
			}
		}
	}

    SetTexturesDirty();
}
    
void NMaterial::BuildTextureParamsCache()
{
    HashMap<FastName, DAVA::NMaterial::RenderPassInstance*>::iterator it = instancePasses.begin();
    HashMap<FastName, DAVA::NMaterial::RenderPassInstance*>::iterator endIt = instancePasses.end();
    while(it != endIt)
    {
        BuildTextureParamsCache(it->second);
        ++it;
    }
}
    
void NMaterial::BuildActiveUniformsCacheParamsCache()
{
    HashMap<FastName, DAVA::NMaterial::RenderPassInstance*>::iterator it = instancePasses.begin();
    HashMap<FastName, DAVA::NMaterial::RenderPassInstance*>::iterator endIt = instancePasses.end();
    while(it != endIt)
    {
        BuildActiveUniformsCacheParamsCache(it->second);
        ++it;
    }
}

void NMaterial::BuildActiveUniformsCacheParamsCache(RenderPassInstance* passInstance)
{
	Shader* shader = passInstance->GetShader();
	passInstance->activeUniformsCache.clear();
	
	uint32 uniformCount = shader->GetUniformCount();
	for(uint32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
	{
		Shader::Uniform * uniform = shader->GetUniform(uniformIndex);
		
		if((UNKNOWN_SEMANTIC == uniform->shaderSemantic ||
			PARAM_COLOR == uniform->shaderSemantic) &&
		   (Shader::UT_SAMPLER_2D != uniform->type &&
			Shader::UT_SAMPLER_CUBE != uniform->type)) //TODO: do something with conditional binding
		{
			NMaterialProperty* prop = GetPropertyValue(uniform->name);
			
			UniformCacheEntry entry;
			entry.uniform = uniform;
			entry.index = uniformIndex;
			entry.prop = prop;
			
			passInstance->activeUniformsCache.push_back(entry);
		}
	}
	
	passInstance->activeUniformsCachePtr = NULL;
	passInstance->activeUniformsCacheSize = 0;
	if(passInstance->activeUniformsCache.size())
	{
		passInstance->activeUniformsCachePtr = &passInstance->activeUniformsCache[0];
		passInstance->activeUniformsCacheSize = passInstance->activeUniformsCache.size();
	}
}

NMaterial::TextureBucket* NMaterial::GetEffectiveTextureBucket(const FastName& textureFastName) const
{
	TextureBucket* bucket = NULL;
	const NMaterial* currentMaterial = this;
	while(currentMaterial &&
		  NULL == bucket)
	{
		bucket = currentMaterial->textures.at(textureFastName);
		
		currentMaterial = currentMaterial->parent;
	}
	
	return bucket;
}

void NMaterial::LoadActiveTextures()
{
	for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
		it != instancePasses.end();
		++it)
	{
		RenderPassInstance* pass = it->second;
		
		for(HashMap<FastName, int32>::iterator texIt = pass->textureIndexMap.begin();
			texIt != pass->textureIndexMap.end();
			++texIt)
		{
			GetOrLoadTextureRecursive(texIt->first);
		}
	}
}

void NMaterial::CleanupUnusedTextures()
{
	for(HashMap<FastName, TextureBucket*>::iterator it = textures.begin();
		it != textures.end();
		++it)
	{
		if(!IsTextureActive(it->first))
		{
			it->second->SetTexture(NULL);
		}
	}
	
	if(parent)
	{
		parent->CleanupUnusedTextures();
	}
}

Texture* NMaterial::GetOrLoadTextureRecursive(const FastName& textureName)
{
	Texture* tex = NULL;
	
	NMaterial* currentMaterial = this;
	while(currentMaterial)
	{
		TextureBucket* bucket = currentMaterial->textures.at(textureName);
		
		if(bucket)
		{
			if(NULL == bucket->GetTexture())
			{
				Texture* tx = Texture::CreateFromFile(bucket->GetPath(), textureName);
				bucket->SetTexture(tx);
				SafeRelease(tx);
			}
			
			tex = bucket->GetTexture();
			break;
		}
		
		currentMaterial = currentMaterial->parent;
	}
	
	return tex;
}

bool NMaterial::IsTextureActive(const FastName& textureName) const
{
	bool active = false;
	for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
		it != instancePasses.end();
		++it)
	{
		RenderPassInstance* pass = it->second;
		if(pass->textureIndexMap.count(textureName) > 0)
		{
			active = true;
			break;
		}
	}
	
	if(!active)
	{
		size_t childrenCount = children.size();
		for(size_t i = 0; i < childrenCount; ++i)
		{
			active = children[i]->IsTextureActive(textureName);
			if(active)
			{
				break;
			}
		}
	}
	
	return active;
}

void NMaterial::SetTexturesDirty()
{
	for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
		it != instancePasses.end();
		++it)
	{
		RenderPassInstance* pass = it->second;
		pass->texturesDirty = true;
	}
	
	size_t childrenCount = children.size();
	for(size_t i = 0; i < childrenCount; ++i)
	{
		children[i]->SetTexturesDirty();
	}
}

void NMaterial::PrepareTextureState(RenderPassInstance* passInstance)
{
	DVASSERT(passInstance);
	
	TextureStateData textureData;
	for(HashMap<FastName, int32>::iterator texIt = passInstance->textureIndexMap.begin();
		texIt != passInstance->textureIndexMap.end();
		++texIt)
	{
		textureData.SetTexture(texIt->second, GetOrLoadTextureRecursive(texIt->first));
		//VI: use commented out part of code for debugging texture setting
		//if(NULL == textureData.GetTexture(texIt->second))
		//{
		//	//VI: this case is mostly for ResEditor
		//	textureData.SetTexture(texIt->second, GetStubTexture(texIt->first));
		//}
	}
	
	UniqueHandle textureState = RenderManager::Instance()->CreateTextureState(textureData);
	passInstance->SetTextureStateHandle(textureState);
	RenderManager::Instance()->ReleaseTextureState(textureState);
	
	passInstance->texturesDirty = false;
}

void NMaterial::BindMaterialTechnique(const FastName & passName, Camera* camera)
{
	if(activePassName != passName)
	{
		activePassName = passName;
		activeRenderPass = baseTechnique->GetPassByName(passName);
		activePassInstance = instancePasses.at(passName);
		
		DVASSERT(activeRenderPass);
		DVASSERT(activePassInstance);
	}
	
	//VI: this call is temporary solution. It will be removed once autobind system and lighting system ready
	//SetupPerFrameProperties(camera);
	
	BindMaterialTextures(activePassInstance);
	
	activePassInstance->FlushState();
	
	BindMaterialProperties(activePassInstance);
}

//void NMaterial::SetupPerFrameProperties(Camera* camera)
//{
//	if(camera && IsDynamicLit() && lights[0])
//	{
//		//const Matrix4 & matrix = camera->GetMatrix();
//		//Vector3 lightPosition0InCameraSpace = lights[0]->GetPosition() * matrix;
//		const Vector4 & lightPositionDirection0InCameraSpace = lights[0]->CalculatePositionDirectionBindVector(camera);
//        
//		SetPropertyValue(NMaterial::PARAM_LIGHT_POSITION0, Shader::UT_FLOAT_VEC4, 1, lightPositionDirection0InCameraSpace.data);
//	}
//}

void NMaterial::BindMaterialTextures(RenderPassInstance* passInstance)
{
	if(passInstance->texturesDirty)
	{
		PrepareTextureState(passInstance);
	}
}

void NMaterial::BindMaterialProperties(RenderPassInstance* passInstance)
{
	//TODO: think of a way to re-bind only changed properties. (Move dirty flag to UniformCacheEntry or something?)
	if(passInstance->propsDirty)
	{
		for(size_t i = 0; i < passInstance->activeUniformsCacheSize; ++i)
		{
			UniformCacheEntry& uniformEntry = passInstance->activeUniformsCachePtr[i];
			uniformEntry.prop = GetPropertyValue(uniformEntry.uniform->name);
		}
		
		passInstance->propsDirty = false;
	}
	
	Shader* shader = passInstance->GetShader();
	for(size_t i = 0; i < passInstance->activeUniformsCacheSize; ++i)
	{
		UniformCacheEntry& uniformEntry = passInstance->activeUniformsCachePtr[i];
		Shader::Uniform* uniform = uniformEntry.uniform;
		
		if(uniformEntry.prop)
		{
			RENDERER_UPDATE_STATS(materialParamUniformBindCount++);
			shader->SetUniformValueByUniform(uniform,
											 uniform->type,
											 uniform->size,
											 uniformEntry.prop->data);
		}
	}
}

void NMaterial::OnMaterialPropertyAdded(const FastName& propName)
{
    // TODO: 
    // now all properties are processed, but 
    // should be processed only one - propName
    // ...

    InvalidateProperties();
}

void NMaterial::OnMaterialPropertyRemoved(const FastName& propName)
{
    // TODO: 
    // now all properties are processed, but 
    // should be processed only one - propName
    // ...

    InvalidateProperties();
}

void NMaterial::InvalidateProperties()
{
    for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
        it != instancePasses.end();
        ++it)
    {
        RenderPassInstance* pass = it->second;
        pass->propsDirty = true;
    }

    this->Retain();

    size_t childrenCount = children.size();
    for(size_t i = 0; i < childrenCount; ++i)
    {
        children[i]->InvalidateProperties();
    }

    this->Release();
}

void NMaterial::Draw(PolygonGroup * polygonGroup)
{
	// TODO: Remove support of OpenGL ES 1.0 from attach render data
	RenderManager::Instance()->SetRenderData(polygonGroup->renderDataObject);
	RenderManager::Instance()->AttachRenderData();
	
	//Logger::FrameworkDebug("[Material::Draw] %s", baseTechnique->GetName().c_str());
	
	// TODO: rethink this code
	if(polygonGroup->renderDataObject->GetIndexBufferID() != 0)
	{
		RenderManager::Instance()->HWDrawElements(polygonGroup->primitiveType, polygonGroup->indexCount, polygonGroup->renderDataObject->GetIndexFormat(), 0);
	}
	else
	{
		RenderManager::Instance()->HWDrawElements(polygonGroup->primitiveType, polygonGroup->indexCount, polygonGroup->renderDataObject->GetIndexFormat(), polygonGroup->indexArray);
	}
}

void NMaterial::Draw(RenderDataObject* renderData, uint16* indices, uint16 indexCount)
{
	DVASSERT(renderData);
	
	RenderManager::Instance()->SetRenderData(renderData);
	RenderManager::Instance()->AttachRenderData();
	
	//Logger::FrameworkDebug("[Material::Draw] %s", baseTechnique->GetName().c_str());
	
	// TODO: rethink this code
	if(renderData->GetIndexBufferID() != 0)
	{
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, renderData->indexCount, renderData->GetIndexFormat(), 0);
	}
	else
	{
		if(renderData->indexCount)
		{
			RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, renderData->indexCount, renderData->GetIndexFormat(), renderData->indices);
		}
		else
		{
			RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, indexCount, EIF_16, indices);
		}
	}
}

//void NMaterial::SetLight(uint32 index, Light * light, bool forceUpdate)
//{
//	if(NMaterial::MATERIALTYPE_INSTANCE == materialType)
//	{
//		if(parent)
//		{
//			parent->SetLight(index, light, forceUpdate);
//		}
//		else
//		{
//			SetLightInternal(index, light, forceUpdate);
//		}
//	}
//	else if(NMaterial::MATERIALTYPE_MATERIAL == materialType)
//	{
//		SetLightInternal(index, light, forceUpdate);
//		
//		for(size_t i = 0; i < children.size(); ++i)
//		{
//			children[i]->SetLightInternal(index, light, forceUpdate);
//		}
//	}
//}
//
//void NMaterial::SetLightInternal(int index, Light* light, bool forceUpdate)
//{
//	bool changed = forceUpdate || (light != lights[index]);
//	lights[index] = light;
//	
//	if(changed && materialDynamicLit)
//	{
//		UpdateLightingProperties(lights[0]);
//	}
//}
//
//void NMaterial::UpdateLightingProperties(Light* light)
//{
//	NMaterialProperty* propAmbientColor = GetMaterialProperty(NMaterial::PARAM_PROP_AMBIENT_COLOR);
//	if(propAmbientColor)
//	{
//		Color lightAmbientColor = (light) ? light->GetAmbientColor() : Color(0, 0, 0, 0);
//		Color materialAmbientColor = *(Color*) propAmbientColor->data;
//		materialAmbientColor = materialAmbientColor * lightAmbientColor;
//		SetPropertyValue(NMaterial::PARAM_LIGHT_AMBIENT_COLOR, Shader::UT_FLOAT_VEC3, 1, &materialAmbientColor);
//	}
//	
//	NMaterialProperty* propDiffuseColor = GetMaterialProperty(NMaterial::PARAM_PROP_DIFFUSE_COLOR);
//	if(propDiffuseColor)
//	{
//		Color lightDiffuseColor = (light) ? light->GetDiffuseColor() : Color(0, 0, 0, 0);
//		Color materialDiffuseColor = *(Color*) propDiffuseColor->data;
//		materialDiffuseColor = materialDiffuseColor * lightDiffuseColor;
//		SetPropertyValue(NMaterial::PARAM_LIGHT_DIFFUSE_COLOR, Shader::UT_FLOAT_VEC3, 1, &materialDiffuseColor);
//	}
//	
//	NMaterialProperty* propSpecularColor = GetMaterialProperty(NMaterial::PARAM_PROP_SPECULAR_COLOR);
//	if(propSpecularColor)
//	{
//		Color lightSpecularColor = (light) ? light->GetSpecularColor() : Color(0, 0, 0, 0);
//		Color materialSpecularColor = *(Color*) propSpecularColor->data;
//		materialSpecularColor = materialSpecularColor * lightSpecularColor;
//		SetPropertyValue(NMaterial::PARAM_LIGHT_SPECULAR_COLOR, Shader::UT_FLOAT_VEC3, 1, &materialSpecularColor);
//	}
//	
//	float32 intensity = (light) ? light->GetIntensity() : 0;
//	SetPropertyValue(NMaterial::PARAM_LIGHT_INTENSITY0, Shader::UT_FLOAT, 1, &intensity);
//}

//bool NMaterial::IsLightingProperty(const FastName& propName) const
//{
//	return (NMaterial::PARAM_PROP_AMBIENT_COLOR == propName ||
//			NMaterial::PARAM_PROP_DIFFUSE_COLOR == propName ||
//			NMaterial::PARAM_PROP_SPECULAR_COLOR == propName);
//}

const RenderStateData& NMaterial::GetRenderState(const FastName& passName) const
{
	RenderPassInstance* pass = instancePasses.at(passName);
	DVASSERT(pass);
	
	return RenderManager::Instance()->GetRenderStateData(pass->GetRenderStateHandle());
}

void NMaterial::GetRenderState(const FastName& passName, RenderStateData& target) const
{
	RenderPassInstance* pass = instancePasses.at(passName);
	DVASSERT(pass);
	
	RenderManager::Instance()->GetRenderStateData(pass->GetRenderStateHandle(), target);
}

void NMaterial::SubclassRenderState(const FastName& passName, RenderStateData& newState)
{
	RenderPassInstance* pass = instancePasses.at(passName);
	DVASSERT(pass);
	
	if(pass)
	{
		DVASSERT(!pass->dirtyState || (pass->GetRenderStateHandle() == instancePassRenderStates.at(passName)));
		
		UniqueHandle stateHandle = RenderManager::Instance()->CreateRenderState(newState);
		pass->SetRenderStateHandle(stateHandle);
		RenderManager::Instance()->ReleaseRenderState(stateHandle);
		
		pass->dirtyState = true;
		
		if(instancePassRenderStates.count(passName) > 0)
		{
			UniqueHandle currentState = instancePassRenderStates.at(passName);
			
			RenderManager::Instance()->RetainRenderState(stateHandle);
			instancePassRenderStates.insert(passName, stateHandle);
			
			RenderManager::Instance()->ReleaseRenderState(currentState);
		}
		else
		{
			RenderManager::Instance()->RetainRenderState(stateHandle);
			instancePassRenderStates.insert(passName, stateHandle);
		}
	}
}

void NMaterial::SubclassRenderState(RenderStateData& newState)
{
	for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
		it != instancePasses.end();
		++it)
	{
		SubclassRenderState(it->first, newState);
	}
}

void NMaterial::UpdateShaderWithFlags()
{
    requiredVertexFormat = 0;
	if(baseTechnique)
	{
		FastNameSet effectiveFlags(16);
		BuildEffectiveFlagSet(effectiveFlags);
		
		for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
			it != instancePasses.end();
			++it)
		{
			RenderPassInstance* pass = it->second;
			RenderTechniquePass* techniquePass = baseTechnique->GetPassByName(it->first);
			
			Shader* shader = techniquePass->CompileShader(effectiveFlags);
			pass->SetShader(shader);
            requiredVertexFormat |= shader->GetRequiredVertexFormat();
			SafeRelease(shader);
			
            BuildTextureParamsCache(pass);
			BuildActiveUniformsCacheParamsCache(pass);                            
		}
	}

    // updateChildren
    {
        this->Retain();

        size_t childrenCount = children.size();
        for(size_t i = 0; i < childrenCount; ++i)
        {
            children[i]->UpdateShaderWithFlags();
        }

        this->Release();
    }
}

//VI: creates material of type MATERIALTYPE_INSTANCE
//VI: These methods DO NOT add newly created materials to the material system
NMaterial* NMaterial::CreateMaterialInstance()
{
	static int32 instanceCounter = 0;
	instanceCounter++;
	
	NMaterial* mat = new NMaterial();
	mat->SetMaterialType(NMaterial::MATERIALTYPE_INSTANCE);
	mat->SetMaterialName(FastName(Format("Instance-%d", instanceCounter)));
	
	return mat;
}

//VI: creates material of type MATERIALTYPE_MATERIAL
//VI: These methods DO NOT add newly created materials to the material system
NMaterial* NMaterial::CreateMaterial(const FastName& materialName,
									 const FastName& templateName,
									 const FastName& defaultQuality)
{
	NMaterial* mat = new NMaterial();
	mat->SetMaterialType(NMaterial::MATERIALTYPE_MATERIAL);
	mat->SetMaterialName(materialName);
	
	const NMaterialTemplate* matTemplate = NMaterialTemplateCache::Instance()->Get(templateName);
	DVASSERT(matTemplate);
	mat->SetMaterialTemplate(matTemplate, defaultQuality);
	
	return mat;
}

NMaterial* NMaterial::CreateGlobalMaterial(const FastName& materialName)
{
	NMaterial* mat = new NMaterial();
	mat->SetMaterialType(NMaterial::MATERIALTYPE_GLOBAL);
	mat->SetMaterialName(materialName);

	return mat;
}

//VI: creates material of type MATERIALTYPE_INSTANCE
//VI: These methods DO NOT add newly created materials to the material system
NMaterial* NMaterial::CreateMaterialInstance(const FastName& materialName,
											 const FastName& templateName,
											 const FastName& defaultQuality)
{
	NMaterial* parentMat = CreateMaterial(materialName, templateName, defaultQuality);
	
	NMaterial* mat = CreateMaterialInstance();
	mat->SetParent(parentMat);
	
	SafeRelease(parentMat);
	
	return mat;
}

bool NMaterial::IsRuntimeFlag(const FastName& flagName)
{
    return IsNamePartOfArray(flagName, RUNTIME_ONLY_FLAGS);
}

bool NMaterial::IsRuntimeProperty(const FastName& propName)
{
    return IsNamePartOfArray(propName, RUNTIME_ONLY_PROPERTIES);
}

bool NMaterial::IsRuntimeTexture(const FastName& textureName)
{
    return IsNamePartOfArray(textureName, RUNTIME_ONLY_TEXTURES);
}

bool NMaterial::IsNamePartOfArray(const FastName& fastName, const Vector<FastName> & propertyArray)
{
    auto it = std::find_if(propertyArray.begin(), propertyArray.end(), [fastName](const FastName& toFind)
    {
        return fastName == toFind;
    });

    return it != propertyArray.end();
}

void NMaterial::SetMaterialTemplateName(const FastName& templateName)
{
	const NMaterialTemplate* matTemplate = NMaterialTemplateCache::Instance()->Get(templateName);
	DVASSERT(matTemplate);
	
	SetMaterialTemplate(matTemplate, currentQuality);
}

FastName NMaterial::GetMaterialTemplateName() const
{
	return (materialTemplate) ? materialTemplate->name : FastName();
}

};
