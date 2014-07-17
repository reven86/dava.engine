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


#ifndef CONTROLPROPERTYGRIDWIDGET_H
#define CONTROLPROPERTYGRIDWIDGET_H

#include "BaseMetadata.h"

#include <QWidget>
#include "basepropertygridwidget.h"

namespace Ui {
class ControlPropertyGridWidget;
}

class ControlPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT
    
public:
	// Control widget states - whether we are displaying info for Default or Custom controls.
	enum eWidgetState
	{
		STATE_DEFAULT_CONTROL,
		STATE_CUSTOM_CONTROL
	};

    explicit ControlPropertyGridWidget(QWidget *parent = 0);
    ~ControlPropertyGridWidget();

    // Initialize with control and metadata.
    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();

protected:
	void ConnectToSignals();
	void DisconnectFromSignals();

	// Change the state of the widget.
	void SetWidgetState(eWidgetState newState, bool forceUpdate = false);

	// Update the properties for Subcontrols.
	void UpdatePropertiesForSubcontrol();

	// Override this method to setup the correct state of the control.
	virtual void UpdateLineEditWidgetWithPropertyValue(QLineEdit* lineEditWidget,
													   const QMetaProperty& curProperty);
    
    bool eventFilter(QObject *object, QEvent *event);
protected slots:
	void OnMorphToCustomControlClicked();
	void OnResetMorphToCustomControlClicked();

private:
    Ui::ControlPropertyGridWidget *ui;
	
	// Current widget state.
	eWidgetState currentWidgetState;
};

#endif // CONTROLPROPERTYGRIDWIDGET_H
