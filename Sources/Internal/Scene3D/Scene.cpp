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


#include "Platform/SystemTimer.h"
#include "FileSystem/FileSystem.h"
#include "Debug/Stats.h"

#include "Scene3D/SceneNodeAnimationList.h"
#include "Scene3D/SceneFile.h"
#include "Scene3D/SceneFileV2.h"
#include "Scene3D/DataNode.h"
#include "Scene3D/ProxyNode.h"
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

#include "Sound/SoundSystem.h"

#include "Scene3D/Systems/SpeedTreeUpdateSystem.h"

#include "Scene3D/Systems/StaticOcclusionSystem.h"
#include "Scene3D/Systems/FoliageSystem.h"

#include "Scene3D/Systems/MaterialSystem.h"

#include "Scene3D/Components/ComponentHelpers.h"

//#include "Entity/Entity.h"
//#include "Entity/EntityManager.h"
//#include "Entity/Components.h"
//
//#include "Entity/VisibilityAABBoxSystem.h"
//#include "Entity/MeshInstanceDrawSystem.h"
//#include "Entity/LandscapeGeometrySystem.h"

namespace DAVA 
{

Texture* Scene::stubTexture2d = NULL;
Texture* Scene::stubTextureCube = NULL;
Texture* Scene::stubTexture2dLightmap = NULL; //this texture should be all-pink without checkers
    
    
Scene::Scene(uint32 _systemsMask /* = SCENE_SYSTEM_ALL_MASK */)
	: Entity()
    , mainCamera(0)
    , drawCamera(0)
	, imposterManager(0)
    , systemsMask(_systemsMask)
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
	, sceneGlobalMaterial(0)
    , isDefaultGlobalMaterial(true)
    , clearBuffers(RenderManager::ALL_BUFFERS)
{   
	CreateComponents();
	CreateSystems();

    // this will force scene to create hidden global material
    SetGlobalMaterial(NULL);
}

void Scene::CreateComponents()
{ }

NMaterial* Scene::GetGlobalMaterial() const
{
    NMaterial *ret = NULL;

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

    if(NULL != globalMaterial)
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
    particleEffectSystem->SetGlobalMaterial(sceneGlobalMaterial);
    
    ImportShadowColor(this);
}

void Scene::InitGlobalMaterial()
{
    if(NULL == stubTexture2d)
    {
        stubTexture2d = Texture::CreatePink(Texture::TEXTURE_2D);
    }

    if(NULL == stubTextureCube)
    {
        stubTextureCube = Texture::CreatePink(Texture::TEXTURE_CUBE);
    }

    if(NULL == stubTexture2dLightmap)
    {
        stubTexture2dLightmap = Texture::CreatePink(Texture::TEXTURE_2D, false);
    }

    Vector3 defaultVec3;
    Color defaultColor(1.0f, 0.0f, 0.0f, 1.0f);
    float32 defaultFloat05 = 0.5f;
    float32 defaultFloat10 = 1.0f;
    Vector2 defaultVec2;
    float32 defaultLightmapSize = 16.0f;
    float32 defaultFogStart = 0.0f;
    float32 defaultFogEnd = 500.0f;

    if(NULL == sceneGlobalMaterial->GetTexture(NMaterial::TEXTURE_ALBEDO)) sceneGlobalMaterial->SetTexture(NMaterial::TEXTURE_ALBEDO, stubTexture2d);
    if(NULL == sceneGlobalMaterial->GetTexture(NMaterial::TEXTURE_NORMAL)) sceneGlobalMaterial->SetTexture(NMaterial::TEXTURE_NORMAL, stubTexture2d);
    if(NULL == sceneGlobalMaterial->GetTexture(NMaterial::TEXTURE_DETAIL)) sceneGlobalMaterial->SetTexture(NMaterial::TEXTURE_DETAIL, stubTexture2d);
    if(NULL == sceneGlobalMaterial->GetTexture(NMaterial::TEXTURE_LIGHTMAP)) sceneGlobalMaterial->SetTexture(NMaterial::TEXTURE_LIGHTMAP, stubTexture2dLightmap);
    if(NULL == sceneGlobalMaterial->GetTexture(NMaterial::TEXTURE_DECAL)) sceneGlobalMaterial->SetTexture(NMaterial::TEXTURE_DECAL, stubTexture2d);
    if(NULL == sceneGlobalMaterial->GetTexture(NMaterial::TEXTURE_CUBEMAP)) sceneGlobalMaterial->SetTexture(NMaterial::TEXTURE_CUBEMAP, stubTextureCube);

    if(NULL == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_LIGHT_POSITION0)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_LIGHT_POSITION0, Shader::UT_FLOAT_VEC3, 1, defaultVec3.data);
    if(NULL == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_PROP_AMBIENT_COLOR)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_PROP_AMBIENT_COLOR, Shader::UT_FLOAT_VEC4, 1, &defaultColor);
    if(NULL == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_PROP_DIFFUSE_COLOR)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_PROP_DIFFUSE_COLOR, Shader::UT_FLOAT_VEC4, 1, &defaultColor);
    if(NULL == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_PROP_SPECULAR_COLOR)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_PROP_SPECULAR_COLOR, Shader::UT_FLOAT_VEC4, 1, &defaultColor);
    if(NULL == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_LIGHT_AMBIENT_COLOR)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_LIGHT_AMBIENT_COLOR, Shader::UT_FLOAT_VEC3, 1, &defaultColor);
    if(NULL == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_LIGHT_DIFFUSE_COLOR)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_LIGHT_DIFFUSE_COLOR, Shader::UT_FLOAT_VEC3, 1, &defaultColor);
    if(NULL == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_LIGHT_SPECULAR_COLOR)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_LIGHT_SPECULAR_COLOR, Shader::UT_FLOAT_VEC3, 1, &defaultColor);
    if(NULL == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_LIGHT_INTENSITY0)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_LIGHT_INTENSITY0, Shader::UT_FLOAT, 1, &defaultFloat05);
    if(NULL == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_MATERIAL_SPECULAR_SHININESS)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_MATERIAL_SPECULAR_SHININESS, Shader::UT_FLOAT, 1, &defaultFloat05);
    if(NULL == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_FOG_LIMIT)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_FOG_LIMIT, Shader::UT_FLOAT, 1, &defaultFloat10);
    if(NULL == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_FOG_COLOR)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_FOG_COLOR, Shader::UT_FLOAT_VEC4, 1, &defaultColor);
    if(NULL == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_FOG_DENSITY)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_FOG_DENSITY, Shader::UT_FLOAT, 1, &defaultFloat05);
    if(NULL == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_FOG_START)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_FOG_START, Shader::UT_FLOAT, 1, &defaultFogStart);
    if(NULL == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_FOG_END)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_FOG_END, Shader::UT_FLOAT, 1, &defaultFogEnd);
    if(NULL == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_FLAT_COLOR)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_FLAT_COLOR, Shader::UT_FLOAT_VEC4, 1, &defaultColor);
    if(NULL == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_TEXTURE0_SHIFT)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_TEXTURE0_SHIFT, Shader::UT_FLOAT_VEC2, 1, defaultVec2.data);
    if(NULL == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_UV_OFFSET)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_UV_OFFSET, Shader::UT_FLOAT_VEC2, 1, defaultVec2.data);
    if(NULL == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_UV_SCALE)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_UV_SCALE, Shader::UT_FLOAT_VEC2, 1, defaultVec2.data);
    if(NULL == sceneGlobalMaterial->GetPropertyValue(NMaterial::PARAM_LIGHTMAP_SIZE)) sceneGlobalMaterial->SetPropertyValue(NMaterial::PARAM_LIGHTMAP_SIZE, Shader::UT_FLOAT, 1, &defaultLightmapSize);
}

