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


#ifndef __HOOD_OBJECT_H__
#define __HOOD_OBJECT_H__

#include "Scene/System/HoodSystem/HoodCollObject.h"
#include "Scene/SceneTypes.h"
#include "Render/RenderHelper.h"

#include "Render/UniqueStateSet.h"

class TextDrawSystem;

struct HoodObject 
{
	HoodObject(DAVA::float32 baseSize);
	virtual ~HoodObject();

	DAVA::float32 baseSize;
	DAVA::float32 objScale;
	DAVA::Color colorX; // axis X
	DAVA::Color colorY; // axis X
	DAVA::Color colorZ; // axis X
	DAVA::Color colorS; // axis selected

	virtual void UpdatePos(const DAVA::Vector3 &pos);
	virtual void UpdateScale(const DAVA::float32 &scale);
    virtual void Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, DAVA::RenderHelper* drawer, TextDrawSystem* textDrawSystem) = 0;

    HoodCollObject* CreateLine(const DAVA::Vector3 &from, const DAVA::Vector3 &to);
	DAVA::Rect DrawAxisText(TextDrawSystem *textDrawSystem, HoodCollObject *x, HoodCollObject *y, HoodCollObject *z);

	DAVA::Vector<HoodCollObject*> collObjects;

protected:
	DAVA::Vector3 GetAxisTextPos(HoodCollObject *axis);
};

#endif
