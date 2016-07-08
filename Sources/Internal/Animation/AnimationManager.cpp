#include "Animation/AnimationManager.h"
#include "Logger/Logger.h"
#include "Debug/Stats.h"
#include "Job/JobManager.h"
#include "Render/Renderer.h"

#include <typeinfo>

namespace DAVA
{
void AnimationManager::AddAnimation(Animation* animation)
{
    Function<void()> fn = Bind(&AnimationManager::AddAnimationInternal, this, animation);
    JobManager::Instance()->CreateMainJob(fn);
}

void AnimationManager::AddAnimationInternal(Animation* animation)
{
    animations.push_back(animation);
}

void AnimationManager::RemoveAnimation(Animation* animation)
{
    Function<void()> fn = Bind(&AnimationManager::RemoveAnimationInternal, this, animation);
    JobManager::Instance()->CreateMainJob(fn);
}

void AnimationManager::RemoveAnimationInternal(Animation* animation)
{
    Vector<Animation*>::iterator endIt = animations.end();
    for (Vector<Animation*>::iterator t = animations.begin(); t != endIt; ++t)
    {
        if (*t == animation)
        {
            animations.erase(t);
            break;
        }
    }
}

void AnimationManager::StopAnimations()
{
    DVASSERT(Thread::IsMainThread());

    Vector<Animation*>::iterator endIt = animations.end();
    for (Vector<Animation*>::iterator t = animations.begin(); t != endIt; ++t)
    {
        Animation* animation = *t;

        animation->owner = 0; // zero owner to avoid any issues (it was a problem with DumpState, when animations was deleted before).
        animation->state &= ~Animation::STATE_IN_PROGRESS;
        animation->state &= ~Animation::STATE_FINISHED;
        animation->state |= Animation::STATE_DELETE_ME;
    }
}

void AnimationManager::DeleteAnimations(AnimatedObject* owner, int32 track)
{
    Function<void()> fn = Bind(&AnimationManager::DeleteAnimationInternal, this, owner, track);
    JobManager::Instance()->CreateMainJob(fn);
}

void AnimationManager::DeleteAnimationInternal(AnimatedObject* owner, int32 track)
{
    Vector<Animation*>::iterator endIt = animations.end();
    for (Vector<Animation*>::iterator t = animations.begin(); t != endIt; ++t)
    {
        Animation* animation = *t;
        if ((track != -1) && (animation->groupId != track))
        {
            continue;
        }

        if (animation->owner == owner)
        {
            animation->owner = 0; // zero owner to avoid any issues (it was a problem with DumpState, when animations was deleted before).
            animation->state &= ~Animation::STATE_IN_PROGRESS;
            animation->state &= ~Animation::STATE_FINISHED;
            animation->state |= Animation::STATE_DELETE_ME;
        }
    }
}

Animation* AnimationManager::FindLastAnimation(AnimatedObject* _owner, int32 _groupId)
{
    DVASSERT(Thread::IsMainThread());

    Vector<Animation*>::iterator endIt = animations.end();
    for (Vector<Animation*>::iterator t = animations.begin(); t != endIt; ++t)
    {
        Animation* animation = *t;

        if ((animation->owner == _owner) && (animation->groupId == _groupId))
        {
            while (animation->next != 0)
            {
                animation = animation->next;
            }
            return animation; // return latest animation in current group
        }
    }
    return 0;
}

bool AnimationManager::IsAnimating(const AnimatedObject* owner, int32 track) const
{
    DVASSERT(Thread::IsMainThread());

    Vector<Animation*>::const_iterator endIt = animations.end();
    for (Vector<Animation*>::const_iterator t = animations.begin(); t != endIt; ++t)
    {
        Animation* animation = *t;

        if ((track != -1) && (animation->groupId != track))
        {
            continue;
        }

        if ((animation->owner == owner) && (animation->state & Animation::STATE_IN_PROGRESS))
        {
            return true;
        }
    }
    return false;
}

Animation* AnimationManager::FindPlayingAnimation(AnimatedObject* owner, int32 _groupId)
{
    DVASSERT(Thread::IsMainThread());

    Vector<Animation*>::iterator endIt = animations.end();
    for (Vector<Animation*>::iterator t = animations.begin(); t != endIt; ++t)
    {
        Animation* animation = *t;

        if ((_groupId != -1) && (animation->groupId != _groupId))
        {
            continue;
        }

        if ((animation->owner == owner) && (animation->state & Animation::STATE_IN_PROGRESS))
        {
            return animation;
        }
    }

    return 0;
}

bool AnimationManager::HasActiveAnimations(AnimatedObject* owner) const
{
    DVASSERT(Thread::IsMainThread());

    Vector<Animation*>::const_iterator endIt = animations.end();
    for (Vector<Animation*>::const_iterator t = animations.begin(); t != endIt; ++t)
    {
        const Animation* animation = *t;

        if ((animation->owner == owner) && !(animation->state & Animation::STATE_FINISHED))
        {
            return true;
        }
    }
    return false;
}

void AnimationManager::Update(float32 timeElapsed)
{
    TIME_PROFILE("AnimationManager::Update");

    DVASSERT(Thread::IsMainThread());

    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_ANIMATIONS))
        return;

    // update animations first
    for (Animation* animation : animations)
    {
        if (animation->state & Animation::STATE_IN_PROGRESS)
        {
            if (!(animation->state & Animation::STATE_PAUSED))
            {
                animation->Update(timeElapsed);
            }
        }
    }

    // process all finish callbacks
    // someone could change animations list on Stop action
    // it produces crash, so we keep that implementation until
    // external code produces crashes here.
    size_t size = animations.size();
    for (size_t k = 0; k < size; ++k)
    {
        Animation* animation = animations[k];

        if (animation->state & Animation::STATE_FINISHED)
        {
            animation->Stop();
        }
    }

    //check all animation and process all callbacks on delete
    for (Animation* animation : animations)
    {
        if (animation->state & Animation::STATE_DELETE_ME)
        {
            if (!(animation->state & Animation::STATE_FINISHED))
            {
                animation->OnCancel();
            }

            if (animation->next && !(animation->next->state & Animation::STATE_DELETE_ME))
            {
                animation->next->state |= Animation::STATE_IN_PROGRESS;
                animation->next->OnStart();
            }
        }
    }

    //we need physically remove animations only after process all callbacks
    for (Animation* animation : animations)
    {
        if (animation->state & Animation::STATE_DELETE_ME)
        {
            releaseCandidates.push_back(animation);
        }
    }

    //remove all release candidates animations
    for (Animation* releaseAnimation : releaseCandidates)
    {
        SafeRelease(releaseAnimation);
    }
    releaseCandidates.clear();
}

