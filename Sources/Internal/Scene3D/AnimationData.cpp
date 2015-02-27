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


#include "Scene3D/AnimationData.h"
#include "Scene3D/SceneNodeAnimationList.h"

namespace DAVA 
{


AnimationData::AnimationData()
{
	
}

AnimationData::~AnimationData()
{
	
}
	
void AnimationData::AddKey(const SceneNodeAnimationKey & key)
{
	keys.push_back(key);
}

SceneNodeAnimationKey AnimationData::Interpolate(float32 t, uint32& startIdxCache) const
{
	if (keys.size() == 1)
	{
		return keys[0];
	}
	
	if (t < keys[startIdxCache].time)
	{
		startIdxCache = 0;
	}
	
	uint32 endIdx = 0;
	for (endIdx = startIdxCache; endIdx < keys.size(); ++endIdx)
	{
		if (keys[endIdx].time > t)
		{
			break;
		}
		startIdxCache = endIdx;
	}
	
	if (endIdx == keys.size())
	{
		endIdx = 0;
	}
	
	const SceneNodeAnimationKey & key1 = keys[startIdxCache];
	const SceneNodeAnimationKey & key2 = keys[endIdx];

	float32 tInter;
	if (endIdx > startIdxCache)
		tInter = (t - key1.time) / (key2.time - key1.time);
	else // interpolate from last to first
		tInter = (t - key1.time) / (duration - key1.time);

	SceneNodeAnimationKey result;
	result.translation.Lerp(key1.translation, key2.translation, tInter);
	result.rotation.Slerp(key1.rotation, key2.rotation, tInter);
	result.scale.Lerp(key1.scale, key2.scale, tInter);

	return result;
}

void AnimationData::SetDuration(float32 _duration)
{
	duration = _duration;
}
	
void AnimationData::SetInvPose(const Matrix4& mat)
{
	invPose = mat;
}

const Matrix4& AnimationData::GetInvPose() const
{
	return invPose;
}

void AnimationData::Save(KeyedArchive * archive, SerializationContext * serializationContext)
{
	DataNode::Save(archive, serializationContext);

	archive->SetFloat("duration", duration);
	archive->SetInt32("keyCount", static_cast<int32>(keys.size()));
	archive->SetMatrix4("invPose", invPose);

	for (uint32 keyIndex = 0; keyIndex < keys.size(); ++keyIndex)
	{
		archive->SetFloat(Format("key_%i_time", keyIndex), keys[keyIndex].time);
		archive->SetVector3(Format("key_%i_translation", keyIndex), keys[keyIndex].translation);
		archive->SetVector3(Format("key_%i_scale", keyIndex), keys[keyIndex].scale);
		archive->SetVector4(Format("key_%i_rotation", keyIndex), Vector4(keys[keyIndex].rotation.x, keys[keyIndex].rotation.y, keys[keyIndex].rotation.z, keys[keyIndex].rotation.w));
	}
}

void AnimationData::Load(KeyedArchive * archive, SerializationContext * serializationContext)
{
	DataNode::Load(archive, serializationContext);

	const int32 keyCount = archive->GetInt32("keyCount");
	keys.resize(keyCount);
	
	SetDuration(archive->GetFloat("duration"));
	SetInvPose(archive->GetMatrix4("invPose"));

	for (int32 keyIndex = 0; keyIndex < keyCount; ++keyIndex)
	{
		keys[keyIndex].time = archive->GetFloat(Format("key_%i_time", keyIndex));
		keys[keyIndex].translation = archive->GetVector3(Format("key_%i_translation", keyIndex));
		keys[keyIndex].scale = archive->GetVector3(Format("key_%i_scale", keyIndex));
		Vector4 rotation = archive->GetVector4(Format("key_%i_rotation", keyIndex));
		keys[keyIndex].rotation = Quaternion(rotation.x, rotation.y, rotation.z, rotation.w);
	}
}
	
AnimationData* AnimationData::Clone() const
{
	AnimationData* copy = new AnimationData();

	copy->invPose = invPose;
	copy->duration = duration;
	copy->keys = keys;

	return copy;
}

}
