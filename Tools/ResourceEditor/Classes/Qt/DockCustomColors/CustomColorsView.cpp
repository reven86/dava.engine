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



#include "CustomColorsView.h"
#include "ui_CustomColorsView.h"

#include "Project/ProjectManager.h"
#include "Classes/Qt/Main/QtMainWindowHandler.h"
#include "../SceneEditor/EditorConfig.h"

CustomColorsView::CustomColorsView(QWidget* parent)
:	QWidget(parent),
	ui(new Ui::CustomColorsView)
{
	ui->setupUi(this);
	
	Init();
}

CustomColorsView::~CustomColorsView()
{
	delete ui;
}

void CustomColorsView::Init()
{
	QObject::connect(ProjectManager::Instance(), SIGNAL(ProjectOpened(const QString &)), this, SLOT(ProjectOpened(const QString &)));

	QtMainWindowHandler* handler = QtMainWindowHandler::Instance();
	connect(ui->buttonCustomColorsEnable, SIGNAL(clicked()), handler, SLOT(ToggleCustomColors()));

	ui->buttonCustomColorsSave->blockSignals(true);
	ui->sliderCustomColorBrushSize->blockSignals(true);
	ui->comboboxCustomColors->blockSignals(true);

	connect(ui->buttonCustomColorsSave, SIGNAL(clicked()), handler, SLOT(SaveTextureCustomColors()));
	connect(ui->sliderCustomColorBrushSize, SIGNAL(valueChanged(int)), handler, SLOT(ChangeBrushSizeCustomColors(int)));
	connect(ui->comboboxCustomColors, SIGNAL(currentIndexChanged(int)), handler, SLOT(ChangeColorCustomColors(int)));
	connect(ui->buttonCustomColorsLoad, SIGNAL(clicked()), handler, SLOT(LoadTextureCustomColors()));

	QtMainWindowHandler::Instance()->RegisterCustomColorsWidgets(ui->buttonCustomColorsEnable,
																 ui->buttonCustomColorsSave,
																 ui->sliderCustomColorBrushSize,
																 ui->comboboxCustomColors,
																 ui->buttonCustomColorsLoad);

	handler->SetCustomColorsWidgetsState(false);
}

void CustomColorsView::InitColors()
{
	QSize iconSize = ui->comboboxCustomColors->iconSize();
	iconSize = iconSize.expandedTo(QSize(100, 0));
	ui->comboboxCustomColors->setIconSize(iconSize);

	Vector<Color> customColors = EditorConfig::Instance()->GetColorPropertyValues("LandscapeCustomColors");
	Vector<String> customColorsDescription = EditorConfig::Instance()->GetComboPropertyValues("LandscapeCustomColorsDescription");
	for(size_t i = 0; i < customColors.size(); ++i)
	{
		QColor color = QColor::fromRgbF(customColors[i].r, customColors[i].g, customColors[i].b, customColors[i].a);

		QImage image(iconSize, QImage::Format_ARGB32);
		image.fill(color);

		QPixmap pixmap(iconSize);
		pixmap.convertFromImage(image, Qt::ColorOnly);

		QIcon icon(pixmap);
		String description = (i >= customColorsDescription.size()) ? "" : customColorsDescription[i];
		ui->comboboxCustomColors->addItem(icon, description.c_str());
	}
}

void CustomColorsView::ProjectOpened(const QString& path)
{
	InitColors();
}
