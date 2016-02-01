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


#include "Base/Platform.h"

#include "Render/Texture.h"
#include "FileSystem/Logger.h"
#include "FileSystem/FileSystem.h"
#include "Utils/Utils.h"
#include "Utils/CRC32.h"

#include "Render/Image/Image.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Image/LibPVRHelper.h"
#include "Render/Image/ImageConvert.h"
#include "Render/PixelFormatDescriptor.h"


#if defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_WIN32__)
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

#define METADATA_CRC_DATA_SIZE 4  //size for CRC32
#define METADATA_CRC_SIZE (METADATA_DATA_OFFSET + METADATA_CRC_DATA_SIZE)  //size for meta data with CRC32
#define METADATA_CRC_KEY 0x5f435243  // equivalent of 'C''R''C''_'

#define METADATA_CUBE_KEY 2
#define METADATA_CUBE_SIZE 6


namespace DAVA
{
namespace PVRHelper
{
bool IsCompressedFormat(PixelFormat format)
{
    switch (format)
    {
    case FORMAT_PVR4:
    case FORMAT_PVR2:
    case FORMAT_ETC1:
        return true;

    case FORMAT_PVR2_2:
    case FORMAT_PVR4_2:
    case FORMAT_EAC_R11_UNSIGNED:
    case FORMAT_EAC_R11_SIGNED:
    case FORMAT_EAC_RG11_UNSIGNED:
    case FORMAT_EAC_RG11_SIGNED:
    case FORMAT_ETC2_RGB:
    case FORMAT_ETC2_RGBA:
    case FORMAT_ETC2_RGB_A1:
        return true;

    default:
        return false;
    }

    return false;
}
}

PVRFile::PVRFile()
    : metaData(nullptr)
    , compressedDataSize(0)
    , compressedData(nullptr)
{
};

PVRFile::~PVRFile()
{
    uint32 count = (uint32)metaDatablocks.size();
    for(uint32 i = 0; i < count; ++i)
    {
        metaDatablocks[i]->Data = nullptr; //it is stored at this->metaData
        delete metaDatablocks[i];
    }
    metaDatablocks.clear();
        
    SafeDeleteArray(metaData);
    SafeDeleteArray(compressedData);
    compressedDataSize = 0;
}


const uint32 PVRTEX3_METADATAIDENT	= 0x03525650;

LibPVRHelper::LibPVRHelper()
{
    name.assign("PVR");
    supportedExtensions.push_back(".pvr");
}

bool LibPVRHelper::CanProcessFile(DAVA::File* file) const
{
    bool isPvrFile = false;

    file->Seek(0, File::SEEK_FROM_START);
    PVRFile *pvrFile = ReadFile(file, false, false);
    if (pvrFile != nullptr)
    {
        isPvrFile = (PVRTEX3_IDENT == pvrFile->header.u32Version);
        delete pvrFile;
    }
    file->Seek(0, File::SEEK_FROM_START);
    return isPvrFile;
}

eErrorCode LibPVRHelper::ReadFile(File *infile, Vector<Image *> &imageSet, int32 fromMipmap) const
{
    PVRFile *pvrFile = ReadFile(infile, true, true);
    if (pvrFile != nullptr)
    {
        bool loaded = LoadImages(pvrFile, imageSet, fromMipmap);
        delete pvrFile;

        if (loaded)
        {
            return eErrorCode::SUCCESS;
        }
    }
    return eErrorCode::ERROR_READ_FAIL;
}

eErrorCode LibPVRHelper::WriteFile(const FilePath & fileName, const Vector<Image *> &imageSet, PixelFormat compressionFormat, ImageQuality quality) const
{
    //not implemented due to external tool
    DVASSERT(0);
    return eErrorCode::ERROR_WRITE_FAIL;
}

eErrorCode LibPVRHelper::WriteFileAsCubeMap(const FilePath & fileName, const Vector<Vector<Image *> > &imageSet, PixelFormat compressionFormat, ImageQuality quality) const
{
    //not implemented due to external tool
    DVASSERT(0);
    return eErrorCode::ERROR_WRITE_FAIL;
}

DAVA::ImageInfo LibPVRHelper::GetImageInfo(File *infile) const
{
    ImageInfo info;

    PVRFile *pvrFile = ReadFile(infile, false, false);
    if (pvrFile != nullptr)
    {
        info.width = pvrFile->header.u32Width;
        info.height = pvrFile->header.u32Height;
        info.format = GetTextureFormat(pvrFile->header);
        info.dataSize = infile->GetSize() - (PVRTEX3_HEADERSIZE + pvrFile->header.u32MetaDataSize);
        info.mipmapsCount = pvrFile->header.u32MIPMapCount;

        delete pvrFile;
    }

    return info;
}

bool LibPVRHelper::AddCRCIntoMetaData(const FilePath &filePathname) const
{
    //read file
    PVRFile *pvrFile = ReadFile(filePathname, true, true);
    if (nullptr == pvrFile)
    {
        return false;
    }

    //read crc
    uint32 presentCRC = 0;
    bool crcIsPresent = GetCRCFromMetaData(pvrFile, &presentCRC);
    if (crcIsPresent)
    {
        delete pvrFile;
        return false;
    }

    //reallocate meta data buffer
    uint8 * oldMetaData = pvrFile->metaData;
    uint32 oldMetaDataSize = pvrFile->header.u32MetaDataSize;

    pvrFile->header.u32MetaDataSize = oldMetaDataSize + METADATA_CRC_SIZE;
    pvrFile->metaData = new uint8[pvrFile->header.u32MetaDataSize];
    if (oldMetaData != nullptr)
    {
        Memcpy(pvrFile->metaData, oldMetaData, oldMetaDataSize);
        delete[] oldMetaData;
    }

    //create metaDataWithCrc
    MetaDataBlock * crcMetaData = new MetaDataBlock();
    crcMetaData->DevFOURCC = PVRTEX3_METADATAIDENT;
    crcMetaData->u32Key = METADATA_CRC_KEY;
    crcMetaData->u32DataSize = METADATA_CRC_DATA_SIZE;
    crcMetaData->Data = pvrFile->metaData + oldMetaDataSize + METADATA_DATA_OFFSET;
    pvrFile->metaDatablocks.push_back(crcMetaData);

    *((uint32 *)(pvrFile->metaData + oldMetaDataSize + METADATA_FOURCC_OFFSET)) = crcMetaData->DevFOURCC;
    *((uint32 *)(pvrFile->metaData + oldMetaDataSize + METADATA_KEY_OFFSET)) = crcMetaData->u32Key;
    *((uint32 *)(pvrFile->metaData + oldMetaDataSize + METADATA_DATASIZE_OFFSET)) = crcMetaData->u32DataSize;
    *((uint32 *)(pvrFile->metaData + oldMetaDataSize + METADATA_DATA_OFFSET)) = CRC32::ForFile(filePathname);

    bool written = false;

    File *file = File::Create(filePathname, File::CREATE | File::WRITE);
    if (file != nullptr)
    {
        file->Write(&pvrFile->header, PVRTEX3_HEADERSIZE);
        file->Write(pvrFile->metaData, pvrFile->header.u32MetaDataSize);

        if (pvrFile->compressedData)
            file->Write(pvrFile->compressedData, pvrFile->compressedDataSize);

        file->Release();

        written = true;
    }
    else
    {
        Logger::Error("[LibPVRHelper::AddCRCIntoMetaData]: cannot open file: %s", filePathname.GetAbsolutePathname().c_str());
    }

    delete pvrFile;
    return written;
}

uint32 LibPVRHelper::GetCRCFromFile(const FilePath &filePathname) const
{
    uint32 crc = 0;
    bool success = GetCRCFromMetaData(filePathname, &crc);
    return success ? crc : CRC32::ForFile(filePathname);
}

bool LibPVRHelper::WriteFileFromMipMapFiles(const FilePath & outputFilePath, const Vector<FilePath> & imgPaths)
{
    DVASSERT(imgPaths.size());

    int32 levelsCount = static_cast<int32>(imgPaths.size());

    Vector<PVRFile *> pvrFiles;
    pvrFiles.reserve(levelsCount);

    uint32 allCompressedDataSize = 0;
    for (int32 i = 0; i < levelsCount; ++i)
    {
        PVRFile * leveli = ReadFile(imgPaths[i], true, true);
        if (leveli != nullptr)
        {
            pvrFiles.push_back(leveli);
            allCompressedDataSize += leveli->compressedDataSize;
        }
    }

    DVASSERT(allCompressedDataSize);

    uint8 * allCompressedData = new uint8[allCompressedDataSize];
    Memset(allCompressedData, 0, allCompressedDataSize);

    uint8 * dataPtr = allCompressedData;

    int32 pvrFilesCount = static_cast<int32>(pvrFiles.size());
    for (int32 i = 0; i < pvrFilesCount; ++i)
    {
        PVRFile * leveli = pvrFiles[i];
        Memcpy(dataPtr, leveli->compressedData, leveli->compressedDataSize);
        dataPtr += leveli->compressedDataSize;
    }

    PVRFile *outPvr = pvrFiles[0];
    outPvr->header.u32MIPMapCount = pvrFilesCount;
    outPvr->compressedDataSize = allCompressedDataSize;
    SafeDeleteArray(outPvr->compressedData);
    outPvr->compressedData = allCompressedData;

    File * outFile = File::Create(outputFilePath, File::CREATE | File::WRITE);
    if (outFile != nullptr)
    {
        if (!WriteFile(outPvr, outFile))
        {
            Logger::Error("[LibPVRHelper] Error to write file: %s", outputFilePath.GetAbsolutePathname().c_str());
        }
    }
    else
    {
        Logger::Error("[LibPVRHelper] Error to write file: %s", outputFilePath.GetAbsolutePathname().c_str());
    }

    SafeRelease(outFile);

    for (int32 i = 0; i < pvrFilesCount; ++i)
    {
        SafeDelete(pvrFiles[i]);
    }

    return true;
}


PVRFile * LibPVRHelper::ReadFile(const FilePath &filePathname, bool readMetaData /*= false*/, bool readData /*= false*/)
{
    File *file = File::Create(filePathname, File::OPEN | File::READ);
    if (nullptr == file)
    {
        Logger::Error("[LibPVRHelper::ReadFile]: cannot read file: %s", filePathname.GetAbsolutePathname().c_str());
        return nullptr;
    }

    PVRFile *pvrFile = ReadFile(file, readMetaData, readData);
    file->Release();

    return pvrFile;
}

PVRFile * LibPVRHelper::ReadFile(File *file, bool readMetaData /*= false*/, bool readData /*= false*/)
{
    if (nullptr == file)
    {
        return nullptr;
    }

    PVRFile *pvrFile = new PVRFile();

    uint32 readSize = file->Read(&pvrFile->header, PVRTEX3_HEADERSIZE);
    if (readSize != PVRTEX3_HEADERSIZE)
    {
        Logger::Error("[LibPVRHelper::ReadFile]: cannot read header from %s", file->GetFilename().GetAbsolutePathname().c_str());
        delete pvrFile;
        return nullptr;
    }

    const bool needSwapBytes = DetectIfNeedSwapBytes(&pvrFile->header);
    PrepareHeader(&pvrFile->header, needSwapBytes);

    if (readMetaData && pvrFile->header.u32MetaDataSize)
    {
        ReadMetaData(file, pvrFile, needSwapBytes);
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
            Logger::Error("[LibPVRHelper::ReadFile]: cannot read data from file");
            delete pvrFile;
            return nullptr;
        }

        if (needSwapBytes)
        {
            SwapDataBytes(pvrFile->header, pvrFile->compressedData, pvrFile->compressedDataSize);
        }
    }

