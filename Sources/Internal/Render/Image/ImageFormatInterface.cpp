#include "Render/Image/ImageFormatInterface.h"
#include "FileSystem/File.h"
#include "Utils/Utils.h"

namespace DAVA
{
ImageFormatInterface::ImageFormatInterface(ImageFormat imageFormat_, const String& interfaceName_, const Vector<String>& extensions, const Vector<PixelFormat>& pixelFormats)
    : supportedFormats(pixelFormats)
    , supportedExtensions(extensions)
    , interfaceName(interfaceName_)
    , imageFormat(imageFormat_)
{
}

// ImageFormatInterface::ImageFormatInterface(ImageFormatInterface &&other)
//     : supportedFormats(std::move(other.supportedFormats))
//     , supportedExtensions(std::move(other.supportedExtensions))
//     , interfaceName(std::move(other.interfaceName))
//     , imageFormat(other.imageFormat)
// {
//     other.imageFormat = ImageFormat::IMAGE_FORMAT_UNKNOWN;
// }
// 
// ImageFormatInterface& ImageFormatInterface::operator=(ImageFormatInterface &&other)
// {
//     if (this != &other)
//     {
//         supportedFormats = std::move(other.supportedFormats);
//         supportedExtensions = std::move(other.supportedExtensions);
//         interfaceName = std::move(other.interfaceName);
//         imageFormat = other.imageFormat;
// 
//         other.imageFormat = ImageFormat::IMAGE_FORMAT_UNKNOWN;
//     }
// 
//     return *this;
// }


ImageFormatInterface::~ImageFormatInterface() = default;


ImageFormat ImageFormatInterface::GetImageFormat() const
{
    return imageFormat;
}

ImageInfo ImageFormatInterface::GetImageInfo(const FilePath& path) const
{
    ScopedPtr<File> infile(File::Create(path, File::OPEN | File::READ));
    if (infile)
    {
        ImageInfo info = GetImageInfo(infile);
        return info;
    }
    return ImageInfo();
}

bool ImageFormatInterface::IsFormatSupported(PixelFormat format) const
{
    return (std::find(supportedFormats.begin(), supportedFormats.end(), format) != supportedFormats.end());
}

bool ImageFormatInterface::IsFileExtensionSupported(const String& extension) const
{
    for (const String& ext : supportedExtensions)
    {
        if (CompareCaseInsensitive(ext, extension) == 0)
        {
            return true;
        }
    }

    return false;
}

const Vector<String>& ImageFormatInterface::Extensions() const
{
    return supportedExtensions;
}

const String& ImageFormatInterface::Name() const
{
    return interfaceName;
}

bool ImageFormatInterface::CanProcessFile(const ScopedPtr<File>& file) const
{
    if (!file)
    {
        DVASSERT(false);
        return false;
    }

    DVASSERT(file->GetPos() == 0);

    bool canProcess = CanProcessFileInternal(file);
    file->Seek(0, File::SEEK_FROM_START);
    return canProcess;
}

bool ImageFormatInterface::CanProcessFileInternal(const ScopedPtr<File>& infile) const
{
    return GetImageInfo(infile).dataSize != 0;
}

}
