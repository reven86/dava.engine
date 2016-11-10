#include "SceneValidationDialog.h"
#include "QtTools/WarningGuard/QtWarningsHandler.h"
#include "QtTools/ConsoleWidget/LoggerOutputObject.h"
#include "SceneValidation.h"

PUSH_QT_WARNING_SUPRESSOR
#include "ui_SceneValidationDialog.h"
POP_QT_WARNING_SUPRESSOR

using namespace DAVA;

SceneValidationDialog::SceneValidationDialog(SceneEditor2* scene, QWidget* parent)
    : QDialog(parent)
    , scene(scene)
    , ui(new Ui::SceneValidationDialog)
{
    ui->setupUi(this);

    connect(ui->selectAllCheckBox, &QCheckBox::clicked, this, &SceneValidationDialog::OnSelectAllClicked);
    connect(ui->buttonGroup, SIGNAL(buttonToggled(int, bool)), this, SLOT(OnOptionToggled(int, bool)));
    connect(ui->validateButton, &QPushButton::clicked, this, &SceneValidationDialog::Validate);
    connect(ui->showConsoleCheckBox, &QCheckBox::toggled, this, &SceneValidationDialog::ShowConsole);

    LoadOptions();
    ui->validateButton->setEnabled(!AreAllOptionsSetTo(false));
    ShowConsole(ui->showConsoleCheckBox->isChecked());

    LoggerOutputObject* loggerOutput = new LoggerOutputObject(this);
    connect(loggerOutput, &LoggerOutputObject::OutputReady, ui->logWidget, &LogWidget::AddMessage, Qt::DirectConnection);
}

void SceneValidationDialog::LoadOptions()
{
    ui->matriciesCheckBox->setChecked(SettingsManager::GetValue(Settings::Internal_Validate_Matrices).AsBool());
    ui->sameNamesCheckBox->setChecked(SettingsManager::GetValue(Settings::Internal_Validate_SameNames).AsBool());
    ui->collisionsCheckBox->setChecked(SettingsManager::GetValue(Settings::Internal_Validate_CollisionProperties).AsBool());
    ui->relevanceCheckBox->setChecked(SettingsManager::GetValue(Settings::Internal_Validate_TexturesRelevance).AsBool());
    ui->materialGroupsCheckBox->setChecked(SettingsManager::GetValue(Settings::Internal_Validate_MaterialGroups).AsBool());

    ui->showConsoleCheckBox->setChecked(SettingsManager::GetValue(Settings::Internal_Validate_ShowConsole).AsBool());
}

void SceneValidationDialog::SaveOptions()
{
    SettingsManager::SetValue(Settings::Internal_Validate_Matrices, VariantType(ui->matriciesCheckBox->isChecked()));
    SettingsManager::SetValue(Settings::Internal_Validate_SameNames, VariantType(ui->sameNamesCheckBox->isChecked()));
    SettingsManager::SetValue(Settings::Internal_Validate_CollisionProperties, VariantType(ui->collisionsCheckBox->isChecked()));
    SettingsManager::SetValue(Settings::Internal_Validate_TexturesRelevance, VariantType(ui->relevanceCheckBox->isChecked()));
    SettingsManager::SetValue(Settings::Internal_Validate_MaterialGroups, VariantType(ui->materialGroupsCheckBox->isChecked()));

    SettingsManager::SetValue(Settings::Internal_Validate_ShowConsole, VariantType(ui->showConsoleCheckBox->isChecked()));
}

void SceneValidationDialog::Validate()
{
    DVASSERT(Thread::IsMainThread());

    ui->validateButton->setEnabled(false);

    if (ui->matriciesCheckBox->isChecked())
    {
        SceneValidation::LogValidationOutput output("Validating matrices");
        SceneValidation::ValidateMatrices(scene, &output);
    }

    if (ui->sameNamesCheckBox->isChecked())
    {
        SceneValidation::LogValidationOutput output("Validating same names");
        SceneValidation::ValidateSameNames(scene, &output);
    }

    if (ui->collisionsCheckBox->isChecked())
    {
        SceneValidation::LogValidationOutput output("Validating collision types");
        SceneValidation::ValidateCollisionProperties(scene, &output);
    }

    if (ui->relevanceCheckBox->isChecked())
    {
        SceneValidation::LogValidationOutput output("Validating textures relevance");
        SceneValidation::ValidateTexturesRelevance(scene, &output);
    }

    if (ui->materialGroupsCheckBox->isChecked())
    {
        SceneValidation::LogValidationOutput output("Validating material groups");
        SceneValidation::ValidateMaterialsGroups(scene, &output);
    }

    ui->validateButton->setEnabled(true);
}

void SceneValidationDialog::OnSelectAllClicked(bool clicked)
{
    foreach (QAbstractButton* optionCheckBox, ui->buttonGroup->buttons())
    {
        optionCheckBox->setChecked(clicked);
    }
}

bool SceneValidationDialog::AreAllOptionsSetTo(bool value)
{
    foreach (QAbstractButton* optionCheckBox, ui->buttonGroup->buttons())
    {
        if (optionCheckBox->isChecked() != value)
            return false;
    }
    return true;
}

void SceneValidationDialog::OnOptionToggled(int buttonId, bool toggled)
{
    if (toggled)
    {
        ui->selectAllCheckBox->setChecked(AreAllOptionsSetTo(true));
        ui->validateButton->setEnabled(true);
    }
    else
    {
        ui->selectAllCheckBox->setChecked(false);
        ui->validateButton->setEnabled(!AreAllOptionsSetTo(false));
    }
}

void SceneValidationDialog::ShowConsole(bool checked)
{
    ui->logWidget->setVisible(checked);
    ui->showConsoleCheckBox->setChecked(checked);
    if (!checked)
    {
        setFixedHeight(minimumSizeHint().height());
    }
    else
    {
        setMinimumHeight(minimumSizeHint().height());
        setMaximumHeight(QWIDGETSIZE_MAX);
    }
}

void SceneValidationDialog::closeEvent(QCloseEvent* event)
{
    SaveOptions();
    event->accept();
}
