#include "ImageArea.h"

#include "Render/PixelFormatDescriptor.h"

#include "TextureBrowser/TextureConvertor.h"
#include "SizeDialog.h"
#include "Project/ProjectManager.h"
#include "Main/QtUtils.h"
#include "Settings/SettingsManager.h"

#include "Tools/PathDescriptor/PathDescriptor.h"
#include "ImageTools/ImageTools.h"

#include "QtTools/FileDialog/FileDialog.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>
#include <QMimeData>

ImageArea::ImageArea(QWidget* parent /*= 0*/)
    : QLabel(parent)
    , image(nullptr)
    , acceptableSize(0, 0)
    , imagePath(SettingsManager::Instance()->GetValue(Settings::Internal_ImageSplitterPathSpecular).AsString())
    , requestedFormat(DAVA::FORMAT_A8)
{
    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    setAcceptDrops(true);
    setAutoFillBackground(true);
    ConnectSignals();
    ClearArea();
}

void ImageArea::dragEnterEvent(QDragEnterEvent* event)
{
    const QMimeData* mimeData = event->mimeData();

    if (!mimeData->hasFormat("text/uri-list"))
    {
        return;
    }

    DAVA::ScopedPtr<DAVA::Image> image(DAVA::ImageSystem::LoadSingleMip(mimeData->urls().first().toLocalFile().toStdString()));
    if (image)
    {
        event->acceptProposedAction();
    }
}

void ImageArea::dropEvent(QDropEvent* event)
{
    SetImage(event->mimeData()->urls().first().toLocalFile().toStdString());
    event->acceptProposedAction();
}

void ImageArea::ConnectSignals()
{
    connect(this, SIGNAL(changed()), this, SLOT(UpdatePreviewPicture()));
}

DAVA::String ImageArea::GetDefaultPath() const
{
    return ProjectManager::Instance()->GetProjectPath().GetAbsolutePathname();
}

void ImageArea::mousePressEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton)
    {
        DAVA::FilePath defaultPath = SettingsManager::Instance()->GetValue(Settings::Internal_ImageSplitterPathSpecular).AsString();
        if (defaultPath.IsEmpty())
        {
            defaultPath = SettingsManager::Instance()->GetValue(Settings::Internal_ImageSplitterPath).AsString();
            if (defaultPath.IsEmpty())
            {
                defaultPath = GetDefaultPath();
            }
        }

        DAVA::String retString = FileDialog::getOpenFileName(this, "Select image",
                                                             defaultPath.GetAbsolutePathname().c_str(),
                                                             PathDescriptor::GetPathDescriptor(PathDescriptor::PATH_IMAGE).fileFilter
                                                             )
                                 .toStdString();
        if (!retString.empty())
        {
            SetImage(retString);
        }
    }

    QLabel::mousePressEvent(ev);
}
void ImageArea::SetImage(const DAVA::ScopedPtr<DAVA::Image>& selectedImage)
{
    DAVA::Vector2 selectedImageSize(selectedImage->GetWidth(), selectedImage->GetHeight());
    if (acceptableSize.IsZero())
    {
        acceptableSize = selectedImageSize;
    }
    if (selectedImageSize == acceptableSize)
    {
        image = selectedImage;
        emit changed();
    }
    else
    {
        QMessageBox::warning(this, "Size error", "Selected image has incorrect size.", QMessageBox::Ok);
    }
}

void ImageArea::SetImage(const DAVA::FilePath& filePath)
{
    DAVA::ScopedPtr<DAVA::Image> image(DAVA::ImageSystem::LoadSingleMip(filePath));
    if (!image)
    {
        QMessageBox::warning(this, "File error", "Can't load image.", QMessageBox::Ok);
        return;
    }

    if ((DAVA::FORMAT_INVALID == requestedFormat) || (image->format == requestedFormat))
    {
        const DAVA::FilePath path = filePath;
        SettingsManager::Instance()->SetValue(Settings::Internal_ImageSplitterPathSpecular, DAVA::VariantType(path.GetAbsolutePathname()));
        imagePath = filePath;
        SetImage(image);
    }
    else
    {
        QMessageBox::warning(this, "Format error", QString("Selected image must be in %1 format.").arg(DAVA::PixelFormatDescriptor::GetPixelFormatString(requestedFormat)), QMessageBox::Ok);
    }
}

void ImageArea::ClearArea()
{
    image.reset();
    setBackgroundRole(QPalette::Dark);
    setPixmap(QPixmap());
    acceptableSize.SetZero();
    emit changed();
}

void ImageArea::UpdatePreviewPicture()
{
    if (image)
    {
        DAVA::Image* scaledImage = DAVA::Image::CopyImageRegion(image, image->GetWidth(), image->GetHeight());
        scaledImage->ResizeImage(this->width(), this->height());
        QPixmap scaledPixmap = QPixmap::fromImage(ImageTools::FromDavaImage(scaledImage));
        DAVA::SafeRelease(scaledImage);
        setPixmap(scaledPixmap);
    }
}

void ImageArea::SetAcceptableSize(const DAVA::Vector2& newSize)
{
    acceptableSize = newSize;
}

DAVA::Vector2 ImageArea::GetAcceptableSize() const
{
    return acceptableSize;
}

DAVA::FilePath const& ImageArea::GetImagePath() const
{
    return imagePath;
}

void ImageArea::SetRequestedImageFormat(const DAVA::PixelFormat format)
{
    requestedFormat = format;
}

DAVA::PixelFormat ImageArea::GetRequestedImageFormat() const
{
    return requestedFormat;
}
