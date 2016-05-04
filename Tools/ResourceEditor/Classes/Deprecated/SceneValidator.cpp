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
#include "Render/Image/LibPVRHelper.h"
#include "Render/TextureDescriptor.h"
#include "Render/Material/NMaterialNames.h"

#include "Scene3D/Components/ComponentHelpers.h"

#include "Main/QtUtils.h"
#include "Project/ProjectManager.h"
#include "Scene/SceneEditor2.h"
#include "Scene/SceneHelper.h"
#include "Settings/SettingsManager.h"
#include "StringConstants.h"

#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"


#include "QtTools/ConsoleWidget/PointerSerializer.h"

template <typename... A>
void PushLogMessage(DAVA::Set<DAVA::String>& messages, DAVA::Entity* object, const char* format, A... args)
{
    auto infoText = DAVA::Format(format, args...);
    if (nullptr != object)
        infoText += PointerSerializer::FromPointer(object);
    messages.insert(infoText);
}

SceneValidator::SceneValidator()
{
    pathForChecking = DAVA::String("");
}

SceneValidator::~SceneValidator()
{
}

bool SceneValidator::ValidateSceneAndShowErrors(DAVA::Scene* scene, const DAVA::FilePath& scenePath)
{
    errorMessages.clear();
    ValidateScene(scene, scenePath, errorMessages);
    ShowErrorDialog(errorMessages);

    return (!errorMessages.empty());
}

void SceneValidator::ValidateScene(DAVA::Scene* scene, const DAVA::FilePath& scenePath, DAVA::Set<DAVA::String>& errorsLog)
{
    if (scene != nullptr)
    {
        DAVA::String tmp = scenePath.GetAbsolutePathname();
        size_t pos = tmp.find("/Data");
        if (pos != DAVA::String::npos)
        {
            SetPathForChecking(tmp.substr(0, pos + 1));
            sceneName = scenePath.GetFilename();
        }

        ValidateSceneNode(scene, errorsLog);
        ValidateMaterials(scene, errorsLog);

        for (DAVA::Set<DAVA::Entity*>::iterator it = emptyNodesForDeletion.begin(); it != emptyNodesForDeletion.end(); ++it)
        {
            DAVA::Entity* node = *it;
            if (node->GetParent() != nullptr)
            {
                node->GetParent()->RemoveNode(node);
            }
        }

        for (DAVA::Set<DAVA::Entity*>::iterator it = emptyNodesForDeletion.begin(); it != emptyNodesForDeletion.end(); ++it)
        {
            DAVA::Entity* node = *it;
            SafeRelease(node);
        }

        emptyNodesForDeletion.clear();
    }
    else
    {
        PushLogMessage(errorsLog, nullptr, "Scene is not initialized!");
    }
}

void SceneValidator::ValidateScales(DAVA::Scene* scene, DAVA::Set<DAVA::String>& errorsLog)
{
    if (nullptr == scene)
        PushLogMessage(errorsLog, nullptr, "Scene is not initializedr!");
    else
        ValidateScalesInternal(scene, errorsLog);
}

void SceneValidator::ValidateScalesInternal(DAVA::Entity* sceneNode, DAVA::Set<DAVA::String>& errorsLog)
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

    if (nullptr == sceneNode)
    {
        return;
    }

    const DAVA::Matrix4& t = sceneNode->GetLocalTransform();
    DAVA::float32 sx = sqrt(t._00 * t._00 + t._10 * t._10 + t._20 * t._20);
    DAVA::float32 sy = sqrt(t._01 * t._01 + t._11 * t._11 + t._21 * t._21);
    DAVA::float32 sz = sqrt(t._02 * t._02 + t._12 * t._12 + t._22 * t._22);

    if ((!FLOAT_EQUAL(sx, 1.0f)) || (!FLOAT_EQUAL(sy, 1.0f)) || (!FLOAT_EQUAL(sz, 1.0f)))
    {
        PushLogMessage(errorsLog, sceneNode, "Node %s: has scale (%.3f, %.3f, %.3f) ! Re-design level. Scene: %s",
                       sceneNode->GetName().c_str(), sx, sy, sz, sceneName.c_str());
    }

    DAVA::int32 count = sceneNode->GetChildrenCount();
    for (DAVA::int32 i = 0; i < count; ++i)
    {
        ValidateScalesInternal(sceneNode->GetChild(i), errorsLog);
    }
}

