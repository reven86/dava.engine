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


#ifndef __DAVAENGINE_SCENENODE_H__
#define __DAVAENGINE_SCENENODE_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"
#include "Scene3D/SceneNodeAnimationKey.h"
#include "Entity/Component.h"
#include "FileSystem/KeyedArchive.h"
#include "Base/HashMap.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Scene3D/EntityFamily.h"
#include "Scene3D/Components/CustomPropertiesComponent.h"

#include "MemoryManager/MemoryProfiler.h"

namespace DAVA
{

class Scene;
class SceneNodeAnimation;
class SceneNodeAnimationKey;
class SceneFileV2;
class DataNode;
class RenderComponent;
class TransformComponent;

/**
    \brief Base class of 3D scene hierarchy. All nodes in our scene graph is inherited from this node.
 */
class Entity : public BaseObject
{
    DAVA_ENABLE_CLASS_ALLOCATION_TRACKING(ALLOC_POOL_ENTITY);

protected:
	virtual ~Entity();
public:	
	Entity();
	
    /**
        \brief Function to set scene for node and it's children. 
        Function goes recursively and set scene for this node, and each child. 
        \param[in] _scene pointer to scene we want to set as holder for this node. 
     */
    virtual void SetScene(Scene * _scene);//TODO: Move SetScene to private
    /**
        \brief Function to return scene of this node. This is virtual function. For Entity's function returns it's scene value. 
        In Scene class function is overloaded and returns self. It required to avoid dynamic casts to find a scene. 
        \returns pointer to the scene that holds this node. 
     */
    virtual Scene * GetScene();
    
    void AddComponent(Component * component);
    void RemoveComponent(Component * component);
    void RemoveComponent(uint32 componentType, uint32 index = 0);
    void DetachComponent(Component * component);

    Component * GetComponent(uint32 componentType, uint32 index = 0) const;
    Component * GetOrCreateComponent(uint32 componentType, uint32 index = 0);
    uint32 GetComponentCount() const;
    uint32 GetComponentCount(uint32 componentType) const;
    
    inline uint64 GetAvailableComponentFlags();


	// working with childs
	virtual void	AddNode(Entity * node);
    
    virtual void    InsertBeforeNode(Entity *newNode, Entity *beforeNode);
    
	virtual void	RemoveNode(Entity * node);
    virtual int32   GetChildrenCountRecursive() const;
    virtual bool    IsMyChildRecursive(const Entity* child) const;
	virtual void	RemoveAllChildren();
	virtual Entity*	GetNextChild(Entity *child);

    inline  Entity* GetChild(int32 index) const;
	inline  int32   GetChildrenCount() const;

    
	virtual bool FindNodesByNamePart(const String & namePart, List<Entity *> &outNodeList);
    
    /**
        \brief Function sets entity unique ID. WARNING: Almost all time this function shouldn't be used by user, because IDs are
        generated automatically. However, it can be used in some exceptional cases, when the user exactly knows what he is doing,
        for example in the ResourceEditor during ReloadModel operation.

        Entity ID is automatically modified in this cases:
         - when entity with ID = 0 is added into scene, new ID will be generated
         - when entity with ID != 0 is added from one scene into another scene, new ID will be generated
         - cloned entity will always have ID = 0
    */
    void SetID(uint32 id);

    /**
        \brief Function return an entity ID, that is unique within scene. This ID is automatically generated, when entity (with empty ID = 0)
        is added into scene. Generated entity ID will be relative to the scene in which that entity was added. 
    */
    uint32 GetID() const;

    /**
        \brief Function reset entity ID, and IDs in all child entities. ID should be reset only for entities that aren't part of the scene.
    */
    void ResetID();

        /**
        \brief Function allows to find necessary entity by id.
        \returns entity with given id or nullptr
    */
    Entity* GetEntityByID(uint32 id);

