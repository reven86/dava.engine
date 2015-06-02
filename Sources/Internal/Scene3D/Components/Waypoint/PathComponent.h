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
#include "Math/Color.h"

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
        Waypoint(const Waypoint&);
        
        FastName name;
        Vector3 position;
        Vector<Edge *> edges;
        bool isStarting = false;
    private:
        KeyedArchive* properties;

    public:
        void AddEdge(Edge *edge);
        void RemoveEdge(Edge *edge);

        void SetProperties(KeyedArchive* p);
        KeyedArchive* GetProperties() const;

        void SetStarting(bool);
        bool IsStarting() const;

        INTROSPECTION(Waypoint,
            MEMBER(name, "Name", I_SAVE | I_VIEW | I_EDIT)
            MEMBER(position, "Waypoint position", I_SAVE | I_EDIT | I_VIEW)
            MEMBER(properties, "Waypoint Properties", I_SAVE | I_EDIT | I_VIEW)
            //MEMBER(isStarting, "Is waypoint starting", I_VIEW) // still editable on property editor. TODO: uncomment when fixed this
            COLLECTION(edges, "Edges", I_SAVE | I_VIEW | I_EDIT)
        );
    };
    
    struct Edge: public InspBase
    {
        Edge();
        ~Edge();
        Edge(const Edge&);
        
        Waypoint * destination;

    private:
        KeyedArchive* properties;

        //For property panel
        void SetDestinationName(const FastName & name);
        const FastName GetDestinationName() const;
        
        void SetDestinationPoint(const Vector3 & point);
        const Vector3 GetDestinationPoint() const;
        
    public:
        void SetProperties(KeyedArchive* p);
        KeyedArchive* GetProperties() const;
        
        INTROSPECTION(Edge,
            PROPERTY("DestinationName", "Destination Name", GetDestinationName, SetDestinationName, I_VIEW)
            PROPERTY("DestinationPoint", "Destination Point", GetDestinationPoint, SetDestinationPoint, I_VIEW)
            MEMBER(properties, "Edge Properties", I_SAVE | I_EDIT | I_VIEW)
        );
    };

public:
	IMPLEMENT_COMPONENT_TYPE(PATH_COMPONENT);

	PathComponent();
    virtual ~PathComponent();

    Component * Clone(Entity * toEntity) override;
    void Serialize(KeyedArchive *archive, SerializationContext *serializationContext) override;
    void Deserialize(KeyedArchive *archive, SerializationContext *serializationContext) override;

    void AddPoint(Waypoint *point);
    void RemovePoint(Waypoint *point);
    
    Waypoint * GetWaypoint(const FastName & name);
    const Vector<Waypoint *> & GetPoints() const;
    Waypoint* GetStartWaypoint() const;
    
    void SetName(const FastName & name);
    const FastName & GetName() const;

    void SetColor(const Color& color);
    const Color& GetColor() const;
    
    void Reset();
    
private:
    
    uint32 GetWaypointIndex(const Waypoint * point);
    
    
    FastName name;
    Color color;
    Vector<Waypoint *> waypoints;

public:
	INTROSPECTION_EXTEND(PathComponent, Component,
        MEMBER(name, "Name", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(color, "Color", I_SAVE | I_VIEW | I_EDIT)
    );
};

inline void PathComponent::Waypoint::SetStarting(bool val)
{
    isStarting = val;
}

inline bool PathComponent::Waypoint::IsStarting() const
{
    return isStarting;
}

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

inline void PathComponent::SetColor(const Color& val)
{
    color = val;
}

inline const Color& PathComponent::GetColor() const
{
    return color;
}

inline KeyedArchive* PathComponent::Waypoint::GetProperties() const
{
    return properties;
}

inline KeyedArchive* PathComponent::Edge::GetProperties() const
{
    return properties;
}


}
#endif //__DAVAENGINE_PATH_COMPONENT_H__
