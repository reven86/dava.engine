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


#include "ui_ImageSplitter.h"
#include "ImageSplitterDialog/ImageSplitterDialog.h"
#include "Project/ProjectManager.h"
#include "ImageTools/ImageTools.h"
#include "SizeDialog.h"
#include "Main/QtUtils.h"

#include <QMessageBox>
#include <QFileDialog>

ImageSplitterDialog::ImageSplitterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageSplitter),
    acceptableSize(0,0)
{
    ui->setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->selectPathWidget->SetClearButtonVisible(false);
    ui->selectPathWidget->setAcceptDrops(false);
    ui->selectPathWidget->SetFileFormatFilter("PNG(*.png)");
    DAVA::FilePath defaultPath = ProjectManager::Instance()->CurProjectPath();
    ui->selectPathWidget->SetOpenDialogDefaultPath(defaultPath);
    ui->saveBtn->setFocus();
    lastSelectedFile = "";
    ConnectSignals();
    rgbaControls.push_back(ui->redImgLbl);
    rgbaControls.push_back(ui->greenImgLbl);
    rgbaControls.push_back(ui->blueImgLbl);
    rgbaControls.push_back(ui->alphaImgLbl);
}

ImageSplitterDialog::~ImageSplitterDialog()
{
    delete ui;
}

void ImageSplitterDialog::ConnectSignals()
{
    connect(ui->selectPathWidget, SIGNAL(PathSelected(DAVA::String)), this, SLOT(PathSelected(DAVA::String)));

    connect(ui->redImgLbl, SIGNAL(changed()), this, SLOT(ImageAreaChanged()));
    connect(ui->greenImgLbl, SIGNAL(changed()), this, SLOT(ImageAreaChanged()));
    connect(ui->blueImgLbl, SIGNAL(changed()), this, SLOT(ImageAreaChanged()));
    connect(ui->alphaImgLbl, SIGNAL(changed()), this, SLOT(ImageAreaChanged()));
    
    connect(ui->restoreBtn, SIGNAL(clicked()), this, SLOT(OnRestoreClicked()));
    
    connect(ui->saveAsBtn, SIGNAL(clicked()), this, SLOT(OnSaveAsClicked()));
    connect(ui->saveBtn, SIGNAL(clicked()), this, SLOT(OnSaveClicked()));
    connect(ui->saveChannelsBtn, SIGNAL(clicked()), this, SLOT(OnSaveChannelsClicked()));
    
    connect(ui->redFillBtn, SIGNAL(clicked()), this, SLOT(OnFillBtnClicked()));
    connect(ui->greenFillBtn, SIGNAL(clicked()), this, SLOT(OnFillBtnClicked()));
    connect(ui->blueFillBtn, SIGNAL(clicked()), this, SLOT(OnFillBtnClicked()));
    connect(ui->alphaFillBtn, SIGNAL(clicked()), this, SLOT(OnFillBtnClicked()));
}

void ImageSplitterDialog::PathSelected(DAVA::String path)
{
    if(path.empty())
    {
        return;
    }
    
    DAVA::FilePath imagePath(path);
    DAVA::Image* image = CreateTopLevelImage(imagePath);
    if(NULL != image && image->GetPixelFormat() == DAVA::FORMAT_RGBA8888)
    {
        lastSelectedFile = imagePath.GetAbsolutePathname();
        SetAcceptableImageSize(DAVA::Vector2(image->GetWidth(), image->GetHeight()));
        
        Channels channels =  ImageTools::CreateSplittedImages(image);
        DAVA::SafeRelease(image);
        
        ui->redImgLbl->SetImage(channels.red);
        ui->greenImgLbl->SetImage(channels.green);
        ui->blueImgLbl->SetImage(channels.blue);
        ui->alphaImgLbl->SetImage(channels.alpha);
        
        channels.ReleaseImages();
    }
    else
    {
        ui->selectPathWidget->blockSignals(true);
        ui->selectPathWidget->setText(lastSelectedFile);
        ui->selectPathWidget->blockSignals(false);
        if(NULL == image)
        {
            QMessageBox::warning(this, "File error", "Cann't load image.", QMessageBox::Ok);
        }
        else if(image->GetPixelFormat() != DAVA::FORMAT_RGBA8888)
        {
            QMessageBox::warning(this, "File error", "Image must be in RGBA8888 format.", QMessageBox::Ok);
        }
    }
}

