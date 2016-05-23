#ifndef __DAVAENGINE_JPEG_HELPER_H__
#define __DAVAENGINE_JPEG_HELPER_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"
#include "Render/Image/ImageFormatInterface.h"

namespace DAVA
{
class LibJpegHelper : public ImageFormatInterface
{
public:
    LibJpegHelper();

    ImageFormat GetImageFormat() const override;

    bool CanProcessFile(File* file) const override;

    eErrorCode ReadFile(File* infile, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams) const override;

    //only RGB888 or A8
    eErrorCode WriteFile(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;

    //only RGB888 or A8
    eErrorCode WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;

    ImageInfo GetImageInfo(File* infile) const override;
};

inline ImageFormat LibJpegHelper::GetImageFormat() const
{
    return IMAGE_FORMAT_JPEG;
}
};

#endif // __DAVAENGINE_JPEG_HELPER_H__