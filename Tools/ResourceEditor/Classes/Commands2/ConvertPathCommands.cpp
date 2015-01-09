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



#include "Commands2/ConvertPathCommands.h"
#include "Commands2/EntityAddCommand.h"
#include "Commands2/EntityRemoveCommand.h"

ExpandPathCommand::ExpandPathCommand(DAVA::Entity* _entity, DAVA::PathComponent* _pathComponent)
    : Command2(CMDID_EXPAND_PATH, "Expand entity path")
    , parentEntity(_entity)
    , pathComponent(_pathComponent)
{
    SafeRetain(parentEntity);

    if(parentEntity && pathComponent)
    {
        // create waypoint entities (waypoint entity + waypoint component)
        const DAVA::Vector<DAVA::PathComponent::Waypoint *> & waypoints = pathComponent->GetPoints();
        DAVA::uint32 waypointsCount = waypoints.size();
        for (DAVA::uint32 wpIdx=0; wpIdx < waypointsCount; ++wpIdx)
        {
            DAVA::PathComponent::Waypoint * waypoint = waypoints[wpIdx];
            DVASSERT(waypoint);

            DAVA::Entity * waypointEntity = CreateWaypointEntity(waypoint,pathComponent->GetName());
            mapWaypoint2Entity[waypoint] = waypointEntity;
        }

        // add edge components
        MapWaypoint2Entity::const_iterator it = mapWaypoint2Entity.begin();
        MapWaypoint2Entity::const_iterator mapEnd = mapWaypoint2Entity.end();
        for (; it != mapEnd; ++it)
        {
            DAVA::PathComponent::Waypoint * const waypoint = it->first;
            DAVA::Entity * const waypointEntity = it->second;
            DVASSERT(waypoint);
            DVASSERT(waypointEntity);

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
                DVASSERT(edgeComponent);

                edgeComponent->SetProperties(edge->properties);
                edgeComponent->SetNextEntity(destinationEntity);
                waypointEntity->AddComponent(edgeComponent);
            }
        }
    }
}

ExpandPathCommand::~ExpandPathCommand()
{
	SafeRelease(parentEntity);

    MapWaypoint2Entity::const_iterator it = mapWaypoint2Entity.begin();
    MapWaypoint2Entity::const_iterator mapEnd = mapWaypoint2Entity.end();
    for (; it != mapEnd; ++it)
    {
        SafeRelease((DAVA::Entity*)it->second);
    }
}

DAVA::Entity* ExpandPathCommand::CreateWaypointEntity(const DAVA::PathComponent::Waypoint* waypoint, const DAVA::FastName & name)
{
    DVASSERT(waypoint);

    DAVA::Entity* waypointEntity = new DAVA::Entity();
    DAVA::WaypointComponent * wpComponent = new DAVA::WaypointComponent();
    DVASSERT(waypointEntity);
    DVASSERT(wpComponent);

    wpComponent->SetPathName(name);
    wpComponent->SetProperties(waypoint->properties);
    waypointEntity->AddComponent(wpComponent);
    waypointEntity->SetName("waypoint");

    DAVA::Matrix4 pm = parentEntity->GetWorldTransform();
    pm.Inverse();

    DAVA::Matrix4 m;
    m.SetTranslationVector(waypoint->position);
    waypointEntity->SetLocalTransform(m * pm);

    return waypointEntity;
}

void ExpandPathCommand::Undo()
{
    MapWaypoint2Entity::const_iterator it = mapWaypoint2Entity.begin();
    MapWaypoint2Entity::const_iterator mapEnd = mapWaypoint2Entity.end();
    for (; it != mapEnd; ++it)
    {
        DAVA::Entity * const waypointEntity = it->second;
        DVASSERT(waypointEntity);

        parentEntity->RemoveNode(waypointEntity);
        UndoInternalCommand(new EntityRemoveCommand(waypointEntity));
    }
}

void ExpandPathCommand::Redo()
{
    MapWaypoint2Entity::const_iterator it = mapWaypoint2Entity.begin();
    MapWaypoint2Entity::const_iterator mapEnd = mapWaypoint2Entity.end();
    for (; it != mapEnd; ++it)
    {
        DAVA::Entity * const waypointEntity = it->second;
        DVASSERT(waypointEntity);

        parentEntity->AddNode(waypointEntity);
        RedoInternalCommand(new EntityAddCommand(waypointEntity, parentEntity));
    }
}

