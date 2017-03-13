#pragma once

#include "Entity/Component.h"
#include "FileSystem/KeyedArchive.h"
#include "Reflection/Reflection.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class CustomPropertiesComponent : public Component
{
protected:
    virtual ~CustomPropertiesComponent();

public:
    CustomPropertiesComponent();

    IMPLEMENT_COMPONENT_TYPE(CUSTOM_PROPERTIES_COMPONENT);

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    KeyedArchive* GetArchive();

    //this method helps to load data for older scene file version
    void LoadFromArchive(const KeyedArchive& srcProperties, SerializationContext* serializationContext);

public:
    INTROSPECTION_EXTEND(CustomPropertiesComponent, Component,
                         MEMBER(properties, "Custom properties", I_SAVE | I_VIEW | I_EDIT)
                         );

    DAVA_VIRTUAL_REFLECTION(CustomPropertiesComponent, Component);

private:
    CustomPropertiesComponent(const KeyedArchive& srcProperties);

private:
    KeyedArchive* properties;
};
};
