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



#include "SceneValidator.h"
#include "EditorSettings.h"

#include "Render/LibPVRHelper.h"
#include "Render/TextureDescriptor.h"

#include "Qt/Main/QtUtils.h"
#include "StringConstants.h"

#include "Scene3D/Components/ComponentHelpers.h"

#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"

SceneValidator::SceneValidator()
{
//    sceneTextureCount = 0;
//    sceneTextureMemory = 0;

    pathForChecking = String("");
}

SceneValidator::~SceneValidator()
{
}

bool SceneValidator::ValidateSceneAndShowErrors(Scene *scene, const DAVA::FilePath &scenePath)
{
    errorMessages.clear();

	ConvertLodToV2(scene);
    ValidateScene(scene, scenePath, errorMessages);

    ShowErrorDialog(errorMessages);
    return (!errorMessages.empty());
}

void SceneValidator::ConvertLodToV2(Scene * scene)
{
	scene->lodSystem->UpdateEntitiesAfterLoad();
	SerachForLod(scene);
}

void SceneValidator::SerachForLod(Entity * currentNode)
{
	for(int32 c = 0; c < currentNode->GetChildrenCount(); ++c)
	{
		Entity * childNode = currentNode->GetChild(c);
		SerachForLod(childNode);
		bool wasReplace = MergeLod(childNode);
		if(wasReplace)
		{
			c--;
		}
	}
}

bool SceneValidator::MergeLod(Entity * entity)
{
	LodComponent * lod = GetLodComponent(entity);
	if(lod)
	{
		RenderComponent * rc = GetRenderC
	}

	return false;
}

void SceneValidator::ValidateScene(Scene *scene, const DAVA::FilePath &scenePath, Set<String> &errorsLog)
{
    if(scene) 
    {
		DAVA::String tmp = scenePath.GetAbsolutePathname();
		size_t pos = tmp.find("/Data");
		if(pos != String::npos)
		{
			SetPathForChecking(tmp.substr(0, pos + 1));
		}

        ValidateSceneNode(scene, errorsLog);

        for (Set<Entity*>::iterator it = emptyNodesForDeletion.begin(); it != emptyNodesForDeletion.end(); ++it)
        {
            Entity * node = *it;
            if (node->GetParent())
            {
                node->GetParent()->RemoveNode(node);
            }
        }
        

		for (Set<Entity *>::iterator it = emptyNodesForDeletion.begin(); it != emptyNodesForDeletion.end(); ++it)
		{
			Entity *node = *it;
			SafeRelease(node);
		}

        emptyNodesForDeletion.clear();
    }
    else 
    {
        errorsLog.insert(String("Scene in NULL!"));
    }
}

void SceneValidator::ValidateScales(Scene *scene, Set<String> &errorsLog)
{
	if(scene) 
	{
		ValidateScalesInternal(scene, errorsLog);
	}
	else 
	{
		errorsLog.insert(String("Scene in NULL!"));
	}
}

