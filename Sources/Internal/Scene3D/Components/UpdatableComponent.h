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


#ifndef __DAVAENGINE_UPDATABLE_COMPONENT_H__
#define __DAVAENGINE_UPDATABLE_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"

namespace DAVA
{

class BaseObject;

class IUpdatable
{
public:
	virtual ~IUpdatable() {};
};

class IUpdatableBeforeTransform : public virtual IUpdatable
{
public:
	virtual void UpdateBeforeTransform(float32 timeElapsed) = 0;
};

class IUpdatableAfterTransform : public virtual IUpdatable
{
public:
	virtual void UpdateAfterTransform(float32 timeElapsed) = 0;
};

class UpdatableComponent : public Component
{
public:
	IMPLEMENT_COMPONENT_TYPE(UPDATABLE_COMPONENT);

	enum eUpdateType
	{
		UPDATE_PRE_TRANSFORM,
		UPDATE_POST_TRANSFORM,

		UPDATES_COUNT
	};
protected:
    ~UpdatableComponent(){};
public:
	UpdatableComponent();
	virtual Component * Clone(Entity * toEntity);

	void SetUpdatableObject(IUpdatable * updatableObject);
	IUpdatable * GetUpdatableObject();

private:
	IUpdatable * updatableObject;
    
public:
    INTROSPECTION_EXTEND(UpdatableComponent, Component,
        MEMBER(updatableObject, "Updatable Object", I_SAVE)
    );

};

}

#endif //__DAVAENGINE_UPDATABLE_COMPONENT_H__