void ImageSplitterDialog::ImageAreaChanged()
{
    ImageArea* sender = dynamic_cast<ImageArea*>(QObject::sender());
    DAVA::Image* image = sender->GetImage();
    DAVA::Vector2 senderImageSize;
    if(image != NULL)
    {
        senderImageSize.Set(image->GetWidth(),image->GetHeight());
    }
    bool isSomeAreaSet = false;
    foreach(ImageArea* control, rgbaControls)
    {
        if(control != sender && control->GetImage() != NULL)
        {
            isSomeAreaSet = true;
            break;
        }
    }
    
    SetAcceptableImageSize(senderImageSize);
    // size restriction for current area must be removed
    // in case of all another ones are empty
    if(!isSomeAreaSet)
    {
        sender->SetAcceptableSize(DAVA::Vector2());
    }
}

void ImageSplitterDialog::OnRestoreClicked()
{
    SetAcceptableImageSize(DAVA::Vector2(0,0));
    ui->redImgLbl->clear();
    ui->greenImgLbl->clear();
    ui->blueImgLbl->clear();
    ui->alphaImgLbl->clear();
    ui->redSpinBox->setValue(0);
    ui->greenSpinBox->setValue(0);
    ui->blueSpinBox->setValue(0);
    ui->alphaSpinBox->setValue(0);
    PathSelected(lastSelectedFile);
}

void ImageSplitterDialog::OnSaveAsClicked(bool saveSplittedImages)
{
    DAVA::FilePath retPath = QFileDialog::getSaveFileName(this, "Select png", ProjectManager::Instance()->CurProjectPath().GetAbsolutePathname().c_str(),"PNG(*.png)").toStdString();
    if(!retPath.IsEmpty())
    {
        Save(retPath, saveSplittedImages);
    }
}

void ImageSplitterDialog::OnSaveClicked()
{
    DAVA::FilePath presentPath = ui->selectPathWidget->text().toStdString();
    if(!presentPath.Exists())
    {
        OnSaveAsClicked();
        return;
    }
    
    if(DAVA::FileSystem::Instance()->DeleteFile(presentPath))
    {
        Save(presentPath, false);
    }
    else
    {
        QMessageBox::warning(this, "Save error", "Cann't overwrite file.", QMessageBox::Ok);
    }
}

void ImageSplitterDialog::OnSaveChannelsClicked()
{
    DAVA::FilePath savePath = ui->selectPathWidget->text().toStdString();
    if(!savePath.Exists())
    {
        QFileDialog dialog;
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setOption(QFileDialog::ShowDirsOnly);
        dialog.setDirectory(ProjectManager::Instance()->CurProjectPath().GetAbsolutePathname().c_str());
        dialog.exec();
        DAVA::FilePath selectedFolder = dialog.selectedFiles().first().toStdString();
        selectedFolder.MakeDirectoryPathname();
        if(!selectedFolder.Exists() || dialog.result() == QDialog::Rejected)
        {
            return;
        }
        savePath = selectedFolder;
    }
    
    Save(savePath, true);
}