void Scene::CreateSystems()
{
	renderSystem = new RenderSystem();
    eventSystem = new EventSystem();

    if(SCENE_SYSTEM_STATIC_OCCLUSION_FLAG & systemsMask)
    {
        staticOcclusionSystem = new StaticOcclusionSystem(this);
        AddSystem(staticOcclusionSystem, (1 << Component::STATIC_OCCLUSION_DATA_COMPONENT), true);
    }

    if(SCENE_SYSTEM_TRANSFORM_FLAG & systemsMask)
    {
        transformSystem = new TransformSystem(this);
        AddSystem(transformSystem, (1 << Component::TRANSFORM_COMPONENT), true);
    }

    if(SCENE_SYSTEM_LOD_FLAG & systemsMask)
    {
        lodSystem = new LodSystem(this);
        AddSystem(lodSystem, (1 << Component::LOD_COMPONENT), true);
    }

    if(SCENE_SYSTEM_SWITCH_FLAG & systemsMask)
    {
        switchSystem = new SwitchSystem(this);
        AddSystem(switchSystem, (1 << Component::SWITCH_COMPONENT), true);
    }

    if(SCENE_SYSTEM_PARTICLE_EFFECT_FLAG & systemsMask)
    {
        particleEffectSystem = new ParticleEffectSystem(this);
        AddSystem(particleEffectSystem, (1 << Component::PARTICLE_EFFECT_COMPONENT), true);
    }

    if(SCENE_SYSTEM_SOUND_UPDATE_FLAG & systemsMask)
    {
        soundSystem = new SoundUpdateSystem(this);
        AddSystem(soundSystem, (1 << Component::TRANSFORM_COMPONENT) | (1 << Component::SOUND_COMPONENT), true);
    }

    if(SCENE_SYSTEM_RENDER_UPDATE_FLAG & systemsMask)
    {
        renderUpdateSystem = new RenderUpdateSystem(this);
        AddSystem(renderUpdateSystem, (1 << Component::TRANSFORM_COMPONENT) | (1 << Component::RENDER_COMPONENT), true);
    }

    if(SCENE_SYSTEM_UPDATEBLE_FLAG & systemsMask)
    {
        updatableSystem = new UpdateSystem(this);
        AddSystem(updatableSystem, (1 << Component::UPDATABLE_COMPONENT));
    }

    if(SCENE_SYSTEM_LIGHT_UPDATE_FLAG & systemsMask)
    {
        lightUpdateSystem = new LightUpdateSystem(this);
        AddSystem(lightUpdateSystem, (1 << Component::TRANSFORM_COMPONENT) | (1 << Component::LIGHT_COMPONENT));
    }

    if(SCENE_SYSTEM_ACTION_UPDATE_FLAG & systemsMask)
    {
        actionSystem = new ActionUpdateSystem(this);
        AddSystem(actionSystem, (1 << Component::ACTION_COMPONENT), true);
    }

    if(SCENE_SYSTEM_SKYBOX_FLAG & systemsMask)
    {
        skyboxSystem = new SkyboxSystem(this);
        AddSystem(skyboxSystem, (1 << Component::RENDER_COMPONENT), true);
    }

    if(SCENE_SYSTEM_MATERIAL_FLAG & systemsMask)
    {
        materialSystem = new MaterialSystem(this);
        AddSystem(materialSystem, (1 << Component::RENDER_COMPONENT));
    }

    if(SCENE_SYSTEM_DEBUG_RENDER_FLAG & systemsMask)
    {
        debugRenderSystem = new DebugRenderSystem(this);
        AddSystem(debugRenderSystem, (1 << Component::DEBUG_RENDER_COMPONENT), true);
    }
    
    if(SCENE_SYSTEM_FOLIAGE_FLAG & systemsMask)
    {
        foliageSystem = new FoliageSystem(this);
        AddSystem(foliageSystem, (1 << Component::RENDER_COMPONENT), true);
    }

    if(SCENE_SYSTEM_SPEEDTREE_UPDATE_FLAG & systemsMask)
    {
        speedTreeUpdateSystem = new SpeedTreeUpdateSystem(this);
        AddSystem(speedTreeUpdateSystem, (1 << Component::SPEEDTREE_COMPONENT), true);
    }

    if(SCENE_SYSTEM_WIND_UPDATE_FLAG & systemsMask)
    {
        windSystem = new WindSystem(this);
        AddSystem(windSystem, (1 << Component::WIND_COMPONENT), true);
    }

    if(SCENE_SYSTEM_WAVE_UPDATE_FLAG & systemsMask)
    {
        waveSystem = new WaveSystem(this);
        AddSystem(waveSystem, (1 << Component::WAVE_COMPONENT), true);
    }
}

