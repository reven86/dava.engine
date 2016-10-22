#include "Render/Texture.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageConvert.h"
#include "Render/Image/ImageSystem.h"
#include "Render/PixelFormatDescriptor.h"

namespace DAVA
{
Image::Image()
    : dataSize(0)
    , width(0)
    , height(0)
    , data(0)
    , mipmapLevel(-1)
    , format(FORMAT_RGB565)
    , cubeFaceID(Texture::INVALID_CUBEMAP_FACE)
    , customDeleter(nullptr)
{
}

Image::~Image()
{
    if (nullptr != customDeleter)
    {
        customDeleter(data);
    }
    else
    {
        SafeDeleteArray(data);
    }

    width = 0;
    height = 0;
}

uint32 Image::GetSizeInBytes(uint32 width, uint32 height, PixelFormat format)
{
    DVASSERT(width != 0 && height != 0);
    DVASSERT(format != PixelFormat::FORMAT_INVALID);

    Size2i blockSize = PixelFormatDescriptor::GetPixelFormatBlockSize(format);
    if (blockSize.dx != 1 || blockSize.dy != 1)
    { // mathematics from PVR SDK
        width = width + ((-1 * width) % blockSize.dx);
        height = height + ((-1 * height) % blockSize.dy);
    }

    uint32 bitsPerPixel = PixelFormatDescriptor::GetPixelFormatSizeInBits(format);
    return (bitsPerPixel * width * height / 8);
}

Image* Image::Create(uint32 width, uint32 height, PixelFormat format)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    uint32 size = GetSizeInBytes(width, height, format);
    if (size > 0)
    {
        Image* image = new Image();
        image->width = width;
        image->height = height;
        image->format = format;
        image->dataSize = size;
        image->data = new uint8[image->dataSize];
        return image;
    }

    Logger::Error("[Image::Create] trying to create image with wrong format");
    return nullptr;
}

Image* Image::CreateFromData(uint32 width, uint32 height, PixelFormat format, const uint8* data)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    Image* image = Image::Create(width, height, format);
    if (image != nullptr && data != nullptr)
    {
        Memcpy(image->data, data, image->dataSize);
    }

    return image;
}

Image* Image::CreatePinkPlaceholder(bool checkers)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    Image* image = Image::Create(16, 16, FORMAT_RGBA8888);
    image->MakePink(checkers);

    return image;
}

void Image::MakePink(bool checkers)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    if (data == nullptr)
        return;

    uint32 pink = 0xffff00ff;
    uint32 gray = (checkers) ? 0xffffff00 : 0xffff00ff;
    bool pinkOrGray = false;

    uint32* writeData = reinterpret_cast<uint32*>(data);
    for (uint32 w = 0; w < width; ++w)
    {
        pinkOrGray = !pinkOrGray;
        for (uint32 h = 0; h < height; ++h)
        {
            *writeData++ = pinkOrGray ? pink : gray;
            pinkOrGray = !pinkOrGray;
        }
    }
}

bool Image::Normalize()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    const int32 formatSize = PixelFormatDescriptor::GetPixelFormatSizeInBytes(format);
    const uint32 pitch = width * formatSize;
    const uint32 dataSize = height * pitch;

    uint8* newImage0Data = new uint8[dataSize];
    Memset(newImage0Data, 0, dataSize);
    bool normalized = ImageConvert::Normalize(format, data, width, height, pitch, newImage0Data);

    SafeDeleteArray(data);
    data = newImage0Data;

    return normalized;
}

