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

#include "Render/Material.h"
#include "Render/Texture.h"
#include "FileSystem/KeyedArchive.h"
#include "Utils/StringFormat.h"
#include "Render/Shader.h"
#include "Render/Image/Image.h"
#include "Render/3D/PolygonGroup.h"
#include "Scene3D/DataNode.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFileV2.h"
#include "Render/Highlevel/Light.h"
#include "Render/TextureDescriptor.h"
#include "Platform/SystemTimer.h"
#include "Render/Highlevel/RenderLayer.h"
#include "FileSystem/FileSystem.h"

namespace DAVA 
{

    
InstanceMaterialState::InstanceMaterialState()
    :   lightmapSize(LIGHTMAP_SIZE_DEFAULT)
    ,   flatColor(1.0f, 1.0f, 1.0f, 1.0f)
    ,   texture0Shift(0.0f, 0.0f)
{
    for (int32 k = 0; k < LIGHT_NODE_MAX_COUNT; ++k)
        lightNodes[k] = 0;
    lightmapTexture = 0;
}

InstanceMaterialState::~InstanceMaterialState()
{
    for (int32 k = 0; k < LIGHT_NODE_MAX_COUNT; ++k)
        SafeRelease(lightNodes[k]);
    SafeRelease(lightmapTexture);
}

void InstanceMaterialState::SetLight(int32 lightIndex, Light * lightNode)
{ 
    SafeRelease(lightNodes[lightIndex]);
    lightNodes[lightIndex] = SafeRetain(lightNode); 
}

Light * InstanceMaterialState::GetLight(int32 lightIndex) 
{ 
    return lightNodes[lightIndex]; 
}

void InstanceMaterialState::SetLightmap(Texture * _lightMapTexture, const FilePath & _lightmapName)
{
    SafeRelease(lightmapTexture);
    lightmapTexture = SafeRetain(_lightMapTexture);
    lightmapName = _lightmapName;
}

void InstanceMaterialState::SetUVOffsetScale(const Vector2 & _uvOffset, const Vector2 _uvScale)
{
    uvOffset = _uvOffset;
    uvScale = _uvScale;
}

int32 InstanceMaterialState::GetLightmapSize()
{
	return lightmapSize;
}

void InstanceMaterialState::SetLightmapSize(int32 size)
{
	lightmapSize = size;
}

void InstanceMaterialState::ClearLightmap()
{
	SafeRelease(lightmapTexture);
	lightmapName = String("");
}

    
void InstanceMaterialState::SetFlatColor(const Color & color)
{
    flatColor = color;
}

const Color & InstanceMaterialState::GetFlatColor()
{
    return flatColor;
}


void InstanceMaterialState::SetTextureShift(const Vector2 & speed)
{
    texture0Shift = speed;
}

const Vector2 & InstanceMaterialState::GetTextureShift()
{
    return texture0Shift;
}

void InstanceMaterialState::Save(KeyedArchive * archive, SerializationContext *serializationContext)
{
	if(NULL != archive)
	{
		if(uvOffset != Vector2())
		{
			archive->SetVector2("ims.uvoffset", uvOffset);
		}
        
		if(uvScale != Vector2())
		{
			archive->SetVector2("ims.uvscale", uvScale);
		}
		
		if(!lightmapName.IsEmpty())
		{
            String filename = lightmapName.GetRelativePathname(serializationContext->GetScenePath());
            archive->SetString("ims.lightmapname", filename);
		}
		
		if(lightmapSize != LIGHTMAP_SIZE_DEFAULT)
		{
			archive->SetInt32("ims.lightmapsize", lightmapSize);
		}

		if(flatColor != Color::White)
		{
			archive->SetByteArrayAsType("ims.flatColor", flatColor);
		}
		
		if(texture0Shift != Vector2())
		{
			archive->SetVector2("ims.texture0Shift", texture0Shift);
		}
	}
}

void InstanceMaterialState::Load(KeyedArchive * archive, SerializationContext *serializationContext)
{
	if(NULL != archive)
	{
		uvOffset = archive->GetVector2("ims.uvoffset");
		uvScale = archive->GetVector2("ims.uvscale");

		String filename = archive->GetString("ims.lightmapname");
		if(!filename.empty())
		{
            FilePath lName = serializationContext->GetScenePath() + filename;

			Texture* lTexture = Texture::CreateFromFile(lName, FastName("albedo"));
			SetLightmap(lTexture, lName);
			lTexture->Release();
		}
        else
        {
			SetLightmap(NULL, String(""));
        }

		flatColor = archive->GetByteArrayAsType("ims.flatColor", Color::White);
		texture0Shift = archive->GetVector2("ims.texture0Shift");
		lightmapSize = archive->GetInt32("ims.lightmapsize", LIGHTMAP_SIZE_DEFAULT);
	}
}

InstanceMaterialState * InstanceMaterialState::Clone()
{
	InstanceMaterialState * newState = new InstanceMaterialState();
    newState->InitFromState(this);
    
	return newState;
}
    
void InstanceMaterialState::InitFromState(const InstanceMaterialState * state)
{
    lightmapTexture = SafeRetain(state->lightmapTexture);
	lightmapName = state->lightmapName;
	lightmapSize = state->lightmapSize;
	uvOffset = state->uvOffset;
	uvScale = state->uvScale;
	flatColor = state->flatColor;
	texture0Shift = state->texture0Shift;
}
   
    
const char8 * Material::GetTypeName(eType format)
{
    switch(format)
    {
        case MATERIAL_UNLIT_TEXTURE:
            return "UNLIT_TEXTURE";
        case MATERIAL_UNLIT_TEXTURE_DETAIL:
            return "UNLIT_TEXTURE_DETAIL";
        case MATERIAL_UNLIT_TEXTURE_DECAL:
            return "UNLIT_TEXTURE_DECAL";
        case MATERIAL_UNLIT_TEXTURE_LIGHTMAP:
            return "UNLIT_TEXTURE_LIGHTMAP";
        case MATERIAL_VERTEX_LIT_TEXTURE:
            return "VERTEX_LIT_TEXTURE";
        case MATERIAL_VERTEX_LIT_DETAIL:
            return "VERTEX_LIT_DETAIL";
        case MATERIAL_VERTEX_LIT_DECAL:
            return "VERTEX_LIT_DECAL";
        case MATERIAL_VERTEX_LIT_LIGHTMAP:
            return "VERTEX_LIT_LIGHTMAP";
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE:
            return "PIXEL_LIT_NORMAL_DIFFUSE";
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR:
            return "PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR";
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP:
            return "PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP";
		case MATERIAL_VERTEX_COLOR_ALPHABLENDED:
			return "VERTEX_COLOR_ALPHABLEND";
		case MATERIAL_VERTEX_COLOR_ALPHABLENDED_FRAME_BLEND:
			return "VERTEX_COLOR_ALPHABLEND_FRAME_BLEND";
		case MATERIAL_FLAT_COLOR:
			return "FLAT_COLOR";
		case MATERIAL_SKYBOX:
			return "SKYBOX";
        case MATERIAL_SPEED_TREE_LEAF:
            return "MATERIAL_SPEED_TREE_LEAF";
        default:
            break;
    };
    return "WRONG MATERIAL";
}

Material::Material() 
    :   DataNode()
    ,	viewOptions(MATERIAL_VIEW_TEXTURE_LIGHTMAP)
    ,   blending(BLENDING_ALPHABLEND)
    ,   isTranslucent(false)
    ,   isTwoSided(false)
    ,	isSetupLightmap(false)
    ,   shininess(1.0f)
    ,   ambientColor(0.2f, 0.2f, 0.2f, 1.0f)
    ,   diffuseColor(0.8f, 0.8f, 0.8f, 1.0f)
    ,   specularColor(0.0f, 0.0f, 0.0f, 1.0f)
    ,   emissiveColor(0.0f, 0.0f, 0.0f, 1.0f)
    ,   treeLeafColor(1.f, 1.f, 1.f, 1.f)
    ,   treeLeafOcclusionOffset(0.f)
    ,   treeLeafOcclusionMul(1.f)
    ,   isFogEnabled(false)
    ,   fogDensity(0.006f)
    ,   fogColor((float32)0x87 / 255.0f, (float32)0xbe / 255.0f, (float32)0xd7 / 255.0f, 1.0f)
    ,	lightingParams(0)
	,	isAlphablend(false)
    ,   isFlatColorEnabled(false)
    ,   isTexture0ShiftEnabled(false)
    ,   isWireframe(false)
    //,   renderStateBlock()
    ,   isExportOwnerLayerEnabled(true)
    ,   ownerLayerName(RenderLayer::GetLayerNameByID(RenderLayer::RENDER_LAYER_AFTER_OPAQUE_ID))
{
    //Reserve memory for Collection
    names.resize(TEXTURE_COUNT);
    
//    if (scene)
//    {
//        DataNode * materialsNode = scene->GetMaterials();
//        materialsNode->AddNode(this);
//    }
    

    
//    type = MATERIAL_UNLIT_TEXTURE;
//    shader = uberShader->GetShader("MATERIAL_TEXTURE");
    for (int32 tc = 0; tc < TEXTURE_COUNT; ++tc)
        textures[tc] = 0;

    SetType(MATERIAL_UNLIT_TEXTURE);
}

Material * Material::Clone(Material *newMaterial /* = NULL */)
{
    if(!newMaterial)
    {
		DVASSERT_MSG(IsPointerToExactClass<Material>(this), "Can clone only Material");
        
        newMaterial = new Material();
    }
    
    newMaterial->pointer = pointer;
    newMaterial->scene = scene;
    newMaterial->index = index;

	newMaterial->CopySettings(this);

    newMaterial->reflective = reflective;
    newMaterial->reflectivity =	reflectivity;	

	newMaterial->uniformTexture0 = uniformTexture0;
	newMaterial->uniformTexture1 = uniformTexture1;
	newMaterial->uniformLightPosition0 = uniformLightPosition0;
	newMaterial->uniformMaterialLightAmbientColor = uniformMaterialLightAmbientColor;
	newMaterial->uniformMaterialLightDiffuseColor = uniformMaterialLightDiffuseColor;
	newMaterial->uniformMaterialLightSpecularColor = uniformMaterialLightSpecularColor;
	newMaterial->uniformMaterialSpecularShininess = uniformMaterialSpecularShininess;
	newMaterial->uniformLightIntensity0 = uniformLightIntensity0;
	newMaterial->uniformLightAttenuationQ = uniformLightAttenuationQ;
	newMaterial->uniformUvOffset = uniformUvOffset;
	newMaterial->uniformUvScale = uniformUvScale;
	newMaterial->uniformFogDensity = uniformFogDensity;
	newMaterial->uniformFogColor = uniformFogColor;
	newMaterial->uniformFlatColor = uniformFlatColor;
    newMaterial->uniformTexture0Shift = uniformTexture0Shift;
    newMaterial->uniformWorldTranslate = uniformWorldTranslate;
    newMaterial->uniformWorldScale = uniformWorldScale;
    newMaterial->uniformTreeLeafColorMul = uniformTreeLeafColorMul;
    newMaterial->uniformTreeLeafOcclusionOffset = uniformTreeLeafOcclusionOffset;

	//renderStateBlock.CopyTo(&newMaterial->renderStateBlock);
    

    newMaterial->isExportOwnerLayerEnabled = isExportOwnerLayerEnabled;
    newMaterial->ownerLayerName = ownerLayerName;

    return newMaterial;
}

void Material::CopySettings(Material *fromMaterial)
{
	DVASSERT(fromMaterial);

	type = fromMaterial->type;
	viewOptions = fromMaterial->viewOptions;

	reflective = fromMaterial->reflective;
	reflectivity =	fromMaterial->reflectivity;

	transparent = fromMaterial->transparent;
	transparency =	fromMaterial->transparency; 
	indexOfRefraction = fromMaterial->indexOfRefraction;

	for(int i = 0; i < TEXTURE_COUNT; i++)
	{
		SafeRelease(textures[i]);

		textures[i] = SafeRetain(fromMaterial->textures[i]);
		textureSlotNames[i] = fromMaterial->textureSlotNames[i];
	}

	names = fromMaterial->names;

	textureSlotCount = fromMaterial->textureSlotCount;

    blending = fromMaterial->blending;

	isTranslucent = fromMaterial->isTranslucent;
	isTwoSided = fromMaterial->isTwoSided;

	isSetupLightmap = fromMaterial->isSetupLightmap;

	shininess = fromMaterial->shininess;

	ambientColor = fromMaterial->ambientColor;
	diffuseColor = fromMaterial->diffuseColor;
	specularColor = fromMaterial->specularColor;
	emissiveColor = fromMaterial->emissiveColor;
    treeLeafColor = fromMaterial->treeLeafColor;

    treeLeafOcclusionMul = fromMaterial->treeLeafOcclusionMul;
    treeLeafOcclusionOffset = fromMaterial->treeLeafOcclusionOffset;

	isFogEnabled = fromMaterial->isFogEnabled;
	fogDensity = fromMaterial->fogDensity;
	fogColor = fromMaterial->fogColor;

	if(fromMaterial->lightingParams)
	{
		SafeDelete(lightingParams);

		lightingParams = new StaticLightingParams();
		lightingParams->transparencyColor = fromMaterial->lightingParams->transparencyColor;
	}

	isAlphablend = fromMaterial->isAlphablend;
	isFlatColorEnabled = fromMaterial->isFlatColorEnabled;

	isTexture0ShiftEnabled = fromMaterial->isTexture0ShiftEnabled;

	isWireframe = fromMaterial->isWireframe;
}


void Material::SetScene(Scene * _scene)
{
    DVASSERT(scene == 0);
    scene = _scene;
}


int32 Material::Release()
{
    int32 retainCount = BaseObject::Release();
    return retainCount;
}

Material::~Material()
{
    for (int32 tc = 0; tc < TEXTURE_COUNT; ++tc)
    {
        SafeRelease(textures[tc]);
    }
	SafeDelete(lightingParams);
}
    
    
Material::eValidationResult Material::Validate(PolygonGroup * polygonGroup)
{
#if RHI_COMPLETE
    RebuildShader();
	
	if(Material::MATERIAL_SKYBOX == type)
	{
		return VALIDATE_COMPATIBLE;
	}
	
    /*
        General check if number of attributes in shader is 
     */
    int32 format = polygonGroup->GetFormat();
    int32 formatBitCount = 0; // number of attributes in polygroup available in the shader
    for (int bit = 1; bit <= EVF_HIGHER_BIT; bit <<= 1)
    {
        if (format & bit)
        {
            if (shader->GetAttributeIndex((eVertexFormat)bit) != -1)
                formatBitCount++;
        }
    }
    // Check if we have more attributes 
    if (shader->GetAttributeCount() > formatBitCount)
    {
        //shader->Get
        
        
        
        return VALIDATE_INCOMPATIBLE;
    }
    
	if(lightingParams == 0)
	{
		lightingParams = new StaticLightingParams();
	}
#endif //RHI_COMPLETE
    return VALIDATE_COMPATIBLE;
}

void Material::RebuildShader()
{
    uniformTexture0 = -1;
    uniformTexture1 = -1;
    uniformLightPosition0 = -1;
    uniformMaterialLightAmbientColor = -1;
    uniformMaterialLightDiffuseColor = -1;
    uniformMaterialLightSpecularColor = -1;
    uniformMaterialSpecularShininess = -1;
    uniformLightIntensity0 = -1;
    uniformLightAttenuationQ = -1;
    uniformUvOffset = -1;
    uniformUvScale = -1;
    uniformFogDensity = -1;
    uniformFogColor = -1;
    uniformFlatColor = -1;
    uniformTexture0Shift = -1;
    uniformWorldTranslate = -1;
    uniformWorldScale = -1;
    uniformTreeLeafColorMul = -1;
    uniformTreeLeafOcclusionOffset = -1;
    
    //VI: don't build old shader for new materials
    
    /*
    String shaderCombileCombo = "";
    
    switch (type)
    {
        case MATERIAL_UNLIT_TEXTURE:
            shaderCombileCombo = "MATERIAL_TEXTURE";
            break;
        case MATERIAL_UNLIT_TEXTURE_LIGHTMAP:
			shaderCombileCombo = "MATERIAL_TEXTURE; MATERIAL_LIGHTMAP";
			if(isSetupLightmap)
			{
				shaderCombileCombo = shaderCombileCombo + ";SETUP_LIGHTMAP";
			}
			break;
        case MATERIAL_UNLIT_TEXTURE_DECAL:
            shaderCombileCombo = "MATERIAL_TEXTURE;MATERIAL_DECAL";
            break;
        case MATERIAL_UNLIT_TEXTURE_DETAIL:
            shaderCombileCombo = "MATERIAL_TEXTURE;MATERIAL_DETAIL";
            break;
        case MATERIAL_VERTEX_LIT_TEXTURE:
            shaderCombileCombo = "MATERIAL_TEXTURE;VERTEX_LIT";
            break;
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE:
            shaderCombileCombo = "MATERIAL_TEXTURE;PIXEL_LIT;DIFFUSE";
            break;
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR:
            shaderCombileCombo = "MATERIAL_TEXTURE;PIXEL_LIT;DIFFUSE;SPECULAR";
            break;
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP:
            shaderCombileCombo = "MATERIAL_TEXTURE;PIXEL_LIT;DIFFUSE;SPECULAR;GLOSS";
            break;
		case MATERIAL_VERTEX_COLOR_ALPHABLENDED:
			shaderCombileCombo = "MATERIAL_TEXTURE;ALPHABLEND;VERTEX_COLOR";
			break;
		case MATERIAL_VERTEX_COLOR_ALPHABLENDED_FRAME_BLEND:
			shaderCombileCombo = "MATERIAL_TEXTURE;ALPHABLEND;VERTEX_COLOR;FRAME_BLEND";
			break;
        case MATERIAL_FLAT_COLOR:
            isFlatColorEnabled = true;
            break;
		case MATERIAL_SKYBOX:
		{
			shaderCombileCombo = "MATERIAL_SKYBOX";
			isFlatColorEnabled = true;
			break;
		}
        case MATERIAL_SPEED_TREE_LEAF:
            shaderCombileCombo = "MATERIAL_SPEED_TREE_LEAF;MATERIAL_TEXTURE;VERTEX_COLOR";
			break;
        default:
            break;
    };
	 
	switch (viewOptions)
	{
		case MATERIAL_VIEW_TEXTURE_LIGHTMAP:
			break;
		case MATERIAL_VIEW_LIGHTMAP_ONLY:
			if (shaderCombileCombo.size() > 0)shaderCombileCombo += ";";
			shaderCombileCombo = shaderCombileCombo + "MATERIAL_VIEW_LIGHTMAP_ONLY";
			break;
		case MATERIAL_VIEW_TEXTURE_ONLY:
			if (shaderCombileCombo.size() > 0)shaderCombileCombo += ";";
			shaderCombileCombo = shaderCombileCombo + "MATERIAL_VIEW_TEXTURE_ONLY";
			break;
		default:
			break;
	}

    if (isTranslucent)
    {
        if (shaderCombileCombo.size() > 0)shaderCombileCombo += ";";
        shaderCombileCombo = shaderCombileCombo + "ALPHATEST";
    }

	if(isAlphablend)
	{
        if (shaderCombileCombo.size() > 0)shaderCombileCombo += ";";
		shaderCombileCombo = shaderCombileCombo + "ALPHABLEND";
	}
    
    if (isFlatColorEnabled)
    {
        if (shaderCombileCombo.size() > 0)shaderCombileCombo += ";";
        shaderCombileCombo = shaderCombileCombo + "FLATCOLOR";
    }
    
    if (isTexture0ShiftEnabled)
    {
        if (shaderCombileCombo.size() > 0)shaderCombileCombo += ";";
        shaderCombileCombo = shaderCombileCombo + "TEXTURE0_SHIFT_ENABLED";
    }
    
    
    //if (isDistanceAttenuation)
    //shaderCombileCombo = shaderCombileCombo + ";DISTANCE_ATTENUATION";
    
    if (isFogEnabled && Renderer::GetOptions()->IsOptionEnabled(RenderOptions::FOG_ENABLE))
        shaderCombileCombo = shaderCombileCombo + ";VERTEX_FOG";

    // Get shader if combo unavailable compile it
    
    
    shader = uberShader->GetShader(shaderCombileCombo);
    
    switch (type) {
        case MATERIAL_UNLIT_TEXTURE:
            break;
        case MATERIAL_UNLIT_TEXTURE_LIGHTMAP:
        case MATERIAL_UNLIT_TEXTURE_DECAL:
        case MATERIAL_UNLIT_TEXTURE_DETAIL:
            uniformTexture0 = shader->FindUniformIndexByName(FastName("texture0"));
            uniformTexture1 = shader->FindUniformIndexByName(FastName("texture1"));
            uniformUvOffset = shader->FindUniformIndexByName(FastName("uvOffset"));
            uniformUvScale = shader->FindUniformIndexByName(FastName("uvScale"));
            
            break;
        case MATERIAL_VERTEX_LIT_TEXTURE:
            //
            uniformLightPosition0 = shader->FindUniformIndexByName(FastName("lightPosition0"));
            uniformMaterialLightAmbientColor = shader->FindUniformIndexByName(FastName("materialLightAmbientColor"));
            uniformMaterialLightDiffuseColor = shader->FindUniformIndexByName(FastName("materialLightDiffuseColor"));
            uniformMaterialLightSpecularColor = shader->FindUniformIndexByName(FastName("materialLightSpecularColor"));
            uniformMaterialSpecularShininess = shader->FindUniformIndexByName(FastName("materialSpecularShininess"));
            uniformLightIntensity0 = shader->FindUniformIndexByName(FastName("lightIntensity0"));
            uniformLightAttenuationQ = shader->FindUniformIndexByName(FastName("uniformLightAttenuationQ"));
            
            break;
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE:
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR:
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP:
            uniformTexture0 = shader->FindUniformIndexByName(FastName("texture0"));
            uniformTexture1 = shader->FindUniformIndexByName(FastName("normalMapTexture"));

            uniformLightPosition0 = shader->FindUniformIndexByName(FastName("lightPosition0"));
            uniformMaterialLightAmbientColor = shader->FindUniformIndexByName(FastName("materialLightAmbientColor"));
            uniformMaterialLightDiffuseColor = shader->FindUniformIndexByName(FastName("materialLightDiffuseColor"));
            uniformMaterialLightSpecularColor = shader->FindUniformIndexByName(FastName("materialLightSpecularColor"));
            uniformMaterialSpecularShininess = shader->FindUniformIndexByName(FastName("materialSpecularShininess"));
            uniformLightIntensity0 = shader->FindUniformIndexByName(FastName("lightIntensity0"));
            uniformLightAttenuationQ = shader->FindUniformIndexByName(FastName("uniformLightAttenuationQ"));
            break;
        case MATERIAL_SPEED_TREE_LEAF:
            uniformWorldTranslate = shader->FindUniformIndexByName(FastName("worldTranslate"));
            uniformTexture0 = shader->FindUniformIndexByName(FastName("texture0"));
            uniformWorldScale = shader->FindUniformIndexByName(FastName("worldScale"));
            uniformTreeLeafColorMul = shader->FindUniformIndexByName(FastName("treeLeafColorMul"));
            uniformTreeLeafOcclusionOffset = shader->FindUniformIndexByName(FastName("treeLeafOcclusionOffset"));
            break;
        default:
            break;
    };
    
    if (isFogEnabled)
    {
        uniformFogDensity = shader->FindUniformIndexByName(FastName("fogDensity"));
        uniformFogColor = shader->FindUniformIndexByName(FastName("fogColor"));
        DVASSERT(uniformFogDensity != -1);
        DVASSERT(uniformFogColor != -1);
    }
    
    if (isFlatColorEnabled)
    {
        uniformFlatColor = shader->FindUniformIndexByName(FastName("flatColor"));
        DVASSERT(uniformFlatColor != -1);
    }
    
    if (isTexture0ShiftEnabled)
    {
        uniformTexture0Shift = shader->FindUniformIndexByName(FastName("texture0Shift"));
        DVASSERT(uniformTexture0Shift != -1);
    }
    
    


    //RetrieveTextureSlotNames();
    */
}

void Material::RetrieveTextureSlotNames()
{
#if RHI_COMPLETE
    // 
    //shader->F
    textureSlotCount = 0;
    for (int32  k = 0; k < shader->GetUniformCount(); ++k)
    {
        if (shader->GetUniformType(k) == Shader::UT_SAMPLER_2D)
        {
            textureSlotNames[textureSlotCount] = shader->GetUniformName(k);
            textureSlotCount++;
        }
    }
#endif RHI_COMPLETE
}

    
uint32 Material::GetTextureSlotCount() const
{
    return textureSlotCount;
}
    
const String & Material::GetTextureSlotName(uint32 index) const
{
    return textureSlotNames[index];
}
    
uint32 Material::GetTextureSlotIndexByName(const String & string) const
{
    return -1;
}

void Material::SetType(eType _type)
{
    type = _type;
    RebuildShader();
}
    
void Material::Save(KeyedArchive * keyedArchive, SerializationContext * serializationContext)
{
    DataNode::Save(keyedArchive, serializationContext);
    
    keyedArchive->SetInt32("mat.texCount", TEXTURE_COUNT);
    for (int32 k = 0; k < TEXTURE_COUNT; ++k)
    {
        if (!names[k].IsEmpty())
        {
            String filename = names[k].GetRelativePathname(serializationContext->GetScenePath());
            keyedArchive->SetString(Format("mat.tex%d", k), filename);
            
            if(serializationContext->IsDebugLogEnabled())
                Logger::FrameworkDebug("--- save material texture: %s", filename.c_str());
        }
    }

    keyedArchive->SetByteArrayAsType("mat.diffuse", diffuseColor);
    keyedArchive->SetByteArrayAsType("mat.ambient", ambientColor);
    keyedArchive->SetByteArrayAsType("mat.specular", specularColor);
    keyedArchive->SetByteArrayAsType("mat.emission", emissiveColor);
    keyedArchive->SetFloat("mat.shininess", shininess);

    keyedArchive->SetByteArrayAsType("mat.tree.leaf.color", treeLeafColor);
    keyedArchive->SetFloat("mat.tree.leaf.occlusion.offset", treeLeafOcclusionOffset);
    keyedArchive->SetFloat("mat.tree.leaf.occlusion.mul", treeLeafOcclusionMul);

    keyedArchive->SetBool("mat.isOpaque", isTranslucent);
    keyedArchive->SetBool("mat.isTwoSided", isTwoSided);

	keyedArchive->SetBool("mat.isAlphablend", isAlphablend);
    keyedArchive->SetInt32("mat.blend", blending);	

    keyedArchive->SetInt32("mat.type", type);
    
    keyedArchive->SetByteArrayAsType("mat.fogcolor", fogColor);
    keyedArchive->SetFloat("mat.fogdencity", fogDensity);
    keyedArchive->SetBool("mat.isFogEnabled", isFogEnabled);

	keyedArchive->SetBool("mat.isFlatColorEnabled", isFlatColorEnabled);
	keyedArchive->SetBool("mat.isTexture0ShiftEnabled", isTexture0ShiftEnabled);

	if(lightingParams)
	{
		keyedArchive->SetByteArrayAsType("mat.staticTransparencyColor", lightingParams->transparencyColor);
	}
}

void Material::Load(KeyedArchive * keyedArchive, SerializationContext * serializationContext)
{
    DataNode::Load(keyedArchive, serializationContext);

	eType mtype = (eType)keyedArchive->GetInt32("mat.type", type);
    
    name = keyedArchive->GetString("name");

    int32 texCount = keyedArchive->GetInt32("mat.texCount");
    for (int32 k = 0; k < texCount; ++k)
    {
		if(mtype == MATERIAL_UNLIT_TEXTURE_LIGHTMAP && k == TEXTURE_LIGHTMAP)
		{
			continue;
		}

        String relativePathname = keyedArchive->GetString(Format("mat.tex%d", k));
        if (!relativePathname.empty())
        {
            if(relativePathname[0] == '~') //path like ~res:/Gfx...
            {
                names[k] = FilePath(relativePathname);
            }
            else
            {
                names[k] = serializationContext->GetScenePath() + relativePathname;
            }
            
            if(serializationContext->IsDebugLogEnabled())
            	Logger::FrameworkDebug("--- load material texture: %s src:%s", relativePathname.c_str(), names[k].GetAbsolutePathname().c_str());
            
            //textures[k] = Texture::CreateFromFile(names[k].GetAbsolutePath());
            textures[k] = Texture::CreateFromFile(names[k], FastName("albedo"));
        }
    }
    
    
    diffuseColor = keyedArchive->GetByteArrayAsType("mat.diffuse", diffuseColor);
    ambientColor = keyedArchive->GetByteArrayAsType("mat.ambient", ambientColor);
    specularColor = keyedArchive->GetByteArrayAsType("mat.specular", specularColor);
    emissiveColor = keyedArchive->GetByteArrayAsType("mat.emission", emissiveColor);
    shininess = keyedArchive->GetFloat("mat.shininess", shininess);

    treeLeafColor = keyedArchive->GetByteArrayAsType("mat.tree.leaf.color", treeLeafColor);
    treeLeafOcclusionOffset = keyedArchive->GetFloat("mat.tree.leaf.occlusion.offset");
    treeLeafOcclusionMul = keyedArchive->GetFloat("mat.tree.leaf.occlusion.mul");

    isTranslucent = keyedArchive->GetBool("mat.isOpaque", isTranslucent);
    isTwoSided = keyedArchive->GetBool("mat.isTwoSided", isTwoSided);

	isAlphablend = keyedArchive->GetBool("mat.isAlphablend", isAlphablend);
    blending = isAlphablend ? BLENDING_ALPHABLEND : BLENDING_NONE;
	

	fogColor = keyedArchive->GetByteArrayAsType("mat.fogcolor", fogColor);
	isFogEnabled = keyedArchive->GetBool("mat.isFogEnabled", isFogEnabled);
	fogDensity = keyedArchive->GetFloat("mat.fogdencity", fogDensity);

	isFlatColorEnabled = keyedArchive->GetBool("mat.isFlatColorEnabled", isFlatColorEnabled);
	isTexture0ShiftEnabled = keyedArchive->GetBool("mat.isTexture0ShiftEnabled", isTexture0ShiftEnabled);

    SetType(mtype);

	if(keyedArchive->IsKeyExists("mat.staticTransparencyColor"))
	{
		if(lightingParams == 0)
			lightingParams = new StaticLightingParams();

		lightingParams->transparencyColor = keyedArchive->GetByteArrayAsType("mat.staticTransparencyColor", Color(0, 0, 0, 0));
	}
}

void Material::SetAlphatest(bool alphatest)
{
    isTranslucent = alphatest;
    RebuildShader();
}

bool Material::GetAlphatest()
{
    return isTranslucent;
}
void Material::SetTwoSided(bool _isTwoSided)
{
    isTwoSided = _isTwoSided;
}
    
bool Material::GetTwoSided()
{
    return isTwoSided;
}
    
void Material::SetAmbientColor(const Color & color)
{
    ambientColor = color;
}
void Material::SetDiffuseColor(const Color & color)
{
    diffuseColor = color;
}
void Material::SetSpecularColor(const Color & color)
{
    specularColor = color;
}
void Material::SetEmissiveColor(const Color & color)
{
    emissiveColor = color;
}

const Color & Material::GetAmbientColor() const
{
    return ambientColor;
}
const Color & Material::GetDiffuseColor() const
{
    return diffuseColor;
}
const Color & Material::GetSpecularColor() const
{
    return specularColor;
}
const Color & Material::GetEmissiveColor() const
{
    return emissiveColor;
}
    
void Material::SetShininess(float32 _shininess)
{
    shininess = _shininess;
}

float32 Material::GetShininess() const
{
    return shininess;
}
    
void Material::SetFog(const bool & _isFogEnabled)
{
    isFogEnabled = _isFogEnabled;
    RebuildShader();
}

void Material::SetViewOption(eViewOptions option)
{
	if(viewOptions != option)
	{
		viewOptions = option;
		RebuildShader();
	}
}

Material::eViewOptions Material::GetViewOption()
{
	return viewOptions;
}
    
const bool & Material::IsFogEnabled() const
{
    return isFogEnabled;
}
    
void Material::SetFogDensity(float32 _fogDensity)
{
    fogDensity = _fogDensity;
}
    
float32 Material::GetFogDensity() const
{
    return fogDensity;
}

void Material::SetFogColor(const Color & _fogColor)
{
    fogColor = _fogColor;
}

const Color & Material::GetFogColor() const
{
    return fogColor;
}

void Material::PrepareRenderState(InstanceMaterialState * instanceMaterialState, Matrix4 * worldMxPtr)
{
    ///float32 timeElapsed = SystemTimer::Instance()->FrameDelta();

	/*
    if(MATERIAL_UNLIT_TEXTURE_LIGHTMAP == type)
	{
        if (NULL == instanceMaterialState->lightmapTexture)
        {
            SetSetupLightmap(true);
			renderStateBlock.SetTexture(NULL, 1);
        }
		else
        {
            SetSetupLightmap(false);
            renderStateBlock.SetTexture(instanceMaterialState->lightmapTexture, 1);
        }
    }
	else if (MATERIAL_UNLIT_TEXTURE_DECAL == type || MATERIAL_UNLIT_TEXTURE_DETAIL == type)
    {
        renderStateBlock.SetTexture(textures[Material::TEXTURE_DECAL], 1);
    }
    
	renderStateBlock.shader = shader;

	if (textures[Material::TEXTURE_DIFFUSE])
	{
		renderStateBlock.SetTexture(textures[Material::TEXTURE_DIFFUSE], 0);
	}


	if(MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE == type || MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR == type || MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP == type)
	{
		if (textures[Material::TEXTURE_NORMALMAP])
		{
			renderStateBlock.SetTexture(textures[Material::TEXTURE_NORMALMAP], 1);
		}
	}
	else
	{
//		if (textures[Material::TEXTURE_DECAL])
//		{
//			renderStateBlock.SetTexture(textures[Material::TEXTURE_DECAL], 1);
//		}
	}

	if (isTranslucent || isTwoSided)
	{
		renderStateBlock.state &= ~RenderState::STATE_CULL;
	}
	else
	{
		renderStateBlock.state |= RenderState::STATE_CULL;
	}


	if(isAlphablend)
	{
		renderStateBlock.state |= RenderState::STATE_BLEND;
		//Dizz: temporary solution
		renderStateBlock.state &= ~RenderState::STATE_DEPTH_WRITE;

		renderStateBlock.SetBlendMode((eBlendMode)blendSrc, (eBlendMode)blendDst);
	}
	else
	{
		//Dizz: temporary solution
		renderStateBlock.state |= RenderState::STATE_DEPTH_WRITE;
		renderStateBlock.state &= ~RenderState::STATE_BLEND;
	}

	if(isWireframe)
	{
		renderStateBlock.SetFillMode(FILLMODE_WIREFRAME);
	}
	else
	{
		renderStateBlock.SetFillMode(FILLMODE_SOLID);
	}

	// render
	RenderManager::Instance()->FlushState(&renderStateBlock);
	RenderManager::Instance()->AttachRenderData();


    if(uniformTexture0 != -1)
    {
        shader->SetUniformValueByIndex(uniformTexture0, 0);
    }
    if(uniformTexture1 != -1)
    {
        shader->SetUniformValueByIndex(uniformTexture1, 1);
    }

	if(isSetupLightmap)
	{
		int32 lightmapSizePosition = shader->FindUniformIndexByName(FastName("lightmapSize"));
		if (lightmapSizePosition != -1)
		{
			shader->SetUniformValueByIndex(lightmapSizePosition, (float32)instanceMaterialState->GetLightmapSize()); 
		}
	}

	if(MATERIAL_UNLIT_TEXTURE_LIGHTMAP == type)
	{
		if (uniformUvOffset != -1)
			shader->SetUniformValueByIndex(uniformUvOffset, instanceMaterialState->uvOffset);
		if (uniformUvScale != -1)
			shader->SetUniformValueByIndex(uniformUvScale, instanceMaterialState->uvScale);
	}

	if (isFogEnabled)
	{
		DVASSERT(uniformFogDensity != -1);
        shader->SetUniformValueByIndex(uniformFogDensity, fogDensity);
		
        DVASSERT(uniformFogColor != -1)
        shader->SetUniformColor3ByIndex(uniformFogColor, fogColor);
	}
    
    if(MATERIAL_SPEED_TREE_LEAF == type)
    {
        if(uniformWorldTranslate != -1)
        {
            Vector3 trVector = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW).GetTranslationVector();
            shader->SetUniformValueByIndex(uniformWorldTranslate, trVector);
        }

        if(uniformWorldScale != -1 && worldMxPtr)
        {
            shader->SetUniformValueByIndex(uniformWorldScale, worldMxPtr->GetScaleVector());
        }
        if(uniformTreeLeafColorMul != -1)
        {
            shader->SetUniformColor3ByIndex(uniformTreeLeafColorMul, treeLeafColor * treeLeafOcclusionMul);
        }
        if(uniformTreeLeafOcclusionOffset != -1)
        {
            shader->SetUniformValueByIndex(uniformTreeLeafOcclusionOffset, treeLeafOcclusionOffset);
        }
    }

    if (instanceMaterialState)
    {
        if (isFlatColorEnabled)
        {
            DVASSERT(uniformFlatColor != -1);
            shader->SetUniformColor4ByIndex(uniformFlatColor, instanceMaterialState->flatColor);
        }
        if (isTexture0ShiftEnabled)
        {
            DVASSERT(uniformTexture0Shift != -1);
            shader->SetUniformValueByIndex(uniformTexture0Shift, instanceMaterialState->texture0Shift);
        }

		if(scene)
		{
			Camera * camera = scene->GetCurrentCamera();
			Light * lightNode0 = instanceMaterialState->GetLight(0);
			if (lightNode0 && camera)
			{
				if (uniformLightPosition0 != -1)
				{
					const Matrix4 & matrix = camera->GetMatrix();
					Vector3 lightPosition0InCameraSpace = lightNode0->GetPosition() * matrix;
                
					shader->SetUniformValueByIndex(uniformLightPosition0, lightPosition0InCameraSpace);
				}
				if (uniformMaterialLightAmbientColor != -1)
				{
					shader->SetUniformColor3ByIndex(uniformMaterialLightAmbientColor, lightNode0->GetAmbientColor() * GetAmbientColor());
				}
				if (uniformMaterialLightDiffuseColor != -1)
				{
					shader->SetUniformColor3ByIndex(uniformMaterialLightDiffuseColor, lightNode0->GetDiffuseColor() * GetDiffuseColor());
				}
				if (uniformMaterialLightSpecularColor != -1)
				{
					shader->SetUniformColor3ByIndex(uniformMaterialLightSpecularColor, lightNode0->GetSpecularColor() * GetSpecularColor());
				}
				if (uniformMaterialSpecularShininess != -1)
				{
					shader->SetUniformValueByIndex(uniformMaterialSpecularShininess, shininess);
				}
            
				if (uniformLightIntensity0 != -1)
				{
					shader->SetUniformValueByIndex(uniformLightIntensity0, lightNode0->GetIntensity());
				}
				if (uniformLightAttenuationQ != -1)
				{
					//shader->SetUniformValue(uniformLightAttenuationQ, lightNode0->GetAttenuation());
				}
			}
		}
    }
	 */
}

void Material::Draw(PolygonGroup * group, InstanceMaterialState * instanceMaterialState, Matrix4 * worldMxPtr)
{
	/*if(isTranslucent && !Renderer::GetOptions()->IsOptionEnabled(RenderOptions::TRANSPARENT_DRAW))
	{
		return;
	}

	if(!isTranslucent && !Renderer::GetOptions()->IsOptionEnabled(RenderOptions::OPAQUE_DRAW))
	{
		return;
	}

	//Dizz: uniformFogDensity != -1 is a check if fog is inabled in shader
	if(isFogEnabled && (uniformFogDensity != -1) && !Renderer::GetOptions()->IsOptionEnabled(RenderOptions::FOG_ENABLE))
	{
		RebuildShader();
	}

	if(isFogEnabled && (uniformFogDensity == -1) && Renderer::GetOptions()->IsOptionEnabled(RenderOptions::FOG_ENABLE))
	{
		RebuildShader();
	}

	RenderManager::Instance()->SetRenderData(group->renderDataObject);

// 	eBlendMode oldSrc;
// 	eBlendMode oldDst;
	if(isAlphablend)
	{
		//oldSrc = RenderManager::Instance()->GetSrcBlend();
		//oldDst = RenderManager::Instance()->GetDestBlend();
	}

	PrepareRenderState(instanceMaterialState, worldMxPtr);

    // TODO: rethink this code
    if (group->renderDataObject->GetIndexBufferID() != 0)
    {
        RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, group->indexCount, group->renderDataObject->GetIndexFormat(), 0);
    }
    else
    {
        RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, group->indexCount, group->renderDataObject->GetIndexFormat(), group->indexArray);
    }

    
	//RenderManager::Instance()->SetTexture(0, 1); 
	//RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
	//if(isAlphablend)
	//{
	//	RenderManager::Instance()->SetBlendMode(oldSrc, oldDst);
	//}
	 */
}



void Material::SetSetupLightmap(bool _isSetupLightmap)
{
	if(isSetupLightmap != _isSetupLightmap)
	{
		isSetupLightmap = _isSetupLightmap;
		RebuildShader();
	}
}
    
bool Material::GetSetupLightmap() const
{
    return isSetupLightmap;
}

    
void Material::SetTexture(eTextureLevel level, Texture * texture)
{
    if (texture == textures[level])return;
    
    SafeRelease(textures[level]);
	names[level] = String("");

    textures[level] = SafeRetain(texture);
	if(textures[level])
	{
		if(!textures[level]->isRenderTarget)
		{
			names[level] = textures[level]->GetPathname();
		}
	}
}

void Material::SetTexture(eTextureLevel level, const FilePath & textureName)
{
    SafeRelease(textures[level]);
    names[level] = FilePath();
 
    Texture *t = Texture::CreateFromFile(textureName, FastName("albedo"));
    if(t)
    {
        textures[level] = t;
        names[level] = textureName;
    }
}

void Material::SetAlphablend(bool _isAlphablend)
{
	isAlphablend = _isAlphablend;
	RebuildShader();
}

bool Material::GetAlphablend()
{
	return isAlphablend;
}

/*RenderState * Material::GetRenderState()
{
	return &renderStateBlock;
}*/

void Material::SetWireframe(bool _isWireframe)
{
    isWireframe = _isWireframe;
}
    
bool Material::GetWireframe()
{
    return isWireframe;
}

void Material::EnableFlatColor(const bool & isEnabled)
{
    isFlatColorEnabled = isEnabled;
    RebuildShader();
    
}

const bool & Material::IsFlatColorEnabled()
{
    return isFlatColorEnabled;
}

void Material::EnableTextureShift(const bool & isEnabled)
{
    isTexture0ShiftEnabled = isEnabled;
    RebuildShader();
    
}

const bool & Material::IsTextureShiftEnabled()
{
    return isTexture0ShiftEnabled;
}

    
const FastName & Material::GetOwnerLayerName()
{
    if(GetAlphablend())
    {
        SetOwnerLayerName(RenderLayer::GetLayerNameByID(RenderLayer::RENDER_LAYER_AFTER_TRANSLUCENT_ID));
    }
    else SetOwnerLayerName(RenderLayer::GetLayerNameByID(RenderLayer::RENDER_LAYER_AFTER_OPAQUE_ID));

    return ownerLayerName;
}

void Material::SetOwnerLayerName(const FastName & fastname)
{
    ownerLayerName = fastname;
    
}
const bool & Material::IsExportOwnerLayerEnabled()
{
    return isExportOwnerLayerEnabled;
}
    
void Material::SetExportOwnerLayer(const bool & isEnabled)
{
    isExportOwnerLayerEnabled = isEnabled;
}

const String& Material::GetName() const
{
    return name;
}

void Material::SetName(const String& materialName)
{
    name = materialName;
}


};