void SceneValidator::ValidateSceneNode(DAVA::Entity* sceneNode, DAVA::Set<DAVA::String>& errorsLog)
{
    if (nullptr == sceneNode)
    {
        return;
    }

    DAVA::int32 count = sceneNode->GetChildrenCount();
    for (DAVA::int32 i = 0; i < count; ++i)
    {
        DAVA::Entity* node = sceneNode->GetChild(i);

        ValidateRenderComponent(node, errorsLog);
        ValidateParticleEffectComponent(node, errorsLog);
        ValidateSceneNode(node, errorsLog);
        ValidateNodeCustomProperties(node);
    }
}

void SceneValidator::ValidateNodeCustomProperties(DAVA::Entity* sceneNode)
{
    if (!GetLight(sceneNode))
    {
        DAVA::KeyedArchive* props = GetCustomPropertiesArchieve(sceneNode);
        if (props != nullptr)
        {
            props->DeleteKey("editor.staticlight.used");
            props->DeleteKey("editor.staticlight.enable");
            props->DeleteKey("editor.staticlight.castshadows");
            props->DeleteKey("editor.staticlight.receiveshadows");
            props->DeleteKey("lightmap.size");
        }
    }
}

void SceneValidator::ValidateRenderComponent(DAVA::Entity* ownerNode, DAVA::Set<DAVA::String>& errorsLog)
{
    DAVA::RenderComponent* rc = static_cast<DAVA::RenderComponent*>(ownerNode->GetComponent(DAVA::Component::RENDER_COMPONENT));
    if (nullptr == rc)
    {
        return;
    }

    DAVA::RenderObject* ro = rc->GetRenderObject();
    if (nullptr == ro)
    {
        return;
    }

    DAVA::uint32 count = ro->GetRenderBatchCount();
    for (DAVA::uint32 b = 0; b < count; ++b)
    {
        DAVA::RenderBatch* renderBatch = ro->GetRenderBatch(b);
        ValidateRenderBatch(ownerNode, renderBatch, errorsLog);
    }

    if (ro->GetType() == DAVA::RenderObject::TYPE_LANDSCAPE)
    {
        ownerNode->SetLocked(true);
        FixIdentityTransform(ownerNode, errorsLog,
                             DAVA::Format("Landscape had wrong transform. Please re-save scene: %s", sceneName.c_str()));

        DAVA::Landscape* landscape = static_cast<DAVA::Landscape*>(ro);
        ValidateLandscape(landscape, errorsLog);

        ValidateCustomColorsTexture(ownerNode, errorsLog);
    }

    if (ro->GetType() == DAVA::RenderObject::TYPE_VEGETATION)
    {
        ownerNode->SetLocked(true);
        FixIdentityTransform(ownerNode, errorsLog,
                             DAVA::Format("Vegetation had wrong transform. Please re-save scene: %s", sceneName.c_str()));
    }
}

void SceneValidator::FixIdentityTransform(DAVA::Entity* ownerNode, DAVA::Set<DAVA::String>& errorsLog, const DAVA::String& errorMessage)
{
    if (ownerNode->GetLocalTransform() != DAVA::Matrix4::IDENTITY)
    {
        ownerNode->SetLocalTransform(DAVA::Matrix4::IDENTITY);
        SceneEditor2* sc = dynamic_cast<SceneEditor2*>(ownerNode->GetScene());
        if (sc != nullptr)
        {
            sc->MarkAsChanged();
        }
        PushLogMessage(errorsLog, ownerNode, errorMessage.c_str());
    }
}

void SceneValidator::ValidateParticleEffectComponent(DAVA::Entity* ownerNode, DAVA::Set<DAVA::String>& errorsLog) const
{
    DAVA::ParticleEffectComponent* effect = GetEffectComponent(ownerNode);
    if (effect != nullptr)
    {
        DAVA::uint32 count = effect->GetEmittersCount();
        for (DAVA::uint32 i = 0; i < count; ++i)
        {
            ValidateParticleEmitter(effect->GetEmitter(i), errorsLog, effect->GetEntity());
        }
    }
}

