#ifndef __DAVAENGINE_DDS_HANDLERS_H__
#define __DAVAENGINE_DDS_HANDLERS_H__

#include "Base/BaseTypes.h"
#include "Render/Image/ImageFormatInterface.h"

namespace DAVA
{
class Image;

struct DDSReader;
struct DDSWriter;

struct DDSReader
{
    virtual ~DDSReader() = default;
    virtual ImageInfo GetImageInfo() = 0;
    virtual bool GetImages(Vector<Image*>& images, const ImageSystem::LoadingParams& loadingParams) = 0;
    virtual bool GetCRC(uint32& crc) const = 0;
    virtual bool AddCRC() = 0;

    static std::unique_ptr<DDSReader> CreateReader(const ScopedPtr<File>& file);
};

struct DDSWriter
{
    virtual ~DDSWriter() = default;
    virtual bool Write(const Vector<Vector<Image*>>& images, PixelFormat dstFormat) = 0;

    static std::unique_ptr<DDSWriter> CreateWriter(const ScopedPtr<File>& file);
};

} // namespace DAVA

#endif // __DAVAENGINE_DDS_HANDLERS_H__
