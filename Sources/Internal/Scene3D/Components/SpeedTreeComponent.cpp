#include "Scene3D/Components/SpeedTreeComponent.h"
#include "FileSystem/KeyedArchive.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"

namespace DAVA
{
SpeedTreeComponent::SpeedTreeComponent()
    :
    trunkOscillationAmplitude(1.f)
    ,
    trunkOscillationSpring(2.f)
    ,
    trunkOscillationDamping(1.4f)
    ,
    leafsOscillationAmplitude(2.f)
    ,
    leafsOscillationSpeed(1.f)
    ,
    maxAnimatedLOD(0)
{
}

SpeedTreeComponent::~SpeedTreeComponent()
{
}

Component* SpeedTreeComponent::Clone(Entity* toEntity)
{
    SpeedTreeComponent* component = new SpeedTreeComponent();
    component->SetEntity(toEntity);

    component->trunkOscillationAmplitude = trunkOscillationAmplitude;
    component->trunkOscillationSpring = trunkOscillationSpring;
    component->trunkOscillationDamping = trunkOscillationDamping;
    component->leafsOscillationAmplitude = leafsOscillationAmplitude;
    component->leafsOscillationSpeed = leafsOscillationSpeed;
    component->SetMaxAnimatedLOD(GetMaxAnimatedLOD());

    return component;
}

void SpeedTreeComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    if (archive != 0)
    {
        archive->SetFloat("stc.trunkOscillationAmplitude", trunkOscillationAmplitude);
        archive->SetFloat("stc.trunkOscillationSpring", trunkOscillationSpring);
        archive->SetFloat("stc.trunkOscillationDamping", trunkOscillationDamping);
        archive->SetFloat("stc.leafsOscillationAmplitude", leafsOscillationAmplitude);
        archive->SetFloat("stc.leafsOscillationSpeed", leafsOscillationSpeed);
        archive->SetInt32("stc.maxAnimatedLOD", maxAnimatedLOD);
    }
}

void SpeedTreeComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (archive)
    {
        trunkOscillationAmplitude = archive->GetFloat("stc.trunkOscillationAmplitude", trunkOscillationAmplitude);
        trunkOscillationSpring = archive->GetFloat("stc.trunkOscillationSpring", trunkOscillationSpring);
        trunkOscillationDamping = archive->GetFloat("stc.trunkOscillationDamping", trunkOscillationDamping);
        leafsOscillationAmplitude = archive->GetFloat("stc.leafsOscillationAmplitude", leafsOscillationAmplitude);
        leafsOscillationSpeed = archive->GetFloat("stc.leafsOscillationSpeed", leafsOscillationSpeed);
        SetMaxAnimatedLOD(archive->GetInt32("stc.maxAnimatedLOD", maxAnimatedLOD));
    }

    Component::Deserialize(archive, serializationContext);
}

void SpeedTreeComponent::SetMaxAnimatedLOD(const int32& lodIndex)
{
    maxAnimatedLOD = lodIndex;
    GlobalEventSystem::Instance()->Event(this, EventSystem::SPEED_TREE_MAX_ANIMATED_LOD_CHANGED);
}
};
