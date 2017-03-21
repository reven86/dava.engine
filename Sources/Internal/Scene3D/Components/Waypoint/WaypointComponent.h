#pragma once

#include "Entity/Component.h"
#include "Reflection/Reflection.h"
#include "Base/Introspection.h"

namespace DAVA
{
class SerializationContext;
class KeyedArchive;
class Entity;

class WaypointComponent : public Component
{
protected:
    ~WaypointComponent();

public:
    IMPLEMENT_COMPONENT_TYPE(WAYPOINT_COMPONENT);

    WaypointComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void SetProperties(KeyedArchive* archieve);
    KeyedArchive* GetProperties() const;

    void SetPathName(const FastName& name);
    const FastName& GetPathName() const;

    void SetStarting(bool);
    bool IsStarting() const;

private:
    FastName pathName;
    KeyedArchive* properties;
    bool isStarting = false;

public:
    INTROSPECTION_EXTEND(WaypointComponent, Component,
                         MEMBER(pathName, "Path Name", I_VIEW)
                         MEMBER(properties, "Waypoint properties", I_VIEW | I_EDIT)
                         );

    DAVA_VIRTUAL_REFLECTION(WaypointComponent, Component);
};

inline KeyedArchive* WaypointComponent::GetProperties() const
{
    return properties;
}

inline void WaypointComponent::SetPathName(const FastName& name)
{
    pathName = name;
}

inline const FastName& WaypointComponent::GetPathName() const
{
    return pathName;
}

inline void WaypointComponent::SetStarting(bool newVal)
{
    isStarting = newVal;
}

inline bool WaypointComponent::IsStarting() const
{
    return isStarting;
}
}
