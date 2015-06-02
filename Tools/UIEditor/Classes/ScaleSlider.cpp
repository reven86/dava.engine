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


#include "ScaleSlider.h"

#include <QMouseEvent>
#include <QStyle>
#include <QStyleOption>

#include "Math/MathHelpers.h"

ScaleSlider::ScaleSlider(QWidget *parent) :
	QSlider(parent)
{
}

ScaleSlider::ScaleSlider(Qt::Orientation orientation, QWidget *parent) :
	QSlider(orientation, parent)
{
}

void ScaleSlider::mousePressEvent (QMouseEvent* event)
{
	// Handle mouse click on the nearest position.
	QStyleOptionSlider opt;
	initStyleOption(&opt);
	QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

	if (event->button() == Qt::LeftButton && sr.contains(event->pos()) == false)
	{
		// Calculate in float to allow clicking between the ticks.
		float newValueRelative = minimum() + (float)((maximum()-minimum()) * event->x()) / (float)width();
		int newVal = DAVA::Round(newValueRelative);

		if (invertedAppearance() == true)
		{
		    setValue(maximum() - newVal);
		}
		else
		{
			setValue(newVal);
		}

		event->accept();
	}
	else
	{
		// Not ours, handle as-is.
		QSlider::mousePressEvent(event);
	}
}

