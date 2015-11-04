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
#include "Render/Image/ImageSystem.h"
#include "Render/Image/ImageConvert.h"
#include "Render/Image/LibDdsHelper.h"

#include "Render/Texture.h"
#include "Render/PixelFormatDescriptor.h"
#include "Render/Renderer.h"
#include <libdxt/nvtt.h>
#include <libdxt/nvtt_extra.h>

#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"

#include "Utils/Utils.h"
#include "Utils/CRC32.h"

#if defined(__DAVAENGINE_WIN_UAP__)

//disabling of warning 
    #pragma warning(push)
    #pragma warning(disable : 4091)
    #include <libatc/TextureConverter.h>
    #pragma warning(pop)

#else
    #include <libatc/TextureConverter.h>
#endif

#define MAKEFOURCC(ch0, ch1, ch2, ch3) ((uint32)((uint8)(ch0)) | ((uint32)((uint8)(ch1)) << 8) | ((uint32)((uint8)(ch2)) << 16) | ((uint32)((uint8)(ch3)) << 24))
#define DDS_MAGIC_NUMBER 0x20534444 // equivalent of 'D''D''S'' '
#define METADATA_CRC_TAG 0x5f435243 // equivalent of 'C''R''C''_'

using namespace nvtt;

namespace DAVA
{
namespace DDSFile
{
struct DDSPixelFormat
{
    uint32 size = 0;
    uint32 flags = 0;
    uint32 fourCC = 0;
    uint32 RGBBitCount = 0;
    uint32 RBitMask = 0;
    uint32 GBitMask = 0;
    uint32 BBitMask = 0;
    uint32 ABitMask = 0;
};

struct MetaData //uint32 reserved1[11]
{
    uint32 reserved0 = 0;
    uint32 reserved1 = 0;
    uint32 reserved2 = 0;
    uint32 reserved3 = 0;
    uint32 reserved4 = 0;
    uint32 reserved5 = 0;
    uint32 davaPixelFormat = 0;
    uint32 crcTag = 0;
    uint32 crcValue = 0;
    uint32 reserved9 = 0;
    uint32 reserved10 = 0;
};

struct Header
{
    uint32 size = 0;
    uint32 flags = 0;
    uint32 height = 0;
    uint32 width = 0;
    uint32 pitchOrLinearSize = 0;
    uint32 depth = 0;
    uint32 mipMapCount = 0;
    MetaData metadata; //uint32 reserved1[11]
    DDSPixelFormat format;
    uint32 caps = 0;
    uint32 caps2 = 0;
    uint32 caps3 = 0;
    uint32 caps4 = 0;
    uint32 reserved2 = 0;
};

struct FileHeader
{
    uint32 magicNumber = 0;
    Header formatHeader;

    bool IsValid() const
    {
        return magicNumber == DDS_MAGIC_NUMBER;
    }
};

FileHeader ReadHeader(File* inFile)
{
    FileHeader header;
    inFile->Seek(0, File::SEEK_FROM_START);
    uint32 read = inFile->Read(&header);
    DVVERIFY(read == sizeof(header));

    return header;
}

}

class QualcommHelper
{
public:
    static const int32 Q_FORMAT_COUNT = 9;

    struct PairQualcommPixelGLFormat
    {
        int32 qFormat;
        PixelFormat davaFormat;
        PairQualcommPixelGLFormat(int32 _qFormat, PixelFormat _davaFormat)
        {
            qFormat = _qFormat;
            davaFormat = _davaFormat;
        }
        PairQualcommPixelGLFormat() = delete;
    };

    static const Array<PairQualcommPixelGLFormat, Q_FORMAT_COUNT> formatPair;

    int32 static GetQualcommFormat(PixelFormat format);
    PixelFormat static GetDavaFormat(int32 qFormat);
};

const Array<QualcommHelper::PairQualcommPixelGLFormat, QualcommHelper::Q_FORMAT_COUNT> QualcommHelper::formatPair =
{ {
QualcommHelper::PairQualcommPixelGLFormat(Q_FORMAT_ATC_RGB, FORMAT_ATC_RGB),
QualcommHelper::PairQualcommPixelGLFormat(Q_FORMAT_ATC_RGBA_EXPLICIT_ALPHA, FORMAT_ATC_RGBA_EXPLICIT_ALPHA),
QualcommHelper::PairQualcommPixelGLFormat(Q_FORMAT_ATC_RGBA_INTERPOLATED_ALPHA, FORMAT_ATC_RGBA_INTERPOLATED_ALPHA),

QualcommHelper::PairQualcommPixelGLFormat(Q_FORMAT_RGBA_8UI, FORMAT_RGBA8888),
QualcommHelper::PairQualcommPixelGLFormat(Q_FORMAT_RGB_8UI, FORMAT_RGB888),
QualcommHelper::PairQualcommPixelGLFormat(Q_FORMAT_RGB5_A1UI, FORMAT_RGBA5551),
QualcommHelper::PairQualcommPixelGLFormat(Q_FORMAT_RGBA_4444, FORMAT_RGBA4444),
QualcommHelper::PairQualcommPixelGLFormat(Q_FORMAT_RGB_565, FORMAT_RGB565),
QualcommHelper::PairQualcommPixelGLFormat(Q_FORMAT_ALPHA_8, FORMAT_A8),
} };

int32 QualcommHelper::GetQualcommFormat(PixelFormat format)
{
    for (auto& pair : formatPair)
    {
        if (pair.davaFormat == format)
        {
            return pair.qFormat;
        }
    }

    Logger::Error("Wrong pixel format (%d).", format);
    return -1;
}

PixelFormat QualcommHelper::GetDavaFormat(int32 format)
{
    for (auto& pair : formatPair)
    {
        if (pair.qFormat == format)
        {
            return pair.davaFormat;
        }
    }

    Logger::Error("Wrong qualcomm format (%d).", format);
    return FORMAT_INVALID;
}

class NvttHelper
{
public:
    struct DDSInfo
    {
        uint32 width;
        uint32 height;
        uint32 dataSize;
        uint32 mipmapsCount;
        uint32 headerSize;
        uint32 faceCount;
        uint32 faceFlags;

        DDSInfo()
        {
            width = 0;
            height = 0;
            dataSize = 0;
            mipmapsCount = 0;
            headerSize = 0;
            faceCount = 0;
            faceFlags = 0;
        }
    };