    /**
    \brief Get string with path by indexes in scenegraph from root node to current node.
    \returns result string.
    */
    DAVA_DEPRECATED(String GetPathID(Entity * root));

    /**
        \brief Get Node by pathID, generated in prev function.
        \returns result Entity.
     */
    DAVA_DEPRECATED(static Entity * GetNodeByPathID(Entity * root, String pathID));

	/**
        \brief Find node by it's name inside this scene node.
        \param[in] name name of object you want to find. 
        \returns pointer to the object if object with such name available, 0 in case if it's not exists.
     */
	virtual Entity *	FindByName(const FastName & name);

    /**
        \brief Find node by it's name inside this scene node.
        \param[in] name name of object you want to find. 
        \returns pointer to the object if object with such name available, 0 in case if it's not exists.
     */
	virtual Entity *	FindByName(const char * name);
    /**
        \brief Set name of this particular node.
        \param[in] new name for this node
     */
    virtual void SetName(const FastName & name);

    /**
        \brief Set name of this particular node.
        \param[in] new name for this node
     */
    virtual void SetName(const char * name);

    /**
        \brief Get name of this particular node.
        \returns name of this node
     */
    inline const FastName & GetName() const;

    /**
        \brief Get full name of this node from root. This function is slow because it go up by hierarchy and make full node name.
        \returns this node full name from root. Example [MaxScene->camera->instance0]
     */
    String GetFullName();
    
    /**
        \brief Set tag for this object.
        Tag can be used to identify object, or find it. You can mark objects with same properies by tag, and later find them using tag criteria. 
     */
    inline void SetTag(int32 _tag);
    
    /**
        \brief Return tag for this object
        \returns tag for this object
     */
    inline int32 GetTag(); 

	
	// virtual updates
	//virtual void	Update(float32 timeElapsed);
	virtual void	Draw();
	
	// properties
	void SetVisible(const bool & isVisible);
	inline const bool GetVisible();
	inline Entity * GetParent();
	DAVA_DEPRECATED(void SetUpdatable(bool isUpdatable));
	DAVA_DEPRECATED(inline bool GetUpdatable(void));
	inline bool IsLodPart(void);
    virtual bool IsLodMain(Entity *childToCheck = NULL);//if childToCheck is NULL checks the caller node
	
	// extract data from current node to use it in animations
	void ExtractCurrentNodeKeyForAnimation(SceneNodeAnimationKey & resultKey);
	
    const Matrix4 & GetLocalTransform(); 

    /**
     \brief This method means that you always modify matrix you get. 
        If you dont want to modify matrix call GetLocalTransform().
     */
	Matrix4 & ModifyLocalTransform(); 
    const Matrix4 & GetWorldTransform() const;
    
    void SetLocalTransform(const Matrix4 & newMatrix);
	Matrix4 AccamulateLocalTransform(Entity * fromParent);
    Matrix4 AccamulateTransformUptoFarParent(Entity * farParent);
    
    /*
        \brief Go down by hierarchy and bake all transforms.
        Function can be used to bake transforms to minimize amount of matrix multiplications.
     */
    virtual void BakeTransforms();


    /*
        \brief Go down by hierarchy and propogate bool custom property to all childs.
     */
	void PropagateBoolProperty(String name, bool value);
    
	enum EntityFlags
    {
        // NODE_STATIC = 0x1,  // this flag means that node is always static and we do not need to update it's worldTransform
        // NODE_DYNAMIC = 0x2, // node automatically become dynamic when we update it's local matrix
        NODE_WORLD_MATRIX_ACTUAL = 1, // if this flag set this means we do not need to rebuild worldMatrix
        NODE_VISIBLE = 1 << 1, // is node and subnodes should draw
        NODE_UPDATABLE = 1 << 2, // is node and subnodes should updates. This flag is updated by the engine and can be changed at any time. Flag is always rise up on node loading
        NODE_IS_LOD_PART = 1 << 3, // node is part of a LOD node.
        NODE_LOCAL_MATRIX_IDENTITY = 1 << 4, // local matrix of this node is identity. Used to avoid unnecessary computations.
        
