#include "ImageTools/ImageTools.h"

#include "TextureCompression/TextureConverter.h"

#include "Base/GlobalEnum.h"
#include "Render/GPUFamilyDescriptor.h"
#include "Render/PixelFormatDescriptor.h"
#include "Render/Image/ImageConvert.h"

#include "Main/QtUtils.h"

using namespace DAVA;

void Channels::ReleaseImages()
{
    DAVA::SafeRelease(red);
    DAVA::SafeRelease(green);
    DAVA::SafeRelease(blue);
    DAVA::SafeRelease(alpha);
}

namespace ImageTools
{
void SaveImage(Image* image, const FilePath& pathname)
{
    ImageSystem::Save(pathname, image, image->format);
}

Image* LoadImage(const FilePath& pathname)
{
    return ImageSystem::LoadSingleMip(pathname);
}

uint32 GetTexturePhysicalSize(const TextureDescriptor* descriptor, const eGPUFamily forGPU, uint32 baseMipMaps)
{
    uint32 size = 0;

    Vector<FilePath> files;

    if (descriptor->IsCubeMap() && forGPU == GPU_ORIGIN)
    {
        Vector<FilePath> faceNames;
        descriptor->GetFacePathnames(faceNames);

        files.reserve(faceNames.size());
        for (auto& faceName : faceNames)
        {
            if (!faceName.IsEmpty())
                files.push_back(faceName);
        }
    }
    else
    {
        files = descriptor->CreateLoadPathnamesForGPU(forGPU);
    }

    for (size_t i = 0; i < files.size(); ++i)
    {
        const FilePath& imagePathname = files[i];
        ImageInfo info = ImageSystem::GetImageInfo(imagePathname);
        if (!info.IsEmpty())
        {
            const auto formatSizeBits = PixelFormatDescriptor::GetPixelFormatSizeInBits(info.format);

            auto m = Min(baseMipMaps, info.mipmapsCount - 1);
            for (; m < info.mipmapsCount; ++m)
            {
                const auto w = (info.width >> m);
                const auto h = (info.height >> m);

                size += (w * h * formatSizeBits / 8);
            }
        }
        else
        {
            Logger::Error("ImageTools::[GetTexturePhysicalSize] Can't detect type of file %s", imagePathname.GetStringValue().c_str());
        }
    }

    return size;
}

void ConvertImage(const DAVA::TextureDescriptor* descriptor, const DAVA::eGPUFamily forGPU, DAVA::TextureConverter::eConvertQuality quality)
{
    if (!descriptor || descriptor->compression[forGPU].format == FORMAT_INVALID)
    {
        return;
    }

    TextureConverter::ConvertTexture(*descriptor, forGPU, true, quality);
}

bool SplitImage(const FilePath& pathname)
{
    ScopedPtr<Image> loadedImage(DAVA::ImageSystem::LoadSingleMip(pathname));
    if (!loadedImage)
    {
        Logger::Error("Can't load image %s", pathname.GetAbsolutePathname().c_str());
        return false;
    }

    if (loadedImage->GetPixelFormat() != FORMAT_RGBA8888)
    {
        Logger::Error("Incorrect image format %s. Must be RGBA8888", PixelFormatDescriptor::GetPixelFormatString(loadedImage->GetPixelFormat()));
        return false;
    }

    Channels channels = CreateSplittedImages(loadedImage);

    FilePath folder(pathname.GetDirectory());

    SaveImage(channels.red, folder + "r.png");
    SaveImage(channels.green, folder + "g.png");
    SaveImage(channels.blue, folder + "b.png");
    SaveImage(channels.alpha, folder + "a.png");

    channels.ReleaseImages();
    return true;
}

bool MergeImages(const FilePath& folder)
{
    DVASSERT(folder.IsDirectoryPathname());

    Channels channels(LoadImage(folder + "r.png"), LoadImage(folder + "g.png"), LoadImage(folder + "b.png"), LoadImage(folder + "a.png"));

    if (channels.IsEmpty())
    {
        Logger::Error("Can't load one or more channel images from folder %s", folder.GetAbsolutePathname().c_str());
        channels.ReleaseImages();
        return false;
    }

    if (!channels.HasFormat(FORMAT_A8))
    {
        Logger::Error("Can't merge images. Source format must be Grayscale 8bit");
        channels.ReleaseImages();
        return false;
    }

    if (!channels.ChannelesResolutionEqual())
    {
        Logger::Error("Can't merge images. Source images must have same size");
        channels.ReleaseImages();
        return false;
    }

    Image* mergedImage = CreateMergedImage(channels);

    ImageSystem::Save(folder + "merged.png", mergedImage);
    channels.ReleaseImages();
    SafeRelease(mergedImage);
    return true;
}

Channels CreateSplittedImages(DAVA::Image* originalImage)
{
    DAVA::Image* r = Image::Create(originalImage->width, originalImage->height, FORMAT_A8);
    DAVA::Image* g = Image::Create(originalImage->width, originalImage->height, FORMAT_A8);
    DAVA::Image* b = Image::Create(originalImage->width, originalImage->height, FORMAT_A8);
    DAVA::Image* a = Image::Create(originalImage->width, originalImage->height, FORMAT_A8);

    int32 size = originalImage->width * originalImage->height;
    int32 pixelSize = PixelFormatDescriptor::GetPixelFormatSizeInBytes(FORMAT_RGBA8888);
    for (int32 i = 0; i < size; ++i)
    {
        int32 offset = i * pixelSize;
        r->data[i] = originalImage->data[offset];
        g->data[i] = originalImage->data[offset + 1];
        b->data[i] = originalImage->data[offset + 2];
        a->data[i] = originalImage->data[offset + 3];
    }
    return Channels(r, g, b, a);
}

DAVA::Image* CreateMergedImage(const Channels& channels)
{
    if (!channels.ChannelesResolutionEqual() || !channels.HasFormat(FORMAT_A8))
    {
        return nullptr;
    }
    Image* mergedImage = Image::Create(channels.red->width, channels.red->height, FORMAT_RGBA8888);
    int32 size = mergedImage->width * mergedImage->height;
    int32 pixelSize = PixelFormatDescriptor::GetPixelFormatSizeInBytes(FORMAT_RGBA8888);
    for (int32 i = 0; i < size; ++i)
    {
        int32 offset = i * pixelSize;
        mergedImage->data[offset] = channels.red->data[i];
        mergedImage->data[offset + 1] = channels.green->data[i];
        mergedImage->data[offset + 2] = channels.blue->data[i];
        mergedImage->data[offset + 3] = channels.alpha->data[i];
    }
    return mergedImage;
}

void SetChannel(DAVA::Image* image, eComponentsRGBA channel, DAVA::uint8 value)
{
    if (image->format != FORMAT_RGBA8888)
    {
        return;
    }
    int32 size = image->width * image->height;
    int32 pixelSize = PixelFormatDescriptor::GetPixelFormatSizeInBytes(FORMAT_RGBA8888);
    int32 offset = channel;
    for (int32 i = 0; i < size; ++i, offset += pixelSize)
    {
        image->data[offset] = value;
    }
}

QImage FromDavaImage(const DAVA::FilePath& pathname)
{
    auto image = LoadImage(pathname);
    if (image)
    {
        QImage img = FromDavaImage(image);
        SafeRelease(image);

        return img;
    }

    return QImage();
}

QImage FromDavaImage(const Image* image)
{
    DVASSERT(image != nullptr);

    if (image->format == FORMAT_RGBA8888)
    {
        QImage qtImage(image->width, image->height, QImage::Format_RGBA8888);
        Memcpy(qtImage.bits(), image->data, image->dataSize);
        return qtImage;
    }
    else if (ImageConvert::CanConvertFromTo(image->format, FORMAT_RGBA8888))
    {
        ScopedPtr<Image> newImage(Image::Create(image->width, image->height, FORMAT_RGBA8888));
        bool converted = ImageConvert::ConvertImage(image, newImage);
        if (converted)
        {
            return FromDavaImage(newImage);
        }
        else
        {
            Logger::Error("[%s]: Error converting from %s", __FUNCTION__, GlobalEnumMap<PixelFormat>::Instance()->ToString(image->format));
            return QImage();
        }
    }
    else
    {
        Logger::Error("[%s]: Converting from %s is not implemented", __FUNCTION__, GlobalEnumMap<PixelFormat>::Instance()->ToString(image->format));
        return QImage();
    }
}

} // namespace ImageTools