    struct PairNvttPixelGLFormat
    {
        static const int32 WRONG_GL_VALUE = -1;
        nvtt::Format nvttFormat;
        PixelFormat davaFormat;
        //int32		 glFormat;

        PairNvttPixelGLFormat(nvtt::Format _nvttFormat, PixelFormat _davaFormat, uint32 _glFormat)
        {
            nvttFormat = _nvttFormat;
            davaFormat = _davaFormat;
            //glFormat   = _glFormat;
        }
    };

    const static PairNvttPixelGLFormat formatNamesMap[];

    static bool InitDecompressor(nvtt::Decompressor& dec, const FilePath& fileName);
    static bool InitDecompressor(nvtt::Decompressor& dec, File* file);
    static bool InitDecompressor(nvtt::Decompressor& dec, const uint8* mem, uint32 size);

    static bool ReadDxtFile(nvtt::Decompressor& dec, Vector<Image*>& imageSet, int32 baseMipMap, bool forceSoftwareConvertation);
    static PixelFormat GetPixelFormat(nvtt::Decompressor& dec);
    static PixelFormat GetPixelFormatByNVTTFormat(nvtt::Format nvttFormat);
    static nvtt::Format GetNVTTFormatByPixelFormat(PixelFormat pixelFormat);
    static bool IsAtcFormat(nvtt::Format format);
    static bool GetInfo(nvtt::Decompressor& dec, DDSInfo& info);
    static uint32 GetDataSize(nvtt::Decompressor& dec);
    static uint32 GetCubeFaceId(uint32 nvttFaceDesc, int faceIndex);
    static void SwapBRChannels(uint8* data, uint32 size);

private:
    static bool DecompressAtc(const nvtt::Decompressor& dec, DDSInfo info, PixelFormat format, Vector<Image*>& imageSet, int32 baseMipMap);
    static bool DecompressDxt(const nvtt::Decompressor& dec, DDSInfo info, Vector<Image*>& imageSet, int32 baseMipMap);
};

const NvttHelper::PairNvttPixelGLFormat NvttHelper::formatNamesMap[] =
{
//FORMAT_NAMES_MAP_COUNT should be increased in case of addition to set
#if defined(GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
  NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1, FORMAT_DXT1, GL_COMPRESSED_RGB_S3TC_DXT1_EXT),
#else
  NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1, FORMAT_DXT1, PairNvttPixelGLFormat::WRONG_GL_VALUE),
#endif //GL_COMPRESSED_RGB_S3TC_DXT1_EXT
    
#if defined(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
  NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1a, FORMAT_DXT1A, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT),
#else
  NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1a, FORMAT_DXT1A, PairNvttPixelGLFormat::WRONG_GL_VALUE),
#endif //GL_COMPRESSED_RGBA_S3TC_DXT1_EXT

#if defined(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT)
  NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT3, FORMAT_DXT3, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT),
#else
  NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT3, FORMAT_DXT3, PairNvttPixelGLFormat::WRONG_GL_VALUE),
#endif //GL_COMPRESSED_RGBA_S3TC_DXT3_EXT

#if defined(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
  NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT5, FORMAT_DXT5, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT),
  NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT5n, FORMAT_DXT5NM, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT),
#else
  NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT5, FORMAT_DXT5, PairNvttPixelGLFormat::WRONG_GL_VALUE),
  NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT5n, FORMAT_DXT5NM, PairNvttPixelGLFormat::WRONG_GL_VALUE),
#endif //GL_COMPRESSED_RGBA_S3TC_DXT5_EXT

  NvttHelper::PairNvttPixelGLFormat(nvtt::Format_ATC_RGB, FORMAT_ATC_RGB, 0),
  NvttHelper::PairNvttPixelGLFormat(nvtt::Format_ATC_RGBA_EXPLICIT_ALPHA, FORMAT_ATC_RGBA_EXPLICIT_ALPHA, 0),
  NvttHelper::PairNvttPixelGLFormat(nvtt::Format_ATC_RGBA_INTERPOLATED_ALPHA, FORMAT_ATC_RGBA_INTERPOLATED_ALPHA, 0),

  NvttHelper::PairNvttPixelGLFormat(nvtt::Format_RGBA, FORMAT_RGBA8888, PairNvttPixelGLFormat::WRONG_GL_VALUE),
};

bool NvttHelper::InitDecompressor(nvtt::Decompressor& dec, const FilePath& fileName)
{
    if (fileName.IsEmpty())
    {
        Logger::Error("[NvttHelper::InitDecompressor] try init with empty name");
        return false;
    }

    if (!dec.initWithDDSFile(fileName.GetAbsolutePathname().c_str()))
    {
        Logger::Error("[NvttHelper::InitDecompressor] Wrong dds file (%s).", fileName.GetAbsolutePathname().c_str());
        return false;
    }

    return true;
}

bool NvttHelper::InitDecompressor(nvtt::Decompressor& dec, File* file)
{
    if (NULL == file)
    {
        Logger::Error("[NvttHelper::InitDecompressor] Wrong handler.");
        return false;
    }

    file->Seek(0, File::SEEK_FROM_START);
    uint32 fileSize = file->GetSize();
    uint8* fileBuffer = new uint8[fileSize];
    file->Read(fileBuffer, fileSize);
    bool initied = dec.initWithDDSFile(fileBuffer, fileSize);
    file->Seek(0, File::SEEK_FROM_START);
    SafeDeleteArray(fileBuffer);
    return initied;
}

bool NvttHelper::InitDecompressor(nvtt::Decompressor& dec, const uint8* mem, uint32 size)
{
    if (NULL == mem || size == 0)
    {
        Logger::Error("[NvttHelper::InitDecompressor] Wrong buffer params.");
        return false;
    }

    if (!dec.initWithDDSFile(mem, size))
    {
        Logger::Error("[NvttHelper::InitDecompressor] Wrong buffer.");
        return false;
    }

    return true;
}

