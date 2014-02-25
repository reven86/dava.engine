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

EditPreviewSettingsDialog::EditPreviewSettingsDialog()
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