#ifndef __EDITOR_LIGHT_SYSTEM_H__
#define __EDITOR_LIGHT_SYSTEM_H__

#include "DAVAEngine.h"

class Command2;
class EditorLightSystem final : public DAVA::SceneSystem
{
    friend class SceneEditor2;
    friend class EditorScene;

public:
    EditorLightSystem(DAVA::Scene* scene);
    ~EditorLightSystem() override;

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    void SceneDidLoaded() override;

    void Process(DAVA::float32 timeElapsed) override;

    void SetCameraLightEnabled(bool enabled);
    bool GetCameraLightEnabled() const;

private:
    void UpdateCameraLightState();
    void UpdateCameraLightPosition();

    void AddCameraLightOnScene();
    void RemoveCameraLightFromScene();

private:
    DAVA::Entity* cameraLight = nullptr;
    DAVA::uint32 lightEntities = 0;
    bool isEnabled = true;
};

inline bool EditorLightSystem::GetCameraLightEnabled() const
{
    return isEnabled;
}


#endif // __EDITOR_LIGHT_SYSTEM_H__