bool NvttHelper::ReadDxtFile(nvtt::Decompressor& dec, Vector<Image*>& imageSet, int32 baseMipMap, bool forceSoftwareConvertation)
{
    DDSInfo info;
    if (!GetInfo(dec, info))
    {
        Logger::Error("[NvttHelper::ReadDxtFile] Error during header reading.");
        return false;
    }

    if (0 == info.width || 0 == info.height || 0 == info.mipmapsCount)
    {
        Logger::Error("[NvttHelper::ReadDxtFile] Wrong mipmapsCount/width/height value in dds header.");
        return false;
    }

    baseMipMap = Min(baseMipMap, (int32)(info.mipmapsCount - 1));

    nvtt::Format format;
    if (!dec.getCompressionFormat(format))
    {
        Logger::Error("[NvttHelper::ReadDxtFile] Getting format information cause error.");
        return false;
    }

    //check hardware support, in case of rgb use nvtt to reorder bytes
    bool isHardwareSupport = PixelFormatDescriptor::GetPixelFormatDescriptor(GetPixelFormatByNVTTFormat(format)).isHardwareSupported;

    if (!forceSoftwareConvertation && isHardwareSupport)
    {
        PixelFormat pixFormat = GetPixelFormat(dec);
        if (pixFormat == FORMAT_INVALID)
        {
            return false;
        }

        uint8* compressedImges = new uint8[info.dataSize];

        if (!dec.getRawData(compressedImges, info.dataSize))
        {
            Logger::Error("[NvttHelper::ReadDxtFile] Reading compressed data cause error in nvtt lib.");

            SafeDeleteArray(compressedImges);
            return false;
        }

        if (format == Format_RGB)
        {
            SwapBRChannels(compressedImges, info.dataSize);
        }

        bool retValue = true;

        uint32 offset = 0;
        for (uint32 faceIndex = 0; faceIndex < info.faceCount; ++faceIndex)
        {
            uint32 faceWidth = info.width;
            uint32 faceHeight = info.height;

            for (uint32 i = 0; i < info.mipmapsCount; ++i)
            {
                uint32 mipSize = 0;
                if (!dec.getMipmapSize(i, mipSize))
                {
                    retValue = false;
                    break;
                }

                if ((int32)i >= baseMipMap)
                { // load only actual image data
                    Image* innerImage = Image::Create(faceWidth, faceHeight, pixFormat);
                    innerImage->mipmapLevel = i - baseMipMap;

                    if (info.faceCount > 1)
                    {
                        innerImage->cubeFaceID = NvttHelper::GetCubeFaceId(info.faceFlags, faceIndex);
                    }

                    Memcpy(innerImage->data, compressedImges + offset, mipSize);
                    imageSet.push_back(innerImage);
                }

                offset += mipSize;
                faceWidth = Max((uint32)1, faceWidth / 2);
                faceHeight = Max((uint32)1, faceHeight / 2);
            }
        }

        SafeDeleteArray(compressedImges);
        return retValue;
    }
    else
    {
#if defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_IPHONE__)
        Logger::Error("[NvttHelper::ReadDxtFile] Android should have hardware decoding of DDS. iPhone should have no support of DDS.");
        return false;
#else //#if defined (__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_IPHONE__)

        if (IsAtcFormat(format))
        {
            return DecompressAtc(dec, info, GetPixelFormatByNVTTFormat(format), imageSet, baseMipMap);
        }
        else
        {
            return DecompressDxt(dec, info, imageSet, baseMipMap);
        }
        
#endif //#if defined (__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_IPHONE__)
    }

    return false;
}

PixelFormat NvttHelper::GetPixelFormat(nvtt::Decompressor& dec)
{
    nvtt::Format innerFormat;

    if (!dec.getCompressionFormat(innerFormat))
    {
        Logger::Error("[NvttHelper::GetPixelFormat] Wrong dds file compression format.");
        return FORMAT_INVALID;
    }

    PixelFormat format = NvttHelper::GetPixelFormatByNVTTFormat(innerFormat);
    if (format == FORMAT_INVALID)
    {
        Logger::Error("[NvttHelper::GetPixelFormat] Can't map nvtt format to pixel format");
    }

    return format;
}

PixelFormat NvttHelper::GetPixelFormatByNVTTFormat(nvtt::Format nvttFormat)
{
    PixelFormat retValue = FORMAT_INVALID;
    for (uint32 i = 0; i < Format_COUNT; ++i)
    {
        if (formatNamesMap[i].nvttFormat == nvttFormat)
        {
            retValue = formatNamesMap[i].davaFormat;
            break;
        }
    }
    return retValue;
}

nvtt::Format NvttHelper::GetNVTTFormatByPixelFormat(PixelFormat pixelFormat)
{
    //bc5 is unsupported, used to determinate fail in search
    for (uint32 i = 0; i < Format_COUNT; ++i)
    {
        if (formatNamesMap[i].davaFormat == pixelFormat)
        {
            return formatNamesMap[i].nvttFormat;
        }
    }

    return Format_BC5;
}

bool NvttHelper::IsAtcFormat(nvtt::Format format)
{
    return (format == Format_ATC_RGB || format == Format_ATC_RGBA_EXPLICIT_ALPHA || format == Format_ATC_RGBA_INTERPOLATED_ALPHA);
}

bool NvttHelper::GetInfo(nvtt::Decompressor& dec, DDSInfo& info)
{
    bool retVal = dec.getInfo(info.mipmapsCount, info.width, info.height, info.dataSize, info.headerSize, info.faceCount, info.faceFlags);
    if (!retVal)
    {
        Logger::Error("[NvttHelper::GetInfo] Error: can't read info from DDS file.");
    }

    return retVal;
}



uint32 NvttHelper::GetDataSize(nvtt::Decompressor& dec)
{
    DDSInfo info;
    GetInfo(dec, info);
    return info.dataSize;
}


uint32 NvttHelper::GetCubeFaceId(uint32 nvttFaceDesc, int faceIndex)
{
    uint32 faceId = Texture::INVALID_CUBEMAP_FACE;

    if (faceIndex >= 0 && faceIndex < 6)
    {
        int bitIndex = 0;
        int faceIdIndex = 0;
        for (int i = 0; i < 6; ++i)
        {
            if ((nvttFaceDesc >> i) & 1)
            {
                bitIndex++;

                if (bitIndex >= (faceIndex + 1))
                {
                    faceIdIndex = i;
                    break;
                }
            }
        }

        static uint32 faceIndexMap[] = {
            rhi::TEXTURE_FACE_POSITIVE_X,
            rhi::TEXTURE_FACE_NEGATIVE_X,
            rhi::TEXTURE_FACE_POSITIVE_Y,
            rhi::TEXTURE_FACE_NEGATIVE_Y,
            rhi::TEXTURE_FACE_POSITIVE_Z,
            rhi::TEXTURE_FACE_NEGATIVE_Z
        };

        faceId = faceIndexMap[faceIdIndex];
    }

    return faceId;
}

