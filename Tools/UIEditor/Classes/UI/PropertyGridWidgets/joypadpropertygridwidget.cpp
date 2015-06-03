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
