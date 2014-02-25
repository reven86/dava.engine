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


#ifndef BACKGROUNDPROPERTYGRIDWIDGET_H
#define BACKGROUNDPROPERTYGRIDWIDGET_H

#include <QWidget>

#include "basepropertygridwidget.h"
#include "StateComboBoxItemDelegate.h"

namespace Ui {
class BackGroundPropertyGridWidget;
}

class BackGroundPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT

public:
    explicit BackGroundPropertyGridWidget(QWidget *parent = 0);
    ~BackGroundPropertyGridWidget();

    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();

private:
    Ui::BackGroundPropertyGridWidget *ui;

protected:
    // Connect/disconnect to the signals.
    void ConnectToSignals();

	virtual void HandleChangePropertySucceeded(const QString& propertyName);

    // Background Control contains Comboboxes which should be processed in the specific way.
    virtual void ProcessComboboxValueChanged(QComboBox* senderWidget, const PROPERTYGRIDWIDGETSITER& iter,
                                             const QString& value);
    virtual void UpdateComboBoxWidgetWithPropertyValue(QComboBox* comboBoxWidget, const QMetaProperty& curProperty);
	
	virtual void OnPropertiesChangedFromExternalSource() {};

    // Fill the combos with appropriate values.
    void FillComboboxes();

    // Handler for the custom combobox values.
    void CustomProcessComboboxValueChanged(const PROPERTYGRIDWIDGETSITER& iter, int value);

    // Pre-process the sprite name.
    QString PreprocessSpriteName(const QString& rawSpriteName);

	//handle elements according drawType
	void HandleDrawTypeComboBox();
	
private:
	void SetStretchCapMaxValues();

private slots:
    void OpenSpriteDialog();
    void RemoveSprite();
};

#endif // BACKGROUNDPROPERTYGRIDWIDGET_H
