#pragma once

#include "Classes/Selection/Selectable.h"

class EntityTransformProxy : public Selectable::TransformProxy
{
public:
    const DAVA::Matrix4& GetWorldTransform(Selectable::Object* object) override;
    const DAVA::Matrix4& GetLocalTransform(Selectable::Object* object) override;
    void SetLocalTransform(Selectable::Object* object, const DAVA::Matrix4& matrix) override;
    bool SupportsTransformType(Selectable::Object* object, Selectable::TransformType) const override;
    bool TransformDependsFromObject(Selectable::Object* dependant, Selectable::Object* dependsOn) const override;
};

class EmitterTransformProxy : public Selectable::TransformProxy
{
public:
    const DAVA::Matrix4& GetWorldTransform(Selectable::Object* object) override;
    const DAVA::Matrix4& GetLocalTransform(Selectable::Object* object) override;
    void SetLocalTransform(Selectable::Object* object, const DAVA::Matrix4& matrix) override;
    bool SupportsTransformType(Selectable::Object* object, Selectable::TransformType) const override;
    bool TransformDependsFromObject(Selectable::Object* dependant, Selectable::Object* dependsOn) const override;
};
