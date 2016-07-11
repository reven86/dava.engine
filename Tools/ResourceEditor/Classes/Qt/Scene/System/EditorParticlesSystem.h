#ifndef __EDITOR_PARTICLES_SYSTEM_H__
#define __EDITOR_PARTICLES_SYSTEM_H__

#include "Entity/SceneSystem.h"
#include "UI/UIEvent.h"

namespace DAVA
{
class Command;
}

class EditorParticlesSystem : public DAVA::SceneSystem
{
    friend class SceneEditor2;

public:
    EditorParticlesSystem(DAVA::Scene* scene);

    void RestartParticleEffects();

private:
    void Draw();

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    void ProcessCommand(const DAVA::Command* command, bool redo);

    void DrawDebugInfoForEffect(DAVA::Entity* effectEntity);
    void DrawEmitter(DAVA::ParticleEmitterInstance* emitter, DAVA::Entity* owner, bool selected);

    void DrawSizeCircle(DAVA::Entity* effectEntity, DAVA::ParticleEmitterInstance* emitter);
    void DrawSizeCircleShockWave(DAVA::Entity* effectEntity, DAVA::ParticleEmitterInstance* emitter);
    void DrawSizeBox(DAVA::Entity* effectEntity, DAVA::ParticleEmitterInstance* emitter);
    void DrawVectorArrow(DAVA::ParticleEmitterInstance* emitter, DAVA::Vector3 center);

private:
    DAVA::Vector<DAVA::Entity*> entities;
};


#endif /* defined(__EDITOR_PARTICLES_SYSTEM_H__) */
