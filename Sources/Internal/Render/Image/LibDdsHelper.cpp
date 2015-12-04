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


#include "Render/Image/LibDdsHelper.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageConvert.h"
#include "Render/Image/DDS/NvttHelper.h"
#include "Render/Image/DDS/QualcommHelper.h"

#include "Render/Texture.h"
#include "Render/PixelFormatDescriptor.h"

#include "Utils/CRC32.h"

namespace DAVA
{

#define MAKEFOURCC(ch0, ch1, ch2, ch3) ((uint32)((uint8)(ch0)) | ((uint32)((uint8)(ch1)) << 8) | ((uint32)((uint8)(ch2)) << 16) | ((uint32)((uint8)(ch3)) << 24))

const uint32 FOURCC_CRC = MAKEFOURCC('C', 'R', 'C', '_');

namespace dds
{
const uint32 FOURCC_DDS = MAKEFOURCC('D', 'D', 'S', ' ');

const uint32 FOURCC_DXT1 = MAKEFOURCC('D', 'X', 'T', '1');
const uint32 FOURCC_DXT2 = MAKEFOURCC('D', 'X', 'T', '2');
const uint32 FOURCC_DXT3 = MAKEFOURCC('D', 'X', 'T', '3');
const uint32 FOURCC_DXT4 = MAKEFOURCC('D', 'X', 'T', '4');
const uint32 FOURCC_DXT5 = MAKEFOURCC('D', 'X', 'T', '5');
const uint32 FOURCC_RXGB = MAKEFOURCC('R', 'X', 'G', 'B');
const uint32 FOURCC_ATI1 = MAKEFOURCC('A', 'T', 'I', '1');
const uint32 FOURCC_ATI2 = MAKEFOURCC('A', 'T', 'I', '2');
const uint32 FOURCC_BC4U = MAKEFOURCC('B', 'C', '4', 'U');
const uint32 FOURCC_BC4S = MAKEFOURCC('B', 'C', '4', 'S');
const uint32 FOURCC_BC5S = MAKEFOURCC('B', 'C', '5', 'S');

const uint32 FOURCC_RGBG = MAKEFOURCC('R', 'G', 'B', 'G');
const uint32 FOURCC_GRGB = MAKEFOURCC('G', 'R', 'G', 'B');

const uint32 FOURCC_ATC_RGB = MAKEFOURCC('A', 'T', 'C', ' ');
const uint32 FOURCC_ATC_RGBA_EXPLICIT_ALPHA = MAKEFOURCC('A', 'T', 'C', 'I');
const uint32 FOURCC_ATC_RGBA_INTERPOLATED_ALPHA = MAKEFOURCC('A', 'T', 'C', 'A');

const uint32 FOURCC_A2XY = MAKEFOURCC('A', '2', 'X', 'Y');

const uint32 FOURCC_UYVY = MAKEFOURCC('U', 'Y', 'V', 'Y');
const uint32 FOURCC_YUY2 = MAKEFOURCC('Y', 'U', 'Y', '2');

const uint32 FOURCC_DX10 = MAKEFOURCC('D', 'X', '1', '0');

const uint32 DDSCAPS_COMPLEX = 0x00000008U;
const uint32 DDSCAPS_TEXTURE = 0x00001000U;
const uint32 DDSCAPS_MIPMAP = 0x00400000U;
const uint32 DDSCAPS2_VOLUME = 0x00200000U;
const uint32 DDSCAPS2_CUBEMAP = 0x00000200U;

const uint32 DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400U;
const uint32 DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800U;
const uint32 DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000U;
const uint32 DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000U;
const uint32 DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000U;
const uint32 DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000U;
const uint32 DDSCAPS2_CUBEMAP_ALL_FACES = 0x0000FC00U;

enum D3D_FORMAT : uint32
{
    // 32 bit RGB formats.
    D3DFMT_R8G8B8 = 20,
    D3DFMT_A8R8G8B8 = 21,
    D3DFMT_X8R8G8B8 = 22,
    D3DFMT_R5G6B5 = 23,
    D3DFMT_X1R5G5B5 = 24,
    D3DFMT_A1R5G5B5 = 25,
    D3DFMT_A4R4G4B4 = 26,
    D3DFMT_R3G3B2 = 27,
    D3DFMT_A8 = 28,
    D3DFMT_A8R3G3B2 = 29,
    D3DFMT_X4R4G4B4 = 30,
    D3DFMT_A2B10G10R10 = 31,
    D3DFMT_A8B8G8R8 = 32,
    D3DFMT_X8B8G8R8 = 33,
    D3DFMT_G16R16 = 34,
    D3DFMT_A2R10G10B10 = 35,

    D3DFMT_A16B16G16R16 = 36,

    // Palette formats.
    D3DFMT_A8P8 = 40,
    D3DFMT_P8 = 41,

    // Luminance formats.
    D3DFMT_L8 = 50,
    D3DFMT_A8L8 = 51,
    D3DFMT_A4L4 = 52,
    D3DFMT_L16 = 81,

