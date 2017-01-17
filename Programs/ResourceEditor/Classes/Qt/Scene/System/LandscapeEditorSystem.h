#pragma once

#include "DAVAEngine.h"
#include "LandscapeEditorDrawSystem.h"

class SceneCollisionSystem;
class EntityModificationSystem;

class LandscapeEditorSystem : public DAVA::SceneSystem
{
public:
    LandscapeEditorSystem(DAVA::Scene* scene, const DAVA::FilePath& cursorPathname);
    ~LandscapeEditorSystem() override;

    bool IsLandscapeEditingEnabled() const;

protected:
    LandscapeEditorDrawSystem::eErrorType IsCanBeEnabled() const;

    void UpdateCursorPosition();

protected:
    SceneCollisionSystem* collisionSystem;
    EntityModificationSystem* modifSystem;
    LandscapeEditorDrawSystem* drawSystem;
    DAVA::Vector2 cursorPosition;
    DAVA::Vector2 prevCursorPos;
    DAVA::Texture* cursorTexture = nullptr;
    DAVA::float32 cursorSize = 0.0f;
    DAVA::float32 landscapeSize = 0.0f;
    bool isIntersectsLandscape = false;
    bool enabled = false;
};
