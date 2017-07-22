#include "Scene3D/Entity.h"
#include "Particles/ParticleEmitter.h"
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Lod/LodComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Components/QualitySettingsComponent.h"
#include "Scene3D/Components/CustomPropertiesComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Components/StaticOcclusionComponent.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Components/Waypoint/WaypointComponent.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/Vegetation/VegetationRenderObject.h"
#include "Render/Highlevel/SpeedTreeObject.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/SoundComponent.h"
#include "Scene3D/Components/SpeedTreeComponent.h"
#include "Scene3D/Components/WindComponent.h"
#include "Scene3D/Components/WaveComponent.h"
#include "Scene3D/Components/Waypoint/PathComponent.h"
#include "Scene3D/Components/Waypoint/EdgeComponent.h"
#include "Scene3D/Components/Controller/SnapToLandscapeControllerComponent.h"

namespace DAVA
{
bool HasComponent(const Entity* fromEntity, const Component::eType componentType)
{
    if (fromEntity != nullptr)
    {
        return (fromEntity->GetComponentCount(componentType) > 0);
    }

    return false;
}

RenderComponent* GetRenderComponent(const Entity* fromEntity)
{
    if (fromEntity)
        return static_cast<RenderComponent*>(fromEntity->GetComponent(Component::RENDER_COMPONENT));
    else
        return nullptr;
}

TransformComponent* GetTransformComponent(const Entity* fromEntity)
{
    if (fromEntity)
        return static_cast<TransformComponent*>(fromEntity->GetComponent(Component::TRANSFORM_COMPONENT));
    else
        return nullptr;
}

SkeletonComponent* GetSkeletonComponent(const Entity* fromEntity)
{
    if (fromEntity)
        return static_cast<SkeletonComponent*>(fromEntity->GetComponent(Component::SKELETON_COMPONENT));
    else
        return nullptr;
}

RenderObject* GetRenderObject(const Entity* fromEntity)
{
    RenderObject* object = nullptr;

    if (nullptr != fromEntity)
    {
        RenderComponent* component = GetRenderComponent(fromEntity);
        if (component)
        {
            object = component->GetRenderObject();
        }
    }

    return object;
}

SpeedTreeObject* GetSpeedTreeObject(const Entity* fromEntity)
{
    RenderObject* ro = GetRenderObject(fromEntity);
    if (ro && ro->GetType() == RenderObject::TYPE_SPEED_TREE)
    {
        return (static_cast<SpeedTreeObject*>(ro));
    }

    return nullptr;
}

ParticleEffectComponent* GetEffectComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return static_cast<ParticleEffectComponent*>(fromEntity->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
    }

    return nullptr;
}

AnimationComponent* GetAnimationComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return static_cast<AnimationComponent*>(fromEntity->GetComponent(Component::ANIMATION_COMPONENT));
    }
    return nullptr;
}

LightComponent* GetLightComponent(const Entity* fromEntity)
{
    if (nullptr != fromEntity)
    {
        return static_cast<LightComponent*>(fromEntity->GetComponent(Component::LIGHT_COMPONENT));
    }

    return nullptr;
}

Light* GetLight(const Entity* fromEntity)
{
    LightComponent* component = GetLightComponent(fromEntity);
    if (component)
    {
        return component->GetLightObject();
    }

    return nullptr;
}

Landscape* GetLandscape(const Entity* fromEntity)
{
    if (nullptr != fromEntity)
    {
        RenderObject* object = GetRenderObject(fromEntity);
        if (object && object->GetType() == RenderObject::TYPE_LANDSCAPE)
        {
            Landscape* landscape = static_cast<Landscape*>(object);
            return landscape;
        }
    }

    return nullptr;
}

VegetationRenderObject* GetVegetation(const Entity* fromEntity)
{
    if (nullptr != fromEntity)
    {
        RenderObject* object = GetRenderObject(fromEntity);
        if (object && object->GetType() == RenderObject::TYPE_VEGETATION)
        {
            VegetationRenderObject* vegetation = static_cast<VegetationRenderObject*>(object);
            return vegetation;
        }
    }

    return nullptr;
}

Camera* GetCamera(const Entity* fromEntity)
{
    if (nullptr != fromEntity)
    {
        CameraComponent* component = static_cast<CameraComponent*>(fromEntity->GetComponent(Component::CAMERA_COMPONENT));
        if (component)
        {
            return component->GetCamera();
        }
    }

    return nullptr;
}

LodComponent* GetLodComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return static_cast<LodComponent*>(fromEntity->GetComponent(Component::LOD_COMPONENT));
    }

    return nullptr;
}

SwitchComponent* GetSwitchComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return static_cast<SwitchComponent*>(fromEntity->GetComponent(Component::SWITCH_COMPONENT));
    }

    return nullptr;
}

ParticleEffectComponent* GetParticleEffectComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return static_cast<ParticleEffectComponent*>(fromEntity->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
    }

    return nullptr;
}

SoundComponent* GetSoundComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return static_cast<SoundComponent*>(fromEntity->GetComponent(Component::SOUND_COMPONENT));
    }

    return nullptr;
}

uint32 GetLodLayersCount(const Entity* fromEntity)
{
    if (!fromEntity)
        return 0;

    if (GetEffectComponent(fromEntity))
        return LodComponent::MAX_LOD_LAYERS;

    RenderObject* object = GetRenderObject(fromEntity);
    if (!object)
        return 0;

    return (object->GetMaxLodIndex() + 1);
}

