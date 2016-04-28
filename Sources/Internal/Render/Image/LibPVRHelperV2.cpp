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


#include "Render/Image/LibPVRHelperV2.h"
#include "Render/Image/Private/PVRFormatHelper.h"

#include "FileSystem/FileSystem.h"
#include "Logger/Logger.h"
#include "Utils/CRC32.h"

namespace DAVA
{
LibPVRHelperV2::LibPVRHelperV2()
    : ImageFormatInterface(ImageFormat::IMAGE_FORMAT_PVR, "PVR")
{
    supportedExtensions.push_back(".pvr");
}

bool LibPVRHelperV2::CanProcessFile(File* file) const
{
    std::unique_ptr<PVRFile> pvrFile = PVRFormatHelper::ReadFile(file, false, false);
    if (pvrFile)
    {
        return (PVRTEX3_IDENT == pvrFile->header.u32Version);
    }
    return false;
}

eErrorCode LibPVRHelperV2::ReadFile(File* infile, Vector<Image*>& imageSet, int32 fromMipmap, int32 firstMipmapIndex) const
{
    if (PVRFormatHelper::LoadImages(infile, imageSet, fromMipmap, firstMipmapIndex))
    {
        return eErrorCode::SUCCESS;
    }

    return eErrorCode::ERROR_READ_FAIL;
}

eErrorCode LibPVRHelperV2::WriteFile(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const
{
    //not implemented due to external tool
    DVASSERT(0);
    return eErrorCode::ERROR_WRITE_FAIL;
}

eErrorCode LibPVRHelperV2::WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const
{
    //not implemented due to external tool
    DVASSERT(0);
    return eErrorCode::ERROR_WRITE_FAIL;
}

eErrorCode LibPVRHelperV2::WriteFile(const FilePath& fileName, const Vector<Image*>& imageSet) const
{
    DVASSERT(0);
    return eErrorCode::ERROR_WRITE_FAIL;
}

eErrorCode LibPVRHelperV2::WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet) const
{
    DVASSERT(0);
    return eErrorCode::ERROR_WRITE_FAIL;
}

DAVA::ImageInfo LibPVRHelperV2::GetImageInfo(File* infile) const
{
    ImageInfo info;

    std::unique_ptr<PVRFile> pvrFile = PVRFormatHelper::ReadFile(infile, false, false);
    if (pvrFile)
    {
        info.width = pvrFile->header.u32Width;
        info.height = pvrFile->header.u32Height;
        info.format = PVRFormatHelper::GetTextureFormat(pvrFile->header);
        info.dataSize = infile->GetSize() - (PVRTEX3_HEADERSIZE + pvrFile->header.u32MetaDataSize);
        info.mipmapsCount = pvrFile->header.u32MIPMapCount;
    }

    return info;
}

bool LibPVRHelperV2::AddCRCIntoMetaData(const FilePath& filePathname) const
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

uint32 LibPVRHelperV2::GetCRCFromFile(const FilePath& filePathname) const
{
    std::unique_ptr<PVRFile> pvrFile = PVRFormatHelper::ReadFile(filePathname, false, false);
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
}
