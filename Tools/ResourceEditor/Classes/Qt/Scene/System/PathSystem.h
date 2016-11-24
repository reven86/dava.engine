#pragma once

#include "Entity/SceneSystem.h"
#include "Base/FastName.h"

#include "Scene3D/Components/Waypoint/PathComponent.h"

#include "Scene/System/ModifSystem.h"
#include "Classes/Qt/Scene/System/EditorSceneSystem.h"

static const DAVA::uint32 WAYPOINTS_DRAW_LIFTING = 1;

class RECommandNotificationObject;
class SceneEditor2;
class PathSystem : public DAVA::SceneSystem, public EntityModificationSystemDelegate, public EditorSceneSystem
{
public:
    PathSystem(DAVA::Scene* scene);
    ~PathSystem() override;

    void EnablePathEdit(bool enable);
    bool IsPathEditEnabled() const;

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    void Process(DAVA::float32 timeElapsed) override;

    void ProcessCommand(const RECommandNotificationObject& commandNotification) override;
    void Draw() override;

    DAVA::Entity* GetCurrrentPath() const;
    const DAVA::Vector<DAVA::Entity*>& GetPathes() const;

    void AddPath(DAVA::Entity* pathEntity);

    DAVA::PathComponent* CreatePathComponent();

    void WillClone(DAVA::Entity* originalEntity) override;
    void DidCloned(DAVA::Entity* originalEntity, DAVA::Entity* newEntity) override;

protected:
    void DrawInEditableMode();
    void DrawInViewOnlyMode();
    void DrawArrow(const DAVA::Vector3& start, const DAVA::Vector3& finish, const DAVA::Color& color);

    DAVA::FastName GeneratePathName() const;
    const DAVA::Color& GetNextPathColor() const;

    void ExpandPathEntity(const DAVA::Entity*);
    void CollapsePathEntity(const DAVA::Entity*);

    DAVA::Vector<DAVA::Entity*> pathes;

    SelectableGroup currentSelection;
    DAVA::Entity* currentPath;

    bool isEditingEnabled;
};

inline const DAVA::Vector<DAVA::Entity*>& PathSystem::GetPathes() const
{
    return pathes;
}

inline DAVA::Entity* PathSystem::GetCurrrentPath() const
{
    return currentPath;
}

inline bool PathSystem::IsPathEditEnabled() const
{
    return isEditingEnabled;
}