uint32 GetLodLayersCount(LodComponent* fromComponent)
{
    if (!fromComponent)
        return 0;

    Entity* entity = fromComponent->GetEntity();

    if (GetEffectComponent(entity))
        return LodComponent::MAX_LOD_LAYERS;

    RenderObject* object = GetRenderObject(entity);
    if (!object)
        return 0;

    return (object->GetMaxLodIndex() + 1);
}

void RecursiveProcessMeshNode(Entity* curr, void* userData, void (*process)(Entity*, void*))
{
    RenderComponent* comp = static_cast<RenderComponent*>(curr->GetComponent(Component::RENDER_COMPONENT));
    if (comp)
    {
        RenderObject* renderObject = comp->GetRenderObject();
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

SpeedTreeComponent* GetSpeedTreeComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return static_cast<SpeedTreeComponent*>(fromEntity->GetComponent(Component::SPEEDTREE_COMPONENT));
    }

    return nullptr;
}

WindComponent* GetWindComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return static_cast<WindComponent*>(fromEntity->GetComponent(Component::WIND_COMPONENT));
    }

    return nullptr;
}

WaveComponent* GetWaveComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return static_cast<WaveComponent*>(fromEntity->GetComponent(Component::WAVE_COMPONENT));
    }

    return nullptr;
}

Entity* FindLandscapeEntity(Entity* rootEntity)
{
    if (GetLandscape(rootEntity))
    {
        return rootEntity;
    }

    DAVA::int32 count = rootEntity->GetChildrenCount();
    for (DAVA::int32 i = 0; i < count; ++i)
    {
        Entity* landscapeEntity = FindLandscapeEntity(rootEntity->GetChild(i));
        if (landscapeEntity)
        {
            return landscapeEntity;
        }
    }

    return nullptr;
}

Entity* FindVegetationEntity(Entity* rootEntity)
{
    if (GetVegetation(rootEntity))
    {
        return rootEntity;
    }

    DAVA::int32 count = rootEntity->GetChildrenCount();
    for (DAVA::int32 i = 0; i < count; ++i)
    {
        Entity* vegetationEntity = FindVegetationEntity(rootEntity->GetChild(i));
        if (vegetationEntity)
        {
            return vegetationEntity;
        }
    }

    return nullptr;
}

Landscape* FindLandscape(Entity* rootEntity)
{
    Entity* entity = FindLandscapeEntity(rootEntity);
    return GetLandscape(entity);
}

VegetationRenderObject* FindVegetation(Entity* rootEntity)
{
    Entity* entity = FindVegetationEntity(rootEntity);
    return GetVegetation(entity);
}

QualitySettingsComponent* GetQualitySettingsComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return (static_cast<QualitySettingsComponent*>(fromEntity->GetComponent(Component::QUALITY_SETTINGS_COMPONENT)));
    }

    return nullptr;
}

CustomPropertiesComponent* GetCustomProperties(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return (static_cast<CustomPropertiesComponent*>(fromEntity->GetComponent(Component::CUSTOM_PROPERTIES_COMPONENT)));
    }

    return nullptr;
}

CustomPropertiesComponent* GetOrCreateCustomProperties(Entity* fromEntity)
{
    if (fromEntity)
    {
        return (static_cast<CustomPropertiesComponent*>(fromEntity->GetOrCreateComponent(Component::CUSTOM_PROPERTIES_COMPONENT)));
    }

    return nullptr;
}

KeyedArchive* GetCustomPropertiesArchieve(const Entity* fromEntity)
{
    CustomPropertiesComponent* comp = GetCustomProperties(fromEntity);
    return (comp != nullptr) ? comp->GetArchive() : nullptr;
}

VariantType* GetCustomPropertiesValueRecursive(Entity* fromEntity, const String& name)
{
    if (fromEntity == nullptr)
        return nullptr;

    KeyedArchive* props = GetCustomPropertiesArchieve(fromEntity);
    if ((props != nullptr) && (props->Count(name) > 0))
    {
        return props->GetVariant(name);
    }
    return GetCustomPropertiesValueRecursive(fromEntity->GetParent(), name);
}

PathComponent* GetPathComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return static_cast<PathComponent*>(fromEntity->GetComponent(Component::PATH_COMPONENT));
    }

    return nullptr;
}

WaypointComponent* GetWaypointComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return static_cast<WaypointComponent*>(fromEntity->GetComponent(Component::WAYPOINT_COMPONENT));
    }

    return NULL;
}

SnapToLandscapeControllerComponent* GetSnapToLandscapeControllerComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return (static_cast<SnapToLandscapeControllerComponent*>(fromEntity->GetComponent(Component::SNAP_TO_LANDSCAPE_CONTROLLER_COMPONENT)));
    }

    return nullptr;
}

StaticOcclusionComponent* GetStaticOcclusionComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return (static_cast<StaticOcclusionComponent*>(fromEntity->GetComponent(Component::STATIC_OCCLUSION_COMPONENT)));
    }

    return nullptr;
}

StaticOcclusionDebugDrawComponent* GetStaticOcclusionDebugDrawComponent(const Entity* fromEntity)
{
    if (fromEntity)
    {
        return (static_cast<StaticOcclusionDebugDrawComponent*>(fromEntity->GetComponent(Component::STATIC_OCCLUSION_DEBUG_DRAW_COMPONENT)));
    }

    return nullptr;
}
}
