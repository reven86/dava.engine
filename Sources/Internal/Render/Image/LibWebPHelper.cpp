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


#include "Render/Image/LibWebPHelper.h"

#include "Render/Image/Image.h"

#include "FileSystem/File.h"

#include "webp/decode.h"
#include "webp/encode.h"

namespace DAVA
{

LibWebPHelper::LibWebPHelper()
{
    name.assign("WEBP");
    supportedExtensions.push_back(".webp");
    supportedFormats = { {FORMAT_RGB888, FORMAT_RGBA8888} };
}

bool LibWebPHelper::IsMyImage(File *infile) const
{
    return GetImageInfo(infile).dataSize != 0;
}

eErrorCode LibWebPHelper::ReadFile(File *infile, Vector<Image *> &imageSet, int32 baseMipMap) const
{
    WebPDecoderConfig config;
    WebPBitstreamFeatures* bitstream = &config.input;
    auto initCStatus = WebPInitDecoderConfig(&config);
    if (0 == initCStatus)
    {
        Logger::Error("[LibWebPHelper::ReadFile] Error in WebPInitDecpderConfig. File %s", infile->GetFilename().GetAbsolutePathname().c_str());
        return eErrorCode::ERROR_READ_FAIL;
    }

    infile->Seek(0, File::SEEK_FROM_START);
    uint32 dataSize = infile->GetSize();
    uint8_t *data = new uint8_t[dataSize];
    SCOPE_EXIT
    {
        SafeDeleteArray(data);
    };
    infile->Read(data, dataSize);
    infile->Seek(0, File::SEEK_FROM_START);

    WebPBitstreamFeatures local_features;
    if (nullptr == bitstream)
    {
        bitstream = &local_features;
    }
    
    auto bsStatus = WebPGetFeatures(data, dataSize, bitstream);
    if (bsStatus != VP8_STATUS_OK)
    {
        Logger::Error("[LibWebPHelper::ReadFile] File %s has wrong WebP header", infile->GetFilename().GetAbsolutePathname().c_str());
        return eErrorCode::ERROR_FILE_FORMAT_INCORRECT;
    }

    uint8_t *newData = nullptr;
    if (bitstream->has_alpha)
    {
        newData = WebPDecodeRGBA(data, dataSize, &bitstream->width, &bitstream->height);
    }
    else
    {
        newData = WebPDecodeRGB(data, dataSize, &bitstream->width, &bitstream->height);
    }
    if (nullptr == newData)
    {
        Logger::Error("[LibWebPHelper::ReadFile] Error during decompression of file %s into WebP.", infile->GetFilename().GetAbsolutePathname().c_str());
        return eErrorCode::ERROR_FILE_FORMAT_INCORRECT;
    }

    Image* image = new Image();

    if (bitstream->has_alpha)
    {
        image->format = FORMAT_RGBA8888;
    }
    else
    {
        image->format = FORMAT_RGB888;
    }
    image->width = bitstream->width;
    image->height = bitstream->height;
    image->data = newData;
    image->dataSize = bitstream->width * bitstream->height * PixelFormatDescriptor::GetPixelFormatSizeInBytes(image->format);

    imageSet.push_back(image);

    return eErrorCode::SUCCESS;
}

eErrorCode LibWebPHelper::WriteFile(const FilePath & fileName, const Vector<Image *> &imageSet, PixelFormat compressionFormat, ImageQuality quality) const
{
    DVASSERT(imageSet.size());
    const Image* original = imageSet[0];
    int32 width = original->width;
    int32 height = original->height;
    uint8_t* imageData = original->data;
    PixelFormat format = original->format;

    if (!(FORMAT_RGBA8888 == format || format == FORMAT_RGB888))
    {
        Logger::Error("[LibWebPHelper::WriteFile] Not supported format");
        return eErrorCode::ERROR_FILE_FORMAT_INCORRECT;
    }

    uint8_t *outData = nullptr;
    SCOPE_EXIT
    {
        SafeDeleteArray(outData);
    };
    uint32 outSize;
    int stride = width * sizeof(*imageData) * PixelFormatDescriptor::GetPixelFormatSizeInBytes(format);
    if (FORMAT_RGB888 == format)
    {
        if (quality == MAX_IMAGE_QUALITY)
        {
            outSize = static_cast<uint32>(WebPEncodeLosslessRGB(imageData, width, height, stride, &outData));
        }
        else
        {
            outSize = static_cast<uint32>(WebPEncodeRGB(imageData, width, height, stride, quality, &outData));
        }
    }
    else
    {
        if (quality == MAX_IMAGE_QUALITY)
        {
            outSize = static_cast<uint32>(WebPEncodeLosslessRGBA(imageData, width, height, stride, &outData));
        }
        else
        {
            outSize = static_cast<uint32>(WebPEncodeRGBA(imageData, width, height, stride, quality, &outData));
        }
    }

    if (nullptr == outData)
    {
        Logger::Error("[LibWebPHelper::WriteFile] Error during compression of WebP into file %s.", fileName.GetAbsolutePathname().c_str());
        return eErrorCode::ERROR_FILE_FORMAT_INCORRECT;
    }

    File *outFile = File::Create(fileName, File::CREATE | File::WRITE);
    if (nullptr == outFile)
    {
        Logger::Error("[LibWebPHelper::WriteFile] File %s could not be opened for writing", fileName.GetAbsolutePathname().c_str());
        return eErrorCode::ERROR_WRITE_FAIL;
    }

    outFile->Write(outData, outSize);
    outFile->Release();

    return eErrorCode::SUCCESS;
}

eErrorCode LibWebPHelper::WriteFileAsCubeMap(const FilePath &fileName, const Vector<Vector<Image *>> &imageSet, PixelFormat compressionFormat, ImageQuality quality) const
{
    Logger::Error("[LibWebPHelper::WriteFileAsCubeMap] For WebP cubeMaps are not supported");
    return eErrorCode::ERROR_WRITE_FAIL;
}

DAVA::ImageInfo LibWebPHelper::GetImageInfo(File *infile) const
{
    WebPDecoderConfig config;
    WebPInitDecoderConfig(&config);
    WebPBitstreamFeatures* const bitstream = &config.input;

    infile->Seek(0, File::SEEK_FROM_START);
    uint32 dataSize = infile->GetSize();
    uint8_t *data = new uint8_t[dataSize];
    SCOPE_EXIT
    {
        SafeDeleteArray(data);
    };
    infile->Read(data, dataSize);
    infile->Seek(0, File::SEEK_FROM_START);

    auto bsStatus = WebPGetFeatures(data, dataSize, bitstream);
    if (bsStatus != VP8_STATUS_OK)
    {
        return ImageInfo();
    }

    ImageInfo info;
    info.height = bitstream->height;
    info.width = bitstream->width;
    if (bitstream->has_alpha)
    {
        info.format = FORMAT_RGBA8888;
    }
    else
    {
        info.format = FORMAT_RGB888;
    }
    auto size = bitstream->width * bitstream->height * PixelFormatDescriptor::GetPixelFormatSizeInBytes(info.format);
    info.dataSize = size;
    info.mipmapsCount = 1;

    SafeDeleteArray(data);

    return info;
}

};
