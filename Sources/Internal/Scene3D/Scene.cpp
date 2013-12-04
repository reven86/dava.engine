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
#include "Render/Image.h"
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
#include "Scene3D/Systems/ParticleEmitterSystem.h"
#include "Scene3D/Systems/ParticleEffectSystem.h"
#include "Scene3D/Systems/UpdateSystem.h"
#include "Scene3D/Systems/LightUpdateSystem.h"
#include "Scene3D/Systems/SwitchSystem.h"
#include "Scene3D/Systems/SoundUpdateSystem.h"
#include "Scene3D/Systems/ActionUpdateSystem.h"
#include "Scene3D/Systems/SkyboxSystem.h"

//#include "Entity/Entity.h"
//#include "Entity/EntityManager.h"
//#include "Entity/Components.h"
//
//#include "Entity/VisibilityAABBoxSystem.h"
//#include "Entity/MeshInstanceDrawSystem.h"
//#include "Entity/LandscapeGeometrySystem.h"

namespace DAVA 
{
    
    
Scene::Scene()
	:   Entity()
    ,   currentCamera(0)
    ,   clipCamera(0)
//    ,   forceLodLayer(-1)
	,	imposterManager(0)
	,	entityManager(0)
	,	referenceNodeSuffixChanged(false)
{   

//	entityManager = new EntityManager();

	CreateComponents();
	CreateSystems();
}

void Scene::CreateComponents()
{
    
}

void Scene::CreateSystems()
{
	renderSystem = new RenderSystem();
	eventSystem = new EventSystem();

    transformSystem = new TransformSystem(this);
    AddSystem(transformSystem, (1 << Component::TRANSFORM_COMPONENT));

    renderUpdateSystem = new RenderUpdateSystem(this);
    AddSystem(renderUpdateSystem, (1 << Component::TRANSFORM_COMPONENT) | (1 << Component::RENDER_COMPONENT));

	lodSystem = new LodSystem(this);
	AddSystem(lodSystem, (1 << Component::LOD_COMPONENT));

    debugRenderSystem = new DebugRenderSystem(this);
    AddSystem(debugRenderSystem, (1 << Component::DEBUG_RENDER_COMPONENT));

	particleEffectSystem = new ParticleEffectSystem(this);
	AddSystem(particleEffectSystem, (1 << Component::PARTICLE_EFFECT_COMPONENT));

	updatableSystem = new UpdateSystem(this);
	AddSystem(updatableSystem, (1 << Component::UPDATABLE_COMPONENT));
    
    lightUpdateSystem = new LightUpdateSystem(this);
    AddSystem(lightUpdateSystem, (1 << Component::TRANSFORM_COMPONENT) | (1 << Component::LIGHT_COMPONENT));

	switchSystem = new SwitchSystem(this);
	AddSystem(switchSystem, (1 << Component::SWITCH_COMPONENT));

	soundSystem = new SoundUpdateSystem(this);
	AddSystem(soundSystem, (1 << Component::TRANSFORM_COMPONENT) | (1 << Component::SOUND_COMPONENT));
	
	actionSystem = new ActionUpdateSystem(this);
	AddSystem(actionSystem, (1 << Component::ACTION_COMPONENT));
	
	skyboxSystem = new SkyboxSystem(this);
	AddSystem(skyboxSystem, (1 << Component::RENDER_COMPONENT));
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
    
    SafeRelease(currentCamera);
    SafeRelease(clipCamera);
    
    for (Map<String, ProxyNode*>::iterator it = rootNodes.begin(); it != rootNodes.end(); ++it)
    {
        SafeRelease(it->second);
    }
    rootNodes.clear();

    // Children should be removed first because they should unregister themselves in managers
	RemoveAllChildren();
    
	SafeRelease(imposterManager);

    transformSystem = 0;
    renderUpdateSystem = 0;
	lodSystem = 0;
    uint32 size = (uint32)systems.size();
    for (uint32 k = 0; k < size; ++k)
        SafeDelete(systems[k]);
    systems.clear();

	SafeDelete(eventSystem);
	SafeDelete(renderSystem);
}

void Scene::RegisterNode(Entity * node)
{
    Light * light = dynamic_cast<Light*>(node);
    if (light)
    {
        lights.insert(light);
    }

	ImposterNode * imposter = dynamic_cast<ImposterNode*>(node);
	if(imposter)
	{
		RegisterImposter(imposter);
	}
    
    uint32 systemsCount = systems.size();
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        uint32 requiredComponents = systems[k]->GetRequiredComponents();
        bool needAdd = ((requiredComponents & node->componentFlags) == requiredComponents);
        
        if (needAdd)
            systems[k]->AddEntity(node);
    }
}