        BOUNDING_VOLUME_AABB = 1 << 5,  // node has axial aligned bounding box.
        BOUNDING_VOLUME_OOB = 1 << 6,   // node has object oriented bounding box.
        BOUNDING_VOLUME_SPHERE = 1 << 7,    // node has bounding sphere.

        NODE_CLIPPED_PREV_FRAME = 1 << 8, // 
        NODE_CLIPPED_THIS_FRAME = 1 << 9, // 
        NODE_INVALID = 1 << 10,  // THIS NODE not passed some of verification stages and marked as invalid. Such nodes shouldn't be drawn.

		TRANSFORM_NEED_UPDATE = 1 << 11,
		TRANSFORM_DIRTY = 1 << 12,
		NODE_DELETED = 1 << 13,
        
        // I decided to put scene flags here to avoid 2 variables. But probably we can create additional variable later if it'll be required.
        SCENE_LIGHTS_MODIFIED = 1 << 14,
        
        ENTITY_INDEX_POSITION = 16,
        ENTITY_INDEX_MASK = 0xffff,
    };
	
    inline void AddFlag(int32 flagToAdd);
    inline void RemoveFlag(int32 flagToRemove);
    inline uint32 GetFlags() const;
    void AddFlagRecursive(int32 flagToAdd);
    void RemoveFlagRecursive(int32 flagToRemove);
    inline uint32 GetIndexInParent() { return (flags >> ENTITY_INDEX_POSITION) & ENTITY_INDEX_MASK; };
    inline void SetIndexInParent(uint32 index) { flags |= (index & ENTITY_INDEX_MASK) << ENTITY_INDEX_POSITION; };
    void SetSceneID(uint32 sceneId);
    uint32 GetSceneID() const;

	// animations 
// 	void ExecuteAnimation(SceneNodeAnimation * animation);	
// 	void DetachAnimation(SceneNodeAnimation * animation);
// 	virtual void StopAllAnimations(bool recursive = true);

	
    virtual Entity* Clone(Entity *dstNode = NULL);
	
    // Do not use variables 
    //std::deque<SceneNodeAnimation *> nodeAnimations;

    // Do we need enum, or we can use virtual functions? 
    enum
    {
        EVENT_CREATE_ENTITY = 1,
        EVENT_DELETE_ENTITY,
        EVENT_ADD_COMPONENT,
        EVENT_DELETE_COMPONENT,
        EVENT_NOTIFY_UPDATE,
    };
    
    
	/**
        \brief function to enable or disable debug drawing for particular node.
        By default it's not recursive. Some objects may support flags only partially.
        For example if node do not have bounding box flag DEBUG_DRAW_AABBOX will not produce any output
        These flags are mostly for debug purposes and we do not guarantee that logic of the debug rendering will remain unchanged between 
        framework versions.
     
        \param[in] debugFlags flags to be set
        \param[in] isRecursive do you want to set flags recursively
     
     */
	void SetDebugFlags(uint32 debugFlags, bool isRecursive = false);  
    /**
        \brief function returns debug flags of specific node
        \returns flags of this specific scene node
     */
    uint32 GetDebugFlags() const;
    	
    void SetSolid(bool isSolid);
    bool GetSolid() const;

	void SetLocked(bool isLocked);
	bool GetLocked() const;

    void SetNotRemovable(bool notRemovabe);
    bool GetNotRemovable() const;
    
    /**
        \brief function returns maximum bounding box of scene in world coordinates.
        \returns bounding box
     */
    virtual AABBox3 GetWTMaximumBoundingBoxSlow();
    
    /**
        \brief virtual function to save node to KeyedArchive
     */
    virtual void Save(KeyedArchive * archive, SerializationContext * serializationContext);
    
