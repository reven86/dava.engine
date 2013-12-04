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



#ifndef __LODDISTANCE_CONTROL_H__
#define __LODDISTANCE_CONTROL_H__

#include "DAVAEngine.h"

using namespace DAVA;

class LodDistanceControl;
class LodDistanceControlDelegate
{
public:
    
    virtual void DistanceChanged(LodDistanceControl *forControl, int32 index, float32 value) = 0;
};

class LodDistanceControl : public UIControl, public UITextFieldDelegate
{
protected:
    virtual ~LodDistanceControl();
public:

	LodDistanceControl(LodDistanceControlDelegate *newDelegate, const Rect &rect, bool rectInAbsoluteCoordinates = false);

    virtual void WillDisappear();
    
    virtual void Update(float32 timeElapsed);

    void SetDistances(float32 *newDistances, int32 *newTriangles, int32 newCount);
    
    virtual void Input(UIEvent *currentInput);
    
    //UITextFieldDelegate
    virtual void TextFieldShouldReturn(UITextField * textField);
    virtual void TextFieldShouldCancel(UITextField * textField);
    virtual void TextFieldLostFocus(UITextField * textField);
	virtual bool TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, 
                                     int32 replacementLength, const WideString & replacementString);
    
    
    static const float32 GetControlHeightForLodCount(int32 lodCount);
    
private:

    void ReleaseControls();
    void UpdateDistanceToCamera();
    
    UIControl **zones;
    UIControl **sliders;
    
    int32 count;
    float32 distances[LodComponent::MAX_LOD_LAYERS];
    int32 triangles[LodComponent::MAX_LOD_LAYERS];
    
    float32 maxDistance;

    LodDistanceControlDelegate *delegate;
    
    int32 mainTouch;
    float32 oldPos;
	float32 newPos;
    
    UIControl *activeSlider;
    void UpdateSliderPos();
    
    Vector2 leftEdge;
    Vector2 rightEdge;
    
    UIControl *leftZone;
    UIControl *rightZone;
    
    UIStaticText *distanceText[LodComponent::MAX_LOD_LAYERS];
    UITextField *distanceTextValues[LodComponent::MAX_LOD_LAYERS];

    UIStaticText *trianglesText[LodComponent::MAX_LOD_LAYERS];
    UIStaticText *trianglesTextValues[LodComponent::MAX_LOD_LAYERS];

    UIStaticText *distanceToCameraText;
    UIStaticText *distanceToCameraValue;
    
    int32 activeLodIndex;
};


#endif //#ifndef __LODDISTANCE_CONTROL_H__
