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

#include "Compression/ZipCompressor.h"
#include "Logger/Logger.h"

#include <miniz/miniz.c>

namespace DAVA
{
bool ZipCompressor::Compress(const Vector<char8>& in, Vector<char8>& out) const
{
    uLong destMaxLength = compressBound(in.size());
    if (out.size() < destMaxLength)
    {
        out.resize(destMaxLength);
    }
    int result = compress(reinterpret_cast<uint8*>(out.data()), &destMaxLength, reinterpret_cast<const uint8*>(in.data()), in.size());
    if (result != Z_OK)
    {
        Logger::Error("can't compress rfc1951 buffer");
        return false;
    }
    out.resize(destMaxLength);
    return true;
}

bool ZipCompressor::Uncompress(const Vector<char8>& in, Vector<char8>& out) const
{
    if (in.size() > std::numeric_limits<int32>::max())
    {
        Logger::Error("too big input buffer for uncompress rfc1951");
        return false;
    }
    uLong uncompressedSize = out.size();
    int decompressResult = uncompress(reinterpret_cast<uint8*>(out.data()), &uncompressedSize, reinterpret_cast<const uint8*>(in.data()), in.size());
    if (decompressResult != Z_OK)
    {
        Logger::Error("can't uncompress rfc1951 buffer");
        return false;
    }
    out.resize(uncompressedSize);
    return true;
}

class ZipPrivateData
{
public:
    mz_zip_archive archive;
    String fileName;
};

ZipFile::ZipFile(const FilePath& file)
{
    zipData.reset(new ZipPrivateData());

    std::memset(&zipData->archive, 0, sizeof(zipData->archive));

    String fileName = file.GetAbsolutePathname();
    zipData->fileName = fileName;

    mz_bool status = mz_zip_reader_init_file(&zipData->archive, fileName.c_str(), 0);
    if (!status)
    {
        throw std::runtime_error("mz_zip_reader_init_file() failed!");
    }
}

ZipFile::~ZipFile()
{
    zipData.reset();
}

uint32 ZipFile::GetNumFiles() const
{
    return mz_zip_reader_get_num_files(&zipData->archive);
}

bool ZipFile::GetFileInfo(uint32 fileIndex, String& fileName, uint32& fileOriginalSize, uint32& fileCompressedSize, bool& isDirectory) const
{
    mz_zip_archive_file_stat fileStat;
    if (!mz_zip_reader_file_stat(&zipData->archive, fileIndex, &fileStat))
    {
        Logger::Error("mz_zip_reader_file_stat() failed!");
        return false;
    }
    fileName = fileStat.m_filename;
    fileOriginalSize = fileStat.m_uncomp_size;
    fileCompressedSize = fileStat.m_comp_size;
    isDirectory = (mz_zip_reader_is_file_a_directory(&zipData->archive, fileIndex) != 0);
    return true;
}

bool ZipFile::LoadFile(const FilePath& fileName, Vector<int8>& fileContent) const
{
    String name = fileName.GetStringValue();

    int fileIndex = mz_zip_reader_locate_file(&zipData->archive, name.c_str(), nullptr, 0);
    if (fileIndex < 0)
    {
        Logger::Error("file: %s not found in archive: %s!", name.c_str(), zipData->fileName.c_str());
        return false;
    }

    mz_zip_archive_file_stat fileStat;
    if (!mz_zip_reader_file_stat(&zipData->archive, fileIndex, &fileStat))
    {
        Logger::Error("mz_zip_reader_file_stat() failed!");
        return false;
    }

    if (fileContent.size() != fileStat.m_uncomp_size)
    {
        fileContent.resize(fileStat.m_uncomp_size);
    }

    mz_bool result = mz_zip_reader_extract_file_to_mem(&zipData->archive, name.c_str(), fileContent.data(), fileContent.size(), 0);
    if (result == 0)
    {
        Logger::Error("can't extract file: %s into memory", name.c_str());
        return false;
    }
    return true;
}

} // end namespace DAVA
