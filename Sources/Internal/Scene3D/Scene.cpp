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


#include "Scene3D/Scene.h"

#include "Render/Texture.h"
#include "Render/Material.h"
#include "Render/3D/StaticMesh.h"
#include "Render/3D/AnimatedMesh.h"
#include "Render/Image/Image.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/RenderOptions.h"
#include "Render/MipmapReplacer.h"

#include "Platform/SystemTimer.h"
#include "FileSystem/FileSystem.h"
#include "Debug/Stats.h"

#include "Scene3D/SceneFile.h"
#include "Scene3D/SceneFileV2.h"
#include "Scene3D/DataNode.h"
#include "Scene3D/ShadowVolumeNode.h"
#include "Render/Highlevel/Light.h"
#include "Scene3D/MeshInstanceNode.h"
#include "Scene3D/ImposterManager.h"
#include "Scene3D/ImposterNode.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/RenderSystem.h"

#include "Entity/SceneSystem.h"
#include "Scene3D/Systems/TransformSystem.h"
#include "Scene3D/Systems/RenderUpdateSystem.h"
#include "Scene3D/Systems/LodSystem.h"
#include "Scene3D/Systems/DebugRenderSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/ParticleEffectSystem.h"
#include "Scene3D/Systems/UpdateSystem.h"
#include "Scene3D/Systems/LightUpdateSystem.h"
#include "Scene3D/Systems/SwitchSystem.h"
#include "Scene3D/Systems/SoundUpdateSystem.h"
#include "Scene3D/Systems/ActionUpdateSystem.h"
#include "Scene3D/Systems/SkyboxSystem.h"
#include "Scene3D/Systems/WindSystem.h"
#include "Scene3D/Systems/WaveSystem.h"
#include "Scene3D/Systems/SkeletonSystem.h"
#include "Scene3D/Systems/AnimationSystem.h"

#include "Sound/SoundSystem.h"

#include "Scene3D/Systems/SpeedTreeUpdateSystem.h"

#include "Scene3D/Systems/StaticOcclusionSystem.h"
#include "Scene3D/Systems/FoliageSystem.h"

#include "Scene3D/Systems/MaterialSystem.h"

#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/SceneCache.h"
#include "UI/UIEvent.h"
#include <functional>


