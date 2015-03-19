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



#include "Utils/Utils.h"
#include "Scene3D/Components/Waypoint/PathComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/VersionInfo.h"


namespace DAVA
{

//== Waypoint ==
PathComponent::Waypoint::Waypoint()
{
    properties = NULL;
}

    
PathComponent::Waypoint::~Waypoint()
{
    for_each(edges.begin(), edges.end(), SafeDelete<PathComponent::Edge>);
    SafeRelease(properties);
}

PathComponent::Waypoint::Waypoint(const Waypoint& cp)
{
    name = cp.name;
    position = cp.position;
    edges = cp.edges;
    isStarting = cp.isStarting;
    SetProperties(cp.GetProperties());
}

void PathComponent::Waypoint::SetProperties(KeyedArchive* p)
{
    if (p != properties)
    {
        SafeRelease(properties);
        properties = SafeRetain(p);
    }
}

void PathComponent::Waypoint::AddEdge(PathComponent::Edge *edge)
{
    edges.push_back(edge);
}

void PathComponent::Waypoint::RemoveEdge(PathComponent::Edge *edge)
{
    uint32 edgesCount = static_cast<uint32>(edges.size());
    for(uint32 e = 0; e < edgesCount; ++e)
    {
        if(edge == edges[e])
        {
            SafeDelete(edges[e]);
            RemoveExchangingWithLast(edges,e);
            break;
        }
    }
}

    
//== Edge ==
PathComponent::Edge::Edge()
{
    properties = NULL;
    destination = NULL;
}

PathComponent::Edge::~Edge()
{
    SafeRelease(properties);
}

PathComponent::Edge::Edge(const Edge& cp)
{
    destination = cp.destination;
    SetProperties(cp.GetProperties());
}

void PathComponent::Edge::SetProperties(KeyedArchive* p)
{
    if (p != properties)
    {
        SafeRelease(properties);
        properties = SafeRetain(p);
    }
}

void PathComponent::Edge::SetDestinationName(const FastName & name)
{
    //do nothing
}

const FastName PathComponent::Edge::GetDestinationName() const
{
    if(destination)
    {
        return destination->name;
    }
    
    return FastName();
}

void PathComponent::Edge::SetDestinationPoint(const Vector3 & point)
{
    //do nothing
}

const Vector3 PathComponent::Edge::GetDestinationPoint() const
{
    if(destination)
    {
        return destination->position;
    }
    
    return Vector3();
}

    
    
//== PathComponent ==
PathComponent::PathComponent()
    :	Component()
{
}
    
PathComponent::~PathComponent()
{
    Reset();
}

PathComponent::Waypoint* NewWaypoint()
{
    return new PathComponent::Waypoint;
}

Component * PathComponent::Clone(Entity * toEntity)
{
	PathComponent * newComponent = new PathComponent();

    newComponent->SetName(name);
    newComponent->SetColor(color);
	newComponent->SetEntity(toEntity);

    const uint32 waypointCount = static_cast<uint32>(waypoints.size());
    if(waypointCount)
    {
        newComponent->waypoints.resize(waypointCount);
        std::generate(newComponent->waypoints.begin(), newComponent->waypoints.end(), NewWaypoint);

        for(uint32 w = 0; w < waypointCount; ++w)
        {
            const Waypoint* waypoint = waypoints[w];
            DVASSERT(waypoint);

            Waypoint* newWaypoint = newComponent->waypoints[w];
            DVASSERT(newWaypoint);

            newWaypoint->name = waypoint->name;
            newWaypoint->position = waypoint->position;
            newWaypoint->isStarting = waypoint->isStarting;
            newWaypoint->SetProperties(waypoint->GetProperties());

            const uint32 edgesCount = static_cast<uint32>(waypoint->edges.size());
            for(uint32 e = 0; e < edgesCount; ++e)
            {
                Edge *edge = waypoint->edges[e];
                DVASSERT(edge);
                DVASSERT(edge->destination);

                uint32 destWaypointIdx = GetWaypointIndex(edge->destination);
                DVASSERT(destWaypointIdx < waypointCount);

                Edge *newEdge = new Edge;

                newEdge->SetProperties(edge->GetProperties());
                newEdge->destination = newComponent->waypoints[destWaypointIdx];
                newWaypoint->AddEdge(newEdge);
            }
        }
    }

    return newComponent;
}

void PathComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
    Component::Serialize(archive, serializationContext);

