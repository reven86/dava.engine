#pragma once

#include "Entity/Component.h"
#include "Scene3D/Components/Waypoint/PathComponent.h"
#include "Reflection/Reflection.h"
#include "Base/Introspection.h"

namespace DAVA
{
class SerializationContext;
class KeyedArchive;
class Entity;

class EdgeComponent : public Component
{
public:
    IMPLEMENT_COMPONENT_TYPE(EDGE_COMPONENT);

    EdgeComponent();
    EdgeComponent(const EdgeComponent&);
    ~EdgeComponent();

    void Init(PathComponent* path, PathComponent::Edge* edge);
    PathComponent* GetPath() const;
    PathComponent::Edge* GetEdge() const;

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
    Entity* nextEntity = nullptr;
    PathComponent* path = nullptr;
    PathComponent::Edge* edge = nullptr;

public:
    INTROSPECTION_EXTEND(EdgeComponent, Component,
                         PROPERTY("Edge properties", "Edge properties", GetProperties, SetProperties, I_SAVE | I_VIEW | I_EDIT)
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
    DVASSERT(path != nullptr);
    DVASSERT(edge != nullptr);
    return edge->GetProperties();
}
}
