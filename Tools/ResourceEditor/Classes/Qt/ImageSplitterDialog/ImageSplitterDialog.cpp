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
#include "Render/Image/ImageSystem.h"

#include "Tools/PathDescriptor/PathDescriptor.h"

#include "Qt/Settings/SettingsManager.h"
#include "QtTools/FileDialog/FileDialog.h"

#include <QMessageBox>
#include <QFileInfo>

ImageSplitterDialog::ImageSplitterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageSplitter()),
    acceptableSize(0,0)
{
    ui->setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    
    ui->path->SetFilter(PathDescriptor::GetPathDescriptor(PathDescriptor::PATH_IMAGE).fileFilter);
        
    DAVA::FilePath defaultPath = SettingsManager::Instance()->GetValue(Settings::Internal_ImageSplitterPath).AsString();
    if (defaultPath.IsEmpty())
    {
        defaultPath = GetDefaultPath();
    }

    ui->path->SetDefaultFolder(QString::fromStdString(defaultPath.GetDirectory().GetAbsolutePathname()));
    ui->path->SetPath(QString::fromStdString(defaultPath.GetAbsolutePathname()));
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
}

void ImageSplitterDialog::ConnectSignals()
{
    connect(ui->path, SIGNAL(pathChanged(const QString&)), SLOT(PathSelected(const QString&)));

    connect(ui->redImgLbl, SIGNAL(changed()), SLOT(ImageAreaChanged()));
    connect(ui->greenImgLbl, SIGNAL(changed()), SLOT(ImageAreaChanged()));
    connect(ui->blueImgLbl, SIGNAL(changed()), SLOT(ImageAreaChanged()));
    connect(ui->alphaImgLbl, SIGNAL(changed()), SLOT(ImageAreaChanged()));
    
    connect(ui->restoreBtn, SIGNAL(clicked()), SLOT(OnRestoreClicked()));
    
    connect(ui->saveAsBtn, SIGNAL(clicked()), SLOT(OnSaveAsClicked()));
    connect(ui->saveBtn, SIGNAL(clicked()), SLOT(OnSaveClicked()));
    connect(ui->saveChannelsBtn, SIGNAL(clicked()), SLOT(OnSaveChannelsClicked()));
    
    connect(ui->redFillBtn, SIGNAL(clicked()), SLOT(OnFillBtnClicked()));
    connect(ui->greenFillBtn, SIGNAL(clicked()), SLOT(OnFillBtnClicked()));
    connect(ui->blueFillBtn, SIGNAL(clicked()), SLOT(OnFillBtnClicked()));
    connect(ui->alphaFillBtn, SIGNAL(clicked()), SLOT(OnFillBtnClicked()));

    connect(ui->reload, SIGNAL(clicked()), SLOT(OnReload()));
    connect(ui->reloadSpecular, SIGNAL(clicked()), SLOT(OnReloadSpecularMap()));
}

void ImageSplitterDialog::PathSelected(const QString& path)
{
    if(path.isEmpty())
    {
        return;
    }

    DAVA::FilePath defaultPath = ui->path->text().toStdString();
    if (defaultPath.IsEmpty())
    {
        defaultPath = GetDefaultPath();
    }
    SettingsManager::Instance()->SetValue(Settings::Internal_ImageSplitterPath, DAVA::VariantType(defaultPath.GetAbsolutePathname()));
    
    DAVA::FilePath imagePath(path.toStdString());
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
        ui->path->SetDefaultFolder(QString::fromStdString(defaultPath.GetDirectory().GetAbsolutePathname()));
        ui->path->SetPath(QString());
        if(NULL == image)
        {
            QMessageBox::warning(this, "File error", "Couldn't load image.", QMessageBox::Ok);
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
    ui->redImgLbl->ClearArea();
    ui->greenImgLbl->ClearArea();
    ui->blueImgLbl->ClearArea();
    ui->alphaImgLbl->ClearArea();
    ui->redSpinBox->setValue(0);
    ui->greenSpinBox->setValue(0);
    ui->blueSpinBox->setValue(0);
    ui->alphaSpinBox->setValue(0);

    PathSelected(QString::fromStdString(lastSelectedFile));
}

void ImageSplitterDialog::OnSaveAsClicked(bool saveSplittedImages)
{
    DAVA::FilePath retPath = FileDialog::getSaveFileName(this, "Select image", ProjectManager::Instance()->GetProjectPath().GetAbsolutePathname().c_str(),
                                                         PathDescriptor::GetPathDescriptor(PathDescriptor::PATH_IMAGE).fileFilter)
                             .toStdString();
    if(!retPath.IsEmpty())
    {
        Save(retPath, saveSplittedImages);
    }
}

void ImageSplitterDialog::OnSaveClicked()
{
    DAVA::FilePath presentPath = ui->path->text().toStdString();
    if(!presentPath.Exists())
    {
        OnSaveAsClicked();
        return;
    }
    
    Save(presentPath, false);
}

void ImageSplitterDialog::OnSaveChannelsClicked()
{
    DAVA::FilePath savePath = ui->path->text().toStdString();
    if(!savePath.Exists())
    {
        auto folder = FileDialog::getExistingDirectory(this, "Select folder to save images", ProjectManager::Instance()->GetProjectPath().GetAbsolutePathname().c_str(), FileDialog::ShowDirsOnly);

        if(folder.isEmpty() || folder.isNull())
        {
            return;
        }
        
        savePath = folder.toStdString();
        savePath.MakeDirectoryPathname();
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

void ImageSplitterDialog::OnReload()
{
    const DAVA::FilePath path = ui->path->text().toStdString();
    PathSelected(QString::fromStdString(path.GetAbsolutePathname()));
}

void ImageSplitterDialog::OnReloadSpecularMap()
{
    const DAVA::FilePath path = ui->alphaImgLbl->GetImagePath();
    ui->alphaImgLbl->SetImage(path);
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
    if(     !DAVA::TextureDescriptor::IsSourceTextureExtension(filePath.GetExtension())
       &&   !saveSplittedImagesSeparately)
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
    
    DAVA::ImageSystem* system = DAVA::ImageSystem::Instance();
    if (saveSplittedImagesSeparately)
    {
        DAVA::String directory = filePath.GetDirectory().GetAbsolutePathname();
        DAVA::String baseName = filePath.GetBasename();
        
        system->Save(directory + baseName + "_red.png", channels.red, channels.red->format);
        system->Save(directory + baseName + "_green.png", channels.green, channels.green->format);
        system->Save(directory + baseName + "_blue.png", channels.blue, channels.blue->format);
        system->Save(directory + baseName + "_alpha.png", channels.alpha, channels.alpha->format);
    }
    else
    {
        DAVA::Image* mergedImage = ImageTools::CreateMergedImage(channels);
        system->Save(filePath, mergedImage, mergedImage->format);
        DAVA::SafeRelease(mergedImage);
        ui->path->SetPath(QString::fromStdString(filePath.GetAbsolutePathname()));
    }
    QMessageBox::information(this, "Save succes", "Saved!", QMessageBox::Ok);
}

DAVA::String ImageSplitterDialog::GetDefaultPath() const
{
    return ProjectManager::Instance()->GetProjectPath().GetAbsolutePathname();
}