void SceneValidator::ValidateScalesInternal(Entity *sceneNode, Set<String> &errorsLog)
{
//  Basic algorithm is here
// 	Matrix4 S, T, R; //Scale Transpose Rotation
// 	S.CreateScale(Vector3(1.5, 0.5, 2.0));
// 	T.CreateTranslation(Vector3(100, 50, 20));
// 	R.CreateRotation(Vector3(0, 1, 0), 2.0);
// 
// 	Matrix4 t = R*S*T; //Calculate complex matrix
// 
//	//Calculate Scale components from complex matrix
// 	float32 sx = sqrt(t._00 * t._00 + t._10 * t._10 + t._20 * t._20);
// 	float32 sy = sqrt(t._01 * t._01 + t._11 * t._11 + t._21 * t._21);
// 	float32 sz = sqrt(t._02 * t._02 + t._12 * t._12 + t._22 * t._22);
// 	Vector3 sCalculated(sx, sy, sz);

	if(!sceneNode) return;

	const Matrix4 & t = sceneNode->GetLocalTransform();
	float32 sx = sqrt(t._00 * t._00 + t._10 * t._10 + t._20 * t._20);
	float32 sy = sqrt(t._01 * t._01 + t._11 * t._11 + t._21 * t._21);
	float32 sz = sqrt(t._02 * t._02 + t._12 * t._12 + t._22 * t._22);

	if ((!FLOAT_EQUAL(sx, 1.0f)) 
		|| (!FLOAT_EQUAL(sy, 1.0f))
		|| (!FLOAT_EQUAL(sz, 1.0f)))
	{
 		errorsLog.insert(Format("Node %s: has scale (%.3f, %.3f, %.3f) ! Re-design level.", sceneNode->GetName().c_str(), sx, sy, sz));
	}

	int32 count = sceneNode->GetChildrenCount();
	for(int32 i = 0; i < count; ++i)
	{
		ValidateScalesInternal(sceneNode->GetChild(i), errorsLog);
	}
}


void SceneValidator::ValidateSceneNode(Entity *sceneNode, Set<String> &errorsLog)
{
    if(!sceneNode) return;
    
    int32 count = sceneNode->GetChildrenCount();
    for(int32 i = 0; i < count; ++i)
    {
        Entity *node = sceneNode->GetChild(i);
        
        ValidateRenderComponent(node, errorsLog);
        ValidateLodComponent(node, errorsLog);
        ValidateParticleEmitterComponent(node, errorsLog);
        ValidateSceneNode(node, errorsLog);
    }
}


void SceneValidator::ValidateRenderComponent(Entity *ownerNode, Set<String> &errorsLog)
{
    RenderComponent *rc = static_cast<RenderComponent *>(ownerNode->GetComponent(Component::RENDER_COMPONENT));
    if(!rc) return;
    
    RenderObject *ro = rc->GetRenderObject();
    if(!ro) return;
    
    uint32 count = ro->GetRenderBatchCount();
    for(uint32 b = 0; b < count; ++b)
    {
        RenderBatch *renderBatch = ro->GetRenderBatch(b);
        ValidateRenderBatch(ownerNode, renderBatch, errorsLog);
    }
    
	if(ro->GetType() == RenderObject::TYPE_LANDSCAPE)
    {
		Landscape *landscape = static_cast<Landscape *>(ro);
        ValidateLandscape(landscape, errorsLog);

		ValidateCustomColorsTexture(ownerNode, errorsLog);
    }
}


void SceneValidator::ValidateLodComponent(Entity *ownerNode, Set<String> &errorsLog)
{
    LodComponent *lodComponent = GetLodComponent(ownerNode);
    if(!lodComponent) return;

    int32 layersCount = lodComponent->GetLodLayersCount();
	if (GetEmitter(ownerNode))
		layersCount = LodComponent::MAX_LOD_LAYERS;
    
    if(layersCount == 0)
    {
        errorsLog.insert(Format("Node %s: Count of layers is 0", ownerNode->GetName().c_str()));
    }
    
    for(int32 layer = 0; layer < layersCount; ++layer)
    {
        float32 distance = lodComponent->GetLodLayerDistance(layer);
        if(LodComponent::INVALID_DISTANCE == distance)
        {
            //TODO: why this function isn't realized for lodcomponent?
            lodComponent->SetLodLayerDistance(layer, LodComponent::GetDefaultDistance(layer));
            errorsLog.insert(Format("Node %s: lod distances weren't correct. Re-save.", ownerNode->GetName().c_str()));
        }
    }
    
    Vector<LodComponent::LodData *>lodLayers;
    lodComponent->GetLodData(lodLayers);
    
    Vector<LodComponent::LodData *>::const_iterator endIt = lodLayers.end();
    int32 layer = 0;
    for(Vector<LodComponent::LodData *>::iterator it = lodLayers.begin(); it != endIt; ++it, ++layer)
    {
        LodComponent::LodData * ld = *it;
        
        if(ld->layer != layer)
        {
            ld->layer = layer;
            errorsLog.insert(Format("Node %s: lod layers weren't correct. Rename childs. Re-save.", ownerNode->GetName().c_str()));
        }
    }
}