    // Floating point formats
    D3DFMT_R16F = 111,
    D3DFMT_G16R16F = 112,
    D3DFMT_A16B16G16R16F = 113,
    D3DFMT_R32F = 114,
    D3DFMT_G32R32F = 115,
    D3DFMT_A32B32G32R32F = 116,
};

enum DXGI_FORMAT : uint32
{
    DXGI_FORMAT_UNKNOWN = 0,

    DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
    DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
    DXGI_FORMAT_R32G32B32A32_UINT = 3,
    DXGI_FORMAT_R32G32B32A32_SINT = 4,

    DXGI_FORMAT_R32G32B32_TYPELESS = 5,
    DXGI_FORMAT_R32G32B32_FLOAT = 6,
    DXGI_FORMAT_R32G32B32_UINT = 7,
    DXGI_FORMAT_R32G32B32_SINT = 8,

    DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
    DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
    DXGI_FORMAT_R16G16B16A16_UNORM = 11,
    DXGI_FORMAT_R16G16B16A16_UINT = 12,
    DXGI_FORMAT_R16G16B16A16_SNORM = 13,
    DXGI_FORMAT_R16G16B16A16_SINT = 14,

    DXGI_FORMAT_R32G32_TYPELESS = 15,
    DXGI_FORMAT_R32G32_FLOAT = 16,
    DXGI_FORMAT_R32G32_UINT = 17,
    DXGI_FORMAT_R32G32_SINT = 18,

    DXGI_FORMAT_R32G8X24_TYPELESS = 19,
    DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
    DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
    DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,

    DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
    DXGI_FORMAT_R10G10B10A2_UNORM = 24,
    DXGI_FORMAT_R10G10B10A2_UINT = 25,

    DXGI_FORMAT_R11G11B10_FLOAT = 26,

    DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
    DXGI_FORMAT_R8G8B8A8_UNORM = 28,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
    DXGI_FORMAT_R8G8B8A8_UINT = 30,
    DXGI_FORMAT_R8G8B8A8_SNORM = 31,
    DXGI_FORMAT_R8G8B8A8_SINT = 32,

    DXGI_FORMAT_R16G16_TYPELESS = 33,
    DXGI_FORMAT_R16G16_FLOAT = 34,
    DXGI_FORMAT_R16G16_UNORM = 35,
    DXGI_FORMAT_R16G16_UINT = 36,
    DXGI_FORMAT_R16G16_SNORM = 37,
    DXGI_FORMAT_R16G16_SINT = 38,

    DXGI_FORMAT_R32_TYPELESS = 39,
    DXGI_FORMAT_D32_FLOAT = 40,
    DXGI_FORMAT_R32_FLOAT = 41,
    DXGI_FORMAT_R32_UINT = 42,
    DXGI_FORMAT_R32_SINT = 43,

    DXGI_FORMAT_R24G8_TYPELESS = 44,
    DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
    DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
    DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,

    DXGI_FORMAT_R8G8_TYPELESS = 48,
    DXGI_FORMAT_R8G8_UNORM = 49,
    DXGI_FORMAT_R8G8_UINT = 50,
    DXGI_FORMAT_R8G8_SNORM = 51,
    DXGI_FORMAT_R8G8_SINT = 52,

    DXGI_FORMAT_R16_TYPELESS = 53,
    DXGI_FORMAT_R16_FLOAT = 54,
    DXGI_FORMAT_D16_UNORM = 55,
    DXGI_FORMAT_R16_UNORM = 56,
    DXGI_FORMAT_R16_UINT = 57,
    DXGI_FORMAT_R16_SNORM = 58,
    DXGI_FORMAT_R16_SINT = 59,

    DXGI_FORMAT_R8_TYPELESS = 60,
    DXGI_FORMAT_R8_UNORM = 61,
    DXGI_FORMAT_R8_UINT = 62,
    DXGI_FORMAT_R8_SNORM = 63,
    DXGI_FORMAT_R8_SINT = 64,
    DXGI_FORMAT_A8_UNORM = 65,

    DXGI_FORMAT_R1_UNORM = 66,

    DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,

    DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
    DXGI_FORMAT_G8R8_G8B8_UNORM = 69,

    DXGI_FORMAT_BC1_TYPELESS = 70,
    DXGI_FORMAT_BC1_UNORM = 71,
    DXGI_FORMAT_BC1_UNORM_SRGB = 72,

    DXGI_FORMAT_BC2_TYPELESS = 73,
    DXGI_FORMAT_BC2_UNORM = 74,
    DXGI_FORMAT_BC2_UNORM_SRGB = 75,

    DXGI_FORMAT_BC3_TYPELESS = 76,
    DXGI_FORMAT_BC3_UNORM = 77,
    DXGI_FORMAT_BC3_UNORM_SRGB = 78,