Vector<Image*> Image::CreateMipMapsImages(bool isNormalMap /* = false */)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    Vector<Image*> imageSet;

    int32 formatSize = PixelFormatDescriptor::GetPixelFormatSizeInBytes(format);
    if (!formatSize)
        return imageSet;

    Image* image0 = SafeRetain(this);
    uint32 imageWidth = width;
    uint32 imageHeight = height;
    uint32 curMipMapLevel = 0;
    image0->mipmapLevel = curMipMapLevel++;

    if (isNormalMap)
        image0->Normalize();

    imageSet.push_back(image0);
    while (imageHeight > 1 || imageWidth > 1)
    {
        uint32 newWidth = imageWidth;
        uint32 newHeight = imageHeight;
        if (newWidth > 1)
            newWidth >>= 1;
        if (newHeight > 1)
            newHeight >>= 1;

        Image* halfSizeImg = Image::Create(newWidth, newHeight, format);
        imageSet.push_back(halfSizeImg);

        Memset(halfSizeImg->GetData(), 0, halfSizeImg->dataSize);

        bool downScaled = ImageConvert::DownscaleTwiceBillinear(format, format,
                                                                image0->data, imageWidth, imageHeight, imageWidth * formatSize,
                                                                halfSizeImg->GetData(), newWidth, newHeight, newWidth * formatSize, isNormalMap);

        if (downScaled)
        {
            halfSizeImg->cubeFaceID = image0->cubeFaceID;
            halfSizeImg->mipmapLevel = curMipMapLevel;

            imageWidth = newWidth;
            imageHeight = newHeight;

            image0 = halfSizeImg;
            curMipMapLevel++;
        }
        else
        {
            for (Image* img : imageSet)
            {
                img->Release();
            }
            imageSet.clear();
            break;
        }
    }

    return imageSet;
}

bool Image::ResizeImage(uint32 newWidth, uint32 newHeight)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    const PixelFormatDescriptor& formatDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(format);
    if (formatDescriptor.isCompressed)
        return false;

    int32 formatSizeInBytes = formatDescriptor.pixelSize / 8;
    const uint32 newDataSize = GetSizeInBytes(newWidth, newHeight, format);

    uint8* newData = new uint8[newDataSize];
    Memset(newData, 0, newDataSize);

    float32 kx = float32(width) / float32(newWidth);
    float32 ky = float32(height) / float32(newHeight);

    float32 xx = 0, yy = 0;
    uint32 offset = 0;
    uint32 offsetOld = 0;
    uint32 posX, posY;
    for (uint32 y = 0; y < newHeight; ++y)
    {
        for (uint32 x = 0; x < newWidth; ++x)
        {
            posX = uint32(xx + 0.5f);
            posY = uint32(yy + 0.5f);
            if (posX >= width)
                posX = width - 1;

            if (posY >= height)
                posY = height - 1;

            offsetOld = (posY * width + posX) * formatSizeInBytes;
            Memcpy(newData + offset, data + offsetOld, formatSizeInBytes);

            xx += kx;
            offset += formatSizeInBytes;
        }
        yy += ky;
        xx = 0;
    }

    // resized data
    width = newWidth;
    height = newHeight;

    SafeDeleteArray(data);
    data = newData;
    dataSize = newDataSize;

    return true;
}

void Image::ResizeCanvas(uint32 newWidth, uint32 newHeight)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    uint8* newData = NULL;
    uint32 newDataSize = 0;
    int32 formatSize = PixelFormatDescriptor::GetPixelFormatSizeInBytes(format);

    if (formatSize > 0)
    {
        newDataSize = newWidth * newHeight * formatSize;
        newData = new uint8[newDataSize];
        Memset(newData, 0, newDataSize);

        uint32 currentLine = 0;
        uint32 indexOnLine = 0;
        uint32 indexInOldData = 0;

        for (uint32 i = 0; i < newDataSize; ++i)
        {
            if ((currentLine + 1) * newWidth * formatSize <= i)
            {
                currentLine++;
            }

            indexOnLine = i - currentLine * newWidth * formatSize;

            if (currentLine < uint32(height))
            {
                // within height of old image
                if (indexOnLine < uint32(width * formatSize))
                {
                    // we have data in old image for new image
                    indexInOldData = currentLine * width * formatSize + indexOnLine;
                    newData[i] = data[indexInOldData];
                }
                else
                {
                    newData[i] = 0;
                }
            }
            else
            {
                newData[i] = 0;
            }
        }

        // resized data
        width = newWidth;
        height = newHeight;

        SafeDeleteArray(data);
        data = newData;
        dataSize = newDataSize;
    }
}