    /**
        \brief virtual function to load node to KeyedArchive
     */
	virtual void Load(KeyedArchive * archive, SerializationContext * serializationContext);
    
    /**
        \brief Function to get node description for debug printing
     */
    virtual String GetDebugDescription();
        
    /**
        \brief This function should be implemented in each node that have data nodes inside it.
     */
    virtual void GetDataNodes(Set<DataNode*> & dataNodes);
    /**
        \brief Function to get data nodes of requested type to specific container you provide.
     */
    template <template <typename, typename> class Container, class T, class A>
    void GetDataNodes(Container<T, A>& container);
    /**
	 \brief Optimize scene before export.
     */
	void OptimizeBeforeExport();

    /**
        \brief Function to get child nodes of requested type and move them to specific container you provide.
        For example if you want to get a list of MeshInstanceNodes you should do the following.
        \code   
        #include "Scene3D/Entity.h"
        #include "Scene3D/MeshInstanceNode.h"  // You should include MeshInstanceNode because Entity class do not know the type of node you want to convert to. 
        
        void YourClass::YourFunction()
        {
            List<MeshInstanceNode*> meshNodes;
            scene->GetChildNodes(meshNodes);
        }
        \endcode
     */
    template <template <typename, typename> class Container, class T, class A>
    void GetChildNodes(Container<T, A>& container);

    template <template <typename, typename> class Container, class A>
    void GetChildEntitiesWithComponent(Container<Entity*, A>& container, Component::eType type);

    uint32 CountChildEntitiesWithComponent(Component::eType type, bool recursive = false) const;

    /**
        \brief This function is called after scene is loaded from file.
        You can perform additional initialization here.
     */
    virtual void SceneDidLoaded();

	// Property names.
	static const char* SCENE_NODE_IS_SOLID_PROPERTY_NAME;
	static const char* SCENE_NODE_IS_LOCKED_PROPERTY_NAME;
    static const char* SCENE_NODE_IS_NOT_REMOVABLE_PROPERTY_NAME;

	void FindComponentsByTypeRecursive(Component::eType type, List<DAVA::Entity*> & components);
    
    Vector<Entity*> children;
    
    void UpdateFamily();

protected:
    void RemoveAllComponents();
    void LoadComponentsV6(KeyedArchive *compsArch, SerializationContext * serializationContext);
    void LoadComponentsV7(KeyedArchive *compsArch, SerializationContext * serializationContext);
   
protected:

    String RecursiveBuildFullName(Entity * node, Entity * endNode);

	void SetParent(Entity * node);

	Scene * scene;
	Entity * parent;
	FastName name;
	int32 tag;
    uint32 flags;
    uint32 id;
    uint32 sceneId;

private:
	Vector<Component *> components;
    EntityFamily * family;
    void DetachComponent(Vector<Component *>::iterator & it);
    void RemoveComponent(Vector<Component *>::iterator & it);

