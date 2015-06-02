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


#include "editpreviewsettingsdialog.h"
#include "ui_editpreviewsettingsdialog.h"

#include "PreviewController.h"
#include "WidgetSignalsBlocker.h"

#include <QMessageBox>

using namespace DAVA;

EditPreviewSettingsDialog::EditPreviewSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditPreviewSettingsDialog),
    isReferenceValueDPI(false)
{
    ui->setupUi(this);
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(OnOKButtonClicked()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(OnCancelButtonClicked()));

    connect(ui->heightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnScreenHeightValueChanged(int)));
    connect(ui->widthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnScreenWidthValueChanged(int)));
    connect(ui->dpiSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnDPIValueChanged(int)));
    connect(ui->diagonalSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnDiagonalValueChanged(double)));
    
    connect(ui->dpiRadioButton, SIGNAL(clicked()), this, SLOT(OnReferenceValueSetToDPI()));
    connect(ui->diagonalRadioButton, SIGNAL(clicked()), this, SLOT(OnReferenceValueSetToDiagonal()));
    
    ui->dpiRadioButton->setChecked(true);
    OnReferenceValueSetToDPI();
}

EditPreviewSettingsDialog::~EditPreviewSettingsDialog()
{
    delete ui;
}

void EditPreviewSettingsDialog::OnOKButtonClicked()
{
    if (!Validate())
    {
        return;
    }
    
    resultData.id = 0;
    resultData.isPredefined = false;
    resultData.deviceName = ui->deviceNameLineEdit->text().toStdString();
    resultData.dpi = ui->dpiSpinBox->value();
    resultData.screenSize.x = ui->widthSpinBox->value();
    resultData.screenSize.y = ui->heightSpinBox->value();

    accept();
}

bool EditPreviewSettingsDialog::Validate()
{
    QString errorMessage;
    if (ui->deviceNameLineEdit->text().isEmpty())
    {
        errorMessage = tr("Please specify the device name.");
        ui->deviceNameLineEdit->setFocus();
    }
    else if (ui->widthSpinBox->value() == 0)
    {
        errorMessage = tr("Please specify the width of the device's screen.");
        ui->widthSpinBox->setFocus();
    }
    else if (ui->heightSpinBox->value() == 0)
    {
        errorMessage = tr("Please specify the height of the device's screen.");
        ui->heightSpinBox->setFocus();
    }
    else if (ui->dpiSpinBox->value() == 0)
    {
        errorMessage = tr("Please specify the DPI of the device");
        ui->dpiSpinBox->setFocus();
    }
    else if (FLOAT_EQUAL(ui->diagonalSpinBox->value(), 0.0f))
    {
        errorMessage = tr("Please specify the diagonal of the device's screen, in inches.");
        ui->diagonalSpinBox->setFocus();
    }
    
    if (errorMessage.isEmpty())
    {
        return true;
    }
    
    QMessageBox msgBox;
    msgBox.setText(errorMessage);
    msgBox.exec();
    
    return false;
}

void EditPreviewSettingsDialog::OnCancelButtonClicked()
{
    reject();
}

void EditPreviewSettingsDialog::OnScreenHeightValueChanged(int /*value*/)
{
    RecalculateScreenParams();
}

void EditPreviewSettingsDialog::OnScreenWidthValueChanged(int /*value*/)
{
    RecalculateScreenParams();
}

void EditPreviewSettingsDialog::OnDPIValueChanged(int /*value*/)
{
    RecalculateScreenParams();
}

void EditPreviewSettingsDialog::OnDiagonalValueChanged(double /*value*/)
{
    RecalculateScreenParams();
}

void EditPreviewSettingsDialog::OnReferenceValueSetToDPI()
{
    isReferenceValueDPI = true;
    ui->dpiSpinBox->setEnabled(true);
    ui->diagonalSpinBox->setEnabled(false);
}

void EditPreviewSettingsDialog::OnReferenceValueSetToDiagonal()
{
    isReferenceValueDPI = false;
    ui->dpiSpinBox->setEnabled(false);
    ui->diagonalSpinBox->setEnabled(true);
}

void EditPreviewSettingsDialog::RecalculateScreenParams()
{
    Vector2 screenSize(ui->widthSpinBox->value(), ui->heightSpinBox->value());
    if (isReferenceValueDPI)
    {
        // Recalculate diagonal.
        float32 diagonalValue = ScreenParamsConverter::ConverDPIToDiagonal(screenSize, ui->dpiSpinBox->value());
        WidgetSignalsBlocker blocker(ui->diagonalSpinBox);
        ui->diagonalSpinBox->setValue(diagonalValue);
    }
    else
    {
        // Recalculate DPI.
        int dpiValue = ScreenParamsConverter::ConvertDiagonalToDPI(screenSize, ui->diagonalSpinBox->value());
        WidgetSignalsBlocker blocker(ui->dpiSpinBox);
        ui->dpiSpinBox->setValue(dpiValue);
    }
}

const PreviewSettingsData& EditPreviewSettingsDialog::GetData() const
{
    return resultData;
}