#ifndef __DAVAENGINE_IMAGE_SYSTEM_H__
#define __DAVAENGINE_IMAGE_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/File.h"
#include "Render/Image/Image.h"
#include "Functional/Function.h"

namespace DAVA
{
class ImageFormatInterface;

namespace ImageSystem
{
struct LoadingParams
{
    LoadingParams(uint32 w = 0, uint32 h = 0, uint32 mipmap = 0)
        : minimalWidth(w)
        , minimalHeight(h)
        , baseMipmap(mipmap)
    {
    }

    uint32 minimalWidth = 0;
    uint32 minimalHeight = 0;
    uint32 baseMipmap = 0;
};

eErrorCode Load(const FilePath& pathname, Vector<Image*>& imageSet, const LoadingParams& loadingParams = LoadingParams());
eErrorCode Load(const ScopedPtr<File>& file, Vector<Image*>& imageSet, const LoadingParams& loadingParams = LoadingParams());

Image* EnsurePowerOf2Image(Image* image);
void EnsurePowerOf2Images(Vector<Image*>& images);

eErrorCode Save(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat = FORMAT_RGBA8888, ImageQuality quality = DEFAULT_IMAGE_QUALITY);
eErrorCode SaveAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat = FORMAT_RGBA8888, ImageQuality quality = DEFAULT_IMAGE_QUALITY);
eErrorCode Save(const FilePath& fileName, Image* image, PixelFormat compressionFormat = FORMAT_RGBA8888, ImageQuality quality = DEFAULT_IMAGE_QUALITY);

ImageFormatInterface* GetImageFormatInterface(ImageFormat fileFormat);
ImageFormatInterface* GetImageFormatInterface(const FilePath& pathName);
ImageFormatInterface* GetImageFormatInterface(const ScopedPtr<File>& file);

ImageInfo GetImageInfo(const FilePath& pathName);

const Vector<String>& GetExtensionsFor(ImageFormat format);

ImageFormat GetImageFormatForExtension(const String& extension);
ImageFormat GetImageFormatForExtension(const FilePath& pathname);

ImageFormat GetImageFormatByName(const String& name);

uint32 GetBaseMipmap(const LoadingParams& sourceImageParams, const LoadingParams& loadingParams);
}
}

#endif // __DAVAENGINE_IMAGE_SYSTEM_H__
