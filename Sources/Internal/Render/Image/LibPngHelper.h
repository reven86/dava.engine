#ifndef __DAVAENGINE_LIBPNG_HELPERS_H__
#define __DAVAENGINE_LIBPNG_HELPERS_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "FileSystem/FilePath.h"

#include "Render/Image/ImageSystem.h"
#include "Render/Image/ImageFormatInterface.h"

namespace DAVA
{
class Texture;
class Sprite;
class Image;

class LibPngHelper : public ImageFormatInterface
{
public:
    LibPngHelper();

    bool CanProcessFile(const ScopedPtr<File>& infile) const override;

    eErrorCode ReadFile(const ScopedPtr<File>& infile, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams) const override;
    eErrorCode WriteFile(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;
    eErrorCode WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;

    ImageInfo GetImageInfo(const ScopedPtr<File>& infile) const override;

    static eErrorCode ReadPngFile(File* infile, Image* image, PixelFormat targetFormat = FORMAT_INVALID);
};
}

#endif // __PNG_IMAGE_H__
