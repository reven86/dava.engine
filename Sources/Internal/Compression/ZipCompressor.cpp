#include "Compression/ZipCompressor.h"
#include "FileSystem/File.h"
#include "Logger/Logger.h"
#include "Base/Exception.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif

#define MINIZ_NO_STDIO
#define MINIZ_NO_ARCHIVE_WRITING_APIS

// Disable warning C4334 on VS2015
#if _MSC_VER >= 1900

    #pragma warning(push)
    #pragma warning(disable : 4334)
    #include <miniz/miniz.c>
    #pragma warning(pop)

#else

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif

    #include <miniz/miniz.c>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif

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
    uLong resultLength = destMaxLength;
    int32 result = compress(out.data(), &resultLength, in.data(), static_cast<uLong>(in.size()));
    if (result != Z_OK)
    {
        Logger::Error("can't compress rfc1951 buffer");
        return false;
    }
    out.resize(resultLength);
    return true;
}

bool ZipCompressor::Decompress(const Vector<uint8>& in, Vector<uint8>& out) const
{
    if (in.size() > static_cast<uint32>(std::numeric_limits<uLong>::max()))
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
    RefPtr<File> file{ nullptr };
};

static size_t file_read_func(void* pOpaque, mz_uint64 file_ofs, void* pBuf, size_t n)
{
    File* file = static_cast<File*>(pOpaque);
    if (!file)
    {
        DVASSERT(false && "pOpaque not point to DAVA::File*");
        Logger::Error("nullptr zip archive File object");
        return 0;
    }
    if (!file->Seek(static_cast<uint32>(file_ofs), File::SEEK_FROM_START))
    {
        Logger::Error("can't set seek pos to %d in zip archive file", static_cast<uint32>(file_ofs));
        return 0;
    }

    uint32 result = file->Read(pBuf, static_cast<uint32>(n));
    if (result != n)
    {
        DVASSERT(false && "can't read bytes from zip archive");
        Logger::Error("can't read bytes from zip archive");
    }

    return static_cast<size_t>(result);
}

ZipFile::ZipFile(RefPtr<File>& file_, const FilePath& fileName)
{
    zipData.reset(new ZipPrivateData());

    Memset(&zipData->archive, 0, sizeof(zipData->archive));

    zipData->file = file_;

    if (!zipData->file)
    {
        DAVA_THROW(DAVA::Exception, "can't open archive file: " + fileName.GetStringValue());
    }

    uint64 fileSize = zipData->file->GetSize();

    zipData->archive.m_pIO_opaque = zipData->file.Get();
    zipData->archive.m_pRead = &file_read_func;
    zipData->archive.m_archive_size = fileSize;

    if (mz_zip_reader_init(&zipData->archive, fileSize, 0) == MZ_FALSE)
    {
        DAVA_THROW(DAVA::Exception, "can't init zip from file: " + fileName.GetStringValue());
    }
}

ZipFile::~ZipFile() = default;

uint32 ZipFile::GetNumFiles() const
{
    return mz_zip_reader_get_num_files(&zipData->archive);
}

bool ZipFile::GetFileInfo(uint32 fileIndex, String& fileName, uint32& fileOriginalSize, uint32& fileCompressedSize, bool& isDirectory) const
{
    mz_zip_archive_file_stat fileStat;
    if (!mz_zip_reader_file_stat(&zipData->archive, fileIndex, &fileStat))
    {
        Logger::Error("can't get file status from zip archive: %s", fileName.c_str());
        return false;
    }

    fileName = fileStat.m_filename;
    fileOriginalSize = static_cast<uint32>(fileStat.m_uncomp_size);
    fileCompressedSize = static_cast<uint32>(fileStat.m_comp_size);
    isDirectory = (mz_zip_reader_is_file_a_directory(&zipData->archive, fileIndex) != 0);

    return true;
}

bool ZipFile::LoadFile(const String& fileName, Vector<uint8>& fileContent) const
{
    int32 fileIndex = mz_zip_reader_locate_file(&zipData->archive, fileName.c_str(), nullptr, 0);
    if (fileIndex < 0)
    {
        Logger::Error("file: %s not found in archive: %s!", fileName.c_str(), fileName.c_str());
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
