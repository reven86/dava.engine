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




#ifndef __DAVAENGINE_CAMERA_CONTROLLER_H__
#define __DAVAENGINE_CAMERA_CONTROLLER_H__

#include "DAVAEngine.h"

namespace DAVA 
{
class CameraController : public BaseObject
{
protected:
    ~CameraController();
public:
    CameraController(float32 newSpeed);
    
    virtual void SetScene(Scene *scene);
    virtual void Input(UIEvent * event);
    
    virtual void Update(float32 timeElapsed) {};

    inline void SetSelection(Entity * _selection)
	{
		selection = _selection;
	}

    void SetSpeed(float32 newSpeed);
    inline float32 GetSpeed() { return speed; };

protected:
    Scene * currScene;
    Entity *selection;
    float32 speed;
};
    
class WASDCameraController : public CameraController
{
protected:
    ~WASDCameraController();
public:
    WASDCameraController(float32 newSpeed);
    
    virtual void Input(UIEvent * event);
    
    virtual void Update(float32 timeElapsed);
	virtual void SetScene(Scene *_scene);

    void LookAtSelection();
    
    
protected:
    float32 viewZAngle, viewYAngle;
    Vector2 oldTouchPoint;
	
	Vector2 startPt;
    Vector2 stopPt;

	void UpdateAngels(Camera * camera);
	void UpdateCamAlt3But(Camera * camera);
	void UpdateCam3But(Camera * camera);
	void UpdateCam2But(Camera * camera);
	    
    float32 radius;
	Vector3 center;
	Camera * lastCamera;
    
};

};


#endif // __DAVAENGINE_CAMERA_CONTROLLER_H__