void ImageSplitterDialog::OnFillBtnClicked()
{
    QPushButton* sender = dynamic_cast<QPushButton*>(QObject::sender());
    ImageArea * targetImageArea = NULL;
    QSpinBox* sourceSpinBox = NULL;
    
    if(sender == ui->redFillBtn)
    {
        targetImageArea = ui->redImgLbl;
        sourceSpinBox = ui->redSpinBox;
    }
    else if(sender == ui->greenFillBtn)
    {
        targetImageArea = ui->greenImgLbl;
        sourceSpinBox = ui->greenSpinBox;
    }
    else if(sender == ui->blueFillBtn)
    {
        targetImageArea = ui->blueImgLbl;
        sourceSpinBox = ui->blueSpinBox;
    }
    else if(sender == ui->alphaFillBtn)
    {
        targetImageArea = ui->alphaImgLbl;
        sourceSpinBox = ui->alphaSpinBox;
    }
    else
    {
        return;
    }
    DAVA::Image* targetImage = targetImageArea->GetImage();
    DAVA::uint8 value = (DAVA::uint8)sourceSpinBox->value();
    
    if(acceptableSize.IsZero())
    {
        SizeDialog sizeDlg(this);
        if(sizeDlg.exec() == QDialog::Rejected)
        {
            return;
        }
        acceptableSize = sizeDlg.GetSize();
    }
    
    DAVA::uint32 width = acceptableSize.x;
    DAVA::uint32 height = acceptableSize.y;
    DAVA::Vector<DAVA::uint8> buffer(width * height,0);
    buffer.assign(buffer.size(), value);
    DAVA::Image* bufferImg = DAVA::Image::CreateFromData(width, height, DAVA::FORMAT_A8, &buffer[0]);
    if(targetImage == NULL)
    {
        targetImageArea->SetImage(bufferImg);
    }
    else
    {
        targetImage->InsertImage(bufferImg, 0, 0);
        targetImageArea->UpdatePreviewPicture();
    }
    
    DAVA::SafeRelease(bufferImg);
}

void ImageSplitterDialog::SetAcceptableImageSize(const DAVA::Vector2& newSize)
{
    acceptableSize = newSize;
    DAVA::String lblText = acceptableSize.IsZero() ? "" : DAVA::Format("%.f * %.f", acceptableSize.x, acceptableSize.y);
    ui->ImageSizeLbl->setText(lblText.c_str());
    ui->redImgLbl->SetAcceptableSize(acceptableSize);
    ui->greenImgLbl->SetAcceptableSize(acceptableSize);
    ui->blueImgLbl->SetAcceptableSize(acceptableSize);
    ui->alphaImgLbl->SetAcceptableSize(acceptableSize);
}

void ImageSplitterDialog::Save(const DAVA::FilePath& filePath, bool saveSplittedImagesSeparately)
{
    if(!filePath.IsEqualToExtension(".png") && !saveSplittedImagesSeparately)
    {
        QMessageBox::warning(this, "Save error", "Wrong file name.", QMessageBox::Ok);
        return;
    }
    
    Channels channels(ui->redImgLbl->GetImage(),
                      ui->greenImgLbl->GetImage(),
                      ui->blueImgLbl->GetImage(),
                      ui->alphaImgLbl->GetImage());
    
    if(channels.IsEmpty())
    {
        QMessageBox::warning(this, "Save error", "One or more channel is incorrect.", QMessageBox::Ok);
        return;
    }
    
    if (saveSplittedImagesSeparately)
    {
        DAVA::String directory = filePath.GetDirectory().GetAbsolutePathname();
        DAVA::String baseName = filePath.GetBasename();
        
        DAVA::ImageLoader::Save(channels.red, directory + baseName + "_red.png");
        DAVA::ImageLoader::Save(channels.green, directory + baseName + "_green.png");
        DAVA::ImageLoader::Save(channels.blue, directory + baseName + "_blue.png");
        DAVA::ImageLoader::Save(channels.alpha, directory + baseName + "_alpha.png");
    }
    else
    {
        DAVA::Image* mergedImage = ImageTools::CreateMergedImage(channels);
        DAVA::ImageLoader::Save(mergedImage, filePath);
        DAVA::SafeRelease(mergedImage);
        ui->selectPathWidget->setText(filePath.GetAbsolutePathname());
    }
    QMessageBox::information(this, "Save succes", "Saved!", QMessageBox::Ok);
}
