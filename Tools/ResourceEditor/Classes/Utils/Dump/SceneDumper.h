#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "Render/RenderBase.h"

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
    enum class eMode
    {
        REQUIRED = 0,
        EXTENDED
    };

    static DAVA::Set<DAVA::FilePath> DumpLinks(const DAVA::FilePath& scenePath, eMode mode, const DAVA::Vector<DAVA::eGPUFamily>& compressedGPUs);

private:
    SceneDumper(const DAVA::FilePath& scenePath, eMode mode, const DAVA::Vector<DAVA::eGPUFamily>& compressedGPUs);
    ~SceneDumper();

    void DumpLinksRecursive(DAVA::Entity* entity, DAVA::Set<DAVA::FilePath>& links) const;

    void DumpCustomProperties(DAVA::KeyedArchive* properties, DAVA::Set<DAVA::FilePath>& links) const;
    void DumpRenderObject(DAVA::RenderObject* renderObject, DAVA::Set<DAVA::FilePath>& links) const;
    void DumpMaterial(DAVA::NMaterial* material, DAVA::Set<DAVA::FilePath>& links, DAVA::Set<DAVA::FilePath>& descriptorPathnames) const;
    void DumpEffect(DAVA::ParticleEffectComponent* effect, DAVA::Set<DAVA::FilePath>& links) const;
    void DumpEmitter(DAVA::ParticleEmitterInstance* emitter, DAVA::Set<DAVA::FilePath>& links, DAVA::Set<DAVA::FilePath>& gfxFolders) const;

    DAVA::Scene* scene = nullptr;
    DAVA::FilePath scenePathname;

    DAVA::Vector<DAVA::eGPUFamily> compressedGPUs;
    SceneDumper::eMode mode = SceneDumper::eMode::REQUIRED;
};
