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
#include "Qt/Settings/SettingsManager.h"
#include "Project/ProjectManager.h"
#include "Render/LibPVRHelper.h"
#include "Render/TextureDescriptor.h"

#include "Qt/Main/QtUtils.h"
#include "StringConstants.h"

#include "Scene3D/Components/ComponentHelpers.h"

#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"

#include "Scene/SceneEditor2.h"

#include "Scene3D/Systems/MaterialSystem.h"

#include "Render/Material/NMaterialNames.h"

SceneValidator::SceneValidator()
{
    pathForChecking = String("");
}

SceneValidator::~SceneValidator()
{
}

bool SceneValidator::ValidateSceneAndShowErrors(Scene *scene, const DAVA::FilePath &scenePath)
{
    errorMessages.clear();

    ValidateScene(scene, scenePath, errorMessages);

    ShowErrorDialog(errorMessages);
    return (!errorMessages.empty());
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
		ValidateMaterials(scene, errorsLog);

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
        ValidateParticleEffectComponent(node, errorsLog);
        ValidateSceneNode(node, errorsLog);
        ValidateNodeCustomProperties(node);
    }
}


void SceneValidator::ValidateNodeCustomProperties(Entity * sceneNode)
{
    if(!GetLight(sceneNode))
    {
        KeyedArchive * props = sceneNode->GetCustomProperties();

        props->DeleteKey("editor.staticlight.used");
        props->DeleteKey("editor.staticlight.enable");
        props->DeleteKey("editor.staticlight.castshadows");
        props->DeleteKey("editor.staticlight.receiveshadows");
        props->DeleteKey("lightmap.size");
    }
}

void SceneValidator::ValidateRenderComponent(Entity *ownerNode, Set<String> &errorsLog)
{
    RenderComponent *rc = static_cast<RenderComponent *>(ownerNode->GetComponent(Component::RENDER_COMPONENT));
    if(!rc) return;
    
    RenderObject *ro = rc->GetRenderObject();
    if(!ro) return;
    
    bool isSpeedTree = false;

    uint32 count = ro->GetRenderBatchCount();
    for(uint32 b = 0; b < count; ++b)
    {
        RenderBatch *renderBatch = ro->GetRenderBatch(b);
        ValidateRenderBatch(ownerNode, renderBatch, errorsLog);

        isSpeedTree |= (renderBatch->GetMaterial() && renderBatch->GetMaterial()->GetMaterialTemplate()->name == NMaterialName::SPEEDTREE_LEAF);
    }
    
    if(isSpeedTree && !IsPointerToExactClass<SpeedTreeObject>(ro))
    {
        Entity * parent = ownerNode->GetParent();
        DVASSERT(parent);
        Entity * nextEntity = parent->GetNextChild(ownerNode);

        ownerNode->Retain();
        parent->RemoveNode(ownerNode);

        SpeedTreeObject * treeObject = new SpeedTreeObject();
        ro->Clone(treeObject);
        rc->SetRenderObject(treeObject);
        treeObject->Release();

        treeObject->RecalcBoundingBox();

        if(nextEntity)
            parent->InsertBeforeNode(ownerNode, nextEntity);
        else
            parent->AddNode(ownerNode);
        ownerNode->Release();
    }

	if(ro->GetType() == RenderObject::TYPE_LANDSCAPE)
    {
        ownerNode->SetLocked(true);
        if(ownerNode->GetLocalTransform() != DAVA::Matrix4::IDENTITY)
        {
            ownerNode->SetLocalTransform(DAVA::Matrix4::IDENTITY);
            SceneEditor2 *sc = dynamic_cast<SceneEditor2 *>(ownerNode->GetScene());
            if(sc)
            {
                sc->MarkAsChanged();
            }
            errorsLog.insert("Landscape had wrong transform. Please re-save scene.");
        }
        
		Landscape *landscape = static_cast<Landscape *>(ro);
        ValidateLandscape(landscape, errorsLog);

		ValidateCustomColorsTexture(ownerNode, errorsLog);
    }
}



void SceneValidator::ValidateParticleEffectComponent(DAVA::Entity *ownerNode, Set<String> &errorsLog)
{
	ParticleEffectComponent *effect = GetEffectComponent(ownerNode);
    if(!effect)
	{
		return;
	}

}

void SceneValidator::ValidateRenderBatch(Entity *ownerNode, RenderBatch *renderBatch, Set<String> &errorsLog)
{
    ownerNode->RemoveFlag(Entity::NODE_INVALID);
    
    NMaterial *material = renderBatch->GetMaterial();
    if(material)
    {
        ConvertIlluminationParamsFromProperty(ownerNode, material);
    }
}

