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



#include "Scene3D/Components/WindComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "FileSystem/KeyedArchive.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA 
{

WindComponent::WindComponent() :
    influenceBbox(Vector3(), 10000.f),
    windForce(1.f),
    windSpeed(1.f)
{
}

WindComponent::~WindComponent()
{
    
}
 
Component * WindComponent::Clone(Entity * toEntity)
{
    WindComponent * component = new WindComponent();
	component->SetEntity(toEntity);
    
    component->windForce = windForce;
    component->windSpeed = windSpeed;
    component->influenceBbox = influenceBbox;
    
    return component;
}

void WindComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	Component::Serialize(archive, serializationContext);

	if(archive != 0)
	{
        archive->SetFloat("wc.windForce", windForce);
        archive->SetFloat("wc.windSpeed", windSpeed);
        archive->SetVariant("wc.aabbox", VariantType(influenceBbox));
    }
}
    
void WindComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	if(archive)
    {
        windForce = archive->GetFloat("wc.windForce");
        windSpeed = archive->GetFloat("wc.windSpeed");
        influenceBbox = archive->GetVariant("wc.aabbox")->AsAABBox3();
	}

	Component::Deserialize(archive, serializationContext);
}
    
Vector3 WindComponent::GetDirection() const
{
    DVASSERT(entity);
    DVASSERT(GetTransformComponent(entity));

    const Matrix4 & wtMx = GetTransformComponent(entity)->GetWorldTransform();

    return Vector3(wtMx._00, wtMx._01, wtMx._02); //Get world direction only: wtMx * Vec3(1, 0, 0)
}

};
