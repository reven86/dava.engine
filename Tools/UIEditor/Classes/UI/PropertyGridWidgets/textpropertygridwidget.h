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


#ifndef TEXTPROPERTYGRIDWIDGET_H
#define TEXTPROPERTYGRIDWIDGET_H

#include <QWidget>
#include "basepropertygridwidget.h"
#include "uitextfieldpropertygridwidget.h"

class QLabel;

class TextPropertyGridWidget : public UITextFieldPropertyGridWidget
{
    Q_OBJECT

public:
    explicit TextPropertyGridWidget(QWidget *parent = 0);
    ~TextPropertyGridWidget();

    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();

protected:
    
	virtual void InsertLocalizationFields();

    // Update the widget with Localization Value when the key is changed.
    virtual void UpdateLocalizationValue();

    virtual void HandleChangePropertySucceeded(const QString& propertyName);
    virtual void HandleChangePropertyFailed(const QString& propertyName);
    
    virtual void UpdateComboBoxWidgetWithPropertyValue(QComboBox* comboBoxWidget, const QMetaProperty& curProperty);
    virtual void ProcessComboboxValueChanged(QComboBox* senderWidget, const PROPERTYGRIDWIDGETSITER& iter,
                                             const QString& value);

	// Handle UI Control State is changed - needed for updating Localization Text.
    virtual void HandleSelectedUIControlStatesChanged(const Vector<UIControl::eControlState>& newStates);

    // Handle dependent checkboxes.
    virtual void UpdateCheckBoxWidgetWithPropertyValue(QCheckBox* checkBoxWidget, const QMetaProperty& curProperty);
	virtual void OnPropertiesChangedFromExternalSource() {};

private:
	QLineEdit *localizationKeyNameLineEdit;
    QLineEdit *localizationKeyTextLineEdit;
	QLabel	*localizationKeyNameLabel;
	QLabel	*localizationKeyTextLabel;
	QCheckBox *multilineCheckBox;
	QCheckBox *multilineBySymbolCheckBox;
};

#endif // TEXTPROPERTYGRIDWIDGET_H
