#pragma once

#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class SnapToLandscapeControllerComponent : public Component
{
public:
    IMPLEMENT_COMPONENT_TYPE(SNAP_TO_LANDSCAPE_CONTROLLER_COMPONENT);

    SnapToLandscapeControllerComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    inline float32 GetHeightOnLandscape() const;
    void SetHeightOnLandscape(float32 height);

protected:
    float32 heightOnLandscape;

public:
    INTROSPECTION_EXTEND(SnapToLandscapeControllerComponent, Component,
                         PROPERTY("heightOnLandscape", "Height On Landscape", GetHeightOnLandscape, SetHeightOnLandscape, I_VIEW | I_EDIT | I_SAVE)
                         );

    DAVA_VIRTUAL_REFLECTION(SnapToLandscapeControllerComponent, Component);
};

inline float32 SnapToLandscapeControllerComponent::GetHeightOnLandscape() const
{
    return heightOnLandscape;
}
}
