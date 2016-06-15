#include "Scene3D/Components/WaveComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/WaveSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "FileSystem/KeyedArchive.h"

namespace DAVA
{
WaveComponent::WaveComponent()
    : amplitude(10.f)
    , lenght(5.f)
    , speed(5.f)
    , infRadius(25.f)
{
    damping = 1.f / infRadius;
}

WaveComponent::WaveComponent(float32 _amlitude, float32 _lenght, float32 _speed, float32 _damping, float32 _infDistance)
    : amplitude(_amlitude)
    , lenght(_lenght)
    , speed(_speed)
    , damping(_damping)
    , infRadius(_infDistance)
{
}

WaveComponent::~WaveComponent()
{
}

Component* WaveComponent::Clone(Entity* toEntity)
{
    WaveComponent* component = new WaveComponent();
    component->SetEntity(toEntity);

    component->amplitude = amplitude;
    component->lenght = lenght;
    component->speed = speed;
    component->damping = damping;
    component->infRadius = infRadius;

    return component;
}

void WaveComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    if (archive != 0)
    {
        archive->SetFloat("wavec.amplitude", amplitude);
        archive->SetFloat("wavec.lenght", lenght);
        archive->SetFloat("wavec.speed", speed);
        archive->SetFloat("wavec.damping", damping);
        archive->SetFloat("wavec.infRadius", infRadius);
    }
}

void WaveComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (archive)
    {
        amplitude = archive->GetFloat("wavec.amplitude");
        lenght = archive->GetFloat("wavec.lenght");
        speed = archive->GetFloat("wavec.speed");
        damping = archive->GetFloat("wavec.damping");
        infRadius = archive->GetFloat("wavec.infRadius");
    }

    Component::Deserialize(archive, serializationContext);
}

void WaveComponent::Trigger()
{
    GlobalEventSystem::Instance()->Event(this, EventSystem::WAVE_TRIGGERED);
}
};
