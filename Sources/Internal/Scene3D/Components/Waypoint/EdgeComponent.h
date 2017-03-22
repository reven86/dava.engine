#pragma once

#include "Entity/Component.h"
#include "Reflection/Reflection.h"
#include "Base/Introspection.h"

namespace DAVA
{
class SerializationContext;
class KeyedArchive;
class Entity;

class EdgeComponent : public Component
{
protected:
    ~EdgeComponent();

public:
    IMPLEMENT_COMPONENT_TYPE(EDGE_COMPONENT);

    EdgeComponent();
    EdgeComponent(const EdgeComponent&);

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void SetNextEntity(Entity* entity);
    Entity* GetNextEntity() const;

    void SetProperties(KeyedArchive* archieve);
    KeyedArchive* GetProperties() const;

private:
    //For property panel
    void SetNextEntityName(const FastName& name);
    const FastName GetNextEntityName() const;

    void SetNextEntityTag(int32 tag);
    int32 GetNextEntityTag() const;

private:
    Entity* nextEntity;
    KeyedArchive* properties;

public:
    INTROSPECTION_EXTEND(EdgeComponent, Component,
                         MEMBER(properties, "Edge properties", I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("nextEntityName", "Next Entity Name", GetNextEntityName, SetNextEntityName, I_VIEW)
                         PROPERTY("nextEntityTag", "Next Entity Tag", GetNextEntityTag, SetNextEntityTag, I_VIEW)
                         );

    DAVA_VIRTUAL_REFLECTION(EdgeComponent, Component);
};

inline Entity* EdgeComponent::GetNextEntity() const
{
    return nextEntity;
}

inline KeyedArchive* EdgeComponent::GetProperties() const
{
    return properties;
}
}
