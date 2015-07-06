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


#ifndef __DAVAENGINE_TRANSFORM_COMPONENT_H__
#define __DAVAENGINE_TRANSFORM_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Scene3D/Systems/TransformSystem.h"
#include "Entity/Component.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA 
{

class Entity;

class TransformComponent : public Component
{
protected:
    virtual ~TransformComponent();
public:
    TransformComponent();

    IMPLEMENT_COMPONENT_TYPE(TRANSFORM_COMPONENT);

	inline Matrix4 * GetWorldTransformPtr();
    inline const Matrix4 & GetWorldTransform();
	inline const Matrix4 & GetLocalTransform();
	Matrix4 & ModifyLocalTransform();

	inline int32 GetIndex();

	void SetLocalTransform(const Matrix4 * transform);
	void SetParent(Entity * node);

    virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SerializationContext *serializationContext);
	virtual void Deserialize(KeyedArchive *archive, SerializationContext *serializationContext);

private:
	Matrix4 localMatrix;
	Matrix4 worldMatrix;
	Matrix4 * parentMatrix;
	Entity * parent; //Entity::parent should be removed

	int32 index;

	friend class TransformSystem;
    
public:

    INTROSPECTION_EXTEND(TransformComponent, Component,
        MEMBER(localMatrix, "Local Transform", I_SAVE | I_VIEW)
        MEMBER(worldMatrix, "World Transform", I_SAVE | I_VIEW)
        MEMBER(parentMatrix, "Parent Matrix", I_SAVE)
    );
};

const Matrix4 & TransformComponent::GetWorldTransform()
{
	return worldMatrix;
}

const Matrix4 & TransformComponent::GetLocalTransform()
{
	return localMatrix;
}

int32 TransformComponent::GetIndex()
{
	return index;
}


Matrix4 * TransformComponent::GetWorldTransformPtr()
{
	return &worldMatrix;
}

};

#endif //__DAVAENGINE_TRANSFORM_COMPONENT_H__