namespace DAVA 
{

Texture* Scene::stubTexture2d = nullptr;
Texture* Scene::stubTextureCube = nullptr;
Texture* Scene::stubTexture2dLightmap = nullptr; //this texture should be all-pink without checkers

EntityCache::~EntityCache()
{
    ClearAll();
}

void EntityCache::Preload(const FilePath &path)
{
    Scene *scene = new Scene(0);
    if(SceneFileV2::ERROR_NO_ERROR == scene->LoadScene(path))
    {
        Entity *srcRootEntity = scene;

        // try to perform little optimization:
        // if scene has single node with identity transform
        // we can skip this entity and move only its children
        if(1 == srcRootEntity->GetChildrenCount())
        {
            Entity *child = srcRootEntity->GetChild(0);
            if(1 == child->GetComponentCount())
            {
                TransformComponent * tr = (TransformComponent *)srcRootEntity->GetComponent(Component::TRANSFORM_COMPONENT);
                if(nullptr != tr && tr->GetLocalTransform() == Matrix4::IDENTITY)
                {
                    srcRootEntity = child;
                }
            }
        }

        auto count = srcRootEntity->GetChildrenCount();

        Vector<Entity*> tempV;
        tempV.reserve(count);
        for(auto i = 0; i < count; ++i)
        {
            tempV.push_back(srcRootEntity->GetChild(i));
        }

        Entity *dstRootEntity = new Entity();
        for(auto i = 0; i < count; ++i)
        {
            dstRootEntity->AddNode(tempV[i]);
        }

        dstRootEntity->ResetID();
        dstRootEntity->SetName(scene->GetName());
        cachedEntities[path] = dstRootEntity;
    }

    SafeRelease(scene);
}

Entity* EntityCache::GetOriginal(const FilePath &path)
{
    Entity *ret = nullptr;

    if(cachedEntities.find(path) == cachedEntities.end())
    {
        Preload(path);
    }

    auto i = cachedEntities.find(path);
    if(i != cachedEntities.end())
    {
        ret = i->second;
    }

    return ret;
}

Entity* EntityCache::GetClone(const FilePath &path)
{
    Entity* ret = nullptr;

    Entity* orig = GetOriginal(path);
    if(nullptr != orig)
    {
        ret = orig->Clone();
    }

    return ret;
}

void EntityCache::Clear(const FilePath &path)
{
    auto i = cachedEntities.find(path);
    if(i != cachedEntities.end())
    {
        SafeRelease(i->second);
        cachedEntities.erase(i);
    }
}

void EntityCache::ClearAll()
{
    for(auto i : cachedEntities)
    {
        SafeRelease(i.second);
    }

    cachedEntities.clear();
}

Scene::Scene(uint32 _systemsMask /* = SCENE_SYSTEM_ALL_MASK */)
	: Entity()
    , transformSystem(0)
    , renderUpdateSystem(0)
    , lodSystem(0)
    , debugRenderSystem(0)
    , particleEffectSystem(0)
    , updatableSystem(0)
    , lightUpdateSystem(0)
    , switchSystem(0)
    , soundSystem(0)
    , actionSystem(0)
    , skyboxSystem(0)
    , staticOcclusionSystem(0)
	, materialSystem(0)
    , foliageSystem(0)
    , windSystem(0)
    , animationSystem(0)
    , staticOcclusionDebugDrawSystem(0)
    , systemsMask(_systemsMask)
    , clearBuffers(RenderManager::ALL_BUFFERS)
    , isDefaultGlobalMaterial(true)
    , sceneGlobalMaterial(0)
    , mainCamera(0)
    , drawCamera(0)
    , imposterManager(0)
    , maxEntityIDCounter(0)
{
    static uint32 idCounter = 0;
    sceneId = ++idCounter;

	CreateComponents();
	CreateSystems();

    // this will force scene to create hidden global material
    SetGlobalMaterial(nullptr);
    
    SceneCache::Instance()->InsertScene(this);

    RenderOptions * options = RenderManager::Instance()->GetOptions();
    options->AddObserver(this);
}

void Scene::CreateComponents()
{ }

NMaterial* Scene::GetGlobalMaterial() const
{
    NMaterial *ret = nullptr;

    // default global material is for internal use only
    // so all external object should assume, that scene hasn't any global material
    if(!isDefaultGlobalMaterial)
    {
        ret = sceneGlobalMaterial;
    }

    return ret;
}

void Scene::SetGlobalMaterial(NMaterial *globalMaterial)
{
    SafeRelease(sceneGlobalMaterial);

    if(nullptr != globalMaterial)
    {
        DVASSERT(globalMaterial->GetMaterialType() == NMaterial::MATERIALTYPE_GLOBAL);

        isDefaultGlobalMaterial = false;
        sceneGlobalMaterial = SafeRetain(globalMaterial);
    }
    else
    {
        isDefaultGlobalMaterial = true;
        sceneGlobalMaterial = NMaterial::CreateGlobalMaterial(FastName("Scene_Global_Material"));
    }

    InitGlobalMaterial();

    renderSystem->SetGlobalMaterial(sceneGlobalMaterial);
    
    if (nullptr != particleEffectSystem)
        particleEffectSystem->SetGlobalMaterial(sceneGlobalMaterial);
    
    ImportShadowColor(this);
}

void Scene::InitGlobalMaterial()
{
    if(nullptr == stubTexture2d)
    {
        stubTexture2d = Texture::CreatePink(Texture::TEXTURE_2D);
    }

    if(nullptr == stubTextureCube)
    {
        stubTextureCube = Texture::CreatePink(Texture::TEXTURE_CUBE);
    }

    if(nullptr == stubTexture2dLightmap)
    {
        stubTexture2dLightmap = Texture::CreatePink(Texture::TEXTURE_2D, false);
    }

    Vector3 defaultVec3;
    Color defaultColor(1.0f, 0.0f, 0.0f, 1.0f);
    //float32 defaultFloat0 = 0.0f;
    float32 defaultFloat05 = 0.5f;
    float32 defaultFloat10 = 1.0f;
    Vector2 defaultVec2;
    Vector2 defaultVec2I(1.f, 1.f);
    float32 defaultLightmapSize = 16.0f;
    float32 defaultFogStart = 0.0f;
    float32 defaultFogEnd = 500.0f;
    float32 defaultFogHeight = 50.0f;
    float32 defaultFogDensity = 0.005f;

    if(sceneGlobalMaterial->GetTexturePath(NMaterial::TEXTURE_ALBEDO).IsEmpty()) sceneGlobalMaterial->SetTexture(NMaterial::TEXTURE_ALBEDO, stubTexture2d);
    if(sceneGlobalMaterial->GetTexturePath(NMaterial::TEXTURE_NORMAL).IsEmpty()) sceneGlobalMaterial->SetTexture(NMaterial::TEXTURE_NORMAL, stubTexture2d);
    if(sceneGlobalMaterial->GetTexturePath(NMaterial::TEXTURE_DETAIL).IsEmpty()) sceneGlobalMaterial->SetTexture(NMaterial::TEXTURE_DETAIL, stubTexture2d);
    if(sceneGlobalMaterial->GetTexturePath(NMaterial::TEXTURE_LIGHTMAP).IsEmpty()) sceneGlobalMaterial->SetTexture(NMaterial::TEXTURE_LIGHTMAP, stubTexture2dLightmap);
    if(sceneGlobalMaterial->GetTexturePath(NMaterial::TEXTURE_DECAL).IsEmpty()) sceneGlobalMaterial->SetTexture(NMaterial::TEXTURE_DECAL, stubTexture2d);
    if(sceneGlobalMaterial->GetTexturePath(NMaterial::TEXTURE_CUBEMAP).IsEmpty()) sceneGlobalMaterial->SetTexture(NMaterial::TEXTURE_CUBEMAP, stubTextureCube);
    if(sceneGlobalMaterial->GetTexturePath(NMaterial::TEXTURE_DECALMASK).IsEmpty()) sceneGlobalMaterial->SetTexture(NMaterial::TEXTURE_DECALMASK, stubTexture2d);
    if(sceneGlobalMaterial->GetTexturePath(NMaterial::TEXTURE_DECALTEXTURE).IsEmpty()) sceneGlobalMaterial->SetTexture(NMaterial::TEXTURE_DECALTEXTURE, stubTexture2d);

    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_LIGHT_POSITION0)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_LIGHT_POSITION0, Shader::UT_FLOAT_VEC3, 1, defaultVec3.data);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_PROP_AMBIENT_COLOR)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_PROP_AMBIENT_COLOR, Shader::UT_FLOAT_VEC4, 1, &defaultColor);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_PROP_DIFFUSE_COLOR)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_PROP_DIFFUSE_COLOR, Shader::UT_FLOAT_VEC4, 1, &defaultColor);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_PROP_SPECULAR_COLOR)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_PROP_SPECULAR_COLOR, Shader::UT_FLOAT_VEC4, 1, &defaultColor);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_LIGHT_AMBIENT_COLOR)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_LIGHT_AMBIENT_COLOR, Shader::UT_FLOAT_VEC3, 1, &defaultColor);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_LIGHT_DIFFUSE_COLOR)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_LIGHT_DIFFUSE_COLOR, Shader::UT_FLOAT_VEC3, 1, &defaultColor);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_LIGHT_SPECULAR_COLOR)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_LIGHT_SPECULAR_COLOR, Shader::UT_FLOAT_VEC3, 1, &defaultColor);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_LIGHT_INTENSITY0)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_LIGHT_INTENSITY0, Shader::UT_FLOAT, 1, &defaultFloat05);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_MATERIAL_SPECULAR_SHININESS)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_MATERIAL_SPECULAR_SHININESS, Shader::UT_FLOAT, 1, &defaultFloat05);
    
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_FOG_LIMIT)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_FOG_LIMIT, Shader::UT_FLOAT, 1, &defaultFloat10);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_FOG_COLOR)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_FOG_COLOR, Shader::UT_FLOAT_VEC4, 1, &defaultColor);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_FOG_DENSITY)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_FOG_DENSITY, Shader::UT_FLOAT, 1, &defaultFogDensity);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_FOG_START)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_FOG_START, Shader::UT_FLOAT, 1, &defaultFogStart);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_FOG_END)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_FOG_END, Shader::UT_FLOAT, 1, &defaultFogEnd);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_FOG_HALFSPACE_DENSITY)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_FOG_HALFSPACE_DENSITY, Shader::UT_FLOAT, 1, &defaultFogDensity);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_FOG_HALFSPACE_FALLOFF)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_FOG_HALFSPACE_FALLOFF, Shader::UT_FLOAT, 1, &defaultFogDensity);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_FOG_HALFSPACE_HEIGHT)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_FOG_HALFSPACE_HEIGHT, Shader::UT_FLOAT, 1, &defaultFogHeight);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_FOG_HALFSPACE_LIMIT)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_FOG_HALFSPACE_LIMIT, Shader::UT_FLOAT, 1, &defaultFloat10);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_FOG_ATMOSPHERE_COLOR_SUN)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_FOG_ATMOSPHERE_COLOR_SUN, Shader::UT_FLOAT_VEC4, 1, &defaultColor);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_FOG_ATMOSPHERE_COLOR_SKY)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_FOG_ATMOSPHERE_COLOR_SKY, Shader::UT_FLOAT_VEC4, 1, &defaultColor);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_FOG_ATMOSPHERE_SCATTERING)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_FOG_ATMOSPHERE_SCATTERING, Shader::UT_FLOAT, 1, &defaultFloat10);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_FOG_ATMOSPHERE_DISTANCE)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_FOG_ATMOSPHERE_DISTANCE, Shader::UT_FLOAT, 1, &defaultFogEnd);

    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_FLAT_COLOR)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_FLAT_COLOR, Shader::UT_FLOAT_VEC4, 1, &defaultColor);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_TEXTURE0_SHIFT)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_TEXTURE0_SHIFT, Shader::UT_FLOAT_VEC2, 1, defaultVec2.data);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_UV_OFFSET)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_UV_OFFSET, Shader::UT_FLOAT_VEC2, 1, defaultVec2.data);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_UV_SCALE)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_UV_SCALE, Shader::UT_FLOAT_VEC2, 1, defaultVec2.data);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_LIGHTMAP_SIZE)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_LIGHTMAP_SIZE, Shader::UT_FLOAT, 1, &defaultLightmapSize);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_DECAL_TILE_SCALE)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_DECAL_TILE_SCALE, Shader::UT_FLOAT_VEC2, 1, &defaultVec2);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_DECAL_TILE_COLOR)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_DECAL_TILE_COLOR, Shader::UT_FLOAT_VEC4, 1, &Color::White);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_DETAIL_TILE_SCALE)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_DETAIL_TILE_SCALE, Shader::UT_FLOAT_VEC2, 1, &defaultVec2);
    if(nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_SHADOW_COLOR)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_SHADOW_COLOR, Shader::UT_FLOAT_VEC4, 1, &defaultColor);

    if (nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_NORMAL_SCALE)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_NORMAL_SCALE, Shader::UT_FLOAT, 1, &defaultFloat10);

    if (nullptr == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_ALPHATEST_THRESHOLD)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_ALPHATEST_THRESHOLD, Shader::UT_FLOAT, 1, &defaultFloat05);
}

