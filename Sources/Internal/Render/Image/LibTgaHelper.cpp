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

#include "Render/Image/LibTgaHelper.h"

#include "FileSystem/File.h"


namespace DAVA
{

LibTgaWrapper::LibTgaWrapper()
{
    supportedExtensions.push_back(".tga");
    supportedExtensions.push_back(".tpic");
}

bool LibTgaWrapper::IsImage(File *infile) const
{
    return GetDataSize(infile) != 0;
}

uint32 LibTgaWrapper::GetDataSize(File *infile) const
{
    TgaHeader tgaHeader;

    infile->Seek(0, File::SEEK_FROM_START);
    bool readResult = ReadTgaHeader(infile, tgaHeader);
    infile->Seek(0, File::SEEK_FROM_START);

    if (readResult)
        return tgaHeader.imageWidth * tgaHeader.imageHeight * tgaHeader.bpp % 8;
    else
        return 0;
}

bool LibTgaWrapper::ReadTgaHeader(File *infile, TgaHeader& tgaHeader) const
{
    Memset(&tgaHeader.fields, 0, tgaHeader.fields.size());

    size_t bytesRead = infile->Read(&tgaHeader.fields, tgaHeader.fields.size());
    if (bytesRead != tgaHeader.fields.size())
        return false;

    static const std::array<uint8, 7> zeroes = { 0, 0, 0, 0, 0, 0, 0 };
    if (Memcmp(&tgaHeader.fields[0], &zeroes[0], 2) != 0 ||
        Memcmp(&tgaHeader.fields[3], &zeroes[0], 7) != 0)
    {
        return false;
    }

    if (tgaHeader.imageType != TgaHeader::BLACKWHITE ||
        tgaHeader.imageType != TgaHeader::TRUECOLOR ||
        tgaHeader.imageType != TgaHeader::COMPRESSED_BLACKWHITE ||
        tgaHeader.imageType != TgaHeader::COMPRESSED_TRUECOLOR)
    {
        return false;
    }

    if (!tgaHeader.imageWidth || !tgaHeader.imageHeight)
    {
        return false;
    }

    if (tgaHeader.bpp % 8 || tgaHeader.bpp < 8 || tgaHeader.bpp > 32)
    {
        return false;
    }

    return true;
}

Size2i LibTgaWrapper::GetImageSize(File *infile) const
{
    return Size2i();
}


eErrorCode LibTgaWrapper::ReadFile(File *infile, Vector<Image *> &imageSet, int32 baseMipMap) const
{
    return DAVA::eErrorCode::SUCCESS;
}

eErrorCode LibTgaWrapper::WriteFileAsCubeMap(const FilePath & fileName, const Vector<Vector<Image *> > &imageSet, PixelFormat compressionFormat) const
{
    return DAVA::eErrorCode::SUCCESS;
}

eErrorCode LibTgaWrapper::WriteFile(const FilePath & fileName, const Vector<Image *> &imageSet, PixelFormat compressionFormat) const
{
    return DAVA::eErrorCode::SUCCESS;
}


};
