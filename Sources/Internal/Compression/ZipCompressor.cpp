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
#include "FileSystem/File.h"
#include "Logger/Logger.h"

#include <cstring> // need on android for std::memset

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif

#define MINIZ_NO_STDIO
#define MINIZ_NO_ARCHIVE_WRITING_APIS

#define MINIZ_USE_UNALIGNED_LOADS_AND_STORES 1
#define MINIZ_LITTLE_ENDIAN 1
//#define MINIZ_HAS_64BIT_REGISTERS 1
#include <miniz/miniz.c>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace DAVA
{
bool ZipCompressor::Compress(const Vector<uint8>& in, Vector<uint8>& out) const
{
    uLong destMaxLength = compressBound(static_cast<uint32>(in.size()));
    if (out.size() < destMaxLength)
    {
        out.resize(destMaxLength);
    }
    int32 result = compress(out.data(), &destMaxLength, in.data(), static_cast<uLong>(in.size()));
    if (result != Z_OK)
    {
        Logger::Error("can't compress rfc1951 buffer");
        return false;
    }
    out.resize(destMaxLength);
    return true;
}

bool ZipCompressor::Uncompress(const Vector<uint8>& in, Vector<uint8>& out) const
{
    if (in.size() > static_cast<uint32>(std::numeric_limits<int32>::max()))
    {
        Logger::Error("too big input buffer for uncompress rfc1951");
        return false;
    }
    uLong uncompressedSize = static_cast<uLong>(out.size());
    int32 decompressResult = uncompress(out.data(), &uncompressedSize, in.data(), static_cast<uLong>(in.size()));
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
    ScopedPtr<File> file{ nullptr };
};

static size_t file_read_func(void* pOpaque, mz_uint64 file_ofs, void* pBuf, size_t n)
{
    Logger::Error("Start file_read_func");
    File* file = static_cast<File*>(pOpaque);
    if (!file)
    {
        DVASSERT(false && "can't happen");
        Logger::Error("nullptr zip archive File object");
        return 0;
    }
    Logger::Error("Start file_read_func before seek");
    if (!file->Seek(static_cast<uint32>(file_ofs), File::SEEK_FROM_START))
    {
        Logger::Error("can't set seek pos to %d in zip archive file", static_cast<uint32>(file_ofs));
        return 0;
    }
    Logger::Error("Start file_read_func before reed");
    uint32 result = file->Read(pBuf, static_cast<uint32>(n));

    if (result != n)
    {
        Logger::Error("can't read bytes from zip archive");
    }

    return static_cast<size_t>(result);
}

ZipFile::ZipFile(const FilePath& fileName)
{
    Logger::Error("phase open zip_file");
    zipData.reset(new ZipPrivateData());

    std::memset(&zipData->archive, 0, sizeof(zipData->archive));

    zipData->file.reset(File::Create(fileName, File::OPEN | File::READ));

    if (!zipData->file)
    {
        Logger::Error("phase before throw exception 1");
        throw std::runtime_error("can't open archive file: " + fileName.GetAbsolutePathname());
    }

    uint32 fileSize = zipData->file->GetSize();

    zipData->archive.m_pIO_opaque = zipData->file.get();
    zipData->archive.m_pRead = &file_read_func;
    zipData->archive.m_archive_size = fileSize;

    if (mz_zip_reader_init(&zipData->archive, fileSize, 0) == 0)
    {
        Logger::Error("phase before throw exception 1");
        throw std::runtime_error("can't init zip from file: " + fileName.GetAbsolutePathname());
    }

    String fName = fileName.GetAbsolutePathname();
    zipData->fileName = fName;
    Logger::Error("phase finish zip_file");
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
    Logger::Error("Start GetFileInfo");
    mz_zip_archive_file_stat fileStat;
    if (!mz_zip_reader_file_stat(&zipData->archive, fileIndex, &fileStat))
    {
        Logger::Error("phase inside ZipFile exception");
        Logger::Error("mz_zip_reader_file_stat() failed!");
        return false;
    }
    Logger::Error("after file_stat");
    fileName = fileStat.m_filename;
    fileOriginalSize = static_cast<uint32>(fileStat.m_uncomp_size);
    fileCompressedSize = static_cast<uint32>(fileStat.m_comp_size);
    isDirectory = (mz_zip_reader_is_file_a_directory(&zipData->archive, fileIndex) != 0);
    Logger::Error("phase inside ZipFile finish GetFileInfo");
    return true;
}

bool ZipFile::LoadFile(const String& fileName, Vector<uint8>& fileContent) const
{
    int32 fileIndex = mz_zip_reader_locate_file(&zipData->archive, fileName.c_str(), nullptr, 0);
    if (fileIndex < 0)
    {
        Logger::Error("file: %s not found in archive: %s!", fileName.c_str(), zipData->fileName.c_str());
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
        fileContent.resize(static_cast<size_t>(fileStat.m_uncomp_size));
    }

    mz_bool result = mz_zip_reader_extract_file_to_mem(&zipData->archive, fileName.c_str(), fileContent.data(), fileContent.size(), 0);
    if (result == 0)
    {
        Logger::Error("can't extract file: %s into memory", fileName.c_str());
        return false;
    }
    return true;
}

} // end namespace DAVA
