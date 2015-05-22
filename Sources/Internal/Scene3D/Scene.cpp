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
#include "Scene3D/ProxyNode.h"
#include "Scene3D/ShadowVolumeNode.h"
#include "Render/Highlevel/Light.h"
#include "Scene3D/MeshInstanceNode.h"
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
#include "Scene3D/SceneCache.h"
#include "UI/UIEvent.h"
#include "Render/Highlevel/RenderPass.h"

#include "Render/Renderer.h"


namespace DAVA 
{

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
	, materialSystem(0)
    , foliageSystem(0)
    , windSystem(0)
    , animationSystem(0)
    , staticOcclusionDebugDrawSystem(0)    
    , systemsMask(_systemsMask)
    , sceneGlobalMaterial(0)
    , mainCamera(0)
    , drawCamera(0)
{
	CreateComponents();
	CreateSystems();

    // this will force scene to create hidden global material
    SetGlobalMaterial(nullptr);
    
    SceneCache::Instance()->InsertScene(this);

    RenderOptions * options = Renderer::GetOptions();
    options->AddObserver(this);
}

void Scene::CreateComponents()
{ }

NMaterial* Scene::GetGlobalMaterial() const
{
    return sceneGlobalMaterial;
}

void Scene::SetGlobalMaterial(NMaterial *globalMaterial)
{
    SafeRelease(sceneGlobalMaterial);
    sceneGlobalMaterial = SafeRetain(globalMaterial);

    renderSystem->SetGlobalMaterial(sceneGlobalMaterial);
    particleEffectSystem->SetGlobalMaterial(sceneGlobalMaterial);
    
    ImportShadowColor(this);
}

rhi::RenderPassConfig& Scene::GetMainPassConfig()
{
    return renderSystem->GetMainRenderPass()->GetPassConfig();
}

void Scene::SetMainPassViewport(const Rect& viewport)
{
    renderSystem->GetMainRenderPass()->SetViewport(viewport);
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
    Renderer::GetOptions()->RemoveObserver(this);

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
    
    for (ProxyNodeMap::iterator it = rootNodes.begin(); it != rootNodes.end(); ++it)
    {
        SafeRelease(it->second);
    }
    rootNodes.clear();

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

	SafeDelete(eventSystem);
	SafeDelete(renderSystem);
}
    
void Scene::RegisterEntity(Entity * entity)
{
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
    
void Scene::AddSystem(SceneSystem * sceneSystem, uint64 componentFlags, uint32 processFlags /*= 0*/, SceneSystem * insertBeforeSceneForProcess /* = NULL */)
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
	
	return NULL;
}


void Scene::AddRootNode(Entity *node, const FilePath &rootNodePath)
{
    ProxyNode * proxyNode = new ProxyNode();
    proxyNode->SetNode(node);
    
	rootNodes[FILEPATH_MAP_KEY(rootNodePath)] = proxyNode;

	//proxyNode->SetName(rootNodePath.GetAbsolutePathname());
}

Entity *Scene::GetRootNode(const FilePath &rootNodePath)
{
	ProxyNodeMap::const_iterator it = rootNodes.find(FILEPATH_MAP_KEY(rootNodePath));
	if (it != rootNodes.end())
	{
        ProxyNode * node = it->second;
		return node->GetNode();
	}
    
    if(rootNodePath.IsEqualToExtension(".sce"))
    {
        SceneFile *file = new SceneFile();
        file->SetDebugLog(true);
        file->LoadScene(rootNodePath, this);
        SafeRelease(file);
    }
    else if(rootNodePath.IsEqualToExtension(".sc2"))
    {
        uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
        SceneFileV2 *file = new SceneFileV2();
        file->EnableDebugLog(false);
        SceneFileV2::eError loadResult = file->LoadScene(rootNodePath, this);
        SafeRelease(file);
				
        uint64 deltaTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
        Logger::FrameworkDebug("[GETROOTNODE TIME] %dms (%ld)", deltaTime, deltaTime);

        if (loadResult != SceneFileV2::ERROR_NO_ERROR)
        {
            return 0;
        }
    }
    
	it = rootNodes.find(FILEPATH_MAP_KEY(rootNodePath));
	if (it != rootNodes.end())
	{
        ProxyNode * node = it->second;
        //int32 nowCount = node->GetNode()->GetChildrenCountRecursive();
		return node->GetNode();
	}
    return 0;
}

void Scene::ReleaseRootNode(const FilePath &rootNodePath)
{
	ProxyNodeMap::iterator it = rootNodes.find(FILEPATH_MAP_KEY(rootNodePath));
	if (it != rootNodes.end())
	{
        it->second->Release();
        rootNodes.erase(it);
	}
}
    
void Scene::ReleaseRootNode(Entity *nodeToRelease)
{
//	for (Map<String, Entity*>::iterator it = rootNodes.begin(); it != rootNodes.end(); ++it)
//	{
//        if (nodeToRelease == it->second) 
//        {
//            Entity * obj = it->second;
//            obj->Release();
//            rootNodes.erase(it);
//            return;
//        }
//	}
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
            if(Renderer::GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_LODS))
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
// 	if(Renderer::GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_ANIMATED_MESHES))
// 	{
// 		size = (int32)animatedMeshes.size();
// 		for (int32 animatedMeshIndex = 0; animatedMeshIndex < size; ++animatedMeshIndex)
// 		{
// 			AnimatedMesh * mesh = animatedMeshes[animatedMeshIndex];
// 			mesh->Update(timeElapsed);
// 		}
// 	}	