    if(NULL != archive)
    {
        archive->SetFastName("name", name);
        archive->SetColor("color", color);
        
        const uint32 waypointCount = static_cast<uint32>(waypoints.size());
        archive->SetUInt32("waypointCount", waypointCount);

        if(waypointCount)
        {
            for(uint32 w = 0; w < waypointCount; ++w)
            {
                const Waypoint *wp = waypoints[w];
                
                KeyedArchive * wpArchieve = new KeyedArchive();

                wpArchieve->SetFastName("name", wp->name);
                wpArchieve->SetVector3("position", wp->position);
                wpArchieve->SetBool("isStarting", wp->isStarting);
                if(wp->GetProperties())
                {
                    wpArchieve->SetArchive("properties", wp->GetProperties());
                }
                
                const uint32 edgesCount = static_cast<uint32>(wp->edges.size());
                wpArchieve->SetUInt32("edgesCount", edgesCount);
                for(uint32 e = 0; e < edgesCount; ++e)
                {
                    Edge *edge = wp->edges[e];
                    
                    KeyedArchive * edgeArchieve = new KeyedArchive();
                    if(edge->GetProperties())
                    {
                        edgeArchieve->SetArchive("properties", edge->GetProperties());
                    }
                    
                    DVASSERT(edge->destination);
                    edgeArchieve->SetUInt32("destination", GetWaypointIndex(edge->destination)); //index in waypoints array
                    
                    wpArchieve->SetArchive(Format("edge_%d", e), edgeArchieve);
                    SafeRelease(edgeArchieve);
                }
                
                archive->SetArchive(Format("waypoint_%d", w), wpArchieve);
                SafeRelease(wpArchieve);
            }
        }
    }
}

    
uint32 PathComponent::GetWaypointIndex(const PathComponent::Waypoint * point)
{
    const uint32 waypointCount = (const uint32)waypoints.size();
    for(uint32 w = 0; w < waypointCount; ++w)
    {
        if(point == waypoints[w])
        {
            return w;
        }
    }
    
    DVASSERT(false);
    return -1;
}

void PathComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
    Component::Deserialize(archive, serializationContext);

    if(archive == NULL) return;
    
    
    DVASSERT(waypoints.size() == 0);
    Reset();

    name = archive->GetFastName("name");
    color = archive->GetColor("color");
    
    const uint32 waypointCount = archive->GetUInt32("waypointCount");
    if(!waypointCount) return;
    
    waypoints.resize(waypointCount);
    std::generate(waypoints.begin(), waypoints.end(), NewWaypoint);

    bool startingWaypointExists = false;

    for(uint32 w = 0; w < waypointCount; ++w)
    {
        Waypoint *wp = waypoints[w];
        
        KeyedArchive * wpArchieve = archive->GetArchive(Format("waypoint_%d", w));
        DVASSERT(wpArchieve);

        wp->name = wpArchieve->GetFastName("name");
        wp->position = wpArchieve->GetVector3("position");
        wp->isStarting = wpArchieve->GetBool("isStarting");
        if (wp->isStarting)
            startingWaypointExists = true;
        
        KeyedArchive *wpProperties = wpArchieve->GetArchive("properties");
        wp->SetProperties(wpProperties);
        
        const uint32 edgesCount = wpArchieve->GetUInt32("edgesCount");
        for(uint32 e = 0; e < edgesCount; ++e)
        {
            Edge *edge = new Edge();
            
            KeyedArchive * edgeArchieve = wpArchieve->GetArchive(Format("edge_%d", e));
            DVASSERT(edgeArchieve);
            
            KeyedArchive *edgeProperties = edgeArchieve->GetArchive("properties");
            edge->SetProperties(edgeProperties);
            
            uint32 index = edgeArchieve->GetUInt32("destination");
            DVASSERT(index < waypointCount);
            edge->destination = waypoints[index];
            
            wp->edges.push_back(edge);
        }
    }

    if (!startingWaypointExists && waypointCount > 0)
    {
        waypoints[0]->isStarting = true;
    }
}

void PathComponent::AddPoint(DAVA::PathComponent::Waypoint *point)
{
    waypoints.push_back(point);
}

void PathComponent::RemovePoint(DAVA::PathComponent::Waypoint *point)
{
    uint32 waypointCount = static_cast<uint32>(waypoints.size());
    for(uint32 w = 0; w < waypointCount; ++w)
    {
        Waypoint *wp = waypoints[w];
        
        uint32 edgesCount = static_cast<uint32>(wp->edges.size());
        for(uint32 e = 0; e < edgesCount; ++e)
        {
            Edge *edge = wp->edges[e];
            if(edge->destination == point)
            {
                SafeDelete(wp->edges[e]);
                wp->edges.erase(wp->edges.begin() + e);
                --e;
                --edgesCount;
            }
        }
        
        if(wp == point)
        {
            SafeDelete(waypoints[w]);
            waypoints.erase(waypoints.begin() + w);
            --w;
            --waypointCount;
        }
    }
}

PathComponent::Waypoint * PathComponent::GetWaypoint(const FastName & name)
{
    const uint32 waypointCount = static_cast<uint32>(waypoints.size());
    for(uint32 w = 0; w < waypointCount; ++w)
    {
        Waypoint *wp = waypoints[w];
        if(wp->GetProperties() && (wp->GetProperties()->GetFastName("name") == name))
        {
            return wp;
        }
    }

    return NULL;
}

void PathComponent::Reset()
{
    for_each(waypoints.begin(), waypoints.end(), SafeDelete<PathComponent::Waypoint>);
}

PathComponent::Waypoint* PathComponent::GetStartWaypoint() const
{
    auto found = find_if(waypoints.begin(), waypoints.end(), [](PathComponent::Waypoint* wp){return wp->IsStarting();});
    return (found == waypoints.end() ? nullptr : *found);
}
    
}