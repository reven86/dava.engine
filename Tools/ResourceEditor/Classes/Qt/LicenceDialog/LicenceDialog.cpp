#include "LicenceDialog.h"
#include "ui_licencedialog.h"

#include "Settings/SettingsManager.h"


namespace
{
}


LicenceDialog::LicenceDialog( QWidget* parent )
    : QDialog( parent, Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint )
    , ui( new Ui::LicenceDialog )
    , accepted( false )
{
    ui->setupUi( this );

    onAcceptCheckbox();

    connect( ui->accept, SIGNAL( clicked() ), SLOT( onAccept() ) );
    connect( ui->decline, SIGNAL( clicked() ), SLOT( onDecline() ) );
    connect( ui->accept, SIGNAL( clicked() ), SLOT( close() ) );
    connect( ui->decline, SIGNAL( clicked() ), SLOT( close() ) );
    connect( ui->accepted, SIGNAL( stateChanged( int ) ), SLOT( onAcceptCheckbox() ) );
}

LicenceDialog::~LicenceDialog()
{
}

bool LicenceDialog::process()
{
    // return true;    // ;)

    accepted = SettingsManager::Instance()->GetValue(ResourceEditor::SETTINGS_LICENCE_ACCEPTED, SettingsManager::INTERNAL).AsBool();
    if ( !accepted )
    {
        exec();
    }

    return accepted;
}

void LicenceDialog::setHtmlText(const QString& text)
{
    ui->text->setHtml(text);
}

void LicenceDialog::onAccept()
{
    const bool isAccepted = ui->accepted->isChecked();
    SettingsManager::Instance()->SetValue(ResourceEditor::SETTINGS_LICENCE_ACCEPTED, VariantType(isAccepted), SettingsManager::INTERNAL);
    accepted = SettingsManager::Instance()->GetValue(ResourceEditor::SETTINGS_LICENCE_ACCEPTED, SettingsManager::INTERNAL).AsBool();
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
