#include "particleeffectpropertygridwidget.h"
#include "ui_particleeffectpropertygridwidget.h"

#include "PropertiesHelper.h"
#include "PropertyNames.h"
#include "ResourcesManageHelper.h"
#include "WidgetSignalsBlocker.h"

#include "CommandsController.h"
#include "ChangePropertyCommand.h"

#include <QFileDialog>

static const QString PARTICLE_EFFECT_VIEW_PROPERTY_BLOCK_NAME = "Particles options";

ParticleEffectPropertyGridWidget::ParticleEffectPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::ParticleEffectPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(PARTICLE_EFFECT_VIEW_PROPERTY_BLOCK_NAME);
}

ParticleEffectPropertyGridWidget::~ParticleEffectPropertyGridWidget()
{
    delete ui;
}


void ParticleEffectPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
    
    // Register properties and invokable methods.
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();
    RegisterCheckBoxWidgetForProperty(propertiesMap, DAVA::PropertyNames::UIPARTICLES_AUTOSTART_PROPERTY, ui->autoStartCheckbox);
	RegisterLineEditWidgetForProperty(propertiesMap, DAVA::PropertyNames::UIPARTICLES_EFFECT_PATH_PROPERTY, ui->effectPathLineEdit);
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, DAVA::PropertyNames::UIPARTICLES_START_DELAY_PROPERTY, ui->startDelaySpinbox);

    RegisterPushButtonWidgetForInvokeMethod(ui->startButton, DAVA::PropertyNames::UIPARTICLES_START_METHOD_NAME);
    RegisterPushButtonWidgetForInvokeMethod(ui->stopButton, DAVA::PropertyNames::UIPARTICLES_STOP_METHOD_NAME);
    RegisterPushButtonWidgetForInvokeMethod(ui->pauseButton, DAVA::PropertyNames::UIPARTICLES_PAUSE_METHOD_NAME);
    RegisterPushButtonWidgetForInvokeMethod(ui->restartButton, DAVA::PropertyNames::UIPARTICLES_RESTART_METHOD_NAME);
    RegisterPushButtonWidgetForInvokeMethod(ui->reloadButton, DAVA::PropertyNames::UIPARTICLES_RELOAD_METHOD_NAME);

    connect(ui->selectEffectPathButton, SIGNAL(clicked()), this, SLOT(OnSelectEffectPathButtonClicked()));
    UpdateButtons();
}

void ParticleEffectPropertyGridWidget::Cleanup()
{
    // Unregister properties and invokable methods.
    UnregisterCheckBoxWidget(ui->autoStartCheckbox);
    UnregisterLineEditWidget(ui->effectPathLineEdit);
    UnregisterDoubleSpinBoxWidget(ui->startDelaySpinbox);

    UnregisterPushButtonWidgetForInvokeMethod(ui->startButton);
    UnregisterPushButtonWidgetForInvokeMethod(ui->stopButton);
    UnregisterPushButtonWidgetForInvokeMethod(ui->reloadButton);
    UnregisterPushButtonWidgetForInvokeMethod(ui->pauseButton);
    UnregisterPushButtonWidgetForInvokeMethod(ui->restartButton);

    disconnect(ui->selectEffectPathButton, SIGNAL(clicked()), this, SLOT(OnSelectEffectPathButtonClicked()));
    BasePropertyGridWidget::Cleanup();
}

void ParticleEffectPropertyGridWidget::OnSelectEffectPathButtonClicked()
{
	QString defaultDir = ResourcesManageHelper::GetResourceRootDirectory();
    QString effectPath = QFileDialog::getOpenFileName(this, tr( "Choose an effect file" ),
                                                      defaultDir,tr( "SC2 files (*.sc2)"));
	if(!effectPath.isNull() && !effectPath.isEmpty())
    {
		effectPath = ResourcesManageHelper::ConvertPathToUnixStyle(effectPath);
		if (ResourcesManageHelper::ValidateResourcePath(effectPath))
        {
            ui->effectPathLineEdit->setText(effectPath);
            HandleLineEditEditingFinished(ui->effectPathLineEdit);
            UpdateButtons();
        }
		else
		{
			ResourcesManageHelper::ShowErrorMessage(effectPath);
		}
    }
}

void ParticleEffectPropertyGridWidget::UpdateButtons()
{
    bool enableButtons = !ui->effectPathLineEdit->text().isEmpty();
    ui->startButton->setEnabled(enableButtons);
    ui->stopButton->setEnabled(enableButtons);
    ui->reloadButton->setEnabled(enableButtons);
    ui->pauseButton->setEnabled(enableButtons);
    ui->restartButton->setEnabled(enableButtons);
}

void ParticleEffectPropertyGridWidget::ProcessDoubleSpinBoxValueChanged(QDoubleSpinBox *doubleSpinBoxWidget,
                                                                const PROPERTYGRIDWIDGETSITER &iter, const double value)
{
    if (!doubleSpinBoxWidget)
    {
        return;
    }

	if (doubleSpinBoxWidget == ui->startDelaySpinbox)
	{
		// Don't update the property if the text wasn't actually changed.
		double curValue = PropertiesHelper::GetAllPropertyValues<double>(this->activeMetadata, iter->second.getProperty().name());
		if (curValue == value)
		{
			return;
		}
        
		BaseCommand* command = new ChangeDoublePropertyCommand(activeMetadata, iter->second, value);
  	  	CommandsController::Instance()->ExecuteCommand(command);
   		SafeRelease(command);
		return;
	}
    
    BasePropertyGridWidget::ProcessDoubleSpinBoxValueChanged(doubleSpinBoxWidget, iter, value);
}

void ParticleEffectPropertyGridWidget::UpdateDoubleSpinBoxWidgetWithPropertyValue(QDoubleSpinBox *doubleSpinBoxWidget,
                                                                          const QMetaProperty& curProperty)
{
    if (!this->activeMetadata)
    {
        return;
    }

	double propertyValue = PropertiesHelper::GetAllPropertyValues<double>(this->activeMetadata,
                                                                          curProperty.name());
    WidgetSignalsBlocker blocker(doubleSpinBoxWidget);
    doubleSpinBoxWidget->setValue(propertyValue);
	UpdateWidgetPalette(doubleSpinBoxWidget, curProperty.name());
}