void SceneValidator::ValidateMaterials(DAVA::Scene *scene, Set<String> &errorsLog)
{
	DAVA::MaterialSystem *matSystem = scene->GetMaterialSystem();
	Set<DAVA::NMaterial *> materials;
	matSystem->BuildMaterialList(scene, materials);

	DAVA::Map<DAVA::Texture *, DAVA::String> texturesMap;
	auto endItMaterials = materials.end();
	for(auto it = materials.begin(); it != endItMaterials; ++it)
	{
		DAVA::uint32 count = (*it)->GetTextureCount();
		for(DAVA::uint32 t = 0; t < count; ++t)
		{
			Texture *tex = (*it)->GetTexture(t);
			if(tex)
			{
				if(((*it)->GetMaterialType() == DAVA::NMaterial::MATERIALTYPE_INSTANCE) && (*it)->GetParent())
				{
					texturesMap[tex] = Format("Material: %s (%s). Texture %s.", (*it)->GetName().c_str(), (*it)->GetParent()->GetName().c_str(), (*it)->GetTextureName(t).c_str());
				}
				else
				{
					texturesMap[tex] = Format("Material: %s. Texture %s.", (*it)->GetName().c_str(), (*it)->GetTextureName(t).c_str());
				}
			}
		}
	}

	auto endItTextures = texturesMap.end();
	for(auto it = texturesMap.begin(); it != endItTextures; ++it)
	{
		ValidateTexture(it->first, it->second, errorsLog);
	}
}


void SceneValidator::ValidateLandscape(Landscape *landscape, Set<String> &errorsLog)
{
    if(!landscape) return;
    
    
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

	//validate heightmap
    bool pathIsCorrect = ValidatePathname(landscape->GetHeightmapPathname(), String("Landscape. Heightmap."));
    if(!pathIsCorrect)
    {
        String path = landscape->GetHeightmapPathname().GetRelativePathname(FilePath(ProjectManager::Instance()->CurProjectDataSourcePath().toStdString()));
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

	ValidateTexture(landscape->GetTexture(texLevel), Format("Landscape. TextureLevel %d", texLevel), errorsLog);
}


void SceneValidator::ConvertIlluminationParamsFromProperty(Entity *ownerNode, NMaterial *material)
{
    IlluminationParams * params = material->GetIlluminationParams();

    VariantType * variant = 0;
    variant = GetCustomPropertyFromParentsTree(ownerNode, "editor.staticlight.used");
    if(variant) params->isUsed = variant->AsBool();
    variant = GetCustomPropertyFromParentsTree(ownerNode, "editor.staticlight.castshadows");
    if(variant) params->castShadow = variant->AsBool();
    variant = GetCustomPropertyFromParentsTree(ownerNode, "editor.staticlight.receiveshadows");
    if(variant) params->receiveShadow = variant->AsBool();

    variant = GetCustomPropertyFromParentsTree(ownerNode, "lightmap.size");
    if(variant)
        params->lightmapSize = variant->AsInt32();
    else if(IsPointerToExactClass<Landscape>(GetRenderObject(ownerNode)))
        params->lightmapSize = 1024;
}

VariantType* SceneValidator::GetCustomPropertyFromParentsTree(Entity *ownerNode, const String & key)
{
    if(!ownerNode)
        return 0;

    KeyedArchive * props = ownerNode->GetCustomProperties();

    if(!props)
        return 0;

    if(props->IsKeyExists(key))
        return props->GetVariant(key);
    else
        return GetCustomPropertyFromParentsTree(ownerNode->GetParent(), key);
}

bool SceneValidator::NodeRemovingDisabled(Entity *node)
{
    KeyedArchive *customProperties = node->GetCustomProperties();
    return (customProperties && customProperties->IsKeyExists(ResourceEditor::EDITOR_DO_NOT_REMOVE));
}


void SceneValidator::ValidateTextureAndShowErrors(Texture *texture, const String &validatedObjectName)
{
    errorMessages.clear();

    ValidateTexture(texture, validatedObjectName, errorMessages);
    ShowErrorDialog(errorMessages);
}

void SceneValidator::ValidateTexture(Texture *texture, const String &validatedObjectName, Set<String> &errorsLog)
{
	if(!texture) return;
	
	const FilePath & texturePathname = texture->GetPathname();

	String path = texturePathname.GetRelativePathname(pathForChecking);
	String textureInfo = path + " for object: " + validatedObjectName;

	if(texture->IsPinkPlaceholder())
	{
		if(texturePathname.IsEmpty())
		{
			errorsLog.insert("Texture not set for object: " + validatedObjectName);
		}
		else
		{
			errorsLog.insert("Can't load texture: " + textureInfo);
		}
		return;
	}

	bool pathIsCorrect = ValidatePathname(texturePathname, validatedObjectName);
	if(!pathIsCorrect)
	{
		errorsLog.insert("Wrong path of: " + textureInfo);
		return;
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
		delete descriptor;
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




