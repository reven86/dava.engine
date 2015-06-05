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


#include "Scene/System/HoodSystem/HoodCollObject.h"

HoodCollObject::HoodCollObject()
	 : btObject(NULL)
	 , btShape(NULL) 
	 , curScale(1.0f)
{ }

HoodCollObject::~HoodCollObject()
{
	if(NULL != btObject)
	{
		delete btObject;
	}

	if(NULL != btShape)
	{
		delete btShape;
	}
}

void HoodCollObject::UpdatePos(const DAVA::Vector3 &pos)
{
	curPos = pos;
	curFrom = (baseFrom * curScale) + curPos;
	curTo = (baseTo * curScale) + curPos;

	// move bullet object to new pos
	btTransform transf = btObject->getWorldTransform();
	transf.setOrigin(btVector3(scaledOffset.x + curFrom.x, scaledOffset.y + curFrom.y, scaledOffset.z + curFrom.z));
	btObject->setWorldTransform(transf);
}

void HoodCollObject::UpdateScale(const DAVA::float32 &scale)
{
	curScale = scale;

	btShape->setLocalScaling(btVector3(curScale, curScale, curScale));
	scaledOffset = DAVA::MultiplyVectorMat3x3(baseOffset * curScale, baseRotate);

	UpdatePos(curPos);
}
