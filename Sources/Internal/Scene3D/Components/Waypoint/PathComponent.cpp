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



#include "Scene3D/Components/Waypoint/PathComponent.h"
#include "Scene3D/Entity.h"

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

void PathComponent::Waypoint::AddEdge(PathComponent::Edge *edge)
{
    edges.push_back(edge);
}

void PathComponent::Waypoint::RemoveEdge(PathComponent::Edge *edge)
{
    uint32 edgesCount = edges.size();
    for(uint32 e = 0; e < edgesCount; ++e)
    {
        if(edge == edges[e])
        {
            SafeDelete(edges[e]);
            edges.erase(edges.begin() + e);
            break;
        }
    }
}

    
//== Edge ==
PathComponent::Edge::Edge()
{
    destination = NULL;
    properties = NULL;
}

PathComponent::Edge::~Edge()
{
    SafeDelete(destination);
    SafeRelease(properties);
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

Component * PathComponent::Clone(Entity * toEntity)
{
	PathComponent * newComponent = new PathComponent();
	newComponent->SetEntity(toEntity);
    
    ScopedPtr<KeyedArchive> archieve(new KeyedArchive());
    SerializationContext context;
    context.SetDebugLogEnabled(false);
    context.SetVersion(SCENE_FILE_CURRENT_VERSION);
    
    Serialize(archieve, &context);
    newComponent->Deserialize(archieve, &context);

    return newComponent;
}

void PathComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
    Component::Serialize(archive, serializationContext);

    if(NULL != archive)
    {
        archive->SetFastName("name", name);
        
        
        const uint32 waypointCount = waypoints.size();
        const Waypoint *firstWaypoint = waypoints.front();
        
        archive->SetUInt32("waypointCount", waypointCount);
        for(uint32 w = 0; w < waypointCount; ++w)
        {
            const Waypoint *wp = waypoints[w];
            
            KeyedArchive * wpArchieve = new KeyedArchive();

            wpArchieve->SetVector3("position", wp->position);
            if(wp->properties)
            {
                wpArchieve->SetArchive("properties", wp->properties);
            }
            
            const uint32 edgesCount = wp->edges.size();
            wpArchieve->SetUInt32("edgesCount", edgesCount);
            for(uint32 e = 0; e < edgesCount; ++e)
            {
                Edge *edge = wp->edges[e];
                
                KeyedArchive * edgeArchieve = new KeyedArchive();
                if(edge->properties)
                {
                    edgeArchieve->SetArchive("properties", edge->properties);
                }

                DVASSERT(edge->destination);
                edgeArchieve->SetUInt32("destination", (edge->destination - firstWaypoint)); //index in waypoints array

                archive->SetArchive(Format("edge_%d", e), edgeArchieve);
                SafeRelease(edgeArchieve);
            }
            
            archive->SetArchive(Format("waypoint_%d", w), wpArchieve);
            SafeRelease(wpArchieve);
        }
    }
}

void PathComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
    Component::Deserialize(archive, serializationContext);

    if(archive == NULL) return;
    
    
    DVASSERT(waypoints.size() == 0);
    Reset();

    name = archive->GetFastName("name");
    
    const uint32 waypointCount = archive->GetUInt32("waypointCount");
    if(!waypointCount) return;
    
    waypoints.resize(waypointCount);
    for(uint32 w = 0; w < waypointCount; ++w)
    {
        waypoints.push_back(new Waypoint());
    }

    for(uint32 w = 0; w < waypointCount; ++w)
    {
        Waypoint *wp = waypoints[w];
        
        KeyedArchive * wpArchieve = archive->GetArchive(Format("waypoint_%d", w));
        DVASSERT(wpArchieve);
        
        wp->position = wpArchieve->GetVector3("position");
        
        KeyedArchive *wpProperties = wpArchieve->GetArchive("properties");
        if(wpProperties)
        {
            wp->properties = new KeyedArchive(*wpProperties);
        }
        
        const uint32 edgesCount = wpArchieve->GetUInt32("edgesCount");
        for(uint32 e = 0; e < edgesCount; ++e)
        {
            Edge *edge = new Edge();
            
            KeyedArchive * edgeArchieve = archive->GetArchive(Format("edge_%d", e));
            DVASSERT(edgeArchieve);
            
            KeyedArchive *edgeProperties = edgeArchieve->GetArchive("properties");
            if(edgeProperties)
            {
                edge->properties = new KeyedArchive(*edgeProperties);
            }
            
            uint32 index = edgeArchieve->GetUInt32("destination");
            DVASSERT(index < waypointCount);
            edge->destination = waypoints[index];
            
            wp->edges.push_back(edge);
        }
    }
}

void PathComponent::AddPoint(DAVA::PathComponent::Waypoint *point)
{
    waypoints.push_back(point);
}

void PathComponent::RemovePoint(DAVA::PathComponent::Waypoint *point)
{
    uint32 waypointCount = waypoints.size();
    for(uint32 w = 0; w < waypointCount; ++w)
    {
        Waypoint *wp = waypoints[w];
        
        uint32 edgesCount = wp->edges.size();
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
    const uint32 waypointCount = waypoints.size();
    for(uint32 w = 0; w < waypointCount; ++w)
    {
        Waypoint *wp = waypoints[w];
        if(wp->properties && (wp->properties->GetFastName("name") == name))
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
    
}