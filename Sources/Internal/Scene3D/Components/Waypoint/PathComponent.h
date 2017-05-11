#pragma once

#include "Entity/Component.h"
#include "Reflection/Reflection.h"
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
    struct Waypoint : public InspBase
    {
        Waypoint();
        ~Waypoint();
        Waypoint(const Waypoint&);

        FastName name;
        Vector3 position;
        Vector<Edge*> edges;
        bool isStarting = false;

    private:
        KeyedArchive* properties;

    public:
        void AddEdge(Edge* edge);
        void RemoveEdge(Edge* edge);

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

        DAVA_VIRTUAL_REFLECTION(Waypoint, InspBase);
    };

    struct Edge : public InspBase
    {
        Edge();
        ~Edge();
        Edge(const Edge&);

        Waypoint* destination = nullptr;

    private:
        KeyedArchive* properties = nullptr;

        //For property panel
        void SetDestinationName(const FastName& name);
        const FastName GetDestinationName() const;

        void SetDestinationPoint(const Vector3& point);
        const Vector3 GetDestinationPoint() const;

    public:
        void SetProperties(KeyedArchive* p);
        KeyedArchive* GetProperties() const;

        INTROSPECTION(Edge,
                      PROPERTY("DestinationName", "Destination Name", GetDestinationName, SetDestinationName, I_VIEW)
                      PROPERTY("DestinationPoint", "Destination Point", GetDestinationPoint, SetDestinationPoint, I_VIEW)
                      MEMBER(properties, "Edge Properties", I_SAVE | I_EDIT | I_VIEW)
                      );

        DAVA_VIRTUAL_REFLECTION(Edge, InspBase);
    };

public:
    IMPLEMENT_COMPONENT_TYPE(PATH_COMPONENT);

    PathComponent();
    virtual ~PathComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void AddPoint(Waypoint* point);
    void InsertPoint(Waypoint* point, uint32 beforeIndex);
    void RemovePoint(Waypoint* point);
    void ExtractPoint(Waypoint* point);

    Waypoint* GetWaypoint(const FastName& name);
    const Vector<Waypoint*>& GetPoints() const;
    Waypoint* GetStartWaypoint() const;

    void SetName(const FastName& name);
    const FastName& GetName() const;

    void SetColor(const Color& color);
    const Color& GetColor() const;

    void Reset();

private:
    uint32 GetWaypointIndex(const Waypoint* point);

    FastName name;
    Color color;
    Vector<Waypoint*> waypoints;

public:
    INTROSPECTION_EXTEND(PathComponent, Component,
                         MEMBER(name, "Name", I_SAVE | I_VIEW | I_EDIT)
                         MEMBER(color, "Color", I_SAVE | I_VIEW | I_EDIT)
                         );

    DAVA_VIRTUAL_REFLECTION(PathComponent, Component);
};

inline void PathComponent::Waypoint::SetStarting(bool val)
{
    isStarting = val;
}

inline bool PathComponent::Waypoint::IsStarting() const
{
    return isStarting;
}

inline const Vector<PathComponent::Waypoint*>& PathComponent::GetPoints() const
{
    return waypoints;
}

inline void PathComponent::SetName(const FastName& _name)
{
    name = _name;
    for (Waypoint* wp : waypoints)
    {
        wp->name = name;
    }
}

inline const FastName& PathComponent::GetName() const
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
