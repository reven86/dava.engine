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


#include "Render/Image/Image.h"
#include "Render/Image/ImageConvert.h"
#include "Render/Image/DDS/NvttHelper.h"
#include "FileSystem/FileSystem.h"

#include <libdxt/nvtt.h>
#include <libdxt/nvtt_extra.h>

namespace DAVA
{
namespace NvttHelper
{
struct PairNvttPixelFormat
{
    nvtt::Format nvttFormat;
    PixelFormat davaFormat;
};

const Map<nvtt::Format, PixelFormat> formatNamesMap =
{
  { nvtt::Format_DXT1, FORMAT_DXT1 },
  { nvtt::Format_DXT1a, FORMAT_DXT1A },
  { nvtt::Format_DXT3, FORMAT_DXT3 },
  { nvtt::Format_DXT5, FORMAT_DXT5 },
  { nvtt::Format_DXT5n, FORMAT_DXT5NM },
  { nvtt::Format_ATC_RGB, FORMAT_ATC_RGB },
  { nvtt::Format_ATC_RGBA_EXPLICIT_ALPHA, FORMAT_ATC_RGBA_EXPLICIT_ALPHA },
  { nvtt::Format_ATC_RGBA_INTERPOLATED_ALPHA, FORMAT_ATC_RGBA_INTERPOLATED_ALPHA },
  { nvtt::Format_RGBA, FORMAT_RGBA8888 }
};

bool InitDecompressor(nvtt::Decompressor& dec, const uint8* mem, uint32 size)
{
    if (NULL == mem || size == 0)
    {
        Logger::Error("[InitDecompressor] Wrong buffer params.");
        return false;
    }

    if (!dec.initWithDDSFile(mem, size))
    {
        Logger::Error("[InitDecompressor] Wrong buffer.");
        return false;
    }

    return true;
}

PixelFormat GetPixelFormatByNVTTFormat(nvtt::Format nvttFormat)
{
    PixelFormat retValue = FORMAT_INVALID;
    auto result = formatNamesMap.find(nvttFormat);
    if (result == formatNamesMap.end())
    {
        return FORMAT_INVALID;
    }
    else
    {
        return result->second;
    }
}

nvtt::Format GetNVTTFormatByPixelFormat(PixelFormat pixelFormat)
{
    for (const auto& formatPair : formatNamesMap)
    {
        if (formatPair.second == pixelFormat)
        {
            return formatPair.first;
        }
    }

    //bc5 is unsupported, used to determinate fail in search
    return nvtt::Format_BC5;
}

ImagePtr DecompressDxtToRGBA(const Image* image)
{
    DVASSERT(image);

    if (!(image->format >= FORMAT_DXT1 && image->format <= FORMAT_DXT5NM))
    {
        Logger::Error("[NvttHelper::DecompressDxt] Wrong compression format (%d).", GlobalEnumMap<PixelFormat>::Instance()->ToString(image->format));
        return ImagePtr(nullptr);
    }

    nvtt::Format innerComprFormat = GetNVTTFormatByPixelFormat(image->format);
    if (nvtt::Format_BC5 == innerComprFormat)
    { //bc5 is unsupported, used to determinate fail in search
        Logger::Error("[NvttHelper::DecompressDxt] Can't work with nvtt::Format_BC5.");
        return ImagePtr(nullptr);
    }

    nvtt::InputOptions inputOptions;
    inputOptions.setTextureLayout(nvtt::TextureType_2D, image->width, image->height);
    inputOptions.setMipmapGeneration(false);

    nvtt::CompressionOptions compressionOptions;
    compressionOptions.setFormat(innerComprFormat);
    if (FORMAT_DXT5NM == image->format)
    {
        inputOptions.setNormalMap(true);
    }

    uint32 headerSize = DECOMPRESSOR_MIN_HEADER_SIZE;
    Vector<uint8> imageBuffer(headerSize + image->dataSize);

    uint32 realHeaderSize = nvtt::Decompressor::getHeader(imageBuffer.data(), headerSize, inputOptions, compressionOptions);
    if (realHeaderSize > DECOMPRESSOR_MIN_HEADER_SIZE)
    {
        Logger::Error("[NvttHelper::DecompressDxt] Header size (%d) is bigger than maximum expected", realHeaderSize);
        return ImagePtr(nullptr);
    }

    nvtt::Decompressor dec;

    Memcpy(imageBuffer.data() + realHeaderSize, image->data, image->dataSize);

    bool initOk = InitDecompressor(dec, imageBuffer.data(), realHeaderSize + image->dataSize);
    if (initOk)
    {
        static const PixelFormat outFormat = FORMAT_RGBA8888;
        ImagePtr newImage(Image::Create(image->width, image->height, outFormat));
        const uint32 mip = 0;
        bool decompressedOk = dec.process(newImage->data, newImage->dataSize, mip);
        if (decompressedOk)
        {
            // nvtt decompresses into BGRA8888, thus we need to swap channels to obtain RGBA8888
            ImageConvert::SwapRedBlueChannels(newImage);

            newImage->mipmapLevel = image->mipmapLevel;
            newImage->cubeFaceID = image->cubeFaceID;

            return newImage;
        }
    }

    return ImagePtr(nullptr);
}

bool WriteDxtFile(const FilePath& outFileName, PixelFormat compressionFormat, const Vector<Vector<Image*>>& srcImages)
{
#ifdef __DAVAENGINE_IPHONE__

    DVASSERT_MSG(false, "No necessary write compressed files on mobile");
    return false;

#elif defined(__DAVAENGINE_WIN_UAP__)
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return false;

#else
    if (!((compressionFormat >= FORMAT_DXT1 && compressionFormat <= FORMAT_DXT5NM) || (compressionFormat == FORMAT_RGBA8888)))
    {
        Logger::Error("[LibDdsHelper::WriteDxtFile] Wrong copression format (%d).", compressionFormat);
        return false;
    }

    nvtt::Format innerComprFormat = NvttHelper::GetNVTTFormatByPixelFormat(compressionFormat);
    if (nvtt::Format_BC5 == innerComprFormat)
    {
        //bc5 is unsupported, used to determinate fail in search
        Logger::Error("[LibDdsHelper::WriteDxtFile] Can't work with nvtt::Format_BC5.");
        return false;
    }
    if (srcImages.empty() || srcImages[0].empty())
    {
        Logger::Error("[LibDdsHelper::WriteDxtFile] Empty incoming image vector.");
        return false;
    }

    uint32 facesCount = srcImages.size();
    uint32 mipmapsCount = srcImages[0].size();

    nvtt::InputOptions inputOptions;
    nvtt::TextureType textureType = (facesCount > 1) ? nvtt::TextureType_Cube : nvtt::TextureType_2D;
    inputOptions.setTextureLayout(textureType, srcImages[0][0]->width, srcImages[0][0]->height);
    inputOptions.setMipmapGeneration(mipmapsCount > 1, mipmapsCount - 1);

    for (uint32 f = 0; f < facesCount; ++f)
    {
        DVVERIFY(srcImages[f].size() == mipmapsCount);
        for (uint32 m = 0; m < mipmapsCount; ++m)
        {
            Image* image = srcImages[f][m];
            ImagePtr convertedImage(nullptr);
            auto inputFormat = image->format;

            if (inputFormat >= FORMAT_DXT1 && inputFormat <= FORMAT_DXT5NM)
            {
                convertedImage = DecompressDxtToRGBA(image);
                if (!convertedImage)
                {
                    Logger::Error("[NvttHelper::WriteDxtFile] Error during decompressing of DXT into RGBA");
                    return false;
                }
            }
            else
            {
                if (inputFormat == FORMAT_RGBA8888)
                {
                    convertedImage = Image::CreateFromData(image->width, image->height, FORMAT_RGBA8888, image->data);
                }
                else
                {
                    convertedImage = Image::Create(image->width, image->height, FORMAT_RGBA8888);
                    ImageConvert::ConvertImageDirect(image, convertedImage);
                }
            }

            ImageConvert::SwapRedBlueChannels(convertedImage);
            inputOptions.setMipmapData(convertedImage->data, convertedImage->width, convertedImage->height, 1, f, m);
        }
    }

    nvtt::CompressionOptions compressionOptions;
    compressionOptions.setFormat(innerComprFormat);
    if (FORMAT_DXT5NM == compressionFormat)
    {
        inputOptions.setNormalMap(true);
    }

    nvtt::OutputOptions outputOptions;
    FilePath tmpFileName = FilePath::CreateWithNewExtension(outFileName, "_dds");
    outputOptions.setFileName(tmpFileName.GetAbsolutePathname().c_str());

    nvtt::Compressor compressor;
    bool compressedOk = compressor.process(inputOptions, compressionOptions, outputOptions);
    if (compressedOk)
    {
        FileSystem::Instance()->DeleteFile(outFileName);
        if (!FileSystem::Instance()->MoveFile(tmpFileName, outFileName, true))
        {
            Logger::Error("[NvttHelper::WriteDxtFile] Temporary dds file renamig failed.");
            compressedOk = false;
        }
    }
    else
    {
        Logger::Error("[LibDdsHelper::WriteDxtFile] Error during writing DDS file (%s).", tmpFileName.GetAbsolutePathname().c_str());
    }

    return compressedOk;
#endif //__DAVAENGINE_IPHONE__
}

bool WriteDdsFile(const FilePath& outFileName, PixelFormat compressionFormat, const Vector<uint8>& compressedData, uint32 width, uint32 height, uint32 mipCount, bool isCubemap)
{
    nvtt::Format innerComprFormat = NvttHelper::GetNVTTFormatByPixelFormat(compressionFormat);

    nvtt::TextureType textureType = isCubemap ? nvtt::TextureType_Cube : nvtt::TextureType_2D;
    nvtt::InputOptions inputOptions;
    inputOptions.setTextureLayout(textureType, width, height);
    inputOptions.setMipmapGeneration(mipCount > 1, mipCount - 1);

    nvtt::CompressionOptions compressionOptions;
    compressionOptions.setFormat(innerComprFormat);

    uint32 headerSize = DECOMPRESSOR_MIN_HEADER_SIZE;
    Vector<uint8> header(headerSize);

    nvtt::Decompressor decompress;
    const uint32 realHeaderSize = decompress.getHeader(header.data(), headerSize, inputOptions, compressionOptions);
    if (realHeaderSize > DECOMPRESSOR_MIN_HEADER_SIZE)
    {
        Logger::Error("[NvttHelper::DecompressDxt] Header size (%d) is bigger than maximum expected", realHeaderSize);
        return false;
    }

    bool res = false;
    const FilePath fileName = FilePath::CreateWithNewExtension(outFileName, "_dds");
    ScopedPtr<File> file(File::Create(fileName, File::CREATE | File::WRITE));
    if (file)
    {
        file->Write(header.data(), realHeaderSize);
        file->Write(compressedData.data(), compressedData.size());
        file.reset();

        FileSystem::Instance()->DeleteFile(outFileName);
        res = FileSystem::Instance()->MoveFile(fileName, outFileName, true);
        if (!res)
            Logger::Error("[LibDdsHelper::WriteDxtFile] Temporary dds file renamig failed.");
    }
    else
    {
        Logger::Error("[LibDdsHelper::WriteDxtFile] Can't open '%s' for writing.", fileName.GetAbsolutePathname().c_str());
    }

    return res;
}
}
}
