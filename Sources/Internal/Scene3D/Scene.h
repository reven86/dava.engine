#pragma once

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"
#include "Scene3D/Entity.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Light.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Scene3D/SceneFileV2.h"
#include "Scene3D/SceneFile/VersionInfo.h"
#include "Base/Observer.h"

namespace DAVA
{
/**
    \defgroup scene3d 3D Engine
  */

class Texture;
class StaticMesh;
class DataNode;
class ShadowVolumeNode;
class Light;
class ShadowRect;
class QuadTree;
class Component;
class SceneSystem;
class RenderSystem;
class RenderUpdateSystem;
class TransformSystem;
class DebugRenderSystem;
class EventSystem;
class ParticleEffectSystem;
class UpdateSystem;
class LightUpdateSystem;
class SwitchSystem;
class SoundUpdateSystem;
class ActionUpdateSystem;
class StaticOcclusionSystem;
class StaticOcclusionDebugDrawSystem;
class SpeedTreeUpdateSystem;
class FoliageSystem;
class WindSystem;
class WaveSystem;
class SkeletonSystem;
class AnimationSystem;
class LandscapeSystem;
class LodSystem;
class ParticleEffectDebugDrawSystem;

class UIEvent;

/**
    \ingroup scene3d
    \brief This class is a code of our 3D Engine scene graph. 
    To visualize any 3d scene you'll need to create Scene object. 
    Scene have visible hierarchy and invisible root nodes. You can add as many root nodes as you want, and do not visualize them.
    For example you can have multiple scenes, load them to one scene, and show each scene when it will be required. 
 */

class EntityCache
{
public:
    ~EntityCache();

    void Preload(const FilePath& path);
    void Clear(const FilePath& path);
    void ClearAll();

    Entity* GetOriginal(const FilePath& path);
    Entity* GetClone(const FilePath& path);

protected:
    Map<FilePath, Entity*> cachedEntities;
};

class RenderPass;

class Scene : public Entity, Observer
{
protected:
    virtual ~Scene();

public:
    enum
    {
        SCENE_SYSTEM_TRANSFORM_FLAG = 1 << 0,
        SCENE_SYSTEM_RENDER_UPDATE_FLAG = 1 << 1,
        SCENE_SYSTEM_LOD_FLAG = 1 << 2,
        SCENE_SYSTEM_DEBUG_RENDER_FLAG = 1 << 3,
        SCENE_SYSTEM_PARTICLE_EFFECT_FLAG = 1 << 4,
        SCENE_SYSTEM_UPDATEBLE_FLAG = 1 << 5,
        SCENE_SYSTEM_LIGHT_UPDATE_FLAG = 1 << 6,
        SCENE_SYSTEM_SWITCH_FLAG = 1 << 7,
        SCENE_SYSTEM_SOUND_UPDATE_FLAG = 1 << 8,
        SCENE_SYSTEM_ACTION_UPDATE_FLAG = 1 << 9,

        SCENE_SYSTEM_STATIC_OCCLUSION_FLAG = 1 << 11,
        SCENE_SYSTEM_LANDSCAPE_FLAG = 1 << 12,
        SCENE_SYSTEM_FOLIAGE_FLAG = 1 << 13,
        SCENE_SYSTEM_SPEEDTREE_UPDATE_FLAG = 1 << 14,
        SCENE_SYSTEM_WIND_UPDATE_FLAG = 1 << 15,
        SCENE_SYSTEM_WAVE_UPDATE_FLAG = 1 << 16,
        SCENE_SYSTEM_SKELETON_UPDATE_FLAG = 1 << 17,
        SCENE_SYSTEM_ANIMATION_FLAG = 1 << 18,

        SCENE_SYSTEM_ALL_MASK = 0xFFFFFFFF
    };

    enum eSceneProcessFlags
    {
        SCENE_SYSTEM_REQUIRE_PROCESS = 1 << 0,
        SCENE_SYSTEM_REQUIRE_INPUT = 1 << 1
    };

    Scene(uint32 systemsMask = SCENE_SYSTEM_ALL_MASK);

    /**
        \brief Function to register entity in scene. This function is called when you add entity to scene.
     */
    void RegisterEntity(Entity* entity);
    /**
        \brief Function to unregister entity from scene. This function is called when you remove entity from scene.
     */
    void UnregisterEntity(Entity* entity);

    /**
        \brief Function to register component in scene. This function is called when you add any component to any entity in scene.
     */
    void RegisterComponent(Entity* entity, Component* component);
    /**
        \brief Function to unregister component from scene. This function is called when you remove any component from any entity in scene.
     */
    void UnregisterComponent(Entity* entity, Component* component);

    virtual void AddSystem(SceneSystem* sceneSystem, uint64 componentFlags, uint32 processFlags = 0, SceneSystem* insertBeforeSceneForProcess = nullptr, SceneSystem* insertBeforeSceneForInput = nullptr);
    virtual void RemoveSystem(SceneSystem* sceneSystem);

