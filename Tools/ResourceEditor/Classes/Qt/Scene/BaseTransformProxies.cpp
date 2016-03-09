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

#include "Scene/BaseTransformProxies.h"
#include "Particles/ParticleEmitterInstance.h"
#include "Scene3D/Components/ParticleEffectComponent.h"

/*
 * EntityTransformProxy
 */
const DAVA::Matrix4& EntityTransformProxy::GetWorldTransform(DAVA::BaseObject* object)
{
    return static_cast<DAVA::Entity*>(object)->GetWorldTransform();
}

const DAVA::Matrix4& EntityTransformProxy::GetLocalTransform(DAVA::BaseObject* object)
{
    return static_cast<DAVA::Entity*>(object)->GetLocalTransform();
}

void EntityTransformProxy::SetLocalTransform(DAVA::BaseObject* object, const DAVA::Matrix4& matrix)
{
    return static_cast<DAVA::Entity*>(object)->SetLocalTransform(matrix);
}

bool EntityTransformProxy::SupportsTransformType(SelectableObject::TransformType type) const
{
    return true;
}

/*
 * EmitterTransformProxy
 */
const DAVA::Matrix4& EmitterTransformProxy::GetWorldTransform(DAVA::BaseObject* object)
{
    static DAVA::Matrix4 currentMatrix;
    currentMatrix.Identity();

    auto emitterInstance = static_cast<DAVA::ParticleEmitterInstance*>(object);
    auto ownerComponent = emitterInstance->GetOwner();
    if ((ownerComponent == nullptr) || (ownerComponent->GetEntity() == nullptr))
    {
        currentMatrix.SetTranslationVector(emitterInstance->GetSpawnPosition());
    }
    else
    {
        const auto& entityTransform = ownerComponent->GetEntity()->GetWorldTransform();
        DAVA::Vector3 center = emitterInstance->GetSpawnPosition();
        TransformPerserveLength(center, DAVA::Matrix3(entityTransform));
        currentMatrix.SetTranslationVector(center + entityTransform.GetTranslationVector());
    }
    return currentMatrix;
}

const DAVA::Matrix4& EmitterTransformProxy::GetLocalTransform(DAVA::BaseObject* object)
{
    static DAVA::Matrix4 currentMatrix;
    currentMatrix.Identity();

    auto emitterInstance = static_cast<DAVA::ParticleEmitterInstance*>(object);
    currentMatrix.SetTranslationVector(emitterInstance->GetSpawnPosition());
    return currentMatrix;
}

void EmitterTransformProxy::SetLocalTransform(DAVA::BaseObject* object, const DAVA::Matrix4& matrix)
{
    auto emitterInstance = static_cast<DAVA::ParticleEmitterInstance*>(object);
    auto ownerComponent = emitterInstance->GetOwner();
    // if ((ownerComponent == nullptr) || (ownerComponent->GetEntity() == nullptr))
    {
        emitterInstance->SetSpawnPosition(DAVA::Vector3(matrix._30, matrix._31, matrix._32));
    }
    // else
    {
        // const auto& entityTransform = ownerComponent->GetEntity()->GetWorldTransform();
        // W = E * S + Et
        // S = (W - Et) * S(-1)
    }
}

bool EmitterTransformProxy::SupportsTransformType(SelectableObject::TransformType type) const
{
    return (type == SelectableObject::TransformType::Translation);
}
