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


#include "Animation/AnimationManager.h"
#include "FileSystem/Logger.h"
#include "Render/RenderManager.h"

#include <typeinfo>

namespace DAVA
{

void AnimationManager::AddAnimation(Animation * animation)
{
    ScopedPtr<Job> job = JobManager::Instance()->CreateJob(JobManager::THREAD_MAIN, Message(this, &AnimationManager::AddAnimationInternal, animation));
}
    
void AnimationManager::AddAnimationInternal(BaseObject * caller, void * param, void *callerData)
{
    Animation * animation = (Animation*)param;
	animations.push_back(animation);
}

void AnimationManager::RemoveAnimation(Animation * animation)
{
    ScopedPtr<Job> job = JobManager::Instance()->CreateJob(JobManager::THREAD_MAIN, Message(this, &AnimationManager::RemoveAnimationInternal, animation));
}
    
void AnimationManager::RemoveAnimationInternal(BaseObject * caller, void * param, void *callerData)
{
    Animation * animation = (Animation*)param;
	for (Vector<Animation*>::iterator t = animations.begin(); t != animations.end(); ++t)
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
    
    for (Vector<Animation*>::iterator t = animations.begin(); t != animations.end(); ++t)
	{
		Animation * animation = *t;
		
        animation->owner = 0;   // zero owner to avoid any issues (it was a problem with DumpState, when animations was deleted before). 
        animation->state &= ~Animation::STATE_IN_PROGRESS;
        animation->state &= ~Animation::STATE_FINISHED;
        animation->state |= Animation::STATE_DELETE_ME;
	}	
}
	
void AnimationManager::DeleteAnimations(AnimatedObject * owner, int32 track)
{
    DeleteAnimationsData * data = new DeleteAnimationsData();
    data->owner = owner;
    data->track = track;
    
    ScopedPtr<Job> job = JobManager::Instance()->CreateJob(JobManager::THREAD_MAIN, Message(this, &AnimationManager::DeleteAnimationInternal, data));
}
    
void AnimationManager::DeleteAnimationInternal(BaseObject * caller, void * param, void *callerData)
{
    DeleteAnimationsData * data = (DeleteAnimationsData*)param;

	for (Vector<Animation*>::iterator t = animations.begin(); t != animations.end(); ++t)
	{
		Animation * animation = *t;
		if ((data->track != -1) && (animation->groupId != data->track))
        {
            continue;
        }
		
		if (animation->owner == data->owner)
		{
            animation->owner = 0;   // zero owner to avoid any issues (it was a problem with DumpState, when animations was deleted before). 
			animation->state &= ~Animation::STATE_IN_PROGRESS;
			animation->state &= ~Animation::STATE_FINISHED;
			animation->state |= Animation::STATE_DELETE_ME;
		}
	}
    
    SafeDelete(data);
}
	
Animation * AnimationManager::FindLastAnimation(AnimatedObject * _owner, int32 _groupId)
{
    DVASSERT(Thread::IsMainThread());
    
	for (Vector<Animation*>::iterator t = animations.begin(); t != animations.end(); ++t)
	{
		Animation * animation = *t;

		if ((animation->owner == _owner) && (animation->groupId == _groupId))
		{
			while(animation->next != 0)
			{
				animation = animation->next;
			}
			return animation; // return latest animation in current group
		}
	}
	return 0;
}

bool AnimationManager::IsAnimating(AnimatedObject * owner, int32 track)
{
    DVASSERT(Thread::IsMainThread());

	for (Vector<Animation*>::iterator t = animations.begin(); t != animations.end(); ++t)
	{
		Animation * animation = *t;

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

Animation * AnimationManager::FindPlayingAnimation(AnimatedObject * owner, int32 _groupId)
{
    DVASSERT(Thread::IsMainThread());

	for (Vector<Animation*>::iterator t = animations.begin(); t != animations.end(); ++t)
	{
		Animation * animation = *t;

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

void AnimationManager::Update(float32 timeElapsed)
{
    DVASSERT(Thread::IsMainThread());

	if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_ANIMATIONS))
		return;
	
	// update animations first
    uint32 size = (uint32)animations.size();
	for (uint32 k = 0; k < size; ++k)
	{
		Animation * animation = animations[k];

		if (animation->state & Animation::STATE_IN_PROGRESS)
		{
			if(!(animation->state & Animation::STATE_PAUSED))
			{
				animation->Update(timeElapsed);
			}	
		}
	}

	// process all finish callbacks
    size = (uint32)animations.size();
	for (uint32 k = 0; k < size; ++k)
	{
		Animation * animation = animations[k];

		if (animation->state & Animation::STATE_FINISHED)
		{
			animation->Stop(); 
		}
	}

	//remove all old animations
    size = (uint32)animations.size();
	for (uint32 k = 0; k < size; ++k)
	{
		Animation * animation = animations[k];

		if (animation->state & Animation::STATE_DELETE_ME)
		{
			if (!(animation->state & Animation::STATE_FINISHED))
			{
				animation->OnCancel();
			}

			if(animation->next && !(animation->next->state  & Animation::STATE_DELETE_ME))
			{
				animation->next->state |= Animation::STATE_IN_PROGRESS;
				animation->next->OnStart();
			}

			SafeRelease(animation);
            
            size = (uint32)animations.size();
            k--;
		}
	}
}
	
void AnimationManager::DumpState()
{
    DVASSERT(Thread::IsMainThread());

	Logger::FrameworkDebug("============================================================");
	Logger::FrameworkDebug("------------ Currently allocated animations - %2d ---------", animations.size());

	for (int k = 0; k < (int)animations.size(); ++k)
	{
		Animation * animation = animations[k];  

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

	for(Vector<Animation*>::iterator i = animations.begin(); i != animations.end(); ++i)
    {
        Animation * &a = *i;
        
        if (a->GetTagId() == tag)
        {
            a->Pause(isPaused);
        }
    }
}

void AnimationManager::SetAnimationsMultiplier(float32 f, int tag)
{
    DVASSERT(Thread::IsMainThread());
    
    for(Vector<Animation*>::iterator i = animations.begin(); i != animations.end(); ++i)
    {
        Animation * &a = *i;
        
        if (a->GetTagId() == tag)
        {
            a->SetTimeMultiplier(f);
        }
    }
}

};