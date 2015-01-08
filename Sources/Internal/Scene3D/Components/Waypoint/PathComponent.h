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



#ifndef __DAVAENGINE_PATH_COMPONENT_H__
#define __DAVAENGINE_PATH_COMPONENT_H__

#include "Entity/Component.h"
#include "Base/Introspection.h"

namespace DAVA
{

class SerializationContext;
class KeyedArchive;
class Entity;
    
class PathComponent : public Component
{
public:
    
    struct Edge;
    struct Waypoint: public InspBase
    {
        Waypoint();
        ~Waypoint();
        
        Vector3 position;
        KeyedArchive *properties;
        Vector<Edge *> edges;
        
        void AddEdge(Edge *edge);
        void RemoveEdge(Edge *edge);
        
        INTROSPECTION(Waypoint,
            MEMBER(position, "Waypoint position", I_SAVE | I_EDIT | I_VIEW)
            MEMBER(properties, "Waypoint Properties", I_SAVE | I_EDIT | I_VIEW)
            COLLECTION(edges, "Edges", I_SAVE | I_VIEW | I_EDIT)
        );
    };
    
    struct Edge: public InspBase
    {
        Edge();
        ~Edge();
        
        Waypoint * destination;
        KeyedArchive *properties;
        
        INTROSPECTION(Edge,
            MEMBER(destination, "Destination", I_SAVE | I_EDIT | I_VIEW)
            MEMBER(properties, "Edge Properties", I_SAVE | I_EDIT | I_VIEW)
        );
    };


protected:
    ~PathComponent();
public:
	IMPLEMENT_COMPONENT_TYPE(PATH_COMPONENT);

	PathComponent();
	virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SerializationContext *serializationContext);
	virtual void Deserialize(KeyedArchive *archive, SerializationContext *serializationContext);

    void AddPoint(Waypoint *point);
    void RemovePoint(Waypoint *point);
    void RemoveAllPoints();
    
    Waypoint * GetWaypoint(const FastName & name);
    const Vector<Waypoint *> & GetPoints() const;
    
    void SetName(const FastName & name);
    const FastName & GetName() const;
    
    void Reset();
    
private:
    
    FastName name;
    Vector<Waypoint *> waypoints;

public:
	INTROSPECTION_EXTEND(PathComponent, Component,
        MEMBER(name, "Name", I_SAVE | I_VIEW | I_EDIT)
        COLLECTION(waypoints, "Waypoints", I_SAVE | I_VIEW | I_EDIT)
    );
};

inline const Vector<PathComponent::Waypoint *> & PathComponent::GetPoints() const
{
    return waypoints;
}

inline void PathComponent::SetName(const FastName & _name)
{
    name = _name;
}
    
inline const FastName & PathComponent::GetName() const
{
    return name;
}

    
    
}
#endif //__DAVAENGINE_PATH_COMPONENT_H__
