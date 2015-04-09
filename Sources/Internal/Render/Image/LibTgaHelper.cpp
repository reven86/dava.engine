/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Render/Image/LibTgaHelper.h"

#include "FileSystem/File.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageConvert.h"


namespace DAVA
{

static const uint8 MAX_BYTES_IN_PIXEL = 16;


LibTgaWrapper::LibTgaWrapper()
{
    supportedExtensions.emplace_back(".tga");
    supportedExtensions.emplace_back(".tpic");
}

bool LibTgaWrapper::IsImage(File *infile) const
{
    return GetDataSize(infile) != 0;
}

uint32 LibTgaWrapper::GetDataSize(File *infile) const
{
    DVASSERT(infile);

    TgaInfo tgaInfo;

    infile->Seek(0, File::SEEK_FROM_START);
    eErrorCode readResult = ReadTgaHeader(infile, tgaInfo);
    infile->Seek(0, File::SEEK_FROM_START);

    if (readResult == SUCCESS)
        return tgaInfo.width * tgaInfo.height * tgaInfo.bytesPerPixel;
    else
        return 0;
}

enum TgaHeaderSpec{
    idlengthOffset = 0,             // 1 byte, should be 0
    colorMapTypeOffset = 1,         // 1 byte, should be 0
    imageTypeOffset = 2,            // 1 byte, can be 2,3,10,11
    colorMapDataOffset = 3,         // 5 bytes, should be 0,0,0,0,0
    originXOffset = 8,              // 2 bytes, should be 0 for bottomleft etc., skip this
    originYOffset = 10,             // 2 bytes, should be 0 for bottomleft etc., skip this
    widthOffset = 12,               // 2 bytes, image width in pixels
    heightOffset = 14,              // 2 bytes, image height in pixels
    bppOffset = 16,                 // 1 byte, bitsPerPixel, can be 8,16,24,32,64,128
    descriptorOffset = 17           // 1 byte, image origin corner and number of alpha bits
};

uint16 LowEndianToUint16(const uint8* p)
{
    return (uint16)(p[0] | ((uint16)(p[1]) << 8));
}

void Uint16ToLowEndian(const uint16 u16, uint8* p)
{
    p[0] = u16 & 0x00FF;
    p[1] = u16 >> 8;
}

DAVA::eErrorCode LibTgaWrapper::ReadTgaHeader(File *infile, TgaInfo& tgaInfo) const
{
    std::array<uint8, 18> fields;
    size_t bytesRead = infile->Read(&fields, fields.size());
    if (bytesRead != fields.size())
        return ERROR_READ_FAIL;

    static const std::array<uint8, 5> zeroes = {{ 0, 0, 0, 0, 0 }};
    if (Memcmp(&fields[idlengthOffset], &zeroes[0], 2) != 0 ||
        Memcmp(&fields[colorMapDataOffset], &zeroes[0], 5) != 0)
    {
        return ERROR_FILE_FORMAT_INCORRECT;
    }

    // obtaining imageType
    tgaInfo.imageType = static_cast<TgaInfo::IMAGE_TYPE>(fields[imageTypeOffset]);
    if (tgaInfo.imageType != TgaInfo::GRAYSCALE &&
        tgaInfo.imageType != TgaInfo::TRUECOLOR &&
        tgaInfo.imageType != TgaInfo::COMPRESSED_GRAYSCALE &&
        tgaInfo.imageType != TgaInfo::COMPRESSED_TRUECOLOR)
    {
        return ERROR_FILE_FORMAT_INCORRECT;
    }

    // obtaining width and height
    tgaInfo.width = LowEndianToUint16(&fields[widthOffset]);    //tga uses low endianness
    tgaInfo.height = LowEndianToUint16(&fields[heightOffset]);  //tga uses low endianness
    if (!tgaInfo.width || !tgaInfo.height)
    {
        return ERROR_FILE_FORMAT_INCORRECT;
    }

    // obtaining bytes per pixel
    tgaInfo.bytesPerPixel = fields[bppOffset] >> 3;
    switch (tgaInfo.bytesPerPixel)
    {
    case 1: case 2: case 3: case 4: case 8: case 16: break;
    default: return ERROR_FILE_FORMAT_INCORRECT;
    }

    // obtaining origin corner info
    uint8 descriptor = fields[descriptorOffset];
    tgaInfo.origin_corner = static_cast<TgaInfo::ORIGIN_CORNER>((descriptor >> 4) & 0x03);

    // obtaining number of alpha bits
    tgaInfo.alphaBits = descriptor & 0x0F;
    switch (tgaInfo.alphaBits)
    {
    case 0: case 1: case 4: case 8: break;
    default: return ERROR_FILE_FORMAT_INCORRECT;
    }

    // defining pixel format
    tgaInfo.pixelFormat = DefinePixelFormat(tgaInfo);
    if (tgaInfo.pixelFormat == FORMAT_INVALID)
        return ERROR_FILE_FORMAT_INCORRECT;

    return SUCCESS;
}

Size2i LibTgaWrapper::GetImageSize(File *infile) const
{
    DVASSERT(infile);
    
    TgaInfo tgaHeader;

    infile->Seek(0, File::SEEK_FROM_START);
    eErrorCode readResult = ReadTgaHeader(infile, tgaHeader);
    infile->Seek(0, File::SEEK_FROM_START);

    if (readResult == SUCCESS)
        return Size2i(tgaHeader.width, tgaHeader.height);
    else
        return Size2i();
}


eErrorCode LibTgaWrapper::ReadFile(File *infile, Vector<Image *> &imageSet, int32 baseMipMap) const
{
    DVASSERT(infile);

    TgaInfo tgaInfo;

    infile->Seek(0, File::SEEK_FROM_START);
    eErrorCode readResult = ReadTgaHeader(infile, tgaInfo);
    if (readResult != SUCCESS)
        return readResult;

    Image* pImage = Image::Create(tgaInfo.width, tgaInfo.height, tgaInfo.pixelFormat);
    ScopedPtr<Image> image(pImage);

    if (tgaInfo.imageType == TgaInfo::COMPRESSED_TRUECOLOR || tgaInfo.imageType == TgaInfo::COMPRESSED_GRAYSCALE)
        readResult = ReadCompressedTga(infile, tgaInfo, image);
    else
        readResult = ReadUncompressedTga(infile, tgaInfo, image);

    if (readResult == SUCCESS)
    {
        ImageConvert::SwapRedBlueChannels(image);
        
        SafeRetain(pImage);
        imageSet.push_back(pImage);
        return SUCCESS;
    }

    return readResult;
}

PixelFormat LibTgaWrapper::DefinePixelFormat(const TgaInfo& tgaInfo) const
{
    if (tgaInfo.imageType == TgaInfo::TRUECOLOR || tgaInfo.imageType == TgaInfo::COMPRESSED_TRUECOLOR)
    {
        switch (tgaInfo.bytesPerPixel)
        {
        case 2:
        {
            switch (tgaInfo.alphaBits)
            {
            case 0:
                return FORMAT_RGB565;
            case 1:
                return FORMAT_RGBA5551;
            case 4:
                return FORMAT_RGBA4444;
            default:
                return FORMAT_INVALID;
            }
        }
        case 3:
        {
            return (tgaInfo.alphaBits == 0) ? FORMAT_RGB888 : FORMAT_INVALID;
        }
        case 4:
        {
            return (tgaInfo.alphaBits == 8) ? FORMAT_RGBA8888 : FORMAT_INVALID;
        }
        case 8:
        {
            return FORMAT_RGBA16161616;
        }
        case 16:
        {
            return FORMAT_RGBA32323232;
        }
        default:
        {
            return FORMAT_INVALID;
        }
        }
    }
    else if (tgaInfo.imageType == TgaInfo::GRAYSCALE || tgaInfo.imageType == TgaInfo::COMPRESSED_GRAYSCALE)
    {
        switch (tgaInfo.bytesPerPixel)
        {
        case 1:
            return FORMAT_A8;
        case 2:
            return FORMAT_A16;
        default:
            return FORMAT_INVALID;
        }
    }
    else
        return FORMAT_INVALID;
}

eErrorCode LibTgaWrapper::ReadUncompressedTga(File *infile, const TgaInfo& tgaInfo, ScopedPtr<Image>& image) const
{
    auto readSize = infile->Read(image->data, image->dataSize);
    if(readSize != image->dataSize)
    {
        return ERROR_READ_FAIL;
    }
    
    switch (tgaInfo.origin_corner)
    {
        case TgaInfo::BOTTOM_LEFT:
            image->FlipVertical();
            break;

        case TgaInfo::BOTTOM_RIGHT:
            image->FlipVertical();
            image->FlipHorizontal();
            break;

        case TgaInfo::TOP_RIGHT:
            image->FlipHorizontal();
            break;

        case TgaInfo::TOP_LEFT:
            break;

        default:
            break;
    }
    
    return SUCCESS;
}

eErrorCode LibTgaWrapper::ReadCompressedTga(File *infile, const TgaInfo& tgaInfo, ScopedPtr<Image>& image) const
{
    uint8 chunkHeader;
    std::array<uint8, MAX_BYTES_IN_PIXEL> pixelBuffer;

    ImageDataWriter dataWriter(image, tgaInfo);

    while (!dataWriter.AtEnd())
    {
        if (infile->Read(&chunkHeader, 1) != 1)
            return ERROR_READ_FAIL;

        if (chunkHeader < 128)  // is raw section
        {
            ++chunkHeader;      // number of raw pixels

            for (uint8 i = 0; (i < chunkHeader) && !dataWriter.AtEnd(); ++i)
            {
                if (infile->Read(pixelBuffer.data(), tgaInfo.bytesPerPixel) != tgaInfo.bytesPerPixel)
                    return ERROR_READ_FAIL;

                dataWriter.Write(pixelBuffer.data());
            }

        }
        else                    // is compressed section
        {
            chunkHeader -= 127; // number of repeated pixels

            if (infile->Read(pixelBuffer.data(), tgaInfo.bytesPerPixel) != tgaInfo.bytesPerPixel)
                return ERROR_READ_FAIL;

            for (uint8 i = 0; (i < chunkHeader) && !dataWriter.AtEnd(); ++i)
            {
                dataWriter.Write(pixelBuffer.data());
            }
        }
    }
    return SUCCESS;
}

eErrorCode LibTgaWrapper::WriteFileAsCubeMap(const FilePath & fileName, const Vector<Vector<Image *> > &imageSet, PixelFormat pixelFormat) const
{
    Logger::Error("[%s] CubeMaps are not supported for TGA", __FUNCTION__);
    return DAVA::eErrorCode::ERROR_WRITE_FAIL;
}

eErrorCode LibTgaWrapper::WriteFile(const FilePath & fileName, const Vector<Image *> &imageSet, PixelFormat piexelFormat) const
{
    DVASSERT(imageSet.size() == 1);
    int32 width = imageSet[0]->width;
    int32 height = imageSet[0]->height;
    uint8* imageData = imageSet[0]->data;
    PixelFormat pixelFormat = imageSet[0]->format;
    DVASSERT(width && height && imageData);

    TgaInfo tgaInfo;
    tgaInfo.width = width;
    tgaInfo.height = height;
    tgaInfo.origin_corner = TgaInfo::TOP_LEFT; // saving as top-left avoids additional transformations from/to our Image format
    tgaInfo.pixelFormat = pixelFormat;
    switch(pixelFormat)
    {
    case FORMAT_RGBA8888:
        tgaInfo.bytesPerPixel = 4;
        tgaInfo.alphaBits = 8;
        tgaInfo.imageType = TgaInfo::TRUECOLOR;
        break; 
    case FORMAT_RGBA5551:
        tgaInfo.bytesPerPixel = 2;
        tgaInfo.alphaBits = 1;
        tgaInfo.imageType = TgaInfo::TRUECOLOR;
        break;
    case FORMAT_RGBA4444:
        tgaInfo.bytesPerPixel = 2;
        tgaInfo.alphaBits = 4;
        tgaInfo.imageType = TgaInfo::TRUECOLOR;
        break;
    case FORMAT_RGB888:
        tgaInfo.bytesPerPixel = 3;
        tgaInfo.alphaBits = 0;
        tgaInfo.imageType = TgaInfo::TRUECOLOR;
        break;
    case FORMAT_RGB565:
        tgaInfo.bytesPerPixel = 2;
        tgaInfo.alphaBits = 0;
        tgaInfo.imageType = TgaInfo::TRUECOLOR;
        break;
    case FORMAT_RGBA16161616:
        tgaInfo.bytesPerPixel = 8;
        tgaInfo.alphaBits = 16;
        tgaInfo.imageType = TgaInfo::TRUECOLOR;
        break;
    case FORMAT_RGBA32323232:
        tgaInfo.bytesPerPixel = 16;
        tgaInfo.alphaBits = 32;
        tgaInfo.imageType = TgaInfo::TRUECOLOR;
        break;
    case FORMAT_A8:
        tgaInfo.bytesPerPixel = 1;
        tgaInfo.alphaBits = 8;
        tgaInfo.imageType = TgaInfo::GRAYSCALE;
        break;
    case FORMAT_A16:
        tgaInfo.bytesPerPixel = 2;
        tgaInfo.alphaBits = 8;
        tgaInfo.imageType = TgaInfo::GRAYSCALE;
        break;
    default:
        Logger::Error("[%s] %s is not supported", __FUNCTION__, GlobalEnumMap<PixelFormat>::Instance()->ToString(pixelFormat));
        DVASSERT(false);
        return ERROR_FILE_FORMAT_INCORRECT;
    }

    std::unique_ptr<uint8[]> convertedData;
    if (tgaInfo.imageType == TgaInfo::TRUECOLOR)
    {
        convertedData.reset(new uint8[width * height * tgaInfo.bytesPerPixel]);
        uint32 pitch = width * tgaInfo.bytesPerPixel;
        ImageConvert::SwapRedBlueChannels(pixelFormat, imageData, width, height, pitch, convertedData.get());
        imageData = convertedData.get();
    }

    ScopedPtr<File> dstFile(File::Create(fileName, File::CREATE | File::WRITE));
    if (!dstFile)
    {
        Logger::Error("[%s] File %s could not be opened for writing", __FUNCTION__, fileName.GetAbsolutePathname().c_str());
        return ERROR_WRITE_FAIL;
    }

    auto res = WriteTgaHeader(dstFile, tgaInfo);
    if (res != SUCCESS)
        return res;

    return WriteUncompressedTga(dstFile, tgaInfo, imageData);
}

DAVA::eErrorCode LibTgaWrapper::WriteTgaHeader(File *dstFile, const TgaInfo& tgaInfo) const
{
    std::array<uint8, 18> fields = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    fields[imageTypeOffset] = static_cast<uint8>(tgaInfo.imageType);
    Uint16ToLowEndian(tgaInfo.width, &fields[widthOffset]);
    Uint16ToLowEndian(tgaInfo.height, &fields[heightOffset]);
    fields[originYOffset] = fields[heightOffset]; // for TOP_LEFT, originX = 0, originY = height
    fields[originYOffset + 1] = fields[heightOffset + 1];
    fields[bppOffset] = tgaInfo.bytesPerPixel << 3;
    fields[descriptorOffset] = tgaInfo.alphaBits | (tgaInfo.origin_corner & 0x03) << 4;

    auto bytesWritten = dstFile->Write(&fields, fields.size());
    return (bytesWritten == fields.size()) ? SUCCESS : ERROR_WRITE_FAIL;
}

eErrorCode LibTgaWrapper::WriteUncompressedTga(File *dstFile, const TgaInfo& tgaInfo, const uint8* data) const
{
    auto dataSize = tgaInfo.width * tgaInfo.height * tgaInfo.bytesPerPixel;
    auto writeSize = dstFile->Write(data, dataSize);
    if (writeSize != dataSize)
    {
        return ERROR_READ_FAIL;
    }

    return SUCCESS;
}

LibTgaWrapper::ImageDataWriter::ImageDataWriter(Image* img, const LibTgaWrapper::TgaInfo& _tgaInfo)
    : tgaInfo(_tgaInfo)
{
    linesRemaining = tgaInfo.height;
    linePixelsRemaining = tgaInfo.width;
    isAtEnd = false;
    ResetPtr(img, tgaInfo);
}

void LibTgaWrapper::ImageDataWriter::Write(uint8* pixel)
{
    if (!isAtEnd)
    {
        Memcpy(ptr, pixel, tgaInfo.bytesPerPixel);
        IncrementPtr();
    }
}

void LibTgaWrapper::ImageDataWriter::ResetPtr(const Image* image, const LibTgaWrapper::TgaInfo& tgaInfo)
{
    switch (tgaInfo.origin_corner)
    {
    case TgaInfo::BOTTOM_LEFT:
    {
        ptr = image->data + (tgaInfo.width * tgaInfo.bytesPerPixel) * (tgaInfo.height - 1);
        ptrInc = tgaInfo.bytesPerPixel;
        ptrNextLineJump = -(tgaInfo.width * tgaInfo.bytesPerPixel * 2);
        break;
    }
    case TgaInfo::BOTTOM_RIGHT:
    {
        ptr = image->data + (tgaInfo.width * tgaInfo.bytesPerPixel * tgaInfo.height) - tgaInfo.bytesPerPixel;
        ptrInc = -tgaInfo.bytesPerPixel;
        ptrNextLineJump = 0;
        break;
    }
    case TgaInfo::TOP_LEFT:
    {
        ptr = image->data;
        ptrInc = tgaInfo.bytesPerPixel;
        ptrNextLineJump = 0;
        break;
    }
    case TgaInfo::TOP_RIGHT:
    {
        ptr = image->data + ((tgaInfo.width - 1) * tgaInfo.bytesPerPixel);
        ptrInc = -tgaInfo.bytesPerPixel;
        ptrNextLineJump = tgaInfo.width * tgaInfo.bytesPerPixel * 2;
        break;
    }
    default:
    {
        DVASSERT(false && "Unknown ORIGIN_CORNER");
    }
    }
}

void LibTgaWrapper::ImageDataWriter::IncrementPtr()
{
    ptr += ptrInc;
    if (--linePixelsRemaining == 0)
    {
        if (--linesRemaining > 0)
        {
            linePixelsRemaining = tgaInfo.width;
            ptr += ptrNextLineJump;
        }
        else
            isAtEnd = true;
    }
}

};
