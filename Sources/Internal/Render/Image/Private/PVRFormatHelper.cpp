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

#include "FileSystem/File.h"

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
#include "libpvr/PVRTError.h"
#include "libpvr/PVRTDecompress.h"
#include "libpvr/PVRTMap.h"
#include "libpvr/PVRTextureHeader.h"
#include "libpvr/PVRTexture.h"
#endif //#if defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_WIN32__)

#define METADATA_FOURCC_OFFSET 0
#define METADATA_KEY_OFFSET 4
#define METADATA_DATASIZE_OFFSET 8
#define METADATA_DATA_OFFSET 12

#define METADATA_CRC_DATA_SIZE 4 //size for CRC32
#define METADATA_CRC_SIZE (METADATA_DATA_OFFSET + METADATA_CRC_DATA_SIZE) //size for meta data with CRC32
#define METADATA_CRC_KEY 0x5f435243 // equivalent of 'C''R''C''_'

#define METADATA_CUBE_KEY 2
#define METADATA_CUBE_SIZE 6

namespace DAVA
{
PVRFile::~PVRFile()
{
    for (MetaDataBlock* block : metaDatablocks)
    {
        block->Data = nullptr;
        delete block;
    }
    metaDatablocks.clear();

    SafeDeleteArray(metaData);
    SafeDeleteArray(compressedData);
    compressedDataSize = 0;
}

namespace PVRFormatHelper
{
const uint32 PVRTEX3_METADATAIDENT = 0x03525650;

bool DetectIfNeedSwapBytes(const PVRHeaderV3* header)
{
    if ((PVRTEX_CURR_IDENT != header->u32Version) && (PVRTEX_CURR_IDENT_REV != header->u32Version))
    {
        return (PVRTIsLittleEndian() == false);
    }

    return (PVRTEX_CURR_IDENT_REV == header->u32Version);
}

void ReadMetaData(File* file, PVRFile* pvrFile)
{
    uint32 metaDataSize = pvrFile->header.u32MetaDataSize;

    DVASSERT(pvrFile->metaData == nullptr);
    pvrFile->metaData = new uint8[metaDataSize];
    uint32 readSize = file->Read(pvrFile->metaData, metaDataSize);
    if (readSize != metaDataSize)
    {
        SafeDeleteArray(pvrFile->metaData);
        return;
    }

    uint8* metaDataPtr = pvrFile->metaData;
    uint32 delta = static_cast<uint32>(metaDataPtr - pvrFile->metaData);
    while (delta < metaDataSize)
    {
        MetaDataBlock* block = new MetaDataBlock();
        block->DevFOURCC = *reinterpret_cast<uint32*>(metaDataPtr);
        metaDataPtr += sizeof(uint32);

        block->u32Key = *reinterpret_cast<uint32*>(metaDataPtr);
        metaDataPtr += sizeof(uint32);

        block->u32DataSize = *reinterpret_cast<uint32*>(metaDataPtr);
        metaDataPtr += sizeof(uint32);

        block->Data = metaDataPtr;
        metaDataPtr += block->u32DataSize;

        pvrFile->metaDatablocks.push_back(block);

        delta = static_cast<uint32>(metaDataPtr - pvrFile->metaData);
    }
}

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

    uint32 readSize = file->Read(&pvrFile->header, PVRTEX3_HEADERSIZE);
    if (readSize != PVRTEX3_HEADERSIZE)
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
        ReadMetaData(file, pvrFile.get());
    }
    else if (pvrFile->header.u32MetaDataSize)
    {
        file->Seek(pvrFile->header.u32MetaDataSize, File::SEEK_FROM_CURRENT);
    }

    pvrFile->compressedDataSize = file->GetSize() - (PVRTEX3_HEADERSIZE + pvrFile->header.u32MetaDataSize);
    if (readData)
    {
        pvrFile->compressedData = new uint8[pvrFile->compressedDataSize];
        readSize = file->Read(pvrFile->compressedData, pvrFile->compressedDataSize);
        if (readSize != pvrFile->compressedDataSize)
        {
            Logger::Error("Can't read PVR data from %s", file->GetFilename().GetStringValue().c_str());
            return std::unique_ptr<PVRFile>();
        }
    }

    return pvrFile;
}

