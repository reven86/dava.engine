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

AnimationSystem::AnimationSystem(Scene * scene)
:	SceneSystem(scene)
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
    for(int i = 0; i < componentsCount; i++) 
    {
        AnimationComponent * comp = activeComponents[i];
        comp->time += timeElapsed;
        if (comp->time > comp->animation->duration)
        {
            comp->currRepeatsCont++;
            if (((comp->repeatsCount==0) || (comp->currRepeatsCont < comp->repeatsCount)))
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

void AnimationSystem::ImmediateEvent(Component * component, uint32 event)
{
    AnimationComponent * comp = DynamicTypeCheck<AnimationComponent*>(component);
    if (!comp) return;
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

void AnimationSystem::AddToActive( AnimationComponent *comp )
{
    if (comp->state == AnimationComponent::STATE_STOPPED)
    {
        activeComponents.push_back(comp);
    }
}

void AnimationSystem::RemoveFromActive( AnimationComponent *comp )
{
    Vector<AnimationComponent*>::iterator it = std::find(activeComponents.begin(), activeComponents.end(), comp);
    DVASSERT(it!=activeComponents.end());
    activeComponents.erase(it);
    comp->state = AnimationComponent::STATE_STOPPED;
}

void AnimationSystem::RemoveEntity(Entity * entity)
{
    AnimationComponent *comp = GetAnimationComponent(entity);
    if (comp->state != AnimationComponent::STATE_STOPPED)
    {
        RemoveFromActive(comp);
    }
}

};
