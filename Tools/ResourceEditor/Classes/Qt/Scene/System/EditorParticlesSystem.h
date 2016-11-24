#pragma once

#include "Entity/SceneSystem.h"
#include "UI/UIEvent.h"

#include "Classes/Qt/Scene/System/EditorSceneSystem.h"

class RECommandNotificationObject;
class EditorParticlesSystem : public DAVA::SceneSystem, public EditorSceneSystem
{
    friend class SceneEditor2;

public:
    EditorParticlesSystem(DAVA::Scene* scene);

    void Draw() override;
    void ProcessCommand(const RECommandNotificationObject& commandNotification) override;

    void RestartParticleEffects();

    DAVA::ParticleLayer* GetForceOwner(DAVA::ParticleForce* force) const;
    DAVA::ParticleEmitterInstance* GetLayerOwner(DAVA::ParticleLayer* layer) const;

private:
    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    void DrawDebugInfoForEffect(DAVA::Entity* effectEntity);
    void DrawEmitter(DAVA::ParticleEmitterInstance* emitter, DAVA::Entity* owner, bool selected);

    void DrawSizeCircle(DAVA::Entity* effectEntity, DAVA::ParticleEmitterInstance* emitter);
    void DrawSizeCircleShockWave(DAVA::Entity* effectEntity, DAVA::ParticleEmitterInstance* emitter);
    void DrawSizeBox(DAVA::Entity* effectEntity, DAVA::ParticleEmitterInstance* emitter);
    void DrawVectorArrow(DAVA::ParticleEmitterInstance* emitter, DAVA::Vector3 center);

private:
    DAVA::Vector<DAVA::Entity*> entities;
};