   	friend class Scene;
    
public:
	INTROSPECTION_EXTEND(Entity, BaseObject,
        PROPERTY("ID", "ID", GetID, SetID, I_VIEW | I_SAVE)
        MEMBER(name, "Name", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(tag, "Tag", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(flags, "Flags", I_SAVE | I_VIEW | I_EDIT )
        PROPERTY("visible", "Visible", GetVisible, SetVisible, I_VIEW | I_EDIT)
    );
};
	
inline const bool Entity::GetVisible()
{
	return (flags & NODE_VISIBLE) != 0;
}
    
inline bool Entity::GetUpdatable(void)
{
	return (flags & NODE_UPDATABLE) != 0;
}
    
inline bool Entity::IsLodPart(void)
{
	return (flags & NODE_IS_LOD_PART) != 0;
}


inline void Entity::AddFlag(int32 flagToAdd)
{
    flags |= flagToAdd;
}
    
inline void Entity::RemoveFlag(int32 flagToRemove)
{
    flags &= ~flagToRemove;
}
    
inline uint32 Entity::GetFlags() const
{
    return flags;
}

inline Entity * Entity::GetParent()
{
	return parent;
}
    
inline const FastName & Entity::GetName() const
{
    return name;
}

inline int32 Entity::GetTag() 
{ 
    return tag; 
}
    
   
inline void Entity::SetTag(int32 _tag)
{
    tag = _tag;
}

template <template <typename, typename> class Container, class T, class A>
void Entity::GetDataNodes(Container<T, A>& container)
{
    Set<DataNode*> objects;
    GetDataNodes(objects);
    
    Set<DataNode*>::const_iterator end = objects.end();
    for (Set<DataNode*>::iterator t = objects.begin(); t != end; ++t)
    {
        DataNode* obj = *t;
        
        T res = dynamic_cast<T> (obj);
        if (res)
            container.push_back(res);
    }	
}

template <template <typename, typename> class Container, class T, class A>
void Entity::GetChildNodes(Container<T, A>& container)
{    
    Vector<Entity*>::const_iterator end = children.end();
    for (Vector<Entity*>::iterator t = children.begin(); t != end; ++t)
    {
        Entity* obj = *t;
        
        T res = dynamic_cast<T> (obj);
        if (res)
            container.push_back(res);
        
        obj->GetChildNodes(container);
    }	
}

template <template <typename, typename> class Container, class A>
void Entity::GetChildEntitiesWithComponent(Container<Entity*, A>& container, Component::eType type)
{
    Vector<Entity*>::const_iterator end = children.end();
    for (Vector<Entity*>::iterator t = children.begin(); t != end; ++t)
    {
        Entity* entity = *t;
        if (entity)
        {
            Component * component = entity->GetComponent(type);
            if (component)
                container.push_back(entity);
        }
        
        entity->GetChildEntitiesWithComponent(container, type);
    }	
}

inline uint64 Entity::GetAvailableComponentFlags()
{
    return family->GetComponentsFlags();
}
    
inline Entity * Entity::GetChild (int32 index) const
{
    return children[index];
}

inline int32 Entity::GetChildrenCount () const
{
    return (int32)children.size();
}

inline uint32 Entity::GetComponentCount () const
{
    return static_cast<uint32>(components.size ());
}

inline void Entity::UpdateFamily ()
{
    EntityFamily::Release (family);
    family = EntityFamily::GetOrCreate (components);
}

inline void Entity::RemoveAllComponents ()
{
    while (!components.empty ())
    {
        RemoveComponent (--components.end ());
    }
}

inline void Entity::RemoveComponent (uint32 componentType, uint32 index)
{
    Component * c = GetComponent (componentType, index);
    if (c)
    {
        RemoveComponent (c);
    }
}

inline void Entity::RemoveComponent (Component * component)
{
    DetachComponent (component);
    SafeDelete (component);
}

inline void Entity::DetachComponent (Component * component)
{
    DVASSERT (component);

    auto it = std::find (components.begin (), components.end (), component);
    DetachComponent (it);
}

inline Scene * Entity::GetScene ()
{
    return scene;
}

inline uint32 Entity::GetComponentCount (uint32 componentType) const
{
    return family->GetComponentsCount (componentType);
}

inline uint32 Entity::GetID() const
{
    return id;
}

inline void Entity::SetID(uint32 id_)
{
    id = id_;
}

inline uint32 Entity::GetSceneID() const
{
    return sceneId;
}

inline void Entity::SetSceneID(uint32 sceneId_)
{
    sceneId = sceneId_;
}

inline void Entity::ResetID()
{
    DVASSERT(nullptr == GetScene() && "ID can safely be reset in entities that aren't part of scene");

    id = 0;
    sceneId = 0;
    for(auto child : children)
    {
        child->ResetID();
    }
}


};

#endif // __DAVAENGINE_SCENENODE_H__





