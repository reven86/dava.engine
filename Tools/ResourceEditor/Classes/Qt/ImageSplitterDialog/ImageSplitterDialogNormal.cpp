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


#include "ImageSplitterDialog/ImageSplitterDialogNormal.h"
#include "ImageTools/ImageTools.h"
#include "Main/mainwindow.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Main/QtUtils.h"

#include <QMessageBox>
#include "ui_ImageSplitterNormal.h"


ImageSplitterDialogNormal::ImageSplitterDialogNormal(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageSplitterNormal())
{
    ui->setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    imageArreas[0] = ui->redImgLbl;
    imageArreas[1] = ui->greenImgLbl;
    imageArreas[2] = ui->blueImgLbl;
    imageArreas[3] = ui->alphaImgLbl;

    for(auto& imageArea : imageArreas)
    {
        imageArea->SetRequestedImageFormat(DAVA::FORMAT_RGBA8888);
    }
    
    connect(ui->saveBtn, &QPushButton::clicked, this, &ImageSplitterDialogNormal::OnSaveClicked);
}

ImageSplitterDialogNormal::~ImageSplitterDialogNormal()
{
}


void ImageSplitterDialogNormal::OnSaveClicked()
{
    auto scene = QtMainWindow::Instance()->GetCurrentScene();
    auto landscape = DAVA::FindLandscape(scene);
    if(nullptr == landscape)
    {
        QMessageBox::warning(this, "Save error", "Scene has no landscape. Cannot create normals.", QMessageBox::Ok);
        return;
    }
    
    for(size_t i = 1; i < imageArreas.size(); ++i)
    {
        auto image = imageArreas[i]->GetImage();
        auto prevImage = imageArreas[i-1]->GetImage();
        
        if((image->GetWidth() != prevImage->GetWidth()) || (image->GetHeight() != prevImage->GetHeight()))
        {
            QMessageBox::warning(this, "Save error", QString( "Images [%1] and [%2] have different size" ).arg(i-1).arg(i), QMessageBox::Ok);
            return;
        }
    }

    auto heightmapPath = landscape->GetHeightmapPathname();

    auto normal1Path(heightmapPath);
    normal1Path.ReplaceFilename("Normal1.png");
    SaveAndReloadNormal(normal1Path, RED, GREEN);

    auto normal2Path(heightmapPath);
    normal2Path.ReplaceFilename("Normal2.png");
    SaveAndReloadNormal(normal2Path, BLUE, ALPHA);
}

void ImageSplitterDialogNormal::SaveAndReloadNormal(const DAVA::FilePath &pathname, int first, int second)
{
    DAVA::ScopedPtr<DAVA::Image> mergedImage(CreateMergedImage(imageArreas[first]->GetImage(), imageArreas[second]->GetImage()));
    SaveImageToFile(mergedImage, pathname);
    
    auto texture = DAVA::Texture::Get(DAVA::TextureDescriptor::GetDescriptorPathname(pathname));
    if(texture)
    {
        texture->Reload();
        texture->Release();
    }
}

DAVA::Image * ImageSplitterDialogNormal::CreateMergedImage(DAVA::Image *firstImage, DAVA::Image *secondImage)
{
    auto mergedImage = Image::Create(firstImage->width, firstImage->height, FORMAT_RGBA8888);

    uint32 size = firstImage->width * firstImage->height;
    auto pixelSize = PixelFormatDescriptor::GetPixelFormatSizeInBytes(FORMAT_RGBA8888);
    DVASSERT(CHANNELS_COUNT == pixelSize);

    for (uint32 i = 0; i < size; ++i)
    {
        auto offset = i * pixelSize;
        
        mergedImage->data[offset + RED] = firstImage->data[offset + RED];
        mergedImage->data[offset + GREEN] = firstImage->data[offset + GREEN];

        mergedImage->data[offset + BLUE] = secondImage->data[offset + RED];
        mergedImage->data[offset + ALPHA] = secondImage->data[offset + GREEN];
    }
    
    return mergedImage;
}