Scene::~Scene()
{
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
    
    uint32 size = (uint32)systems.size();
    for (uint32 k = 0; k < size; ++k)
        SafeDelete(systems[k]);
    systems.clear();

    systemsToProcess.clear();

	SafeDelete(eventSystem);
	SafeDelete(renderSystem);
}
    
void Scene::RegisterEntity(Entity * entity)
{
    uint32 systemsCount = systems.size();
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        systems[k]->RegisterEntity(entity);
    }
}

void Scene::UnregisterEntity(Entity * entity)
{
    uint32 systemsCount = systems.size();
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        systems[k]->UnregisterEntity(entity);
    }
}

void Scene::RegisterComponent(Entity * entity, Component * component)
{
    DVASSERT(entity && component);
    uint32 systemsCount = systems.size();
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        systems[k]->RegisterComponent(entity, component);
    }
}

void Scene::UnregisterComponent(Entity * entity, Component * component)
{
    DVASSERT(entity && component);
    uint32 systemsCount = systems.size();
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
    uint32 updatedComponentFlag = 1 << componentType;
    uint32 componentsInEntity = entity->GetAvailableComponentFlags();

    for (uint32 k = 0; k < systemsCount; ++k)
    {
        uint32 requiredComponentFlags = systems[k]->GetRequiredComponents();
        
        if (((requiredComponentFlags & updatedComponentFlag) != 0) && ((requiredComponentFlags & componentsInEntity) == requiredComponentFlags))
        {
			eventSystem->NotifySystem(systems[k], entity, event);
        }
    }
#else
    uint32 componentsInEntity = entity->GetAvailableComponentFlags();
    Set<SceneSystem*> & systemSetForType = componentTypeMapping.GetValue(componentsInEntity);
    
    for (Set<SceneSystem*>::iterator it = systemSetForType.begin(); it != systemSetForType.end(); ++it)
    {
        SceneSystem * system = *it;
        uint32 requiredComponentFlags = system->GetRequiredComponents();
        if ((requiredComponentFlags & componentsInEntity) == requiredComponentFlags)
            eventSystem->NotifySystem(system, entity, event);
    }
#endif
}
#endif
    
