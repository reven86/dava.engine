#include "Base/BaseTypes.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Commands2/ConvertPathCommands.h"
#include "Commands2/RECommandIDs.h"

ExpandPathCommand::ExpandPathCommand(DAVA::PathComponent* pathComponent)
    : RECommand(CMDID_EXPAND_PATH, "Expand entity path")
    , pathEntity(nullptr)
{
    DVASSERT(pathComponent);
    pathEntity = pathComponent->GetEntity();
    DVASSERT(pathEntity);

    SafeRetain(pathEntity);

    // create waypoint entities (waypoint entity + waypoint component) and entity add commands
    MapWaypoint2Entity mapWaypoint2Entity;
    const DAVA::Vector<DAVA::PathComponent::Waypoint*>& waypoints = pathComponent->GetPoints();
    DAVA::uint32 waypointsCount = static_cast<DAVA::uint32>(waypoints.size());
    entityAddCommands.reserve(waypointsCount);
    for (DAVA::uint32 wpIdx = 0; wpIdx < waypointsCount; ++wpIdx)
    {
        DAVA::PathComponent::Waypoint* waypoint = waypoints[wpIdx];
        DVASSERT(waypoint);

        DAVA::ScopedPtr<DAVA::Entity> wpEntity(CreateWaypointEntity(waypoint, pathComponent->GetName()));
        mapWaypoint2Entity[waypoint] = wpEntity;
        entityAddCommands.push_back(new EntityAddCommand(wpEntity, pathEntity));
    }

    // add edge components
    MapWaypoint2Entity::const_iterator it = mapWaypoint2Entity.begin();
    MapWaypoint2Entity::const_iterator mapEnd = mapWaypoint2Entity.end();
    for (; it != mapEnd; ++it)
    {
        DAVA::PathComponent::Waypoint* const waypoint = it->first;
        DAVA::Entity* const wpEntity = it->second;
        DVASSERT(waypoint);
        DVASSERT(wpEntity);

        DAVA::uint32 edgesCount = waypoint->edges.size();
        for (DAVA::uint32 edgeIdx = 0; edgeIdx < edgesCount; ++edgeIdx)
        {
            DAVA::PathComponent::Edge* edge = waypoint->edges[edgeIdx];
            DVASSERT(edge);

            DAVA::PathComponent::Waypoint* destination = edge->destination;
            DVASSERT(destination);

            DAVA::Entity* destinationEntity = mapWaypoint2Entity[destination];
            DVASSERT(destinationEntity);

            DAVA::EdgeComponent* edgeComponent = new DAVA::EdgeComponent();

            edgeComponent->SetProperties(edge->GetProperties());
            edgeComponent->SetNextEntity(destinationEntity);
            wpEntity->AddComponent(edgeComponent);
        }
    }
}

ExpandPathCommand::~ExpandPathCommand()
{
    SafeRelease(pathEntity);

    for (EntityAddCommand* cmd : entityAddCommands)
    {
        DAVA::SafeDelete(cmd);
    }
    entityAddCommands.clear();
}

DAVA::Entity* ExpandPathCommand::CreateWaypointEntity(const DAVA::PathComponent::Waypoint* waypoint, const DAVA::FastName& pathname)
{
    DVASSERT(waypoint);

    DAVA::Entity* wpEntity = new DAVA::Entity();
    DAVA::WaypointComponent* wpComponent = new DAVA::WaypointComponent();

    wpComponent->SetPathName(pathname);
    wpComponent->SetProperties(waypoint->GetProperties());
    wpEntity->AddComponent(wpComponent);
    wpEntity->SetName(waypoint->name);

    if (waypoint->IsStarting())
    {
        wpComponent->SetStarting(true);
        wpEntity->SetNotRemovable(true);
    }

    DAVA::Matrix4 m;
    m.SetTranslationVector(waypoint->position);
    wpEntity->SetLocalTransform(m);

    return wpEntity;
}

void ExpandPathCommand::Undo()
{
    DAVA::uint32 count = static_cast<DAVA::uint32>(entityAddCommands.size());
    for (DAVA::int32 i = count - 1; i >= 0; --i)
    {
        entityAddCommands[i]->Undo();
    }
}

void ExpandPathCommand::Redo()
{
    DAVA::uint32 count = static_cast<DAVA::uint32>(entityAddCommands.size());
    for (DAVA::uint32 i = 0; i < count; ++i)
    {
        entityAddCommands[i]->Redo();
    }
}

DAVA::Entity* ExpandPathCommand::GetEntity() const
{
    return pathEntity;
}

DAVA::PathComponent::Waypoint* NewWaypoint()
{
    return new DAVA::PathComponent::Waypoint;
}