void SceneValidator::ValidateParticleEmitter(DAVA::ParticleEmitter* emitter, DAVA::Set<DAVA::String>& errorsLog, DAVA::Entity* owner) const
{
    DVASSERT(emitter);

    if (nullptr == emitter)
    {
        return;
    }

    if (emitter->configPath.IsEmpty())
    {
        PushLogMessage(errorsLog, owner, "Empty config path for emitter %s. Scene: %s",
                       emitter->name.c_str(), sceneName.c_str());
    }

    const DAVA::Vector<DAVA::ParticleLayer*>& layers = emitter->layers;

    DAVA::uint32 count = static_cast<DAVA::uint32>(layers.size());
    for (DAVA::uint32 i = 0; i < count; ++i)
    {
        if (layers[i]->type == DAVA::ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
        {
            ValidateParticleEmitter(layers[i]->innerEmitter, errorsLog, owner);
        }
    }
}

void SceneValidator::ValidateRenderBatch(DAVA::Entity* ownerNode, DAVA::RenderBatch* renderBatch, DAVA::Set<DAVA::String>& errorsLog)
{
    ownerNode->RemoveFlag(DAVA::Entity::NODE_INVALID);
}

void SceneValidator::ValidateMaterials(DAVA::Scene* scene, DAVA::Set<DAVA::String>& errorsLog)
{
    DAVA::Set<DAVA::NMaterial*> materials;
    SceneHelper::BuildMaterialList(scene, materials, false);
    auto globalMaterial = scene->GetGlobalMaterial();
    if (nullptr != globalMaterial)
    {
        materials.erase(globalMaterial);
    }

    const QVector<ProjectManager::AvailableMaterialTemplate>* materialTemplates = 0;
    if (ProjectManager::Instance() != nullptr)
    {
        materialTemplates = ProjectManager::Instance()->GetAvailableMaterialTemplates();
    }

    DAVA::FastName textureNames[] = {
        DAVA::NMaterialTextureName::TEXTURE_ALBEDO,
        DAVA::NMaterialTextureName::TEXTURE_NORMAL,
        DAVA::NMaterialTextureName::TEXTURE_DETAIL,
        DAVA::NMaterialTextureName::TEXTURE_LIGHTMAP,
        DAVA::NMaterialTextureName::TEXTURE_DECAL,
        DAVA::NMaterialTextureName::TEXTURE_CUBEMAP,
        DAVA::NMaterialTextureName::TEXTURE_DECALMASK,
        DAVA::NMaterialTextureName::TEXTURE_DECALTEXTURE,
    };

    DAVA::Map<DAVA::Texture*, DAVA::String> texturesMap;
    auto endItMaterials = materials.end();
    for (auto it = materials.begin(); it != endItMaterials; ++it)
    {
        /// pre load all textures from all configs
        DAVA::NMaterial* material = *it;
        DAVA::uint32 currentConfig = material->GetCurrentConfigIndex();
        for (DAVA::uint32 i = 0; i < material->GetConfigCount(); ++i)
        {
            material->SetCurrentConfigIndex(i);
            material->PreBuildMaterial(DAVA::PASS_FORWARD);
        }

        material->SetCurrentConfigIndex(currentConfig);

        for (const DAVA::FastName& textureName : textureNames)
        {
            if ((*it)->HasLocalTexture(textureName))
            {
                DAVA::Texture* tex = (*it)->GetLocalTexture(textureName);
                if ((*it)->GetParent())
                {
                    texturesMap[tex] = DAVA::Format("Material: %s (parent - %s). Texture %s.", (*it)->GetMaterialName().c_str(), (*it)->GetParent()->GetMaterialName().c_str(), textureName.c_str());
                }
                else
                {
                    texturesMap[tex] = DAVA::Format("Material: %s. Texture %s.", (*it)->GetMaterialName().c_str(), textureName.c_str());
                }
            }
        }

        bool qualityGroupIsOk = false;
        DAVA::FastName materialGroup = (*it)->GetQualityGroup();

        // if some group is set in material we should check it exists in quality system
        if (materialGroup.IsValid())
        {
            size_t qcount = DAVA::QualitySettingsSystem::Instance()->GetMaterialQualityGroupCount();
            for (size_t q = 0; q < qcount; ++q)
            {
                if (materialGroup == DAVA::QualitySettingsSystem::Instance()->GetMaterialQualityGroupName(q))
                {
                    qualityGroupIsOk = true;
                    break;
                }
            }

            if (!qualityGroupIsOk)
            {
                errorsLog.insert(DAVA::Format("Material \"%s\" has unknown quality group \"%s\"", (*it)->GetMaterialName().c_str(), materialGroup.c_str()));
            }
        }

        if ((*it)->GetEffectiveFXName().IsValid() && materialTemplates && (*it)->GetEffectiveFXName() != DAVA::NMaterialName::SHADOW_VOLUME) //ShadowVolume material is non-assignable and it's okey
        {
            // ShadowVolume material is non-assignable and it's okey
            bool templateFound = false;
            for (int i = 0; i < materialTemplates->size(); ++i)
            {
                if (!strcmp(materialTemplates->at(i).path.toStdString().c_str(), (*it)->GetEffectiveFXName().c_str()))
                {
                    templateFound = true;
                    break;
                }
            }
            if (!templateFound)
            {
                errorsLog.insert(DAVA::Format("Material \"%s\" has non-assignable template", (*it)->GetMaterialName().c_str()));
            }
        }
    }

    auto endItTextures = texturesMap.end();
    for (auto it = texturesMap.begin(); it != endItTextures; ++it)
    {
        ValidateTexture(it->first, it->second, errorsLog);
    }
}

void SceneValidator::ValidateLandscape(DAVA::Landscape* landscape, DAVA::Set<DAVA::String>& errorsLog)
{
    if (nullptr == landscape)
        return;
    ValidateLandscapeTexture(landscape, DAVA::Landscape::TEXTURE_COLOR, errorsLog);
    ValidateLandscapeTexture(landscape, DAVA::Landscape::TEXTURE_TILE, errorsLog);
    ValidateLandscapeTexture(landscape, DAVA::Landscape::TEXTURE_TILEMASK, errorsLog);

    //validate heightmap
    bool pathIsCorrect = ValidatePathname(landscape->GetHeightmapPathname(), DAVA::String("Landscape. Heightmap."));
    if (!pathIsCorrect)
    {
        DAVA::String path = landscape->GetHeightmapPathname().GetRelativePathname(ProjectManager::Instance()->GetDataSourcePath());
        PushLogMessage(errorsLog, nullptr, "Wrong path of Heightmap: %s. Scene: %s", path.c_str(), sceneName.c_str());
    }
}

void SceneValidator::ValidateLandscapeTexture(DAVA::Landscape* landscape, const DAVA::FastName& texLevel, DAVA::Set<DAVA::String>& errorsLog)
{
    DAVA::Texture* texture = landscape->GetMaterial()->GetEffectiveTexture(texLevel);
    if (texture)
    {
        DAVA::FilePath landTexName = landscape->GetMaterial()->GetEffectiveTexture(texLevel)->GetPathname();
        if (!IsTextureDescriptorPath(landTexName) && landTexName.GetAbsolutePathname().size() > 0)
        {
            texture->SetPathname(DAVA::TextureDescriptor::GetDescriptorPathname(landTexName));
        }

        ValidateTexture(texture, DAVA::Format("Landscape. %s", texLevel.c_str()), errorsLog);
    }
}

DAVA::VariantType* SceneValidator::GetCustomPropertyFromParentsTree(DAVA::Entity* ownerNode, const DAVA::String& key)
{
    DAVA::KeyedArchive* props = GetCustomPropertiesArchieve(ownerNode);
    if (nullptr == props)
    {
        return 0;
    }

    if (props->IsKeyExists(key))
    {
        return props->GetVariant(key);
    }
    else
    {
        return GetCustomPropertyFromParentsTree(ownerNode->GetParent(), key);
    }
}

bool SceneValidator::NodeRemovingDisabled(DAVA::Entity* node)
{
    DAVA::KeyedArchive* customProperties = GetCustomPropertiesArchieve(node);
    return (customProperties && customProperties->IsKeyExists(ResourceEditor::EDITOR_DO_NOT_REMOVE));
}

void SceneValidator::ValidateTextureAndShowErrors(DAVA::Texture* texture, const DAVA::String& validatedObjectName)
{
    errorMessages.clear();

    ValidateTexture(texture, validatedObjectName, errorMessages);
    ShowErrorDialog(errorMessages);
}

void SceneValidator::ValidateTexture(DAVA::Texture* texture, const DAVA::String& validatedObjectName, DAVA::Set<DAVA::String>& errorsLog)
{
    if (nullptr == texture)
    {
        return;
    }

    const DAVA::FilePath& texturePathname = texture->GetPathname();

    DAVA::String path = texturePathname.GetRelativePathname(pathForChecking);
    DAVA::String textureInfo = path + " for object: " + validatedObjectName;

    if (texture->IsPinkPlaceholder())
    {
        if (texturePathname.IsEmpty())
        {
            PushLogMessage(errorsLog, nullptr, "Texture not set for object: %s. Scene: %s",
                           validatedObjectName.c_str(), sceneName.c_str());
        }
        else
        {
            PushLogMessage(errorsLog, nullptr, "Can't load texture: %s. Scene: %s",
                           textureInfo.c_str(), sceneName.c_str());
        }
        return;
    }

    bool pathIsCorrect = ValidatePathname(texturePathname, validatedObjectName);
    if (!pathIsCorrect)
    {
        PushLogMessage(errorsLog, nullptr, "Wrong path of: %s. Scene: %s",
                       textureInfo.c_str(), sceneName.c_str());
        return;
    }

    if (!DAVA::IsPowerOf2(texture->GetWidth()) || !DAVA::IsPowerOf2(texture->GetHeight()))
    {
        PushLogMessage(errorsLog, nullptr, "Texture %s has now power of two dimensions. Scene: %s",
                       textureInfo.c_str(), sceneName.c_str());
    }

    if ((texture->GetWidth() > 2048) || (texture->GetHeight() > 2048))
    {
        PushLogMessage(errorsLog, nullptr, "Texture %s is too big. Scene: %s",
                       textureInfo.c_str(), sceneName.c_str());
    }
}

bool SceneValidator::WasTextureChanged(DAVA::Texture* texture, DAVA::eGPUFamily forGPU)
{
    if (IsFBOTexture(texture))
    {
        return false;
    }

    DAVA::FilePath texturePathname = texture->GetPathname();
    return (IsPathCorrectForProject(texturePathname) && IsTextureChanged(texturePathname, forGPU));
}

bool SceneValidator::IsFBOTexture(DAVA::Texture* texture)
{
    if (texture->isRenderTarget)
    {
        return true;
    }

    DAVA::String::size_type textTexturePos = texture->GetPathname().GetAbsolutePathname().find("Text texture");
    if (DAVA::String::npos != textTexturePos)
    {
        return true; //is text texture
    }

    return false;
}

DAVA::FilePath SceneValidator::SetPathForChecking(const DAVA::FilePath& pathname)
{
    DAVA::FilePath oldPath = pathForChecking;
    pathForChecking = pathname;
    return oldPath;
}

bool SceneValidator::ValidateTexturePathname(const DAVA::FilePath& pathForValidation, DAVA::Set<DAVA::String>& errorsLog)
{
    DVASSERT_MSG(!pathForChecking.IsEmpty(), "Need to set pathname for DataSource folder");

    bool pathIsCorrect = IsPathCorrectForProject(pathForValidation);
    if (pathIsCorrect)
    {
        DAVA::String textureExtension = pathForValidation.GetExtension();
        if (!DAVA::TextureDescriptor::IsSupportedTextureExtension(textureExtension))
        {
            PushLogMessage(errorsLog, nullptr, "Path %s has incorrect extension. Scene: %s",
                           pathForValidation.GetAbsolutePathname().c_str(), sceneName.c_str());
            return false;
        }
    }
    else
    {
        PushLogMessage(errorsLog, nullptr, "Path %s is incorrect for project %s. Scene: %s",
                       pathForValidation.GetAbsolutePathname().c_str(), pathForChecking.GetAbsolutePathname().c_str(), sceneName.c_str());
    }

    return pathIsCorrect;
}

bool SceneValidator::ValidateHeightmapPathname(const DAVA::FilePath& pathForValidation, DAVA::Set<DAVA::String>& errorsLog)
{
    DVASSERT_MSG(!pathForChecking.IsEmpty(), "Need to set pathname for DataSource folder");

    bool pathIsCorrect = IsPathCorrectForProject(pathForValidation);
    if (pathIsCorrect)
    {
        auto extension = pathForValidation.GetExtension();

        bool isSourceTexture = false;
        bool isHeightmap = false;
        if (!extension.empty())
        {
            if (DAVA::TextureDescriptor::IsSourceTextureExtension(extension))
                isSourceTexture = true;
            else if (DAVA::CompareCaseInsensitive(extension, DAVA::Heightmap::FileExtension()) == 0)
                isHeightmap = true;
        }

        pathIsCorrect = isSourceTexture || isHeightmap;
        if (!pathIsCorrect)
        {
            PushLogMessage(errorsLog, nullptr, "Heightmap path %s is wrong. Scene: %s",
                           pathForValidation.GetAbsolutePathname().c_str(), sceneName.c_str());
            return false;
        }

        DAVA::ScopedPtr<DAVA::Heightmap> heightmap(new DAVA::Heightmap());
        if (isSourceTexture)
        {
            DAVA::ScopedPtr<DAVA::Image> image(CreateTopLevelImage(pathForValidation));
            pathIsCorrect = heightmap->BuildFromImage(image);
        }
        else
        {
            pathIsCorrect = heightmap->Load(pathForValidation);
        }

        if (!pathIsCorrect)
        {
            PushLogMessage(errorsLog, nullptr, "Can't load Heightmap from path %s. Scene: %s",
                           pathForValidation.GetAbsolutePathname().c_str(), sceneName.c_str());
            return false;
        }

        pathIsCorrect = DAVA::IsPowerOf2(heightmap->Size() - 1);
        if (!pathIsCorrect)
        {
            PushLogMessage(errorsLog, nullptr, "Heightmap %s has wrong size. Scene: %s",
                           pathForValidation.GetAbsolutePathname().c_str(), sceneName.c_str());
        }

        return pathIsCorrect;
    }
    else
    {
        PushLogMessage(errorsLog, nullptr, "Path %s is incorrect for project %s.",
                       pathForValidation.GetAbsolutePathname().c_str(), pathForChecking.GetAbsolutePathname().c_str());
    }

    return pathIsCorrect;
}

bool SceneValidator::ValidatePathname(const DAVA::FilePath& pathForValidation, const DAVA::String& validatedObjectName)
{
    DVASSERT(!pathForChecking.IsEmpty());
    //Need to set path to DataSource/3d for path correction
    //Use SetPathForChecking();

    DAVA::String pathname = pathForValidation.GetAbsolutePathname();

    DAVA::String::size_type fboFound = pathname.find(DAVA::String("FBO"));
    DAVA::String::size_type resFound = pathname.find(DAVA::String("~res:"));
    if ((DAVA::String::npos != fboFound) || (DAVA::String::npos != resFound))
    {
        return true;
    }

    return IsPathCorrectForProject(pathForValidation);
}

bool SceneValidator::IsPathCorrectForProject(const DAVA::FilePath& pathname)
{
    DAVA::String normalizedPath = pathname.GetAbsolutePathname();
    DAVA::String::size_type foundPos = normalizedPath.find(pathForChecking.GetAbsolutePathname());
    return (DAVA::String::npos != foundPos);
}

void SceneValidator::EnumerateNodes(DAVA::Scene* scene)
{
    DAVA::int32 nodesCount = 0;
    if (scene != nullptr)
    {
        for (DAVA::int32 i = 0; i < scene->GetChildrenCount(); ++i)
        {
            nodesCount += EnumerateSceneNodes(scene->GetChild(i));
        }
    }
}

DAVA::int32 SceneValidator::EnumerateSceneNodes(DAVA::Entity* node)
{
    //TODO: lode node can have several nodes at layer

    DAVA::int32 nodesCount = 1;
    for (DAVA::int32 i = 0; i < node->GetChildrenCount(); ++i)
    {
        nodesCount += EnumerateSceneNodes(node->GetChild(i));
    }

    return nodesCount;
}

bool SceneValidator::IsTextureChanged(const DAVA::FilePath& texturePathname, DAVA::eGPUFamily forGPU)
{
    bool isChanged = false;

    DAVA::TextureDescriptor* descriptor = DAVA::TextureDescriptor::CreateFromFile(texturePathname);
    if (descriptor != nullptr)
    {
        isChanged = IsTextureChanged(descriptor, forGPU);
        delete descriptor;
    }

    return isChanged;
}

bool SceneValidator::IsTextureChanged(const DAVA::TextureDescriptor* descriptor, DAVA::eGPUFamily forGPU)
{
    DVASSERT(descriptor);

    return !descriptor->IsCompressedTextureActual(forGPU);
}

bool SceneValidator::IsTextureDescriptorPath(const DAVA::FilePath& path)
{
    return path.IsEqualToExtension(DAVA::TextureDescriptor::GetDescriptorExtension());
}

void SceneValidator::ValidateCustomColorsTexture(DAVA::Entity* landscapeEntity, DAVA::Set<DAVA::String>& errorsLog)
{
    DAVA::KeyedArchive* customProps = GetCustomPropertiesArchieve(landscapeEntity);
    if (customProps != nullptr && customProps->IsKeyExists(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP))
    {
        DAVA::String currentSaveName = customProps->GetString(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP);
        DAVA::FilePath path = "/" + currentSaveName;

        if (!DAVA::TextureDescriptor::IsSourceTextureExtension(path.GetExtension()))
        {
            PushLogMessage(errorsLog, landscapeEntity,
                           "Custom colors texture has to have .png, .jpeg or .tga extension. Scene: %s", sceneName.c_str());
        }

        DAVA::String::size_type foundPos = currentSaveName.find("DataSource/3d/");
        if (DAVA::String::npos == foundPos)
        {
            PushLogMessage(errorsLog, landscapeEntity,
                           "Custom colors texture has to begin from DataSource/3d/. Scene: %s", sceneName.c_str());
        }
    }
}

bool SceneValidator::ValidateColor(DAVA::Color& color)
{
    bool ok = true;
    for (DAVA::int32 i = 0; i < 4; ++i)
    {
        if (color.color[i] < 0.f || color.color[i] > 1.f)
        {
            color.color[i] = DAVA::Clamp(color.color[i], 0.f, 1.f);
            ok = false;
        }
    }

    return ok;
}

void SceneValidator::FindSwitchesWithDifferentLODs(DAVA::Entity* entity, DAVA::Set<DAVA::FastName>& names)
{
    if (IsEntityHasDifferentLODsCount(entity))
    {
        names.insert(entity->GetName());
    }
    else
    {
        const DAVA::uint32 count = entity->GetChildrenCount();
        for (DAVA::uint32 i = 0; i < count; ++i)
        {
            FindSwitchesWithDifferentLODs(entity->GetChild(i), names);
        }
    }
}

bool SceneValidator::IsEntityHasDifferentLODsCount(DAVA::Entity* entity)
{
    if ((GetSwitchComponent(entity) == NULL) || (GetLodComponent(entity) == NULL))
    {
        return false;
    }

    DAVA::RenderObject* ro = GetRenderObject(entity);
    if (ro != nullptr)
    {
        return IsObjectHasDifferentLODsCount(ro);
    }

    return false;
}

bool SceneValidator::IsObjectHasDifferentLODsCount(DAVA::RenderObject* renderObject)
{
    DVASSERT(renderObject);

    DAVA::int32 maxLod[2] = { -1, -1 };

    const DAVA::uint32 count = renderObject->GetRenderBatchCount();
    for (DAVA::uint32 i = 0; i < count; ++i)
    {
        DAVA::int32 lod, sw;
        renderObject->GetRenderBatch(i, lod, sw);

        DVASSERT(sw < 2);
        if ((lod > maxLod[sw]) && (sw >= 0 && sw < 2))
        {
            maxLod[sw] = lod;
        }
    }

    return ((maxLod[0] != maxLod[1]) && (maxLod[0] != -1 && maxLod[1] != -1));
}

void SceneValidator::ExtractEmptyRenderObjectsAndShowErrors(DAVA::Entity* entity, const EmptyRenderObjectCallback& callback)
{
    DAVA::Set<DAVA::String> errors;
    SceneValidator::ExtractEmptyRenderObjects(entity, errors, callback);
    if (!errors.empty())
    {
        ShowErrorDialog(errors);
    }
}

void SceneValidator::ExtractEmptyRenderObjects(DAVA::Entity* entity, DAVA::Set<DAVA::String>& errorsLog, const EmptyRenderObjectCallback& callback)
{
    auto renderObject = GetRenderObject(entity);
    if ((nullptr != renderObject) && (0 == renderObject->GetRenderBatchCount()) && DAVA::RenderObject::TYPE_MESH == renderObject->GetType())
    {
        callback(entity, renderObject);
        entity->RemoveComponent(DAVA::Component::RENDER_COMPONENT);
        PushLogMessage(errorsLog, entity, "Entity %s has empty render object", entity->GetName().c_str());
    }

    const DAVA::uint32 count = entity->GetChildrenCount();
    for (DAVA::uint32 i = 0; i < count; ++i)
    {
        ExtractEmptyRenderObjects(entity->GetChild(i), errorsLog, callback);
    }
}
