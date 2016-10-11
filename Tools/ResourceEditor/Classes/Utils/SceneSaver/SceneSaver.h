#pragma once

#include "FileSystem/FilePath.h"
#include "Render/Texture.h"
#include "Scene3D/Components/ParticleEffectComponent.h"

#include "Utils/SceneUtils/SceneUtils.h"

namespace DAVA
{
class Entity;
class Scene;
class ParticleEmitter;
}

class SceneSaver
{
public:
    SceneSaver();
    virtual ~SceneSaver();

    void SetInFolder(const DAVA::FilePath& folderPathname);
    void SetOutFolder(const DAVA::FilePath& folderPathname);

    void SaveFile(const DAVA::String& fileName);
    void ResaveFile(const DAVA::String& fileName);
    void SaveScene(DAVA::Scene* scene, const DAVA::FilePath& fileName);

    void EnableCopyConverted(bool enabled);

    void ResaveYamlFilesRecursive(const DAVA::FilePath& folder) const;

private:
    void ReleaseTextures();

    void CopyTextures(DAVA::Scene* scene);
    void CopyTexture(const DAVA::FilePath& texturePathname);

    void CopyReferencedObject(DAVA::Entity* node);
    void CopyEffects(DAVA::Entity* node);
    void CopyAllParticlesEmitters(DAVA::ParticleEmitterInstance* instance);
    void CopyEmitterByPath(const DAVA::FilePath& emitterConfigPath);
    void CopyEmitter(DAVA::ParticleEmitter* emitter);
    DAVA::Set<DAVA::FilePath> EnumAlternativeEmittersFilepaths(const DAVA::FilePath& originalFilepath) const;

    void CopyCustomColorTexture(DAVA::Scene* scene, const DAVA::FilePath& sceneFolder);

    SceneUtils sceneUtils;
    DAVA::TexturesMap texturesForSave;
    DAVA::Set<DAVA::FilePath> effectFolders;
    bool copyConverted = false;
};
