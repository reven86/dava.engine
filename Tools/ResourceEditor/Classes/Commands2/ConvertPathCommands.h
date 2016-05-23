#ifndef __CONVERT_PATH_COMMANDS_H__
#define __CONVERT_PATH_COMMANDS_H__

#include "FileSystem/KeyedArchive.h"
#include "Commands2/Base/Command2.h"

#include "Scene3D/Components/Waypoint/PathComponent.h"
#include "Scene3D/Components/Waypoint/WaypointComponent.h"
#include "Scene3D/Components/Waypoint/EdgeComponent.h"
#include "Commands2/EntityAddCommand.h"
#include "Commands2/EntityRemoveCommand.h"
#include "Commands2/AddComponentCommand.h"
#include "Commands2/RemoveComponentCommand.h"

class ExpandPathCommand : public Command2
{
public:
    ExpandPathCommand(DAVA::PathComponent* _pathComponent);
    ~ExpandPathCommand();

    virtual void Undo();
    virtual void Redo();
    virtual DAVA::Entity* GetEntity() const;

protected:
    typedef DAVA::Map<DAVA::PathComponent::Waypoint*, DAVA::Entity*> MapWaypoint2Entity;

    DAVA::Entity* CreateWaypointEntity(const DAVA::PathComponent::Waypoint* waypoint, const DAVA::FastName& name);

    DAVA::Entity* pathEntity;
    DAVA::Vector<EntityAddCommand*> entityAddCommands;
};

class CollapsePathCommand : public Command2
{
public:
    CollapsePathCommand(DAVA::PathComponent* _pathComponent);
    ~CollapsePathCommand();

    virtual void Undo();
    virtual void Redo();
    virtual DAVA::Entity* GetEntity() const;

protected:
    DAVA::PathComponent::Waypoint* CreateWaypoint(DAVA::Entity* entity);
    void InitWaypoint(DAVA::PathComponent::Waypoint* waypoint, DAVA::Entity* wpEntity, DAVA::WaypointComponent* wpComponent);

    typedef DAVA::Map<DAVA::Entity*, DAVA::uint32> MapEntity2uint;
    typedef DAVA::List<DAVA::Entity*> EntityList;

    DAVA::Entity* pathEntity;
    DAVA::PathComponent* origPathComponent;
    DAVA::PathComponent* destPathComponent;
    DAVA::Vector<EntityRemoveCommand*> entityRemoveCommands;
    AddComponentCommand* addNextComponent;
    RemoveComponentCommand* removePrevComponent;
};

#endif // __CONVERT_PATH_COMMANDS_H__
