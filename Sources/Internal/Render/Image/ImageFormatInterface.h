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
    virtual ~ImageFormatInterface() = default;

    virtual ImageFormat GetImageFormat() const = 0;
    virtual bool CanProcessFile(File* file) const = 0;

    virtual eErrorCode ReadFile(File* infile, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams) const = 0;

    virtual eErrorCode WriteFile(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const = 0;
    virtual eErrorCode WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const = 0;

    virtual ImageInfo GetImageInfo(File* infile) const = 0;
    inline ImageInfo GetImageInfo(const FilePath& path) const;

    inline bool IsFormatSupported(PixelFormat format) const;
    inline bool IsFileExtensionSupported(const String& extension) const;

    inline const Vector<String>& Extensions() const;
    inline const char* Name() const;

protected:
    Vector<PixelFormat> supportedFormats;
    Vector<String> supportedExtensions;
    String name;
};

ImageInfo ImageFormatInterface::GetImageInfo(const FilePath& path) const
{
    File* infile = File::Create(path, File::OPEN | File::READ);
    if (nullptr == infile)
    {
        return ImageInfo();
    }
    ImageInfo info = GetImageInfo(infile);
    infile->Release();
    return info;
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

inline const Vector<String>& ImageFormatInterface::Extensions() const
{
    return supportedExtensions;
}

inline const char* ImageFormatInterface::Name() const
{
    return name.c_str();
}
};

#endif // __DAVAENGINE_IMAGE_FORMAT_INTERFACE_H__