    return pvrFile;
}

bool LibPVRHelper::LoadImages(const PVRFile *pvrFile, Vector<Image *> &imageSet, int32 fromMipMap)
{
    if (nullptr == pvrFile || pvrFile->compressedData == NULL)
    {
        return false;
    }

    const uint32 & mipmapLevelCount = pvrFile->header.u32MIPMapCount;

    fromMipMap = Min(fromMipMap, (int32)(mipmapLevelCount - 1));

    bool loadAllPvrData = true;
    for (uint32 i = fromMipMap; i < mipmapLevelCount; ++i)
    {
        loadAllPvrData &= LoadMipMapLevel(pvrFile, i, (i - fromMipMap), imageSet);
    }

    return loadAllPvrData;
}

bool LibPVRHelper::WriteFile(const PVRFile * pvrFile, File *outFile)
{
    if (nullptr == pvrFile || nullptr == outFile)
    {
        return false;
    }

    uint32 writeSize = outFile->Write(&pvrFile->header, PVRTEX3_HEADERSIZE);
    if (writeSize != PVRTEX3_HEADERSIZE)
        return false;

    writeSize = outFile->Write(pvrFile->metaData, pvrFile->header.u32MetaDataSize);
    if (writeSize != pvrFile->header.u32MetaDataSize)
        return false;

    writeSize = outFile->Write(pvrFile->compressedData, pvrFile->compressedDataSize);
    if (writeSize != pvrFile->compressedDataSize)
        return false;

    return true;
}

bool LibPVRHelper::DetectIfNeedSwapBytes(const PVRHeaderV3 *header)
{
    if ((PVRTEX_CURR_IDENT != header->u32Version) &&
        (PVRTEX_CURR_IDENT_REV != header->u32Version))
    {
        return (PVRTIsLittleEndian() == false);
    }

    return (PVRTEX_CURR_IDENT_REV == header->u32Version);
}

void LibPVRHelper::PrepareHeader(PVRHeaderV3 *header, const bool swapBytes)
{
    if ((PVRTEX_CURR_IDENT != header->u32Version) &&
        (PVRTEX_CURR_IDENT_REV != header->u32Version))
    {
        //legacy pvr
        uint32 u32HeaderSize = PVRTEX3_HEADERSIZE;
        if (swapBytes)
        {
            u32HeaderSize = Min(u32HeaderSize, (uint32)PVRTByteSwap32(header->u32Version));
            uint8 *headerData = (uint8 *)&header->u32Version;
            for (uint32 i = 0; i < u32HeaderSize; i += sizeof(uint32))
            {
                PVRTByteSwap(headerData + i, sizeof(uint32));
            }
        }

        //Get a pointer to the header.
        PVRHeaderV2* sLegacyTextureHeader = (PVRHeaderV2*)header;

        //We only really need the channel type.
        uint64 tempFormat;
        EPVRTColourSpace tempColourSpace;
        bool tempIsPreMult;

        //Map the enum to get the channel type.
        EPVRTVariableType u32CurrentChannelType = ePVRTVarTypeUnsignedByte;
        MapLegacyTextureEnumToNewFormat((PVRTPixelType)(sLegacyTextureHeader->dwpfFlags & 0xff), tempFormat, tempColourSpace, u32CurrentChannelType, tempIsPreMult);
        header->u32ChannelType = u32CurrentChannelType;
    }
    else if (swapBytes)
    {
        header->u32ChannelType = PVRTByteSwap32(header->u32ChannelType);
        header->u32ColourSpace = PVRTByteSwap32(header->u32ColourSpace);
        header->u32Depth = PVRTByteSwap32(header->u32Depth);
        header->u32Flags = PVRTByteSwap32(header->u32Flags);
        header->u32Height = PVRTByteSwap32(header->u32Height);
        header->u32MetaDataSize = PVRTByteSwap32(header->u32MetaDataSize);
        header->u32MIPMapCount = PVRTByteSwap32(header->u32MIPMapCount);
        header->u32NumFaces = PVRTByteSwap32(header->u32NumFaces);
        header->u32NumSurfaces = PVRTByteSwap32(header->u32NumSurfaces);
        header->u32Version = PVRTByteSwap32(header->u32Version);
        header->u32Width = PVRTByteSwap32(header->u32Width);
        PVRTByteSwap((uint8*)&header->u64PixelFormat, sizeof(uint64));
    }
}

