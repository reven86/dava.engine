#ifndef __DAVAENGINE_IMAGE_SYSTEM_H__
#define __DAVAENGINE_IMAGE_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/File.h"
#include "Render/Image/Image.h"
#include <memory>

namespace DAVA
{
class ImageFormatInterface;

class ImageSystem : public Singleton<ImageSystem>
{
public:
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

    ImageSystem();

    eErrorCode Load(const FilePath& pathname, Vector<Image*>& imageSet, const LoadingParams& loadingParams = LoadingParams()) const;
    eErrorCode Load(File* file, Vector<Image*>& imageSet, const LoadingParams& loadingParams = LoadingParams()) const;

    Image* EnsurePowerOf2Image(Image* image) const;
    void EnsurePowerOf2Images(Vector<Image*>& images) const;

    eErrorCode Save(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat = FORMAT_RGBA8888, ImageQuality quality = DEFAULT_IMAGE_QUALITY) const;
    eErrorCode SaveAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat = FORMAT_RGBA8888, ImageQuality quality = DEFAULT_IMAGE_QUALITY) const;
    eErrorCode Save(const FilePath& fileName, Image* image, PixelFormat compressionFormat = FORMAT_RGBA8888, ImageQuality quality = DEFAULT_IMAGE_QUALITY) const;

    ImageFormatInterface* GetImageFormatInterface(ImageFormat fileFormat) const;
    ImageFormatInterface* GetImageFormatInterface(const FilePath& pathName) const;
    ImageFormatInterface* GetImageFormatInterface(File* file) const;

    ImageInfo GetImageInfo(const FilePath& pathName) const;

    const Vector<String>& GetExtensionsFor(ImageFormat format) const;

    ImageFormat GetImageFormatForExtension(const String& extension) const;
    ImageFormat GetImageFormatForExtension(const FilePath& pathname) const;

    ImageFormat GetImageFormatByName(const String& name) const;

    static uint32 GetBaseMipmap(const LoadingParams& sourceImageParams, const LoadingParams& loadingParams);

private:
    ImageInfo GetImageInfo(File* infile) const;

    Array<std::unique_ptr<ImageFormatInterface>, IMAGE_FORMAT_COUNT> wrappers;
};
};



#endif // __DAVAENGINE_IMAGE_SYSTEM_H__
