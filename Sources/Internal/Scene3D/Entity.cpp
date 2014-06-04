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


#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneNodeAnimation.h"
#include "Scene3D/SceneNodeAnimationList.h"
#include "FileSystem/KeyedArchive.h"
#include "Base/ObjectFactory.h"
#include "Utils/StringFormat.h"
#include "Render/RenderHelper.h"
#include "Scene3D/SceneFileV2.h"
#include "FileSystem/FileSystem.h"
#include "Debug/Stats.h"
#include "Scene3D/Systems/TransformSystem.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/DebugRenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Utils/Random.h"

#define CUSTOM_PROPERTIES_COMPONENT_SAVE_SCENE_VERSION 8
#define USE_VECTOR(x) (((1 << x) & vectorComponentsMask) != 0)


namespace DAVA
{

const int COMPONENT_COUNT_V6 = 18;
const int COMPONENTS_IN_VECTOR_COUNT = 3;
const int COMPONENTS_BY_NAME_SAVE_SCENE_VERSION = 10;
    
uint32 vectorComponentsMask = (1 << Component::TRANSFORM_COMPONENT) | (1 << Component::RENDER_COMPONENT) | (1 << Component::LOD_COMPONENT);

// Property Names.
const char* Entity::SCENE_NODE_IS_SOLID_PROPERTY_NAME = "editor.isSolid";
const char* Entity::SCENE_NODE_IS_LOCKED_PROPERTY_NAME = "editor.isLocked";
	
Entity::Entity()
: scene(0)
, parent(0)
, tag(0)
#if !defined(COMPONENT_STORAGE_STDMAP)
, componentsMap(4)
#endif
{
	//    Logger::FrameworkDebug("Entity: %p", this);
	componentFlags = 0;
		
	components.resize(COMPONENTS_IN_VECTOR_COUNT);
	for (uint32 k = 0; k < COMPONENTS_IN_VECTOR_COUNT; ++k)
		components[k] = 0;
		
	defaultLocalTransform.Identity();
	//animation = 0;
	//debugFlags = DEBUG_DRAW_NONE;
	flags = NODE_VISIBLE | NODE_UPDATABLE | NODE_LOCAL_MATRIX_IDENTITY;
		
	AddComponent(new TransformComponent());
		
	//    Stats::Instance()->RegisterEvent("Scene.Update.Entity.Update", "Entity update time");
	//    Stats::Instance()->RegisterEvent("Scene.Draw.Entity.Draw", "Entity draw time");
}
	
Entity::~Entity()
{
	/*
		TODO: Double check that everything is working fine.
		*/
	//    if (scene)
	//    {
	//        scene->UnregisterNode(this);
	//        scene = 0;
	//    }
        
    GlobalEventSystem::Instance()->RemoveAllEvents(this);
		
	RemoveAllChildren();
		
	RemoveAllComponents();
		
	SetScene(0);
	//  Logger::Debug("~Entity: %p", this);



}
    
void Entity::AddComponent(Component * component)
{
	component->SetEntity(this);
		
	uint32 componentType = component->GetType();
	if(USE_VECTOR(componentType))
	{
		DVASSERT(NULL == components[componentType]);
		components[componentType] = component;
	}
	else
	{
#if defined(COMPONENT_STORAGE_STDMAP)
			
		ComponentsMap::iterator it = componentsMap.find(componentType);
		if(componentsMap.end() == it)
		{
			Vector<Component*>* componentsVector = new Vector<Component*>();
			std::pair<ComponentsMap::iterator, bool> insertResult =
            componentsMap.insert(std::pair<uint32, Vector<Component*>* >(componentType, componentsVector));
			it = insertResult.first;
		}
			
		it->second->reserve(it->second->size() + 1); //reserve memory to avoid capacity growth
		it->second->push_back(component);
#else
			
		Vector<Component*>* componentsVector = componentsMap.at(componentType);
		if(NULL == componentsVector)
		{
			componentsVector = new Vector<Component*>();
			componentsMap.insert(componentType, componentsVector);
		}
			
		componentsVector->reserve(componentsVector->size() + 1); //reserve memory to avoid capacity growth
		componentsVector->push_back(component);
#endif
	}
		
	componentFlags |= 1 << component->GetType();

	if (scene)
		scene->AddComponent(this, component);
}
    
void Entity::RemoveAllComponents()
{
	for(int32 i = 0; i < COMPONENTS_IN_VECTOR_COUNT; ++i)
	{
		if(components[i])
		{
			CleanupComponent(components[i], 0);
            SafeDelete(components[i]);
		}
	}

	for(ComponentsMap::iterator it = componentsMap.begin();
		it != componentsMap.end();
		++it)
	{
		Vector<Component*>* componentsVector = it->second;
			
		if(NULL != componentsVector)
		{
			int componentCount = componentsVector->size();
				
			for(Vector<Component*>::iterator compIt = componentsVector->begin();
				compIt != componentsVector->end(); ++compIt)
			{
				componentCount--;
				CleanupComponent(*compIt, componentCount);
                SafeDelete(*compIt);
            }
		}

		SafeDelete(componentsVector);
	}

	componentsMap.clear();
}
	
void Entity::RemoveComponent(Component * component)
{
	int componentCount = 0;
	uint32 componentType = component->GetType();
    
    DetachComponent(component);

	if (!USE_VECTOR(componentType))
	{
        Vector<Component*>* componentsVector = NULL;

#if defined(COMPONENT_STORAGE_STDMAP)
        ComponentsMap::iterator it = componentsMap.find( componentType );
        componentsVector = (it != componentsMap.end()) ? it->second : NULL;
#else
        componentsVector = componentsMap[componentType];
#endif

        if ( componentsVector != NULL )
        {
            componentCount = componentsVector->size();
        }
    }

	CleanupComponent(component, componentCount);
    SafeDelete(component);
}

void Entity::DetachComponent( Component * component )
{
    if ( scene )
        scene->RemoveComponent( this, component );

    uint32 componentType = component->GetType();
    uint32 componentCount = 0;

    if (USE_VECTOR(componentType))
    {
        components[componentType] = 0;
    }
    else
    {
#if defined(COMPONENT_STORAGE_STDMAP)

        ComponentsMap::iterator it = componentsMap.find( componentType );
        if (componentsMap.end() != it)
        {
            for (Vector<Component*>::iterator i = it->second->begin();
                i != it->second->end(); ++i )
            {
                if ((*i) == component)
                {
                    it->second->erase(i);
                    break;
                }
            }
        }

#else

        Vector<Component*>* componentsVector = componentsMap[componentType];
        if (NULL != componentsVector)
        {
            for (Vector<Component*>::iterator i = componentsVector->begin();
                i != componentsVector->end(); ++i)
            {
                if ((*i) == component)
                {
                    componentsVector->erase(i);
                    break;
                }
            }
            componentCount = componentsVector->size();
        }

#endif
    }

    CleanupComponent(component, componentCount);
}
    
void Entity::RemoveComponent(uint32 componentType, uint32 index)
{
	if (scene)
	{
		Component *c = GetComponent(componentType, index);
		if(c)
			scene->RemoveComponent(this, c);
	}

	Component* component = NULL;
	int componentCount = 0;
	if(USE_VECTOR(componentType))
	{
		component = components[componentType];
		components[componentType] = NULL;
	}
	else
	{
#if defined(COMPONENT_STORAGE_STDMAP)
			
		ComponentsMap::iterator it = componentsMap.find(componentType);
		if(componentsMap.end() != it &&
			it->second->size() > index)
		{
			component = it->second->at(index);
			it->second->erase(it->second->begin() + index);
				
			componentCount = it->second->size();
		}
			
#else
			
		Vector<Component*>* componentsVector = componentsMap[componentType];
		if(NULL != componentsVector &&
			componentsVector->size() > index)
		{
			component = componentsVector->at(index);
			componentsVector->erase(componentsVector->begin() + index);
				
			componentCount = componentsVector->size();
		}
			
#endif
	}
		
	if(NULL != component)
	{
		CleanupComponent(component, componentCount);
        SafeDelete(component);
	}
}
    
inline void Entity::CleanupComponent(Component* component, uint32 componentCount)
{
	component->SetEntity(0);
		
	if(componentCount <= 0)
	{
		componentFlags &= ~(1 << component->GetType());
	}
}
    
Component * Entity::GetComponent(uint32 componentType, uint32 index) const
{
	Component* component = NULL;
	if(USE_VECTOR(componentType))
	{
		component = components[componentType];
	}
	else
	{
#if defined(COMPONENT_STORAGE_STDMAP)
			
		ComponentsMap::iterator it = componentsMap.find(componentType);
		if(componentsMap.end() != it &&
			it->second->size() > index)
		{
			component = it->second->at(index);
		}
			
#else
			
		Vector<Component*>* componentsVector = componentsMap[componentType];
		if(NULL != componentsVector &&
			componentsVector->size() > index)
		{
			component = componentsVector->at(index);
		}
			
#endif
	}
		
	return component;
}
	
Component * Entity::GetOrCreateComponent(uint32 componentType, uint32 index)
{
	Component * ret = GetComponent(componentType, index);
	if(!ret)
	{
		ret = Component::CreateByType(componentType);
		AddComponent(ret);
	}
		
	return ret;
}
    
uint32 Entity::GetComponentCount()
{
	uint32 count = 0;
	for (uint32 k = 0; k < COMPONENTS_IN_VECTOR_COUNT; ++k)
		if ((componentFlags >> k) & 1)
			count++;
		
#if defined(COMPONENT_STORAGE_STDMAP)
		
	if(componentsMap.size() > 0)
	{
			
		for(ComponentsMap::iterator it = componentsMap.begin();
			it != componentsMap.end(); ++it)
		{
			count += it->second->size();
		}
	}
		
#else
		
	if(componentsMap.size() > 0)
	{
		ComponentsMap::iterator end = componentsMap.end();
		for(ComponentsMap::iterator it = componentsMap.begin();
			it != end;
			++it)
		{
			Vector<Component*>* componentsVector = it->second;
				
			if(NULL != componentsVector)
			{
				count += componentsVector->size();
			}
		}
	}

#endif
	return count;
}
	
uint32 Entity::GetComponentCount(uint32 componentType)
{
	int componentCount = 0;
		
	if(USE_VECTOR(componentType))
	{
		if(components[componentType] != NULL)
		{
			componentCount = 1;
		}
	}
	else
	{
#if defined(COMPONENT_STORAGE_STDMAP)
			
		ComponentsMap::iterator it = componentsMap.find(componentType);
		if(componentsMap.end() != it)
		{
			componentCount = it->second->size();
		}
			
#else
			
		Vector<Component*>* componentsVector = componentsMap[componentType];
		if(NULL != componentsVector)
		{
			componentCount = componentsVector->size();
		}
			
#endif
	}
		
	return componentCount;
}
	
void Entity::SetScene(Scene * _scene)
{
	if (scene == _scene)
	{
		return;
	}
	// РЎheck
	if (scene)
	{
		scene->UnregisterNode(this);
	}
	scene = _scene;
	if (scene)
	{
		scene->RegisterNode(this);
		GlobalEventSystem::Instance()->PerformAllEventsFromCache(this);
	}
		
		
		
	const std::vector<Entity*>::iterator & childrenEnd = children.end();
	for (std::vector<Entity*>::iterator t = children.begin(); t != childrenEnd; ++t)
	{
		(*t)->SetScene(_scene);
	}
}
    
Scene * Entity::GetScene()
{
	return scene;
}
	
	
void Entity::SetParent(Entity * _parent)
{
	parent = _parent;
	((TransformComponent*)GetComponent(Component::TRANSFORM_COMPONENT))->SetParent(parent);
}
	
void Entity::AddNode(Entity * node)
{
	if (node)
	{
		node->Retain();
		if (node->parent)
		{
			node->parent->RemoveNode(node);
		}
        uint32 insertPosition = (uint32)children.size();
		children.push_back(node);
        node->SetIndexInParent(insertPosition);
		node->SetParent(this);
		node->SetScene(GetScene());
	}
}
    
void Entity::InsertBeforeNode(Entity *newNode, Entity *beforeNode)
{
	if(newNode && newNode != beforeNode)
	{
		bool canBeInserted = false;

		//need 2 passes because iterator will be invalidated when the entity
		//is already present in the list of children and changes its position.

		Vector<Entity*>::iterator itEnd = children.end();
		for (Vector<Entity*>::iterator it = children.begin(); it != itEnd; ++it)
		{
			if(beforeNode == (*it))
			{
				canBeInserted = true;
				break;
			}
		}

		if(canBeInserted)
		{
			newNode->Retain();
			if (newNode->parent)
			{
				newNode->parent->RemoveNode(newNode);
			}

			itEnd = children.end();
			for (Vector<Entity*>::iterator it = children.begin(); it != itEnd; ++it)
			{
				if(beforeNode == (*it))
				{
					children.insert(it, newNode);
					newNode->SetParent(this);
					newNode->SetScene(GetScene());
					break;
				}
			}
		}
	}
}


void Entity::RemoveNode(Entity * node)
{
	if (!node)
	{
		return;
	}
		
	const std::vector<Entity*>::iterator & childrenEnd = children.end();
	for (std::vector<Entity*>::iterator t = children.begin(); t != childrenEnd; ++t)
	{
		if (*t == node)
		{
			children.erase(t);
			if (node)
			{
				node->SetScene(0);
                node->SetIndexInParent(ENTITY_INDEX_MASK);
				node->SetParent(0);
				node->Release();
			}
			break;
		}
	}
		
}
		
Entity* Entity::GetNextChild(Entity *child)
{
	Entity* next = NULL;
		
	for(uint32 i = 0; i < children.size(); i++)
	{
		if(children[i] == child)
		{
			if((i + 1) < children.size())
			{
				next = children[i + 1];
			}
			break;
		}
	}
		
	return next;
}
    
int32 Entity::GetChildrenCountRecursive() const
{
	int32 result = 0;
	result += (int32)children.size();
	for (std::vector<Entity*>::const_iterator t = children.begin(); t != children.end(); ++t)
	{
		Entity *node = *t;
		result += node->GetChildrenCountRecursive();
	}
	return result;
}
	
    
void Entity::RemoveAllChildren()
{
	for (std::vector<Entity*>::iterator t = children.begin(); t != children.end(); ++t)
	{
		Entity *node = *t;
		node->SetScene(0);
		node->SetParent(0);
		node->Release();
	}
	children.clear();
}

Entity *	Entity::FindByName(const FastName & searchName)
{
	if (name == searchName)
		return this;
		
	uint32 size = (uint32)children.size();
	for (uint32 c = 0; c < size; ++c)
	{
		Entity * res = children[c]->FindByName(searchName);
		if (res != 0)
		{
			return res;
		}
	}
	return 0;
}

Entity *	Entity::FindByName(const char * searchName)
{
    return FindByName(FastName(searchName));
}
	
	
// void Entity::ExecuteAnimation(SceneNodeAnimation * _animation)
// {
// 	nodeAnimations.push_back(_animation);
// 	//	printf("-- add animation: %d node: %s anim: %s\n", nodeAnimations.size(), name.c_str(), _animation->GetParent()->name.c_str());
// 	//	if (_animation->GetParent()->name == "a1")
// 	//	{
// 	//		int k = 0;
// 	//		k++;
// 	//	}
// }
// 	
// void Entity::DetachAnimation(SceneNodeAnimation * animation)
// {
// 	//	int32 size = nodeAnimations.size();
// 	for (std::deque<SceneNodeAnimation*>::iterator t = nodeAnimations.begin(); t != nodeAnimations.end(); ++t)
// 	{
// 		if (*t == animation)
// 		{
// 			nodeAnimations.erase(t);
// 			break;
// 		}
// 	}
// 	//	int32 sizeAfter = nodeAnimations.size();
// 	//	if (sizeAfter != size - 1)
// 	//	{
// 	//		printf("******** Error with animation detach");
// 	//	}
// }
// 	
// void Entity::StopAllAnimations(bool recursive)
// {
// 	nodeAnimations.clear();
// 	if (recursive)
// 	{
// 		uint32 size = (uint32)children.size();
// 		for (uint32 c = 0; c < size; ++c)
// 			children[c]->StopAllAnimations(recursive);
// 	}
// }
	
void Entity::RestoreOriginalTransforms()
{
	SetLocalTransform(GetDefaultLocalTransform());
		
	uint32 size = (uint32)children.size();
	for (uint32 c = 0; c < size; ++c)
		children[c]->RestoreOriginalTransforms();
}
    
void Entity::BakeTransforms()
{
	uint32 size = (uint32)children.size();
	if(size == 1 && (0 == GetComponent(Component::LOD_COMPONENT)) && (0 == GetComponent(Component::SWITCH_COMPONENT))) // propagate matrices
	{
		for (uint32 c = 0; c < size; ++c)
		{
			children[c]->SetLocalTransform(children[c]->GetLocalTransform() * GetLocalTransform());
			children[c]->SetDefaultLocalTransform(children[c]->GetDefaultLocalTransform() * defaultLocalTransform);
		}
		SetLocalTransform(Matrix4::IDENTITY);
		AddFlag(NODE_LOCAL_MATRIX_IDENTITY);
	}
		
	for(uint32 c = 0; c < size; ++c)
	{
		children[c]->BakeTransforms();
	}
}
	
void Entity::PropagateBoolProperty(String name, bool value)
{
	KeyedArchive *currentProperties = GetCustomProperties();
	currentProperties->SetBool(name, value);
		
	uint32 size = (uint32)children.size();
	if (size > 0) // propagate value to children
	{
		for (uint32 c = 0; c < size; ++c)
		{
			children[c]->PropagateBoolProperty(name, value);
		}
	}
}
	
	
	
void Entity::ExtractCurrentNodeKeyForAnimation(SceneNodeAnimationKey & key)
{
	const Matrix4 & localTransform = GetLocalTransform();
	key.time = 0.0f;
	key.translation.x = localTransform._30;
	key.translation.y = localTransform._31;
	key.translation.z = localTransform._32;
	key.rotation.Construct(localTransform);
	//key.matrix = localTransform;
}
	
    
//void Entity::Update(float32 timeElapsed)
//{
//    //Stats::Instance()->BeginTimeMeasure("Scene.Update.Entity.Update", this);
//
////    if (!(flags & NODE_UPDATABLE))return;
//
//    inUpdate = true;
//	// TODO - move node update to render because any of objects can change params of other objects
//	if (nodeAnimations.size() != 0)
//	{
//		Quaternion blendedRotation;
//		Vector3 blendedTranslation;
//		float32 accumWeight = 0.0f;
//		std::deque<SceneNodeAnimation*>::const_iterator end = nodeAnimations.end();
//		for (std::deque<SceneNodeAnimation*>::iterator it = nodeAnimations.begin(); it != end; ++it)
//		{
//			SceneNodeAnimation * animation = *it;
//			SceneNodeAnimationKey & key = animation->Intepolate(animation->GetCurrentTime());
//			if (accumWeight == 0.0f)
//			{
//				blendedTranslation = key.translation;
//				blendedRotation = key.rotation;
//				accumWeight = animation->weight;
//			}else
//			{
//				float32 factor = animation->weight / (accumWeight + animation->weight);
//				accumWeight += accumWeight;
//				blendedTranslation.Lerp(blendedTranslation, key.translation, factor);
//				blendedRotation.Slerp(blendedRotation, key.rotation, factor);
//			}
//			//key.GetMatrix(localTransform);
//		}
//		Matrix4 localTransformTrans;
//		Matrix4 localTransformRot;
//		Matrix4 localTransformFinal;
//		localTransformTrans.CreateTranslation(blendedTranslation);
//		localTransformRot = blendedRotation.GetMatrix();
//
//		localTransform = localTransformRot * localTransformTrans;
//
////		if (nodeAnimations.size() != 1)
////		{
////			printf("-- blended node: %s\n", name.c_str());
////			std::deque<SceneNodeAnimation*>::const_iterator end = nodeAnimations.end();
////			for (std::deque<SceneNodeAnimation*>::iterator it = nodeAnimations.begin(); it != end; ++it)
////			{
////				SceneNodeAnimation * animation = *it;
////				printf(">>> blend: %s wei: %f inDelay: %f\n", animation->GetParent()->name.c_str(), animation->weight, animation->delayTime);
////			}
////		}
//	}
//
//	UpdateTransform();
//	uint32 size = (uint32)children.size();
//	for (uint32 c = 0; c < size; ++c)
//	{
//		children[c]->Update(timeElapsed);
//	}
//
//	//printf("- node: %s tr: %f %f %f\n", name.c_str(), localTransform.data[12], localTransform.data[13], localTransform.data[14]);
//
//
//	inUpdate = false;
//
//    if (!removedCache.empty())
//    {
//        for (std::deque<Entity*>::iterator t = removedCache.begin(); t != removedCache.end(); ++t)
//        {
//            RemoveNode(*t);
//        }
//        removedCache.clear();
//    }
//    //Stats::Instance()->EndTimeMeasure("Scene.Update.Entity.Update", this);
//}
	
void Entity::Draw()
{
	//Stats::Instance()->BeginTimeMeasure("Scene.Draw.Entity.Draw", this);
		
	if (!(flags & NODE_VISIBLE) || !(flags & NODE_UPDATABLE) || (flags & NODE_INVALID))return;
		
	//uint32 size = (uint32)children.size();
	const Vector<Entity*>::iterator & itEnd = children.end();
	for (Vector<Entity*>::iterator it = children.begin(); it != itEnd; ++it)
		(*it)->Draw();
	if (scene)
		scene->nodeCounter++;
		
#if 0
	if (debugFlags & DEBUG_DRAW_AABOX_CORNERS)
	{
		//		Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
		//		Matrix4 finalMatrix = worldTransform * prevMatrix;
		//		RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);
			
		AABBox3 box = GetWTMaximumBoundingBoxSlow();
		if(box == AABBox3())
		{
			box.min = Vector3(0, 0, 0) * GetWorldTransform();
			box.max = box.min;
		}
			
		RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
		RenderManager::Instance()->SetState(RenderStateBlock::STATE_COLORMASK_ALL | RenderStateBlock::STATE_DEPTH_WRITE | RenderStateBlock::STATE_DEPTH_TEST);
		RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		RenderHelper::Instance()->DrawCornerBox(box);
		RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
		RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		//		RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);
	}
		
	if (debugFlags & DEBUG_DRAW_RED_AABBOX)
	{
		AABBox3 box = GetWTMaximumBoundingBoxSlow();
		RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
		RenderManager::Instance()->SetState(RenderStateBlock::STATE_COLORMASK_ALL | RenderStateBlock::STATE_DEPTH_WRITE | RenderStateBlock::STATE_DEPTH_TEST);
		RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
		RenderHelper::Instance()->DrawBox(box);
		RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
		RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
#endif
		
		
	//Stats::Instance()->EndTimeMeasure("Scene.Draw.Entity.Draw", this);
}
	
    
void Entity::SceneDidLoaded()
{
	const Vector<Entity*>::const_iterator & itEnd = children.end();
	for (Vector<Entity*>::iterator it = children.begin(); it != itEnd; ++it)
		(*it)->SceneDidLoaded();
}
    
Entity* Entity::Clone(Entity *dstNode)
{
	if (!dstNode)
	{
		DVASSERT_MSG(IsPointerToExactClass<Entity>(this), "Can clone only Entity");
		dstNode = new Entity();
	}
	dstNode->defaultLocalTransform = defaultLocalTransform;
		
	dstNode->RemoveAllComponents();
	for (uint32 k = 0; k < COMPONENTS_IN_VECTOR_COUNT;++k)
	{
		if(components[k])
		{
			dstNode->AddComponent(components[k]->Clone(dstNode));
		}
	}
		
#if defined(COMPONENT_STORAGE_STDMAP)
		
	for(ComponentsMap::iterator it = componentsMap.begin();
		it != componentsMap.end(); ++it)
	{
		for(Vector<Component*>::iterator compIt = it->second->begin();
			compIt != it->second->end(); ++compIt)
		{
			dstNode->AddComponent((*compIt)->Clone(dstNode));
		}
	}
		
#else
		
	for(ComponentsMap::iterator it = componentsMap.begin();
		it != componentsMap.end(); ++it)
	{
		Vector<Component*>* componentsVector = it->second;
			
		if(NULL != componentsVector)
		{
			for(Vector<Component*>::iterator compIt = componentsVector->begin();
				compIt != componentsVector->end(); ++compIt)
			{
				dstNode->AddComponent((*compIt)->Clone(dstNode));
			}
		}
	}
		
#endif
		
	dstNode->name = name;
	dstNode->tag = tag;
	//dstNode->flags = flags;
		
	//dstNode->nodeAnimations = nodeAnimations;
		
	dstNode->RemoveAllChildren();
    dstNode->children.reserve(children.size());
	std::vector<Entity*>::iterator it = children.begin();
	const std::vector<Entity*>::iterator & childsEnd = children.end();
	for(; it != childsEnd; it++)
	{
		Entity *n = (*it)->Clone();
		dstNode->AddNode(n);
		n->Release();
	}
		
	return dstNode;
}
	
void Entity::SetDebugFlags(uint32 debugFlags, bool isRecursive)
{
	DebugRenderComponent * debugComponent = cast_if_equal<DebugRenderComponent*>(GetComponent(Component::DEBUG_RENDER_COMPONENT));
		
	if(!debugComponent)
	{
		AddComponent(new DebugRenderComponent());
		debugComponent = cast_if_equal<DebugRenderComponent*>(GetComponent(Component::DEBUG_RENDER_COMPONENT));
		debugComponent->SetDebugFlags(DebugRenderComponent::DEBUG_AUTOCREATED);
	}
		
	debugComponent->SetDebugFlags(debugFlags);
	if(0 == debugFlags)
	{
		if(debugComponent->GetDebugFlags() & DebugRenderComponent::DEBUG_AUTOCREATED)
		{
			RemoveComponent(Component::DEBUG_RENDER_COMPONENT);
		}
	}
		
	if (isRecursive)
	{
		std::vector<Entity*>::iterator it = children.begin();
		const std::vector<Entity*>::iterator & childrenEnd = children.end();
		for(; it != childrenEnd; it++)
		{
			Entity *n = (*it);
			n->SetDebugFlags(debugFlags, isRecursive);
		}
	}
}

uint32 Entity::GetDebugFlags() const
{
	DebugRenderComponent * debugComponent = cast_if_equal<DebugRenderComponent*>(GetComponent(Component::DEBUG_RENDER_COMPONENT));
	if(debugComponent)
	{
		return debugComponent->GetDebugFlags();
	}

    return 0;
}

void Entity::SetName(const FastName & _name)
{
	name = _name;
}

void Entity::SetName(const char * _name)
{
	name = FastName(_name);
}
	
String Entity::GetFullName()
{
	return RecursiveBuildFullName(this, scene);
}
	
String Entity::RecursiveBuildFullName(Entity * node, Entity * endNode)
{
	if (!node)
		return "";
        
	if (node->GetParent() != endNode)
	{
		return RecursiveBuildFullName(node->GetParent(), endNode) + String("->") + String(node->name.c_str());
	}else
	{
		return String(node->name.c_str());
	}
}
    
bool Entity::FindNodesByNamePart(const String &namePart, List<Entity *> &outNodeList)
{
	bool isFind = false;
	size_t fp = name.find(namePart);
	if (fp != String::npos)
	{
		outNodeList.push_back(this);
		isFind = true;
	}
		
	int32 sz = (int32)children.size();
	for (int32 i = 0; i < sz; i++)
	{
		if (children[i]->FindNodesByNamePart(namePart, outNodeList))
		{
			isFind = true;
		}
	}
		
	return isFind;
}
	
AABBox3 Entity::GetWTMaximumBoundingBoxSlow()
{
	AABBox3 retBBox;
		
	RenderComponent * renderComponent = static_cast<RenderComponent*>(GetComponent(Component::RENDER_COMPONENT));
	TransformComponent * transformComponent = static_cast<TransformComponent*>(GetComponent(Component::TRANSFORM_COMPONENT));
	if (renderComponent && transformComponent)
	{
		AABBox3 wtBox;
		renderComponent->GetRenderObject()->GetBoundingBox().GetTransformedBox(transformComponent->GetWorldTransform(), wtBox);
		retBBox.AddAABBox(wtBox);
	}
		
	const Vector<Entity*>::iterator & itEnd = children.end();
	for (Vector<Entity*>::iterator it = children.begin(); it != itEnd; ++it)
	{
		AABBox3 box = (*it)->GetWTMaximumBoundingBoxSlow();
		if(  (AABBOX_INFINITY != box.min.x && AABBOX_INFINITY != box.min.y && AABBOX_INFINITY != box.min.z)
			&&(-AABBOX_INFINITY != box.max.x && -AABBOX_INFINITY != box.max.y && -AABBOX_INFINITY != box.max.z))
		{
			retBBox.AddAABBox(box);
		}
	}
		
	return retBBox;
}
    
String Entity::GetDebugDescription()
{
	return Format("children: %d ", GetChildrenCount());
}
	
    
void Entity::Save(KeyedArchive * archive, SerializationContext * serializationContext)
{
	// Perform refactoring and add Matrix4, Vector4 types to VariantType and KeyedArchive
	BaseObject::Save(archive);

	archive->SetString("name", String(name.c_str()));
	archive->SetInt32("tag", tag);
	archive->SetByteArrayAsType("localTransform", GetLocalTransform());
	archive->SetByteArrayAsType("defaultLocalTransform", defaultLocalTransform);
		
	archive->SetUInt32("flags", flags);
	//    archive->SetUInt32("debugFlags", debugFlags);
        
	KeyedArchive *compsArch = new KeyedArchive();
	uint32 savedIndex = 0;
	for(uint32 i = 0; i < components.size(); ++i)
	{
		if(NULL != components[i])
		{
			//don't save empty custom properties
			if(Component::CUSTOM_PROPERTIES_COMPONENT == i)
			{
				CustomPropertiesComponent* customProps = cast_if_equal<CustomPropertiesComponent*>(components[i]);
				if(customProps && customProps->GetArchive()->Count() <= 0)
				{
					continue;
				}
			}
				
			KeyedArchive *compArch = new KeyedArchive();
			components[i]->Serialize(compArch, serializationContext);
			compsArch->SetArchive(KeyedArchive::GenKeyFromIndex(savedIndex), compArch);
			compArch->Release();
			savedIndex++;
		}
	}
		
#if defined(COMPONENT_STORAGE_STDMAP)
		
	for(ComponentsMap::iterator it = componentsMap.begin();
		it != componentsMap.end(); ++it)
	{
		for(Vector<Component*>::iterator compIt = it->second->begin();
			compIt != it->second->end(); ++compIt)
		{
			KeyedArchive *compArch = new KeyedArchive();
			(*compIt)->Serialize(compArch, sceneFileV2);
			compsArch->SetArchive(compTypeName, compArch);
			compArch->Release();
			savedIndex++;
		}
	}
		
#else
		
	for(ComponentsMap::iterator it = componentsMap.begin();
		it != componentsMap.end();
		++it)
	{
		Vector<Component*>* componentsVector = it->second;
			
		if(NULL != componentsVector)
		{
			for(Vector<Component*>::iterator compIt = componentsVector->begin();
				compIt != componentsVector->end(); ++compIt)
			{
				KeyedArchive *compArch = new KeyedArchive();
				(*compIt)->Serialize(compArch, serializationContext);
				compsArch->SetArchive(KeyedArchive::GenKeyFromIndex(savedIndex), compArch);
				compArch->Release();
				savedIndex++;
			}
		}
	}
		
#endif
		
	compsArch->SetUInt32("count", savedIndex);
	archive->SetArchive("components", compsArch);
	compsArch->Release();
}
	
void Entity::Load(KeyedArchive * archive, SerializationContext * serializationContext)
{
	BaseObject::Load(archive);
        
	name = FastName(archive->GetString("name", "").c_str());
	tag = archive->GetInt32("tag", 0);
		
	flags = archive->GetUInt32("flags", NODE_VISIBLE);
	flags |= NODE_UPDATABLE;
	flags &= ~TRANSFORM_DIRTY;
		
	const Matrix4 & localTransform = archive->GetByteArrayAsType("localTransform", GetLocalTransform());
	SetLocalTransform(localTransform);
	defaultLocalTransform = archive->GetByteArrayAsType("defaultLocalTransform", defaultLocalTransform);
		
	/// InvalidateLocalTransform();
	//    debugFlags = archive->GetUInt32("debugFlags", 0);
		
	KeyedArchive *compsArch = archive->GetArchive("components");
		
	if(serializationContext->GetVersion() < COMPONENTS_BY_NAME_SAVE_SCENE_VERSION)
	{
		LoadComponentsV6(compsArch, serializationContext);
	}
	else
	{
		LoadComponentsV7(compsArch, serializationContext);
	}
		
	if(serializationContext->GetVersion() < CUSTOM_PROPERTIES_COMPONENT_SAVE_SCENE_VERSION)
	{
		KeyedArchive* customProps = archive->GetArchiveFromByteArray("customprops");
			
		if(customProps != NULL)
		{
			CustomPropertiesComponent* customPropsComponent = GetCustomPropertiesComponent();
			customPropsComponent->LoadFromArchive(*customProps, serializationContext);
				
			SafeRelease(customProps);
		}
	}
}
    
void Entity::LoadComponentsV6(KeyedArchive *compsArch, SerializationContext * serializationContext)
{
	if(NULL != compsArch)
	{
		for(uint32 i = 0; i < COMPONENT_COUNT_V6; ++i)
		{
			KeyedArchive *compArch = compsArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
			if(NULL != compArch)
			{
				uint32 compType = compArch->GetUInt32("comp.type", 0xFFFFFFFF);
				if(compType != 0xFFFFFFFF)
				{
					//VI{
					//Need to swap these 2 components since their order in the enum
					//has been changed
					if(Component::DEBUG_RENDER_COMPONENT == compType)
					{
						compType = Component::LOD_COMPONENT;
						compArch->SetUInt32("comp.type", compType);
					}
					else if(Component::LOD_COMPONENT == compType)
					{
						compType = Component::DEBUG_RENDER_COMPONENT;
						compArch->SetUInt32("comp.type", compType);
					}
					//}VI
						
					Component *comp = Component::CreateByType(compType);
					if(NULL != comp)
					{
						if(compType == Component::TRANSFORM_COMPONENT)
							RemoveComponent(compType);
						AddComponent(comp);
						comp->Deserialize(compArch, serializationContext);
					}
				}
			}
		}
	}		
}
	
CustomPropertiesComponent* Entity::GetCustomPropertiesComponent()
{
	CustomPropertiesComponent* component = (CustomPropertiesComponent *) GetComponent(Component::CUSTOM_PROPERTIES_COMPONENT);
	if(NULL == component)
	{
		component = new CustomPropertiesComponent();
		AddComponent(component);
	}
		
	return component;
}
	
void Entity::LoadComponentsV7(KeyedArchive *compsArch, SerializationContext * serializationContext)
{
	if(NULL != compsArch)
	{
		uint32 componentCount = compsArch->GetUInt32("count");
		for(uint32 i = 0; i < componentCount; ++i)
		{
			KeyedArchive *compArch = compsArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
			if(NULL != compArch)
			{
				String componentType = compArch->GetString("comp.typename");
				Component* comp = ObjectFactory::Instance()->New<Component>(componentType);
				if(NULL != comp)
				{
					if(comp->GetType() == Component::TRANSFORM_COMPONENT)
					{
						RemoveComponent(comp->GetType());
					}
						
					AddComponent(comp);
					comp->Deserialize(compArch, serializationContext);
				}
			}
		}
	}
}
	
KeyedArchive * Entity::GetCustomProperties()
{
	CustomPropertiesComponent* component = GetCustomPropertiesComponent();
	return component->GetArchive();
}
    
void Entity::SetSolid(bool isSolid)
{
	GetCustomProperties()->SetBool(SCENE_NODE_IS_SOLID_PROPERTY_NAME, isSolid);
}
    
bool Entity::GetSolid()
{
	return GetCustomProperties()->GetBool(SCENE_NODE_IS_SOLID_PROPERTY_NAME, false);
}
	
void Entity::SetLocked(bool isLocked)
{
	GetCustomProperties()->SetBool(SCENE_NODE_IS_LOCKED_PROPERTY_NAME, isLocked);
}
	
bool Entity::GetLocked()
{
	return GetCustomProperties()->GetBool(SCENE_NODE_IS_LOCKED_PROPERTY_NAME, false);
}
	
void Entity::GetDataNodes(Set<DataNode*> & dataNodes)
{
	for (uint32 k = 0; k < components.size(); ++k)
	{
		if(components[k])
		{
			components[k]->GetDataNodes(dataNodes);
		}
	}
		
#if defined(COMPONENT_STORAGE_STDMAP)
		
	for(ComponentsMap::iterator it = componentsMap.begin();
		it != componentsMap.end(); ++it)
	{
		for(Vector<Component*>::iterator compIt = it->second->begin();
			compIt != it->second->end(); ++compIt)
		{
			(*compIt)->GetDataNodes(dataNodes);
		}
	}
		
#else
		
	for(ComponentsMap::iterator it = componentsMap.begin();
		it != componentsMap.end(); ++it)
	{
		Vector<Component*>* componentsVector = it->second;
			
		if(NULL != componentsVector)
		{
			for(Vector<Component*>::iterator compIt = componentsVector->begin();
				compIt != componentsVector->end(); ++compIt)
			{
				(*compIt)->GetDataNodes(dataNodes);
			}
		}
	}
		
#endif
		
	uint32 size = (uint32)children.size();
	for (uint32 c = 0; c < size; ++c)
	{
		children[c]->GetDataNodes(dataNodes);
	}
}
	
void Entity::OptimizeBeforeExport()
{
	for (uint32 i = 0; i < Component::COMPONENT_COUNT; ++i)
	{
        uint32 componentsCount = GetComponentCount(i);
        for(uint32 index = 0; index < componentsCount; ++index)
        {
            Component *c = GetComponent(i, index);
            c->OptimizeBeforeExport();
        }
	}
		
	uint32 size = (uint32)children.size();
	for (uint32 c = 0; c < size; ++c)
	{
		children[c]->OptimizeBeforeExport();
	}
}
    
void Entity::AddFlagRecursive(int32 flagToAdd)
{
	AddFlag(flagToAdd);
	const Vector<Entity*>::iterator &itEnd = children.end();
	for (Vector<Entity*>::iterator it = children.begin(); it != itEnd; ++it)
	{
		(*it)->AddFlagRecursive(flagToAdd);
	}
}
	
void Entity::RemoveFlagRecursive(int32 flagToRemove)
{
	RemoveFlag(flagToRemove);
	const Vector<Entity*>::iterator &itEnd = children.end();
	for (Vector<Entity*>::iterator it = children.begin(); it != itEnd; ++it)
	{
		(*it)->RemoveFlagRecursive(flagToRemove);
	}
}
	
bool Entity::IsLodMain(Entity *childToCheck)
{
	if (!parent || !IsLodPart())
	{
		return true;
	}
		
	return parent->IsLodMain(this);
}
	
String Entity::GetPathID(Entity * root)
{
	String result;
	Entity * curr = this;
	Entity * parent = NULL;
	int32 sz, i;
		
	while (curr != root)
	{
		parent = curr->GetParent();
		sz = parent->GetChildrenCount();
		for (i = 0; i < sz; i++)
		{
			if (curr == parent->GetChild(i))
			{
				result = Format("%d:", i) + result;
				break;
			}
		}
		curr = parent;
	}
	return result;
}
	
Entity * Entity::GetNodeByPathID(Entity * root, String pathID)
{
	Entity * result = root;
	int32 offs = 0;
	int32 index = 0;
	int32 sz = pathID.size();
	char val;
	while (offs < sz)
	{
		val = pathID[offs];
		if (val < '0' || val > '9')
		{
			offs++;
			if (index >=0 && result->GetChildrenCount() > index)
				result = result->GetChild(index);
			else
				return NULL;
			continue;
		}
		index = index * 10 + val - '0';
		offs++;
	}
	return result;
}
	
Matrix4 & Entity::ModifyLocalTransform()
{
	return ((TransformComponent*)GetComponent(Component::TRANSFORM_COMPONENT))->ModifyLocalTransform();
}
	
void Entity::SetLocalTransform(const Matrix4 & newMatrix)
{
	TIME_PROFILE("Entity::SetLocalTransform");
	((TransformComponent*)GetComponent(Component::TRANSFORM_COMPONENT))->SetLocalTransform(&newMatrix);
}
	
const Matrix4 & Entity::GetLocalTransform()
{
	return ((TransformComponent*)GetComponent(Component::TRANSFORM_COMPONENT))->GetLocalTransform();
}
	
const Matrix4 & Entity::GetWorldTransform() const
{
	return ((TransformComponent*)GetComponent(Component::TRANSFORM_COMPONENT))->GetWorldTransform();
}
	
void Entity::SetVisible(const bool &isVisible)
{
	RenderComponent * renderComponent = (RenderComponent *)GetComponent(Component::RENDER_COMPONENT);
	if(isVisible)
	{
		AddFlag(NODE_VISIBLE);
			
		if(renderComponent)
		{
			RenderObject * renderObject = renderComponent->GetRenderObject();
			renderObject->SetFlags(renderObject->GetFlags() | RenderObject::VISIBLE);
			if ((renderObject->GetFlags() & RenderObject::NEED_UPDATE) &&
				renderObject->GetRenderSystem() != NULL)
			{
				renderObject->GetRenderSystem()->MarkForUpdate(renderObject);
			}
		}
	}
	else
	{
		RemoveFlag(NODE_VISIBLE);
		if(renderComponent)
		{
			RenderObject * renderObject = renderComponent->GetRenderObject();
			renderObject->SetFlags(renderObject->GetFlags() & ~RenderObject::VISIBLE);
		}
	}
        
    ParticleEffectComponent *effect = GetEffectComponent(this);
    if (effect)
        effect->SetRenderObjectVisible(isVisible);
		
	int32 count = GetChildrenCount();
	for(int32 i = 0; i < count; ++i)
	{
		GetChild(i)->SetVisible(isVisible);
	}
}
	
void Entity::SetUpdatable(bool isUpdatable)
{
	RenderComponent * renderComponent = (RenderComponent *)GetComponent(Component::RENDER_COMPONENT);
	if(isUpdatable)
	{
		AddFlag(NODE_UPDATABLE);
		if(renderComponent)
		{
			renderComponent->GetRenderObject()->SetFlags(renderComponent->GetRenderObject()->GetFlags() | RenderObject::VISIBLE);
		}
	}
	else
	{
		RemoveFlag(NODE_UPDATABLE);
		if(renderComponent)
		{
			renderComponent->GetRenderObject()->SetFlags(renderComponent->GetRenderObject()->GetFlags() & ~RenderObject::VISIBLE);
		}
	}
		
	int32 count = GetChildrenCount();
	for(int32 i = 0; i < count; ++i)
	{
		GetChild(i)->SetUpdatable(isUpdatable);
	}
}
	
Matrix4 Entity::AccamulateLocalTransform(Entity * fromParent)
{
	if (fromParent == this)
	{
		return GetLocalTransform();
	}
	return GetLocalTransform() * parent->AccamulateLocalTransform(fromParent);
}

Matrix4 Entity::AccamulateTransformUptoFarParent(Entity * farParent)
{
    if (farParent == this)
    {
        return Matrix4::IDENTITY;
    }
    return GetLocalTransform() * parent->AccamulateTransformUptoFarParent(farParent);
}
	
void Entity::FindComponentsByTypeRecursive(Component::eType type, List<DAVA::Entity*> & components)
{
	Component * component = GetComponent(type);
	if (component)
	{
		components.push_back(this);
	}
		
	uint32 childCount = GetChildrenCount();
	for(uint32 i = 0 ; i < childCount; ++i)
	{
		GetChild(i)->FindComponentsByTypeRecursive(type, components);
	}
}
	
	
	
};
