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



#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Entity.h"
#include "Particles/ParticleEmitter.h"
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/QualitySettingsComponent.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/SkyboxRenderObject.h"
#include "Scene3D/Components/TransformComponent.h"

namespace DAVA
{

RenderComponent * GetRenderComponent(const Entity *fromEntity)
{
	return static_cast<RenderComponent*>(fromEntity->GetComponent(Component::RENDER_COMPONENT));
}

TransformComponent * GetTransformComponent(Entity * fromEntity)
{
	return static_cast<TransformComponent*>(fromEntity->GetComponent(Component::TRANSFORM_COMPONENT));
}

RenderObject * GetRenderObject(const Entity * fromEntity)
{
	RenderObject * object = 0;

	if(NULL != fromEntity)
	{
		RenderComponent * component = GetRenderComponent(fromEntity);
		if(component)
		{
			object = component->GetRenderObject();
		}
	}

	return object;
}

SkyboxRenderObject * GetSkybox(const Entity * fromEntity)
{
    RenderObject *ro = GetRenderObject(fromEntity);
    if(ro && ro->GetType() == RenderObject::TYPE_SKYBOX)
    {
        return (static_cast<SkyboxRenderObject *>(ro));
    }
    
    return NULL;
}
   

ParticleEffectComponent * GetEffectComponent(Entity *fromEntity)
{
	if(fromEntity)
	{
		return static_cast<ParticleEffectComponent*>(fromEntity->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
	}

	return NULL;
}

LightComponent *GetLightComponent(Entity * fromEntity)
{
    if(NULL != fromEntity)
    {
        return static_cast<LightComponent*>(fromEntity->GetComponent(Component::LIGHT_COMPONENT));
    }

    return NULL;
}

Light * GetLight( Entity * fromEntity )
{
    LightComponent * component = GetLightComponent(fromEntity);
    if(component)
    {
        return component->GetLightObject();
    }

	return NULL;
}

Landscape * GetLandscape( Entity * fromEntity )
{
	if(NULL != fromEntity)
	{
		RenderObject * object = GetRenderObject(fromEntity);
		if(object && object->GetType() == RenderObject::TYPE_LANDSCAPE)
		{
			Landscape *landscape = static_cast<Landscape *>(object);
			return landscape;
		}
	}

	return NULL;
}

Camera * GetCamera(Entity * fromEntity)
{
	if(NULL != fromEntity)
	{
		CameraComponent *component = static_cast<CameraComponent *>(fromEntity->GetComponent(Component::CAMERA_COMPONENT));
		if(component)
		{
			return component->GetCamera();
		}
	}
    
    return NULL;
}
    
LodComponent * GetLodComponent(Entity *fromEntity)
{
    if(fromEntity)
    {
        return static_cast<LodComponent*>(fromEntity->GetComponent(Component::LOD_COMPONENT));
    }
    
    return NULL;
}

SwitchComponent * GetSwitchComponent(Entity *fromEntity)
{
	if(fromEntity)
	{
		return (SwitchComponent*) fromEntity->GetComponent(Component::SWITCH_COMPONENT);
	}

	return NULL;
}

uint32 GetLodLayersCount(Entity *fromEntity)
{
    if (!fromEntity) return 0;
	
	if(GetEffectComponent(fromEntity)) 
		return LodComponent::MAX_LOD_LAYERS;

    RenderObject *object = GetRenderObject(fromEntity);
    if(!object) 
		return 0;
    
    return (object->GetMaxLodIndex() + 1);
}
    
uint32 GetLodLayersCount(LodComponent *fromComponent)
{
    if(!fromComponent) return 0;

    Entity *entity = fromComponent->GetEntity();

	if(GetEffectComponent(entity)) 
		return LodComponent::MAX_LOD_LAYERS;

	RenderObject *object = GetRenderObject(entity);
	if(!object) 
		return 0;
    
    return (object->GetMaxLodIndex() + 1);
}

void RecursiveProcessMeshNode(Entity * curr, void * userData, void(*process)(Entity*, void *))
{
	RenderComponent * comp = (RenderComponent*)curr->GetComponent(Component::RENDER_COMPONENT);
	if (comp)
	{
		RenderObject * renderObject = comp->GetRenderObject();
		if (renderObject->GetType() == RenderObject::TYPE_MESH)
		{
			process(curr, userData);
		}
	}
	else
	{
		for (int32 i = 0; i < curr->GetChildrenCount(); i++)
			RecursiveProcessMeshNode(curr->GetChild(i), userData, process);
	}
}



void RecursiveProcessLodNode(Entity * curr, int32 lod, void * userData, void(*process)(Entity*, void*))
{
	LodComponent * lodComp = (LodComponent*)curr->GetComponent(Component::LOD_COMPONENT);
	if (lodComp)
	{
		Vector<LodComponent::LodData*> retLodLayers;
		lodComp->GetLodData(retLodLayers);
		for (Vector<LodComponent::LodData*>::iterator it = retLodLayers.begin(); it != retLodLayers.end(); ++it)
		{
			LodComponent::LodData * data = *it;
			if (data->layer == lod)
			{
				for (Vector<Entity*>::iterator i = data->nodes.begin(); i != data->nodes.end(); ++i)
				{
					process((*i), userData);
				}
				break;
			}
		}
	}
	else
	{
		for (int32 i = 0; i < curr->GetChildrenCount(); i++)
			RecursiveProcessLodNode(curr->GetChild(i), lod, userData, process);
	}
}



Entity * FindLandscapeEntity(Entity * rootEntity)
{
	if(GetLandscape(rootEntity))
	{
		return rootEntity;
	}

	DAVA::int32 count = rootEntity->GetChildrenCount();
	for(DAVA::int32 i = 0; i < count; ++i)
	{
		Entity *landscapeEntity = FindLandscapeEntity(rootEntity->GetChild(i));
		if(landscapeEntity)
		{
			return landscapeEntity;
		}
	}

	return NULL;
}

Landscape * FindLandscape(Entity * rootEntity)
{
	Entity *entity = FindLandscapeEntity(rootEntity);
	return GetLandscape(entity);
}

QualitySettingsComponent * GetQualitySettingsComponent(const Entity * fromEntity)
{
    if(fromEntity)
    {
		return (static_cast<QualitySettingsComponent *>(fromEntity->GetComponent(Component::QUALITY_SETTINGS_COMPONENT)));
    }
    
    return NULL;
}
    
}
