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

    bool CanProcessFile(const ScopedPtr<File>& infile) const override;

    eErrorCode ReadFile(const ScopedPtr<File>& infile, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams) const override;

    //only RGB888 or A8
    eErrorCode WriteFile(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;

    //only RGB888 or A8
    eErrorCode WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;

    ImageInfo GetImageInfo(const ScopedPtr<File>& infile) const override;
};
};

#endif // __DAVAENGINE_JPEG_HELPER_H__