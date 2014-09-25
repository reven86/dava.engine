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


#include "platformpropertygridwidget.h"
#include "ui_platformpropertygridwidget.h"

static const QString RECT_PROPERTY_BLOCK_NAME = "Platform";

PlatformPropertyGridWidget::PlatformPropertyGridWidget(QWidget *parent) :
    RootPropertyGridWidget(parent),
    ui(new Ui::PlatformPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(RECT_PROPERTY_BLOCK_NAME);

    ui->platformNameLineEdit->setValidator(new QRegExpValidator(HierarchyTreeNode::GetNameRegExp(), this));
 	ui->platformNameLineEdit->installEventFilter(this);
}

PlatformPropertyGridWidget::~PlatformPropertyGridWidget()
{
    delete ui;
}

void PlatformPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
    
    // Build the properties map to make the properties search faster.
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();
    
    // Initialize the widgets.
    RegisterLineEditWidgetForProperty(propertiesMap, "Name", ui->platformNameLineEdit, true);
    RegisterSpinBoxWidgetForProperty(propertiesMap, "Height", ui->screenHeightSpinBox, true);
    RegisterSpinBoxWidgetForProperty(propertiesMap, "Width", ui->screenWidthSpinBox, true);
}

void PlatformPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();

    UnregisterLineEditWidget(ui->platformNameLineEdit);
    UnregisterSpinBoxWidget(ui->screenHeightSpinBox);
    UnregisterSpinBoxWidget(ui->screenWidthSpinBox);
}