void Image::ResizeToSquare()
{
    uint32 newImageSize = Max(width, height);
    ResizeCanvas(newImageSize, newImageSize);
}

Image* Image::CopyImageRegion(const Image* imageToCopy,
                              uint32 newWidth, uint32 newHeight,
                              uint32 xOffset, uint32 yOffset)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    uint32 oldWidth = imageToCopy->GetWidth();
    uint32 oldHeight = imageToCopy->GetHeight();
    DVASSERT((newWidth + xOffset) <= oldWidth && (newHeight + yOffset) <= oldHeight);

    PixelFormat format = imageToCopy->GetPixelFormat();
    int32 formatSize = PixelFormatDescriptor::GetPixelFormatSizeInBytes(format);

    Image* newImage = Image::Create(newWidth, newHeight, format);

    uint8* oldData = imageToCopy->GetData();
    uint8* newData = newImage->data;

    for (uint32 i = 0; i < newHeight; ++i)
    {
        Memcpy((newData + newWidth * i * formatSize),
               (oldData + (oldWidth * (yOffset + i) + xOffset) * formatSize),
               formatSize * newWidth);
    }

    return newImage;
}

Image* Image::CopyImageRegion(const Image* imageToCopy, const Rect& rect)
{
    return CopyImageRegion(imageToCopy, uint32(rect.dx), uint32(rect.dy), uint32(rect.x), uint32(rect.y));
}

void Image::InsertImage(const Image* image, uint32 dstX, uint32 dstY,
                        uint32 srcX /* = 0 */, uint32 srcY /* = 0 */,
                        uint32 srcWidth /* = -1 */, uint32 srcHeight /* = -1 */)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    if (image == nullptr)
    {
        return;
    }

    if (GetPixelFormat() != image->GetPixelFormat())
    {
        return;
    }

    if (dstX >= width || dstY >= height ||
        srcX >= image->GetWidth() || srcY >= image->GetHeight())
    {
        return;
    }

    uint32 insertWidth = (srcWidth == uint32(-1)) ? image->GetWidth() : srcWidth;
    uint32 insertHeight = (srcHeight == uint32(-1)) ? image->GetHeight() : srcHeight;

    if (srcX + insertWidth > image->GetWidth())
    {
        insertWidth = image->GetWidth() - srcX;
    }
    if (dstX + insertWidth > width)
    {
        insertWidth = width - dstX;
    }

    if (srcY + insertHeight > image->GetHeight())
    {
        insertHeight = image->GetHeight() - srcY;
    }
    if (dstY + insertHeight > height)
    {
        insertHeight = height - dstY;
    }

    PixelFormat format = GetPixelFormat();
    int32 formatSize = PixelFormatDescriptor::GetPixelFormatSizeInBytes(format);

    uint8* srcData = image->GetData();
    uint8* dstData = data;

    for (uint32 i = 0; i < insertHeight; ++i)
    {
        Memcpy(dstData + (width * (dstY + i) + dstX) * formatSize,
               srcData + (image->GetWidth() * (srcY + i) + srcX) * formatSize,
               formatSize * insertWidth);
    }
}

void Image::InsertImage(const Image* image, const Vector2& dstPos, const Rect& srcRect)
{
    InsertImage(image, uint32(dstPos.x), uint32(dstPos.y), uint32(srcRect.x), uint32(srcRect.y), uint32(srcRect.dx), uint32(srcRect.dy));
}

bool Image::Save(const FilePath& path) const
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();
    return ImageSystem::Save(path, const_cast<Image*>(this), format) == eErrorCode::SUCCESS;
}