void AnimationManager::DumpState()
{
    DVASSERT(Thread::IsMainThread());

    Logger::FrameworkDebug("============================================================");
    Logger::FrameworkDebug("------------ Currently allocated animations - %2d ---------", animations.size());

    for (int k = 0, end = static_cast<int>(animations.size()); k < end; ++k)
    {
        Animation* animation = animations[k];

        String ownerName = "no owner";
        if (animation->owner)
            ownerName = typeid(*animation->owner).name();
        Logger::FrameworkDebug("addr:0x%08x state:%d class: %s ownerClass: %s", animation, animation->state, typeid(*animation).name(), ownerName.c_str());
    }

    Logger::FrameworkDebug("============================================================");
}

void AnimationManager::PauseAnimations(bool isPaused, int tag)
{
    DVASSERT(Thread::IsMainThread());

    Vector<Animation*>::iterator endIt = animations.end();
    for (Vector<Animation*>::iterator t = animations.begin(); t != endIt; ++t)
    {
        Animation*& a = *t;

        if (a->GetTagId() == tag)
        {
            a->Pause(isPaused);
        }
    }
}

void AnimationManager::SetAnimationsMultiplier(float32 f, int tag)
{
    DVASSERT(Thread::IsMainThread());

    Vector<Animation*>::iterator endIt = animations.end();
    for (Vector<Animation*>::iterator t = animations.begin(); t != endIt; ++t)
    {
        Animation*& a = *t;

        if (a->GetTagId() == tag)
        {
            a->SetTimeMultiplier(f);
        }
    }
}
};