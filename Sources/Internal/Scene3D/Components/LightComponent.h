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


#ifndef __DAVAENGINE_SCENE3D_LIGHT_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_LIGHT_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Render/Highlevel/Light.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA 
{

class LightComponent : public Component
{
protected:
    ~LightComponent();
public:
    LightComponent(Light * _light = 0);
    
    IMPLEMENT_COMPONENT_TYPE(LIGHT_COMPONENT);
    virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SerializationContext *serializationContext);
	virtual void Deserialize(KeyedArchive *archive, SerializationContext *serializationContext);

    void SetLightObject(Light * _light);
    Light * GetLightObject() const;
    
    const bool IsDynamic();
	void SetDynamic(const bool & isDynamic);

    void SetLightType(const uint32 & _type);
    void SetAmbientColor(const Color & _color);
    void SetDiffuseColor(const Color & _color);
    void SetIntensity(const float32& intensity);
    
    const uint32 GetLightType();
    const Color GetAmbientColor();
    const Color GetDiffuseColor();
    const float32 GetIntensity();
    
    const Vector3 GetPosition() const;
    const Vector3 GetDirection() const;
    void SetPosition(const Vector3 & _position);
    void SetDirection(const Vector3& _direction);
    
private:
    Light * light;
    
    void NotifyRenderSystemLightChanged();
    
public:
    
    INTROSPECTION_EXTEND(LightComponent, Component,
        //MEMBER(light, "Light", I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("isDynamic", "isDynamic", IsDynamic, SetDynamic, I_VIEW | I_EDIT)

        PROPERTY("lightType", InspDesc("type", GlobalEnumMap<Light::eType>::Instance()), GetLightType, SetLightType, I_VIEW | I_EDIT)
        PROPERTY("Ambient Color", "Ambient Color", GetAmbientColor, SetAmbientColor, I_VIEW | I_EDIT)
        PROPERTY("Color", "Color", GetDiffuseColor, SetDiffuseColor, I_VIEW | I_EDIT)
        PROPERTY("Intensity", "Intensity", GetIntensity, SetIntensity, I_VIEW | I_EDIT)
    
        //VI: seems we don't need this
        //PROPERTY("position", "position", GetPosition, SetPosition, I_VIEW)
        //PROPERTY("direction", "direction", GetDirection, SetDirection, I_VIEW)
    );
};


};

#endif //__DAVAENGINE_SCENE3D_LIGHT_COMPONENT_H__