    //virtual void ImmediateEvent(Entity * entity, uint32 componentType, uint32 event);

    Vector<SceneSystem*> systems;
    Vector<SceneSystem*> systemsToProcess;
    Vector<SceneSystem*> systemsToInput;
    //HashMap<uint32, Set<SceneSystem*> > componentTypeMapping;
    TransformSystem* transformSystem;
    RenderUpdateSystem* renderUpdateSystem;
    LodSystem* lodSystem;
    DebugRenderSystem* debugRenderSystem;
    EventSystem* eventSystem;
    ParticleEffectSystem* particleEffectSystem;
    UpdateSystem* updatableSystem;
    LightUpdateSystem* lightUpdateSystem;
    SwitchSystem* switchSystem;
    RenderSystem* renderSystem;
    SoundUpdateSystem* soundSystem;
    ActionUpdateSystem* actionSystem;
    StaticOcclusionSystem* staticOcclusionSystem;
    SpeedTreeUpdateSystem* speedTreeUpdateSystem;
    FoliageSystem* foliageSystem;
    VersionInfo::SceneVersion version;
    WindSystem* windSystem;
    WaveSystem* waveSystem;
    AnimationSystem* animationSystem;
    StaticOcclusionDebugDrawSystem* staticOcclusionDebugDrawSystem;
    SkeletonSystem* skeletonSystem;
    LandscapeSystem* landscapeSystem;
    ParticleEffectDebugDrawSystem* particleEffectDebugDrawSystem;

    /**
        \brief Overloaded GetScene returns this, instead of normal functionality.
     */
    Scene* GetScene() override;

    void HandleEvent(Observable* observable) override; //Handle RenderOptions

    //virtual void StopAllAnimations(bool recursive = true);

    virtual void Update(float32 timeElapsed);
    void Draw() override;
    void SceneDidLoaded() override;

    virtual void SetupTestLighting();

    Camera* GetCamera(int32 n);
    void AddCamera(Camera* c);
    bool RemoveCamera(Camera* c);
    inline int32 GetCameraCount();

    void SetCurrentCamera(Camera* camera);
    Camera* GetCurrentCamera() const;

    /* 
        This camera is used for visualization setup only. Most system functions use mainCamere, draw camera is used to setup matrices for render. If you do not call this function GetDrawCamera returns currentCamera. 
        You can use SetCustomDrawCamera function if you want to test frustum clipping, and view the scene from different angles.
     */
    void SetCustomDrawCamera(Camera* camera);
    Camera* GetDrawCamera() const;

    Set<Light*>& GetLights();
    Light* GetNearestDynamicLight(Light::eType type, Vector3 position);

    void CreateComponents();
    void CreateSystems();

    EventSystem* GetEventSystem() const;
    RenderSystem* GetRenderSystem() const;
    AnimationSystem* GetAnimationSystem() const;
    ParticleEffectDebugDrawSystem* GetParticleEffectDebugDrawSystem() const;

    virtual SceneFileV2::eError LoadScene(const DAVA::FilePath& pathname);
    virtual SceneFileV2::eError SaveScene(const DAVA::FilePath& pathname, bool saveForGame = false);

    virtual void OptimizeBeforeExport();

    DAVA::NMaterial* GetGlobalMaterial() const;
    void SetGlobalMaterial(DAVA::NMaterial* globalMaterial);

    void OnSceneReady(Entity* rootNode);

    void Input(UIEvent* event);
    void InputCancelled(UIEvent* event);

    /**
        \brief This functions activate and deactivate scene systems
     */
    virtual void Activate();
    virtual void Deactivate();

    EntityCache cache;

    void SetMainPassProperties(uint32 priority, const Rect& viewport, uint32 width, uint32 height, PixelFormat format);
    void SetMainRenderTarget(rhi::HTexture color, rhi::HTexture depthStencil, rhi::LoadAction colorLoadAction, const Color& clearColor);

public: // deprecated methods
    DAVA_DEPRECATED(rhi::RenderPassConfig& GetMainPassConfig());

protected:
    void UpdateLights();

    void RegisterEntitiesInSystemRecursively(SceneSystem* system, Entity* entity);
    void UnregisterEntitiesInSystemRecursively(SceneSystem* system, Entity* entity);

    bool RemoveSystem(Vector<SceneSystem*>& storage, SceneSystem* system);

    uint64 updateTime;

    uint64 drawTime;
    uint32 nodeCounter;

    uint32 systemsMask;
    uint32 maxEntityIDCounter;

    float32 sceneGlobalTime = 0.f;

    Vector<Camera*> cameras;

    NMaterial* sceneGlobalMaterial;

    Camera* mainCamera;
    Camera* drawCamera;

    Set<Light*> lights;

    friend class Entity;
};

int32 Scene::GetCameraCount()
{
    return static_cast<int32>(cameras.size());
}
};
