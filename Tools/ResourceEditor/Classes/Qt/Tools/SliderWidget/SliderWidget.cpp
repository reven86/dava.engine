/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "SliderWidget.h"
#include "PopupEditorDialog.h"

#include <QLayout>
#include <QSlider>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>

#include <QEvent>

#include "DAVAEngine.h"
#include "StringConstants.h"

const int SliderWidget::DEF_LOWEST_VALUE = -999;
const int SliderWidget::DEF_HIGHEST_VALUE = 999;

SliderWidget::SliderWidget(QWidget* parent)
: QWidget(parent)
, isRangeChangingBlocked(true)
, rangeBoundMin(DEF_LOWEST_VALUE)
, rangeBoundMax(DEF_HIGHEST_VALUE)
{
	InitUI();
	Init(true, 10, 0, 0);
	SetRangeVisible(true);
	SetRangeChangingBlocked(false);

	ConnectToSignals();
}

SliderWidget::~SliderWidget()
{
}

void SliderWidget::InitUI()
{
	QHBoxLayout* layout = new QHBoxLayout();
	labelMinValue = new QLabel(this);
	labelMaxValue = new QLabel(this);
	sliderValue = new QSlider(this);
	spinCurValue = new QSpinBox(this);

	layout->addWidget(labelMinValue);
	layout->addWidget(sliderValue);
	layout->addWidget(labelMaxValue);
	layout->addWidget(spinCurValue);

	setLayout(layout);

	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(3);
	sliderValue->setOrientation(Qt::Horizontal);
	sliderValue->setTickPosition(QSlider::TicksBothSides);
	spinCurValue->setToolTip(ResourceEditor::SLIDER_WIDGET_CURRENT_VALUE.c_str());
}

void SliderWidget::ConnectToSignals()
{
	connect(sliderValue, SIGNAL(valueChanged(int)), this, SLOT(SliderValueChanged(int)));
	connect(spinCurValue, SIGNAL(valueChanged(int)), this, SLOT(SpinValueChanged(int)));
}

void SliderWidget::Init(bool symmetric, int max, int min, int value)
{
	isSymmetric = symmetric;
	currentValue = value;
	SetRange(min, max);

	UpdateControls();
}

void SliderWidget::SetRange(int min, int max)
{
	if (min > max)
	{
		qSwap(max, min);
	}

	maxValue = qMin(max, rangeBoundMax);
	minValue = qMax(min, rangeBoundMin);
	if (IsSymmetric())
	{
		minValue = -max;
		if (min > max)
		{
			qSwap(max, min);
		}
	}

	if (currentValue > maxValue)
	{
		SetValue(maxValue);
	}
	else if (currentValue < minValue)
	{
		SetValue(minValue);
	}
}

void SliderWidget::SetRangeMax(int max)
{
	SetRange(minValue, max);
	UpdateControls();
}

int SliderWidget::GetRangeMax()
{
	return maxValue;
}

void SliderWidget::SetRangeMin(int min)
{
	SetRange(min, maxValue);
	UpdateControls();
}

int SliderWidget::GetRangeMin()
{
	return minValue;
}

void SliderWidget::SetSymmetric(bool symmetric)
{
	isSymmetric = symmetric;

	if (IsSymmetric())
	{
	}

	SetRangeBoundaries(rangeBoundMin, rangeBoundMax);
	UpdateControls();
}

bool SliderWidget::IsSymmetric()
{
	return isSymmetric;
}

void SliderWidget::SetValue(int value)
{
	if (value == currentValue)
	{
		return;
	}

	value = qMax(minValue, value);
	value = qMin(maxValue, value);

	currentValue = value;
	EmitValueChanged();
	UpdateControls();
}

int SliderWidget::GetValue()
{
	return currentValue;
}

void SliderWidget::SliderValueChanged(int newValue)
{
	SetValue(newValue);
}

void SliderWidget::RangeChanged(int newMinValue, int newMaxValue)
{
	SetRange(newMinValue, newMaxValue);
}

