#ifndef __SCENE_DUMPER_H__
#define __SCENE_DUMPER_H__

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
class Scene;
class Entity;
class RenderObject;
class KeyedArchive;
class ParticleEffectComponent;
class ParticleEmitter;
class ParticleEmitterInstance;
class NMaterial;
}

class SceneDumper
{
public:
    using SceneLinks = DAVA::Set<DAVA::FilePath>;

    static SceneLinks DumpLinks(const DAVA::FilePath& scenePath);

private:
    SceneDumper(const DAVA::FilePath& scenePath);
    ~SceneDumper();

    void DumpLinksRecursive(DAVA::Entity* entity, SceneLinks& links) const;

    void DumpCustomProperties(DAVA::KeyedArchive* properties, SceneLinks& links) const;
    void DumpRenderObject(DAVA::RenderObject* renderObject, SceneLinks& links) const;
    void DumpMaterial(DAVA::NMaterial* material, SceneLinks& links, DAVA::Set<DAVA::FilePath>& descriptorPathnames) const;
    void DumpEffect(DAVA::ParticleEffectComponent* effect, SceneLinks& links) const;
    void DumpEmitter(DAVA::ParticleEmitterInstance* emitter, SceneLinks& links, SceneLinks& gfxFolders) const;

    DAVA::Scene* scene = nullptr;
    DAVA::FilePath scenePathname;
};


#endif // __SCENE_DUMPER_H__
