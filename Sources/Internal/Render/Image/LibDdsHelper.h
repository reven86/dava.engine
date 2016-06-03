#pragma once

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"
#include "Render/Image/ImageFormatInterface.h"

namespace DAVA
{
class File;
class Image;

class LibDdsHelper : public ImageFormatInterface
{
public:
    LibDdsHelper();

    static bool AddCRCIntoMetaData(const FilePath& filePathname);
    static uint32 GetCRCFromMetaData(const FilePath& filePathname);

    eErrorCode ReadFile(const ScopedPtr<File>& infile, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams) const override;

    eErrorCode WriteFile(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;
    eErrorCode WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;

    ImageInfo GetImageInfo(const ScopedPtr<File>& infile) const override;

    static bool DecompressImageToRGBA(const DAVA::Image& image, Vector<DAVA::Image*>& imageSet, bool forceSoftwareConvertation = false);

    static bool CanCompressTo(PixelFormat format);
    static bool CompressFromRGBA(const Image* image, Image* dstImage);

    static bool CanDecompressFrom(PixelFormat format);
    static bool DecompressToRGBA(const Image* image, Image* dstImage);

protected:
    bool CanProcessFileInternal(const ScopedPtr<File>& infile) const override;

    static eErrorCode ReadFile(const ScopedPtr<File>& file, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams, bool forceSoftwareConvertation = false);

private:
    eErrorCode WriteFileInternal(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const;
};
};
