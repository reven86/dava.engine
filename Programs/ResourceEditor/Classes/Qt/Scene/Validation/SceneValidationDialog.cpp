#include "Qt/Scene/Validation/SceneValidationDialog.h"
#include "Qt/Scene/Validation/SceneValidation.h"
#include "Qt/Scene/Validation/ValidationProgressConsumer.h"
#include "Qt/Scene/SceneEditor2.h"

#include "QtTools/WarningGuard/QtWarningsHandler.h"
#include "QtTools/ConsoleWidget/LoggerOutputObject.h"

#include "Classes/Application/REGlobal.h"
#include "Classes/Project/ProjectManagerData.h"

#include <QCloseEvent>

PUSH_QT_WARNING_SUPRESSOR
#include "ui_SceneValidationDialog.h"
POP_QT_WARNING_SUPRESSOR

SceneValidationDialog::SceneValidationDialog(DAVA::Scene* scene, QWidget* parent)
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
    SettingsManager::SetValue(Settings::Internal_Validate_Matrices, DAVA::VariantType(ui->matriciesCheckBox->isChecked()));
    SettingsManager::SetValue(Settings::Internal_Validate_SameNames, DAVA::VariantType(ui->sameNamesCheckBox->isChecked()));
    SettingsManager::SetValue(Settings::Internal_Validate_CollisionProperties, DAVA::VariantType(ui->collisionsCheckBox->isChecked()));
    SettingsManager::SetValue(Settings::Internal_Validate_TexturesRelevance, DAVA::VariantType(ui->relevanceCheckBox->isChecked()));
    SettingsManager::SetValue(Settings::Internal_Validate_MaterialGroups, DAVA::VariantType(ui->materialGroupsCheckBox->isChecked()));

    SettingsManager::SetValue(Settings::Internal_Validate_ShowConsole, DAVA::VariantType(ui->showConsoleCheckBox->isChecked()));
}

void SceneValidationDialog::Validate()
{
    DVASSERT(DAVA::Thread::IsMainThread());

    ProjectManagerData* data = REGlobal::GetDataNode<ProjectManagerData>();
    SceneValidation validation(data);

    ui->validateButton->setEnabled(false);

    ValidationProgressToLog progressToLog;

    if (ui->matriciesCheckBox->isChecked())
    {
        ValidationProgress validationProgress;
        validationProgress.SetProgressConsumer(&progressToLog);
        validation.ValidateMatrices(scene, validationProgress);
    }

    if (ui->sameNamesCheckBox->isChecked())
    {
        ValidationProgress validationProgress;
        validationProgress.SetProgressConsumer(&progressToLog);
        validation.ValidateSameNames(scene, validationProgress);
    }

    if (ui->collisionsCheckBox->isChecked())
    {
        ValidationProgress validationProgress;
        validationProgress.SetProgressConsumer(&progressToLog);
        validation.ValidateCollisionProperties(scene, validationProgress);
    }

    if (ui->relevanceCheckBox->isChecked())
    {
        ValidationProgress validationProgress;
        validationProgress.SetProgressConsumer(&progressToLog);
        validation.ValidateTexturesRelevance(scene, validationProgress);
    }

    if (ui->materialGroupsCheckBox->isChecked())
    {
        ValidationProgress validationProgress;
        validationProgress.SetProgressConsumer(&progressToLog);
        validation.ValidateMaterialsGroups(scene, validationProgress);
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
