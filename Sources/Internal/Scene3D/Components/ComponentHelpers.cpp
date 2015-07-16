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
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Components/QualitySettingsComponent.h"
#include "Scene3D/Components/CustomPropertiesComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/SoundComponent.h"
#include "Scene3D/Components/SkeletonComponent.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/Vegetation/VegetationRenderObject.h"
#include "Render/Highlevel/SpeedTreeObject.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/SpeedTreeComponent.h"
#include "Scene3D/Components/WindComponent.h"
#include "Scene3D/Components/WaveComponent.h"
#include "Scene3D/Components/Waypoint/PathComponent.h"
#include "Scene3D/Components/Waypoint/EdgeComponent.h"
#include "Scene3D/Components/Controller/SnapToLandscapeControllerComponent.h"

namespace DAVA
{


RenderComponent * GetRenderComponent(const Entity *fromEntity)
{
    if(fromEntity)
	    return static_cast<RenderComponent*>(fromEntity->GetComponent(Component::RENDER_COMPONENT));
    else
        return nullptr;
}

TransformComponent * GetTransformComponent(const Entity *fromEntity)
{
    if(fromEntity)
	    return static_cast<TransformComponent*>(fromEntity->GetComponent(Component::TRANSFORM_COMPONENT));
    else
        return nullptr;
}

SkeletonComponent * GetSkeletonComponent(const Entity *fromEntity)
{
    if(fromEntity)
        return static_cast<SkeletonComponent*>(fromEntity->GetComponent(Component::SKELETON_COMPONENT));
    else
        return nullptr;
}

RenderObject * GetRenderObject(const Entity *fromEntity)
{
	RenderObject * object = nullptr;

	if(nullptr != fromEntity)
	{
		RenderComponent * component = GetRenderComponent(fromEntity);
		if(component)
		{
			object = component->GetRenderObject();
		}
	}

	return object;
}

SpeedTreeObject * GetSpeedTreeObject(const Entity *fromEntity)
{
    RenderObject *ro = GetRenderObject(fromEntity);
    if(ro && ro->GetType() == RenderObject::TYPE_SPEED_TREE)
    {
        return (static_cast<SpeedTreeObject *>(ro));
    }

    return nullptr;
}



ParticleEffectComponent * GetEffectComponent(const Entity *fromEntity)
{
	if(fromEntity)
	{
		return static_cast<ParticleEffectComponent*>(fromEntity->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
	}

	return nullptr;
}

AnimationComponent * GetAnimationComponent(const Entity *fromEntity)
{
    if(fromEntity)
    {
        return static_cast<AnimationComponent*>(fromEntity->GetComponent(Component::ANIMATION_COMPONENT));
    }
    return nullptr;
}


LightComponent *GetLightComponent(const Entity *fromEntity)
{
    if(nullptr != fromEntity)
    {
        return static_cast<LightComponent*>(fromEntity->GetComponent(Component::LIGHT_COMPONENT));
    }

    return nullptr;
}

Light * GetLight(const Entity *fromEntity )
{
    LightComponent * component = GetLightComponent(fromEntity);
    if(component)
    {
        return component->GetLightObject();
    }

	return nullptr;
}

Landscape * GetLandscape(const Entity *fromEntity)
{
	if(nullptr != fromEntity)
	{
		RenderObject * object = GetRenderObject(fromEntity);
		if(object && object->GetType() == RenderObject::TYPE_LANDSCAPE)
		{
			Landscape *landscape = static_cast<Landscape *>(object);
			return landscape;
		}
	}

	return nullptr;
}

VegetationRenderObject * GetVegetation(const Entity *fromEntity)
{
    if(nullptr != fromEntity)
    {
        RenderObject * object = GetRenderObject(fromEntity);
        if(object && object->GetType() == RenderObject::TYPE_VEGETATION)
        {
            VegetationRenderObject *vegetation = static_cast<VegetationRenderObject *>(object);
            return vegetation;
        }
    }

    return nullptr;
}

Camera * GetCamera(const Entity *fromEntity)
{
	if(nullptr != fromEntity)
	{
		CameraComponent *component = static_cast<CameraComponent *>(fromEntity->GetComponent(Component::CAMERA_COMPONENT));
		if(component)
		{
			return component->GetCamera();
		}
	}
    
    return nullptr;
}
    
LodComponent * GetLodComponent(const Entity *fromEntity)
{
    if(fromEntity)
    {
        return static_cast<LodComponent*>(fromEntity->GetComponent(Component::LOD_COMPONENT));
    }
    
    return nullptr;
}

SwitchComponent * GetSwitchComponent(const Entity *fromEntity)
{
	if(fromEntity)
	{
		return (SwitchComponent*) fromEntity->GetComponent(Component::SWITCH_COMPONENT);
	}

	return nullptr;
}

SoundComponent * GetSoundComponent(const Entity *fromEntity)
{
    if(fromEntity)
    {
        return static_cast<SoundComponent *>(fromEntity->GetComponent(Component::SOUND_COMPONENT));
    }

    return nullptr;
}

uint32 GetLodLayersCount(const Entity *fromEntity)
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

SpeedTreeComponent * GetSpeedTreeComponent(const Entity *fromEntity)
{
    if(fromEntity)
    {
        return static_cast<SpeedTreeComponent *>(fromEntity->GetComponent(Component::SPEEDTREE_COMPONENT));
    }
    
    return nullptr;
}

WindComponent * GetWindComponent(const Entity *fromEntity)
{
    if(fromEntity)
    {
        return static_cast<WindComponent *>(fromEntity->GetComponent(Component::WIND_COMPONENT));
    }
    
    return nullptr;
}

WaveComponent * GetWaveComponent(const Entity *fromEntity)
{
    if(fromEntity)
    {
        return static_cast<WaveComponent *>(fromEntity->GetComponent(Component::WAVE_COMPONENT));
    }

    return nullptr;
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

	return nullptr;
}

Entity * FindVegetationEntity(Entity * rootEntity)
{
    if(GetVegetation(rootEntity))
    {
        return rootEntity;
    }
        
    DAVA::int32 count = rootEntity->GetChildrenCount();
    for(DAVA::int32 i = 0; i < count; ++i)
    {
        Entity *vegetationEntity = FindVegetationEntity(rootEntity->GetChild(i));
        if(vegetationEntity)
        {
            return vegetationEntity;
        }
    }
        
    return nullptr;
}

Landscape * FindLandscape(Entity * rootEntity)
{
	Entity *entity = FindLandscapeEntity(rootEntity);
	return GetLandscape(entity);
}

VegetationRenderObject* FindVegetation(Entity * rootEntity)
{
    Entity *entity = FindVegetationEntity(rootEntity);
    return GetVegetation(entity);
}


QualitySettingsComponent * GetQualitySettingsComponent(const Entity *fromEntity)
{
    if(fromEntity)
    {
		return (static_cast<QualitySettingsComponent *>(fromEntity->GetComponent(Component::QUALITY_SETTINGS_COMPONENT)));
    }
    
    return nullptr;
}
    
CustomPropertiesComponent * GetCustomProperties(const Entity *fromEntity)
{
    if(fromEntity)
    {
		return (static_cast<CustomPropertiesComponent *>(fromEntity->GetComponent(Component::CUSTOM_PROPERTIES_COMPONENT)));
    }
    
    return nullptr;
    
}
    
CustomPropertiesComponent * GetOrCreateCustomProperties(Entity *fromEntity)
{
    if(fromEntity)
    {
        return (static_cast<CustomPropertiesComponent *>(fromEntity->GetOrCreateComponent(Component::CUSTOM_PROPERTIES_COMPONENT)));
    }
    
    return nullptr;
}


KeyedArchive * GetCustomPropertiesArchieve(const Entity *fromEntity)
{
    CustomPropertiesComponent * comp = GetCustomProperties(fromEntity);
    if(comp)
    {
        return comp->GetArchive();
    }
    
    return nullptr;
}

PathComponent * GetPathComponent(const Entity *fromEntity)
{
    if(fromEntity)
    {
        return (PathComponent*) fromEntity->GetComponent(Component::PATH_COMPONENT);
    }

        return nullptr;
}

WaypointComponent * GetWaypointComponent(const Entity *fromEntity)
{
    if (fromEntity)
    {
        return (WaypointComponent*)fromEntity->GetComponent(Component::WAYPOINT_COMPONENT);
    }

    return NULL;
}

EdgeComponent* FindEdgeComponent(const Entity *fromEntity, const Entity *toEntity)
{
    uint32 count = fromEntity->GetComponentCount(Component::EDGE_COMPONENT);
    for (uint32 i = 0; i < count; ++i)
    {
        EdgeComponent* edge = static_cast<EdgeComponent*>(fromEntity->GetComponent(Component::EDGE_COMPONENT, i));
        DVASSERT(edge);
        if (edge->GetNextEntity() == toEntity)
        {
            return edge;
        }
    }
    return nullptr;
}

SnapToLandscapeControllerComponent * GetSnapToLandscapeControllerComponent(const Entity *fromEntity)
{
    if(fromEntity)
    {
        return (static_cast<SnapToLandscapeControllerComponent *>(fromEntity->GetComponent(Component::SNAP_TO_LANDSCAPE_CONTROLLER_COMPONENT)));
    }
    
    return nullptr;
}
    
}