void NvttHelper::SwapBRChannels(uint8* data, uint32 size)
{
    for (uint32 i = 0; i < size; i += 4)
    {
        //RGBA <-> BGRA

        uint8* rComponent = data + i;

        uint8* bComponent = data + i + 2;
        uint8 tmp = *rComponent;
        *rComponent = *bComponent;
        *bComponent = tmp;
    }
}

bool NvttHelper::DecompressDxt(const nvtt::Decompressor& dec, DDSInfo info, Vector<Image*>& imageSet, int32 baseMipMap)
{
    for (uint32 faceIndex = 0; faceIndex < info.faceCount; ++faceIndex)
    {
        uint32 faceWidth = info.width;
        uint32 faceHeight = info.height;

        for (int32 i = 0; i < baseMipMap; ++i)
        {
            faceWidth = Max((uint32)1, faceWidth / 2);
            faceHeight = Max((uint32)1, faceHeight / 2);
        }

        for (uint32 i = baseMipMap; i < info.mipmapsCount; ++i)
        {
            // load only actual image data
            Image* innerImage = Image::Create(faceWidth, faceHeight, FORMAT_RGBA8888);
            if (dec.process(innerImage->data, innerImage->dataSize, i, faceIndex))
            {
                SwapBRChannels(innerImage->data, innerImage->dataSize);

                innerImage->mipmapLevel = i - baseMipMap;

                if (info.faceCount > 1)
                {
                    innerImage->cubeFaceID = NvttHelper::GetCubeFaceId(info.faceFlags, faceIndex);
                }

                imageSet.push_back(innerImage);
            }
            else
            {
                Logger::Error("nvtt lib compression fail.");
                SafeRelease(innerImage);
                return false;
            }

            faceWidth = Max((uint32)1, faceWidth / 2);
            faceHeight = Max((uint32)1, faceHeight / 2);
        }
    }
    return true;
}

bool NvttHelper::DecompressAtc(const nvtt::Decompressor& dec, DDSInfo info, PixelFormat format, Vector<Image*>& imageSet, int32 baseMipMap)
{
#if defined(__DAVAENGINE_WIN_UAP__)
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return false;

#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
#if defined(__DAVAENGINE_MACOS__)
    if (format == FORMAT_ATC_RGBA_INTERPOLATED_ALPHA)
    {
        Logger::Error("Decompressing FORMAT_ATC_RGBA_INTERPOLATED_ALPHA disabled on OSX platform, because of bug in qualcomm library");
        return false;
    }
#endif
    uint8* compressedImges = new uint8[info.dataSize];

    if (!dec.getRawData(compressedImges, info.dataSize))
    {
        Logger::Error("[NvttHelper::DecompressAtc] Reading compressed data cause error in nvtt lib.");

        SafeDeleteArray(compressedImges);
        return false;
    }

    bool res = true;
    unsigned char* buffer = compressedImges;
    const int32 qualcommFormat = QualcommHelper::GetQualcommFormat(format);

    for (uint32 faceIndex = 0; faceIndex < info.faceCount; ++faceIndex)
    {
        uint32 faceWidth = info.width;
        uint32 faceHeight = info.height;

        for (int32 i = 0; i < baseMipMap; ++i)
        {
            unsigned int mipMapSize = 0;
            dec.getMipmapSize(i, mipMapSize);
            buffer += mipMapSize;

            faceWidth = Max((uint32)1, faceWidth / 2);
            faceHeight = Max((uint32)1, faceHeight / 2);
        }

        for (uint32 i = baseMipMap; i < info.mipmapsCount; ++i)
        {
            TQonvertImage srcImg = { 0 };
            TQonvertImage dstImg = { 0 };

            srcImg.nWidth = faceWidth;
            srcImg.nHeight = faceHeight;
            srcImg.nFormat = qualcommFormat;
            dec.getMipmapSize(i, srcImg.nDataSize);
            srcImg.pData = buffer;
            buffer += srcImg.nDataSize;

            dstImg.nWidth = faceWidth;
            dstImg.nHeight = faceHeight;
            dstImg.nFormat = Q_FORMAT_RGBA_8UI;
            dstImg.nDataSize = 0;
            dstImg.pData = NULL;

            if (Qonvert(&srcImg, &dstImg) != Q_SUCCESS ||
                dstImg.nDataSize == 0)
            {
                Logger::Error("[NvttHelper::DecompressAtc] Reading decompress atc data.");
                res = false;
                break;
            }

            dstImg.pData = new unsigned char[dstImg.nDataSize];
            if (Qonvert(&srcImg, &dstImg) != Q_SUCCESS)
            {
                Logger::Error("[NvttHelper::DecompressAtc] Reading decompress atc data.");
                SafeDeleteArray(dstImg.pData);

                res = false;
                break;
            }

            Image* innerImage = Image::CreateFromData(faceWidth, faceHeight, FORMAT_RGBA8888, dstImg.pData);
            innerImage->mipmapLevel = i - baseMipMap;

            if (info.faceCount > 1)
            {
                innerImage->cubeFaceID = NvttHelper::GetCubeFaceId(info.faceFlags, faceIndex);
            }

            //SwapBRChannels(innerImage->data, innerImage->dataSize);
            imageSet.push_back(innerImage);

            faceWidth = Max((uint32)1, faceWidth / 2);
            faceHeight = Max((uint32)1, faceHeight / 2);

            SafeDeleteArray(dstImg.pData);
        }
    }

    SafeDeleteArray(compressedImges);
    return res;
#else
    return false;
#endif //defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_WIN32__)
}

LibDdsHelper::LibDdsHelper()
{
    name.assign("DDS");
    supportedExtensions.push_back(".dds");
    supportedFormats = { { FORMAT_ATC_RGB,
                           FORMAT_ATC_RGBA_EXPLICIT_ALPHA,
                           FORMAT_ATC_RGBA_INTERPOLATED_ALPHA,
                           FORMAT_DXT1,
                           FORMAT_REMOVED_DXT_1N,
                           FORMAT_DXT1A,
                           FORMAT_DXT3,
                           FORMAT_DXT5,
                           FORMAT_DXT5NM,
                           FORMAT_RGBA8888 } };
}

bool LibDdsHelper::IsMyImage(File* infile) const
{
    DDSFile::FileHeader header = DDSFile::ReadHeader(infile);
    return header.IsValid();
}

eErrorCode LibDdsHelper::ReadFile(File* infile, Vector<Image*>& imageSet, int32 baseMipMap) const
{
    return ReadFile(infile, imageSet, baseMipMap, false);
}

