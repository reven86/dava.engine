
#include "Scene3D/Scene.h"
#include "Render/Texture.h"
#include "Render/3D/StaticMesh.h"
#include "Render/Image/Image.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/RenderOptions.h"
#include "Render/MipmapReplacer.h"

#include "Time/SystemTimer.h"
#include "FileSystem/FileSystem.h"

#include "Scene3D/SceneFileV2.h"
#include "Scene3D/DataNode.h"
#include "Render/Highlevel/Light.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/RenderSystem.h"

#include "Entity/SceneSystem.h"
#include "Scene3D/Systems/TransformSystem.h"
#include "Scene3D/Systems/RenderUpdateSystem.h"
#include "Scene3D/Lod/LodComponent.h"
#include "Scene3D/Lod/LodSystem.h"
#include "Scene3D/Systems/DebugRenderSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/ParticleEffectSystem.h"
#include "Scene3D/Systems/UpdateSystem.h"
#include "Scene3D/Systems/LightUpdateSystem.h"
#include "Scene3D/Systems/SwitchSystem.h"
#include "Scene3D/Systems/ActionUpdateSystem.h"
#include "Scene3D/Systems/WindSystem.h"
#include "Scene3D/Systems/WaveSystem.h"
#include "Scene3D/Systems/SkeletonSystem.h"
#include "Scene3D/Systems/AnimationSystem.h"
#include "Scene3D/Systems/LandscapeSystem.h"
#include "Scene3D/Systems/SoundUpdateSystem.h"
#include "Scene3D/Systems/ParticleEffectDebugDrawSystem.h"

#include "Scene3D/Components/SingleComponents/TransformSingleComponent.h"

#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Concurrency/Thread.h"

#include "Sound/SoundSystem.h"

#include "Scene3D/Systems/SpeedTreeUpdateSystem.h"

#include "Scene3D/Systems/StaticOcclusionSystem.h"
#include "Scene3D/Systems/FoliageSystem.h"

#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/TransformComponent.h"
#include "UI/UIEvent.h"
#include "Render/Highlevel/RenderPass.h"

#include "Render/Renderer.h"

#include <functional>

