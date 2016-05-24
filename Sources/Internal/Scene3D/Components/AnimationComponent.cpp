#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/AnimationData.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Components/ComponentHelpers.h"

namespace DAVA
{
AnimationComponent::AnimationComponent()
    : animation(NULL)
    , time(0.0f)
    , frameIndex(0)
    , repeatsCount(1)
    , currRepeatsCont(0)
    , state(STATE_STOPPED)
{
}

AnimationComponent::~AnimationComponent()
{
    SafeRelease(animation);
}

Component* AnimationComponent::Clone(Entity* toEntity)
{
    AnimationComponent* newAnimation = new AnimationComponent();
    newAnimation->SetEntity(toEntity);

    newAnimation->time = time;
    newAnimation->animation = SafeRetain(animation);
    newAnimation->repeatsCount = repeatsCount;
    newAnimation->currRepeatsCont = 0;
    newAnimation->state = STATE_STOPPED; //for another state we need add this one to AnimationSystem

    return newAnimation;
}

void AnimationComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    if (NULL != archive)
    {
        archive->SetVariant("animation", VariantType(animation->GetNodeID()));
        archive->SetUInt32("repeatsCount", repeatsCount);
    }
}

void AnimationComponent::Deserialize(KeyedArchive* archive, SerializationContext* sceneFile)
{
    if (NULL != archive)
    {
        AnimationData* newAnimation = static_cast<AnimationData*>(sceneFile->GetDataBlock(archive->GetVariant("animation")->AsUInt64()));
        if (animation != newAnimation)
        {
            SafeRelease(animation);
            animation = SafeRetain(newAnimation);
        }
        repeatsCount = archive->GetUInt32("repeatsCount", 1);
    }

    Component::Deserialize(archive, sceneFile);
}

void AnimationComponent::GetDataNodes(Set<DataNode*>& dataNodes)
{
    if (animation)
        dataNodes.insert(animation);
}

void AnimationComponent::SetAnimation(AnimationData* _animation)
{
    if (_animation == animation)
        return;

    SafeRelease(animation);
    animation = SafeRetain(_animation);
}

void AnimationComponent::SetIsPlaying(bool value)
{
    if (state == STATE_STOPPED && value)
        Start();
    if (state == STATE_PLAYING && !value)
        Stop();
}

bool AnimationComponent::GetIsPlaying() const
{
    return state == STATE_PLAYING;
}

void AnimationComponent::Start()
{
    GlobalEventSystem::Instance()->Event(this, EventSystem::START_ANIMATION);
}

void AnimationComponent::StopAfterNRepeats(int32 numberOfRepeats)
{
    repeatsCount = numberOfRepeats;
}

void AnimationComponent::Stop()
{
    if (state == STATE_STOPPED)
        return;
    GlobalEventSystem::Instance()->Event(this, EventSystem::STOP_ANIMATION);
    animationTransform.Identity();
}
};