eErrorCode LibDdsHelper::WriteFile(const FilePath& outFileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const
{
    //creating tmp dds file, nvtt accept only filename.dds as input, because of this the last letter befor "." should be changed to "_".
    if (!outFileName.IsEqualToExtension(".dds"))
    {
        Logger::Error("[LibDdsHelper::WriteFile] Wrong output file name specified: '%s'", outFileName.GetAbsolutePathname().c_str());
        return eErrorCode::ERROR_FILE_FORMAT_INCORRECT;
    }

    if (compressionFormat == FORMAT_ATC_RGB ||
        compressionFormat == FORMAT_ATC_RGBA_EXPLICIT_ALPHA ||
        compressionFormat == FORMAT_ATC_RGBA_INTERPOLATED_ALPHA)
    {
        return WriteAtcFile(outFileName, imageSet, compressionFormat) ? eErrorCode::SUCCESS : eErrorCode::ERROR_WRITE_FAIL;
    }
    else
    {
        Vector<Vector<Image*>> imageSets;
        imageSets.push_back(imageSet);
        return WriteDxtFile(outFileName, imageSets, compressionFormat, false) ? eErrorCode::SUCCESS : eErrorCode::ERROR_WRITE_FAIL;
    }
}

eErrorCode LibDdsHelper::WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const
{
    if (imageSet.size() != Texture::CUBE_FACE_COUNT)
    {
        Logger::Error("[LibDdsHelper::WriteFile] Wrong input image set.");
        return eErrorCode::ERROR_WRITE_FAIL;
    }

    //creating tmp dds file, nvtt accept only filename.dds as input, because of this the last letter befor "." should be changed to "_".
    if (!fileName.IsEqualToExtension(".dds"))
    {
        Logger::Error("[LibDdsHelper::WriteFile] Wrong output file name specifed: '%s'", fileName.GetAbsolutePathname().c_str());
        return eErrorCode::ERROR_FILE_FORMAT_INCORRECT;
    }

    if (compressionFormat == FORMAT_ATC_RGB ||
        compressionFormat == FORMAT_ATC_RGBA_EXPLICIT_ALPHA ||
        compressionFormat == FORMAT_ATC_RGBA_INTERPOLATED_ALPHA)
    {
        return WriteAtcFileAsCubemap(fileName, imageSet, compressionFormat) ? eErrorCode::SUCCESS : eErrorCode::ERROR_WRITE_FAIL;
    }
    else
    {
        return WriteDxtFile(fileName, imageSet, compressionFormat, true) ? eErrorCode::SUCCESS : eErrorCode::ERROR_WRITE_FAIL;
    }
}

ImageInfo LibDdsHelper::GetImageInfo(File* infile) const
{
    const DDSFile::FileHeader fileHeader = DDSFile::ReadHeader(infile);

    ImageInfo info;
    info.width = fileHeader.formatHeader.width;
    info.height = fileHeader.formatHeader.height;
    info.dataSize = infile->GetSize() - sizeof(DDSFile::FileHeader);
    info.format = static_cast<PixelFormat>(fileHeader.formatHeader.metadata.davaPixelFormat);
    if (info.format == FORMAT_INVALID)
    {
        nvtt::Decompressor dec;
        if (NvttHelper::InitDecompressor(dec, infile))
        {
            info.format = NvttHelper::GetPixelFormat(dec);
        }
    }
    info.mipmapsCount = fileHeader.formatHeader.mipMapCount;

    return info;
}

bool LibDdsHelper::AddCRCIntoMetaData(const FilePath& filePathname) const
{
    ScopedPtr<File> ddsFile(File::Create(filePathname, File::READ | File::OPEN));
    if (!ddsFile)
    {
        Logger::Error("[LibDdsHelper::AddCRCIntoMetaData] cannot open file %s", filePathname.GetStringValue().c_str());
        return false;
    }

    DDSFile::FileHeader header = DDSFile::ReadHeader(ddsFile);
    if (!header.IsValid())
    {
        Logger::Error("[LibDdsHelper::AddCRCIntoMetaData] is not DDS file %s", filePathname.GetStringValue().c_str());
        return false;
    }

    if (header.formatHeader.metadata.crcTag == METADATA_CRC_TAG)
    {
        Logger::Error("[LibDdsHelper::AddCRCIntoMetaData] CRC is already added into %s", filePathname.GetStringValue().c_str());
        return false;
    }

    if (header.formatHeader.metadata.crcTag != 0 || header.formatHeader.metadata.crcValue != 0)
    {
        Logger::Error("[LibDdsHelper::AddCRCIntoMetaData] reserved for CRC place is used in %s", filePathname.GetStringValue().c_str());
        return false;
    }

    const uint32 fileSize = ddsFile->GetSize();
    Vector<char8> fileBuffer(fileSize);

    ddsFile->Seek(0, File::SEEK_FROM_START);
    if (ddsFile->Read(fileBuffer.data(), fileSize) != fileSize)
    {
        Logger::Error("[LibDdsHelper::AddCRCIntoMetaData]: cannot read from file %s", filePathname.GetStringValue().c_str());
        return false;
    }

    ddsFile.reset(); //to close file for reading

    DDSFile::FileHeader* outHeader = reinterpret_cast<DDSFile::FileHeader*>(fileBuffer.data());
    outHeader->formatHeader.metadata.crcTag = METADATA_CRC_TAG;
    outHeader->formatHeader.metadata.crcValue = CRC32::ForBuffer(fileBuffer.data(), fileSize);

    ddsFile.reset(File::Create(filePathname, File::WRITE | File::CREATE));
    if (fileSize != ddsFile->Write(fileBuffer.data(), fileSize))
    {
        Logger::Error("[LibDdsHelper::AddCRCIntoMetaData]: cannot write to file %s", filePathname.GetAbsolutePathname().c_str());
        return false;
    }

    return true;
}

uint32 LibDdsHelper::GetCRCFromFile(const FilePath& filePathname) const
{
    ScopedPtr<File> ddsFile(File::Create(filePathname, File::READ | File::OPEN));
    if (!ddsFile)
    {
        Logger::Error("[LibDdsHelper::GetCRCFromFile] cannot open file %s", filePathname.GetStringValue().c_str());
        return false;
    }

    DDSFile::FileHeader header = DDSFile::ReadHeader(ddsFile);
    if (!header.IsValid())
    {
        Logger::Error("[LibDdsHelper::GetCRCFromFile] is not DDS file %s", filePathname.GetStringValue().c_str());
        return false;
    }

    if (header.formatHeader.metadata.crcTag == METADATA_CRC_TAG)
    {
        return header.formatHeader.metadata.crcValue;
    }

    ddsFile.reset(); //to close file before crc calculation
    return CRC32::ForFile(filePathname);
}

eErrorCode LibDdsHelper::ReadFile(File* file, Vector<Image*>& imageSet, int32 baseMipMap, bool forceSoftwareConvertation)
{
    if (nullptr == file)
    {
        return eErrorCode::ERROR_FILE_NOTFOUND;
    }
    nvtt::Decompressor dec;

    if (!NvttHelper::InitDecompressor(dec, file))
    {
        return eErrorCode::ERROR_FILE_FORMAT_INCORRECT;
    }

    return NvttHelper::ReadDxtFile(dec, imageSet, baseMipMap, forceSoftwareConvertation) ? eErrorCode::SUCCESS : eErrorCode::ERROR_READ_FAIL;
}

bool LibDdsHelper::DecompressImageToRGBA(const Image& image, Vector<Image*>& imageSet, bool forceSoftwareConvertation)
{
    if (!(image.format >= FORMAT_DXT1 && image.format <= FORMAT_DXT5NM))
    {
        Logger::Error("[LibDdsHelper::DecompressImageToRGBA] Wrong copression format (%d).", image.format);
        return false;
    }
    nvtt::Format innerComprFormat = NvttHelper::GetNVTTFormatByPixelFormat(image.format);
    if (nvtt::Format_BC5 == innerComprFormat)
    { //bc5 is unsupported, used to determinate fail in search
        Logger::Error("[LibDdsHelper::DecompressImageToRGBA] Can't work with nvtt::Format_BC5.");
        return false;
    }

    InputOptions inputOptions;
    inputOptions.setTextureLayout(TextureType_2D, image.width, image.height);
    inputOptions.setMipmapGeneration(false);

    CompressionOptions compressionOptions;
    compressionOptions.setFormat(innerComprFormat);
    if (FORMAT_DXT5NM == image.format)
    {
        inputOptions.setNormalMap(true);
    }

    uint32 headerSize = DECOMPRESSOR_MIN_HEADER_SIZE;
    uint8* imageHeaderBuffer = new uint8[headerSize];

    uint32 realHeaderSize = nvtt::Decompressor::getHeader(imageHeaderBuffer, headerSize, inputOptions, compressionOptions);

    nvtt::Decompressor dec;

    uint8* compressedImageBuffer = new uint8[realHeaderSize + image.dataSize];
    Memcpy(compressedImageBuffer, imageHeaderBuffer, realHeaderSize);
    Memcpy(compressedImageBuffer + realHeaderSize, image.data, image.dataSize);
    SafeDeleteArray(imageHeaderBuffer);

    bool retValue = NvttHelper::InitDecompressor(dec, compressedImageBuffer, realHeaderSize + image.dataSize);
    if (retValue)
    {
        retValue = NvttHelper::ReadDxtFile(dec, imageSet, 0, forceSoftwareConvertation);
    }

    SafeDeleteArray(compressedImageBuffer);
    return retValue;
}


bool LibDdsHelper::WriteAtcFile(const FilePath& fileNameOriginal, const Vector<Image*>& imageSet, PixelFormat compressionFormat)
{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    DVASSERT_MSG(false, "Qualcomm doesn't provide texture converter library for ios/android");
    return false;

#elif defined(__DAVAENGINE_WIN_UAP__)
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return false;

#else

    if (compressionFormat != FORMAT_ATC_RGB &&
        compressionFormat != FORMAT_ATC_RGBA_EXPLICIT_ALPHA &&
        compressionFormat != FORMAT_ATC_RGBA_INTERPOLATED_ALPHA)
    {
        Logger::Error("[LibDdsHelper::WriteAtcFile] Wrong copression format (%d).", compressionFormat);
        return false;
    }

    //VI: calculate image buffer size
    uint32 compressedDataSize = 0;
    Vector<uint32> mipSize;
    mipSize.resize(imageSet.size());
    uint32 dataCount = static_cast<uint32>(imageSet.size());
    if (imageSet.size() == 0)
    {
        Logger::Error("[LibDdsHelper::WriteAtcFile] Empty income image vector.");
        return false;
    }

    auto pixelSize = PixelFormatDescriptor::GetPixelFormatSizeInBytes(imageSet[0]->format);
    auto atcFormat = QualcommHelper::GetQualcommFormat(imageSet[0]->format);
    for (uint32 i = 0; i < dataCount; ++i)
    {
        TQonvertImage srcImg = { 0 };

        srcImg.nWidth = imageSet[i]->width;
        srcImg.nHeight = imageSet[i]->height;
        srcImg.nFormat = atcFormat;
        srcImg.nDataSize = imageSet[i]->width * imageSet[i]->height * pixelSize;
        srcImg.pData = imageSet[i]->data;

        TQonvertImage dstImg = { 0 };
        dstImg.nWidth = imageSet[i]->width;
        dstImg.nHeight = imageSet[i]->height;
        dstImg.nFormat = QualcommHelper::GetQualcommFormat(compressionFormat);
        dstImg.nDataSize = 0;
        dstImg.pData = NULL;

        if (Qonvert(&srcImg, &dstImg) != Q_SUCCESS || dstImg.nDataSize == 0)
        {
            Logger::Error("[LibDdsHelper::WriteAtcFile] Error converting (%s).", fileNameOriginal.GetAbsolutePathname().c_str());
            return false;
        }
        compressedDataSize += dstImg.nDataSize;
        mipSize[i] = dstImg.nDataSize;
    }

    //VI: convert faces
    unsigned int headerSize = DECOMPRESSOR_MIN_HEADER_SIZE;
    Vector<unsigned char> outFileBufferData(headerSize + compressedDataSize);
    unsigned char* header = outFileBufferData.data();
    unsigned char* compressedData = header + headerSize;
    unsigned char* buffer = compressedData;
    for (uint32 i = 0; i < dataCount; ++i)
    {
        TQonvertImage srcImg = { 0 };

        srcImg.nWidth = imageSet[i]->width;
        srcImg.nHeight = imageSet[i]->height;
        srcImg.nFormat = atcFormat;
        srcImg.nDataSize = imageSet[i]->width * imageSet[i]->height * pixelSize;
        srcImg.pData = imageSet[i]->data;

        TQonvertImage dstImg = { 0 };
        dstImg.nWidth = imageSet[i]->width;
        dstImg.nHeight = imageSet[i]->height;
        dstImg.nFormat = QualcommHelper::GetQualcommFormat(compressionFormat);
        dstImg.nDataSize = mipSize[i];
        dstImg.pData = buffer;
        buffer += dstImg.nDataSize;

        if (Qonvert(&srcImg, &dstImg) != Q_SUCCESS || dstImg.nDataSize == 0)
        {
            Logger::Error("[LibDdsHelper::WriteAtcFile] Error converting (%s).", fileNameOriginal.GetAbsolutePathname().c_str());
            return false;
        }
    }

    nvtt::Format innerComprFormat = NvttHelper::GetNVTTFormatByPixelFormat(compressionFormat);

    nvtt::TextureType textureType = nvtt::TextureType_2D;
    InputOptions inputOptions;
    inputOptions.setTextureLayout(textureType, imageSet[0]->width, imageSet[0]->height);
    inputOptions.setMipmapGeneration(dataCount > 1, dataCount - 1);

    CompressionOptions compressionOptions;
    compressionOptions.setFormat(innerComprFormat);

    nvtt::Decompressor decompress;
    const uint32 realHeaderSize = decompress.getHeader(header, headerSize, inputOptions, compressionOptions);

    const FilePath fileName = FilePath::CreateWithNewExtension(fileNameOriginal, "_dds");
    bool res = false;
    File* file = File::Create(fileName, File::CREATE | File::WRITE);
    if (file)
    {
        { //Store PixelFormat format in metadata
            DVASSERT(realHeaderSize >= sizeof(DDSFile::FileHeader));
            DDSFile::FileHeader* outHeader = reinterpret_cast<DDSFile::FileHeader*>(header);
            outHeader->formatHeader.metadata.davaPixelFormat = compressionFormat;
        }

        file->Write(header, realHeaderSize);
        file->Write(compressedData, compressedDataSize);

        SafeRelease(file);
        FileSystem::Instance()->DeleteFile(fileNameOriginal);
        res = FileSystem::Instance()->MoveFile(fileName, fileNameOriginal, true);
        if (!res)
            Logger::Error("[LibDdsHelper::WriteDxtFile] Temporary dds file renamig failed.");
    }
    else
    {
        Logger::Error("[LibDdsHelper::WriteDxtFile] Temporary dds file renamig failed.");
    }

    return res;
#endif
}

bool LibDdsHelper::WriteDxtFile(const DAVA::FilePath& fileNameOriginal, const Vector<Vector<DAVA::Image*>>& imageSet, DAVA::PixelFormat compressionFormat, bool isCubemap)
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
    if (imageSet.empty() || imageSet[0].empty())
    {
        Logger::Error("[LibDdsHelper::WriteDxtFile] Empty incoming image vector.");
        return false;
    }

    Vector<Vector<Image*>> workingImages = imageSet;
    for (auto& imageVec : workingImages)
    {
        for_each(imageVec.begin(), imageVec.end(), SafeRetain<Image>);
    }
    SCOPE_EXIT
    {
        for (auto& imageVec : workingImages)
        {
            for_each(imageVec.begin(), imageVec.end(), SafeRelease<Image>);
        }
    };

    for (auto& imageVec : workingImages)
    {
        for (auto& image : imageVec)
        {
            auto inputFormat = image->format;

            if (inputFormat >= FORMAT_DXT1 && inputFormat <= FORMAT_DXT5NM)
            {
                Vector<Image*> decomprImages;
                if (DecompressImageToRGBA(*image, decomprImages, true) && decomprImages.size() == 1)
                {
                    SafeRelease(image);
                    image = decomprImages[0];
                }
                else
                {
                    Logger::Error("[LibDdsHelper::WriteDxtFile] Error during decompressing of DXT into RGBA");
                    for_each(decomprImages.begin(), decomprImages.end(), SafeRelease<Image>);
                    return false;
                }
            }
            else if (inputFormat != FORMAT_RGBA8888)
            {
                Image* newImage = Image::Create(image->width, image->height, FORMAT_RGBA8888);
                ImageConvert::ConvertImageDirect(image, newImage);

                newImage->cubeFaceID = image->cubeFaceID;
                newImage->mipmapLevel = image->mipmapLevel;

                SafeRelease(image);
                image = newImage;
            }
        }
    }

    nvtt::TextureType textureType = isCubemap ? nvtt::TextureType_Cube : nvtt::TextureType_2D;

    int facesCount = static_cast<int>(workingImages.size());
    int mipmapsCount = static_cast<int>(workingImages[0].size());
    InputOptions inputOptions;
    inputOptions.setTextureLayout(textureType, workingImages[0][0]->width, workingImages[0][0]->height);
    inputOptions.setMipmapGeneration(mipmapsCount > 1, mipmapsCount - 1);

    for (int f = 0; f < facesCount; ++f)
    {
        for (int m = 0; m < mipmapsCount; ++m)
        {
            Image* image = workingImages[f][m];
            ImageConvert::SwapRedBlueChannels(image);
            inputOptions.setMipmapData(image->data, image->width, image->height, 1, f, m);
            ImageConvert::SwapRedBlueChannels(image);
        }
    }

    CompressionOptions compressionOptions;
    compressionOptions.setFormat(innerComprFormat);
    if (FORMAT_DXT5NM == compressionFormat)
    {
        inputOptions.setNormalMap(true);
    }

    OutputOptions outputOptions;
    FilePath fileName = FilePath::CreateWithNewExtension(fileNameOriginal, "_dds");
    outputOptions.setFileName(fileName.GetAbsolutePathname().c_str());

    Compressor compressor;
    bool ret = compressor.process(inputOptions, compressionOptions, outputOptions);
    if (ret)
    {
        FileSystem::Instance()->DeleteFile(fileNameOriginal);
        if (!FileSystem::Instance()->MoveFile(fileName, fileNameOriginal, true))
        {
            Logger::Error("[LibDdsHelper::WriteDxtFile] Temporary dds file renamig failed.");
            ret = false;
        }
        else
        {
            //I hope we Will remove this code after refactoring of DAVA::Image class
            ScopedPtr<File> ddsFile(File::Create(fileNameOriginal, File::OPEN | File::READ | File::WRITE));

            DDSFile::FileHeader ddsHeader;
            ddsFile->Read(&ddsHeader);
            ddsHeader.formatHeader.metadata.davaPixelFormat = compressionFormat;

            ddsFile->Seek(0, File::SEEK_FROM_START);
            ddsFile->Write(&ddsHeader);
        }
    }
    else
    {
        Logger::Error("[LibDdsHelper::WriteDxtFile] Error during writing DDS file (%s).", fileName.GetAbsolutePathname().c_str());
    }

    return ret;
#endif //__DAVAENGINE_IPHONE__
}

