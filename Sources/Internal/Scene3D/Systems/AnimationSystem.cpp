#include "Scene3D/Systems/AnimationSystem.h"
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Entity.h"
#include "Debug/DVAssert.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Debug/Stats.h"
#include "Scene3D/AnimationData.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"

namespace DAVA
{
AnimationSystem::AnimationSystem(Scene* scene)
    : SceneSystem(scene)
{
    if (scene)
    {
        scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::START_ANIMATION);
        scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::STOP_ANIMATION);
    }
}

AnimationSystem::~AnimationSystem()
{
}

void AnimationSystem::Process(float32 timeElapsed)
{
    TIME_PROFILE("AnimationSystem::Process");

    int componentsCount = static_cast<int32>(activeComponents.size());
    for (int i = 0; i < componentsCount; i++)
    {
        AnimationComponent* comp = activeComponents[i];
        comp->time += timeElapsed;
        if (comp->time > comp->animation->duration)
        {
            comp->currRepeatsCont++;
            if (((comp->repeatsCount == 0) || (comp->currRepeatsCont < comp->repeatsCount)))
            {
                comp->time -= comp->animation->duration;
            }
            else
            {
                RemoveFromActive(comp);
                componentsCount--;
                i--;
                comp->animationTransform.Identity();
                continue;
            }
        }

        Matrix4 animTransform;
        comp->animation->Interpolate(comp->time, comp->frameIndex).GetMatrix(animTransform);
        comp->animationTransform = comp->animation->invPose * animTransform;
        GlobalEventSystem::Instance()->Event(comp, EventSystem::ANIMATION_TRANSFORM_CHANGED);
    }
}

void AnimationSystem::ImmediateEvent(Component* component, uint32 event)
{
    DVASSERT(component->GetType() == Component::ANIMATION_COMPONENT);
    AnimationComponent* comp = static_cast<AnimationComponent*>(component);
    if (event == EventSystem::START_ANIMATION)
    {
        if (comp->state == AnimationComponent::STATE_STOPPED)
            AddToActive(comp);
        comp->state = AnimationComponent::STATE_PLAYING;
        comp->currRepeatsCont = 0;
    }
    else if (event == EventSystem::STOP_ANIMATION)
        RemoveFromActive(comp);
}

void AnimationSystem::AddToActive(AnimationComponent* comp)
{
    if (comp->state == AnimationComponent::STATE_STOPPED)
    {
        activeComponents.push_back(comp);
    }
}

void AnimationSystem::RemoveFromActive(AnimationComponent* comp)
{
    Vector<AnimationComponent*>::iterator it = std::find(activeComponents.begin(), activeComponents.end(), comp);
    DVASSERT(it != activeComponents.end());
    activeComponents.erase(it);
    comp->state = AnimationComponent::STATE_STOPPED;
}

void AnimationSystem::RemoveEntity(Entity* entity)
{
    AnimationComponent* comp = GetAnimationComponent(entity);
    if (comp->state != AnimationComponent::STATE_STOPPED)
    {
        RemoveFromActive(comp);
    }
}
};
