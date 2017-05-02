#include "LicenceDialog.h"
#include "ui_LicenceDialog.h"

#include "Settings/SettingsManager.h"

namespace
{
}

LicenceDialog::LicenceDialog(QWidget* parent)
    : QDialog(parent, Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint)
    , ui(new Ui::LicenceDialog)
    , accepted(false)
{
    ui->setupUi(this);

    onAcceptCheckbox();

    connect(ui->accept, SIGNAL(clicked()), SLOT(onAccept()));
    connect(ui->decline, SIGNAL(clicked()), SLOT(onDecline()));
    connect(ui->accept, SIGNAL(clicked()), SLOT(close()));
    connect(ui->decline, SIGNAL(clicked()), SLOT(close()));
    connect(ui->accepted, SIGNAL(stateChanged(int)), SLOT(onAcceptCheckbox()));
}

LicenceDialog::~LicenceDialog()
{
}

bool LicenceDialog::process()
{
    return true; // ;)
    //  Disabled funny license
    //    accepted = SettingsManager::GetValue(Settings::Internal_LicenceAccepted).AsBool();
    //    if (!accepted)
    //    {
    //        exec();
    //    }
    //
    //    return accepted;
}

void LicenceDialog::setHtmlText(const QString& text)
{
    ui->text->setHtml(text);
}

void LicenceDialog::onAccept()
{
    accepted = ui->accepted->isChecked();
    SettingsManager::SetValue(Settings::Internal_LicenceAccepted, DAVA::VariantType(accepted));
}

void LicenceDialog::onDecline()
{
    accepted = false;
}

void LicenceDialog::onAcceptCheckbox()
{
    const bool isAccepted = ui->accepted->isChecked();
    ui->accept->setEnabled(isAccepted);
}