void Scene::UnregisterNode(Entity * node)
{
    uint32 systemsCount = systems.size();
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        uint32 requiredComponents = systems[k]->GetRequiredComponents();
        bool needRemove = ((requiredComponents & node->componentFlags) == requiredComponents);
        
        if (needRemove)
            systems[k]->RemoveEntity(node);
    }

    Light * light = dynamic_cast<Light*>(node);
    if (light)
        lights.erase(light);

	ImposterNode * imposter = dynamic_cast<ImposterNode*>(node);
	if(imposter)
	{
		UnregisterImposter(imposter);
	}
}
    
void Scene::AddComponent(Entity * entity, Component * component)
{
    uint32 oldComponentFlags = entity->componentFlags;
    entity->componentFlags |= (1 << component->GetType());
    uint32 systemsCount = systems.size();
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        uint32 requiredComponents = systems[k]->GetRequiredComponents();
        bool wasBefore = ((requiredComponents & oldComponentFlags) == requiredComponents);
        bool needAdd = ((requiredComponents & entity->componentFlags) == requiredComponents);
        
        if ((!wasBefore) && (needAdd))
            systems[k]->AddEntity(entity);
    }
}
    
void Scene::RemoveComponent(Entity * entity, Component * component)
{
    uint32 oldComponentFlags = entity->componentFlags;
    entity->componentFlags &= ~(1 << component->GetType());
    
    uint32 systemsCount = systems.size();
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        uint32 requiredComponents = systems[k]->GetRequiredComponents();
        bool wasBefore = ((requiredComponents & oldComponentFlags) == requiredComponents);
        bool shouldBeNow = ((requiredComponents & entity->componentFlags) == requiredComponents);
        
        if ((wasBefore) && (!shouldBeNow))
            systems[k]->RemoveEntity(entity);
    }
}
    
void Scene::ImmediateEvent(Entity * entity, uint32 componentType, uint32 event)
{
    uint32 systemsCount = systems.size();
    uint32 updatedComponentFlag = 1 << componentType;
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        uint32 requiredComponentFlags = systems[k]->GetRequiredComponents();
        uint32 componentsInEntity = entity->GetAvailableComponentFlags();
        
        if (((requiredComponentFlags & updatedComponentFlag) != 0) && ((requiredComponentFlags & componentsInEntity) == requiredComponentFlags))
        {
			eventSystem->NotifySystem(systems[k], entity, event);
        }
    }
}
    
void Scene::AddSystem(SceneSystem * sceneSystem, uint32 componentFlags)
{
    sceneSystem->SetRequiredComponents(componentFlags);
    systems.push_back(sceneSystem);
}
    
void Scene::RemoveSystem(SceneSystem * sceneSystem)
{
    Vector<SceneSystem*>::const_iterator endIt = systems.end();
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
	
SceneNodeAnimationList * Scene::GetAnimation(const String & name)
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
    
    rootNodes[rootNodePath.GetAbsolutePathname()] = proxyNode;
    proxyNode->SetName(rootNodePath.GetAbsolutePathname());
}

Entity *Scene::GetRootNode(const FilePath &rootNodePath)
{
	Map<String, ProxyNode*>::const_iterator it;
	it = rootNodes.find(rootNodePath.GetAbsolutePathname());
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
    
	it = rootNodes.find(rootNodePath.GetAbsolutePathname());
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
	Map<String, ProxyNode*>::iterator it;
	it = rootNodes.find(rootNodePath.GetAbsolutePathname());
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
    
    
	updatableSystem->UpdatePreTransform();

    transformSystem->Process();

	updatableSystem->UpdatePostTransform();

	if(RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_LODS))
	{
		lodSystem->SetCamera(currentCamera);
		lodSystem->Process();
	}
	

	switchSystem->Process();
    
//	entityManager->Flush();
	int32 size;
	
	size = (int32)animations.size();
	for (int32 animationIndex = 0; animationIndex < size; ++animationIndex)
	{
		SceneNodeAnimationList * anim = animations[animationIndex];
		anim->Update(timeElapsed);
	}
	

	referenceNodeSuffixChanged = false;

	if(RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_ANIMATED_MESHES))
	{
		size = (int32)animatedMeshes.size();
		for (int32 animatedMeshIndex = 0; animatedMeshIndex < size; ++animatedMeshIndex)
		{
			AnimatedMesh * mesh = animatedMeshes[animatedMeshIndex];
			mesh->Update(timeElapsed);
		}
	}

	//if(imposterManager)
	//{
	//	imposterManager->Update(timeElapsed);
	//}
    
    updateTime = SystemTimer::Instance()->AbsoluteMS() - time;
}