    updateTime = SystemTimer::Instance()->AbsoluteMS() - time;
}

void Scene::Draw()
{
    TIME_PROFILE("Scene::Draw");

    //TODO: remove this crap with shadow color
    if (sceneGlobalMaterial && sceneGlobalMaterial->HasLocalProperty(NMaterialParamName::PARAM_SHADOW_COLOR))
    {
        const float32 * propDataPtr = sceneGlobalMaterial->GetLocalPropValue(NMaterialParamName::PARAM_SHADOW_COLOR);
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SHADOW_COLOR, propDataPtr, (pointer_size)sceneGlobalMaterial);
    }
    else
    {
        Color defShadowColor(1.f, 0.f, 0.f, 1.f);
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SHADOW_COLOR, defShadowColor.color, (pointer_size)this);
    }
    
    uint64 time = SystemTimer::Instance()->AbsoluteMS();        
    
    renderSystem->Render();
    
    //foliageSystem->DebugDrawVegetation();
    
	drawTime = SystemTimer::Instance()->AbsoluteMS() - time;
}
    
void Scene::SceneDidLoaded()
{
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
    

SceneFileV2::eError Scene::SaveScene(const DAVA::FilePath & pathname, bool saveForGame /*= false*/)
{
    ScopedPtr<SceneFileV2> file(new SceneFileV2());
	file->EnableDebugLog(false);
	file->EnableSaveForGame(saveForGame);
	return file->SaveScene(pathname, this);
}
    
void Scene::OptimizeBeforeExport()
{
#if RHI_COMPLETE
    Set<NMaterial*> materials;
    materialSystem->BuildMaterialList(materials);
    
    Set<NMaterial *>::const_iterator endIt = materials.end();
    for(Set<NMaterial *>::const_iterator it = materials.begin(); it != endIt; ++it)
        (*it)->ReleaseIlluminationParams();
#endif  // RHI_COMPLETE

    ImportShadowColor(this);

    Entity::OptimizeBeforeExport();
}

void Scene::ImportShadowColor(Entity * rootNode)
{
    if(NULL != sceneGlobalMaterial)
    {
		Entity * landscapeNode = FindLandscapeEntity(rootNode);
		if(NULL != landscapeNode)
		{
			// try to get shadow color for landscape
			KeyedArchive * props = GetCustomPropertiesArchieve(landscapeNode);
			if (props->IsKeyExists("ShadowColor"))
			{
                if (sceneGlobalMaterial->HasLocalProperty(NMaterialParamName::PARAM_SHADOW_COLOR))
                    sceneGlobalMaterial->RemoveProperty(NMaterialParamName::PARAM_SHADOW_COLOR);

				Color shadowColor = props->GetVariant("ShadowColor")->AsColor();
                sceneGlobalMaterial->AddProperty(NMaterialParamName::PARAM_SHADOW_COLOR, shadowColor.color, rhi::ShaderProp::TYPE_FLOAT4);					
				props->DeleteKey("ShadowColor");
			}
		}
    }
}

void Scene::OnSceneReady(Entity * rootNode)
{
    ImportShadowColor(rootNode);
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
#if RHI_COMPLETE
    if (options->IsOptionEnabled(RenderOptions::REPLACE_LIGHTMAP_MIPMAPS))
        MipMapReplacer::ReplaceMipMaps(this, NMaterial::TEXTURE_LIGHTMAP);
    if (options->IsOptionEnabled(RenderOptions::REPLACE_ALBEDO_MIPMAPS))
        MipMapReplacer::ReplaceMipMaps(this, NMaterial::TEXTURE_ALBEDO);
#endif // RHI_COMPLETE

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