bool LoadImages(File* infile, Vector<Image*>& imageSet, int32 fromMipMap, int32 firstMipmapIndex)
{
    std::unique_ptr<PVRFile> pvrFile = PVRFormatHelper::ReadFile(infile, false, false);
    if (!pvrFile)
    {
        return false;
    }

    PixelFormat pxFormat = GetTextureFormat(pvrFile->header);
    if (pvrFile->header.u32Height != pvrFile->header.u32Width && (pxFormat == FORMAT_PVR2 || pxFormat == FORMAT_PVR4))
    {
        Logger::Error("Non-square textures with %s compression are unsupported. Failed to load : %s", GlobalEnumMap<PixelFormat>::Instance()->ToString(pxFormat),
                      infile->GetFilename().GetStringValue().c_str());
        return false;
    }

    // we don't need metadata for loading?
    if (pvrFile->header.u32MetaDataSize)
    {
        infile->Seek(pvrFile->header.u32MetaDataSize, File::SEEK_FROM_CURRENT);
    }

    uint32 mipmapLevelCount = pvrFile->header.u32MIPMapCount;
    fromMipMap = Min(fromMipMap, int32(mipmapLevelCount - 1));

    //    bool loadAllPvrData = true;
    //    for (uint32 i = fromMipMap; i < mipmapLevelCount; ++i)
    //    {
    //        loadAllPvrData &= LoadMipMapLevel(pvrFile, i, (i - fromMipMap) + firstMipmapIndex, imageSet);
    //    }
    //
    //    return loadAllPvrData;

    DVASSERT(false);

    return false;
}

bool WriteFile(const FilePath& pathname, const PVRFile& pvrFile)
{
    ScopedPtr<File> file(File::Create(pathname, File::CREATE | File::WRITE));
    if (file)
    {
        file->Write(&pvrFile.header);
        file->Write(pvrFile.metaData, pvrFile.header.u32MetaDataSize);
        if (pvrFile.compressedData != nullptr)
        {
            file->Write(pvrFile.compressedData, pvrFile.compressedDataSize);
        }
        return true;
    }

    Logger::Error("Сanэt open file: %s", pathname.GetStringValue().c_str());
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
    uint8* oldMetaData = pvrFile.metaData;
    uint32 oldMetaDataSize = pvrFile.header.u32MetaDataSize;

    pvrFile.header.u32MetaDataSize = oldMetaDataSize + METADATA_CRC_SIZE;
    pvrFile.metaData = new uint8[pvrFile.header.u32MetaDataSize];
    if (oldMetaData != nullptr)
    {
        Memcpy(pvrFile.metaData, oldMetaData, oldMetaDataSize);
        delete[] oldMetaData;
    }

    //create metaDataWithCrc
    MetaDataBlock* crcMetaData = new MetaDataBlock();
    crcMetaData->DevFOURCC = PVRTEX3_METADATAIDENT;
    crcMetaData->u32Key = METADATA_CRC_KEY;
    crcMetaData->u32DataSize = METADATA_CRC_DATA_SIZE;
    crcMetaData->Data = pvrFile.metaData + oldMetaDataSize + METADATA_DATA_OFFSET;
    pvrFile.metaDatablocks.push_back(crcMetaData);

    *(reinterpret_cast<uint32*>(pvrFile.metaData + oldMetaDataSize + METADATA_FOURCC_OFFSET)) = crcMetaData->DevFOURCC;
    *(reinterpret_cast<uint32*>(pvrFile.metaData + oldMetaDataSize + METADATA_KEY_OFFSET)) = crcMetaData->u32Key;
    *(reinterpret_cast<uint32*>(pvrFile.metaData + oldMetaDataSize + METADATA_DATASIZE_OFFSET)) = crcMetaData->u32DataSize;
    *(reinterpret_cast<uint32*>(pvrFile.metaData + oldMetaDataSize + METADATA_DATA_OFFSET)) = crc;
}

PixelFormat GetCompressedFormat(const uint64 pixelFormat)
{
    switch (pixelFormat)
    {
    case ePVRTPF_PVRTCI_2bpp_RGB:
    {
        return FORMAT_PVR2;
    }
    case ePVRTPF_PVRTCI_2bpp_RGBA:
    {
        return FORMAT_PVR2;
    }
    case ePVRTPF_PVRTCI_4bpp_RGB:
    {
        return FORMAT_PVR4;
    }
    case ePVRTPF_PVRTCI_4bpp_RGBA:
    {
        return FORMAT_PVR4;
    }
    case ePVRTPF_PVRTCII_2bpp:
    {
        return FORMAT_PVR2_2;
    }
    case ePVRTPF_PVRTCII_4bpp:
    {
        return FORMAT_PVR4_2;
    }
    case ePVRTPF_ETC1:
    {
        return FORMAT_ETC1;
    }
    case ePVRTPF_EAC_R11:
    {
        return FORMAT_EAC_R11_UNSIGNED;
    }
    case ePVRTPF_ETC2_RGB:
    {
        return FORMAT_ETC2_RGB;
    }
    case ePVRTPF_ETC2_RGB_A1:
    {
        return FORMAT_ETC2_RGB_A1;
    }
    case ePVRTPF_EAC_RG11:
    {
        return FORMAT_EAC_RG11_UNSIGNED;
    }
    case ePVRTPF_ETC2_RGBA:
    {
        return FORMAT_ETC2_RGBA;
    }

    default:
        break;
    }

    return FORMAT_INVALID;
}