    DXGI_FORMAT_BC4_TYPELESS = 79,
    DXGI_FORMAT_BC4_UNORM = 80,
    DXGI_FORMAT_BC4_SNORM = 81,

    DXGI_FORMAT_BC5_TYPELESS = 82,
    DXGI_FORMAT_BC5_UNORM = 83,
    DXGI_FORMAT_BC5_SNORM = 84,

    DXGI_FORMAT_B5G6R5_UNORM = 85,
    DXGI_FORMAT_B5G5R5A1_UNORM = 86,
    DXGI_FORMAT_B8G8R8A8_UNORM = 87,
    DXGI_FORMAT_B8G8R8X8_UNORM = 88
};

enum DDS_PIXELFORMAT_FLAGS : uint32
{
    DDPF_ALPHAPIXELS = 0x1,
    DDPF_ALPHA = 0x2,
    DDPF_FOURCC = 0x4,
    DDPF_RGB = 0x40,
    DDPF_YUV = 0x200,
    DDPF_LUMINANCE = 0x20000,
    DDPF_NORMAL = 0x80000000 // Custom nv flag
};

struct DDS_PIXELFORMAT
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
static_assert(sizeof(DDS_PIXELFORMAT) == 32, "Invalid DDS Pixelformat size");

struct DDS_RESERVED //uint32 reserved1[11]
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
static_assert(sizeof(DDS_RESERVED) == 44, "Invalid reserved[11] size");

struct DDS_HEADER
{
    uint32 size = 0;
    uint32 flags = 0;
    uint32 height = 0;
    uint32 width = 0;
    uint32 pitchOrLinearSize = 0;
    uint32 depth = 0;
    uint32 mipMapCount = 0;
    DDS_RESERVED reserved;
    DDS_PIXELFORMAT format;
    uint32 caps = 0;
    uint32 caps2 = 0;
    uint32 caps3 = 0;
    uint32 caps4 = 0;
    uint32 reserved2 = 0;
};
static_assert(sizeof(DDS_HEADER) == 124, "Invalid DDS Header size");

struct DDS_HEADER_DXT10
{
    uint32 dxgiFormat = 0;
    uint32 resourceDimension = 0;
    uint32 miscFlag = 0;
    uint32 arraySize = 0;
    uint32 miscFlags2 = 0;
};
static_assert(sizeof(DDS_HEADER_DXT10) == 20, "Invalid DDS DXT10 Header size");
}

class DDSHandler;
using DDSHandlerPtr = std::unique_ptr<DDSHandler>;

class DDSHandler
{
public:
    static DDSHandlerPtr CreateForFile(const FilePtr& file);

    ImageInfo GetImageInfo();
    bool GetTextures(Vector<Image*>& images, uint32 firstMip);
    bool GetCRC(uint32& crc);
    bool WriteCRC();
    bool WritePixelFormat(PixelFormat format);

private:
    explicit DDSHandler(const FilePtr& file);

