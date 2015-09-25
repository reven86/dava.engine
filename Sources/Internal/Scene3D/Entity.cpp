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
#include "Scene3D/Components/ComponentHelpers.h"
#include <functional>

#define USE_VECTOR(x) ((((uint64)1 << (uint64)x) & vectorComponentsMask) != (uint64)0)

namespace DAVA
{

const int COMPONENT_COUNT_V6 = 18;
    
uint64 vectorComponentsMask = MAKE_COMPONENT_MASK(Component::TRANSFORM_COMPONENT) | MAKE_COMPONENT_MASK(Component::RENDER_COMPONENT) | MAKE_COMPONENT_MASK(Component::LOD_COMPONENT);

// Property Names
const char* Entity::SCENE_NODE_IS_SOLID_PROPERTY_NAME = "editor.isSolid";
const char* Entity::SCENE_NODE_IS_LOCKED_PROPERTY_NAME = "editor.isLocked";
const char* Entity::SCENE_NODE_IS_NOT_REMOVABLE_PROPERTY_NAME = "editor.isNotRemovable";
    
	
Entity::Entity()
    : scene(nullptr)
    , parent(nullptr)
    , tag(0)
    , family(nullptr)
    , id(0)
    , sceneId(0)
{
	flags = NODE_VISIBLE | NODE_UPDATABLE | NODE_LOCAL_MATRIX_IDENTITY;
    UpdateFamily();
		
	AddComponent(new TransformComponent());
}
	
Entity::~Entity()
{
	RemoveAllChildren();	
	RemoveAllComponents();	
	SetScene(nullptr);
    EntityFamily::Release(family);
}

bool ComponentLessPredicate(Component * left, Component * right)
{
    return left->GetType() < right->GetType();
}
    
void Entity::AddComponent(Component * component)
{
	component->SetEntity(this);
    components.push_back(component);
		
    std::stable_sort(components.begin(), components.end(), ComponentLessPredicate);
    UpdateFamily();

	if (scene)
		scene->RegisterComponent(this, component);
}

void Entity::DetachComponent(Vector<Component *>::iterator & it)
{
    Component * c = *it;

    if (scene)
    {
        scene->UnregisterComponent (this, c);
    }
    
    components.erase(it);
    UpdateFamily();
    c->SetEntity(nullptr);
}
  
Component * Entity::GetComponent(uint32 componentType, uint32 index) const
{
    Component * ret = nullptr;
    uint32 maxCount = family->GetComponentsCount(componentType);
    if(index < maxCount)
    {
        ret = components[family->GetComponentIndex(componentType, index)];
    }
    
    return ret;
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
	
void Entity::SetScene(Scene * _scene)
{
	if (scene == _scene)
	{
		return;
	}

	if (scene)
	{
		scene->UnregisterEntity(this);
	}
	scene = _scene;
	if (scene)
	{
		scene->RegisterEntity(this);
        for(auto & it : components)
        {
            GlobalEventSystem::Instance()->PerformAllEventsFromCache(it);
        }
	}

	const Vector<Entity*>::iterator & childrenEnd = children.end();
	for (Vector<Entity*>::iterator t = children.begin(); t != childrenEnd; ++t)
	{
		(*t)->SetScene(_scene);
	}
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
		
	const Vector<Entity*>::iterator & childrenEnd = children.end();
	for (Vector<Entity*>::iterator t = children.begin(); t != childrenEnd; ++t)
	{
		if (*t == node)
		{
			children.erase(t);
			if (node)
			{
                node->SetScene(nullptr);
                node->SetIndexInParent(ENTITY_INDEX_MASK);
                node->SetParent(nullptr);
				node->Release();
			}
			break;
		}
	}
		
}
		
Entity* Entity::GetNextChild(Entity *child)
{
    Entity* next = nullptr;
		
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
	for (Vector<Entity*>::const_iterator t = children.begin(); t != children.end(); ++t)
	{
		Entity *node = *t;
		result += node->GetChildrenCountRecursive();
	}
	return result;
}

bool Entity::IsMyChildRecursive(const Entity* child) const
{
    if (std::find(children.begin(), children.end(), child) != children.end())
    {
        return true;
    }
    else
    {
        return std::any_of(children.begin(), children.end(), [&](const Entity* ch){return ch->IsMyChildRecursive(child);});
    }
}
    
void Entity::RemoveAllChildren()
{
	for (Vector<Entity*>::iterator t = children.begin(); t != children.end(); ++t)
	{
		Entity *node = *t;
        node->SetScene(nullptr);
        node->SetParent(nullptr);
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
	
    
void Entity::BakeTransforms()
{
	uint32 size = (uint32)children.size();
    if (size == 1 && (0 == GetComponent(Component::RENDER_COMPONENT) && 0 == GetComponent(Component::PARTICLE_EFFECT_COMPONENT) && 0 == GetComponent(Component::ANIMATION_COMPONENT))) // propagate matrices
    {
		children[0]->SetLocalTransform(children[0]->GetLocalTransform() * GetLocalTransform());
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
	KeyedArchive *currentProperties = GetOrCreateCustomProperties(this)->GetArchive();
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
		
	dstNode->RemoveAllComponents();
    size_t size = components.size();
	for (size_t k = 0; k < size;++k)
	{
        dstNode->AddComponent(components[k]->Clone(dstNode));
	}
		
	dstNode->name = name;
	dstNode->tag = tag;
    dstNode->sceneId = sceneId;
    dstNode->id = 0;
    
    //flags are intentionally not cloned
	//dstNode->flags = flags;
		
	dstNode->RemoveAllChildren();
    dstNode->children.reserve(children.size());
	Vector<Entity*>::iterator it = children.begin();
	const Vector<Entity*>::iterator & childsEnd = children.end();
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
		Vector<Entity*>::iterator it = children.begin();
		const Vector<Entity*>::iterator & childrenEnd = children.end();
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
	BaseObject::SaveObject(archive);

	archive->SetString("name", String(name.c_str()));
	archive->SetInt32("tag", tag);
    archive->SetUInt32("id", id);
	archive->SetByteArrayAsType("localTransform", GetLocalTransform());
		
	archive->SetUInt32("flags", flags);
	//    archive->SetUInt32("debugFlags", debugFlags);
        
	KeyedArchive *compsArch = new KeyedArchive();
	uint32 savedIndex = 0;
    for (uint32 i = 0; i < components.size (); ++i)
    {
        if (components[i]->GetType () < Component::DEBUG_COMPONENTS)
        {
            //don't save empty custom properties
            if (Component::CUSTOM_PROPERTIES_COMPONENT == i)
            {
                CustomPropertiesComponent* customProps = cast_if_equal<CustomPropertiesComponent*> (components[i]);
                if (customProps && customProps->GetArchive ()->Count () <= 0)
                {
                    continue;
                }
            }

            KeyedArchive *compArch = new KeyedArchive ();
            components[i]->Serialize (compArch, serializationContext);
            compsArch->SetArchive (KeyedArchive::GenKeyFromIndex (savedIndex), compArch);
            compArch->Release ();
            savedIndex++;
        }
	}
		
	compsArch->SetUInt32("count", savedIndex);
	archive->SetArchive("components", compsArch);
	compsArch->Release();
}
	
void Entity::Load(KeyedArchive * archive, SerializationContext * serializationContext)
{
	BaseObject::LoadObject(archive);
        
	name = FastName(archive->GetString("name", "").c_str());
	tag = archive->GetInt32("tag", 0);

    id = archive->GetUInt32("id", 0);
    if(nullptr != serializationContext->GetScene())
    {
        sceneId = serializationContext->GetScene()->GetSceneID();
    }
		
	flags = archive->GetUInt32("flags", NODE_VISIBLE);
	flags |= NODE_UPDATABLE;
	flags &= ~TRANSFORM_DIRTY;

	const Matrix4 & localTransform = archive->GetByteArrayAsType("localTransform", GetLocalTransform());
	SetLocalTransform(localTransform);
		
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
			CustomPropertiesComponent* customPropsComponent = GetOrCreateCustomProperties(this);
			customPropsComponent->LoadFromArchive(*customProps, serializationContext);
				
			customProps->Release();
		}
	}
}
    
void Entity::LoadComponentsV6(KeyedArchive *compsArch, SerializationContext * serializationContext)
{
    if (nullptr != compsArch)
	{
		for(uint32 i = 0; i < COMPONENT_COUNT_V6; ++i)
		{
			KeyedArchive *compArch = compsArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
            if (nullptr != compArch)
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
                    if (nullptr != comp)
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
		
void Entity::LoadComponentsV7(KeyedArchive *compsArch, SerializationContext * serializationContext)
{
    if (nullptr != compsArch)
	{
		uint32 componentCount = compsArch->GetUInt32("count");
		for(uint32 i = 0; i < componentCount; ++i)
		{
			KeyedArchive *compArch = compsArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
            if (nullptr != compArch)
			{
				String componentType = compArch->GetString("comp.typename");
				Component* comp = ObjectFactory::Instance()->New<Component>(componentType);
                if (nullptr != comp)
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
	    
void Entity::SetSolid(bool isSolid)
{
    KeyedArchive *props = GetOrCreateCustomProperties(this)->GetArchive();
	props->SetBool(SCENE_NODE_IS_SOLID_PROPERTY_NAME, isSolid);
}
    
bool Entity::GetSolid() const
{
    KeyedArchive *props = GetCustomPropertiesArchieve(this);
    if (nullptr != props)
    {
        return props->GetBool(SCENE_NODE_IS_SOLID_PROPERTY_NAME, false);
    }
	return false;
}
	
void Entity::SetLocked(bool isLocked)
{
    KeyedArchive *props = GetOrCreateCustomProperties(this)->GetArchive();
	props->SetBool(SCENE_NODE_IS_LOCKED_PROPERTY_NAME, isLocked);
}
	
bool Entity::GetLocked() const
{
    KeyedArchive *props = GetCustomPropertiesArchieve(this);
    if (nullptr != props)
    {
        return props->GetBool(SCENE_NODE_IS_LOCKED_PROPERTY_NAME, false);
    }
	return false;
}
	
void Entity::SetNotRemovable(bool notRemovable)
{
    KeyedArchive *props = GetOrCreateCustomProperties(this)->GetArchive();
    props->SetBool(SCENE_NODE_IS_NOT_REMOVABLE_PROPERTY_NAME, notRemovable);
}

bool Entity::GetNotRemovable() const
{
    KeyedArchive *props = GetCustomPropertiesArchieve(this);
    if (nullptr != props)
    {
        return props->GetBool(SCENE_NODE_IS_NOT_REMOVABLE_PROPERTY_NAME, false);
    }
    return false;
}

    
void Entity::GetDataNodes(Set<DataNode*> & dataNodes)
{
    size_t size = components.size();
	for (size_t k = 0; k < size; ++k)
	{
        components[k]->GetDataNodes(dataNodes);
	}

		
    size = children.size();
	for (size_t c = 0; c < size; ++c)
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
    if (nullptr == parent || !IsLodPart())
	{
		return true;
	}
		
	return parent->IsLodMain(this);
}
	
String Entity::GetPathID(Entity * root)
{
	String result;
	Entity * curr = this;
    Entity * parent = nullptr;
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
	int32 sz = static_cast<int32>(pathID.size());
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
                return nullptr;
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
//	TIME_PROFILE("Entity::SetLocalTransform");
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
			
        if (nullptr != renderComponent)
		{
			RenderObject * renderObject = renderComponent->GetRenderObject();
			renderObject->SetFlags(renderObject->GetFlags() | RenderObject::VISIBLE);
			if ((renderObject->GetFlags() & RenderObject::NEED_UPDATE) &&
                renderObject->GetRenderSystem() != nullptr)
			{
				renderObject->GetRenderSystem()->MarkForUpdate(renderObject);
			}
		}
	}
	else
	{
		RemoveFlag(NODE_VISIBLE);
        if (nullptr != renderComponent)
		{
			RenderObject * renderObject = renderComponent->GetRenderObject();
			renderObject->SetFlags(renderObject->GetFlags() & ~RenderObject::VISIBLE);
		}
	}
        
    ParticleEffectComponent *effect = GetEffectComponent(this);
    if (nullptr != effect)
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
        if (nullptr != renderComponent)
		{
			renderComponent->GetRenderObject()->SetFlags(renderComponent->GetRenderObject()->GetFlags() | RenderObject::VISIBLE);
		}
	}
	else
	{
		RemoveFlag(NODE_UPDATABLE);
        if (nullptr != renderComponent)
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
    if (nullptr != component)
	{
		components.push_back(this);
	}
		
	uint32 childCount = GetChildrenCount();
	for(uint32 i = 0 ; i < childCount; ++i)
	{
		GetChild(i)->FindComponentsByTypeRecursive(type, components);
	}
}

uint32 Entity::CountChildEntitiesWithComponent(Component::eType type, bool recursive /* = false */) const
{
    uint32 count = 0;
    for (auto childEntity : children)
    {
        if (childEntity->GetComponent(type))
        {
            ++count;
        }
        if (recursive)
        {
            count += childEntity->CountChildEntitiesWithComponent(type, recursive);
        }
    }
    return count;
}

inline void Entity::RemoveComponent (Vector<Component *>::iterator & it)
{
    if (it != components.end ())
    {
        Component * c = *it;
        GlobalEventSystem::Instance()->RemoveAllEvents(c);
        DetachComponent (it);
        SafeDelete (c);
    }
}

Entity* Entity::GetEntityByID(uint32 id)
{
    Entity* ret = nullptr;

    if (this->id == id)
    {
        ret = this;
    }
    else
    {
        for (auto child : children)
        {
            ret = child->GetEntityByID(id);
            if (nullptr != ret)
            {
                break;
            }
        }
    }

    return ret;
}

};