PixelFormat GetFloatTypeFormat(const uint64 pixelFormat)
{
    switch (pixelFormat)
    {
    case PVRTGENPIXELID4('r', 'g', 'b', 'a', 16, 16, 16, 16):
    {
        return FORMAT_RGBA16161616;
    }
    case PVRTGENPIXELID3('r', 'g', 'b', 16, 16, 16):
    {
        return FORMAT_INVALID;
    }
    case PVRTGENPIXELID2('l', 'a', 16, 16):
    {
        return FORMAT_INVALID;
    }
    case PVRTGENPIXELID1('l', 16):
    {
        return FORMAT_INVALID;
    }
    case PVRTGENPIXELID1('a', 16):
    {
        return FORMAT_A16;
    }
    case PVRTGENPIXELID4('r', 'g', 'b', 'a', 32, 32, 32, 32):
    {
        return FORMAT_RGBA32323232;
    }
    case PVRTGENPIXELID3('r', 'g', 'b', 32, 32, 32):
    {
        return FORMAT_INVALID;
    }
    case PVRTGENPIXELID2('l', 'a', 32, 32):
    {
        return FORMAT_INVALID;
    }
    case PVRTGENPIXELID1('l', 32):
    {
        return FORMAT_INVALID;
    }
    case PVRTGENPIXELID1('a', 32):
    {
        return FORMAT_INVALID;
    }

    default:
        break;
    }

    return FORMAT_INVALID;
}

PixelFormat GetUnsignedByteFormat(const uint64 pixelFormat)
{
    switch (pixelFormat)
    {
    case PVRTGENPIXELID4('r', 'g', 'b', 'a', 8, 8, 8, 8):
    {
        return FORMAT_RGBA8888;
    }
    case PVRTGENPIXELID3('r', 'g', 'b', 8, 8, 8):
    {
        return FORMAT_RGB888;
    }
    case PVRTGENPIXELID2('l', 'a', 8, 8):
    {
        return FORMAT_INVALID;
    }
    case PVRTGENPIXELID1('l', 8):
    {
        return FORMAT_A8;
    }
    case PVRTGENPIXELID1('a', 8):
    {
        return FORMAT_A8;
    }
    case PVRTGENPIXELID4('b', 'g', 'r', 'a', 8, 8, 8, 8):
    {
        return FORMAT_INVALID;
    }
    case PVRTGENPIXELID4('r', 'g', 'b', 'a', 4, 4, 4, 4):
    {
        return FORMAT_RGBA4444;
    }
    case PVRTGENPIXELID4('r', 'g', 'b', 'a', 5, 5, 5, 1):
    {
        return FORMAT_RGBA5551;
    }
    case PVRTGENPIXELID3('r', 'g', 'b', 5, 6, 5):
    {
        return FORMAT_RGB565;
    }

    default:
        break;
    }

    return FORMAT_INVALID;
}

PixelFormat GetUnsignedShortFormat(const uint64 pixelFormat)
{
    switch (pixelFormat)
    {
    case PVRTGENPIXELID4('r', 'g', 'b', 'a', 4, 4, 4, 4):
    {
        return FORMAT_RGBA4444;
    }
    case PVRTGENPIXELID4('r', 'g', 'b', 'a', 5, 5, 5, 1):
    {
        return FORMAT_RGBA5551;
    }
    case PVRTGENPIXELID3('r', 'g', 'b', 5, 6, 5):
    {
        return FORMAT_RGB565;
    }

    default:
        break;
    }

    return FORMAT_INVALID;
}

PixelFormat GetTextureFormat(const PVRHeaderV3& textureHeader)
{
    uint64 pixelFormat = textureHeader.u64PixelFormat;

    //Get the last 32 bits of the pixel format.
    uint64 pixelFormatPartHigh = pixelFormat & PVRTEX_PFHIGHMASK;

    //Check for a compressed format (The first 8 bytes will be 0, so the whole thing will be equal to the last 32 bits).
    if (pixelFormatPartHigh == 0)
    {
        //Format and type == 0 for compressed textures.
        return GetCompressedFormat(pixelFormat);
    }
    else
    {
        EPVRTVariableType channelType = EPVRTVariableType(textureHeader.u32ChannelType);
        switch (channelType)
        {
        case ePVRTVarTypeFloat:
        {
            return GetFloatTypeFormat(pixelFormat);
        }
        case ePVRTVarTypeUnsignedByteNorm:
        {
            return GetUnsignedByteFormat(pixelFormat);
        }
        case ePVRTVarTypeUnsignedShortNorm:
        {
            return GetUnsignedShortFormat(pixelFormat);
        }
        default:
            break;
        }
    }

    return FORMAT_INVALID;
}

} // PVRFormatHelper
} //DAVA
