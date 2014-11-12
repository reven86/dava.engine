#include "joypadpropertygridwidget.h"
#include "ui_joypadpropertygridwidget.h"

#include "PropertyNames.h"
#include "ResourcesManageHelper.h"
#include "WidgetSignalsBlocker.h"
#include "PropertiesHelper.h"

#include "QFileDialog"

static const QString JOYPAD_PROPERTY_BLOCK_NAME = "Joypad";

JoypadPropertyGridWidget::JoypadPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::JoypadPropertyGridWidget)
{
    SetPropertyBlockName(JOYPAD_PROPERTY_BLOCK_NAME);
    ui->setupUi(this);
    
    connect(ui->openSpriteButton, SIGNAL(clicked()), this, SLOT(OpenSpriteDialog()));
    connect(ui->removeSpriteButton, SIGNAL(clicked()), this, SLOT(RemoveSprite()));
    
    InstallEventFiltersForWidgets(this);
}

JoypadPropertyGridWidget::~JoypadPropertyGridWidget()
{
    delete ui;
}

void JoypadPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    RegisterLineEditWidgetForProperty(propertiesMap, PropertyNames::JOYPAD_STICK_SPRITE_PROPERTY_NAME, ui->spriteLineEdit, false, true);
    RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::JOYPAD_STICK_SPRITE_FRAME_PROPERTY_NAME, this->ui->frameSpinBox, false, true);

    RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::JOYPAD_DEAD_AREA_PROPERTY_NAME, this->ui->deadAreaSpinBox, false, true);
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, PropertyNames::JOYPAD_DIGITAL_SENSE_PROPERTY_NAME, this->ui->digitalSenseSpinBox, false, true);

	ui->spriteLineEdit->setEnabled(true);
}

void JoypadPropertyGridWidget::OpenSpriteDialog()
{
	QString currentSpriteDir = ResourcesManageHelper::GetDefaultSpritesPath(this->ui->spriteLineEdit->text());
    QString spriteName = QFileDialog::getOpenFileName(this, tr("Choose a sprite file"),
                                                      currentSpriteDir,
                                                      tr("Sprites (*.txt)"));
	if(!spriteName.isNull() && !spriteName.isEmpty())
    {
		spriteName = ResourcesManageHelper::ConvertPathToUnixStyle(spriteName);
		if (ResourcesManageHelper::ValidateResourcePath(spriteName))
        {
            WidgetSignalsBlocker blocker(ui->spriteLineEdit);
            ui->spriteLineEdit->setText(PreprocessSpriteName(spriteName));
            HandleLineEditEditingFinished(ui->spriteLineEdit);
        }
		else
		{
			ResourcesManageHelper::ShowErrorMessage(spriteName);
		}
    }
}

void JoypadPropertyGridWidget::RemoveSprite()
{
    if (!ui->spriteLineEdit->text().isEmpty())
    {
        WidgetSignalsBlocker blocker(ui->spriteLineEdit);
        ui->spriteLineEdit->setText("");
        HandleLineEditEditingFinished(ui->spriteLineEdit);
    }
}

void JoypadPropertyGridWidget::UpdateDoubleSpinBoxWidgetWithPropertyValue(QDoubleSpinBox *doubleSpinBoxWidget,
                                                                          const QMetaProperty& curProperty)
{
    if (!this->activeMetadata)
    {
        return;
    }

    double propertyValue = PropertiesHelper::GetAllPropertyValues<double>(this->activeMetadata,
                                                                          curProperty.name());
    WidgetSignalsBlocker blocker(doubleSpinBoxWidget);
    
    if (doubleSpinBoxWidget == ui->digitalSenseSpinBox)
    {
        doubleSpinBoxWidget->setValue(propertyValue);
        return;
    }

    BasePropertyGridWidget::UpdateDoubleSpinBoxWidgetWithPropertyValue(doubleSpinBoxWidget, curProperty);
}
