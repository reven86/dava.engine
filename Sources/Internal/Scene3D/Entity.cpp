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
#include "Scene3D/Components/DebugRenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Utils/Random.h"

#define CUSTOM_PROPERTIES_COMPONENT_SAVE_SCENE_VERSION 8


namespace DAVA
{
    
    

// Property Names.
const char* Entity::SCENE_NODE_IS_SOLID_PROPERTY_NAME = "editor.isSolid";
const char* Entity::SCENE_NODE_IS_LOCKED_PROPERTY_NAME = "editor.isLocked";

Entity::Entity()
	: scene(0)
	, parent(0)
    , tag(0)
	, entity(0)
{
//    Logger::FrameworkDebug("Entity: %p", this);
    componentFlags = 0;

	components.resize(Component::COMPONENT_COUNT);
    for (uint32 k = 0; k < Component::COMPONENT_COUNT; ++k)
        components[k] = 0;
    
    defaultLocalTransform.Identity();
	//animation = 0;
    //debugFlags = DEBUG_DRAW_NONE;
    flags = NODE_VISIBLE | NODE_UPDATABLE | NODE_LOCAL_MATRIX_IDENTITY;
    
	AddComponent(ScopedPtr<TransformComponent> (new TransformComponent()));
    
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

	RemoveAllChildren();
	SetScene(0);

	for(int32 i = 0; i < Component::COMPONENT_COUNT; ++i)
	{
		if(components[i])
		{
			RemoveComponent(components[i]);
		}
	}

//  Logger::FrameworkDebug("~Entity: %p", this);
}
    
void Entity::AddComponent(Component * component)
{
	DVASSERT(NULL == components[component->GetType()]);
	
	component->SetEntity(this);

	components[component->GetType()] = component;
	component->Retain();

    if (scene)
        scene->AddComponent(this, component);

    // SHOULD BE DONE AFTER scene->AddComponent
    componentFlags |= 1 << component->GetType();
}

void Entity::RemoveComponent(Component * component)
{
	if (scene)
	{
		scene->RemoveComponent(this, component);
	}
		
	component->SetEntity(0);
	components[component->GetType()] = 0;
		
	// SHOULD BE DONE AFTER scene->RemoveComponent
	componentFlags &= ~(1 << component->GetType());
	component->Release();
}

void Entity::RemoveComponent(uint32 componentType)
{
    if (components[componentType])
        RemoveComponent(components[componentType]);
}

    
Component * Entity::GetComponent(uint32 componentType) const
{
    return components[componentType];
}

Component * Entity::GetOrCreateComponent(uint32 componentType)
{
	Component * ret = components[componentType];
	if(!ret)
	{
		ret = Component::CreateByType(componentType);
		AddComponent(ret);
		ret->Release();
	}

	return ret;
}
    
uint32 Entity::GetComponentCount()
{
	uint32 bits = componentFlags;
	uint32 count = 0;
	for(; 0 != bits; count++)
	{
		bits &= bits - 1;
	}
	return count;
}

void Entity::SetScene(Scene * _scene)
{
    if (scene == _scene)
    {
        return;
    }
    // Сheck 
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


void Entity::SetParent(Entity * node)
{
	parent = node;
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
		children.push_back(node);
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
                node->SetParent(0);
                node->Release();
            }
			break;
		}
	}
	
}
	
