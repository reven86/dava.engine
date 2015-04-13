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


#ifndef EDITFONTDIALOG_H
#define EDITFONTDIALOG_H

#include <QDialog>

#include <QSpinBox>
#include <QPushButton>
#include <QRadioButton>
#include <QLineEdit>
#include <QComboBox>

#include "Base/BaseTypes.h"

namespace Ui {
class EditFontDialog;
}
class ControlNode;

class EditFontDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditFontDialog(QWidget *parent = 0);
    ~EditFontDialog();
    void UpdateFontPreset(ControlNode *selectedControlNode);
    DAVA::String EditFontDialog::findFont(ControlNode *node);
private:
    Ui::EditFontDialog *ui;
    
    //ChangeFontPropertyCommandData dialogResult;
    DAVA::String currentLocale;
    
    void ConnectToSignals();
    void DisconnectFromSignals();
    
    virtual void ProcessComboBoxValueChanged(QComboBox *senderWidget, const QString& value);
    virtual void ProcessPushButtonClicked(QPushButton *senderWidget);
    
    void UpdateDefaultFontParams();
    void UpdateLocalizedFontParams();
    
    void UpdateLineEditWidgetWithPropertyValue(QLineEdit *lineEditWidget);
    void UpdatePushButtonWidgetWithPropertyValue(QPushButton *pushButtonWidget);
    void UpdateSpinBoxWidgetWithPropertyValue(QSpinBox *spinBoxWidget);
    void UpdateComboBoxWidgetWithPropertyValue(QComboBox *comboBoxWidget);

private slots:
    void OnOkButtonClicked();
    void OnPushButtonClicked();
    void OnSpinBoxValueChanged(int newValue);
    void OnComboBoxValueChanged(QString value);
};

#endif // EDITFONTDIALOG_H
