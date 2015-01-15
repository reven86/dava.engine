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


/*
 *  TestScreen.h
 *  TemplateProjectMacOS
 *
 *  Created by Vitaliy  Borodovsky on 3/21/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef __PROPERTY_LINE_EDIT_CONTROL_H__
#define __PROPERTY_LINE_EDIT_CONTROL_H__

#include "DAVAEngine.h"

using namespace DAVA;


class GridControl : public UIControl
{
protected:
	~GridControl();
public:	
	GridControl();
	
	virtual void WillAppear();
	virtual void Update(float32 timeElapsed);
	virtual void Draw(const UIGeometricData &geometricData);
};

class PropertyLineEditControl : public UIControl
{
protected:
	~PropertyLineEditControl();
public:
	PropertyLineEditControl();
	
	
	virtual void Update(float32 timeElapsed);
	virtual void Draw(const UIGeometricData &geometricData);
	
	virtual void Input(UIEvent * touch);

	
	class PropertyRect
	{
	public:
		PropertyRect(float32 _x, float32 _y)
		{
			x = _x;
			y = _y;
		}
		
		float32 x;
		float32 y;
		Rect GetRect();
	};
	
	
private:
	//UITextField * textField[10];
	
	Vector2 CalcRealPosition(const PropertyRect & rect, bool absolute = true);
	Rect RectFromPosition(const Vector2 & pos);
	void FromMouseToPoint(Vector2 vec, PropertyRect & rect);
	const Rect & GetWorkZone();
	int32 FindActiveValueFromPosition(const Vector2 & absolutePosition);
	int32 FindValueForInsertion(const Vector2 & absolutePosition);
	
	Rect workZone;
	
	float32 minX;
	float32 minY;
	float32 maxX;
	float32 maxY;
	
	float32 windowX;
	float32 windowY;
	
	int32 activeValueIndex;
	
	Vector<PropertyRect> values;
};

#endif // __PROPERTY_LINE_EDIT_CONTROL_H__