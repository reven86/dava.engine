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

#include <QMessageBox>
#include "PlaneLODDialog.h"
#include "ui_planeloddialog.h"

#include "Qt/Main/mainwindow.h"
#include "Qt/Main/QtUtils.h"
#include "Tools/PathDescriptor/PathDescriptor.h"

#include "QtTools/FileDialog/FileDialog.h"

using namespace DAVA;

PlaneLODDialog::PlaneLODDialog(DAVA::uint32 layersCount, const DAVA::FilePath & defaultTexturePath, QWidget *parent /*= 0*/)
	: QDialog(parent)
	, ui(new Ui::QtPlaneLODDialog)
    , selectedLayer(-1)
    , selectedTextureSize(0)
{
	ui->setupUi(this);

    connect(this, SIGNAL(accepted()), this, SLOT(OnOk()));
    connect(this, SIGNAL(rejected()), this, SLOT(OnCancel()));
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    connect(ui->textureButton, SIGNAL(clicked()), this, SLOT(OnTextureSelect()));

    for(uint32 i = 0; i < layersCount; i++)
        ui->lodLevelBox->addItem(QString("LOD %1").arg(i));
    ui->lodLevelBox->setCurrentIndex(layersCount-1);

    texturePath = QString(defaultTexturePath.GetAbsolutePathname().c_str());
    ui->textureLineEdit->setText(texturePath);

	setWindowModality(Qt::WindowModal);
}

PlaneLODDialog::~PlaneLODDialog()
{
}

void PlaneLODDialog::OnCancel()
{
}

void PlaneLODDialog::OnOk()
{
    selectedLayer = ui->lodLevelBox->currentIndex();

    bool isOK = false;
    uint32 selectedSize = ui->textureSizeBox->currentText().toUInt(&isOK);
    if(isOK)
        selectedTextureSize = selectedSize;
}

void PlaneLODDialog::OnTextureSelect()
{
    QString selectedPath = FileDialog::getSaveFileName(this, QString("Save texture"), texturePath, PathDescriptor::GetPathDescriptor(PathDescriptor::PATH_IMAGE).fileFilter);
    if(selectedPath.isEmpty())
        return;

    texturePath = selectedPath;
    ui->textureLineEdit->setText(texturePath);
}

int32 PlaneLODDialog::GetSelectedLayer()
{
    return selectedLayer;
}

FilePath PlaneLODDialog::GetSelectedTexturePath()
{
    return PathnameToDAVAStyle(texturePath);
}

uint32 PlaneLODDialog::GetSelectedTextureSize()
{
    return selectedTextureSize;
}