CollapsePathCommand::CollapsePathCommand(DAVA::PathComponent* pathComponent)
    : RECommand(CMDID_COLLAPSE_PATH, "Collapse path entities")
    , pathEntity(nullptr)
    , origPathComponent(pathComponent)
    , destPathComponent(nullptr)
    , addNextComponent(nullptr)
    , removePrevComponent(nullptr)
{
    DVASSERT(origPathComponent);
    pathEntity = origPathComponent->GetEntity();
    DVASSERT(pathEntity);

    SafeRetain(pathEntity);

    destPathComponent = new DAVA::PathComponent();

    const DAVA::FastName& pathName = origPathComponent->GetName();
    destPathComponent->SetName(pathName);
    destPathComponent->SetColor(origPathComponent->GetColor());

    addNextComponent = new AddComponentCommand(pathEntity, destPathComponent);
    removePrevComponent = new RemoveComponentCommand(pathEntity, origPathComponent);

    // define the list of entities to collapse
    DAVA::uint32 entityCount = 0;
    MapEntity2uint mapEntity2uint;
    EntityList entitiesToCollapse;
    pathEntity->GetChildEntitiesWithComponent(entitiesToCollapse, DAVA::Component::WAYPOINT_COMPONENT);
    EntityList::const_iterator end = entitiesToCollapse.end();
    for (EntityList::iterator it = entitiesToCollapse.begin(); it != end;)
    {
        DAVA::Entity* wpEntity = *it;
        DVASSERT(wpEntity);

        DAVA::WaypointComponent* wpComponent = GetWaypointComponent(wpEntity);
        DVASSERT(wpComponent);

        if (wpComponent->GetPathName() == pathName)
        {
            ++it;
            entityRemoveCommands.push_back(new EntityRemoveCommand(wpEntity));
            mapEntity2uint[wpEntity] = entityCount++;
        }
        else
        {
            EntityList::iterator itDel = it++;
            entitiesToCollapse.erase(itDel);
        }
    }

    // create final waypoints
    DAVA::Vector<DAVA::PathComponent::Waypoint*> waypointsVec;
    waypointsVec.resize(entityCount);
    std::generate(waypointsVec.begin(), waypointsVec.end(), NewWaypoint);

    // assign final waypoints and edges
    EntityList::iterator itEntity = entitiesToCollapse.begin();
    EntityList::const_iterator itEnd = entitiesToCollapse.end();
    for (DAVA::uint32 i = 0; i < entityCount && itEntity != itEnd; ++i, ++itEntity)
    {
        DAVA::Entity* wpEntity = *itEntity;
        DAVA::WaypointComponent* wpComponent = GetWaypointComponent(wpEntity);

        DAVA::PathComponent::Waypoint* waypoint = waypointsVec[i];
        DVASSERT(waypoint);

        destPathComponent->AddPoint(waypoint);

        // init waypoint
        InitWaypoint(waypoint, wpEntity, wpComponent);

        // init waypoint edges
        DAVA::uint32 count = wpEntity->GetComponentCount(DAVA::Component::EDGE_COMPONENT);
        for (DAVA::uint32 iEdge = 0; iEdge < count; ++iEdge)
        {
            DAVA::EdgeComponent* edgeComponent = static_cast<DAVA::EdgeComponent*>((*itEntity)->GetComponent(DAVA::Component::EDGE_COMPONENT, iEdge));
            DVASSERT(edgeComponent);

            DAVA::Entity* destEntity = edgeComponent->GetNextEntity();
            DVASSERT(destEntity);
            MapEntity2uint::const_iterator itFoundEntity = mapEntity2uint.find(destEntity);
            if (itFoundEntity != mapEntity2uint.end())
            {
                DAVA::uint32 destIdx = itFoundEntity->second;
                DVASSERT(destIdx < entityCount);

                DAVA::PathComponent::Edge* edge = new DAVA::PathComponent::Edge;
                edge->destination = waypointsVec[destIdx];
                edge->SetProperties(edgeComponent->GetProperties());

                waypointsVec[i]->AddEdge(edge);
            }
        }
    }
}

CollapsePathCommand::~CollapsePathCommand()
{
    SafeRelease(pathEntity);

    DAVA::SafeDelete(addNextComponent);
    DAVA::SafeDelete(removePrevComponent);

    for (EntityRemoveCommand* cmd : entityRemoveCommands)
    {
        DAVA::SafeDelete(cmd);
    }
    entityRemoveCommands.clear();
}

void CollapsePathCommand::InitWaypoint(DAVA::PathComponent::Waypoint* waypoint, DAVA::Entity* wpEntity, DAVA::WaypointComponent* wpComponent)
{
    waypoint->name = wpEntity->GetName();
    waypoint->SetProperties(wpComponent->GetProperties());
    waypoint->SetStarting(wpComponent->IsStarting());
    DAVA::Matrix4 m = wpEntity->GetLocalTransform();
    waypoint->position = m.GetTranslationVector();
}

void CollapsePathCommand::Redo()
{
    DAVA::uint32 count = static_cast<DAVA::uint32>(entityRemoveCommands.size());
    for (DAVA::uint32 i = 0; i < count; ++i)
    {
        entityRemoveCommands[i]->Redo();
    }
    addNextComponent->Redo();
    removePrevComponent->Redo();
}

void CollapsePathCommand::Undo()
{
    removePrevComponent->Undo();
    addNextComponent->Undo();
    DAVA::uint32 count = static_cast<DAVA::uint32>(entityRemoveCommands.size());
    for (DAVA::int32 i = count - 1; i >= 0; --i)
    {
        entityRemoveCommands[i]->Undo();
    }
}

DAVA::Entity* CollapsePathCommand::GetEntity() const
{
    return pathEntity;
}