    bool WriteMagicWord();
    bool WriteMainHeader();
    bool ReadMagicWord();
    bool ReadHeaders();
    void FetchPixelFormats();
    void FetchFacesInfo();
    bool IsSupportedFormat();

private:
    ScopedPtr<File> file;
    dds::DDS_HEADER mainHeader;
    std::unique_ptr<dds::DDS_HEADER_DXT10> extHeader;
    bool needConvertFormat = false;
    dds::D3D_FORMAT d3dPixelFormat;
    PixelFormat davaPixelFormat = FORMAT_INVALID;
    uint32 faceCount = 0;
    Array<rhi::TextureFace, Texture::CUBE_FACE_COUNT> faces;
};

DDSHandlerPtr DDSHandler::CreateForFile(const FilePtr& file)
{
    DVASSERT(file);

    DDSHandlerPtr ddsFile(new DDSHandler(file));
    uint32 pos = file->GetPos();

    if (ddsFile->ReadMagicWord() && ddsFile->ReadHeaders())
    {
        return ddsFile;
    }
    else
    {
        file->Seek(pos, File::SEEK_FROM_START);
        return nullptr;
    }
}

DDSHandler::DDSHandler(const FilePtr& _ddsFile)
    : file(_ddsFile)
{
}

bool DDSHandler::IsSupportedFormat()
{
    return (davaPixelFormat != FORMAT_INVALID);
}

ImageInfo DDSHandler::GetImageInfo()
{
    ImageInfo info;

    FetchPixelFormats();

    if (IsSupportedFormat())
    {
        info.width = mainHeader.width;
        info.height = mainHeader.height;

        info.format = davaPixelFormat;

        uint32 headersSize = sizeof(dds::FOURCC_DDS) + sizeof(dds::DDS_HEADER);
        if (extHeader)
        {
            headersSize += sizeof(dds::DDS_HEADER_DXT10);
        }

        info.dataSize = file->GetSize() - headersSize;
        info.mipmapsCount = (0 == mainHeader.mipMapCount) ? 1 : mainHeader.mipMapCount;
    }

    return info;
}

bool DDSHandler::ReadMagicWord()
{
    uint32 dataRead = 0;
    if (sizeof(dds::FOURCC_DDS) == file->Read(&dataRead))
    {
        return dds::FOURCC_DDS == dataRead;
    }
    return false;
}

bool DDSHandler::WriteMagicWord()
{
    if (sizeof(dds::FOURCC_DDS) == file->Write(&dds::FOURCC_DDS, sizeof(dds::FOURCC_DDS)))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool DDSHandler::WriteMainHeader()
{
    if (sizeof(mainHeader) == file->Write(&mainHeader, sizeof(mainHeader)))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool DDSHandler::ReadHeaders()
{
    DVVERIFY(!extHeader);

    if (sizeof(dds::DDS_HEADER) != file->Read(&mainHeader))
    {
        return false;
    }

    if ((mainHeader.format.flags & dds::DDPF_FOURCC) && (mainHeader.format.fourCC == dds::FOURCC_DX10))
    {
        extHeader.reset(new dds::DDS_HEADER_DXT10);
        if (sizeof(dds::DDS_HEADER_DXT10) != file->Read(&extHeader))
        {
            return false;
        }
    }

    return true;
}

bool DDSHandler::WriteCRC()
{
    FilePath filepath = file->GetFilename();
    if (mainHeader.reserved.crcTag == FOURCC_CRC)
    {
        Logger::Error("CRC is already added into %s", filepath.GetStringValue().c_str());
        return false;
    }

    if (mainHeader.reserved.crcTag != 0 || mainHeader.reserved.crcValue != 0)
    {
        Logger::Error("Reserved for CRC place is used in %s", filepath.GetStringValue().c_str());
        return false;
    }

    const uint32 fileSize = file->GetSize();
    Vector<char8> fileBuffer(fileSize);

    file->Seek(0, File::SEEK_FROM_START);
    if (file->Read(fileBuffer.data(), fileSize) != fileSize)
    {
        Logger::Error("Cannot read from file %s", filepath.GetStringValue().c_str());
        return false;
    }

    mainHeader.reserved.crcTag = FOURCC_CRC;
    mainHeader.reserved.crcValue = CRC32::ForBuffer(fileBuffer.data(), fileSize);

    file->Seek(0, File::SEEK_FROM_START);

    if (!WriteMagicWord() || !WriteMainHeader())
    {
        Logger::Error("Cannot write to file %s", filepath.GetAbsolutePathname().c_str());
        return false;
    }

    return true;
}

bool DDSHandler::WritePixelFormat(PixelFormat format)
{
    FilePath filepath = file->GetFilename();
    mainHeader.reserved.davaPixelFormat = format;
    file->Seek(0, File::SEEK_FROM_START);

    if (!WriteMagicWord() || !WriteMainHeader())
    {
        Logger::Error("Cannot write to file %s", filepath.GetAbsolutePathname().c_str());
        return false;
    }

    return true;
}

bool DDSHandler::GetCRC(uint32& crc)
{
    if (mainHeader.reserved.crcTag == FOURCC_CRC)
    {
        crc = mainHeader.reserved.crcValue;
        return true;
    }
    else
    {
        return false;
    }
}

void DDSHandler::FetchFacesInfo()
{
    if ((mainHeader.caps2 & dds::DDSCAPS2_CUBEMAP_ALL_FACES) == dds::DDSCAPS2_CUBEMAP_ALL_FACES)
    {
        faceCount = Texture::CUBE_FACE_COUNT;
        for (uint32 face = 0; face < faceCount; ++face)
        {
            faces[face] = static_cast<rhi::TextureFace>(face);
        }
    }
    else
    {
        static const uint32 ddsFaces[] = {
            dds::DDSCAPS2_CUBEMAP_POSITIVEX,
            dds::DDSCAPS2_CUBEMAP_NEGATIVEX,
            dds::DDSCAPS2_CUBEMAP_POSITIVEY,
            dds::DDSCAPS2_CUBEMAP_NEGATIVEY,
            dds::DDSCAPS2_CUBEMAP_POSITIVEZ,
            dds::DDSCAPS2_CUBEMAP_NEGATIVEZ
        };

        faceCount = 0;
        for (int face = 0; face < Texture::CUBE_FACE_COUNT; ++face)
        {
            if (mainHeader.caps2 & ddsFaces[face])
            {
                faces[faceCount++] = static_cast<rhi::TextureFace>(face);
            }
        }

        if (faceCount == 0)
        {
            faceCount = 1;
        }
    }
}

void DDSHandler::FetchPixelFormats()
{
    const uint32& flags = mainHeader.format.flags;
    const uint32& bitCount = mainHeader.format.RGBBitCount;
    const uint32& rMask = mainHeader.format.RBitMask;
    const uint32& gMask = mainHeader.format.GBitMask;
    const uint32& bMask = mainHeader.format.BBitMask;
    const uint32& aMask = mainHeader.format.ABitMask;
    bool flagRGB = (flags & dds::DDPF_RGB && !(flags & dds::DDPF_ALPHAPIXELS));
    bool flagRGBA = (flags & dds::DDPF_RGB && (flags & dds::DDPF_ALPHAPIXELS));
    bool flagAlpha = (flags & dds::DDPF_ALPHA) != 0;
    bool flagFourCC = (flags & dds::DDPF_FOURCC) != 0;

    needConvertFormat = false;
    davaPixelFormat = FORMAT_INVALID;

    if (flagRGBA)
    {
        if (rMask == 0xff && gMask == 0xff00 && bMask == 0xff0000 && aMask == 0xff000000)
        {
            davaPixelFormat = FORMAT_RGBA8888;
        }
        else if (rMask == 0x7c00 && gMask == 0x3e0 && bMask == 0x1f && aMask == 0x8000)
        {
            needConvertFormat = true;
            d3dPixelFormat = dds::D3DFMT_A1R5G5B5;
            davaPixelFormat = FORMAT_RGBA5551;
        }
        else if (rMask == 0xff0000 && gMask == 0xff00 && bMask == 0xff && aMask == 0xff000000)
        {
            needConvertFormat = true;
            d3dPixelFormat = dds::D3DFMT_A8R8G8B8;
            davaPixelFormat = FORMAT_RGBA8888;
        }
        else if (rMask == 0xf00 && gMask == 0xf0 && bMask == 0xf && aMask == 0xf000)
        {
            needConvertFormat = true;
            d3dPixelFormat = dds::D3DFMT_A4R4G4B4;
            davaPixelFormat = FORMAT_RGBA4444;
        }
    }
    else if (flagRGB)
    {
        if (rMask == 0xf800 && gMask == 0x7e0 && bMask == 0x1f)
        {
            davaPixelFormat = FORMAT_RGB565;
        }
        else if (rMask == 0xff0000 && gMask == 0xff00 && bMask == 0xff)
        {
            if (bitCount == 24)
            {
                davaPixelFormat = FORMAT_RGB888;
            }
            else if (bitCount == 32)
            {
                needConvertFormat = true;
                d3dPixelFormat = dds::D3DFMT_X8R8G8B8;
                davaPixelFormat = FORMAT_RGB888;
            }
        }

    }
    else if (flagAlpha && bitCount == 8 && aMask == 0xff)
    {
        davaPixelFormat = FORMAT_A8;
    }
    else if (flagFourCC)
    {
        if (mainHeader.format.fourCC == dds::FOURCC_DX10)
        {
            DVASSERT(extHeader);
            switch (extHeader->dxgiFormat)
            {
            case dds::DXGI_FORMAT_R8G8B8A8_UNORM:
            {
                davaPixelFormat = FORMAT_RGBA8888;
                break;
            }
            case dds::DXGI_FORMAT_B5G5R5A1_UNORM:
            {
                needConvertFormat = true;
                d3dPixelFormat = dds::D3DFMT_A1R5G5B5;
                davaPixelFormat = FORMAT_RGBA5551;
                break;
            }
            case dds::DXGI_FORMAT_B5G6R5_UNORM:
            {
                davaPixelFormat = FORMAT_RGB565;
                break;
            }
            case dds::DXGI_FORMAT_BC1_UNORM:
            {
                davaPixelFormat = FORMAT_DXT1;
                break;
            }
            case dds::DXGI_FORMAT_BC2_UNORM:
            {
                davaPixelFormat = FORMAT_DXT3;
                break;
            }
            case dds::DXGI_FORMAT_BC3_UNORM:
            {
                davaPixelFormat = (flags & dds::DDPF_NORMAL) ? FORMAT_DXT5NM : FORMAT_DXT5;
                break;
            }
            case dds::DXGI_FORMAT_R16G16B16A16_FLOAT:
            {
                davaPixelFormat = FORMAT_RGBA16161616;
                break;
            }
            case dds::DXGI_FORMAT_R32G32B32A32_FLOAT:
            {
                davaPixelFormat = FORMAT_RGBA32323232;
                break;
            }
            }
        }
        else
        {
            const uint32& fourcc = mainHeader.format.fourCC;
            if (fourcc == dds::FOURCC_DXT1)
            {
                davaPixelFormat = FORMAT_DXT1;
            }
            else if (fourcc == dds::FOURCC_DXT3)
            {
                davaPixelFormat = FORMAT_DXT3;
            }
            else if (fourcc == dds::FOURCC_DXT5)
            {
                davaPixelFormat = (flags & dds::DDPF_NORMAL) ? FORMAT_DXT5NM : FORMAT_DXT5;
            }
            else if (fourcc == dds::FOURCC_ATC_RGB)
            {
                davaPixelFormat = FORMAT_ATC_RGB;
            }
            else if (fourcc == dds::FOURCC_ATC_RGBA_EXPLICIT_ALPHA)
            {
                davaPixelFormat = FORMAT_ATC_RGBA_EXPLICIT_ALPHA;
            }
            else if (fourcc == dds::FOURCC_ATC_RGBA_INTERPOLATED_ALPHA)
            {
                davaPixelFormat = FORMAT_ATC_RGBA_INTERPOLATED_ALPHA;
            }
            else if (fourcc == dds::D3DFMT_A16B16G16R16F)
            {
                davaPixelFormat = FORMAT_RGBA16161616;
            }
            else if (fourcc == dds::D3DFMT_A32B32G32R32F)
            {
                davaPixelFormat = FORMAT_RGBA32323232;
            }
        }
    }
}

uint32 GetFormatSizeInBytes(dds::D3D_FORMAT format)
{
    switch (format)
    {
    case dds::D3DFMT_R3G3B2:
    case dds::D3DFMT_A8:
    case dds::D3DFMT_P8:
    case dds::D3DFMT_L8:
    case dds::D3DFMT_A4L4:
        return 1;

    case dds::D3DFMT_R5G6B5:
    case dds::D3DFMT_X1R5G5B5:
    case dds::D3DFMT_A1R5G5B5:
    case dds::D3DFMT_A4R4G4B4:
    case dds::D3DFMT_X4R4G4B4:
    case dds::D3DFMT_A8R3G3B2:
    case dds::D3DFMT_A8P8:
    case dds::D3DFMT_A8L8:
    case dds::D3DFMT_L16:
    case dds::D3DFMT_R16F:
        return 2;

    case dds::D3DFMT_R8G8B8:
        return 3;

    case dds::D3DFMT_A8R8G8B8:
    case dds::D3DFMT_A8B8G8R8:
    case dds::D3DFMT_X8R8G8B8:
    case dds::D3DFMT_X8B8G8R8:
    case dds::D3DFMT_A2B10G10R10:
    case dds::D3DFMT_A2R10G10B10:
    case dds::D3DFMT_G16R16:
    case dds::D3DFMT_G16R16F:
    case dds::D3DFMT_R32F:
        return 4;

    case dds::D3DFMT_A16B16G16R16:
    case dds::D3DFMT_A16B16G16R16F:
    case dds::D3DFMT_G32R32F:
        return 8;

    case dds::D3DFMT_A32B32G32R32F:
        return 16;

    default:
        DVASSERT(false && "undefined format");
    }
}

void ConvertImage(const uint8* srcData, Image* dstImage, dds::D3D_FORMAT srcFormat, PixelFormat dstFormat)
{
    DVASSERT(dstImage);
    DVASSERT(dstImage->format == dstFormat);

    const uint32& w = dstImage->width;
    const uint32& h = dstImage->height;

    uint32 srcPitch = w * GetFormatSizeInBytes(srcFormat);
    uint32 dstPitch = w * PixelFormatDescriptor::GetPixelFormatSizeInBytes(dstFormat);

    switch (srcFormat)
    {
    case dds::D3DFMT_A8R8G8B8:
    {
        ConvertDirect<BGRA8888, RGBA8888, ConvertBGRA8888toRGBA8888> convert;
        convert(srcData, w, h, srcPitch, dstImage->data, w, h, dstPitch);
        break;
    }
    case dds::D3DFMT_X8R8G8B8:
    {
        ConvertDirect<BGRA8888, RGB888, ConvertBGRA8888toRGB888> convert;
        convert(srcData, w, h, srcPitch, dstImage->data, w, h, dstPitch);
        break;
    }
    case dds::D3DFMT_A1R5G5B5:
    {
        ConvertDirect<uint16, uint16, ConvertARGB1555toRGBA5551> convert;
        convert(srcData, w, h, srcPitch, dstImage->data, w, h, dstPitch);
        break;
    }
    case dds::D3DFMT_A4R4G4B4:
    {
        ConvertDirect<uint16, uint16, ConvertARGB4444toRGBA4444> convert;
        convert(srcData, w, h, srcPitch, dstImage->data, w, h, dstPitch);
        break;
    }
    //     case dds::D3DFMT_A16B16G16R16F:
    //     {
    //         ConvertDirect<ABGR16161616, RGBA16161616, ConvertABGR16161616toRGBA16161616> convert;
    //         convert(srcData, w, h, srcPitch, dstImage->data, w, h, dstPitch);
    //         break;
    //     }
    //     case dds::D3DFMT_A32B32G32R32F:
    //     {
    //         ConvertDirect<ABGR32323232, RGBA32323232, ConvertABGR32323232toRGBA32323232> convert;
    //         convert(srcData, w, h, srcPitch, dstImage->data, w, h, dstPitch);
    //         break;
    //     }
    default:
        DVASSERT(false && "undefined format");
    }
}

bool DDSHandler::GetTextures(Vector<Image*>& images, uint32 baseMipMap)
{
    ImageInfo info = GetImageInfo();
    if (info.format == FORMAT_INVALID)
    {
        Logger::Error("Invalid/unsupported format of DDS file '%s'", file->GetFilename().GetStringValue().c_str());
        return false;
    }
    if (0 == info.width || 0 == info.height || 0 == info.mipmapsCount)
    {
        Logger::Error("Wrong mipmapsCount/width/height value for DDS file '%s'");
        return false;
    }

    FetchFacesInfo();

    baseMipMap = Min(baseMipMap, (info.mipmapsCount - 1));

    Vector<uint8> rawData(info.dataSize);
    uint8* dataBegin = rawData.data();
    auto readSize = file->Read(dataBegin, info.dataSize);
    if (readSize != info.dataSize)
    {
        Logger::Error("Can't read data from %s", file->GetFilename().GetStringValue().c_str());
        return false;
    }

    const uint32 bitsPerPixel = needConvertFormat ?
    GetFormatSizeInBytes(d3dPixelFormat) * 8 :
    PixelFormatDescriptor::GetPixelFormatSizeInBits(davaPixelFormat);

    uint32 offset = 0;
    for (uint32 faceIndex = 0; faceIndex < faceCount; ++faceIndex)
    {
        uint32 mipWidth = info.width;
        uint32 mipHeight = info.height;

        for (uint32 mip = 0; mip < info.mipmapsCount; ++mip)
        {
            uint32 bytesInMip = mipWidth * mipHeight * bitsPerPixel / 8;

            if (mip >= baseMipMap)
            {
                Image* innerImage = Image::Create(mipWidth, mipHeight, info.format);
                DVASSERT(innerImage);

                if (needConvertFormat)
                {
                    ConvertImage(dataBegin + offset, innerImage, d3dPixelFormat, davaPixelFormat);
                }
                else
                {
                    Memcpy(innerImage->data, dataBegin + offset, innerImage->dataSize);
                }

                innerImage->mipmapLevel = mip - baseMipMap;

                if (faceCount > 1)
                {
                    innerImage->cubeFaceID = faces[faceIndex];
                }

                images.push_back(innerImage);
            }

            offset += bytesInMip;
            mipWidth = Max(1U, mipWidth / 2);
            mipHeight = Max(1U, mipHeight / 2);
        }
    }

    return true;
}

LibDdsHelper::LibDdsHelper()
    : ImageFormatInterface(
      IMAGE_FORMAT_DDS,
      "DDS",
      { ".dds" },
      { FORMAT_ATC_RGB,
        FORMAT_ATC_RGBA_EXPLICIT_ALPHA,
        FORMAT_ATC_RGBA_INTERPOLATED_ALPHA,
        FORMAT_DXT1,
        FORMAT_REMOVED_DXT_1N,
        FORMAT_DXT1A,
        FORMAT_DXT3,
        FORMAT_DXT5,
        FORMAT_DXT5NM,
        FORMAT_RGBA8888 })
{
}

bool LibDdsHelper::CanProcessFile(const FilePtr& infile) const
{
    DVASSERT(infile);
    DDSHandlerPtr reader = DDSHandler::CreateForFile(infile);
    infile->Seek(0, File::SEEK_FROM_START);
    return (reader.get() != nullptr);
}

eErrorCode LibDdsHelper::ReadFile(const FilePtr& infile, Vector<Image*>& imageSet, uint32 baseMipMap) const
{
    DVASSERT(infile);

    DDSHandlerPtr reader = DDSHandler::CreateForFile(infile);
    if (reader && reader->GetTextures(imageSet, baseMipMap))
    {
        return eErrorCode::SUCCESS;
    }
    else
    {
        return eErrorCode::ERROR_FILE_FORMAT_INCORRECT;
    }
}

eErrorCode LibDdsHelper::WriteFile(const FilePath& outFileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const
{
    if (!outFileName.IsEqualToExtension(".dds"))
    {
        Logger::Error("[LibDdsHelper::WriteFile] Wrong output file name specified: '%s'", outFileName.GetAbsolutePathname().c_str());
        return eErrorCode::ERROR_FILE_FORMAT_INCORRECT;
    }

    bool writeOk = false;

    if (compressionFormat == FORMAT_ATC_RGB ||
        compressionFormat == FORMAT_ATC_RGBA_EXPLICIT_ALPHA ||
        compressionFormat == FORMAT_ATC_RGBA_INTERPOLATED_ALPHA)
    {
        writeOk = QualcommHelper::WriteAtcFile(outFileName, imageSet, compressionFormat);
    }
    else
    {
        Vector<Vector<Image*>> imageSets;
        imageSets.push_back(imageSet);
        writeOk = NvttHelper::WriteDxtFile(outFileName, compressionFormat, imageSets);
    }

    if (writeOk)
    {
        ScopedPtr<File> ddsFile(File::Create(outFileName, File::OPEN | File::READ | File::WRITE));

        DDSHandlerPtr ddsHandler = DDSHandler::CreateForFile(ddsFile);
        if (ddsHandler && ddsHandler->WritePixelFormat(compressionFormat))
        {
            return DAVA::eErrorCode::SUCCESS;
        }
        else
        {
            Logger::Error("[LibDdsHelper::WriteFile] Can't add pixel format to metadata of %s", outFileName.GetStringValue().c_str());
            return eErrorCode::ERROR_WRITE_FAIL;
        }
    }
    else
    {
        return DAVA::eErrorCode::ERROR_WRITE_FAIL;
    }
}

eErrorCode LibDdsHelper::WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const
{
    if (imageSet.size() != Texture::CUBE_FACE_COUNT)
    {
        Logger::Error("[LibDdsHelper::WriteFile] Wrong input image set.");
        return eErrorCode::ERROR_WRITE_FAIL;
    }

    if (!fileName.IsEqualToExtension(".dds"))
    {
        Logger::Error("[LibDdsHelper::WriteFile] Wrong output file name specifed: '%s'", fileName.GetAbsolutePathname().c_str());
        return eErrorCode::ERROR_FILE_FORMAT_INCORRECT;
    }

    bool writeOk = false;

    if (compressionFormat == FORMAT_ATC_RGB ||
        compressionFormat == FORMAT_ATC_RGBA_EXPLICIT_ALPHA ||
        compressionFormat == FORMAT_ATC_RGBA_INTERPOLATED_ALPHA)
    {
        writeOk = QualcommHelper::WriteAtcFileAsCubemap(fileName, imageSet, compressionFormat);
    }
    else
    {
        writeOk = NvttHelper::WriteDxtFile(fileName, compressionFormat, imageSet);
    }

    if (writeOk)
    {
        ScopedPtr<File> ddsFile(File::Create(fileName, File::OPEN | File::READ | File::WRITE));

        DDSHandlerPtr ddsHandler = DDSHandler::CreateForFile(ddsFile);
        if (ddsHandler && ddsHandler->WritePixelFormat(compressionFormat))
        {
            return DAVA::eErrorCode::SUCCESS;
        }
        else
        {
            Logger::Error("[LibDdsHelper::WriteFile] Can't add pixel format to metadata of %s", fileName.GetStringValue().c_str());
            return eErrorCode::ERROR_WRITE_FAIL;
        }
    }
    else
    {
        return DAVA::eErrorCode::ERROR_WRITE_FAIL;
    }
}

DAVA::ImageInfo LibDdsHelper::GetImageInfo(const FilePtr& infile) const
{
    DDSHandlerPtr reader(DDSHandler::CreateForFile(infile));
    if (reader)
    {
        return reader->GetImageInfo();
    }
    else
    {
        return ImageInfo();
    }
}

bool LibDdsHelper::AddCRCIntoMetaData(const FilePath& filePathname) const
{
    ScopedPtr<File> ddsFile(File::Create(filePathname, File::OPEN | File::READ | File::WRITE));
    if (!ddsFile)
    {
        Logger::Error("[LibDdsHelper::AddCRCIntoMetaData] cannot open file %s", filePathname.GetStringValue().c_str());
        return false;
    }

    DDSHandlerPtr ddsHandler(DDSHandler::CreateForFile(ddsFile));
    if (ddsHandler)
    {
        return ddsHandler->WriteCRC();
    }
    else
    {
        Logger::Error("[LibDdsHelper::AddCRCIntoMetaData] is not a DDS file %s", filePathname.GetStringValue().c_str());
        return false;
    }
}

uint32 LibDdsHelper::GetCRCFromFile(const FilePath& filePathname) const
{
    ScopedPtr<File> ddsFile(File::Create(filePathname, File::READ | File::OPEN));
    if (!ddsFile)
    {
        Logger::Error("[LibDdsHelper::GetCRCFromFile] cannot open file %s", filePathname.GetStringValue().c_str());
        return false;
    }

    DDSHandlerPtr reader(DDSHandler::CreateForFile(ddsFile));
    if (reader)
    {
        uint32 crc;
        if (reader->GetCRC(crc))
        {
            return crc;
        }
        else
        {
            reader.reset();
            return CRC32::ForFile(filePathname);
        }
    }
    else
    {
        Logger::Error("[LibDdsHelper::GetCRCFromFile] is not a DDS file %s", filePathname.GetStringValue().c_str());
        return false;
    }
}

ImagePtr LibDdsHelper::DecompressToRGBA(const Image* image)
{
    DVASSERT(image);

    switch (image->format)
    {
    case FORMAT_DXT1:
    case FORMAT_DXT1A:
    case FORMAT_DXT3:
    case FORMAT_DXT5:
    case FORMAT_DXT5NM:
    {
        return NvttHelper::DecompressDxtToRGBA(image);
    }
    case FORMAT_ATC_RGB:
    case FORMAT_ATC_RGBA_EXPLICIT_ALPHA:
    case FORMAT_ATC_RGBA_INTERPOLATED_ALPHA:
    {
        return QualcommHelper::DecompressAtcToRGBA(image);
    }
    }

    return ImagePtr(nullptr);
}
}
