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


#include "HangingObjectsHeight.h"

#include "Tools/EventFilterDoubleSpinBox/EventFilterDoubleSpinBox.h"

#include "DAVAEngine.h"

#include <QObject>
#include <QHBoxLayout>
#include <QLabel>

using namespace DAVA;

HangingObjectsHeight::HangingObjectsHeight(QWidget *parent /*= 0*/)
	: QWidget(parent)
{
	heightValue = new EventFilterDoubleSpinBox(this);
	heightValue->setToolTip("Min height for hanging objects");
	heightValue->setMinimum(-100);
	heightValue->setMaximum(100);	
	heightValue->setSingleStep(0.01);
	heightValue->setDecimals(4);

	QLabel *caption = new QLabel("Min height:", this);

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->setContentsMargins(0, 0, 0, 0);

	setLayout(layout);

	layout->addWidget(caption);
	layout->addWidget(heightValue);


	QObject::connect(heightValue, SIGNAL(valueChanged(double)), this, SLOT(ValueChanged(double)));
}

void HangingObjectsHeight::SetHeight( DAVA::float32 value )
{
	heightValue->setValue(value);
}

void HangingObjectsHeight::ValueChanged( double value )
{
	emit HeightChanged(value);
}



