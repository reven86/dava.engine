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


#ifndef SCROLLVIEWPROPERTYGRIDWIDGET_H
#define SCROLLVIEWPROPERTYGRIDWIDGET_H

#include <QWidget>
#include "basepropertygridwidget.h"

namespace Ui {
class ScrollViewPropertyGridWidget;
}

class ScrollViewPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT

public:
    explicit ScrollViewPropertyGridWidget(QWidget *parent = 0);
    ~ScrollViewPropertyGridWidget();

    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();

private:
    Ui::ScrollViewPropertyGridWidget *ui;
	
	void UpdateMaximumValue();

protected:
    // Connect/disconnect to the signals.
    void ConnectToSignals();
	
	virtual void OnPropertiesChangedFromExternalSource();
	virtual void HandleChangePropertySucceeded(const QString& propertyName);

    // Change value of double spin box
    virtual void ProcessDoubleSpinBoxValueChanged(QDoubleSpinBox *doubleSpinBoxWidget, const PROPERTYGRIDWIDGETSITER &iter,
                                                               const double value);
    // Update of internal propeperties
    virtual void UpdateDoubleSpinBoxWidgetWithPropertyValue(QDoubleSpinBox *doubleSpinBoxWidget,
                                                                const QMetaProperty& curProperty);
private slots:
    // Use this slot to update value on Horizontal or Vertical Scroll position spin according to appropritate slider's position
    void OnSliderValueChanged(int);
};

#endif // SCROLLVIEWPROPERTYGRIDWIDGET_H