void LibPVRHelper::SwapDataBytes(const PVRHeaderV3 &header, uint8 *data, const uint32 dataSize)
{
    uint32 ui32VariableSize = 0;
    switch (header.u32ChannelType)
    {
        case ePVRTVarTypeFloat:
        case ePVRTVarTypeUnsignedInteger:
        case ePVRTVarTypeUnsignedIntegerNorm:
        case ePVRTVarTypeSignedInteger:
        case ePVRTVarTypeSignedIntegerNorm:
        {
            ui32VariableSize = 4;
            break;
        }
        case ePVRTVarTypeUnsignedShort:
        case ePVRTVarTypeUnsignedShortNorm:
        case ePVRTVarTypeSignedShort:
        case ePVRTVarTypeSignedShortNorm:
        {
            ui32VariableSize = 2;
            break;
        }
        case ePVRTVarTypeUnsignedByte:
        case ePVRTVarTypeUnsignedByteNorm:
        case ePVRTVarTypeSignedByte:
        case ePVRTVarTypeSignedByteNorm:
        {
            ui32VariableSize = 1;
            break;
        }

        default:
            return;
    }

    //If the size of the variable type is greater than 1, then we need to byte swap.
    //Loop through and byte swap all the data. It's swapped in place so no need to do anything special.
    for (uint32 i = 0; i < dataSize; i += ui32VariableSize)
    {
        PVRTByteSwap(data + i, ui32VariableSize);
    }
}

void LibPVRHelper::ReadMetaData(File *file, PVRFile *pvrFile, const bool swapBytes)
{
    const uint32 & metaDataSize = pvrFile->header.u32MetaDataSize;

    pvrFile->metaData = new uint8[metaDataSize];
    uint32 readSize = file->Read(pvrFile->metaData, metaDataSize);
    if (readSize != metaDataSize)
    {
        SafeDeleteArray(pvrFile->metaData);
        return;
    }

    uint8 *metaDataPtr = pvrFile->metaData;
    uint32 delta = static_cast<uint32>(metaDataPtr - pvrFile->metaData);
    while (delta < metaDataSize)
    {
        MetaDataBlock *block = new MetaDataBlock();

        uint32 fourCC = *(uint32*)metaDataPtr;
        block->DevFOURCC = (swapBytes) ? PVRTByteSwap32(fourCC) : fourCC;

        metaDataPtr += sizeof(uint32);

        uint32 dataKey = *(uint32*)metaDataPtr;
        block->u32Key = (swapBytes) ? PVRTByteSwap32(dataKey) : dataKey;

        metaDataPtr += sizeof(uint32);

        uint32 dataSize = *(uint32*)metaDataPtr;
        block->u32DataSize = (swapBytes) ? PVRTByteSwap32(dataSize) : dataSize;

        metaDataPtr += sizeof(uint32);

        if (swapBytes)
        {
            PVRTByteSwap(metaDataPtr, dataSize);
        }

        block->Data = metaDataPtr;
        metaDataPtr += dataSize;

        pvrFile->metaDatablocks.push_back(block);

        delta = static_cast<uint32>(metaDataPtr - pvrFile->metaData);
    }
}

bool LibPVRHelper::LoadMipMapLevel(const PVRFile *pvrFile, const uint32 fileMipMapLevel, const uint32 imageMipMapLevel, Vector<Image *> &imageSet)
{
    //Texture setup
    const PVRHeaderV3 &compressedHeader = pvrFile->header;
    uint8 *pTextureData = pvrFile->compressedData;

    const PixelFormatDescriptor &formatDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(GetTextureFormat(compressedHeader));
    if (!IsFormatSupported(formatDescriptor))
    {
        Logger::Error("[LibPVRHelper::LoadMipMapLevel] Unsupported format");
        return false;
    }

    const bool imageIsCompressed = PVRHelper::IsCompressedFormat(formatDescriptor.formatID);

    uint32 cubemapLayout = GetCubemapLayout(pvrFile);
    for (uint32 faceIndex = 0; faceIndex < compressedHeader.u32NumFaces; ++faceIndex)
    {
        Image *image = new Image();

        image->width = PVRT_MAX(1, compressedHeader.u32Width >> fileMipMapLevel);
        image->height = PVRT_MAX(1, compressedHeader.u32Height >> fileMipMapLevel);
        image->format = formatDescriptor.formatID;
        image->mipmapLevel = imageMipMapLevel;

        if (cubemapLayout != 0)
        {
            image->cubeFaceID = (cubemapLayout & (0x0000000F << (faceIndex * 4))) >> (faceIndex * 4);
        }

        if (formatDescriptor.isHardwareSupported || !imageIsCompressed)
        {
            bool imageLoaded = CopyToImage(image, fileMipMapLevel, faceIndex, compressedHeader, pTextureData);
            if (!imageLoaded)
            {
                image->Release();
                return false;
            }
        }
        else
        {
            //Create a near-identical texture header for the decompressed header.
            PVRHeaderV3 decompressedHeader = CreateDecompressedHeader(compressedHeader);
            if (!AllocateImageData(image, fileMipMapLevel, decompressedHeader))
            {
                image->Release();
                return false;
            }

            image->format = FORMAT_RGBA8888;

            //Setup temporary variables.
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
            uint8* pTempDecompData = image->data;
            uint8* pTempCompData = (uint8*)pTextureData + GetMipMapLayerOffset(fileMipMapLevel, faceIndex, compressedHeader);
#endif
            if ((FORMAT_PVR4 == formatDescriptor.formatID) || (FORMAT_PVR2 == formatDescriptor.formatID))
            {
#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_WIN_UAP__)
                DVASSERT_MSG(false, "Must be hardware supported PVR Compression");
                image->Release();
                return false;
#else //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)

                int do2bitMode = (FORMAT_PVR2 == formatDescriptor.formatID) ? 1 : 0;
                PVRTDecompressPVRTC(pTempCompData, do2bitMode, image->width, image->height, pTempDecompData);
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
            }
#if !defined(__DAVAENGINE_IPHONE__)
            else if (FORMAT_ETC1 == formatDescriptor.formatID)
            {
                //Create a near-identical texture header for the decompressed header.
#if defined (__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_WIN_UAP__)
                DVASSERT_MSG(false, "Must be hardware supported ETC1");
                image->Release();
                return false;
#else
                PVRTDecompressETC(pTempCompData, image->width, image->height, pTempDecompData, 0);
#endif //defined (__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_WIN_UAP__)
            }
#endif //#if !defined(__DAVAENGINE_IPHONE__)
            else
            {
                image->MakePink();
            }
        }

        imageSet.push_back(image);
    }

    return true;
}