void Scene::Draw()
{
    TIME_PROFILE("Scene::Draw");

    //Sprite * fboSprite = Sprite::CreateAsRenderTarget(512, 512, FORMAT_RGBA8888);
	//RenderManager::Instance()->SetRenderTarget(fboSprite);
	//RenderManager::Instance()->SetViewport(Rect(0, 0, 512, 512), false);
    nodeCounter = 0;
    uint64 time = SystemTimer::Instance()->AbsoluteMS();

	shadowVolumes.clear();
    
    //const GLenum discards[]  = {GL_DEPTH_ATTACHMENT, GL_COLOR_ATTACHMENT0};
    //RENDER_VERIFY(glDiscardFramebufferEXT(GL_FRAMEBUFFER,2,discards));
    //glDepthMask(GL_TRUE);
    //RENDER_VERIFY(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
    
    if(imposterManager)
	{
		//imposterManager->ProcessQueue();
	}
    
    RenderManager::Instance()->SetCullMode(FACE_BACK);
    RenderManager::Instance()->SetState(RenderState::DEFAULT_3D_STATE);
    RenderManager::Instance()->FlushState();
	RenderManager::Instance()->ClearDepthBuffer();
    //glDepthMask(GL_TRUE);
    //RENDER_VERIFY(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
    
	
    if (currentCamera)
    {
        currentCamera->Set();
    }
    
    Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
    renderSystem->SetCamera(currentCamera);
    renderUpdateSystem->Process();
	actionSystem->Process(); //update action system before particles and render
	particleEffectSystem->Process();
	skyboxSystem->Process();
    renderSystem->Render();
    debugRenderSystem->SetCamera(currentCamera);
    debugRenderSystem->Process();
	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, currentCamera->GetMatrix());
	//renderSystem->DebugDrawSpatialTree();

    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);
    
    if(imposterManager)
	{
		imposterManager->Draw();
	}

	RenderManager::Instance()->SetState(RenderState::DEFAULT_2D_STATE_BLEND);
	drawTime = SystemTimer::Instance()->AbsoluteMS() - time;

	//Image * image = Image::Create(512, 512, FORMAT_RGBA8888);
	//RENDER_VERIFY(glReadPixels(0, 0, 512, 512, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)image->data));
	//image->Save("img.png");
	//RenderManager::Instance()->RestoreRenderTarget();
}

	
void Scene::StopAllAnimations(bool recursive )
{
	int32 size = (int32)animations.size();
	for (int32 animationIndex = 0; animationIndex < size; ++animationIndex)
	{
		SceneNodeAnimationList * anim = animations[animationIndex];
		anim->StopAnimation();
	}
	Entity::StopAllAnimations(recursive);
}
    
    
void Scene::SetCurrentCamera(Camera * _camera)
{
    SafeRelease(currentCamera);
    currentCamera = SafeRetain(_camera);
    SafeRelease(clipCamera);
    clipCamera = SafeRetain(_camera);
}

Camera * Scene::GetCurrentCamera() const
{
    return currentCamera;
}

void Scene::SetClipCamera(Camera * _camera)
{
    SafeRelease(clipCamera);
    clipCamera = SafeRetain(_camera);
}

Camera * Scene::GetClipCamera() const
{
    return clipCamera;
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

void Scene::SetReferenceNodeSuffix(const String & suffix)
{
	referenceNodeSuffix = suffix;
	referenceNodeSuffixChanged = true;
}

const String & Scene::GetReferenceNodeSuffix()
{
	return referenceNodeSuffix;
}

bool Scene::IsReferenceNodeSuffixChanged()
{
	return referenceNodeSuffixChanged;
}

EventSystem * Scene::GetEventSystem()
{
	return eventSystem;
}

RenderSystem * Scene::GetRenderSystem() const
{
	return renderSystem;
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
    ScopedPtr<SceneFileV2> file( new SceneFileV2() );
	file->EnableDebugLog(false);
	file->EnableSaveForGame(saveForGame);
	return file->SaveScene(pathname, this);
}




};