void SceneValidator::ValidateParticleEmitterComponent(DAVA::Entity *ownerNode, Set<String> &errorsLog)
{
	ParticleEmitter * emitter = GetEmitter(ownerNode);
    if(!emitter)
	{
		return;
	}

	if (GetLodComponent(ownerNode) == NULL)
	{
		ownerNode->AddComponent(new LodComponent());
	}

	ValidateParticleEmitter(emitter, errorsLog);
	
}

bool SceneValidator::ValidateParticleEmitter(ParticleEmitter* emitter, Set<String> &errorsLog)
{
	if (!emitter)
	{
		return true;
	}

	//validate layers
	bool validationResult = true;
	for (int32 i = 0; i < (int32)emitter->GetLayers().size(); ++i)
	{
		if (emitter->GetLayers()[i]->IsFrameBlendEnabled()&&(emitter->GetLayers()[i]->GetSprite()->GetFrameCount()==1))
		{
			String validationMsg = emitter->GetConfigPath().GetAbsolutePathname().c_str();			
			validationMsg += " - layer ";
			validationMsg += emitter->GetLayers()[i]->layerName;
			validationMsg += " have \"Enable frame blending\" checked while sprite have only 1 frame. Frame blending would just waste time";
			errorsLog.insert(validationMsg);
			validationResult = false;
		}
	}
	
	if (!emitter->Is3DFlagCorrect())
	{
		// Don't use Format() helper here - the string with path might be too long for Format().
		String validationMsg = ("\"3d\" flag value is wrong for Particle Emitter Configuration file ");
		validationMsg += emitter->GetConfigPath().GetAbsolutePathname().c_str();
		validationMsg += ". Please verify whether you are using the correct configuration file.\n\"3d\" flag for this Particle Emitter will be reset to TRUE.";
		errorsLog.insert(validationMsg);

		// Yuri Coder, 2013/05/08. Since Particle Editor works with 3D Particles only - have to set this flag
		// manually.
		emitter->Set3D(true);
		validationResult = false;
	}
			
	return validationResult;
}

void SceneValidator::ValidateRenderBatch(Entity *ownerNode, RenderBatch *renderBatch, Set<String> &errorsLog)
{
    ownerNode->RemoveFlag(Entity::NODE_INVALID);
    
    
    NMaterial *material = renderBatch->GetMaterial();
    if(material)
    {
        ValidateMaterial(material, errorsLog);
    }
    
//     InstanceMaterialState *materialState = renderBatch->GetMaterialInstance();
//     if(materialState)
//     {
//         ValidateInstanceMaterialState(materialState, errorsLog);
// 		ConvertLightmapSizeFromProperty(ownerNode, materialState);
//     }
    
    
    PolygonGroup *polygonGroup = renderBatch->GetPolygonGroup();
    if(polygonGroup)
    {
        if(material)
        {
//             if (material->Validate(polygonGroup) == Material::VALIDATE_INCOMPATIBLE)
//             {
//                 ownerNode->AddFlag(Entity::NODE_INVALID);
//                 errorsLog.insert(Format("Material: %s incompatible with node:%s.", material->GetName().c_str(), ownerNode->GetFullName().c_str()));
//                 errorsLog.insert("For lightmapped objects check second coordinate set. For normalmapped check tangents, binormals.");
//             }
        }
    }
}