DAVA::Entity* ExpandPathCommand::GetEntity() const
{
    return parentEntity;
}

CollapsePathCommand::CollapsePathCommand(DAVA::Entity* _parentEntity, DAVA::FastName & _pathName)
    : Command2(CMDID_COLLAPSE_PATH, "Collapse path entities")
    , parentEntity(_parentEntity)
    , pathName(_pathName)
    
{
    if (parentEntity)
    {
        SafeRetain(parentEntity);

        // get destination PathComponent
        destPathComponent = NULL;
        DAVA::uint32 count = parentEntity->GetComponentCount(DAVA::Component::PATH_COMPONENT);
        for (DAVA::uint32 i=0; i<count; ++i)
        {
            DAVA::PathComponent* pathComponent = static_cast<DAVA::PathComponent*>(parentEntity->GetComponent(DAVA::Component::PATH_COMPONENT,i));
            DVASSERT(pathComponent);
            if (pathComponent->GetName()==pathName)
            {
                destPathComponent = pathComponent;
                break;
            }
        }

        if (!destPathComponent)
        {
            destPathComponent = new DAVA::PathComponent;
            DVASSERT(destPathComponent);
            destPathComponent->SetName(pathName);
            parentEntity->AddComponent(destPathComponent);
        }

        // create origin waypoints hierarchy
        MapWaypoint2uint mapWaypoint2uint;
        const DAVA::Vector<DAVA::PathComponent::Waypoint *> & waypoints = destPathComponent->GetPoints();
        DAVA::uint32 waypointsCount = waypoints.size();
        waypointsOriginState.resize(waypointsCount);
        for (DAVA::uint32 wpIdx=0; wpIdx < waypointsCount; ++wpIdx)
        {
            DAVA::PathComponent::Waypoint * waypoint = waypoints[wpIdx];
            DVASSERT(waypoint);

            mapWaypoint2uint[waypoint] = wpIdx;
            InitMyWaypoint(waypointsOriginState[wpIdx],waypoint);
        }

        // assign origin edges
        for (DAVA::uint32 wpIdx=0; wpIdx<waypointsCount; ++wpIdx)
        {
            DAVA::PathComponent::Waypoint * waypoint = waypoints[wpIdx];
            MyWaypoint& myWaypoint = waypointsOriginState[wpIdx];

            DAVA::uint32 edgesCount = waypoint->edges.size();
            for (DAVA::uint32 i=0; i<edgesCount; ++i)
            {
                DAVA::PathComponent::Edge* edge = waypoint->edges[i];
                DVASSERT(edge);
                myWaypoint.edges.push_back(MyEdge(mapWaypoint2uint[edge->destination],edge->properties));
            }
        }

        // create final waypoints hierarchy
        DAVA::uint32 entityCount=0;
        MapEntity2uint mapEntity2uint;
        parentEntity->GetChildEntitiesWithComponent(entitiesToCollapse, DAVA::Component::WAYPOINT_COMPONENT);
        EntityList::const_iterator end = entitiesToCollapse.end();
        for(EntityList::iterator it=entitiesToCollapse.begin(); it != end;)
        {
            DAVA::Entity* wpEntity = *it;
            DVASSERT(wpEntity);

            DAVA::WaypointComponent* wpComponent = static_cast<DAVA::WaypointComponent*>(wpEntity->GetComponent(DAVA::Component::WAYPOINT_COMPONENT,0));
            DVASSERT(wpComponent);

            if (wpComponent->GetPathName() == pathName)
            {
                SafeRetain(wpEntity);
                ++it;

                mapEntity2uint[wpEntity] = entityCount++;
                waypointsFinalState.push_back(MyWaypoint());
                InitMyWaypoint(waypointsFinalState.back(),wpEntity,wpComponent);
            }
            else
            {
                EntityList::iterator itDel = it++;
                entitiesToCollapse.erase(itDel);
            }
        }

        // assign final edges
        EntityList::iterator itEntity = entitiesToCollapse.begin();
        EntityList::const_iterator itEnd = entitiesToCollapse.end();
        for (DAVA::uint32 iEntity=0; iEntity < entityCount && itEntity != itEnd; ++iEntity, ++itEntity)
        {
            DAVA::uint32 count = (*itEntity)->GetComponentCount(DAVA::Component::EDGE_COMPONENT);
            for (DAVA::uint32 iEdge=0; iEdge < count; ++iEdge)
            {
                DAVA::EdgeComponent* edgeComponent = static_cast<DAVA::EdgeComponent*>((*itEntity)->GetComponent(DAVA::Component::EDGE_COMPONENT,iEdge));
                DVASSERT(edgeComponent);

                DAVA::Entity* destEntity = edgeComponent->GetNextEntity();
                DVASSERT(destEntity);
                waypointsFinalState[iEntity].edges.push_back(MyEdge(mapEntity2uint[destEntity], edgeComponent->GetProperties()));
            }
        }
    }
}

