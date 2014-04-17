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
		
AnimationManager::AnimationManager()
{
#ifdef ANIMATIONS_DEBUG	
	animationLoggerEnabled = false;
#endif
}
	
AnimationManager::~AnimationManager()
{
}
	
void AnimationManager::SetAnimationLoggerEnabled(bool isEnabled)
{
#ifdef ANIMATIONS_DEBUG
	animationLoggerEnabled = isEnabled;
#endif
}
bool AnimationManager::IsAnimationLoggerEnabled()
{
#ifdef ANIMATIONS_DEBUG
	return animationLoggerEnabled;
#endif
	return false;
}


void AnimationManager::AddAnimation(Animation * animation)
{
	animationMutex.Lock();
	animations.push_back(animation);
	animationMutex.Unlock();

//#ifdef ANIMATIONS_DEBUG
//	if(animationLoggerEnabled)
//	{
//		Logger::FrameworkDebug("ANIMATION LOGGER: AnimationManager::AddAnimation 0x%x   new animations size %d", (int)animation, animations.size());
//	}
//#endif
}

void AnimationManager::RemoveAnimation(Animation * animation)
{
	//Debug::Log("");
	//Logger::FrameworkDebug("RemoveAnimation: before animations: %d\n", animations.size());
	//std::remo ve(animations.begin(), animations.end(), animation);

	animationMutex.Lock();
	for (Vector<Animation*>::iterator t = animations.begin(); t != animations.end(); ++t)
	{
		if (*t == animation)
		{
			animations.erase(t);
//#ifdef ANIMATIONS_DEBUG
//			if(animationLoggerEnabled)
//			{
//				Logger::FrameworkDebug("ANIMATION LOGGER: AnimationManager::RemoveAnimation 0x%x   new animations size %d", (int)animation, animations.size());
//			}
//#endif
			break;
		}
	}
	animationMutex.Unlock();
	//Logger::FrameworkDebug("RemoveAnimation: after animations: %d\n", animations.size());
	
}
    
void AnimationManager::StopAnimations()
{
	animationMutex.Lock();
    for (Vector<Animation*>::iterator t = animations.begin(); t != animations.end(); ++t)
	{
		Animation * animation = *t;
		
        animation->owner = 0;   // zero owner to avoid any issues (it was a problem with DumpState, when animations was deleted before). 
        animation->state &= ~Animation::STATE_IN_PROGRESS;
        animation->state &= ~Animation::STATE_FINISHED;
        animation->state |= Animation::STATE_DELETE_ME;
	}	
	animationMutex.Unlock();
}
	
void AnimationManager::DeleteAnimations(AnimatedObject * _owner, int32 track)
{
	animationMutex.Lock();
	for (Vector<Animation*>::iterator t = animations.begin(); t != animations.end(); ++t)
	{
		Animation * animation = *t;
        animationMutex.Unlock();

		if ((track != -1) && (animation->groupId != track))
        {
            animationMutex.Lock();
            continue;
        }
		
		if (animation->owner == _owner)
		{
//#ifdef ANIMATIONS_DEBUG
//			if(animationLoggerEnabled)
//			{
//				Logger::FrameworkDebug("ANIMATION LOGGER: AnimationManager::Set  DELETE_ME 0x%x    for owner 0x%x", (int)animation, (int)_owner);
//			}
//#endif
            animation->owner = 0;   // zero owner to avoid any issues (it was a problem with DumpState, when animations was deleted before). 
			animation->state &= ~Animation::STATE_IN_PROGRESS;
			animation->state &= ~Animation::STATE_FINISHED;
			animation->state |= Animation::STATE_DELETE_ME;
		}
        
        animationMutex.Lock();
	}
	animationMutex.Unlock();
}
	
Animation * AnimationManager::FindLastAnimation(AnimatedObject * _owner, int32 _groupId)
{
	animationMutex.Lock();
	for (Vector<Animation*>::iterator t = animations.begin(); t != animations.end(); ++t)
	{
		Animation * animation = *t;
        animationMutex.Unlock();

		if ((animation->owner == _owner) && (animation->groupId == _groupId))
		{
			while(animation->next != 0)
			{
				animation = animation->next;
			}
			return animation; // return latest animation in current group
		}

        animationMutex.Lock();
	}
	animationMutex.Unlock();
	return 0;
}

bool AnimationManager::IsAnimating(AnimatedObject * owner, int32 track)
{
	animationMutex.Lock();
	for (Vector<Animation*>::iterator t = animations.begin(); t != animations.end(); ++t)
	{
		Animation * animation = *t;
        animationMutex.Unlock();

		if ((track != -1) && (animation->groupId != track))
        {
            animationMutex.Lock();
            continue;
        }

		
		if ((animation->owner == owner) && (animation->state & Animation::STATE_IN_PROGRESS))
		{
			return true;
		}
        
        animationMutex.Lock();
	}

	animationMutex.Unlock();
	return false;
}

Animation * AnimationManager::FindPlayingAnimation(AnimatedObject * owner, int32 _groupId)
{
	animationMutex.Lock();
	for (Vector<Animation*>::iterator t = animations.begin(); t != animations.end(); ++t)
	{
		Animation * animation = *t;
        animationMutex.Unlock();

		if ((_groupId != -1) && (animation->groupId != _groupId))
        {
            animationMutex.Lock();
            continue;
        }

		if ((animation->owner == owner) && (animation->state & Animation::STATE_IN_PROGRESS))
		{
			return animation;
		}

        animationMutex.Lock();
    }

	animationMutex.Unlock();
	return 0;
}

