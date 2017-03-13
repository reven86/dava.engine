#include "Animation/AnimatedObject.h"
#include "Animation/AnimationManager.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(AnimatedObject)
{
    ReflectionRegistrator<AnimatedObject>::Begin()
    .End();
}

AnimatedObject::AnimatedObject()
//	: animationsStorage(0)
{
}

AnimatedObject::~AnimatedObject()
{
    StopAnimations();
}

void AnimatedObject::StopAnimations(int32 track)
{
    AnimationManager::Instance()->DeleteAnimations(this, track);
}

bool AnimatedObject::IsAnimating(int32 track) const
{
    return AnimationManager::Instance()->IsAnimating(this, track);
}

Animation* AnimatedObject::FindPlayingAnimation(int32 track /*= -1*/)
{
    return AnimationManager::Instance()->FindPlayingAnimation(this, track);
}
}

/*
void AnimatedObject::AddAnimation(Animation * animation)
{
	
}
void RemoveAnimation(Animation * animation);
{
	
}*/
/*virtual Animation * FindNextAnimationForTrack(int track)
{

}

void Animation::Update()
{
	for (int k = 0; k < MAX_NUMBER_OF_TRACKS; ++k)
	{
		
		
	}
}*/