bool LibDdsHelper::WriteAtcFileAsCubemap(const DAVA::FilePath& fileNameOriginal, const Vector<Vector<DAVA::Image*>>& imageSet, DAVA::PixelFormat compressionFormat)
{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    DVASSERT_MSG(false, "Qualcomm doesn't provide texture converter library for ios/android");
    return false;

#elif defined(__DAVAENGINE_WIN_UAP__)
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return false;
    
#else

    if (compressionFormat != FORMAT_ATC_RGB &&
        compressionFormat != FORMAT_ATC_RGBA_EXPLICIT_ALPHA &&
        compressionFormat != FORMAT_ATC_RGBA_INTERPOLATED_ALPHA)
    {
        Logger::Error("[LibDdsHelper::WriteAtcFile] Wrong copression format (%d).", compressionFormat);
        return false;
    }
    if (imageSet.size() == 0)
    {
        Logger::Error("[LibDdsHelper::WriteAtcFile] Empty income image vector.");
        return false;
    }

    //VI: calculate image buffer size
    const int facesCount = static_cast<int>(imageSet.size());
    const int mipmapsCount = static_cast<int>(imageSet[0].size());

    uint32 compressedDataSize = 0;
    Vector<Vector<uint32>> mipSize;
    mipSize.resize(facesCount);

    const int32 qualcommFormat = QualcommHelper::GetQualcommFormat(compressionFormat);

    auto pixelSize = PixelFormatDescriptor::GetPixelFormatSizeInBytes(imageSet[0][0]->format);
    auto atcFormat = QualcommHelper::GetQualcommFormat(imageSet[0][0]->format);
    for (int32 f = 0; f < facesCount; ++f)
    {
        mipSize[f].resize(mipmapsCount);

        for (int32 m = 0; m < mipmapsCount; ++m)
        {
            Image* image = imageSet[f][m];

            TQonvertImage srcImg = { 0 };

            srcImg.nWidth = image->width;
            srcImg.nHeight = image->height;
            srcImg.nFormat = atcFormat;
            srcImg.nDataSize = image->width * image->height * pixelSize;
            srcImg.pData = image->data;

            TQonvertImage dstImg = { 0 };
            dstImg.nWidth = image->width;
            dstImg.nHeight = image->height;
            dstImg.nFormat = qualcommFormat;
            dstImg.nDataSize = 0;
            dstImg.pData = NULL;

            if (Qonvert(&srcImg, &dstImg) != Q_SUCCESS || dstImg.nDataSize == 0)
            {
                Logger::Error("[LibDdsHelper::WriteAtcFile] Error converting (%s).", fileNameOriginal.GetAbsolutePathname().c_str());
                return false;
            }

            compressedDataSize += dstImg.nDataSize;
            mipSize[f][m] = dstImg.nDataSize;
        }
    }

    //VI: convert faces
    unsigned int headerSize = DECOMPRESSOR_MIN_HEADER_SIZE;
    Vector<unsigned char> outFileBufferData(headerSize + compressedDataSize);
    unsigned char* header = outFileBufferData.data();
    unsigned char* compressedData = header + headerSize;
    unsigned char* buffer = compressedData;
    for (int32 f = 0; f < facesCount; ++f)
    {
        for (int32 m = 0; m < mipmapsCount; ++m)
        {
            Image* image = imageSet[f][m];

            TQonvertImage srcImg = { 0 };

            srcImg.nWidth = image->width;
            srcImg.nHeight = image->height;
            srcImg.nFormat = atcFormat;
            srcImg.nDataSize = image->width * image->height * pixelSize;
            srcImg.pData = image->data;

            TQonvertImage dstImg = { 0 };
            dstImg.nWidth = image->width;
            dstImg.nHeight = image->height;
            dstImg.nFormat = qualcommFormat;
            dstImg.nDataSize = mipSize[f][m];
            dstImg.pData = buffer;
            buffer += dstImg.nDataSize;

            if (Qonvert(&srcImg, &dstImg) != Q_SUCCESS || dstImg.nDataSize == 0)
            {
                Logger::Error("[LibDdsHelper::WriteAtcFile] Error converting (%s).", fileNameOriginal.GetAbsolutePathname().c_str());
                return false;
            }
        }
    }

    nvtt::Format innerComprFormat = NvttHelper::GetNVTTFormatByPixelFormat(compressionFormat);

    nvtt::TextureType textureType = nvtt::TextureType_Cube;
    InputOptions inputOptions;
    inputOptions.setTextureLayout(textureType, imageSet[0][0]->width, imageSet[0][0]->height);

    inputOptions.setMipmapGeneration(mipmapsCount > 1, mipmapsCount - 1);

    CompressionOptions compressionOptions;
    compressionOptions.setFormat(innerComprFormat);

    nvtt::Decompressor decompress;
    const uint32 realHeaderSize = decompress.getHeader(header, headerSize, inputOptions, compressionOptions);

    const FilePath fileName = FilePath::CreateWithNewExtension(fileNameOriginal, "_dds");
    bool res = false;
    File* file = File::Create(fileName, File::CREATE | File::WRITE);
    if (file)
    {
        { //Store PixelFormat format in metadata
            DVASSERT(realHeaderSize >= sizeof(DDSFile::FileHeader));
            DDSFile::FileHeader* outHeader = reinterpret_cast<DDSFile::FileHeader*>(header);
            outHeader->formatHeader.metadata.davaPixelFormat = compressionFormat;
        }

        file->Write(header, realHeaderSize);
        file->Write(compressedData, compressedDataSize);

        SafeRelease(file);
        FileSystem::Instance()->DeleteFile(fileNameOriginal);
        res = FileSystem::Instance()->MoveFile(fileName, fileNameOriginal, true);
        if (!res)
            Logger::Error("[LibDdsHelper::WriteDxtFile] Temporary dds file renamig failed.");
    }
    else
    {
        Logger::Error("[LibDdsHelper::WriteDxtFile] Temporary dds file renamig failed.");
    }

    return res;
#endif
}
};
