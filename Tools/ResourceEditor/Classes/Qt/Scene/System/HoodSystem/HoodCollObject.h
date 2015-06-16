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


#ifndef __HOOD_COLLISION_OBJECT_H__
#define __HOOD_COLLISION_OBJECT_H__

#include "Scene/SceneTypes.h"

// bullet
#include "bullet/btBulletCollisionCommon.h"

// framework
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"

struct HoodCollObject
{
	HoodCollObject();
	~HoodCollObject();

	btCollisionObject* btObject;
	btCollisionShape* btShape;

	DAVA::Vector3 baseFrom;
	DAVA::Vector3 baseTo;
	DAVA::Vector3 baseOffset;
	DAVA::Matrix4 baseRotate;
	DAVA::Vector3 scaledOffset;

	DAVA::Vector3 curFrom;
	DAVA::Vector3 curTo;

	DAVA::Vector3 curPos;
	DAVA::float32 curScale;

	ST_Axis axis;

	void UpdatePos(const DAVA::Vector3 &pos);
	void UpdateScale(const DAVA::float32 &scale);
};

#endif // __HOOD_COLLISION_OBJECT_H__
