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
#include "Qt/Main/mainwindow.h"
#include "Qt/Tools/QtWaitDialog/QtWaitDialog.h"

#include "QtTools/FileDialog/FileDialog.h"

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

	setWindowModality(Qt::WindowModal);
}

SpeedTreeImportDialog::~SpeedTreeImportDialog()
{
}

int SpeedTreeImportDialog::exec()
{
    OnXMLSelect();
    if(!xmlFiles.size())
        return 0;

    return QDialog::exec();
}

void SpeedTreeImportDialog::OnCancel()
{
}

void SpeedTreeImportDialog::OnOk()
{
#ifdef __DAVAENGINE_SPEEDTREE__
    sc2FolderPath = ui->sc2EditLine->text().toStdString();
    sc2FolderPath.MakeDirectoryPathname();

    FilePath texturesDirPath = sc2FolderPath + "images/";

    //make out files
    Vector<FilePath> outFiles = xmlFiles;
    for(size_t i = 0; i < outFiles.size(); ++i)
    {
        outFiles[i].ReplaceDirectory(sc2FolderPath);
        outFiles[i].ReplaceExtension(".sc2");
    }

    //import all trees
	QtMainWindow::Instance()->WaitStart("Importing tree", "Please wait...");

    for(size_t i = 0; i < xmlFiles.size(); ++i)
    {
        SpeedTreeImporter::ImportSpeedTreeFromXML(xmlFiles[i], outFiles[i], texturesDirPath);
    }
    
    QtMainWindow::Instance()->WaitStop();

    //make info message
    QString message("SpeedTree models: \n");
    for(size_t i = 0; i < outFiles.size(); ++i)
        message += (outFiles[i].GetFilename() + "\n").c_str();
    message += "\nwas imported to:\n" + QString(sc2FolderPath.GetAbsolutePathname().c_str());

    QMessageBox::information(this, "SpeedTree Import", message, QMessageBox::Ok);

    //open importet trees
    if(ui->checkBox->isChecked())
    {
        for(size_t i = 0; i < outFiles.size(); ++i)
        {
            QtMainWindow::Instance()->OpenScene(outFiles[i].GetAbsolutePathname().c_str());
        }
    }
#endif
}

void SpeedTreeImportDialog::OnXMLSelect()
{
    QString dialogPath;
    if(xmlFiles.size())
        dialogPath = QString(xmlFiles[0].GetDirectory().GetAbsolutePathname().c_str());

    QStringList selectedFiles = FileDialog::getOpenFileNames(QtMainWindow::Instance(), "Import SpeedTree", dialogPath, "SpeedTree RAW File (*.xml)");
    if(!selectedFiles.size())
        return;

    xmlFiles.clear();
    for(int32 i = 0; i < selectedFiles.size(); ++i)
        xmlFiles.push_back(FilePath(selectedFiles.at(i).toStdString()));

    if(sc2FolderPath.IsEmpty())
        SetSC2FolderValue(ProjectManager::Instance()->GetDataSourcePath().GetAbsolutePathname().c_str());

    ui->xmlListWidget->clear();
    ui->xmlListWidget->addItems(selectedFiles);
}

void SpeedTreeImportDialog::OnSc2Select()
{
    QString dialogPath = ProjectManager::Instance()->GetProjectPath().GetAbsolutePathname().c_str();
    if(!sc2FolderPath.IsEmpty())
        dialogPath = QString(sc2FolderPath.GetAbsolutePathname().c_str());

    QString selectedPath = FileDialog::getExistingDirectory(QtMainWindow::Instance(), "Select .sc2 file", dialogPath);
    if(selectedPath.isEmpty())
        return;

    SetSC2FolderValue(selectedPath);
}

void SpeedTreeImportDialog::SetSC2FolderValue(const QString & path)
{
    sc2FolderPath = path.toStdString();
    sc2FolderPath.MakeDirectoryPathname();
    ui->sc2EditLine->setText(QString(sc2FolderPath.GetAbsolutePathname().c_str()));
}