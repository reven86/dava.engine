/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Base/BaseTypes.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Commands2/ConvertPathCommands.h"

ExpandPathCommand::ExpandPathCommand(DAVA::PathComponent* pathComponent)
    : Command2(CMDID_EXPAND_PATH, "Expand entity path")
    , pathEntity(nullptr)
{
    DVASSERT(pathComponent);
    pathEntity = pathComponent->GetEntity();
    DVASSERT(pathEntity);

    SafeRetain(pathEntity);

    // create waypoint entities (waypoint entity + waypoint component) and entity add commands
    MapWaypoint2Entity mapWaypoint2Entity;
    const DAVA::Vector<DAVA::PathComponent::Waypoint *> & waypoints = pathComponent->GetPoints();
    DAVA::uint32 waypointsCount = waypoints.size();
    entityAddCommands.reserve(waypointsCount);
    for (DAVA::uint32 wpIdx=0; wpIdx < waypointsCount; ++wpIdx)
    {
        DAVA::PathComponent::Waypoint * waypoint = waypoints[wpIdx];
        DVASSERT(waypoint);

        DAVA::ScopedPtr<DAVA::Entity> wpEntity(CreateWaypointEntity(waypoint,pathComponent->GetName()));
        mapWaypoint2Entity[waypoint] = wpEntity;
        entityAddCommands.push_back(new EntityAddCommand(wpEntity, pathEntity));
    }

    // add edge components
    MapWaypoint2Entity::const_iterator it = mapWaypoint2Entity.begin();
    MapWaypoint2Entity::const_iterator mapEnd = mapWaypoint2Entity.end();
    for (; it != mapEnd; ++it)
    {
        DAVA::PathComponent::Waypoint * const waypoint = it->first;
        DAVA::Entity * const wpEntity = it->second;
        DVASSERT(waypoint);
        DVASSERT(wpEntity);

        DAVA::uint32 edgesCount = waypoint->edges.size();
        for (DAVA::uint32 edgeIdx=0; edgeIdx < edgesCount; ++edgeIdx)
        {
            DAVA::PathComponent::Edge * edge = waypoint->edges[edgeIdx];
            DVASSERT(edge);

            DAVA::PathComponent::Waypoint * destination = edge->destination;
            DVASSERT(destination);

            DAVA::Entity * destinationEntity = mapWaypoint2Entity[destination];
            DVASSERT(destinationEntity);

            DAVA::EdgeComponent * edgeComponent = new DAVA::EdgeComponent();

            edgeComponent->SetProperties(edge->GetProperties());
            edgeComponent->SetNextEntity(destinationEntity);
            wpEntity->AddComponent(edgeComponent);
        }
    }
}

ExpandPathCommand::~ExpandPathCommand()
{
	SafeRelease(pathEntity);

    DAVA::uint32 count = entityAddCommands.size();
    for (DAVA::uint32 i=0; i<count; ++i)
    {
        DAVA::SafeDelete(entityAddCommands[i]);
    }
}

DAVA::Entity* ExpandPathCommand::CreateWaypointEntity(const DAVA::PathComponent::Waypoint* waypoint, const DAVA::FastName & pathname)
{
    DVASSERT(waypoint);

    DAVA::Entity* wpEntity = new DAVA::Entity();
    DAVA::WaypointComponent * wpComponent = new DAVA::WaypointComponent();

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
    DAVA::uint32 count = entityAddCommands.size();
    for (DAVA::int32 i=count-1; i >= 0; --i)
    {
        UndoInternalCommand(entityAddCommands[i]);
    }
}

void ExpandPathCommand::Redo()
{
    DAVA::uint32 count = entityAddCommands.size();
    for (DAVA::uint32 i=0; i<count; ++i)
    {
        RedoInternalCommand(entityAddCommands[i]);
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
    : Command2(CMDID_COLLAPSE_PATH, "Collapse path entities")
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
    for(EntityList::iterator it=entitiesToCollapse.begin(); it != end;)
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
    std::generate(waypointsVec.begin(),waypointsVec.end(),NewWaypoint);

    // assign final waypoints and edges
    EntityList::iterator itEntity = entitiesToCollapse.begin();
    EntityList::const_iterator itEnd = entitiesToCollapse.end();
    for (DAVA::uint32 i=0; i < entityCount && itEntity != itEnd; ++i, ++itEntity)
    {
        DAVA::Entity* wpEntity = *itEntity;
        DAVA::WaypointComponent* wpComponent = GetWaypointComponent(wpEntity);

        DAVA::PathComponent::Waypoint* waypoint = waypointsVec[i];
        DVASSERT(waypoint);

        destPathComponent->AddPoint(waypoint);

        // init waypoint
        InitWaypoint(waypoint,wpEntity,wpComponent);

        // init waypoint edges
        DAVA::uint32 count = wpEntity->GetComponentCount(DAVA::Component::EDGE_COMPONENT);
        for (DAVA::uint32 iEdge=0; iEdge < count; ++iEdge)
        {
            DAVA::EdgeComponent* edgeComponent = static_cast<DAVA::EdgeComponent*>((*itEntity)->GetComponent(DAVA::Component::EDGE_COMPONENT,iEdge));
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

    DAVA::uint32 count = entityRemoveCommands.size();
    for (DAVA::uint32 i=0; i<count; ++i)
    {
        DAVA::SafeDelete(entityRemoveCommands[i]);
    }
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
    DAVA::uint32 count = entityRemoveCommands.size();
    for (DAVA::uint32 i=0; i < count; ++i)
    {
        RedoInternalCommand(entityRemoveCommands[i]);
    }

    RedoInternalCommand(addNextComponent);
    RedoInternalCommand(removePrevComponent);
}

void CollapsePathCommand::Undo()
{
    UndoInternalCommand(removePrevComponent);
    UndoInternalCommand(addNextComponent);

    DAVA::uint32 count = entityRemoveCommands.size();
    for (DAVA::int32 i=count-1; i >= 0; --i)
    {
        UndoInternalCommand(entityRemoveCommands[i]);
    }
}

DAVA::Entity* CollapsePathCommand::GetEntity() const
{
    return pathEntity;
}