void SceneValidator::ValidateMaterial(NMaterial *material, Set<String> &errorsLog)
{
	if(NULL != material)
	{
		for(uint32 iTex = 0; iTex < material->GetTextureCount(); ++iTex)
		{
			Texture *texture = material->GetTexture(iTex);
			if(texture)
			{
// 				ValidateTexture(texture, texture->GetTextureMap((Material::eTextureLevel)iTex), Format("Material: %s. TextureLevel %d.", material->GetName().c_str(), iTex), errorsLog);
// 
// 				FilePath matTexName = material->GetTextureName((Material::eTextureLevel)iTex);
// 				if(!matTexName.IsEmpty() && !IsTextureDescriptorPath(matTexName))
// 				{
// 					material->SetTexture((Material::eTextureLevel)iTex, TextureDescriptor::GetDescriptorPathname(matTexName));
// 				}
			}
		}
	}
}


void SceneValidator::ValidateInstanceMaterialState(InstanceMaterialState *materialState, Set<String> &errorsLog)
{
    if(materialState->GetLightmap())
    {
        ValidateTexture(materialState->GetLightmap(), materialState->GetLightmapName(), "InstanceMaterialState, lightmap", errorsLog);
    }
    
    FilePath lightmapName = materialState->GetLightmapName();
    if(!IsTextureDescriptorPath(lightmapName))
    {
        Texture *lightmap = SafeRetain(materialState->GetLightmap());
        
        if(lightmapName.IsEmpty())
        {
            materialState->SetLightmap(lightmap, FilePath());
        }
        else
        {
            materialState->SetLightmap(lightmap, TextureDescriptor::GetDescriptorPathname(lightmapName));
        }
        
        SafeRelease(lightmap);
    }
}


void SceneValidator::ValidateLandscape(Landscape *landscape, Set<String> &errorsLog)
{
    if(!landscape) return;
    
	if(landscape->GetTiledShaderMode() == Landscape::TILED_MODE_TILE_DETAIL_MASK)
	{
		for(int32 i = 0; i < Landscape::TEXTURE_COUNT; ++i)
		{
			Landscape::eTextureLevel texLevel = (Landscape::eTextureLevel)i;
			if(texLevel == Landscape::TEXTURE_COLOR || texLevel == Landscape::TEXTURE_TILE_MASK || texLevel == Landscape::TEXTURE_TILE0)
			{
				ValidateLandscapeTexture(landscape, texLevel, errorsLog);
			}

			Color color = landscape->GetTileColor(texLevel);
			if (!ValidateColor(color))
			{
				landscape->SetTileColor(texLevel, color);
			}
		}
	}
	else
	{
		for(int32 i = 0; i < Landscape::TEXTURE_COUNT; ++i)
		{
			Landscape::eTextureLevel texLevel = (Landscape::eTextureLevel)i;
			if(		(Landscape::TEXTURE_DETAIL == texLevel)
				||	(Landscape::TEXTURE_TILE_FULL == texLevel
				&&	(landscape->GetTiledShaderMode() == Landscape::TILED_MODE_TILEMASK
				|| landscape->GetTiledShaderMode() == Landscape::TILED_MODE_TILE_DETAIL_MASK)))
			{
				continue;
			}

			ValidateLandscapeTexture(landscape, texLevel, errorsLog);
		}
	}
    

	//validate heightmap
    bool pathIsCorrect = ValidatePathname(landscape->GetHeightmapPathname(), String("Landscape. Heightmap."));
    if(!pathIsCorrect)
    {
        String path = landscape->GetHeightmapPathname().GetRelativePathname(EditorSettings::Instance()->GetDataSourcePath());
        errorsLog.insert("Wrong path of Heightmap: " + path);
    }
}

void SceneValidator::ValidateLandscapeTexture(Landscape *landscape, Landscape::eTextureLevel texLevel, Set<String> &errorsLog)
{
	DAVA::FilePath landTexName = landscape->GetTextureName(texLevel);
	if(!IsTextureDescriptorPath(landTexName) &&
	   landTexName.GetAbsolutePathname().size() > 0)
	{
		landscape->SetTextureName(texLevel, TextureDescriptor::GetDescriptorPathname(landTexName));
	}

	ValidateTexture(landscape->GetTexture(texLevel), landscape->GetTextureName(texLevel), Format("Landscape. TextureLevel %d", texLevel), errorsLog);
}