void Image::FlipHorizontal()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    switch (format)
    {
    case FORMAT_A8:
        FlipHorizontal(reinterpret_cast<uint8*>(data), width, height);
        break;

    case FORMAT_A16:
    case FORMAT_RGBA5551:
    case FORMAT_RGBA4444:
    case FORMAT_RGB565:
        FlipHorizontal(reinterpret_cast<uint16*>(data), width, height);
        break;

    case FORMAT_RGBA8888:
        FlipHorizontal(reinterpret_cast<uint32*>(data), width, height);
        break;

    case FORMAT_RGB888:
        FlipHorizontal(reinterpret_cast<RGB888*>(data), width, height);
        break;

    default:
        DVASSERT(false && "Not implemented");
        break;
    }
}

void Image::FlipVertical()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    switch (format)
    {
    case FORMAT_A8:
        FlipVertical(reinterpret_cast<uint8*>(data), width, height);
        break;

    case FORMAT_A16:
    case FORMAT_RGBA5551:
    case FORMAT_RGBA4444:
    case FORMAT_RGB565:
        FlipVertical(reinterpret_cast<uint16*>(data), width, height);
        break;

    case FORMAT_RGBA8888:
        FlipVertical(reinterpret_cast<uint32*>(data), width, height);
        break;

    case FORMAT_RGB888:
        FlipVertical(reinterpret_cast<RGB888*>(data), width, height);
        break;

    default:
        DVASSERT(false && "Not implemented");
        break;
    }
}

void Image::RotateDeg(int degree)
{
    degree %= 360;

    switch (degree)
    {
    case 0:
        return;
    case 90:
    case -270:
        Rotate90Right();
        return;
    case 180:
    case -180:
        FlipHorizontal();
        FlipVertical();
        return;
    case 270:
    case -90:
        Rotate90Left();
        return;
    default:
        DVASSERT(false && "unsupported rotate degree");
        return;
    }
}

void Image::Rotate90Right()
{
    DVASSERT((width == height) && "image should be square to rotate");
    DVASSERT(dataSize && "image is empty or dataSize is not set");

    uint8* newData = new uint8[dataSize];

    switch (format)
    {
    case FORMAT_A8:
        Rotate90Right(reinterpret_cast<uint8*>(data), reinterpret_cast<uint8*>(newData), height);
        break;

    case FORMAT_A16:
    case FORMAT_RGBA5551:
    case FORMAT_RGBA4444:
    case FORMAT_RGB565:
        Rotate90Right(reinterpret_cast<uint16*>(data), reinterpret_cast<uint16*>(newData), height);
        break;

    case FORMAT_RGBA8888:
        Rotate90Right(reinterpret_cast<uint32*>(data), reinterpret_cast<uint32*>(newData), height);
        break;

    case FORMAT_RGB888:
        Rotate90Right(reinterpret_cast<RGB888*>(data), reinterpret_cast<RGB888*>(newData), height);
        break;

    default:
        DVASSERT(false && "Not implemented");
        break;
    }

    SafeDeleteArray(data);
    data = newData;
}

void Image::Rotate90Left()
{
    DVASSERT((width == height) && "image should be square to rotate");
    DVASSERT(dataSize && "image is empty or dataSize is not set");

    uint8* newData = new uint8[dataSize];

    switch (format)
    {
    case FORMAT_A8:
        Rotate90Left(reinterpret_cast<uint8*>(data), reinterpret_cast<uint8*>(newData), height);
        break;

    case FORMAT_A16:
    case FORMAT_RGBA5551:
    case FORMAT_RGBA4444:
    case FORMAT_RGB565:
        Rotate90Left(reinterpret_cast<uint16*>(data), reinterpret_cast<uint16*>(newData), height);
        break;

    case FORMAT_RGBA8888:
        Rotate90Left(reinterpret_cast<uint32*>(data), reinterpret_cast<uint32*>(newData), height);
        break;

    case FORMAT_RGB888:
        Rotate90Left(reinterpret_cast<RGB888*>(data), reinterpret_cast<RGB888*>(newData), height);
        break;

    default:
        DVASSERT(false && "Not implemented");
        break;
    }

    SafeDeleteArray(data);
    data = newData;
}
};
