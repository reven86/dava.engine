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



#ifndef __DAVAENGINE_ANIMATION_COMPONENT_H__
#define __DAVAENGINE_ANIMATION_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Scene3D/Systems/AnimationSystem.h"
#include "Entity/Component.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Base/Message.h"

namespace DAVA 
{

class AnimationData;

class AnimationComponent : public Component
{
protected:
	virtual ~AnimationComponent();
public:
	AnimationComponent();

	IMPLEMENT_COMPONENT_TYPE(ANIMATION_COMPONENT);

	virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SerializationContext *serializationContext);
	virtual void Deserialize(KeyedArchive *archive, SerializationContext *serializationContext);
	virtual void GetDataNodes(Set<DataNode*> & dataNodes);

	void SetAnimation(AnimationData* animation);

 	bool GetIsPlaying() const;
 	void SetIsPlaying(bool value);

    void Start();
    void Stop();
    void StopAfterNRepeats(int32 numberOfRepeats);    

    enum eState
    {
        STATE_PLAYING,  
        STATE_PAUSED,   
        STATE_STOPPED   
    };

private:

	friend class AnimationSystem;
    friend class TransformSystem;
	AnimationData* animation;
	float32 time;
	uint32 frameIndex;
    uint32 repeatsCount;
    uint32 currRepeatsCont;
    eState state;	

    /*completion message stuff*/	
    Message playbackComplete;		

    Matrix4 animationTransform;
public:

	INTROSPECTION_EXTEND(AnimationComponent, Component,
        MEMBER(repeatsCount, "repeatsCount", I_VIEW | I_EDIT | I_SAVE)
       	PROPERTY("isPlaying", "isPlaying", GetIsPlaying, SetIsPlaying, I_SAVE | I_EDIT | I_VIEW)
	);
};



};

#endif //__DAVAENGINE_ANIMATION_COMPONENT_H__
