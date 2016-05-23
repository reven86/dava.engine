#ifndef __PATH_SYSTEM_H__
#define __PATH_SYSTEM_H__

#include "Entity/SceneSystem.h"
#include "Base/FastName.h"

#include "Scene/System/SelectionSystem.h"
#include "Scene3D/Components/Waypoint/PathComponent.h"

#include "Scene/System/ModifSystem.h"

static const DAVA::uint32 WAYPOINTS_DRAW_LIFTING = 1;

class SceneEditor2;
class PathSystem : public DAVA::SceneSystem, public EntityModificationSystemDelegate
{
    friend class SceneEditor2;

public:
    PathSystem(DAVA::Scene* scene);
    ~PathSystem() override;

    void EnablePathEdit(bool enable);
    bool IsPathEditEnabled() const;

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    void Process(DAVA::float32 timeElapsed) override;

    DAVA::Entity* GetCurrrentPath() const;
    const DAVA::Vector<DAVA::Entity*>& GetPathes() const;

    void AddPath(DAVA::Entity* pathEntity);

    DAVA::PathComponent* CreatePathComponent();

    void WillClone(DAVA::Entity* originalEntity) override;
    void DidCloned(DAVA::Entity* originalEntity, DAVA::Entity* newEntity) override;

protected:
    void Draw();
    void DrawInEditableMode();
    void DrawInViewOnlyMode();
    void DrawArrow(const DAVA::Vector3& start, const DAVA::Vector3& finish, const DAVA::Color& color);

    void ProcessCommand(const Command2* command, bool redo);

    DAVA::FastName GeneratePathName() const;
    const DAVA::Color& GetNextPathColor() const;

    void ExpandPathEntity(const DAVA::Entity*);
    void CollapsePathEntity(const DAVA::Entity*);

    SceneEditor2* sceneEditor;

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

#endif // __PATH_SYSTEM_H__