uint32 LibPVRHelper::GetCubemapLayout(const PVRFile *pvrFile)
{
    uint32 layout = 0;

    const MetaDataBlock *cubeMetaData = GetCubemapMetadata(pvrFile);
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
    else if (pvrFile->header.u32NumFaces > 1)
    {
        static uint32 faces[] = {
            rhi::TEXTURE_FACE_POSITIVE_X,
            rhi::TEXTURE_FACE_NEGATIVE_X,
            rhi::TEXTURE_FACE_POSITIVE_Y,
            rhi::TEXTURE_FACE_NEGATIVE_Y,
            rhi::TEXTURE_FACE_POSITIVE_Z,
            rhi::TEXTURE_FACE_NEGATIVE_Z
        };
        for (uint32 i = 0; i < pvrFile->header.u32NumFaces; ++i)
        {
            layout = layout | (faces[i] << (i * 4));
        }
    }

    return layout;
}

const MetaDataBlock *LibPVRHelper::GetCubemapMetadata(const PVRFile *pvrFile)
{
    uint32 count = static_cast<uint32>(pvrFile->metaDatablocks.size());
    for (uint32 i = 0; i < count; ++i)
    {
        if (pvrFile->metaDatablocks[i]->DevFOURCC == PVRTEX3_METADATAIDENT &&
            pvrFile->metaDatablocks[i]->u32Key == METADATA_CUBE_KEY &&
            pvrFile->metaDatablocks[i]->u32DataSize == METADATA_CUBE_SIZE)
        {
            return pvrFile->metaDatablocks[i];
        }
    }

    return nullptr;
}

bool LibPVRHelper::GetCRCFromMetaData(const FilePath &filePathname, uint32* outputCRC)
{
    File *file = File::Create(filePathname, File::OPEN | File::READ);
    if (nullptr == file)
    {
        return false;
    }

    bool crcRead = false;

    const PVRFile *pvrFile = ReadFile(file, true, false);
    if (pvrFile != nullptr)
    {
        crcRead = GetCRCFromMetaData(pvrFile, outputCRC);
        delete pvrFile;
    }

    file->Release();
    return crcRead;
}

bool LibPVRHelper::GetCRCFromMetaData(const PVRFile *pvrFile, uint32* outputCRC)
{
    if (nullptr == pvrFile)
    {
        return false;
    }

    bool crcRead = false;

    uint32 metaDataCount = static_cast<uint32>(pvrFile->metaDatablocks.size());
    for (uint32 i = 0; i < metaDataCount; ++i)
    {
        const MetaDataBlock * block = pvrFile->metaDatablocks[i];
        if (block->u32Key == METADATA_CRC_KEY)
        {
            *outputCRC = *((uint32*)block->Data);

            crcRead = true;
            break;
        }
    }

    return crcRead;
}

uint32 LibPVRHelper::GetBitsPerPixel(uint64 pixelFormat)
{
#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_WIN_UAP__)
    if((pixelFormat & PVRTEX_PFHIGHMASK) != 0)
    {
        uint8 *pixelFormatChar = (uint8 *)&pixelFormat;
        return (pixelFormatChar[4] + pixelFormatChar[5] + pixelFormatChar[6] + pixelFormatChar[7]);
    }
    else
    {
        switch (pixelFormat)
        {
            case ePVRTPF_BW1bpp:
                return 1;
            case ePVRTPF_PVRTCI_2bpp_RGB:
            case ePVRTPF_PVRTCI_2bpp_RGBA:
            case ePVRTPF_PVRTCII_2bpp:
                return 2;
            case ePVRTPF_PVRTCI_4bpp_RGB:
            case ePVRTPF_PVRTCI_4bpp_RGBA:
            case ePVRTPF_PVRTCII_4bpp:
            case ePVRTPF_ETC1:
            case ePVRTPF_EAC_R11:
            case ePVRTPF_ETC2_RGB:
            case ePVRTPF_ETC2_RGB_A1:
            case ePVRTPF_DXT1:
            case ePVRTPF_BC4:
                return 4;
            case ePVRTPF_DXT2:
            case ePVRTPF_DXT3:
            case ePVRTPF_DXT4:
            case ePVRTPF_DXT5:
            case ePVRTPF_BC5:
            case ePVRTPF_EAC_RG11:
            case ePVRTPF_ETC2_RGBA:
                return 8;
            case ePVRTPF_YUY2:
            case ePVRTPF_UYVY:
            case ePVRTPF_RGBG8888:
            case ePVRTPF_GRGB8888:
                return 16;
            case ePVRTPF_SharedExponentR9G9B9E5:
                return 32;
            case ePVRTPF_NumCompressedPFs:
                return 0;
        }
    }
    return 0;
#else //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_WIN_UAP__)
    return PVRTGetBitsPerPixel(pixelFormat);
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_WIN_UAP__)
}

void LibPVRHelper::GetFormatMinDims(uint64 pixelFormat, uint32 &minX, uint32 &minY, uint32 &minZ)
{
#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_WIN_UAP__)
    switch(pixelFormat)
    {
        case ePVRTPF_DXT1:
        case ePVRTPF_DXT2:
        case ePVRTPF_DXT3:
        case ePVRTPF_DXT4:
        case ePVRTPF_DXT5:
        case ePVRTPF_BC4:
        case ePVRTPF_BC5:
        case ePVRTPF_ETC1:
        case ePVRTPF_ETC2_RGB:
        case ePVRTPF_ETC2_RGBA:
        case ePVRTPF_ETC2_RGB_A1:
        case ePVRTPF_EAC_R11:
        case ePVRTPF_EAC_RG11:
            minX = 4;
            minY = 4;
            minZ = 1;
            break;
        case ePVRTPF_PVRTCI_4bpp_RGB:
        case ePVRTPF_PVRTCI_4bpp_RGBA:
            minX = 8;
            minY = 8;
            minZ = 1;
            break;
        case ePVRTPF_PVRTCI_2bpp_RGB:
        case ePVRTPF_PVRTCI_2bpp_RGBA:
            minX = 16;
            minY = 8;
            minZ = 1;
            break;
        case ePVRTPF_PVRTCII_4bpp:
            minX = 4;
            minY = 4;
            minZ = 1;
            break;
        case ePVRTPF_PVRTCII_2bpp:
            minX = 8;
            minY = 4;
            minZ = 1;
            break;
        case ePVRTPF_UYVY:
        case ePVRTPF_YUY2:
        case ePVRTPF_RGBG8888:
        case ePVRTPF_GRGB8888:
            minX = 2;
            minY = 1;
            minZ = 1;
            break;
        case ePVRTPF_BW1bpp:
            minX = 8;
            minY = 1;
            minZ = 1;
            break;
        default: //Non-compressed formats all return 1.
            minX = 1;
            minY = 1;
            minZ = 1;
            break;
    }
#else //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_WIN_UAP__)
    PVRTGetFormatMinDims(pixelFormat, minX, minY, minZ);
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_WIN_UAP__)
}

