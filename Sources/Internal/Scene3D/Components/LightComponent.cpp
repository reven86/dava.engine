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


#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Scene.h"
#include "Render/Highlevel/RenderSystem.h"

namespace DAVA 
{
    

LightComponent::LightComponent(Light * _light)
{
    light = SafeRetain(_light);
}

LightComponent::~LightComponent()
{
    SafeRelease(light);
}
    
void LightComponent::SetLightObject(Light * _light)
{
	SafeRelease(light);
    light = SafeRetain(_light);
}
    
Light * LightComponent::GetLightObject() const
{
    return light;
}
    
Component * LightComponent::Clone(Entity * toEntity)
{
    LightComponent * component = new LightComponent();
	component->SetEntity(toEntity);
    
    if(light)
        component->light = (Light*)light->Clone();
    
    return component;
}

void LightComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	Component::Serialize(archive, serializationContext);

	if(NULL != archive && NULL != light)
	{
		KeyedArchive *lightArch = new KeyedArchive();
		light->Save(lightArch, serializationContext);

		archive->SetArchive("lc.light", lightArch);

		lightArch->Release();
	}
}

void LightComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	if(NULL != archive)
	{
		KeyedArchive *lightArch = archive->GetArchive("lc.light");
		if(NULL != lightArch)
		{
			Light* l = new Light();
			l->Load(lightArch, serializationContext);
			SetLightObject(l);
			l->Release();
		}
	}

	Component::Deserialize(archive, serializationContext);
}

const bool LightComponent::IsDynamic()
{
    return (light) ? light->IsDynamic() : false;
}

void LightComponent::SetDynamic(const bool & isDynamic)
{
    if(light)
    {
        light->SetDynamic(isDynamic);
        
        NotifyRenderSystemLightChanged();
    }
}
    
void LightComponent::SetLightType(const uint32 & _type)
{
    if(light)
    {
        light->SetType((Light::eType)_type);
        
        NotifyRenderSystemLightChanged();
    }
}

void LightComponent::SetAmbientColor(const Color & _color)
{
    if(light)
    {
        light->SetAmbientColor(_color);
        
        NotifyRenderSystemLightChanged();
    }
}

void LightComponent::SetDiffuseColor(const Color & _color)
{
    if(light)
    {
        light->SetDiffuseColor(_color);
        
        NotifyRenderSystemLightChanged();
    }
}

void LightComponent::SetIntensity(const float32& intensity)
{
    if(light)
    {
        light->SetIntensity(intensity);
        
        NotifyRenderSystemLightChanged();
    }
}
    
const uint32 LightComponent::GetLightType()
{
    if(light)
    {
        return light->GetType();
    }

    return Light::TYPE_SKY;
}

const Color LightComponent::GetAmbientColor()
{
    if(light)
    {
        return light->GetAmbientColor();
    }

    return Color();
}

const Color LightComponent::GetDiffuseColor()
{
    if(light)
    {
        return light->GetDiffuseColor();
    }
    
    return Color();
}

const float32 LightComponent::GetIntensity()
{
    if(light)
    {
        return light->GetIntensity();
    }
    
    return 0.0f;
}
    
const Vector3 LightComponent::GetPosition() const
{
    if(light)
    {
        return light->GetPosition();
    }
    
    return Vector3();
}

const Vector3 LightComponent::GetDirection() const
{
    if(light)
    {
        return light->GetDirection();
    }
    
    return Vector3();
}

void LightComponent::SetPosition(const Vector3 & position)
{
    if(light)
    {
        light->SetPosition(position);
        
        NotifyRenderSystemLightChanged();
    }
}

void LightComponent::SetDirection(const Vector3& direction)
{
    if(light)
    {
        light->SetDirection(direction);
        
        NotifyRenderSystemLightChanged();
    }
}

void LightComponent::NotifyRenderSystemLightChanged()
{
    if(entity)
    {
        Scene* curScene = entity->GetScene();
        if(curScene)
        {
            curScene->renderSystem->SetForceUpdateLights();
        }
    }
}


};
