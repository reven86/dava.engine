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


namespace DAVA
{

LibTgaWrapper::LibTgaWrapper()
{
    supportedExtensions.push_back(".tga");
    supportedExtensions.push_back(".tpic");
}

bool LibTgaWrapper::IsImage(File *infile) const
{
    return GetDataSize(infile) != 0;
}

uint32 LibTgaWrapper::GetDataSize(File *infile) const
{
    DVASSERT(infile);

    TgaHeader tgaHeader;
    PixelFormat pixelFormat;

    infile->Seek(0, File::SEEK_FROM_START);
    eErrorCode readResult = ReadTgaHeader(infile, tgaHeader, pixelFormat);
    infile->Seek(0, File::SEEK_FROM_START);

    if (readResult == SUCCESS)
        return tgaHeader.width * tgaHeader.height * (tgaHeader.bpp >> 3);
    else
        return 0;
}

DAVA::eErrorCode LibTgaWrapper::ReadTgaHeader(File *infile, TgaHeader& tgaHeader, PixelFormat& pixelFormat) const
{
    Memset(&tgaHeader.fields, 0, tgaHeader.fields.size());

    size_t bytesRead = infile->Read(&tgaHeader.fields, tgaHeader.fields.size());
    if (bytesRead != tgaHeader.fields.size())
        return ERROR_READ_FAIL;

    static const std::array<uint8, 9> zeroes = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    if (Memcmp(&tgaHeader.fields[0], &zeroes[0], 2) != 0 ||
        Memcmp(&tgaHeader.fields[3], &zeroes[0], 9) != 0)
    {
        return ERROR_FILE_FORMAT_INCORRECT;
    }

    if (tgaHeader.imageType != TgaHeader::GRAYSCALE ||
        tgaHeader.imageType != TgaHeader::TRUECOLOR ||
        tgaHeader.imageType != TgaHeader::COMPRESSED_GRAYSCALE ||
        tgaHeader.imageType != TgaHeader::COMPRESSED_TRUECOLOR)
    {
        return ERROR_FILE_FORMAT_INCORRECT;
    }

    if (!tgaHeader.width || !tgaHeader.height)
    {
        return ERROR_FILE_FORMAT_INCORRECT;
    }

    if (   tgaHeader.bpp !=   8 
        && tgaHeader.bpp !=  16 
        && tgaHeader.bpp !=  24 
        && tgaHeader.bpp !=  32
        && tgaHeader.bpp !=  64
        && tgaHeader.bpp != 128)
    {
        return ERROR_FILE_FORMAT_INCORRECT;
    }

    if (   tgaHeader.descriptor != 0
        && tgaHeader.descriptor != 1
        && tgaHeader.descriptor != 4
        && tgaHeader.descriptor != 8)
    {
        return ERROR_FILE_FORMAT_INCORRECT;
    }

    pixelFormat = DefinePixelFormat(tgaHeader);
    if (pixelFormat == FORMAT_INVALID)
        return ERROR_FILE_FORMAT_INCORRECT;

    return SUCCESS;
}

Size2i LibTgaWrapper::GetImageSize(File *infile) const
{
    DVASSERT(infile);
    
    TgaHeader tgaHeader;
    PixelFormat pixelFormat;

    infile->Seek(0, File::SEEK_FROM_START);
    eErrorCode readResult = ReadTgaHeader(infile, tgaHeader, pixelFormat);
    infile->Seek(0, File::SEEK_FROM_START);

    if (readResult == SUCCESS)
        return Size2i(tgaHeader.width, tgaHeader.height);
    else
        return Size2i();
}


eErrorCode LibTgaWrapper::ReadFile(File *infile, Vector<Image *> &imageSet, int32 baseMipMap) const
{
    DVASSERT(infile);

    TgaHeader tgaHeader;
    PixelFormat pixelFormat;

    infile->Seek(0, File::SEEK_FROM_START);
    eErrorCode readResult = ReadTgaHeader(infile, tgaHeader, pixelFormat);
    if (readResult != SUCCESS)
        return readResult;

    Image* pImage = Image::Create(tgaHeader.width, tgaHeader.height, pixelFormat);
    ScopedPtr<Image> image(pImage);

    if (tgaHeader.imageType == TgaHeader::TRUECOLOR || tgaHeader.imageType == TgaHeader::GRAYSCALE)
        readResult = ReadUncompressedTga(infile, tgaHeader, image);
    else
        readResult = ReadCompressedTga(infile, tgaHeader, image);

    if (readResult == SUCCESS)
    {
        SafeRetain(pImage);
        imageSet.push_back(pImage);
        return SUCCESS;
    }
    else
        return readResult;
}

PixelFormat LibTgaWrapper::DefinePixelFormat(const TgaHeader& tgaHeader) const
{
    if (tgaHeader.imageType == TgaHeader::TRUECOLOR || tgaHeader.imageType == TgaHeader::COMPRESSED_TRUECOLOR)
    {
        if (tgaHeader.bpp == 16)
        {
            if (tgaHeader.descriptor == 0)
                return FORMAT_RGB565;
            else if (tgaHeader.descriptor == 1)
                return FORMAT_RGBA5551;
            else if (tgaHeader.descriptor == 4)
                return FORMAT_RGBA4444;
            else
                return FORMAT_INVALID;
        }
        else if (tgaHeader.bpp == 24)
        {
            if (tgaHeader.descriptor == 0)
                return FORMAT_RGB888;
            else
                return FORMAT_INVALID;
        }
        else if (tgaHeader.bpp == 32)
        {
            if (tgaHeader.descriptor == 8)
                return FORMAT_RGBA8888;
            else
                return FORMAT_INVALID;
        }
        else if (tgaHeader.bpp == 64)
        {
            return FORMAT_RGBA16161616;
        }
        else if (tgaHeader.bpp == 128)
        {
            return FORMAT_RGBA32323232;
        }
        else
            return FORMAT_INVALID;
    }
    else if (tgaHeader.imageType == TgaHeader::GRAYSCALE || tgaHeader.imageType == TgaHeader::COMPRESSED_GRAYSCALE)
    {
        if (tgaHeader.bpp == 8)
            return FORMAT_A8;
        else if (tgaHeader.bpp == 16)
            return FORMAT_A16;
        else
            return FORMAT_INVALID;
    }
    else
        return FORMAT_INVALID;
}

eErrorCode LibTgaWrapper::ReadUncompressedTga(File *infile, const TgaHeader& tgaHeader, ScopedPtr<Image>& image) const
{
    uint32 bytesRead = infile->Read(image->data, image->dataSize);
    if (bytesRead != image->dataSize)
        return ERROR_READ_FAIL;

//     uint32 bytesPerPixel = tgaHeader.bpp >> 3;
//     for (uint32 cswap = 0; (cswap+2) < bytesRead; cswap += bytesPerPixel)
//     {
//         image->data[cswap] ^= image->data[cswap + 2] ^=
//             image->data[cswap] ^= image->data[cswap + 2];
//     }
    
    return SUCCESS;
}

eErrorCode LibTgaWrapper::ReadCompressedTga(File *infile, const TgaHeader& tgaHeader, ScopedPtr<Image>& image) const
{
    uint32 bytesPerPixel = tgaHeader.bpp >> 3;

    uint8 chunkHeader;
    Vector<uint8> pixelBuffer(bytesPerPixel);

    for (uint32 bytesWritten = 0; bytesWritten < image->dataSize;)
    {
        if (infile->Read(&chunkHeader, 1) != 1)
            return ERROR_READ_FAIL;

        if (chunkHeader < 128)  // is raw section
        {
            ++chunkHeader;      // number of raw pixels

            for (uint8 i = 0; 
                (i < chunkHeader) && (bytesWritten < image->dataSize); 
                ++i, bytesWritten += bytesPerPixel)
            {
                if (infile->Read(&image->data[bytesWritten], bytesPerPixel) != bytesPerPixel)
                    return ERROR_READ_FAIL;
            }
            
        }
        else                    // is compressed section
        {
            chunkHeader -= 127; // number of repeated pixels

            if (infile->Read(&pixelBuffer, bytesPerPixel) != bytesPerPixel)
                return ERROR_READ_FAIL;

            for (uint8 i = 0; 
                (i < chunkHeader) && (bytesWritten < image->dataSize); 
                ++i, bytesWritten += bytesPerPixel)
            {
                Memcpy(&image->data[bytesWritten], &pixelBuffer, bytesPerPixel);
            }
        }
    }
}

eErrorCode LibTgaWrapper::WriteFileAsCubeMap(const FilePath & fileName, const Vector<Vector<Image *> > &imageSet, PixelFormat compressionFormat) const
{
    return DAVA::eErrorCode::SUCCESS;
}

eErrorCode LibTgaWrapper::WriteFile(const FilePath & fileName, const Vector<Image *> &imageSet, PixelFormat compressionFormat) const
{
    return DAVA::eErrorCode::SUCCESS;
}


};