uint32 LibPVRHelper::GetTextureDataSize(PVRHeaderV3 textureHeader, int32 mipLevel, bool allSurfaces, bool allFaces)
{
#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_WIN_UAP__)
    //The smallest divisible sizes for a pixel format
    uint32 uiSmallestWidth = 1;
    uint32 uiSmallestHeight = 1;
    uint32 uiSmallestDepth = 1;

    uint64 PixelFormatPartHigh = textureHeader.u64PixelFormat & PVRTEX_PFHIGHMASK;

    //If the pixel format is compressed, get the pixel format's minimum dimensions.
    if (PixelFormatPartHigh == 0)
    {
        GetFormatMinDims(textureHeader.u64PixelFormat, uiSmallestWidth, uiSmallestHeight, uiSmallestDepth);
    }

    //Needs to be 64-bit integer to support 16kx16k and higher sizes.
    uint64 uiDataSize = 0;
    if (mipLevel == -1)
    {
        for (uint8 uiCurrentMIP = 0; uiCurrentMIP<textureHeader.u32MIPMapCount; ++uiCurrentMIP)
        {
            //Get the dimensions of the current MIP Map level.
            uint32 uiWidth = PVRT_MAX(1, textureHeader.u32Width >> uiCurrentMIP);
            uint32 uiHeight = PVRT_MAX(1, textureHeader.u32Height >> uiCurrentMIP);
            uint32 uiDepth = PVRT_MAX(1, textureHeader.u32Depth >> uiCurrentMIP);

            //If pixel format is compressed, the dimensions need to be padded.
            if (PixelFormatPartHigh == 0)
            {
                uiWidth = uiWidth + ((-1 * uiWidth) % uiSmallestWidth);
                uiHeight = uiHeight + ((-1 * uiHeight) % uiSmallestHeight);
                uiDepth = uiDepth + ((-1 * uiDepth) % uiSmallestDepth);
            }

            //Add the current MIP Map's data size to the total.
            uiDataSize += (uint64)GetBitsPerPixel(textureHeader.u64PixelFormat) * (uint64)uiWidth * (uint64)uiHeight * (uint64)uiDepth;
        }
    }
    else
    {
        //Get the dimensions of the specified MIP Map level.
        uint32 uiWidth = PVRT_MAX(1, textureHeader.u32Width >> mipLevel);
        uint32 uiHeight = PVRT_MAX(1, textureHeader.u32Height >> mipLevel);
        uint32 uiDepth = PVRT_MAX(1, textureHeader.u32Depth >> mipLevel);

        //If pixel format is compressed, the dimensions need to be padded.
        if (PixelFormatPartHigh == 0)
        {
            uiWidth = uiWidth + ((-1 * uiWidth) % uiSmallestWidth);
            uiHeight = uiHeight + ((-1 * uiHeight) % uiSmallestHeight);
            uiDepth = uiDepth + ((-1 * uiDepth) % uiSmallestDepth);
        }

        //Work out the specified MIP Map's data size
        uiDataSize=GetBitsPerPixel(textureHeader.u64PixelFormat) * uiWidth * uiHeight * uiDepth;
    }

    //The number of faces/surfaces to register the size of.
    uint32 numfaces = ((allFaces) ? (textureHeader.u32NumFaces) : (1));
    uint32 numsurfs = ((allSurfaces) ? (textureHeader.u32NumSurfaces) : (1));

    //Multiply the data size by number of faces and surfaces specified, and return.
    return (uint32)(uiDataSize / 8) * numsurfs * numfaces;

#else //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_WIN_UAP__)
    PVRTextureHeaderV3 *header = (PVRTextureHeaderV3 *)&textureHeader;
    return PVRTGetTextureDataSize(*header, mipLevel, allSurfaces, allFaces);
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_WIN_UAP__)
}

int32 LibPVRHelper::GetMipMapLayerOffset(uint32 mipMapLevel, uint32 faceIndex, const PVRHeaderV3 &header)
{
    int32 offset = 0;
    for (uint32 uiMIPMap = 0; uiMIPMap < mipMapLevel; ++uiMIPMap)
    {
        offset += GetTextureDataSize(header, uiMIPMap, false, true);
    }
    return offset + faceIndex * GetTextureDataSize(header, mipMapLevel, false, false);
}

