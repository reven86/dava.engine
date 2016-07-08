#ifndef __LANDSCAPE_EDITOR_SYSTEM__
#define __LANDSCAPE_EDITOR_SYSTEM__

#include "DAVAEngine.h"
#include "LandscapeEditorDrawSystem.h"

class SceneCollisionSystem;
class SceneSelectionSystem;
class EntityModificationSystem;

class LandscapeEditorSystem : public DAVA::SceneSystem
{
public:
    LandscapeEditorSystem(DAVA::Scene* scene, const DAVA::FilePath& cursorPathname);
    virtual ~LandscapeEditorSystem();

    bool IsLandscapeEditingEnabled() const;

protected:
    LandscapeEditorDrawSystem::eErrorType IsCanBeEnabled() const;

    void UpdateCursorPosition();

protected:
    SceneCollisionSystem* collisionSystem;
    SceneSelectionSystem* selectionSystem;
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

#endif //__LANDSCAPE_EDITOR_SYSTEM__
