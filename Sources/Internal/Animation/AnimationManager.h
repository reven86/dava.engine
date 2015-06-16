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


#ifndef __DAVAENGINE_ANIMATIONMANAGER_H__
#define __DAVAENGINE_ANIMATIONMANAGER_H__

//#define ANIMATIONS_DEBUG

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Scene2D/GameObject.h"
#include "Animation/Animation.h"


namespace DAVA
{
	
/**
	\ingroup animationsystem
	\brief AnimationManager is the heart of our animation systems. It does all groundwork. 
	You do not need it at all and SDK do everything to process animations in background. 
*/
class AnimationManager : public Singleton <AnimationManager>
{
public:
	/**
		\brief Updates all animations in the system on current frame
		This method is called from ApplicationCore::Update function. 
		If you override this function you should call this method in your update or animations will not work correctly. 
		\param[in] timeElapsed time passed from previous frame
	 */
	void Update(float32 timeElapsed);
	
	// void DebugRender();
	/**
		\brief Dump animations state to console
	 */
	void DumpState();
	
	Animation * FindPlayingAnimation(AnimatedObject * owner, int32 _groupId);
	
    void StopAnimations();
	void PauseAnimations(bool isPaused, int32 tag = 0);
	void SetAnimationsMultiplier(float32 f, int32 tag = 0);
private:
	Animation * FindLastAnimation(AnimatedObject * owner, int32 _groupId);
	bool IsAnimating(const AnimatedObject * owner, int32 trackId) const;
	
	void AddAnimation(Animation * _animation);
	void AddAnimationInternal(Animation * animation);

    void RemoveAnimation(Animation * _animation);
	void RemoveAnimationInternal(Animation * animation);
	
	bool HasActiveAnimations(AnimatedObject * owner) const;
	/*
	 Function remove all animations for given object from update and delete objects and their references
	 */
	void DeleteAnimations(AnimatedObject * _owner, int32 track = -1);
	void DeleteAnimationInternal(AnimatedObject * owner, int32 track);
    struct DeleteAnimationsData
    {
        AnimatedObject * owner;
        int32 track;
    };
	
	Vector<Animation*> animations;
    Vector<Animation*> releaseCandidates;
	
	friend class Animation;
	friend class AnimatedObject;
};
	
};

#endif // __DAVAENGINE_ANIMATIONMANAGER_H__