void SceneValidator::ConvertLightmapSizeFromProperty(Entity *ownerNode, InstanceMaterialState *materialState)
{
	KeyedArchive * props = ownerNode->GetCustomProperties();
	Map<String, VariantType*> map = props->GetArchieveData();
	for(Map<String, VariantType*>::iterator it = map.begin(); it != map.end(); it++)
	{
		String key = it->first;
		if(key.find("lightmap.size") != String::npos && ((RenderComponent*)ownerNode->GetComponent(Component::RENDER_COMPONENT))->GetRenderObject()->GetType() != RenderObject::TYPE_LANDSCAPE)
		{
			materialState->SetLightmapSize(props->GetInt32(key, 128));
			props->DeleteKey(key);
			break;
		}
	}
}

bool SceneValidator::NodeRemovingDisabled(Entity *node)
{
    KeyedArchive *customProperties = node->GetCustomProperties();
    return (customProperties && customProperties->IsKeyExists(ResourceEditor::EDITOR_DO_NOT_REMOVE));
}


void SceneValidator::ValidateTextureAndShowErrors(Texture *texture, const FilePath &textureName, const String &validatedObjectName)
{
    errorMessages.clear();

    ValidateTexture(texture, textureName, validatedObjectName, errorMessages);
    ShowErrorDialog(errorMessages);
}

void SceneValidator::ValidateTexture(Texture *texture, const FilePath &texturePathname, const String &validatedObjectName, Set<String> &errorsLog)
{
	if(!texture) return;
	
	String path = texturePathname.GetRelativePathname(pathForChecking);
	String textureInfo = path + " for object: " + validatedObjectName;

	if(texture->IsPinkPlaceholder())
	{
		errorsLog.insert("Can't load texture: " + textureInfo);
	}

	bool pathIsCorrect = ValidatePathname(texturePathname, validatedObjectName);
	if(!pathIsCorrect)
	{
		errorsLog.insert("Wrong path of: " + textureInfo);
	}
	
	if(!IsPowerOf2(texture->GetWidth()) || !IsPowerOf2(texture->GetHeight()))
	{
		errorsLog.insert("Wrong size of " + textureInfo);
	}
    
    if(texture->GetWidth() > 2048 || texture->GetHeight() > 2048)
	{
		errorsLog.insert("Texture is too big. " + textureInfo);
	}
}



//void SceneValidator::EnumerateSceneTextures()
//{
//    SceneData *sceneData = SceneDataManager::Instance()->SceneGetActive();
//    DVASSERT_MSG(sceneData, "Illegal situation");
//    
//    Map<String, Texture *> textureMap;
//    SceneDataManager::EnumerateTextures(sceneData->GetScene(), textureMap);
//    sceneTextureCount = textureMap.size();
//
//    KeyedArchive *settings = EditorSettings::Instance()->GetSettings();
//    String projectPath = settings->GetString("ProjectPath");
//
//    sceneTextureMemory = 0;
//	for(Map<String, Texture *>::const_iterator it = textureMap.begin(); it != textureMap.end(); ++it)
//	{
//		Texture *t = it->second;
//        if(String::npos == t->GetPathname().GetAbsolutePathname().find(projectPath))
//        {   // skip all textures that are not related the scene
//            continue;
//        }
//
//        
//        if(String::npos != t->GetPathname().GetAbsolutePathname().find(projectPath))
//        {   //We need real info about textures size. In Editor on desktop pvr textures are decompressed to RGBA8888, so they have not real size.
//            FilePath imageFileName = TextureDescriptor::GetPathnameForFormat(t->GetPathname(), t->GetSourceFileFormat());
//            switch (t->GetSourceFileFormat())
//            {
//                case DAVA::PVR_FILE:
//                {
//                    sceneTextureMemory += LibPVRHelper::GetDataLength(imageFileName);
//                    break;
//                }
//                    
//                case DAVA::DXT_FILE:
//                {
//                    sceneTextureMemory += (int32)LibDxtHelper::GetDataSize(imageFileName);
//                    break;
//                }
//                    
//                default:
//                    sceneTextureMemory += t->GetDataSize();
//                    break;
//            }
//        }
//	}
//}