void Scene::AddSystem(SceneSystem * sceneSystem, uint32 componentFlags, bool needProcess /* = false */, SceneSystem * insertBeforeSceneForProcess /* = NULL */)
{
    sceneSystem->SetRequiredComponents(componentFlags);
    //Set<SceneSystem*> & systemSetForType = componentTypeMapping.GetValue(componentFlags);
    //systemSetForType.insert(sceneSystem);
    systems.push_back(sceneSystem);

    bool wasInsertedForUpdate = false;
    if(needProcess)
    {
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
    }
    DVASSERT(needProcess == wasInsertedForUpdate);
}
    
void Scene::RemoveSystem(SceneSystem * sceneSystem)
{
    Vector<SceneSystem*>::iterator endIt = systemsToProcess.end();
    for(Vector<SceneSystem*>::iterator it = systemsToProcess.begin(); it != endIt; ++it)
    {
        if(*it == sceneSystem)
        {
            systemsToProcess.erase(it);
            break;;
        }
    }

    endIt = systems.end();
    for(Vector<SceneSystem*>::iterator it = systems.begin(); it != endIt; ++it)
    {
        if(*it == sceneSystem)
        {
            systems.erase(it);
            return;
        }
    }

    DVASSERT_MSG(false, "System must be at systems array");
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
	
void Scene::AddAnimation(SceneNodeAnimationList * animation)
{
	if (animation)
	{
		animation->Retain();
		animations.push_back(animation);
	}
}

SceneNodeAnimationList * Scene::GetAnimation(int32 index)
{
	return animations[index];
}
	
SceneNodeAnimationList * Scene::GetAnimation(const FastName & name)
{
	int32 size = (int32)animations.size();
	for (int32 k = 0; k < size; ++k)
	{
		SceneNodeAnimationList * node = animations[k];
		if (node->GetName() == name)
			return node;
	}
	return 0;
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
        file->LoadScene(rootNodePath, this);
        SafeRelease(file);
				
        uint64 deltaTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
        Logger::FrameworkDebug("[GETROOTNODE TIME] %dms (%ld)", deltaTime, deltaTime);
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
    
    if(NULL != sceneGlobalMaterial)
    {
        NMaterialProperty* propShadowColor = sceneGlobalMaterial->GetMaterialProperty(NMaterial::PARAM_SHADOW_COLOR);
        if(NULL != propShadowColor)
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
    uint32 systemsCount = systems.size();
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


/*void Scene::Save(KeyedArchive * archive)
{
    // Perform refactoring and add Matrix4, Vector4 types to VariantType and KeyedArchive
    Entity::Save(archive);
    
    
    
    
    
}

void Scene::Load(KeyedArchive * archive)
{
    Entity::Load(archive);
}*/
    

SceneFileV2::eError Scene::Save(const DAVA::FilePath & pathname, bool saveForGame /*= false*/)
{
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
    if(NULL != sceneGlobalMaterial)
    {
        NMaterialProperty* propShadowColor = sceneGlobalMaterial->GetMaterialProperty(NMaterial::PARAM_SHADOW_COLOR);
        if(NULL == propShadowColor)
        {
            Entity * landscapeNode = FindLandscapeEntity(rootNode);
            
            if(NULL != landscapeNode)
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

};
