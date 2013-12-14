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
#include "SpeedTreeImport/SpeedTreeImportDialog.h"
#include "SpeedTreeImporter.h"
#include "ui_treeimportdialog.h"
#include "Qt/Project/ProjectManager.h"
#include "Qt/Tools/QtFileDialog/QtFileDialog.h"
#include "Qt/Main/mainwindow.h"
#include "Qt/Tools/QtWaitDialog/QtWaitDialog.h"

using namespace DAVA;

SpeedTreeImportDialog::SpeedTreeImportDialog(QWidget *parent /*= 0*/)
	: QDialog(parent)
	, ui(new Ui::QtTreeImportDialog)
{
	ui->setupUi(this);

    connect(this, SIGNAL(accepted()), this, SLOT(OnOk()));
    connect(this, SIGNAL(rejected()), this, SLOT(OnCancel()));
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    connect(ui->xmlButton, SIGNAL(clicked()), this, SLOT(OnXMLSelect()));
    connect(ui->sc2Button, SIGNAL(clicked()), this, SLOT(OnSc2Select()));
    connect(ui->imagesButton, SIGNAL(clicked()), this, SLOT(OnImagesSelect()));

	setWindowModality(Qt::WindowModal);
}

SpeedTreeImportDialog::~SpeedTreeImportDialog()
{
}

int SpeedTreeImportDialog::exec()
{
    OnXMLSelect();
    if(xmlFilePath.IsEmpty())
        return 0;

    return QDialog::exec();
}

void SpeedTreeImportDialog::OnCancel()
{
}

void SpeedTreeImportDialog::OnOk()
{
	QtMainWindow::Instance()->WaitStart("Importing tree", "Please wait...");
    SpeedTreeImporter::ImportSpeedTreeFromXML(xmlFilePath, sc2FilePath, texturesDirPath);
    QtMainWindow::Instance()->WaitStop();

    QMessageBox::information(this, "SpeedTree Import", QString(("SpeedTree model was imported to " + sc2FilePath.GetAbsolutePathname()).c_str()), QMessageBox::Ok);

    if(ui->checkBox->isChecked())
        QtMainWindow::Instance()->OpenScene(sc2FilePath.GetAbsolutePathname().c_str());
}

void SpeedTreeImportDialog::OnXMLSelect()
{
    QString selectedPath = QtFileDialog::getOpenFileName(QtMainWindow::Instance(), "Import SpeedTree", GetDefaultDialogPath(xmlFilePath), "SpeedTree RAW File (*.xml)");
    if(selectedPath.isEmpty())
        return;

    xmlFilePath = selectedPath.toStdString();

    UpdateEditLines();
}

void SpeedTreeImportDialog::OnSc2Select()
{
    QString selectedPath = QtFileDialog::getSaveFileName(QtMainWindow::Instance(), "Select .sc2 file", GetDefaultDialogPath(sc2FilePath), "SC2 File (*.sc2)");
    if(selectedPath.isEmpty())
        return;

    sc2FilePath = selectedPath.toStdString();

    UpdateEditLines(false);
}

void SpeedTreeImportDialog::OnImagesSelect()
{
    QString selectedPath = QtFileDialog::getExistingDirectory(QtMainWindow::Instance(), "Select textures directory", GetDefaultDialogPath(texturesDirPath));
    if(selectedPath.isEmpty())
        return;

    texturesDirPath = selectedPath.toStdString();

    UpdateEditLines(false, false);
}

void SpeedTreeImportDialog::UpdateEditLines(bool makeSc2PathDefault /* = true */, bool makeTexturesDirDefault /* = true */)
{
    if(makeSc2PathDefault)
    {
        QString dataSourcePath = ProjectManager::Instance()->CurProjectDataSourcePath();
        sc2FilePath = FilePath(dataSourcePath.toStdString() + FilePath::CreateWithNewExtension(xmlFilePath, ".sc2").GetFilename().c_str());
    }

    if(makeTexturesDirDefault)
    {
        texturesDirPath = sc2FilePath.GetDirectory() + "images/";
    }

    ui->xmlLineEdit->setText(QString(xmlFilePath.GetAbsolutePathname().c_str()));
    ui->sc2EditLine->setText(QString(sc2FilePath.GetAbsolutePathname().c_str()));
    ui->imagesLineEdit->setText(QString(texturesDirPath.GetAbsolutePathname().c_str()));

    ui->okButton->setDisabled(false);
    ui->sc2Button->setDisabled(false);
    ui->imagesButton->setDisabled(false);
}

QString SpeedTreeImportDialog::GetDefaultDialogPath(const FilePath & forPath)
{
    QString curDir = ProjectManager::Instance()->CurProjectPath();
    if(!forPath.IsEmpty())
        curDir = QString(forPath.GetAbsolutePathname().c_str());

    return curDir;
}