bool SceneValidator::WasTextureChanged(Texture *texture, eGPUFamily forGPU)
{
    if(IsFBOTexture(texture))
    {
        return false;
    }
    
    FilePath texturePathname = texture->GetPathname();
    return (IsPathCorrectForProject(texturePathname) && IsTextureChanged(texturePathname, forGPU));
}

bool SceneValidator::IsFBOTexture(Texture *texture)
{
    if(texture->isRenderTarget)
    {
        return true;
    }

    String::size_type textTexturePos = texture->GetPathname().GetAbsolutePathname().find("Text texture");
    if(String::npos != textTexturePos)
    {
        return true; //is text texture
    }
    
    return false;
}



FilePath SceneValidator::SetPathForChecking(const FilePath &pathname)
{
    FilePath oldPath = pathForChecking;
    pathForChecking = pathname;
    return oldPath;
}


bool SceneValidator::ValidateTexturePathname(const FilePath &pathForValidation, Set<String> &errorsLog)
{
	DVASSERT_MSG(!pathForChecking.IsEmpty(), "Need to set pathname for DataSource folder");

	bool pathIsCorrect = IsPathCorrectForProject(pathForValidation);
	if(pathIsCorrect)
	{
		String textureExtension = pathForValidation.GetExtension();
		String::size_type extPosition = TextureDescriptor::GetSupportedTextureExtensions().find(textureExtension);
		if(String::npos == extPosition)
		{
			errorsLog.insert(Format("Path %s has incorrect extension", pathForValidation.GetAbsolutePathname().c_str()));
			return false;
		}
	}
	else
	{
		errorsLog.insert(Format("Path %s is incorrect for project %s", pathForValidation.GetAbsolutePathname().c_str(), pathForChecking.GetAbsolutePathname().c_str()));
	}

	return pathIsCorrect;
}

bool SceneValidator::ValidateHeightmapPathname(const FilePath &pathForValidation, Set<String> &errorsLog)
{
	DVASSERT_MSG(!pathForChecking.IsEmpty(), "Need to set pathname for DataSource folder");

	bool pathIsCorrect = IsPathCorrectForProject(pathForValidation);
	if(pathIsCorrect)
	{
		String::size_type posPng = pathForValidation.GetAbsolutePathname().find(".png");
		String::size_type posHeightmap = pathForValidation.GetAbsolutePathname().find(Heightmap::FileExtension());
        
        pathIsCorrect = ((String::npos != posPng) || (String::npos != posHeightmap));
        if(!pathIsCorrect)
        {
            errorsLog.insert(Format("Heightmap path %s is wrong", pathForValidation.GetAbsolutePathname().c_str()));
            return false;
        }
        
        Heightmap *heightmap = new Heightmap();
        if(String::npos != posPng)
        {
            Image *image = CreateTopLevelImage(pathForValidation);
            pathIsCorrect = heightmap->BuildFromImage(image);
            SafeRelease(image);
        }
        else
        {
            pathIsCorrect = heightmap->Load(pathForValidation);
        }

        
        if(!pathIsCorrect)
        {
            SafeRelease(heightmap);
            errorsLog.insert(Format("Can't load Heightmap from path %s", pathForValidation.GetAbsolutePathname().c_str()));
            return false;
        }
        
        
        pathIsCorrect = IsPowerOf2(heightmap->Size() - 1);
        if(!pathIsCorrect)
        {
            errorsLog.insert(Format("Heightmap %s has wrong size", pathForValidation.GetAbsolutePathname().c_str()));
        }
        
        SafeRelease(heightmap);
		return pathIsCorrect;
	}
	else
	{
		errorsLog.insert(Format("Path %s is incorrect for project %s", pathForValidation.GetAbsolutePathname().c_str(), pathForChecking.GetAbsolutePathname().c_str()));
	}

	return pathIsCorrect;
}


