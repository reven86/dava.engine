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


#include "Render/Image/LibPVRHelper.h"
#include "Render/Image/Private/PVRFormatHelper.h"
#include "Render/Image/Image.h"
#include "Render/PixelFormatDescriptor.h"

#include "FileSystem/FileSystem.h"
#include "Logger/Logger.h"
#include "Utils/CRC32.h"

namespace DAVA
{
LibPVRHelper::LibPVRHelper()
    : ImageFormatInterface(ImageFormat::IMAGE_FORMAT_PVR, "PVR")
{
    supportedExtensions.push_back(".pvr");
}

bool LibPVRHelper::CanProcessFileInternal(File* file) const
{
    std::unique_ptr<PVRFile> pvrFile = PVRFormatHelper::ReadFile(file, false, false);
    if (pvrFile)
    {
        return (PVRTEX3_IDENT == pvrFile->header.u32Version);
    }
    return false;
}

eErrorCode LibPVRHelper::Load(File* infile, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams) const
{
    if (PVRFormatHelper::LoadImages(infile, imageSet, loadingParams))
    {
        DVASSERT(imageSet.empty() == false);
        return eErrorCode::SUCCESS;
    }

    return eErrorCode::ERROR_READ_FAIL;
}

eErrorCode LibPVRHelper::ReadFile(File* infile, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams) const
{
    eErrorCode loadResult = Load(infile, imageSet, loadingParams);
    if (eErrorCode::SUCCESS != loadResult || imageSet.empty())
    {
        return loadResult;
    }

    return eErrorCode::SUCCESS;
}

eErrorCode LibPVRHelper::WriteFile(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const
{
    if (imageSet.empty())
    {
        return eErrorCode::ERROR_WRITE_FAIL;
    }

    if (imageSet[0]->format != compressionFormat)
    {
        return eErrorCode::ERROR_FILE_FORMAT_INCORRECT;
    }

    return Save(fileName, imageSet);
}

eErrorCode LibPVRHelper::WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const
{
    if (imageSet.empty())
    {
        return eErrorCode::ERROR_WRITE_FAIL;
    }

    for (const Vector<Image*>& imgSet : imageSet)
    {
        if (imgSet.empty())
        {
            return eErrorCode::ERROR_WRITE_FAIL;
        }

        if (imgSet[0]->format != compressionFormat)
        {
            return eErrorCode::ERROR_FILE_FORMAT_INCORRECT;
        }
    }

    return SaveCubeMap(fileName, imageSet);
}

eErrorCode LibPVRHelper::Save(const FilePath& fileName, const Vector<Image*>& imageSet) const
{
    if (imageSet.empty())
    {
        Logger::Error("Can't save empty images vector");
        return eErrorCode::ERROR_WRITE_FAIL;
    }

    std::unique_ptr<PVRFile> pvrFile = PVRFormatHelper::GenerateHeader(imageSet);
    if (pvrFile)
    {
        if (PVRFormatHelper::WriteFile(fileName, *pvrFile))
        {
            return eErrorCode::SUCCESS;
        }
    }

    return eErrorCode::ERROR_WRITE_FAIL;
}

eErrorCode LibPVRHelper::SaveCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet) const
{
    uint32 faceCount = static_cast<uint32>(imageSet.size());
    if (faceCount != Texture::CUBE_FACE_COUNT)
    {
        Logger::Error("Can't save cube images vector due wrong face count: %u of %u", faceCount, Texture::CUBE_FACE_COUNT);
        return eErrorCode::ERROR_WRITE_FAIL;
    }

    uint32 mipmapsCount = static_cast<uint32>(imageSet[0].size());
    for (const Vector<Image*>& faceImageSet : imageSet)
    {
        uint32 faceMipmpsCount = static_cast<uint32>(faceImageSet.size());
        if (faceMipmpsCount == 0 || faceMipmpsCount != mipmapsCount)
        {
            Logger::Error("Can't save cube images vector due wrong mip count: %u of %u", faceMipmpsCount, mipmapsCount);
            return eErrorCode::ERROR_WRITE_FAIL;
        }
    }

    std::unique_ptr<PVRFile> pvrFile = PVRFormatHelper::GenerateCubeHeader(imageSet);
    if (pvrFile)
    {
        if (PVRFormatHelper::WriteFile(fileName, *pvrFile))
        {
            return eErrorCode::SUCCESS;
        }
    }

    return eErrorCode::ERROR_WRITE_FAIL;
}

DAVA::ImageInfo LibPVRHelper::GetImageInfo(File* infile) const
{
    ImageInfo info;

    std::unique_ptr<PVRFile> pvrFile = PVRFormatHelper::ReadFile(infile, false, false);
    if (pvrFile)
    {
        info.width = pvrFile->header.u32Width;
        info.height = pvrFile->header.u32Height;
        info.format = PVRFormatHelper::GetTextureFormat(pvrFile->header);
        info.dataSize = infile->GetSize() - (PVRFile::HEADER_SIZE + pvrFile->header.u32MetaDataSize);
        info.mipmapsCount = pvrFile->header.u32MIPMapCount;
    }

    return info;
}

bool LibPVRHelper::AddCRCIntoMetaData(const FilePath& filePathname) const
{
    //read file
    std::unique_ptr<PVRFile> pvrFile = PVRFormatHelper::ReadFile(filePathname, true, true);
    if (!pvrFile)
    {
        return false;
    }

    //read crc
    uint32 presentCRC = 0;
    bool crcIsPresent = PVRFormatHelper::GetCRCFromMetaData(*pvrFile, &presentCRC);
    if (crcIsPresent)
    {
        return false;
    }

    uint32 calculatedCRC = CRC32::ForFile(filePathname);
    PVRFormatHelper::AddCRCToMetaData(*pvrFile, calculatedCRC);

    return PVRFormatHelper::WriteFile(filePathname, *pvrFile);
}

uint32 LibPVRHelper::GetCRCFromFile(const FilePath& filePathname) const
{
    std::unique_ptr<PVRFile> pvrFile = PVRFormatHelper::ReadFile(filePathname, true, false);
    if (pvrFile)
    {
        uint32 presentCRC = 0;
        bool crcIsPresent = PVRFormatHelper::GetCRCFromMetaData(*pvrFile, &presentCRC);
        if (crcIsPresent)
        {
            return presentCRC;
        }
    }

    return 0;
}

Image* LibPVRHelper::DecodeToRGBA8888(Image* encodedImage) const
{
    return PVRFormatHelper::DecodeToRGBA8888(encodedImage);
}
}
