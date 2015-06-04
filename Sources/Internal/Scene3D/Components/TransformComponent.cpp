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


#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"

namespace DAVA
{

    
TransformComponent::TransformComponent()
{
    localMatrix = Matrix4::IDENTITY;
	worldMatrix = Matrix4::IDENTITY;
	parentMatrix = 0;
	parent = 0;
}
    
TransformComponent::~TransformComponent()
{
    
}

Component * TransformComponent::Clone(Entity * toEntity)
{
    TransformComponent * newTransform = new TransformComponent();
	newTransform->SetEntity(toEntity);
	newTransform->localMatrix = localMatrix;
	newTransform->worldMatrix = worldMatrix;
    newTransform->parent = this->parent;

    return newTransform;
}


void TransformComponent::SetLocalTransform(const Matrix4 * transform)
{
	localMatrix = *transform;
	if(!parent)
	{
		worldMatrix = *transform;
	}

	GlobalEventSystem::Instance()->Event(this, EventSystem::LOCAL_TRANSFORM_CHANGED);
}

void TransformComponent::SetParent(Entity * node)
{
	parent = node;

	if(node)
	{
		parentMatrix = ((TransformComponent*)node->GetComponent(Component::TRANSFORM_COMPONENT))->GetWorldTransformPtr();
	}
	else
	{
		parentMatrix = 0;
	}

	GlobalEventSystem::Instance()->Event(this, EventSystem::TRANSFORM_PARENT_CHANGED);
}

Matrix4 & TransformComponent::ModifyLocalTransform()
{
	GlobalEventSystem::Instance()->Event(this, EventSystem::LOCAL_TRANSFORM_CHANGED);
	return localMatrix;
}

void TransformComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	Component::Serialize(archive, serializationContext);

	if(NULL != archive)
	{
		archive->SetMatrix4("tc.localMatrix", localMatrix);
		archive->SetMatrix4("tc.worldMatrix", worldMatrix);
	}
}

void TransformComponent::Deserialize(KeyedArchive *archive, SerializationContext *sceneFile)
{
	if(NULL != archive)
	{
		localMatrix = archive->GetMatrix4("tc.localMatrix", Matrix4::IDENTITY);
		worldMatrix = archive->GetMatrix4("tc.worldMatrix", Matrix4::IDENTITY);
	}

	Component::Deserialize(archive, sceneFile);
}

};