void AnimationManager::Update(float32 timeElapsed)
{
	if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_ANIMATIONS))
		return;
	//int animationsCount = (int)animations.size();
	//NSLog(@"animations: %d\n", animations.size());
	
	// update animations first
	animationMutex.Lock();
    uint32 size = (uint32)animations.size();
	for (uint32 k = 0; k < size; ++k)
	{
		Animation * animation = animations[k];
        animationMutex.Unlock();

		if (animation->state & Animation::STATE_IN_PROGRESS)
		{
			if(!(animation->state & Animation::STATE_PAUSED))
			{
				animation->Update(timeElapsed);
			}	
		}
        
        animationMutex.Lock();
	}

	// process all finish callbacks
    size = (uint32)animations.size();
	for (uint32 k = 0; k < size; ++k)
	{
		Animation * animation = animations[k];
        animationMutex.Unlock();

		if (animation->state & Animation::STATE_FINISHED)
		{
			//#ifdef ANIMATIONS_DEBUG
			//			if(animationLoggerEnabled)
			//			{
			//				Logger::FrameworkDebug("ANIMATION LOGGER: AnimationManager::Finishing animation 0x%x    for owner 0x%x", (int)animation, (int)animation->owner);
			//				Logger::FrameworkDebug("ANIMATION LOGGER: AnimationManager::Finishing index %d", k);
			//			}
			//#endif
			animation->Stop(); 
		}
        animationMutex.Lock();
	}

	//remove all old animations
    size = (uint32)animations.size();
	for (uint32 k = 0; k < size; ++k)
	{
		Animation * animation = animations[k];
        animationMutex.Unlock();

		if (animation->state & Animation::STATE_DELETE_ME)
		{
			if (!(animation->state & Animation::STATE_FINISHED))
			{
				animation->OnCancel();
			}

//#ifdef ANIMATIONS_DEBUG
//			if(animationLoggerEnabled)
//			{
//				Logger::FrameworkDebug("ANIMATION LOGGER: AnimationManager::Deleting animation 0x%x    for owner 0x%x", (int)animation, (int)animation->owner);
//				Logger::FrameworkDebug("ANIMATION LOGGER: AnimationManager::Finishing index %d", k);
//			}
//#endif
			if(animation->next && !(animation->next->state  & Animation::STATE_DELETE_ME))
			{
//#ifdef ANIMATIONS_DEBUG
//				if(animationLoggerEnabled)
//				{
//					Logger::FrameworkDebug("ANIMATION LOGGER: AnimationManager::Starting next animation 0x%x    for owner 0x%x", (int)animation->next, (int)animation->next->owner);
//				}
//#endif
				animation->next->state |= Animation::STATE_IN_PROGRESS;
				animation->next->OnStart();
			}
//#ifdef ANIMATIONS_DEBUG
//			if(animationLoggerEnabled)
//			{
//				for (int n = 0; n < (int)animations.size(); ++n)
//				{
//					if(animations[n]->next == animation)
//					{
//						Logger::Error("PIZDEC");
//						Logger::FrameworkDebug("ANIMATION LOGGER: AnimationManager::Animation 0x%x (%d) used as next for 0x%x (%d)   for owner 0x%x", (int)animation, k, animations[n], n, (int)animation->owner);
//					}
//				}
//			}
//#endif

			SafeRelease(animation);

            animationMutex.Lock();
            
            size = (uint32)animations.size();
            k--;
            
//#ifdef ANIMATIONS_DEBUG
//			if(animationLoggerEnabled)
//			{
//				Logger::FrameworkDebug("ANIMATION LOGGER: AnimationManager::After Deleting index %d", k);
//			}
//#endif
            
		}
	}
	animationMutex.Unlock();
}
	
void AnimationManager::DumpState()
{
	Logger::FrameworkDebug("============================================================");
	Logger::FrameworkDebug("------------ Currently allocated animations - %2d ---------", animations.size());
	animationMutex.Lock();
	for (int k = 0; k < (int)animations.size(); ++k)
	{
		Animation * animation = animations[k];  

        String ownerName = "no owner";
        if (animation->owner)
            ownerName = typeid(*animation->owner).name();
		Logger::FrameworkDebug("addr:0x%08x state:%d class: %s ownerClass: %s", animation, animation->state, typeid(*animation).name(), ownerName.c_str());
	}
	animationMutex.Unlock();
	Logger::FrameworkDebug("============================================================");
}


void AnimationManager::PauseAnimations(bool isPaused, int tag)
{
	animationMutex.Lock();
	for(Vector<Animation*>::iterator i = animations.begin(); i != animations.end(); ++i)
    {
        Animation * &a = *i;
        animationMutex.Unlock();
        
        if (a->GetTagId() == tag)
        {
            a->Pause(isPaused);
        }
        animationMutex.Lock();
    }
	animationMutex.Unlock();
}

void AnimationManager::SetAnimationsMultiplier(float32 f, int tag)
{
	animationMutex.Lock();
    for(Vector<Animation*>::iterator i = animations.begin(); i != animations.end(); ++i)
    {
        Animation * &a = *i;
        animationMutex.Unlock();
        
        if (a->GetTagId() == tag)
        {
            a->SetTimeMultiplier(f);
        }
        animationMutex.Lock();
    }
	animationMutex.Unlock();
}

};