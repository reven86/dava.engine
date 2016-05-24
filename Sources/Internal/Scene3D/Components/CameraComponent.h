#ifndef __DAVAENGINE_CAMERA_COMPONENT_H__
#define __DAVAENGINE_CAMERA_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class Camera;

class CameraComponent : public Component
{
protected:
    virtual ~CameraComponent();

public:
    CameraComponent(Camera* _camera = 0);

    Camera* GetCamera();
    void SetCamera(Camera* _camera);

    IMPLEMENT_COMPONENT_TYPE(CAMERA_COMPONENT);

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

private:
    Camera* camera;

public:
    INTROSPECTION_EXTEND(CameraComponent, Component,
                         PROPERTY("camera", "Camera", GetCamera, SetCamera, I_SAVE | I_VIEW | I_EDIT)
                         );
};
};

#endif //__DAVAENGINE_CAMERA_COMPONENT_H__