void Scene::CreateSystems()
{
	renderSystem = new RenderSystem();
    eventSystem = new EventSystem();

    if(SCENE_SYSTEM_STATIC_OCCLUSION_FLAG & systemsMask)
    {
        staticOcclusionSystem = new StaticOcclusionSystem(this);
        AddSystem(staticOcclusionSystem, MAKE_COMPONENT_MASK(Component::STATIC_OCCLUSION_DATA_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if(SCENE_SYSTEM_ANIMATION_FLAG & systemsMask)
    {
        animationSystem = new AnimationSystem(this);
        AddSystem(animationSystem, MAKE_COMPONENT_MASK(Component::ANIMATION_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if(SCENE_SYSTEM_TRANSFORM_FLAG & systemsMask)
    {
        transformSystem = new TransformSystem(this);
        AddSystem(transformSystem, MAKE_COMPONENT_MASK(Component::TRANSFORM_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    } 

    if(SCENE_SYSTEM_LOD_FLAG & systemsMask)
    {
        lodSystem = new LodSystem(this);
        AddSystem(lodSystem, MAKE_COMPONENT_MASK(Component::LOD_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if(SCENE_SYSTEM_SWITCH_FLAG & systemsMask)
    {
        switchSystem = new SwitchSystem(this);
        AddSystem(switchSystem, MAKE_COMPONENT_MASK(Component::SWITCH_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if(SCENE_SYSTEM_PARTICLE_EFFECT_FLAG & systemsMask)
    {
        particleEffectSystem = new ParticleEffectSystem(this);
        particleEffectSystem->SetGlobalMaterial(GetGlobalMaterial());
        AddSystem(particleEffectSystem, MAKE_COMPONENT_MASK(Component::PARTICLE_EFFECT_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if(SCENE_SYSTEM_SOUND_UPDATE_FLAG & systemsMask)
    {
        soundSystem = new SoundUpdateSystem(this);
        AddSystem(soundSystem, MAKE_COMPONENT_MASK(Component::TRANSFORM_COMPONENT) | MAKE_COMPONENT_MASK(Component::SOUND_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if(SCENE_SYSTEM_RENDER_UPDATE_FLAG & systemsMask)
    {
        renderUpdateSystem = new RenderUpdateSystem(this);
        AddSystem(renderUpdateSystem, MAKE_COMPONENT_MASK(Component::TRANSFORM_COMPONENT) | MAKE_COMPONENT_MASK(Component::RENDER_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if(SCENE_SYSTEM_UPDATEBLE_FLAG & systemsMask)
    {
        updatableSystem = new UpdateSystem(this);
        AddSystem(updatableSystem, MAKE_COMPONENT_MASK(Component::UPDATABLE_COMPONENT));
    }

    if(SCENE_SYSTEM_LIGHT_UPDATE_FLAG & systemsMask)
    {
        lightUpdateSystem = new LightUpdateSystem(this);
        AddSystem(lightUpdateSystem, MAKE_COMPONENT_MASK(Component::TRANSFORM_COMPONENT) | MAKE_COMPONENT_MASK(Component::LIGHT_COMPONENT));
    }

    if(SCENE_SYSTEM_ACTION_UPDATE_FLAG & systemsMask)
    {
        actionSystem = new ActionUpdateSystem(this);
        AddSystem(actionSystem, MAKE_COMPONENT_MASK(Component::ACTION_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if(SCENE_SYSTEM_SKYBOX_FLAG & systemsMask)
    {
        skyboxSystem = new SkyboxSystem(this);
        AddSystem(skyboxSystem, MAKE_COMPONENT_MASK(Component::RENDER_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if(SCENE_SYSTEM_MATERIAL_FLAG & systemsMask)
    {
        materialSystem = new MaterialSystem(this);
        AddSystem(materialSystem, MAKE_COMPONENT_MASK(Component::RENDER_COMPONENT));
    }

    if(SCENE_SYSTEM_DEBUG_RENDER_FLAG & systemsMask)
    {
        debugRenderSystem = new DebugRenderSystem(this);
        AddSystem(debugRenderSystem, MAKE_COMPONENT_MASK(Component::DEBUG_RENDER_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }
    
    if(SCENE_SYSTEM_FOLIAGE_FLAG & systemsMask)
    {
        foliageSystem = new FoliageSystem(this);
        AddSystem(foliageSystem, MAKE_COMPONENT_MASK(Component::RENDER_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if(SCENE_SYSTEM_SPEEDTREE_UPDATE_FLAG & systemsMask)
    {
        speedTreeUpdateSystem = new SpeedTreeUpdateSystem(this);
        AddSystem(speedTreeUpdateSystem, MAKE_COMPONENT_MASK(Component::SPEEDTREE_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if(SCENE_SYSTEM_WIND_UPDATE_FLAG & systemsMask)
    {
        windSystem = new WindSystem(this);
        AddSystem(windSystem, MAKE_COMPONENT_MASK(Component::WIND_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if(SCENE_SYSTEM_WAVE_UPDATE_FLAG & systemsMask)
    {
        waveSystem = new WaveSystem(this);
        AddSystem(waveSystem, MAKE_COMPONENT_MASK(Component::WAVE_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if(SCENE_SYSTEM_SKELETON_UPDATE_FLAG & systemsMask)
    {
        skeletonSystem = new SkeletonSystem(this);
        AddSystem(skeletonSystem, MAKE_COMPONENT_MASK(Component::SKELETON_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }
}

Scene::~Scene()
{
    RenderManager::Instance()->GetOptions()->RemoveObserver(this);

    SceneCache::Instance()->RemoveScene(this);
    
	for (Vector<AnimatedMesh*>::iterator t = animatedMeshes.begin(); t != animatedMeshes.end(); ++t)
	{
		AnimatedMesh * obj = *t;
		obj->Release();
	}
	animatedMeshes.clear();
	
	for (Vector<Camera*>::iterator t = cameras.begin(); t != cameras.end(); ++t)
	{
		Camera * obj = *t;
		obj->Release();
	}
	cameras.clear();
    
    SafeRelease(mainCamera);
    SafeRelease(drawCamera);

    // Children should be removed first because they should unregister themselves in managers
	RemoveAllChildren();
    
	SafeRelease(imposterManager);

    SafeRelease(sceneGlobalMaterial);

    transformSystem = 0;
    renderUpdateSystem = 0;
    lodSystem = 0;
    debugRenderSystem = 0;
    particleEffectSystem = 0;
    updatableSystem = 0;
    lightUpdateSystem = 0;
    switchSystem = 0;
    soundSystem = 0;
    actionSystem = 0;
    skyboxSystem = 0;
    staticOcclusionSystem = 0;
    materialSystem = 0;
    speedTreeUpdateSystem = 0;
    foliageSystem = 0;
    windSystem = 0;
    waveSystem = 0;
    animationSystem = 0;
    
    uint32 size = (uint32)systems.size();
    for (uint32 k = 0; k < size; ++k)
        SafeDelete(systems[k]);
    systems.clear();

    systemsToProcess.clear();
    systemsToInput.clear();
    cache.ClearAll();

	SafeDelete(eventSystem);
	SafeDelete(renderSystem);
}
    
void Scene::RegisterEntity(Entity * entity)
{
    if( entity->GetID() == 0 || 
        entity->GetSceneID() == 0 ||
        entity->GetSceneID() != sceneId)
    {
        entity->SetID(++maxEntityIDCounter);
        entity->SetSceneID(sceneId);
    }

    for(auto& system : systems)
    {
        system->RegisterEntity(entity);
    }
}

void Scene::UnregisterEntity(Entity * entity)
{
    for(auto& system : systems)
    {
        system->UnregisterEntity(entity);
    }
}

void Scene::RegisterEntitiesInSystemRecursively(SceneSystem *system, Entity * entity)
{
    system->RegisterEntity(entity);
    for (int32 i=0, sz = entity->GetChildrenCount(); i<sz; ++i)
        RegisterEntitiesInSystemRecursively(system, entity->GetChild(i));
}
void Scene::UnregisterEntitiesInSystemRecursively(SceneSystem *system, Entity * entity)
{
    system->UnregisterEntity(entity);
    for (int32 i=0, sz = entity->GetChildrenCount(); i<sz; ++i)
        UnregisterEntitiesInSystemRecursively(system, entity->GetChild(i));
}

void Scene::RegisterComponent(Entity * entity, Component * component)
{
    DVASSERT(entity && component);
    uint32 systemsCount = static_cast<uint32>(systems.size());
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        systems[k]->RegisterComponent(entity, component);
    }
}

void Scene::UnregisterComponent(Entity * entity, Component * component)
{
    DVASSERT(entity && component);
    uint32 systemsCount = static_cast<uint32>(systems.size());
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        systems[k]->UnregisterComponent(entity, component);
    }
    
}


#if 0 // Removed temporarly if everything will work with events can be removed fully.
void Scene::ImmediateEvent(Entity * entity, uint32 componentType, uint32 event)
{
#if 1
    uint32 systemsCount = systems.size();
    uint64 updatedComponentFlag = MAKE_COMPONENT_MASK(componentType);
    uint64 componentsInEntity = entity->GetAvailableComponentFlags();

    for (uint32 k = 0; k < systemsCount; ++k)
    {
        uint64 requiredComponentFlags = systems[k]->GetRequiredComponents();
        
        if (((requiredComponentFlags & updatedComponentFlag) != 0) && ((requiredComponentFlags & componentsInEntity) == requiredComponentFlags))
        {
			eventSystem->NotifySystem(systems[k], entity, event);
        }
    }
#else
    uint64 componentsInEntity = entity->GetAvailableComponentFlags();
    Set<SceneSystem*> & systemSetForType = componentTypeMapping.GetValue(componentsInEntity);
    
    for (Set<SceneSystem*>::iterator it = systemSetForType.begin(); it != systemSetForType.end(); ++it)
    {
        SceneSystem * system = *it;
        uint64 requiredComponentFlags = system->GetRequiredComponents();
        if ((requiredComponentFlags & componentsInEntity) == requiredComponentFlags)
            eventSystem->NotifySystem(system, entity, event);
    }
#endif
}
#endif
    
void Scene::AddSystem(SceneSystem * sceneSystem, uint64 componentFlags, uint32 processFlags /*= 0*/, SceneSystem * insertBeforeSceneForProcess /* = nullptr */)
{
    sceneSystem->SetRequiredComponents(componentFlags);
    //Set<SceneSystem*> & systemSetForType = componentTypeMapping.GetValue(componentFlags);
    //systemSetForType.insert(sceneSystem);
    systems.push_back(sceneSystem);

    if(processFlags & SCENE_SYSTEM_REQUIRE_PROCESS)
    {
        bool wasInsertedForUpdate = false;
        if(insertBeforeSceneForProcess)
        {
            Vector<SceneSystem*>::iterator itEnd = systemsToProcess.end();
            for (Vector<SceneSystem*>::iterator it = systemsToProcess.begin(); it != itEnd; ++it)
            {
                if(insertBeforeSceneForProcess == (*it))
                {
                    systemsToProcess.insert(it, sceneSystem);
                    wasInsertedForUpdate = true;
                    break;
                }
            }
        }
        else
        {
            systemsToProcess.push_back(sceneSystem);
            wasInsertedForUpdate = true;
        }
        DVASSERT(wasInsertedForUpdate);
    }
    
    if(processFlags & SCENE_SYSTEM_REQUIRE_INPUT)
    {
        systemsToInput.push_back(sceneSystem);
    }
    
    RegisterEntitiesInSystemRecursively(sceneSystem, this);
}
    
void Scene::RemoveSystem(SceneSystem * sceneSystem)
{
    UnregisterEntitiesInSystemRecursively(sceneSystem, this);
    
    RemoveSystem(systemsToProcess, sceneSystem);
    RemoveSystem(systemsToInput, sceneSystem);

    DVVERIFY(RemoveSystem(systems, sceneSystem));
}

    
bool Scene::RemoveSystem(Vector<SceneSystem*> &storage, SceneSystem *system)
{
    Vector<SceneSystem*>::iterator endIt = storage.end();
    for(Vector<SceneSystem*>::iterator it = storage.begin(); it != endIt; ++it)
    {
        if(*it == system)
        {
            storage.erase(it);
            return true;
        }
    }
    
    return false;
}
    
    
    
Scene * Scene::GetScene()
{
    return this;
}
    
void Scene::AddAnimatedMesh(AnimatedMesh * mesh)
{
	if (mesh)
	{
		mesh->Retain();
		animatedMeshes.push_back(mesh);
	}	
}

void Scene::RemoveAnimatedMesh(AnimatedMesh * mesh)
{
	
}

AnimatedMesh * Scene::GetAnimatedMesh(int32 index)
{
	return animatedMeshes[index];
}
	
	
	
void Scene::AddCamera(Camera * camera)
{
	if (camera)
	{
		camera->Retain();
		cameras.push_back(camera);
	}
}

Camera * Scene::GetCamera(int32 n)
{
	if (n >= 0 && n < (int32)cameras.size())
		return cameras[n];
	
	return nullptr;
}
    
void Scene::SetupTestLighting()
{
#ifdef __DAVAENGINE_IPHONE__
//	glShadeModel(GL_SMOOTH);
//	// enable lighting
//	glEnable(GL_LIGHTING);
//	glEnable(GL_NORMALIZE);
//	
//	// deactivate all lights
//	for (int i=0; i<8; i++)  glDisable(GL_LIGHT0 + i);
//	
//	// ambiental light to nothing
//	GLfloat ambientalLight[]= {0.2f, 0.2f, 0.2f, 1.0f};
//	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientalLight);
//	
////	GLfloat light_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };  // delete
//	//GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
//	GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
//	
//	GLfloat light_diffuse[4];
//	light_diffuse[0]=1.0f;
//	light_diffuse[1]=1.0f;
//	light_diffuse[2]=1.0f;
//	light_diffuse[3]=1.0f;
//	
//	GLfloat lightPos[] = { 0.0f, 0.0f, 1.0f, 0.0f };
//	
//	// activate this light
//	glEnable(GL_LIGHT0);
//	
//	//always position 0,0,0 because light  is moved with transformations
//	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
//	
//	// colors 
//	glLightfv(GL_LIGHT0, GL_AMBIENT, light_diffuse); // now like diffuse color
//	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
//	glLightfv(GL_LIGHT0, GL_SPECULAR,light_specular);
//	
//	//specific values for this light
//	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1);
//	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0);
//	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0);
//	
//	//other values
//	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 30.0f);
//	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 0.0f);
//	GLfloat spotdirection[] = { 0.0f, 0.0f, -1.0f, 0.0f }; // irrelevant for this light (I guess)
//	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spotdirection); 
#endif
}
    
void Scene::Update(float timeElapsed)
{
    TIME_PROFILE("Scene::Update");

    uint64 time = SystemTimer::Instance()->AbsoluteMS();

    uint32 size = (uint32)systemsToProcess.size();
    for (uint32 k = 0; k < size; ++k)
    {
        SceneSystem * system = systemsToProcess[k];
        if((systemsMask & SCENE_SYSTEM_UPDATEBLE_FLAG) && system == transformSystem)
        {
            updatableSystem->UpdatePreTransform(timeElapsed);
            transformSystem->Process(timeElapsed);
            updatableSystem->UpdatePostTransform(timeElapsed);
        }
        else if(system == lodSystem)
        {
            if(RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_LODS))
            {
                lodSystem->Process(timeElapsed);
            }
        }
        else
        {
            system->Process(timeElapsed);
        }
    }

// 	int32 size;
// 	
// 	size = (int32)animations.size();
// 	for (int32 animationIndex = 0; animationIndex < size; ++animationIndex)
// 	{
// 		SceneNodeAnimationList * anim = animations[animationIndex];
// 		anim->Update(timeElapsed);
// 	}
// 
// 	if(RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_ANIMATED_MESHES))
// 	{
// 		size = (int32)animatedMeshes.size();
// 		for (int32 animatedMeshIndex = 0; animatedMeshIndex < size; ++animatedMeshIndex)
// 		{
// 			AnimatedMesh * mesh = animatedMeshes[animatedMeshIndex];
// 			mesh->Update(timeElapsed);
// 		}
// 	}

	//if(imposterManager)
	//{
	//	imposterManager->Update(timeElapsed);
	//}

    updateTime = SystemTimer::Instance()->AbsoluteMS() - time;
}

void Scene::Draw()
{
	TIME_PROFILE("Scene::Draw");

	//float timeElapsed = SystemTimer::Instance()->FrameDelta();

	shadowVolumes.clear();
    
    if(nullptr != sceneGlobalMaterial)
    {
        NMaterialProperty* propShadowColor = sceneGlobalMaterial->GetMaterialProperty(NMaterial::PARAM_SHADOW_COLOR);
        if(nullptr != propShadowColor)
        {
            DVASSERT(Shader::UT_FLOAT_VEC4 == propShadowColor->type);
            
            float32* propDataPtr = (float32*)propShadowColor->data;
            Color shadowColor(propDataPtr[0], propDataPtr[1], propDataPtr[2], propDataPtr[3]);
            renderSystem->SetShadowRectColor(shadowColor);
        }
    }
    
    uint64 time = SystemTimer::Instance()->AbsoluteMS();
    
    if(imposterManager)
	{
		//imposterManager->ProcessQueue();
	}
    
    renderSystem->Render(clearBuffers);
    
    //foliageSystem->DebugDrawVegetation();
    
	drawTime = SystemTimer::Instance()->AbsoluteMS() - time;
}
    
void Scene::SceneDidLoaded()
{
    maxEntityIDCounter = 0;

    std::function<void(Entity *)> findMaxId = [&](Entity *entity)
    {
        if(maxEntityIDCounter < entity->id) maxEntityIDCounter = entity->id;
        for (auto child : entity->children) findMaxId(child);
    };

    findMaxId(this);

    uint32 systemsCount = static_cast<uint32>(systems.size());
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        systems[k]->SceneDidLoaded();
    }
}


	
// void Scene::StopAllAnimations(bool recursive )
// {
// 	int32 size = (int32)animations.size();
// 	for (int32 animationIndex = 0; animationIndex < size; ++animationIndex)
// 	{
// 		SceneNodeAnimationList * anim = animations[animationIndex];
// 		anim->StopAnimation();
// 	}
// 	Entity::StopAllAnimations(recursive);
// }
    
    
void Scene::SetCurrentCamera(Camera * _camera)
{
    SafeRelease(mainCamera);
    mainCamera = SafeRetain(_camera);
    SafeRelease(drawCamera);
    drawCamera = SafeRetain(_camera);
}

Camera * Scene::GetCurrentCamera() const
{
    return mainCamera;
}

void Scene::SetCustomDrawCamera(Camera * _camera)
{
    SafeRelease(drawCamera);
    drawCamera = SafeRetain(_camera);
}

Camera * Scene::GetDrawCamera() const
{
    return drawCamera;
}
 
//void Scene::SetForceLodLayer(int32 layer)
//{
//    forceLodLayer = layer;
//}
//int32 Scene::GetForceLodLayer()
//{
//    return forceLodLayer;
//}
//
//int32 Scene::RegisterLodLayer(float32 nearDistance, float32 farDistance)
//{
//    LodLayer newLevel;
//    newLevel.nearDistance = nearDistance;
//    newLevel.farDistance = farDistance;
//    newLevel.nearDistanceSq = nearDistance * nearDistance;
//    newLevel.farDistanceSq = farDistance * farDistance;
//    int i = 0;
//    
//    for (Vector<LodLayer>::iterator it = lodLayers.begin(); it < lodLayers.end(); it++)
//    {
//        if (nearDistance < it->nearDistance)
//        {
//            lodLayers.insert(it, newLevel);
//            return i;
//        }
//        i++;
//    }
//    
//    lodLayers.push_back(newLevel);
//    return i;
//}
//    
//void Scene::ReplaceLodLayer(int32 layerNum, float32 nearDistance, float32 farDistance)
//{
//    DVASSERT(layerNum < (int32)lodLayers.size());
//    
//    lodLayers[layerNum].nearDistance = nearDistance;
//    lodLayers[layerNum].farDistance = farDistance;
//    lodLayers[layerNum].nearDistanceSq = nearDistance * nearDistance;
//    lodLayers[layerNum].farDistanceSq = farDistance * farDistance;
//    
//    
////    LodLayer newLevel;
////    newLevel.nearDistance = nearDistance;
////    newLevel.farDistance = farDistance;
////    newLevel.nearDistanceSq = nearDistance * nearDistance;
////    newLevel.farDistanceSq = farDistance * farDistance;
////    int i = 0;
////    
////    for (Vector<LodLayer>::iterator it = lodLayers.begin(); it < lodLayers.end(); it++)
////    {
////        if (nearDistance < it->nearDistance)
////        {
////            lodLayers.insert(it, newLevel);
////            return i;
////        }
////        i++;
////    }
////    
////    lodLayers.push_back(newLevel);
////    return i;
//}
//    
    

void Scene::AddDrawTimeShadowVolume(ShadowVolumeNode * shadowVolume)
{
	shadowVolumes.push_back(shadowVolume);
}

    
void Scene::UpdateLights()
{
    
    
    
    
}
    
Light * Scene::GetNearestDynamicLight(Light::eType type, Vector3 position)
{
    switch(type)
    {
        case Light::TYPE_DIRECTIONAL:
            
            break;
            
        default:
            break;
    };
    
	float32 squareMinDistance = 10000000.0f;
	Light * nearestLight = 0;

	Set<Light*> & lights = GetLights();
	const Set<Light*>::iterator & endIt = lights.end();
	for (Set<Light*>::iterator it = lights.begin(); it != endIt; ++it)
	{
		Light * node = *it;
		if(node->IsDynamic())
		{
			const Vector3 & lightPosition = node->GetPosition();

			float32 squareDistanceToLight = (position - lightPosition).SquareLength();
			if (squareDistanceToLight < squareMinDistance)
			{
				squareMinDistance = squareDistanceToLight;
				nearestLight = node;
			}
		}
	}

	return nearestLight;
}

Set<Light*> & Scene::GetLights()
{
    return lights;
}

void Scene::RegisterImposter(ImposterNode * imposter)
{
	if(!imposterManager)
	{
		imposterManager = new ImposterManager(this);
	}
	
	imposterManager->Add(imposter);
}

void Scene::UnregisterImposter(ImposterNode * imposter)
{
	imposterManager->Remove(imposter);

	if(imposterManager->IsEmpty())
	{
		SafeRelease(imposterManager);
	}
}

EventSystem * Scene::GetEventSystem() const
{
	return eventSystem;
}

RenderSystem * Scene::GetRenderSystem() const
{
	return renderSystem;
}

MaterialSystem * Scene::GetMaterialSystem() const
{
    return materialSystem;
}

AnimationSystem * Scene::GetAnimationSystem() const
{
    return animationSystem;
}

/*void Scene::Save(KeyedArchive * archive)
{
    // Perform refactoring and add Matrix4, Vector4 types to VariantType and KeyedArchive
    Entity::Save(archive);
    
    
    
    
    
}

void Scene::Load(KeyedArchive * archive)
{
    Entity::Load(archive);
}*/
    

SceneFileV2::eError Scene::LoadScene(const DAVA::FilePath & pathname)
{
    SceneFileV2::eError ret = SceneFileV2::ERROR_FAILED_TO_CREATE_FILE;

    RemoveAllChildren();
    SetName(pathname.GetFilename().c_str());

    if(pathname.IsEqualToExtension(".sce"))
    {
        ScopedPtr<SceneFile> file(new SceneFile());
        file->SetDebugLog(true);
        if(file->LoadScene(pathname, this))
        {
            ret = SceneFileV2::ERROR_NO_ERROR;
        }
    }
    else if(pathname.IsEqualToExtension(".sc2"))
    {
        ScopedPtr<SceneFileV2> file(new SceneFileV2());
        file->EnableDebugLog(false);
        ret = file->LoadScene(pathname, this);
    }

    return ret;
}

SceneFileV2::eError Scene::SaveScene(const DAVA::FilePath & pathname, bool saveForGame /*= false*/)
{
    std::function<void(Entity *)> resolveId = [&](Entity *entity)
    {
        if(0 == entity->id) entity->id = ++maxEntityIDCounter;
        for(auto child : entity->children) resolveId(child);
    };

    resolveId(this);

    ScopedPtr<SceneFileV2> file(new SceneFileV2());
    file->EnableDebugLog(false);
	file->EnableSaveForGame(saveForGame);
	return file->SaveScene(pathname, this);
}
    
void Scene::OptimizeBeforeExport()
{
    Set<NMaterial*> materials;
    materialSystem->BuildMaterialList(this, materials);
    
    ImportShadowColor(this);

    Set<NMaterial *>::const_iterator endIt = materials.end();
    for(Set<NMaterial *>::const_iterator it = materials.begin(); it != endIt; ++it)
        (*it)->ReleaseIlluminationParams();

    Entity::OptimizeBeforeExport();
}

void Scene::ImportShadowColor(Entity * rootNode)
{
    if(nullptr != sceneGlobalMaterial)
    {
		Entity * landscapeNode = FindLandscapeEntity(rootNode);
		if(nullptr != landscapeNode)
		{
			// try to get shadow color for landscape
			KeyedArchive * props = GetCustomPropertiesArchieve(landscapeNode);
			if (props->IsKeyExists("ShadowColor"))
			{
				Color shadowColor = props->GetVariant("ShadowColor")->AsColor();
				sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_SHADOW_COLOR,
					Shader::UT_FLOAT_VEC4,
					1,
                    shadowColor.color);

				props->DeleteKey("ShadowColor");
			}
		}
    }
}

void Scene::OnSceneReady(Entity * rootNode)
{
    ImportShadowColor(rootNode);
}

void Scene::SetClearBuffers(uint32 buffers) 
{
    clearBuffers = buffers;
}
uint32 Scene::GetClearBuffers() const 
{
    return clearBuffers;
}

    
void Scene::Input(DAVA::UIEvent *event)
{
    uint32 size = (uint32)systemsToInput.size();
    for (uint32 k = 0; k < size; ++k)
    {
        SceneSystem * system = systemsToInput[k];
        system->Input(event);
    }
}

void Scene::HandleEvent(Observable * observable)
{
    RenderOptions * options = dynamic_cast<RenderOptions *>(observable);
    if (options->IsOptionEnabled(RenderOptions::REPLACE_LIGHTMAP_MIPMAPS))
        MipMapReplacer::ReplaceMipMaps(this, NMaterial::TEXTURE_LIGHTMAP);
    if (options->IsOptionEnabled(RenderOptions::REPLACE_ALBEDO_MIPMAPS))
        MipMapReplacer::ReplaceMipMaps(this, NMaterial::TEXTURE_ALBEDO);

    if (options->IsOptionEnabled(RenderOptions::DEBUG_DRAW_STATIC_OCCLUSION) && !staticOcclusionDebugDrawSystem)
    {
        staticOcclusionDebugDrawSystem = new StaticOcclusionDebugDrawSystem(this);
        AddSystem(staticOcclusionDebugDrawSystem, MAKE_COMPONENT_MASK(Component::STATIC_OCCLUSION_COMPONENT), 0, renderUpdateSystem);
    }
    else if (!options->IsOptionEnabled(RenderOptions::DEBUG_DRAW_STATIC_OCCLUSION) && staticOcclusionDebugDrawSystem)
    {
        RemoveSystem(staticOcclusionDebugDrawSystem);
        SafeDelete(staticOcclusionDebugDrawSystem);
    }
}
    
void Scene::Activate()
{
    for(auto system : systems)
    {
        system->Activate();
    }
}

void Scene::Deactivate()
{
    for(auto system : systems)
    {
        system->Deactivate();
    }
}

};