void SliderWidget::EmitValueChanged()
{
	sliderValue->setToolTip(QString::number(currentValue));
	emit ValueChanged(currentValue);
}

void SliderWidget::UpdateControls()
{
	bool blocked = signalsBlocked();
	blockSignals(true);

	labelMinValue->setNum(GetRangeMin());
	labelMaxValue->setNum(GetRangeMax());

	spinCurValue->setRange(GetRangeMin(), GetRangeMax());
	spinCurValue->setValue(GetValue());

	sliderValue->setRange(GetRangeMin(), GetRangeMax());
	sliderValue->setValue(GetValue());

	blockSignals(blocked);
}

void SliderWidget::SetRangeChangingBlocked(bool blocked)
{
	if (blocked == isRangeChangingBlocked)
	{
		return;
	}

	if (blocked)
	{
		labelMinValue->removeEventFilter(this);
		labelMaxValue->removeEventFilter(this);
		labelMinValue->setToolTip("");
		labelMaxValue->setToolTip("");
	}
	else
	{
		labelMinValue->installEventFilter(this);
		labelMaxValue->installEventFilter(this);
		labelMinValue->setToolTip(ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_TOOLTIP.c_str());
		labelMaxValue->setToolTip(ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_TOOLTIP.c_str());
	}

	isRangeChangingBlocked = blocked;
}

bool SliderWidget::IsRangeChangingBlocked()
{
	return isRangeChangingBlocked;
}

void SliderWidget::SpinValueChanged(int newValue)
{
	SetValue(spinCurValue->value());
}

bool SliderWidget::eventFilter(QObject* obj, QEvent* ev)
{
	if (obj == labelMinValue || obj == labelMaxValue)
	{
		if (ev->type() == QEvent::MouseButtonDblClick)
		{
			int val = (obj == labelMinValue) ? minValue : maxValue;

			QLabel* label = (QLabel*)obj;
			PopupEditorDialog* dialog = new PopupEditorDialog(val, rangeBoundMin, rangeBoundMax, label, this);

			QPoint pos = label->mapToGlobal(QPoint(0, 0));
			dialog->move(pos.x(), pos.y());
			dialog->setMinimumHeight(label->height());

			connect(dialog, SIGNAL(ValueReady(const QWidget*, int)),
					this, SLOT(OnValueReady(const QWidget*, int)));

			dialog->show();
			//dialog will self-delete after close

			return true;
		}
	}

	return QObject::eventFilter(obj, ev);
}

void SliderWidget::OnValueReady(const QWidget *widget, int value)
{
	QLabel* label = (QLabel*)widget;
	label->setNum(value);

	int min = minValue;
	int max = maxValue;
	if (label == labelMinValue)
	{
		min = qMax(value, DEF_LOWEST_VALUE);
		if (IsSymmetric())
		{
			max = -min;
		}
	}
	else if (label == labelMaxValue)
	{
		max = qMin(value, DEF_HIGHEST_VALUE);
	}
	SetRange(min, max);
	UpdateControls();
}

void SliderWidget::SetRangeVisible(bool visible)
{
	if (visible == isRangeVisible)
	{
		return;
	}

	labelMaxValue->setVisible(visible);
	labelMinValue->setVisible(visible);

	isRangeVisible = visible;
}

bool SliderWidget::IsRangeVisible()
{
	return isRangeVisible;
}

void SliderWidget::SetRangeBoundaries(int min, int max)
{
	if (IsSymmetric())
	{
		min = -max;
	}

	if (min > max)
	{
		qSwap(min, max);
	}

	rangeBoundMax = max;
	rangeBoundMin = min;

	SetRange(minValue, maxValue);
	UpdateControls();
}

void SliderWidget::SetCurValueVisible(bool visible)
{
	spinCurValue->setVisible(visible);
}

bool SliderWidget::IsCurValueVisible()
{
	return spinCurValue->isVisible();
}