namespace DAVA
{
//TODO: remove this crap with shadow color
EntityCache::~EntityCache()
{
    ClearAll();
}

void EntityCache::Preload(const FilePath& path)
{
    Scene* scene = new Scene(0);
    if (SceneFileV2::ERROR_NO_ERROR == scene->LoadScene(path))
    {
        Entity* srcRootEntity = scene;

        // try to perform little optimization:
        // if scene has single node with identity transform
        // we can skip this entity and move only its children
        if (1 == srcRootEntity->GetChildrenCount())
        {
            Entity* child = srcRootEntity->GetChild(0);
            if (1 == child->GetComponentCount())
            {
                TransformComponent* tr = static_cast<TransformComponent*>(srcRootEntity->GetComponent(Component::TRANSFORM_COMPONENT));
                if (nullptr != tr && tr->GetLocalTransform() == Matrix4::IDENTITY)
                {
                    srcRootEntity = child;
                }
            }
        }

        auto count = srcRootEntity->GetChildrenCount();

        Vector<Entity*> tempV;
        tempV.reserve(count);
        for (auto i = 0; i < count; ++i)
        {
            tempV.push_back(srcRootEntity->GetChild(i));
        }

        Entity* dstRootEntity = new Entity();
        for (auto i = 0; i < count; ++i)
        {
            dstRootEntity->AddNode(tempV[i]);
        }

        dstRootEntity->ResetID();
        dstRootEntity->SetName(scene->GetName());
        cachedEntities[path] = dstRootEntity;
    }

    SafeRelease(scene);
}

Entity* EntityCache::GetOriginal(const FilePath& path)
{
    Entity* ret = nullptr;

    if (cachedEntities.find(path) == cachedEntities.end())
    {
        Preload(path);
    }

    auto i = cachedEntities.find(path);
    if (i != cachedEntities.end())
    {
        ret = i->second;
    }

    return ret;
}

Entity* EntityCache::GetClone(const FilePath& path)
{
    Entity* ret = nullptr;

    Entity* orig = GetOriginal(path);
    if (nullptr != orig)
    {
        ret = orig->Clone();
    }

    return ret;
}

void EntityCache::Clear(const FilePath& path)
{
    auto i = cachedEntities.find(path);
    if (i != cachedEntities.end())
    {
        SafeRelease(i->second);
        cachedEntities.erase(i);
    }
}

void EntityCache::ClearAll()
{
    for (auto& i : cachedEntities)
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
    , staticOcclusionSystem(0)
    , foliageSystem(0)
    , windSystem(0)
    , animationSystem(0)
    , staticOcclusionDebugDrawSystem(0)
    , particleEffectDebugDrawSystem(0)
    , systemsMask(_systemsMask)
    , maxEntityIDCounter(0)
    , sceneGlobalMaterial(0)
    , mainCamera(0)
    , drawCamera(0)
{
    static uint32 idCounter = 0;
    sceneId = ++idCounter;

    CreateComponents();
    CreateSystems();

    // this will force scene to create hidden global material
    SetGlobalMaterial(nullptr);

    RenderOptions* options = Renderer::GetOptions();
    options->AddObserver(this);
}

void Scene::CreateComponents()
{
}

NMaterial* Scene::GetGlobalMaterial() const
{
    return sceneGlobalMaterial;
}

void Scene::SetGlobalMaterial(NMaterial* globalMaterial)
{
    SafeRelease(sceneGlobalMaterial);
    sceneGlobalMaterial = SafeRetain(globalMaterial);

    renderSystem->SetGlobalMaterial(sceneGlobalMaterial);

    if (nullptr != particleEffectSystem)
        particleEffectSystem->SetGlobalMaterial(sceneGlobalMaterial);
}

void Scene::SetMainPassProperties(uint32 priority, const Rect& viewport, uint32 width, uint32 height, PixelFormat format)
{
    renderSystem->SetMainPassProperties(priority, viewport, width, height, format);
}

void Scene::SetMainRenderTarget(rhi::HTexture color, rhi::HTexture depthStencil, rhi::LoadAction colorLoadAction, const Color& clearColor)
{
    renderSystem->SetMainRenderTarget(color, depthStencil, colorLoadAction, clearColor);
}

rhi::RenderPassConfig& Scene::GetMainPassConfig()
{
    return renderSystem->GetMainPassConfig();
}

void Scene::CreateSystems()
{
    renderSystem = new RenderSystem();
    eventSystem = new EventSystem();

    if (SCENE_SYSTEM_STATIC_OCCLUSION_FLAG & systemsMask)
    {
        staticOcclusionSystem = new StaticOcclusionSystem(this);
        AddSystem(staticOcclusionSystem, MAKE_COMPONENT_MASK(Component::STATIC_OCCLUSION_DATA_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if (SCENE_SYSTEM_ANIMATION_FLAG & systemsMask)
    {
        animationSystem = new AnimationSystem(this);
        AddSystem(animationSystem, MAKE_COMPONENT_MASK(Component::ANIMATION_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if (SCENE_SYSTEM_TRANSFORM_FLAG & systemsMask)
    {
        transformSystem = new TransformSystem(this);
        AddSystem(transformSystem, MAKE_COMPONENT_MASK(Component::TRANSFORM_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);

        transformSingleComponent = new TransformSingleComponent;
    }

    if (SCENE_SYSTEM_LOD_FLAG & systemsMask)
    {
        lodSystem = new LodSystem(this);
        AddSystem(lodSystem, MAKE_COMPONENT_MASK(Component::LOD_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if (SCENE_SYSTEM_SWITCH_FLAG & systemsMask)
    {
        switchSystem = new SwitchSystem(this);
        AddSystem(switchSystem, MAKE_COMPONENT_MASK(Component::SWITCH_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if (SCENE_SYSTEM_PARTICLE_EFFECT_FLAG & systemsMask)
    {
        particleEffectSystem = new ParticleEffectSystem(this);
        particleEffectSystem->SetGlobalMaterial(GetGlobalMaterial());
        AddSystem(particleEffectSystem, MAKE_COMPONENT_MASK(Component::PARTICLE_EFFECT_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if (SCENE_SYSTEM_SOUND_UPDATE_FLAG & systemsMask)
    {
        soundSystem = new SoundUpdateSystem(this);
        AddSystem(soundSystem, MAKE_COMPONENT_MASK(Component::TRANSFORM_COMPONENT) | MAKE_COMPONENT_MASK(Component::SOUND_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if (SCENE_SYSTEM_RENDER_UPDATE_FLAG & systemsMask)
    {
        renderUpdateSystem = new RenderUpdateSystem(this);
        AddSystem(renderUpdateSystem, MAKE_COMPONENT_MASK(Component::TRANSFORM_COMPONENT) | MAKE_COMPONENT_MASK(Component::RENDER_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if (SCENE_SYSTEM_UPDATEBLE_FLAG & systemsMask)
    {
        updatableSystem = new UpdateSystem(this);
        AddSystem(updatableSystem, MAKE_COMPONENT_MASK(Component::UPDATABLE_COMPONENT));
    }

    if (SCENE_SYSTEM_LIGHT_UPDATE_FLAG & systemsMask)
    {
        lightUpdateSystem = new LightUpdateSystem(this);
        AddSystem(lightUpdateSystem, MAKE_COMPONENT_MASK(Component::TRANSFORM_COMPONENT) | MAKE_COMPONENT_MASK(Component::LIGHT_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if (SCENE_SYSTEM_ACTION_UPDATE_FLAG & systemsMask)
    {
        actionSystem = new ActionUpdateSystem(this);
        AddSystem(actionSystem, MAKE_COMPONENT_MASK(Component::ACTION_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if (SCENE_SYSTEM_DEBUG_RENDER_FLAG & systemsMask)
    {
        debugRenderSystem = new DebugRenderSystem(this);
        AddSystem(debugRenderSystem, MAKE_COMPONENT_MASK(Component::DEBUG_RENDER_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if (SCENE_SYSTEM_LANDSCAPE_FLAG & systemsMask)
    {
        landscapeSystem = new LandscapeSystem(this);
        AddSystem(landscapeSystem, MAKE_COMPONENT_MASK(Component::RENDER_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if (SCENE_SYSTEM_FOLIAGE_FLAG & systemsMask)
    {
        foliageSystem = new FoliageSystem(this);
        AddSystem(foliageSystem, MAKE_COMPONENT_MASK(Component::RENDER_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if (SCENE_SYSTEM_SPEEDTREE_UPDATE_FLAG & systemsMask)
    {
        speedTreeUpdateSystem = new SpeedTreeUpdateSystem(this);
        AddSystem(speedTreeUpdateSystem, MAKE_COMPONENT_MASK(Component::SPEEDTREE_COMPONENT) | MAKE_COMPONENT_MASK(Component::RENDER_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if (SCENE_SYSTEM_WIND_UPDATE_FLAG & systemsMask)
    {
        windSystem = new WindSystem(this);
        AddSystem(windSystem, MAKE_COMPONENT_MASK(Component::WIND_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if (SCENE_SYSTEM_WAVE_UPDATE_FLAG & systemsMask)
    {
        waveSystem = new WaveSystem(this);
        AddSystem(waveSystem, MAKE_COMPONENT_MASK(Component::WAVE_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if (SCENE_SYSTEM_SKELETON_UPDATE_FLAG & systemsMask)
    {
        skeletonSystem = new SkeletonSystem(this);
        AddSystem(skeletonSystem, MAKE_COMPONENT_MASK(Component::SKELETON_COMPONENT), SCENE_SYSTEM_REQUIRE_PROCESS);
    }

    if (DAVA::Renderer::GetOptions()->IsOptionEnabled(DAVA::RenderOptions::DEBUG_DRAW_STATIC_OCCLUSION) && !staticOcclusionDebugDrawSystem)
    {
        staticOcclusionDebugDrawSystem = new DAVA::StaticOcclusionDebugDrawSystem(this);
        AddSystem(staticOcclusionDebugDrawSystem, MAKE_COMPONENT_MASK(DAVA::Component::STATIC_OCCLUSION_COMPONENT), 0, renderUpdateSystem);
    }

    if (DAVA::Renderer::GetOptions()->IsOptionEnabled(RenderOptions::DEBUG_DRAW_PARTICLES) && particleEffectDebugDrawSystem == nullptr)
    {
        particleEffectDebugDrawSystem = new ParticleEffectDebugDrawSystem(this);
        AddSystem(particleEffectDebugDrawSystem, 0);
    }
}

Scene::~Scene()
{
    Renderer::GetOptions()->RemoveObserver(this);

    for (Vector<Camera*>::iterator t = cameras.begin(); t != cameras.end(); ++t)
    {
        Camera* obj = *t;
        obj->Release();
    }
    cameras.clear();

    SafeRelease(mainCamera);
    SafeRelease(drawCamera);

    // Children should be removed first because they should unregister themselves in managers
    RemoveAllChildren();

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
    staticOcclusionSystem = 0;
    speedTreeUpdateSystem = 0;
    foliageSystem = 0;
    windSystem = 0;
    waveSystem = 0;
    animationSystem = 0;

    size_t size = systems.size();
    for (size_t k = 0; k < size; ++k)
        SafeDelete(systems[k]);
    systems.clear();

    SafeDelete(transformSingleComponent);

    systemsToProcess.clear();
    systemsToInput.clear();
    cache.ClearAll();

    SafeDelete(eventSystem);
    SafeDelete(renderSystem);
}

void Scene::RegisterEntity(Entity* entity)
{
    if (entity->GetID() == 0 ||
        entity->GetSceneID() == 0 ||
        entity->GetSceneID() != sceneId)
    {
        entity->SetID(++maxEntityIDCounter);
        entity->SetSceneID(sceneId);
    }

    for (auto& system : systems)
    {
        system->RegisterEntity(entity);
    }
}

void Scene::UnregisterEntity(Entity* entity)
{
    if (transformSingleComponent)
    {
        transformSingleComponent->EraseEntity(entity);
    }

    for (auto& system : systems)
    {
        system->UnregisterEntity(entity);
    }
}

void Scene::RegisterEntitiesInSystemRecursively(SceneSystem* system, Entity* entity)
{
    system->RegisterEntity(entity);
    for (int32 i = 0, sz = entity->GetChildrenCount(); i < sz; ++i)
        RegisterEntitiesInSystemRecursively(system, entity->GetChild(i));
}
void Scene::UnregisterEntitiesInSystemRecursively(SceneSystem* system, Entity* entity)
{
    system->UnregisterEntity(entity);
    for (int32 i = 0, sz = entity->GetChildrenCount(); i < sz; ++i)
        UnregisterEntitiesInSystemRecursively(system, entity->GetChild(i));
}

void Scene::RegisterComponent(Entity* entity, Component* component)
{
    DVASSERT(entity && component);
    uint32 systemsCount = static_cast<uint32>(systems.size());
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        systems[k]->RegisterComponent(entity, component);
    }
}

void Scene::UnregisterComponent(Entity* entity, Component* component)
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

void Scene::AddSystem(SceneSystem* sceneSystem, uint64 componentFlags, uint32 processFlags /*= 0*/, SceneSystem* insertBeforeSceneForProcess /* = nullptr */, SceneSystem* insertBeforeSceneForInput /* = nullptr*/)
{
    sceneSystem->SetRequiredComponents(componentFlags);
    //Set<SceneSystem*> & systemSetForType = componentTypeMapping.GetValue(componentFlags);
    //systemSetForType.insert(sceneSystem);
    systems.push_back(sceneSystem);

    auto insertSystemBefore = [sceneSystem](Vector<SceneSystem*>& container, SceneSystem* beforeThisSystem)
    {
        if (beforeThisSystem != nullptr)
        {
            Vector<SceneSystem*>::iterator itEnd = container.end();
            for (Vector<SceneSystem*>::iterator it = container.begin(); it != itEnd; ++it)
            {
                if (beforeThisSystem == (*it))
                {
                    container.insert(it, sceneSystem);
                    return true;
                }
            }
        }
        else
        {
            container.push_back(sceneSystem);
            return true;
        }

        return false;
    };

    if (processFlags & SCENE_SYSTEM_REQUIRE_PROCESS)
    {
        bool wasInsertedForProcess = insertSystemBefore(systemsToProcess, insertBeforeSceneForProcess);
        DVASSERT(wasInsertedForProcess);
    }

    if (processFlags & SCENE_SYSTEM_REQUIRE_INPUT)
    {
        bool wasInsertedForInput = insertSystemBefore(systemsToInput, insertBeforeSceneForInput);
        DVASSERT(wasInsertedForInput);
    }

    sceneSystem->SetScene(this);
    RegisterEntitiesInSystemRecursively(sceneSystem, this);
}

void Scene::RemoveSystem(SceneSystem* sceneSystem)
{
    UnregisterEntitiesInSystemRecursively(sceneSystem, this);

    RemoveSystem(systemsToProcess, sceneSystem);
    RemoveSystem(systemsToInput, sceneSystem);

    bool removed = RemoveSystem(systems, sceneSystem);
    if (removed)
    {
        sceneSystem->SetScene(nullptr);
    }
    else
    {
        DVASSERT(false, "Failed to remove system from scene");
    }
}

bool Scene::RemoveSystem(Vector<SceneSystem*>& storage, SceneSystem* system)
{
    Vector<SceneSystem*>::iterator endIt = storage.end();
    for (Vector<SceneSystem*>::iterator it = storage.begin(); it != endIt; ++it)
    {
        if (*it == system)
        {
            storage.erase(it);
            return true;
        }
    }

    return false;
}

Scene* Scene::GetScene()
{
    return this;
}

void Scene::AddCamera(Camera* camera)
{
    if (camera)
    {
        camera->Retain();
        cameras.push_back(camera);
    }
}

bool Scene::RemoveCamera(Camera* c)
{
    const auto& it = std::find(cameras.begin(), cameras.end(), c);
    if (it != cameras.end())
    {
        SafeRelease(*it);
        cameras.erase(it);
        return true;
    }
    return false;
}

Camera* Scene::GetCamera(int32 n)
{
    if (n >= 0 && n < static_cast<int32>(cameras.size()))
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
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_UPDATE)

    uint64 time = SystemTimer::GetMs();

    size_t size = systemsToProcess.size();
    for (size_t k = 0; k < size; ++k)
    {
        SceneSystem* system = systemsToProcess[k];
        if ((systemsMask & SCENE_SYSTEM_UPDATEBLE_FLAG) && system == transformSystem)
        {
            updatableSystem->UpdatePreTransform(timeElapsed);
            transformSystem->Process(timeElapsed);
            updatableSystem->UpdatePostTransform(timeElapsed);
        }
        else if (system == lodSystem)
        {
            if (Renderer::GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_LODS))
            {
                lodSystem->Process(timeElapsed);
            }
        }
        else
        {
            system->Process(timeElapsed);
        }
    }

    if (transformSingleComponent)
    {
        transformSingleComponent->Clear();
    }

    updateTime = SystemTimer::GetMs() - time;
    sceneGlobalTime += timeElapsed;
}

void Scene::Draw()
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_DRAW)

    //TODO: re-think configuring global dynamic bindings
    static Color defShadowColor(1.f, 0.f, 0.f, 1.f);
    static Color defWaterClearColor(0.f, 0.f, 0.f, 0.f);

    const float32* shadowDataPtr = defShadowColor.color;
    const float32* waterDataPtr = defWaterClearColor.color;
    if (sceneGlobalMaterial && sceneGlobalMaterial->HasLocalProperty(DAVA::NMaterialParamName::DEPRECATED_SHADOW_COLOR_PARAM))
        shadowDataPtr = sceneGlobalMaterial->GetLocalPropValue(DAVA::NMaterialParamName::DEPRECATED_SHADOW_COLOR_PARAM);
    if (sceneGlobalMaterial && sceneGlobalMaterial->HasLocalProperty(DAVA::NMaterialParamName::WATER_CLEAR_COLOR))
        waterDataPtr = sceneGlobalMaterial->GetLocalPropValue(DAVA::NMaterialParamName::WATER_CLEAR_COLOR);

    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SHADOW_COLOR, shadowDataPtr, reinterpret_cast<pointer_size>(shadowDataPtr));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WATER_CLEAR_COLOR, waterDataPtr, reinterpret_cast<pointer_size>(waterDataPtr));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_GLOBAL_TIME, &sceneGlobalTime, reinterpret_cast<pointer_size>(&sceneGlobalTime));

    uint64 time = SystemTimer::GetMs();

    renderSystem->Render();

    if (particleEffectDebugDrawSystem != nullptr)
        particleEffectDebugDrawSystem->Draw();
    //foliageSystem->DebugDrawVegetation();

    drawTime = SystemTimer::GetMs() - time;
}

void Scene::SceneDidLoaded()
{
    maxEntityIDCounter = 0;

    std::function<void(Entity*)> findMaxId = [&](Entity* entity)
    {
        if (maxEntityIDCounter < entity->id)
            maxEntityIDCounter = entity->id;
        for (auto child : entity->children) findMaxId(child);
    };

    findMaxId(this);

    uint32 systemsCount = static_cast<uint32>(systems.size());
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        systems[k]->SceneDidLoaded();
    }
}

void Scene::SetCurrentCamera(Camera* _camera)
{
    SafeRelease(mainCamera);
    mainCamera = SafeRetain(_camera);
    SafeRelease(drawCamera);
    drawCamera = SafeRetain(_camera);
}

Camera* Scene::GetCurrentCamera() const
{
    return mainCamera;
}

void Scene::SetCustomDrawCamera(Camera* _camera)
{
    SafeRelease(drawCamera);
    drawCamera = SafeRetain(_camera);
}

Camera* Scene::GetDrawCamera() const
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

void Scene::UpdateLights()
{
}

Light* Scene::GetNearestDynamicLight(Light::eType type, Vector3 position)
{
    switch (type)
    {
    case Light::TYPE_DIRECTIONAL:

        break;

    default:
        break;
    };

    float32 squareMinDistance = 10000000.0f;
    Light* nearestLight = 0;

    Set<Light*>& lights = GetLights();
    const Set<Light*>::iterator& endIt = lights.end();
    for (Set<Light*>::iterator it = lights.begin(); it != endIt; ++it)
    {
        Light* node = *it;
        if (node->IsDynamic())
        {
            const Vector3& lightPosition = node->GetPosition();

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

Set<Light*>& Scene::GetLights()
{
    return lights;
}

EventSystem* Scene::GetEventSystem() const
{
    return eventSystem;
}

RenderSystem* Scene::GetRenderSystem() const
{
    return renderSystem;
}

AnimationSystem* Scene::GetAnimationSystem() const
{
    return animationSystem;
}

ParticleEffectDebugDrawSystem* Scene::GetParticleEffectDebugDrawSystem() const
{
    return particleEffectDebugDrawSystem;
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

SceneFileV2::eError Scene::LoadScene(const DAVA::FilePath& pathname)
{
    SceneFileV2::eError ret = SceneFileV2::ERROR_FAILED_TO_CREATE_FILE;

    RemoveAllChildren();
    SetName(pathname.GetFilename().c_str());

    if (pathname.IsEqualToExtension(".sc2"))
    {
        ScopedPtr<SceneFileV2> file(new SceneFileV2());
        file->EnableDebugLog(false);
        ret = file->LoadScene(pathname, this);
    }

    return ret;
}

SceneFileV2::eError Scene::SaveScene(const DAVA::FilePath& pathname, bool saveForGame /*= false*/)
{
    std::function<void(Entity*)> resolveId = [&](Entity* entity)
    {
        if (0 == entity->id)
            entity->id = ++maxEntityIDCounter;
        for (auto child : entity->children) resolveId(child);
    };

    resolveId(this);

    ScopedPtr<SceneFileV2> file(new SceneFileV2());
    file->EnableDebugLog(false);
    file->EnableSaveForGame(saveForGame);
    return file->SaveScene(pathname, this);
}

void Scene::OptimizeBeforeExport()
{
    List<NMaterial*> materials;
    GetDataNodes(materials);

    const auto RemoveMaterialFlag = [](NMaterial* material, const FastName& flagName) {
        if (material->HasLocalFlag(flagName))
        {
            material->RemoveFlag(flagName);
        }
    };

    for (auto& mat : materials)
    {
        RemoveMaterialFlag(mat, NMaterialFlagName::FLAG_ILLUMINATION_USED);
        RemoveMaterialFlag(mat, NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_CASTER);
        RemoveMaterialFlag(mat, NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_RECEIVER);

        if (mat->HasLocalProperty(NMaterialParamName::PARAM_LIGHTMAP_SIZE))
        {
            mat->RemoveProperty(NMaterialParamName::PARAM_LIGHTMAP_SIZE);
        }
    }

    Entity::OptimizeBeforeExport();
}

void Scene::OnSceneReady(Entity* rootNode)
{
}

void Scene::Input(DAVA::UIEvent* event)
{
    for (SceneSystem* system : systemsToInput)
    {
        system->Input(event);
    }
}

void Scene::InputCancelled(UIEvent* event)
{
    for (SceneSystem* system : systemsToInput)
    {
        system->InputCancelled(event);
    }
}

void Scene::HandleEvent(Observable* observable)
{
    RenderOptions* options = dynamic_cast<RenderOptions*>(observable);

    if (options->IsOptionEnabled(RenderOptions::REPLACE_LIGHTMAP_MIPMAPS))
        MipMapReplacer::ReplaceMipMaps(this, NMaterialTextureName::TEXTURE_LIGHTMAP);
    if (options->IsOptionEnabled(RenderOptions::REPLACE_ALBEDO_MIPMAPS))
        MipMapReplacer::ReplaceMipMaps(this, NMaterialTextureName::TEXTURE_ALBEDO);

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

    if (DAVA::Renderer::GetOptions()->IsOptionEnabled(RenderOptions::DEBUG_DRAW_PARTICLES) && particleEffectDebugDrawSystem == nullptr)
    {
        particleEffectDebugDrawSystem = new ParticleEffectDebugDrawSystem(this);
        AddSystem(particleEffectDebugDrawSystem, 0);
    }
    else if (!DAVA::Renderer::GetOptions()->IsOptionEnabled(RenderOptions::DEBUG_DRAW_PARTICLES) && particleEffectDebugDrawSystem != nullptr)
    {
        RemoveSystem(particleEffectDebugDrawSystem);
        SafeDelete(particleEffectDebugDrawSystem);
    }
}

void Scene::Activate()
{
    for (auto system : systems)
    {
        system->Activate();
    }
}

void Scene::Deactivate()
{
    for (auto system : systems)
    {
        system->Deactivate();
    }
}
};
