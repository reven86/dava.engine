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


#include "Scene3D/SceneNodeAnimation.h"
#include "Scene3D/SceneNodeAnimationList.h"

namespace DAVA 
{


SceneNodeAnimation::SceneNodeAnimation(int32 _keyCount)
{
    TAG_SWITCH(MemoryManager::TAG_SCENE)
    
	keyCount = _keyCount;
	bindNode = 0;
	startIdx = 0;
	keys = new SceneNodeAnimationKey[keyCount];
	apply = true;
	weight = 0.0f;
	delayTime = 0.0f;
	parent = 0;
}

SceneNodeAnimation::~SceneNodeAnimation()
{
    TAG_SWITCH(MemoryManager::TAG_SCENE)
    
	SafeDeleteArray(keys);
}
	
void SceneNodeAnimation::SetKey(int32 index, const SceneNodeAnimationKey & key)
{
    TAG_SWITCH(MemoryManager::TAG_SCENE)
    
	keys[index] = key;
}

SceneNodeAnimationKey & SceneNodeAnimation::Intepolate(float32 t)
{
    TAG_SWITCH(MemoryManager::TAG_SCENE)
    
	if (keyCount == 1)
	{
		currentValue = keys[0];
		return currentValue;
	}
	
	if (t < keys[startIdx].time)
	{
		startIdx = 0;
	}
	
	int32 endIdx = 0;
	for (endIdx = startIdx; endIdx < keyCount; ++endIdx)
	{
		if (keys[endIdx].time > t)
		{
			break;
		}
		startIdx = endIdx;
	}
	
	if (endIdx == keyCount)
	{
		currentValue = keys[keyCount - 1];
		return currentValue;
	}
	
	SceneNodeAnimationKey & key1 = keys[startIdx];
	SceneNodeAnimationKey & key2 = keys[endIdx];

	float32 tInter = (t - key1.time) / (key2.time - key1.time);

	currentValue.translation.Lerp(key1.translation, key2.translation, tInter);
	currentValue.rotation.Slerp(key1.rotation, key2.rotation, tInter);
	//currentValue.matrix = key1.matrix;
	return currentValue;
}

void SceneNodeAnimation::SetDuration(float32 _duration)
{
    TAG_SWITCH(MemoryManager::TAG_SCENE)
    
	duration = _duration;
}
	
void SceneNodeAnimation::SetBindNode(Entity * _bindNode)
{
    TAG_SWITCH(MemoryManager::TAG_SCENE)
    
	bindNode = _bindNode;
}
	
void SceneNodeAnimation::SetBindName(const FastName & _bindName)
{
    TAG_SWITCH(MemoryManager::TAG_SCENE)
    
	bindName = _bindName;
}
	
void SceneNodeAnimation::Update(float32 timeElapsed)
{
    TAG_SWITCH(MemoryManager::TAG_SCENE)
    
	delayTime -= timeElapsed;
	if (delayTime <= 0.0f)
	{
		delayTime = 0.0f;
		currentTime += timeElapsed + delayTime;
		if (currentTime > duration)
		{
			currentTime = duration;
			//bindNode->DetachAnimation(this);
		}
	}
}
	
void SceneNodeAnimation::Execute()
{
    DVASSERT(0);
// 	startIdx = 0;
// 	currentTime = 0;
// 	bindNode->ExecuteAnimation(this);
}
	
Vector3 SceneNodeAnimation::SetStartPosition(const Vector3 & position)
{
    TAG_SWITCH(MemoryManager::TAG_SCENE)
    
	Vector3 sPos = keys[0].translation;
	for (int idx = 0; idx < keyCount; ++idx)
	{
		keys[idx].translation = position + keys[idx].translation - sPos;
	}
	return position - sPos;
}	

void SceneNodeAnimation::ShiftStartPosition(const Vector3 & shift)
{
    TAG_SWITCH(MemoryManager::TAG_SCENE)
    
	for (int idx = 0; idx < keyCount; ++idx)
	{
		keys[idx].translation += shift;
	}
}
	
void SceneNodeAnimation::SetParent(SceneNodeAnimationList * list)
{
    TAG_SWITCH(MemoryManager::TAG_SCENE)
    
	parent = list;
}

SceneNodeAnimationList * SceneNodeAnimation::GetParent()
{
    TAG_SWITCH(MemoryManager::TAG_SCENE)
    
	return parent;
}

}