CollapsePathCommand::~CollapsePathCommand()
{
    SafeRelease(parentEntity);
    EntityList::const_iterator end = entitiesToCollapse.end();
    for(EntityList::iterator itEntity=entitiesToCollapse.begin(); itEntity != end; ++itEntity)
    {
        SafeRelease(*itEntity);
    }
}

void CollapsePathCommand::InitMyWaypoint(MyWaypoint& myWaypoint, DAVA::PathComponent::Waypoint* waypoint)
{
    myWaypoint.properties = waypoint->properties;
    myWaypoint.position = waypoint->position;
}

void CollapsePathCommand::InitMyWaypoint(MyWaypoint& myWaypoint, DAVA::Entity* wpEntity, DAVA::WaypointComponent* wpComponent)
{
    myWaypoint.properties = wpComponent->GetProperties();
    DAVA::Matrix4 m = wpEntity->GetLocalTransform() * parentEntity->GetWorldTransform();
    myWaypoint.position = m.GetTranslationVector();
}

void CollapsePathCommand::ReconstructWaypointHierarchy(const DAVA::Vector<MyWaypoint>& myWaypointsVec)
{
    if (destPathComponent)
    {
        destPathComponent->RemoveAllPoints();

        DAVA::uint32 wpCount = myWaypointsVec.size();

        DAVA::Vector<DAVA::PathComponent::Waypoint*> waypointsVec;
        waypointsVec.reserve(wpCount);

        // create waypoints
        for (DAVA::uint32 wpIdx=0; wpIdx < wpCount; ++wpIdx)
        {
            waypointsVec.push_back(new DAVA::PathComponent::Waypoint);
        }

        // init waypoints & adges
        for (DAVA::uint32 wpIdx=0; wpIdx < wpCount; ++wpIdx)
        {
            const MyWaypoint& myWaypoint = myWaypointsVec[wpIdx];
            DAVA::PathComponent::Waypoint* waypoint = waypointsVec[wpIdx];
            DVASSERT(waypoint);

            destPathComponent->AddPoint(waypoint);

            waypoint->properties = myWaypoint.properties;
            waypoint->position = myWaypoint.position;

            DAVA::List<MyEdge>::const_iterator it = myWaypoint.edges.begin();
            DAVA::List<MyEdge>::const_iterator end = myWaypoint.edges.end();
            for (; it != end; ++it)
            {
                DAVA::PathComponent::Edge* edge = new DAVA::PathComponent::Edge();
                DVASSERT(edge);
                edge->properties = it->properties;
                edge->destination = waypointsVec[it->wpId];
                waypoint->AddEdge(edge);
            }
        }
    }
}

void CollapsePathCommand::Redo()
{
    if (destPathComponent)
    {
        EntityList::const_iterator end = entitiesToCollapse.end();
        for(EntityList::iterator it=entitiesToCollapse.begin(); it != end; ++it)
        {
            DAVA::Entity* const nextEntity = *it;
            DVASSERT(nextEntity);

            parentEntity->RemoveNode(nextEntity);
            RedoInternalCommand(new EntityRemoveCommand(nextEntity));
        }

        ReconstructWaypointHierarchy(waypointsFinalState);
    }
}

void CollapsePathCommand::Undo()
{
    if (destPathComponent)
    {
        EntityList::const_iterator end = entitiesToCollapse.end();
        for(EntityList::iterator it=entitiesToCollapse.begin(); it != end; ++it)
        {
            DAVA::Entity* const nextEntity = *it;
            DVASSERT(nextEntity);

            parentEntity->AddNode(nextEntity);
            UndoInternalCommand(new EntityAddCommand(nextEntity,parentEntity));
        }

        ReconstructWaypointHierarchy(waypointsOriginState);
    }
}

DAVA::Entity* CollapsePathCommand::GetEntity() const
{
    return parentEntity;
}
