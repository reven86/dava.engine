#include "particleeffectpropertygridwidget.h"
#include "ui_particleeffectpropertygridwidget.h"

#include "PropertyNames.h"
#include "ResourcesManageHelper.h"

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
    
    RegisterPushButtonWidgetForInvokeMethod(ui->startButton, DAVA::PropertyNames::UIPARTICLES_START_METHOD_NAME);
    RegisterPushButtonWidgetForInvokeMethod(ui->stopButton, DAVA::PropertyNames::UIPARTICLES_STOP_METHOD_NAME);
    RegisterPushButtonWidgetForInvokeMethod(ui->reloadButton, DAVA::PropertyNames::UIPARTICLES_RELOAD_METHOD_NAME);
    
    connect(ui->selectEffectPathButton, SIGNAL(clicked()), this, SLOT(OnSelectEffectPathButtonClicked()));
    UpdateButtons();
}

void ParticleEffectPropertyGridWidget::Cleanup()
{
    // Unregister properties and invokable methods.
    UnregisterCheckBoxWidget(ui->autoStartCheckbox);
    UnregisterLineEditWidget(ui->effectPathLineEdit);

    UnregisterPushButtonWidgetForInvokeMethod(ui->startButton);
    UnregisterPushButtonWidgetForInvokeMethod(ui->stopButton);
    UnregisterPushButtonWidgetForInvokeMethod(ui->reloadButton);

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
}