void LibPVRHelper::MapLegacyTextureEnumToNewFormat(PVRTPixelType OldFormat, uint64& newType, EPVRTColourSpace& newCSpace, EPVRTVariableType& newChanType, bool& isPreMult)
{
#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_WIN_UAP__)
    //Default value.
    isPreMult = false;

    switch (OldFormat)
    {
        case MGLPT_ARGB_4444:
        {
            newType = PVRTGENPIXELID4('a','r','g','b',4,4,4,4);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case MGLPT_ARGB_1555:
        {
            newType = PVRTGENPIXELID4('a','r','g','b',1,5,5,5);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case MGLPT_RGB_565:
        {
            newType = PVRTGENPIXELID3('r','g','b',5,6,5);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case MGLPT_RGB_555:
        {
            newType = PVRTGENPIXELID4('x','r','g','b',1,5,5,5);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case MGLPT_RGB_888:
        {
            newType = PVRTGENPIXELID3('r','g','b',8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }

        case MGLPT_ARGB_8888:
        {
            newType = PVRTGENPIXELID4('a','r','g','b',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }

        case MGLPT_ARGB_8332:
        {
            newType = PVRTGENPIXELID4('a','r','g','b',8,3,3,2);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case MGLPT_I_8:
        {
            newType = PVRTGENPIXELID1('i',8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case MGLPT_AI_88:
        {
            newType = PVRTGENPIXELID2('a','i',8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case MGLPT_1_BPP:
        {
            newType = ePVRTPF_BW1bpp;
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case MGLPT_VY1UY0:
        {
            newType = ePVRTPF_YUY2;
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case MGLPT_Y1VY0U:
        {
            newType = ePVRTPF_UYVY;
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case MGLPT_PVRTC2:
        {
            newType = ePVRTPF_PVRTCI_2bpp_RGBA;
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case MGLPT_PVRTC4:
        {
            newType = ePVRTPF_PVRTCI_4bpp_RGBA;
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case OGL_RGBA_4444:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',4,4,4,4);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case OGL_RGBA_5551:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',5,5,5,1);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case OGL_RGBA_8888:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case OGL_RGB_565:
        {
            newType = PVRTGENPIXELID3('r','g','b',5,6,5);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case OGL_RGB_555:
        {
            newType = PVRTGENPIXELID4('r','g','b','x',5,5,5,1);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case OGL_RGB_888:
        {
            newType = PVRTGENPIXELID3('r','g','b',8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case OGL_I_8:
        {
            newType = PVRTGENPIXELID1('l',8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case OGL_AI_88:
        {
            newType = PVRTGENPIXELID2('l','a',8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case OGL_PVRTC2:
        {
            newType = ePVRTPF_PVRTCI_2bpp_RGBA;
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case OGL_PVRTC4:
        {
            newType = ePVRTPF_PVRTCI_4bpp_RGBA;
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case OGL_BGRA_8888:
        {
            newType = PVRTGENPIXELID4('b','g','r','a',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case OGL_A_8:
        {
            newType = PVRTGENPIXELID1('a',8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case OGL_PVRTCII4:
        {
            newType = ePVRTPF_PVRTCII_4bpp;
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case OGL_PVRTCII2:
        {
            newType = ePVRTPF_PVRTCII_2bpp;
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        //#ifdef _WIN32
        //        case D3D_DXT1:
        //        {
        //            newType = ePVRTPF_DXT1;
        //            newCSpace = ePVRTCSpacelRGB;
        //            newChanType = ePVRTVarTypeUnsignedByteNorm;
        //            break;
        //        }
        //
        //        case D3D_DXT2:
        //        {
        //            newType = ePVRTPF_DXT2;
        //            newCSpace = ePVRTCSpacelRGB;
        //            newChanType = ePVRTVarTypeUnsignedByteNorm;
        //            isPreMult = true;
        //            break;
        //        }
        //
        //        case D3D_DXT3:
        //        {
        //            newType = ePVRTPF_DXT3;
        //            newCSpace = ePVRTCSpacelRGB;
        //            newChanType = ePVRTVarTypeUnsignedByteNorm;
        //            break;
        //        }
        //
        //        case D3D_DXT4:
        //        {
        //            newType = ePVRTPF_DXT4;
        //            newCSpace = ePVRTCSpacelRGB;
        //            newChanType = ePVRTVarTypeUnsignedByteNorm;
        //            isPreMult = true;
        //            break;
        //        }
        //
        //        case D3D_DXT5:
        //        {
        //            newType = ePVRTPF_DXT5;
        //            newCSpace = ePVRTCSpacelRGB;
        //            newChanType = ePVRTVarTypeUnsignedByteNorm;
        //            break;
        //        }
        //
        //#endif
        case D3D_RGB_332:
        {
            newType = PVRTGENPIXELID3('r','g','b',3,3,2);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }

        case D3D_AL_44:
        {
            newType = PVRTGENPIXELID2('a','l',4,4);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }

        case D3D_LVU_655:
        {
            newType = PVRTGENPIXELID3('l','g','r',6,5,5);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedIntegerNorm;
            break;
        }

        case D3D_XLVU_8888:
        {
            newType = PVRTGENPIXELID4('x','l','g','r',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedIntegerNorm;
            break;
        }

        case D3D_QWVU_8888:
        {
            newType = PVRTGENPIXELID4('a','b','g','r',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedIntegerNorm;
            break;
        }

        case D3D_ABGR_2101010:
        {
            newType = PVRTGENPIXELID4('a','b','g','r',2,10,10,10);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }

        case D3D_ARGB_2101010:
        {
            newType = PVRTGENPIXELID4('a','r','g','b',2,10,10,10);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }

        case D3D_AWVU_2101010:
        {
            newType = PVRTGENPIXELID4('a','r','g','b',2,10,10,10);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }

        case D3D_GR_1616:
        {
            newType = PVRTGENPIXELID2('g','r',16,16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }

        case D3D_VU_1616:
        {
            newType = PVRTGENPIXELID2('g','r',16,16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedIntegerNorm;
            break;
        }

        case D3D_ABGR_16161616:
        {
            newType = PVRTGENPIXELID4('a','b','g','r',16,16,16,16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }

        case D3D_R16F:
        {
            newType = PVRTGENPIXELID1('r',16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedFloat;
            break;
        }

        case D3D_GR_1616F:
        {
            newType = PVRTGENPIXELID2('g','r',16,16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedFloat;
            break;
        }

        case D3D_ABGR_16161616F:
        {
            newType = PVRTGENPIXELID4('a','b','g','r',16,16,16,16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedFloat;
            break;
        }

        case D3D_R32F:
        {
            newType = PVRTGENPIXELID1('r',32);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedFloat;
            break;
        }

        case D3D_GR_3232F:
        {
            newType = PVRTGENPIXELID2('g','r',32,32);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedFloat;
            break;
        }

        case D3D_ABGR_32323232F:
        {
            newType = PVRTGENPIXELID4('a','b','g','r',32,32,32,32);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedFloat;
            break;
        }

        case ETC_RGB_4BPP:
        {
            newType = ePVRTPF_ETC1;
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case D3D_A8:
        {
            newType = PVRTGENPIXELID1('a',8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }

        case D3D_V8U8:
        {
            newType = PVRTGENPIXELID2('g','r',8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedIntegerNorm;
            break;
        }

        case D3D_L16:
        {
            newType = PVRTGENPIXELID1('l',16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }

        case D3D_L8:
        {
            newType = PVRTGENPIXELID1('l',8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }

        case D3D_AL_88:
        {
            newType = PVRTGENPIXELID2('a','l',8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }

        case D3D_UYVY:
        {
            newType = ePVRTPF_UYVY;
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case D3D_YUY2:
        {
            newType = ePVRTPF_YUY2;
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case DX10_R32G32B32A32_FLOAT:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',32,32,32,32);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedFloat;
            break;
        }

        case DX10_R32G32B32A32_UINT:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',32,32,32,32);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedInteger;
            break;
        }

        case DX10_R32G32B32A32_SINT:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',32,32,32,32);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedInteger;
            break;
        }

        case DX10_R32G32B32_FLOAT:
        {
            newType = PVRTGENPIXELID3('r','g','b',32,32,32);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedFloat;
            break;
        }

        case DX10_R32G32B32_UINT:
        {
            newType = PVRTGENPIXELID3('r','g','b',32,32,32);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedInteger;
            break;
        }

        case DX10_R32G32B32_SINT:
        {
            newType = PVRTGENPIXELID3('r','g','b',32,32,32);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedInteger;
            break;
        }

        case DX10_R16G16B16A16_FLOAT:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',16,16,16,16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedFloat;
            break;
        }

        case DX10_R16G16B16A16_UNORM:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',16,16,16,16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case DX10_R16G16B16A16_UINT:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',16,16,16,16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedShort;
            break;
        }

        case DX10_R16G16B16A16_SNORM:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',16,16,16,16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedShortNorm;
            break;
        }

        case DX10_R16G16B16A16_SINT:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',16,16,16,16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedShort;
            break;
        }

        case DX10_R32G32_FLOAT:
        {
            newType = PVRTGENPIXELID2('r','g',32,32);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedFloat;
            break;
        }

        case DX10_R32G32_UINT:
        {
            newType = PVRTGENPIXELID2('r','g',32,32);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedInteger;
            break;
        }

        case DX10_R32G32_SINT:
        {
            newType = PVRTGENPIXELID2('r','g',32,32);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedInteger;
            break;
        }

        case DX10_R10G10B10A2_UNORM:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',10,10,10,2);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }

        case DX10_R10G10B10A2_UINT:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',10,10,10,2);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedInteger;
            break;
        }

        case DX10_R11G11B10_FLOAT:
        {
            newType = PVRTGENPIXELID3('r','g','b',11,11,10);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedFloat;
            break;
        }

        case DX10_R8G8B8A8_UNORM:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case DX10_R8G8B8A8_UNORM_SRGB:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case DX10_R8G8B8A8_UINT:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByte;
            break;
        }

        case DX10_R8G8B8A8_SNORM:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedByteNorm;
            break;
        }

        case DX10_R8G8B8A8_SINT:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedByte;
            break;
        }

        case DX10_R16G16_FLOAT:
        {
            newType = PVRTGENPIXELID2('r','g',16,16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedFloat;
            break;
        }

        case DX10_R16G16_UNORM:
        {
            newType = PVRTGENPIXELID2('r','g',16,16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case DX10_R16G16_UINT:
        {
            newType = PVRTGENPIXELID2('r','g',16,16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedShort;
            break;
        }

        case DX10_R16G16_SNORM:
        {
            newType = PVRTGENPIXELID2('r','g',16,16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedShortNorm;
            break;
        }

        case DX10_R16G16_SINT:
        {
            newType = PVRTGENPIXELID2('r','g',16,16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedShort;
            break;
        }

        case DX10_R32_FLOAT:
        {
            newType = PVRTGENPIXELID1('r',32);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedFloat;
            break;
        }

        case DX10_R32_UINT:
        {
            newType = PVRTGENPIXELID1('r',32);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedInteger;
            break;
        }

        case DX10_R32_SINT:
        {
            newType = PVRTGENPIXELID1('r',32);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedInteger;
            break;
        }

        case DX10_R8G8_UNORM:
        {
            newType = PVRTGENPIXELID2('r','g',8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case DX10_R8G8_UINT:
        {
            newType = PVRTGENPIXELID2('r','g',8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByte;
            break;
        }

        case DX10_R8G8_SNORM:
        {
            newType = PVRTGENPIXELID2('r','g',8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedByteNorm;
            break;
        }

        case DX10_R8G8_SINT:
        {
            newType = PVRTGENPIXELID2('r','g',8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedByte;
            break;
        }

        case DX10_R16_FLOAT:
        {
            newType = PVRTGENPIXELID1('r',16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedFloat;
            break;
        }

        case DX10_R16_UNORM:
        {
            newType = PVRTGENPIXELID1('r',16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case DX10_R16_UINT:
        {
            newType = PVRTGENPIXELID1('r',16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedShort;
            break;
        }

        case DX10_R16_SNORM:
        {
            newType = PVRTGENPIXELID1('r',16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedShortNorm;
            break;
        }

        case DX10_R16_SINT:
        {
            newType = PVRTGENPIXELID1('r',16);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedShort;
            break;
        }

        case DX10_R8_UNORM:
        {
            newType = PVRTGENPIXELID1('r',8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case DX10_R8_UINT:
        {
            newType = PVRTGENPIXELID1('r',8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByte;
            break;
        }

        case DX10_R8_SNORM:
        {
            newType = PVRTGENPIXELID1('r',8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedByteNorm;
            break;
        }

        case DX10_R8_SINT:
        {
            newType = PVRTGENPIXELID1('r',8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedByte;
            break;
        }

        case DX10_A8_UNORM:
        {
            newType = PVRTGENPIXELID1('r',8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case DX10_R1_UNORM:
        {
            newType = ePVRTPF_BW1bpp;
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case DX10_R9G9B9E5_SHAREDEXP:
        {
            newType = ePVRTPF_SharedExponentR9G9B9E5;
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeSignedFloat;
            break;
        }

        case DX10_R8G8_B8G8_UNORM:
        {
            newType = ePVRTPF_RGBG8888;
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case DX10_G8R8_G8B8_UNORM:
        {
            newType = ePVRTPF_GRGB8888;
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        //#ifdef _WIN32
        //        case DX10_BC1_UNORM:
        //        {
        //            newType = ePVRTPF_DXT1;
        //            newCSpace = ePVRTCSpacelRGB;
        //            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
        //            break;
        //        }
        //
        //        case DX10_BC1_UNORM_SRGB:
        //        {
        //            newType = ePVRTPF_DXT1;
        //            newCSpace = ePVRTCSpacesRGB;
        //            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
        //            break;
        //        }
        //
        //        case DX10_BC2_UNORM:
        //        {
        //            newType = ePVRTPF_DXT3;
        //            newCSpace = ePVRTCSpacelRGB;
        //            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
        //            break;
        //        }
        //
        //        case DX10_BC2_UNORM_SRGB:
        //        {
        //            newType = ePVRTPF_DXT3;
        //            newCSpace = ePVRTCSpacesRGB;
        //            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
        //            break;
        //        }
        //
        //        case DX10_BC3_UNORM:
        //        {
        //            newType = ePVRTPF_DXT5;
        //            newCSpace = ePVRTCSpacelRGB;
        //            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
        //            break;
        //        }
        //
        //        case DX10_BC3_UNORM_SRGB:
        //        {
        //            newType = ePVRTPF_DXT5;
        //            newCSpace = ePVRTCSpacesRGB;
        //            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
        //            break;
        //        }
        //
        //        case DX10_BC4_UNORM:
        //        {
        //            newType = ePVRTPF_BC4;
        //            newCSpace = ePVRTCSpacesRGB;
        //            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
        //            break;
        //        }
        //
        //        case DX10_BC4_SNORM:
        //        {
        //            newType = ePVRTPF_BC4;
        //            newCSpace = ePVRTCSpacelRGB;
        //            newChanType = ePVRTVarTypeSignedIntegerNorm;
        //            break;
        //        }
        //
        //        case DX10_BC5_UNORM:
        //        {
        //            newType = ePVRTPF_BC5;
        //            newCSpace = ePVRTCSpacelRGB;
        //            newChanType = ePVRTVarTypeUnsignedIntegerNorm;
        //            break;
        //        }
        //
        //        case DX10_BC5_SNORM:
        //        {
        //            newType = ePVRTPF_BC5;
        //            newCSpace = ePVRTCSpacelRGB;
        //            newChanType = ePVRTVarTypeSignedIntegerNorm;
        //            break;
        //        }
        //
        //#endif
        case ePT_VG_sRGBX_8888:
        {
            newType = PVRTGENPIXELID4('r','g','b','x',8,8,8,8);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case ePT_VG_sRGBA_8888:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case ePT_VG_sRGBA_8888_PRE:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            isPreMult = true;
            break;
        }

        case ePT_VG_sRGB_565:
        {
            newType = PVRTGENPIXELID3('r','g','b',5,6,5);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case ePT_VG_sRGBA_5551:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',5,5,5,1);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case ePT_VG_sRGBA_4444:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',4,4,4,4);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case ePT_VG_sL_8:
        {
            newType = PVRTGENPIXELID1('l',8);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case ePT_VG_lRGBX_8888:
        {
            newType = PVRTGENPIXELID4('r','g','b','x',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case ePT_VG_lRGBA_8888:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case ePT_VG_lRGBA_8888_PRE:
        {
            newType = PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            isPreMult = true;
            break;
        }

        case ePT_VG_lL_8:
        {
            newType = PVRTGENPIXELID1('l',8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case ePT_VG_A_8:
        {
            newType = PVRTGENPIXELID1('a',8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case ePT_VG_BW_1:
        {
            newType = ePVRTPF_BW1bpp;
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case ePT_VG_sXRGB_8888:
        {
            newType = PVRTGENPIXELID4('x','r','g','b',8,8,8,8);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case ePT_VG_sARGB_8888:
        {
            newType = PVRTGENPIXELID4('a','r','g','b',8,8,8,8);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case ePT_VG_sARGB_8888_PRE:
        {
            newType = PVRTGENPIXELID4('a','r','g','b',8,8,8,8);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            isPreMult = true;
            break;
        }

        case ePT_VG_sARGB_1555:
        {
            newType = PVRTGENPIXELID4('a','r','g','b',1,5,5,5);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case ePT_VG_sARGB_4444:
        {
            newType = PVRTGENPIXELID4('a','r','g','b',4,4,4,4);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case ePT_VG_lXRGB_8888:
        {
            newType = PVRTGENPIXELID4('x','r','g','b',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case ePT_VG_lARGB_8888:
        {
            newType = PVRTGENPIXELID4('a','r','g','b',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case ePT_VG_lARGB_8888_PRE:
        {
            newType = PVRTGENPIXELID4('a','r','g','b',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            isPreMult = true;
            break;
        }

        case ePT_VG_sBGRX_8888:
        {
            newType = PVRTGENPIXELID4('b','g','r','x',8,8,8,8);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case ePT_VG_sBGRA_8888:
        {
            newType = PVRTGENPIXELID4('b','g','r','a',8,8,8,8);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case ePT_VG_sBGRA_8888_PRE:
        {
            newType = PVRTGENPIXELID4('b','g','r','a',8,8,8,8);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            isPreMult = true;
            break;
        }

        case ePT_VG_sBGR_565:
        {
            newType = PVRTGENPIXELID3('b','g','r',5,6,5);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case ePT_VG_sBGRA_5551:
        {
            newType = PVRTGENPIXELID4('b','g','r','a',5,5,5,1);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case ePT_VG_sBGRA_4444:
        {
            newType = PVRTGENPIXELID4('b','g','r','x',4,4,4,4);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case ePT_VG_lBGRX_8888:
        {
            newType = PVRTGENPIXELID4('b','g','r','x',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case ePT_VG_lBGRA_8888:
        {
            newType = PVRTGENPIXELID4('b','g','r','a',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case ePT_VG_lBGRA_8888_PRE:
        {
            newType = PVRTGENPIXELID4('b','g','r','a',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            isPreMult = true;
            break;
        }

        case ePT_VG_sXBGR_8888:
        {
            newType = PVRTGENPIXELID4('x','b','g','r',8,8,8,8);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case ePT_VG_sABGR_8888:
        {
            newType = PVRTGENPIXELID4('a','b','g','r',8,8,8,8);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case ePT_VG_sABGR_8888_PRE:
        {
            newType = PVRTGENPIXELID4('a','b','g','r',8,8,8,8);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            isPreMult = true;
            break;
        }

        case ePT_VG_sABGR_1555:
        {
            newType = PVRTGENPIXELID4('a','b','g','r',1,5,5,5);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case ePT_VG_sABGR_4444:
        {
            newType = PVRTGENPIXELID4('x','b','g','r',4,4,4,4);
            newCSpace = ePVRTCSpacesRGB;
            newChanType = ePVRTVarTypeUnsignedShortNorm;
            break;
        }

        case ePT_VG_lXBGR_8888:
        {
            newType = PVRTGENPIXELID4('x','b','g','r',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case ePT_VG_lABGR_8888:
        {
            newType = PVRTGENPIXELID4('a','b','g','r',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            break;
        }

        case ePT_VG_lABGR_8888_PRE:
        {
            newType = PVRTGENPIXELID4('a','b','g','r',8,8,8,8);
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeUnsignedByteNorm;
            isPreMult = true;
            break;
        }
        default:
        {
            newType = ePVRTPF_NumCompressedPFs;
            newCSpace = ePVRTCSpacelRGB;
            newChanType = ePVRTVarTypeNumVarTypes;
            break;
        }
    }

#else //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_WIN_UAP__)
    PVRTMapLegacyTextureEnumToNewFormat(OldFormat, newType, newCSpace, newChanType, isPreMult);
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_WIN_UAP__)
}

const PixelFormat LibPVRHelper::GetTextureFormat(const PVRHeaderV3& textureHeader)
{
    uint64 pixelFormat = textureHeader.u64PixelFormat;
    EPVRTVariableType ChannelType = (EPVRTVariableType)textureHeader.u32ChannelType;

    //Get the last 32 bits of the pixel format.
    uint64 PixelFormatPartHigh = pixelFormat&PVRTEX_PFHIGHMASK;

    //Check for a compressed format (The first 8 bytes will be 0, so the whole thing will be equal to the last 32 bits).
    if (PixelFormatPartHigh == 0)
    {
        //Format and type == 0 for compressed textures.
        return GetCompressedFormat(pixelFormat);
    }
    else
    {
        switch (ChannelType)
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

const PixelFormat LibPVRHelper::GetCompressedFormat(const uint64 pixelFormat)
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

const PixelFormat LibPVRHelper::GetFloatTypeFormat(const uint64 pixelFormat)
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
    }

    return FORMAT_INVALID;
}

const PixelFormat LibPVRHelper::GetUnsignedByteFormat(const uint64 pixelFormat)
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
    }

    return FORMAT_INVALID;
}

const PixelFormat LibPVRHelper::GetUnsignedShortFormat(const uint64 pixelFormat)
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
    }

    return FORMAT_INVALID;
}

bool LibPVRHelper::IsFormatSupported(const PixelFormatDescriptor &format)
{
    if (FORMAT_INVALID == format.formatID)
    {
        Logger::Error("[LibPVRHelper::IsFormatSupported] FORMAT_INVALID");
        return false;
    }

    if ((FORMAT_RGBA16161616 == format.formatID) && !format.isHardwareSupported)
    {
        Logger::Error("[LibPVRHelper::IsFormatSupported] FORMAT_RGBA16161616 is unsupported");
        return false;
    }

    if ((FORMAT_RGBA32323232 == format.formatID) && !format.isHardwareSupported)
    {
        Logger::Error("[LibPVRHelper::IsFormatSupported] FORMAT_RGBA32323232 is unsupported");
        return false;
    }

#if defined (__DAVAENGINE_IPHONE__)
    if (FORMAT_ETC1 == format.formatID)
    {
        Logger::Error("[LibPVRHelper::IsFormatSupported] FORMAT_ETC1 not supported for IOS");
        return false;
    }
#endif

    return true;
}

PVRHeaderV3 LibPVRHelper::CreateDecompressedHeader(const PVRHeaderV3 &compressedHeader)
{
    PVRHeaderV3 decompressedHeader = compressedHeader;
    decompressedHeader.u32ChannelType = ePVRTVarTypeUnsignedByteNorm;
    decompressedHeader.u32ColourSpace = ePVRTCSpacelRGB;
    decompressedHeader.u64PixelFormat = PVRTGENPIXELID4('r', 'g', 'b', 'a', 8, 8, 8, 8);

    return decompressedHeader;
}

bool LibPVRHelper::CopyToImage(Image *image, uint32 mipMapLevel, uint32 faceIndex, const PVRHeaderV3 &header, const uint8 *pvrData)
{
    if (AllocateImageData(image, mipMapLevel, header))
    {
        //Setup temporary variables.
        uint8* data = (uint8*)pvrData + GetMipMapLayerOffset(mipMapLevel, faceIndex, header);

        if (image->format == FORMAT_RGBA4444)
        {
            ConvertDirect<uint16, uint16, ConvertABGR4444toRGBA4444> convert;
            convert(data, image->width, image->height, image->width * sizeof(uint16), image->data);
        }
        else if (image->format == FORMAT_RGBA5551)
        {
            ConvertDirect<uint16, uint16, ConvertABGR1555toRGBA5551> convert;
            convert(data, image->width, image->height, image->width * sizeof(uint16), image->data);
        }
        else
        {
            Memcpy(image->data, data, image->dataSize);
        }

        return true;
    }

    return false;
}

bool LibPVRHelper::AllocateImageData(DAVA::Image *image, uint32 mipMapLevel, const DAVA::PVRHeaderV3 &header)
{
    image->dataSize = GetTextureDataSize(header, mipMapLevel, false, (header.u32NumFaces == 1));
    image->data = new uint8[image->dataSize];
    if (!image->data)
    {
        Logger::Error("[LibPVRHelper::AllocateImageData] Unable to allocate memory to compressed texture.\n");
        return false;
    }

    return true;
}

};