bool SceneValidator::ValidatePathname(const FilePath &pathForValidation, const String &validatedObjectName)
{
    DVASSERT(!pathForChecking.IsEmpty());
    //Need to set path to DataSource/3d for path correction  
    //Use SetPathForChecking();
    
    String pathname = pathForValidation.GetAbsolutePathname();
    
    String::size_type fboFound = pathname.find(String("FBO"));
    String::size_type resFound = pathname.find(String("~res:"));
    if((String::npos != fboFound) || (String::npos != resFound))
    {
        return true;   
    }
    
    return IsPathCorrectForProject(pathForValidation);
}

bool SceneValidator::IsPathCorrectForProject(const FilePath &pathname)
{
    String normalizedPath = pathname.GetAbsolutePathname();
    String::size_type foundPos = normalizedPath.find(pathForChecking.GetAbsolutePathname());
    return (String::npos != foundPos);
}


void SceneValidator::EnumerateNodes(DAVA::Scene *scene)
{
    int32 nodesCount = 0;
    if(scene)
    {
        for(int32 i = 0; i < scene->GetChildrenCount(); ++i)
        {
            nodesCount += EnumerateSceneNodes(scene->GetChild(i));
        }
    }
}

int32 SceneValidator::EnumerateSceneNodes(DAVA::Entity *node)
{
    //TODO: lode node can have several nodes at layer
    
    int32 nodesCount = 1;
    for(int32 i = 0; i < node->GetChildrenCount(); ++i)
    {
        nodesCount += EnumerateSceneNodes(node->GetChild(i));
    }
    
    return nodesCount;
}


bool SceneValidator::IsTextureChanged(const FilePath &texturePathname, eGPUFamily forGPU)
{
    bool isChanged = false;

	TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(texturePathname);
    if(descriptor)
    {
        isChanged = IsTextureChanged(descriptor, forGPU);
        SafeRelease(descriptor);
    }

    return isChanged;
}

bool SceneValidator::IsTextureChanged(const TextureDescriptor *descriptor, eGPUFamily forGPU)
{
    DVASSERT(descriptor);
    
    return !descriptor->IsCompressedTextureActual(forGPU);
}


bool SceneValidator::IsTextureDescriptorPath(const FilePath &path)
{
	return path.IsEqualToExtension(TextureDescriptor::GetDescriptorExtension());
}

void SceneValidator::ValidateCustomColorsTexture(Entity *landscapeEntity, Set<String> &errorsLog)
{
	KeyedArchive* customProps = landscapeEntity->GetCustomProperties();
	if(customProps->IsKeyExists(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP))
	{
		String currentSaveName = customProps->GetString(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP);
		FilePath path = "/" + currentSaveName;
		if(!path.IsEqualToExtension(".png"))
		{
			errorsLog.insert("Custom colors texture has to have .png extension.");
		}
        
        String::size_type foundPos = currentSaveName.find("DataSource/3d/");
        if(String::npos == foundPos)
        {
			errorsLog.insert("Custom colors texture has to begin from DataSource/3d/.");
        }
	}
}

bool SceneValidator::ValidateColor(Color& color)
{
	bool ok = true;
	for(int32 i = 0; i < 4; ++i)
	{
		if (color.color[i] < 0.f || color.color[i] > 1.f)
		{
			color.color[i] = Clamp(color.color[i], 0.f, 1.f);
			ok = false;
		}
	}
	return ok;
}





