#include "Scene/BaseTransformProxies.h"
#include "Particles/ParticleEmitterInstance.h"
#include "Scene3D/Components/ParticleEffectComponent.h"

/*
 * EntityTransformProxy
 */
const DAVA::Matrix4& EntityTransformProxy::GetWorldTransform(Selectable::Object* object)
{
    return static_cast<DAVA::Entity*>(object)->GetWorldTransform();
}

const DAVA::Matrix4& EntityTransformProxy::GetLocalTransform(Selectable::Object* object)
{
    return static_cast<DAVA::Entity*>(object)->GetLocalTransform();
}

void EntityTransformProxy::SetLocalTransform(Selectable::Object* object, const DAVA::Matrix4& matrix)
{
    return static_cast<DAVA::Entity*>(object)->SetLocalTransform(matrix);
}

bool EntityTransformProxy::SupportsTransformType(Selectable::Object* object, Selectable::TransformType type) const
{
    return (type == Selectable::TransformType::Disabled) || (static_cast<DAVA::Entity*>(object)->GetLocked() == false);
}

bool EntityTransformProxy::TransformDependsFromObject(Selectable::Object* dependant, Selectable::Object* dependsOn) const
{
    DVASSERT_MSG(dependant != dependsOn, "[TransformDependsFromObject] One object provided to both parameters");

    auto asEntity = Selectable(dependsOn).AsEntity();
    return (asEntity != nullptr) && asEntity->IsMyChildRecursive(static_cast<DAVA::Entity*>(dependant));
}

/*
 * EmitterTransformProxy
 */
const DAVA::Matrix4& EmitterTransformProxy::GetWorldTransform(Selectable::Object* object)
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

const DAVA::Matrix4& EmitterTransformProxy::GetLocalTransform(Selectable::Object* object)
{
    static DAVA::Matrix4 currentMatrix;
    currentMatrix.Identity();

    auto emitterInstance = static_cast<DAVA::ParticleEmitterInstance*>(object);
    currentMatrix.SetTranslationVector(emitterInstance->GetSpawnPosition());
    return currentMatrix;
}

void EmitterTransformProxy::SetLocalTransform(Selectable::Object* object, const DAVA::Matrix4& matrix)
{
    auto emitterInstance = static_cast<DAVA::ParticleEmitterInstance*>(object);
    emitterInstance->SetSpawnPosition(DAVA::Vector3(matrix._30, matrix._31, matrix._32));
}

bool EmitterTransformProxy::SupportsTransformType(Selectable::Object* object, Selectable::TransformType type) const
{
    auto emitterInstance = static_cast<DAVA::ParticleEmitterInstance*>(object);
    if (emitterInstance->IsInnerEmitter())
        return false;

    return (type == Selectable::TransformType::Disabled) || (type == Selectable::TransformType::Translation);
}

bool EmitterTransformProxy::TransformDependsFromObject(Selectable::Object* dependant, Selectable::Object* dependsOn) const
{
    DVASSERT_MSG(dependant != dependsOn, "[TransformDependsFromObject] One object provided to both parameters");

    auto asEntity = Selectable(dependsOn).AsEntity();
    if (asEntity == nullptr)
        return false;

    // check if emitter instance contained inside entity
    auto component = static_cast<DAVA::ParticleEffectComponent*>(asEntity->GetComponent(DAVA::Component::PARTICLE_EFFECT_COMPONENT));
    if (component != nullptr)
    {
        for (DAVA::uint32 i = 0, e = component->GetEmittersCount(); i < e; ++i)
        {
            if (component->GetEmitterInstance(i) == dependant)
                return true;
        }
    }

    // or it's children
    for (DAVA::int32 i = 0, e = asEntity->GetChildrenCount(); i < e; ++i)
    {
        if (TransformDependsFromObject(dependant, asEntity->GetChild(i)))
            return true;
    }

    return false;
}
