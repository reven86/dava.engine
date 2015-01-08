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



#ifndef __CONVERT_PATH_COMMANDS_H__
#define __CONVERT_PATH_COMMANDS_H__

#include "FileSystem/KeyedArchive.h"
#include "Commands2/Command2.h"
#include "Scene3D/Components/Waypoint/PathComponent.h"
#include "Scene3D/Components/Waypoint/WaypointComponent.h"

typedef DAVA::Map<DAVA::PathComponent::Waypoint*, DAVA::Entity*> MapWaypoint2Entity;
typedef DAVA::Map<DAVA::PathComponent::Waypoint*, DAVA::uint32> MapWaypoint2uint;
typedef DAVA::Map<DAVA::Entity*, DAVA::uint32> MapEntity2uint;
typedef DAVA::List<DAVA::Entity*> EntityList;

class ExpandPathCommand : public Command2
{
public:
    ExpandPathCommand(DAVA::Entity* parentEntity, DAVA::PathComponent* _pathComponent);
    ~ExpandPathCommand();

    virtual void Undo();
    virtual void Redo();
    virtual DAVA::Entity* GetEntity() const;

protected:
    DAVA::Entity* CreateWaypointEntity(const DAVA::PathComponent::Waypoint* waypoint, const DAVA::FastName & name);

    DAVA::Entity* parentEntity;
    DAVA::PathComponent* pathComponent;
    MapWaypoint2Entity mapWaypoint2Entity;
};

class CollapsePathCommand : public Command2
{
public:
    CollapsePathCommand(DAVA::Entity* parentEntity, DAVA::FastName & pathName);
    ~CollapsePathCommand();

    virtual void Undo();
    virtual void Redo();
    virtual DAVA::Entity* GetEntity() const;

protected:
    struct MyEdge
    {
        MyEdge(DAVA::uint32 id, DAVA::KeyedArchive* ar) : wpId(id), properties(ar) {}
        DAVA::uint32 wpId;
        DAVA::KeyedArchive* properties;
    };
    struct MyWaypoint
    {
        DAVA::Vector3 position;
        DAVA::KeyedArchive* properties;
        DAVA::List<MyEdge> edges;
    };

protected:
    DAVA::PathComponent::Waypoint* CreateWaypoint(DAVA::Entity* entity);
    void InitMyWaypoint(MyWaypoint& myWaypoint, DAVA::PathComponent::Waypoint* waypoint);
    void InitMyWaypoint(MyWaypoint& myWaypoint, DAVA::Entity* wpEntity, DAVA::WaypointComponent* wpComponent);
    void ReconstructWaypointHierarchy(const DAVA::Vector<MyWaypoint>& waypointsVec);

    DAVA::Entity* parentEntity;
    DAVA::FastName pathName;
    DAVA::PathComponent* destPathComponent;
    EntityList entitiesToCollapse;
    DAVA::Vector<MyWaypoint> waypointsOriginState;
    DAVA::Vector<MyWaypoint> waypointsFinalState;
};

#endif // __CONVERT_PATH_COMMANDS_H__
