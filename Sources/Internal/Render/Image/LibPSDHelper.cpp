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


#include "Render/Image/LibPSDHelper.h"
#include "Render/Image/Image.h"
#include "FileSystem/File.h"
#include "webp/decode.h"
#include "webp/encode.h"

namespace DAVA
{
LibPSDHelper::LibPSDHelper()
{
    name.assign("PSD");
    supportedExtensions.emplace_back(".psd");
    supportedFormats = { { FORMAT_RGBA8888 } };
}

bool LibPSDHelper::CanProcessFile(File* infile) const
{
    return GetImageInfo(infile).dataSize != 0;
}

eErrorCode LibPSDHelper::ReadFile(File* infile, Vector<Image*>& imageSet, int32 baseMipMap) const
{
    Logger::Error("[LibPSDHelper::WriteFileAsCubeMap] Reading PSD not implemented");
    return eErrorCode::ERROR_READ_FAIL;
}

eErrorCode LibPSDHelper::WriteFile(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const
{
    Logger::Error("[LibPSDHelper::WriteFileAsCubeMap] PSD writing is not supported");
    return eErrorCode::ERROR_WRITE_FAIL;
}

eErrorCode LibPSDHelper::WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const
{
    Logger::Error("[LibPSDHelper::WriteFileAsCubeMap] PSD writing is not supported");
    return eErrorCode::ERROR_WRITE_FAIL;
}

DAVA::ImageInfo LibPSDHelper::GetImageInfo(File* infile) const
{
    ImageInfo info;

    return info;
}
};