Entity * Entity::GetChild(int32 index) const
{
	return children[index];
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

int32 Entity::GetChildrenCount() const
{
    return (int32)children.size();
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


Entity *	Entity::FindByName(const String & searchName)
{
	if (name == searchName)
		return this;
	
	uint32 size = (uint32)children.size();
	for (uint32 c = 0; c < size; ++c)
	{
		Entity * res = children[c]->FindByName(searchName);
		if (res != 0)return res;
	}
	return 0;
}

	
void Entity::ExecuteAnimation(SceneNodeAnimation * _animation)
{
	nodeAnimations.push_back(_animation);
//	printf("-- add animation: %d node: %s anim: %s\n", nodeAnimations.size(), name.c_str(), _animation->GetParent()->name.c_str()); 
//	if (_animation->GetParent()->name == "a1")
//	{
//		int k = 0;
//		k++;
//	}
}
	
void Entity::DetachAnimation(SceneNodeAnimation * animation)
{
//	int32 size = nodeAnimations.size();
	for (std::deque<SceneNodeAnimation*>::iterator t = nodeAnimations.begin(); t != nodeAnimations.end(); ++t)
	{
		if (*t == animation)
		{
			nodeAnimations.erase(t);
			break;
		}
	}
//	int32 sizeAfter = nodeAnimations.size();
//	if (sizeAfter != size - 1)
//	{
//		printf("******** Error with animation detach");
//	}	
}

void Entity::StopAllAnimations(bool recursive)
{
	nodeAnimations.clear();
	if (recursive)
	{
		uint32 size = (uint32)children.size();
		for (uint32 c = 0; c < size; ++c)
			children[c]->StopAllAnimations(recursive);
	}	
}

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
    if(size == 1 && (0 == GetComponent(Component::LOD_COMPONENT))) // propagate matrices
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
    
    //uint32 size = components.size();
    for (uint32 k = 0; k < Component::COMPONENT_COUNT;++k)
	{
		if(components[k])
		{
			if(dstNode->components[k])
			{
				dstNode->RemoveComponent(dstNode->components[k]);
			}

			Component * clonedComponent = components[k]->Clone(dstNode);
            dstNode->RemoveComponent(clonedComponent->GetType());
			dstNode->AddComponent(clonedComponent);
			clonedComponent->Release();
		}
	}
    
    dstNode->name = name;
    dstNode->tag = tag;
    
    dstNode->SetVisible(GetVisible());
    //dstNode->flags = flags;
    
    dstNode->nodeAnimations = nodeAnimations;
    
	dstNode->RemoveAllChildren();
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
	DebugRenderComponent * debugComponent = cast_if_equal<DebugRenderComponent*>(components[Component::DEBUG_RENDER_COMPONENT]);

	if(!debugComponent)
	{
		AddComponent(ScopedPtr<DebugRenderComponent> (new DebugRenderComponent()));
		debugComponent = cast_if_equal<DebugRenderComponent*>(components[Component::DEBUG_RENDER_COMPONENT]);
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
	DebugRenderComponent * debugComponent = cast_if_equal<DebugRenderComponent*>(components[Component::DEBUG_RENDER_COMPONENT]);
	if(debugComponent)
	{
		return debugComponent->GetDebugFlags();
	}
	else
	{
		return 0;
	}
}

void Entity::SetName(const String & _name)
{
    name = _name;
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
        return RecursiveBuildFullName(node->GetParent(), endNode) + String("->") + node->name; 
    }else
    {
        return node->name;
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

    
void Entity::Save(KeyedArchive * archive, SceneFileV2 * sceneFileV2)
{
    // Perform refactoring and add Matrix4, Vector4 types to VariantType and KeyedArchive
    BaseObject::Save(archive);
        
    archive->SetString("name", name);
    archive->SetInt32("tag", tag);
    archive->SetByteArrayAsType("localTransform", GetLocalTransform());
    archive->SetByteArrayAsType("defaultLocalTransform", defaultLocalTransform);
    
    archive->SetUInt32("flags", flags);
//    archive->SetUInt32("debugFlags", debugFlags);
        
	KeyedArchive *compsArch = new KeyedArchive();
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
			components[i]->Serialize(compArch, sceneFileV2);
			compsArch->SetArchive(KeyedArchive::GenKeyFromIndex(i), compArch);
			compArch->Release();
		}
	}
	
	archive->SetArchive("components", compsArch);
	compsArch->Release();
}

void Entity::Load(KeyedArchive * archive, SceneFileV2 * sceneFileV2)
{
    BaseObject::Load(archive);
        
    name = archive->GetString("name", "");
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
	if(NULL != compsArch)
	{
		for(uint32 i = 0; i < components.size(); ++i)
		{
			KeyedArchive *compArch = compsArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
			if(NULL != compArch)
			{
				uint32 compType = compArch->GetUInt32("comp.type", 0xFFFFFFFF);
				if(compType != 0xFFFFFFFF)
				{
					Component *comp = Component::CreateByType(compType);
					if(NULL != comp)
					{
						comp->Deserialize(compArch, sceneFileV2);
                        RemoveComponent(comp->GetType());
						AddComponent(comp);
						comp->Release();
					}
				}
			}
		}
	}
	
	if(sceneFileV2->GetVersion() < CUSTOM_PROPERTIES_COMPONENT_SAVE_SCENE_VERSION)
	{
		KeyedArchive* customProps = archive->GetArchiveFromByteArray("customprops");
		
		if(customProps != NULL)
		{
			CustomPropertiesComponent* customPropsComponent = GetCustomPropertiesComponent();
			customPropsComponent->LoadFromArchive(*customProps, sceneFileV2);
			
			SafeRelease(customProps);
		}
	}
}
	
CustomPropertiesComponent* Entity::GetCustomPropertiesComponent()
{
	CustomPropertiesComponent* component = (CustomPropertiesComponent *) GetComponent(Component::CUSTOM_PROPERTIES_COMPONENT);
	if(NULL == component)
	{
		component = (CustomPropertiesComponent *) Component::CreateByType(Component::CUSTOM_PROPERTIES_COMPONENT);

		AddComponent(component);
		component->Release();
	}
	
    return component;
}
	
KeyedArchive* Entity::GetCustomProperties()
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
    for (uint32 k = 0; k < Component::COMPONENT_COUNT; ++k)
    {
        if(components[k])
        {
            components[k]->GetDataNodes(dataNodes);
        }
    }

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
		if (components[i])
			components[i]->OptimizeBeforeExport();
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
    
void Entity::SetFog_Kostil(float32 density, const Color &color)
{
    Vector<Material *> materials;
    GetDataNodes(materials);
    
    for(int32 i = 0; i < (int32)materials.size(); ++i)
    {
        materials[i]->SetFogDensity(density);
        materials[i]->SetFogColor(color);
    }
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

void Entity::SetVisible(const bool & isVisible)
{
	RenderComponent * renderComponent = (RenderComponent *)GetComponent(Component::RENDER_COMPONENT);
	if(isVisible) 
	{
		AddFlag(NODE_VISIBLE);
		
		if(renderComponent)
		{
			renderComponent->GetRenderObject()->SetFlags(renderComponent->GetRenderObject()->GetFlags() | RenderObject::VISIBLE);
		}
	}
	else 
	{
		RemoveFlag(NODE_VISIBLE);
		if(renderComponent)
		{
			renderComponent->GetRenderObject()->SetFlags(renderComponent->GetRenderObject()->GetFlags() & ~RenderObject::VISIBLE);
		}
	}

	int32 count = GetChildrenCount();
	for(int32 i = 0; i < count; ++i)
	{
		GetChild(i)->SetVisible(isVisible);
	}
}

void Entity::SetLodVisible(bool isLodVisible)
{
	RenderComponent * renderComponent = (RenderComponent *)GetComponent(Component::RENDER_COMPONENT);
	if(isLodVisible) 
	{
		if(renderComponent)
		{
			renderComponent->GetRenderObject()->SetFlags(renderComponent->GetRenderObject()->GetFlags() | RenderObject::VISIBLE_LOD);
		}
	}
	else 
	{
		if(renderComponent)
		{
			renderComponent->GetRenderObject()->SetFlags(renderComponent->GetRenderObject()->GetFlags() & ~RenderObject::VISIBLE_LOD);
		}
	}

	int32 count = GetChildrenCount();
	for(int32 i = 0; i < count; ++i)
	{
		GetChild(i)->SetLodVisible(isLodVisible);
	}
}

void Entity::SetSwitchVisible(bool isSwitchVisible)
{
	RenderComponent * renderComponent = (RenderComponent *)GetComponent(Component::RENDER_COMPONENT);
	if(isSwitchVisible) 
	{
		if(renderComponent)
		{
			renderComponent->GetRenderObject()->SetFlags(renderComponent->GetRenderObject()->GetFlags() | RenderObject::VISIBLE_SWITCH);
		}
	}
	else 
	{
		if(renderComponent)
		{
			renderComponent->GetRenderObject()->SetFlags(renderComponent->GetRenderObject()->GetFlags() & ~RenderObject::VISIBLE_SWITCH);
		}
	}

	int32 count = GetChildrenCount();
	for(int32 i = 0; i < count; ++i)
	{
		GetChild(i)->SetVisible(isSwitchVisible);
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
