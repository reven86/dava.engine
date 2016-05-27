#pragma once

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "Render/Image/ImageSystem.h"

namespace DAVA
{
class Image;

class ImageFormatInterface
{
public:
    ImageFormatInterface(ImageFormat imageFormat, const String& interfaceName, const Vector<String>& extensions, const Vector<PixelFormat>& pixelFormats);
//     ImageFormatInterface(ImageFormatInterface &&other);
//     ImageFormatInterface& operator=(ImageFormatInterface &&other);

    virtual ~ImageFormatInterface();

    const String& Name() const;
    ImageInfo GetImageInfo(const FilePath& path) const;
    virtual ImageInfo GetImageInfo(const ScopedPtr<File>& infile) const = 0;

    ImageFormat GetImageFormat() const;
    const Vector<String>& Extensions() const;

    bool CanProcessFile(const ScopedPtr<File>& file) const;

    virtual eErrorCode ReadFile(const ScopedPtr<File>& infile, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams) const = 0;

    virtual eErrorCode WriteFile(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const = 0;
    virtual eErrorCode WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const = 0;

    bool IsFormatSupported(PixelFormat format) const;
    bool IsFileExtensionSupported(const String& extension) const;

protected:

    virtual bool CanProcessFileInternal(const ScopedPtr<File>& file) const;

private:
    Vector<PixelFormat> supportedFormats;
    Vector<String> supportedExtensions;
    String interfaceName;
    ImageFormat imageFormat = ImageFormat::IMAGE_FORMAT_UNKNOWN;
};

}

