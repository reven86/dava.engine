#ifndef __DAVAENGINE_IMAGE_FORMAT_INTERFACE_H__
#define __DAVAENGINE_IMAGE_FORMAT_INTERFACE_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/File.h"
#include "Utils/Utils.h"
#include "Render/PixelFormatDescriptor.h"
#include "Render/Image/ImageSystem.h"

namespace DAVA
{
class Image;

class ImageFormatInterface
{
public:
    inline explicit ImageFormatInterface(ImageFormat format, String name, Vector<String> extensions, Vector<PixelFormat> pixelFormats);
    virtual ~ImageFormatInterface() = default;

    inline ImageFormat GetImageFormat() const;
    inline const String& GetFormatName() const;
    inline const Vector<String>& GetExtensions() const;

    inline ImageInfo GetImageInfo(const FilePath& path) const;
    virtual ImageInfo GetImageInfo(const ScopedPtr<File>& infile) const = 0;

    inline bool IsFormatSupported(PixelFormat format) const;
    inline bool IsFileExtensionSupported(const String& extension) const;

    virtual eErrorCode ReadFile(const ScopedPtr<File>& infile, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams) const = 0;

    virtual eErrorCode WriteFile(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const = 0;
    virtual eErrorCode WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const = 0;

    virtual bool CanProcessFile(const ScopedPtr<File>& file) const = 0;

protected:
    ImageFormat imageFormat;
    String name;
    Vector<String> supportedExtensions;
    Vector<PixelFormat> supportedFormats;
};

ImageFormatInterface::ImageFormatInterface(ImageFormat _format, String _name, Vector<String> _extensions, Vector<PixelFormat> _pixelFormats)
    : imageFormat(_format)
    , name(_name)
    , supportedExtensions(std::move(_extensions))
    , supportedFormats(std::move(_pixelFormats))
{
}

ImageFormat ImageFormatInterface::GetImageFormat() const
{
    return imageFormat;
}

ImageInfo ImageFormatInterface::GetImageInfo(const FilePath& path) const
{
    ScopedPtr<File> infile(File::Create(path, File::OPEN | File::READ));
    return (infile ? GetImageInfo(infile) : ImageInfo());
}

inline bool ImageFormatInterface::IsFormatSupported(PixelFormat format) const
{
    return (std::find(supportedFormats.begin(), supportedFormats.end(), format) != supportedFormats.end());
}

inline bool ImageFormatInterface::IsFileExtensionSupported(const String& extension) const
{
    for (Vector<String>::const_iterator it = supportedExtensions.begin(); it != supportedExtensions.end(); ++it)
    {
        const bool isEqual = (CompareCaseInsensitive(*it, extension) == 0);
        if (isEqual)
        {
            return true;
        }
    }

    return false;
}

inline const Vector<String>& ImageFormatInterface::GetExtensions() const
{
    return supportedExtensions;
}

inline const String& ImageFormatInterface::GetFormatName() const
{
    return name;
}
};

#endif // __DAVAENGINE_IMAGE_FORMAT_INTERFACE_H__