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


#include "Render/Image/Private/PVRFormatHelper.h"
#include "Render/Image/Image.h"
#include "Render/PixelFormatDescriptor.h"

#include "Render/Texture.h"

#include "FileSystem/File.h"

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
#include "libpvr/PVRTError.h"
#include "libpvr/PVRTDecompress.h"
#include "libpvr/PVRTMap.h"
#include "libpvr/PVRTextureHeader.h"
#include "libpvr/PVRTexture.h"
#endif //#if defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_WIN32__)

namespace DAVA
{
PVRFile::~PVRFile()
{
    for (MetaDataBlock* block : metaDatablocks)
    {
        delete block;
    }
    metaDatablocks.clear();
}

namespace PVRFormatHelper
{
const uint32 PVRTEX3_METADATAIDENT = 0x03525650;

const uint32 METADATA_FOURCC_OFFSET = 0;
const uint32 METADATA_KEY_OFFSET = 4;
const uint32 METADATA_DATASIZE_OFFSET = 8;
const uint32 METADATA_DATA_OFFSET = 12;

const uint32 METADATA_CRC_DATA_SIZE = 4; //size for CRC32
const uint32 METADATA_CRC_SIZE = (METADATA_DATA_OFFSET + METADATA_CRC_DATA_SIZE); //size for meta data with CRC32
const uint32 METADATA_CRC_KEY = 0x5f435243; // equivalent of 'C''R''C''_'

const uint32 METADATA_CUBE_KEY = 2;
const uint32 METADATA_CUBE_SIZE = 6;

bool DetectIfNeedSwapBytes(const PVRHeaderV3* header)
{
    if ((PVRTEX_CURR_IDENT != header->u32Version) && (PVRTEX_CURR_IDENT_REV != header->u32Version))
    {
        return (PVRTIsLittleEndian() == false);
    }

    return (PVRTEX_CURR_IDENT_REV == header->u32Version);
}

uint32 GetMipmapDataSize(PixelFormat format, uint32 width, uint32 height)
{
    DVASSERT(width != 0 && height != 0);

    Size2i blockSize = PixelFormatDescriptor::GetPixelFormatBlockSize(format);
    if (blockSize.dx != 1 || blockSize.dy != 1)
    {
        width = width + ((-1 * width) % blockSize.dx);
        height = height + ((-1 * height) % blockSize.dy);
    }

    uint32 bitsPerPixel = PixelFormatDescriptor::GetPixelFormatSizeInBits(format);
    return (bitsPerPixel * width * height / 8);
}

uint32 GetDataSize(const Vector<Image*>& imageSet)
{
    uint32 dataSize = 0;

    for (const Image* image : imageSet)
    {
        dataSize += GetMipmapDataSize(image->format, image->width, image->height);
    }

    return dataSize;
}

uint32 GetDataSize(const Vector<Vector<Image*>>& imageSet)
{
    uint32 dataSize = 0;

    for (const Vector<Image*>& faceImages : imageSet)
    {
        for (const Image* image : faceImages)
        {
            dataSize += GetMipmapDataSize(image->format, image->width, image->height);
        }
    }

    return dataSize;
}

PixelFormat GetDAVAFormatFromPVR(uint64 pixelFormat)
{
    switch (pixelFormat)
    {
    case ePVRTPF_PVRTCI_2bpp_RGB:
        return FORMAT_PVR2;
    case ePVRTPF_PVRTCI_2bpp_RGBA:
        return FORMAT_PVR2;
    case ePVRTPF_PVRTCI_4bpp_RGB:
        return FORMAT_PVR4;
    case ePVRTPF_PVRTCI_4bpp_RGBA:
        return FORMAT_PVR4;
    case ePVRTPF_PVRTCII_2bpp:
        return FORMAT_PVR2_2;
    case ePVRTPF_PVRTCII_4bpp:
        return FORMAT_PVR4_2;
    case ePVRTPF_ETC1:
        return FORMAT_ETC1;
    case ePVRTPF_EAC_R11:
        return FORMAT_EAC_R11_UNSIGNED;
    case ePVRTPF_ETC2_RGB:
        return FORMAT_ETC2_RGB;
    case ePVRTPF_ETC2_RGB_A1:
        return FORMAT_ETC2_RGB_A1;
    case ePVRTPF_EAC_RG11:
        return FORMAT_EAC_RG11_UNSIGNED;
    case ePVRTPF_ETC2_RGBA:
        return FORMAT_ETC2_RGBA;
    case PVRTGENPIXELID4('r', 'g', 'b', 'a', 8, 8, 8, 8):
        return FORMAT_RGBA8888;
    case PVRTGENPIXELID4('r', 'g', 'b', 'a', 5, 5, 5, 1):
        return FORMAT_RGBA5551;
    case PVRTGENPIXELID4('r', 'g', 'b', 'a', 4, 4, 4, 4):
        return FORMAT_RGBA4444;
    case PVRTGENPIXELID3('r', 'g', 'b', 8, 8, 8):
        return FORMAT_RGB888;
    case PVRTGENPIXELID3('r', 'g', 'b', 5, 6, 5):
        return FORMAT_RGB565;
    case PVRTGENPIXELID1('l', 8):
    case PVRTGENPIXELID1('a', 8):
        return FORMAT_A8;
    case PVRTGENPIXELID1('a', 16):
        return FORMAT_A16;
    case PVRTGENPIXELID4('r', 'g', 'b', 'a', 16, 16, 16, 16):
        return FORMAT_RGBA16161616;
    case PVRTGENPIXELID4('r', 'g', 'b', 'a', 32, 32, 32, 32):
        return FORMAT_RGBA32323232;
    default:
        break;
    }

    return FORMAT_INVALID;
}

uint64 GetPVRFormatFromDAVA(PixelFormat pixelFormat)
{
    switch (pixelFormat)
    {
    case FORMAT_PVR2:
        return ePVRTPF_PVRTCI_2bpp_RGBA; //ePVRTPF_PVRTCI_2bpp_RGB;
    case FORMAT_PVR4:
        return ePVRTPF_PVRTCI_4bpp_RGBA; //ePVRTPF_PVRTCI_4bpp_RGB
    case FORMAT_PVR2_2:
        return ePVRTPF_PVRTCII_2bpp;
    case FORMAT_PVR4_2:
        return ePVRTPF_PVRTCII_4bpp;
    case FORMAT_ETC1:
        return ePVRTPF_ETC1;
    case FORMAT_EAC_R11_UNSIGNED:
        return ePVRTPF_EAC_R11;
    case FORMAT_ETC2_RGB:
        return ePVRTPF_ETC2_RGB;
    case FORMAT_ETC2_RGB_A1:
        return ePVRTPF_ETC2_RGB_A1;
    case FORMAT_EAC_RG11_UNSIGNED:
        return ePVRTPF_EAC_RG11;
    case FORMAT_ETC2_RGBA:
        return ePVRTPF_ETC2_RGBA;
    case FORMAT_RGBA8888:
        return PVRTGENPIXELID4('r', 'g', 'b', 'a', 8, 8, 8, 8);
    case FORMAT_RGBA5551:
        return PVRTGENPIXELID4('r', 'g', 'b', 'a', 5, 5, 5, 1);
    case FORMAT_RGBA4444:
        return PVRTGENPIXELID4('r', 'g', 'b', 'a', 4, 4, 4, 4);
    case FORMAT_RGB888:
        return PVRTGENPIXELID3('r', 'g', 'b', 8, 8, 8);
    case FORMAT_RGB565:
        return PVRTGENPIXELID3('r', 'g', 'b', 5, 6, 5);
    case FORMAT_A8:
        return PVRTGENPIXELID1('a', 8);
    case FORMAT_A16:
        return PVRTGENPIXELID1('a', 16);
    case FORMAT_RGBA16161616:
        return PVRTGENPIXELID4('r', 'g', 'b', 'a', 16, 16, 16, 16);
    case FORMAT_RGBA32323232:
        return PVRTGENPIXELID4('r', 'g', 'b', 'a', 32, 32, 32, 32);

    default:
        DVASSERT(false);
        break;
    }

    return ePVRTPF_NumCompressedPFs;
}

uint32 GetPVRChannelType(PixelFormat pixelFormat)
{
    switch (pixelFormat)
    {
    case FORMAT_PVR2:
    case FORMAT_PVR4:
    case FORMAT_PVR2_2:
    case FORMAT_PVR4_2:
    case FORMAT_ETC1:
    case FORMAT_EAC_R11_UNSIGNED:
    case FORMAT_ETC2_RGB:
    case FORMAT_ETC2_RGB_A1:
    case FORMAT_EAC_RG11_UNSIGNED:
    case FORMAT_ETC2_RGBA:
        return ePVRTVarTypeUnsignedByteNorm;

    case FORMAT_RGBA8888:
    case FORMAT_RGBA5551:
    case FORMAT_RGBA4444:
    case FORMAT_RGB888:
    case FORMAT_RGB565:
    case FORMAT_A8:
        return ePVRTVarTypeUnsignedByteNorm;

    case FORMAT_A16:
    case FORMAT_RGBA16161616:
    case FORMAT_RGBA32323232:
        return ePVRTVarTypeFloat;

    default:
        DVASSERT(false);
        break;
    }

    return ePVRTVarTypeUnsignedByteNorm;
}

PixelFormat GetTextureFormat(const PVRHeaderV3& textureHeader)
{
    return GetDAVAFormatFromPVR(textureHeader.u64PixelFormat);
}

const MetaDataBlock* GetCubemapMetadata(const PVRFile& pvrFile)
{
    for (MetaDataBlock* block : pvrFile.metaDatablocks)
    {
        if (block->DevFOURCC == PVRTEX3_METADATAIDENT &&
            block->u32Key == METADATA_CUBE_KEY &&
            block->u32DataSize == METADATA_CUBE_SIZE)
        {
            return block;
        }
    }

    return nullptr;
}

void AddMetaData(PVRFile& pvrFile, MetaDataBlock* block)
{
    pvrFile.header.u32MetaDataSize += (block->u32DataSize + METADATA_DATA_OFFSET);
    pvrFile.metaDatablocks.push_back(block);
}

bool ReadMetaData(File* file, PVRFile* pvrFile)
{
    uint32 metaDataSize = pvrFile->header.u32MetaDataSize;
    while (metaDataSize != 0)
    {
        MetaDataBlock *block = new MetaDataBlock();

        uint32 readSize = file->Read(&block->DevFOURCC);
        if (readSize != sizeof(uint32))
        {
            return false;
        }

        readSize = file->Read(&block->u32Key);
        if (readSize != sizeof(uint32))
        {
            return false;
        }

        readSize = file->Read(&block->u32DataSize);
        if (readSize != sizeof(uint32))
        {
            return false;
        }

        if (block->u32DataSize != 0)
        {
            block->Data = new uint8[block->u32DataSize];
            readSize = file->Read(block->Data, block->u32DataSize);
            if (readSize != block->u32DataSize)
            {
                return false;
            }
        }

        DVASSERT(metaDataSize >= block->u32DataSize + METADATA_DATA_OFFSET);
        metaDataSize -= (block->u32DataSize + METADATA_DATA_OFFSET);
        pvrFile->metaDatablocks.push_back(block);
    }

    return true;
}

uint32 GetCubemapLayout(const PVRFile& pvrFile)
{
    uint32 layout = 0;

    const MetaDataBlock* cubeMetaData = GetCubemapMetadata(pvrFile);
    if (cubeMetaData)
    {
        for (uint32 index = 0; index < cubeMetaData->u32DataSize; ++index)
        {
            switch (cubeMetaData->Data[index])
            {
            case 'X':
            {
                layout = layout | (rhi::TEXTURE_FACE_POSITIVE_X << (index * 4));
                break;
            }

            case 'x':
            {
                layout = layout | (rhi::TEXTURE_FACE_NEGATIVE_X << (index * 4));
                break;
            }

            case 'Y':
            {
                layout = layout | (rhi::TEXTURE_FACE_POSITIVE_Y << (index * 4));
                break;
            }

            case 'y':
            {
                layout = layout | (rhi::TEXTURE_FACE_NEGATIVE_Y << (index * 4));
                break;
            }

            case 'Z':
            {
                layout = layout | (rhi::TEXTURE_FACE_POSITIVE_Z << (index * 4));
                break;
            }

            case 'z':
            {
                layout = layout | (rhi::TEXTURE_FACE_NEGATIVE_Z << (index * 4));
                break;
            }
            }
        }
    }
    else if (pvrFile.header.u32NumFaces > 1)
    {
        static uint32 faces[] = {
            rhi::TEXTURE_FACE_POSITIVE_X,
            rhi::TEXTURE_FACE_NEGATIVE_X,
            rhi::TEXTURE_FACE_POSITIVE_Y,
            rhi::TEXTURE_FACE_NEGATIVE_Y,
            rhi::TEXTURE_FACE_POSITIVE_Z,
            rhi::TEXTURE_FACE_NEGATIVE_Z
        };
        for (uint32 i = 0; i < pvrFile.header.u32NumFaces; ++i)
        {
            layout = layout | (faces[i] << (i * 4));
        }
    }

    return layout;
}

//external methods

std::unique_ptr<PVRFile> ReadFile(const FilePath& pathname, bool readMetaData, bool readData)
{
    ScopedPtr<File> file(File::Create(pathname, File::OPEN | File::READ));
    if (file)
    {
        return ReadFile(file, readMetaData, readData);
    }

    return std::unique_ptr<PVRFile>();
}

std::unique_ptr<PVRFile> ReadFile(File* file, bool readMetaData /*= false*/, bool readData /*= false*/)
{
    DVASSERT(file != nullptr);
    DVASSERT(file->GetPos() == 0);

    std::unique_ptr<PVRFile> pvrFile(new PVRFile());

    uint32 readSize = file->Read(&pvrFile->header, PVRFile::HEADER_SIZE);
    if (readSize != PVRFile::HEADER_SIZE)
    {
        Logger::Error("Can't read PVR header from %s", file->GetFilename().GetStringValue().c_str());
        return std::unique_ptr<PVRFile>();
    }

    if (DetectIfNeedSwapBytes(&pvrFile->header))
    {
        Logger::Error("Can't load PVR with swapped bytes %s", file->GetFilename().GetStringValue().c_str());
        return std::unique_ptr<PVRFile>();
    }

    if (readMetaData && pvrFile->header.u32MetaDataSize)
    {
        bool read = ReadMetaData(file, pvrFile.get());
        if (!read)
        {
            Logger::Error("Can't read metadata from PVR header from %s", file->GetFilename().GetStringValue().c_str());
            return std::unique_ptr<PVRFile>();
        }
    }
    else if (pvrFile->header.u32MetaDataSize)
    {
        file->Seek(pvrFile->header.u32MetaDataSize, File::SEEK_FROM_CURRENT);
    }

    if (readData)
    {
        uint32 compressedDataSize = file->GetSize() - (PVRFile::HEADER_SIZE + pvrFile->header.u32MetaDataSize);
        pvrFile->compressedData.resize(compressedDataSize);
        readSize = file->Read(pvrFile->compressedData.data(), compressedDataSize);
        if (readSize != compressedDataSize)
        {
            Logger::Error("Can't read PVR data from %s", file->GetFilename().GetStringValue().c_str());
            return std::unique_ptr<PVRFile>();
        }
    }

    return pvrFile;
}

std::unique_ptr<PVRFile> GenerateHeader(const Vector<Image*>& imageSet)
{
    Image* zeroMip = imageSet[0];

    std::unique_ptr<PVRFile> pvrFile(new PVRFile());

    pvrFile->header.u64PixelFormat = GetPVRFormatFromDAVA(zeroMip->format);
    pvrFile->header.u32ChannelType = GetPVRChannelType(zeroMip->format);

    pvrFile->header.u32Width = zeroMip->width;
    pvrFile->header.u32Height = zeroMip->height;

    pvrFile->header.u32NumFaces = 1;

    DVASSERT(zeroMip->cubeFaceID == Texture::INVALID_CUBEMAP_FACE);

    pvrFile->header.u32MIPMapCount = static_cast<uint32>(imageSet.size());

    uint32 compressedDataSize = GetDataSize(imageSet);
    pvrFile->compressedData.resize(compressedDataSize);

    uint8* compressedDataPtr = pvrFile->compressedData.data();
    for (const Image* image : imageSet)
    {
        Memcpy(compressedDataPtr, image->data, image->dataSize);
        compressedDataPtr += GetMipmapDataSize(image->format, image->width, image->height);
    }

    return pvrFile;
}

std::unique_ptr<PVRFile> GenerateCubeHeader(const Vector<Vector<Image*>>& imageSet)
{
    const Vector<Image*>& zeroFaceImageSet = imageSet[0];
    Image* zeroMip = zeroFaceImageSet[0];

    std::unique_ptr<PVRFile> pvrFile(new PVRFile());

    pvrFile->header.u64PixelFormat = GetPVRFormatFromDAVA(zeroMip->format);
    pvrFile->header.u32ChannelType = GetPVRChannelType(zeroMip->format);

    pvrFile->header.u32Width = zeroMip->width;
    pvrFile->header.u32Height = zeroMip->height;

    pvrFile->header.u32NumFaces = static_cast<uint32>(imageSet.size());
    pvrFile->header.u32MIPMapCount = static_cast<uint32>(zeroFaceImageSet.size());

    uint32 compressedDataSize = GetDataSize(imageSet);
    pvrFile->compressedData.resize(compressedDataSize);
    uint8* compressedDataPtr = pvrFile->compressedData.data();
    for (uint32 mip = 0; mip < pvrFile->header.u32MIPMapCount; ++mip)
    {
        for (const Vector<Image*>& faceImageSet : imageSet)
        {
            Image* image = faceImageSet[mip];
            Memcpy(compressedDataPtr, image->data, image->dataSize);
            compressedDataPtr += GetMipmapDataSize(image->format, image->width, image->height);
        }
    }

    //    if(pvrFile->header.u32NumFaces > 1)
    {
        MetaDataBlock* cubeMetaBlock = new MetaDataBlock();
        cubeMetaBlock->DevFOURCC = PVRTEX3_METADATAIDENT;
        cubeMetaBlock->u32Key = METADATA_CUBE_KEY;
        cubeMetaBlock->u32DataSize = METADATA_CUBE_SIZE;
        cubeMetaBlock->Data = new uint8[METADATA_CUBE_SIZE];
        Memcpy(cubeMetaBlock->Data, "XxYyZz", METADATA_CUBE_SIZE);
        AddMetaData(*pvrFile, cubeMetaBlock);
    }

    return pvrFile;
}

bool LoadImages(File* infile, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams)
{
    std::unique_ptr<PVRFile> pvrFile = PVRFormatHelper::ReadFile(infile, true, false);
    if (!pvrFile)
    {
        return false;
    }

    DVASSERT(pvrFile->header.u32NumSurfaces == 1);

    PixelFormat pxFormat = GetTextureFormat(pvrFile->header);
    if (pvrFile->header.u32Height != pvrFile->header.u32Width && (pxFormat == FORMAT_PVR2 || pxFormat == FORMAT_PVR4))
    {
        Logger::Error("Non-square textures with %s compression are unsupported. Failed to load : %s", GlobalEnumMap<PixelFormat>::Instance()->ToString(pxFormat),
                      infile->GetFilename().GetStringValue().c_str());
        return false;
    }

    if (pvrFile->header.u32MIPMapCount == 0)
    {
        Logger::Error("File %s has no mipmaps", infile->GetFilename().GetStringValue().c_str());
        return false;
    }

    uint32 fromMipMap = Min(loadingParams.baseMipmap, pvrFile->header.u32MIPMapCount - 1);

    uint32 cubemapLayout = GetCubemapLayout(*pvrFile);
    for (uint32 mip = 0; mip < pvrFile->header.u32MIPMapCount; ++mip)
    {
        for (uint32 surface = 0; surface < pvrFile->header.u32NumSurfaces; ++surface)
        {
            for (uint32 face = 0; face < pvrFile->header.u32NumFaces; ++face)
            {
                uint32 mipWidth = pvrFile->header.u32Width >> mip;
                uint32 mipHeight = pvrFile->header.u32Height >> mip;
                uint32 mipDataSize = GetMipmapDataSize(pxFormat, mipWidth, mipHeight);

                if (mip < fromMipMap)
                {
                    infile->Seek(mipDataSize, File::SEEK_FROM_CURRENT);
                }
                else
                {
                    Image* image = new Image();
                    image->width = mipWidth;
                    image->height = mipHeight;
                    image->format = pxFormat;
                    image->mipmapLevel = (mip - fromMipMap) + loadingParams.firstMipmapIndex;
                    if (cubemapLayout != 0)
                    {
                        image->cubeFaceID = (cubemapLayout & (0x0000000F << (face * 4))) >> (face * 4);
                    }

                    image->dataSize = mipDataSize;
                    image->data = new uint8[image->dataSize];
                    uint32 dz = infile->Read(image->data, image->dataSize);
                    if (dz != image->dataSize)
                    {
                        image->Release();
                        Logger::Error("Cannot read mip %d, fase %d from file", mip, face, infile->GetFilename().GetStringValue().c_str());
                        return false;
                    }

                    imageSet.push_back(image);
                }
            }
        }
    }

    return true;
}

Image* DecodeToRGBA8888(Image* encodedImage)
{
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
    Image* decodedImage = Image::Create(encodedImage->width, encodedImage->height, PixelFormat::FORMAT_RGBA8888);
    decodedImage->mipmapLevel = encodedImage->mipmapLevel;
    decodedImage->cubeFaceID = encodedImage->cubeFaceID;

    if (encodedImage->format == PixelFormat::FORMAT_PVR2)
    {
        int retCode = PVRTDecompressPVRTC(encodedImage->data, 1, encodedImage->width, encodedImage->height, decodedImage->data);
        DVVERIFY(retCode == encodedImage->dataSize);
    }
    else if (encodedImage->format == PixelFormat::FORMAT_PVR4)
    {
        int retCode = PVRTDecompressPVRTC(encodedImage->data, 0, encodedImage->width, encodedImage->height, decodedImage->data);
        DVVERIFY(retCode == encodedImage->dataSize);
    }
    else if (encodedImage->format == PixelFormat::FORMAT_ETC1)
    {
        int retCode = PVRTDecompressETC(encodedImage->data, encodedImage->width, encodedImage->height, decodedImage->data, 0);
        DVVERIFY(retCode == encodedImage->dataSize);
    }
    else
    {
        Logger::Error("Can't decode PVR: source Image has unknown format %s", GlobalEnumMap<PixelFormat>::Instance()->ToString(encodedImage->format));
        SafeRelease(decodedImage);
    }

    return decodedImage;
#else //#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
    return nullptr;
#endif //#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
}

bool WriteFile(const FilePath& pathname, const PVRFile& pvrFile)
{
    ScopedPtr<File> file(File::Create(pathname, File::CREATE | File::WRITE));
    if (file)
    {
        file->Write(&pvrFile.header);

        for (MetaDataBlock *block : pvrFile.metaDatablocks)
        {
            file->Write(&block->DevFOURCC);
            file->Write(&block->u32Key);
            file->Write(&block->u32DataSize);
            if (block->u32DataSize != 0)
            {
                file->Write(block->Data, block->u32DataSize);
            }
        }
        
        if (!pvrFile.compressedData.empty())
        {
            file->Write(pvrFile.compressedData.data(), static_cast<uint32>(pvrFile.compressedData.size()));
        }
        return true;
    }

    Logger::Error("Сan't open file: %s", pathname.GetStringValue().c_str());
    return false;
}

bool GetCRCFromMetaData(const PVRFile& pvrFile, uint32* outputCRC)
{
    for (MetaDataBlock* block : pvrFile.metaDatablocks)
    {
        if (block->u32Key == METADATA_CRC_KEY)
        {
            *outputCRC = *(reinterpret_cast<uint32*>(block->Data));
            return true;
        }
    }

    return false;
}

void AddCRCToMetaData(PVRFile& pvrFile, uint32 crc)
{
    //reallocate meta data buffer
    MetaDataBlock* crcMetaData = new MetaDataBlock();
    crcMetaData->DevFOURCC = PVRTEX3_METADATAIDENT;
    crcMetaData->u32Key = METADATA_CRC_KEY;
    crcMetaData->u32DataSize = METADATA_CRC_DATA_SIZE;
    crcMetaData->Data = new uint8[sizeof(uint32)];
    *(reinterpret_cast<uint32*>(crcMetaData->Data)) = crc;

    AddMetaData(pvrFile, crcMetaData);
}

} // PVRFormatHelper
} //DAVA
