#include "FileSystem/ResourceArchive.h"
#include "FileSystem/Private/ZipArchive.h"
#include "FileSystem/Private/PackArchive.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"

//     +---------------+       +-------------------+
//     |ResourceArchive+-------+ResourceArchiveImpl|
//     +---------------+       +-------------------+
//                          +----^   ^------+
//                          |               |
//                          |               |
//                          |               |
//                    +-----+-----+   +-----+----+
//                    |DavaArchive|   |ZipArchive|
//                    +-----------+   +----------+

namespace DAVA
{
ResourceArchive::ResourceArchive(const FilePath& archiveName)
{
    Logger::Info("line: start: %s, %d", __FUNCTION__, __LINE__);

    const String& fileName = archiveName.GetAbsolutePathname();

    Logger::Info("fileName: %s, %d, fileName: ", __FUNCTION__, __LINE__, fileName.c_str());

    ScopedPtr<File> f(File::Create(fileName, File::OPEN | File::READ));

    Logger::Info("f: %s, %d 0x%X", __FUNCTION__, __LINE__, f.get());

    if (!f)
    {
        Logger::Info("line: %s, %d", __FUNCTION__, __LINE__);
        throw std::runtime_error("can't open resource archive: " + fileName);
    }

    Logger::Info("line: %s, %d", __FUNCTION__, __LINE__);

    Array<char8, 4> lastFourBytes;

    Logger::Info("line: %s, %d", __FUNCTION__, __LINE__);

    uint64 fSize = f->GetSize();

    if (fSize < 4)
    {
        throw std::runtime_error("file not dvpk and not zip archive: " + fileName);
    }

    if (!f->Seek(fSize - 4, File::SEEK_FROM_START))
    {
        Logger::Info("line: %s, %d", __FUNCTION__, __LINE__);
        throw std::runtime_error("can't seek to last 4 bytes DVPK marker");
    }

    Logger::Info("line: %s, %d", __FUNCTION__, __LINE__);

    uint32 count = f->Read(lastFourBytes.data(), static_cast<uint32>(lastFourBytes.size()));

    Logger::Info("line: %s, %d %c, %c, %c, %c", __FUNCTION__, __LINE__, lastFourBytes[0], lastFourBytes[1],
                 lastFourBytes[2], lastFourBytes[3]);

    if (count != lastFourBytes.size())
    {
        Logger::Info("line: %s, %d", __FUNCTION__, __LINE__);

        throw std::runtime_error("can't read from resource archive: " + fileName);
    }

    Logger::Info("line: %s, %d", __FUNCTION__, __LINE__);

    f.reset();

    Logger::Info("line: %s, %d", __FUNCTION__, __LINE__);

    if (PackFormat::FileMarker == lastFourBytes)
    {
        Logger::Info("line: %s, %d", __FUNCTION__, __LINE__);

        impl.reset(new PackArchive(fileName));
    }
    else
    {
        Logger::Info("line: %s, %d", __FUNCTION__, __LINE__);

        impl.reset(new ZipArchive(fileName));
    }
}

ResourceArchive::~ResourceArchive() = default;

ResourceArchive::FileInfo::FileInfo(const char8* relativePath_,
                                    uint32 originalSize_,
                                    uint32 originalCrc32_,
                                    uint32 compressedSize_,
                                    uint32 compressedCrc32_,
                                    Compressor::Type compressionType_)
    : relativeFilePath(relativePath_)
    , originalSize(originalSize_)
    , originalCrc32(originalCrc32_)
    , compressedSize(compressedSize_)
    , compressedCrc32(compressedCrc32_)
    , compressionType(compressionType_)
{
}

const Vector<ResourceArchive::FileInfo>& ResourceArchive::GetFilesInfo() const
{
    return impl->GetFilesInfo();
}

bool ResourceArchive::HasFile(const String& relativeFilePath) const
{
    return impl->HasFile(relativeFilePath);
}

const ResourceArchive::FileInfo* ResourceArchive::GetFileInfo(const String& relativeFilePath) const
{
    return impl->GetFileInfo(relativeFilePath);
}

bool ResourceArchive::LoadFile(const String& relativeFilePath,
                               Vector<uint8>& output) const
{
    return impl->LoadFile(relativeFilePath, output);
}

bool ResourceArchive::UnpackToFolder(const FilePath& dir) const
{
    Vector<uint8> content;

    for (auto& res : impl->GetFilesInfo())
    {
        FilePath filePath = dir + res.relativeFilePath;
        FilePath directory = filePath.GetDirectory();
        FileSystem::Instance()->CreateDirectory(directory, true);

        if (!LoadFile(res.relativeFilePath, content))
        {
            Logger::Error("can't unpack file: %s", res.relativeFilePath.c_str());
            return false;
        }

        ScopedPtr<File> file(File::Create(filePath, File::WRITE | File::CREATE));
        if (!file)
        {
            Logger::Error("can't open file: %s", filePath.GetStringValue().c_str());
            return false;
        }
        uint32 bytesInFile = file->Write(content.data(), static_cast<uint32>(content.size()));
        if (bytesInFile != content.size())
        {
            Logger::Error("can't write file: %s", res.relativeFilePath.c_str());
            return false;
        }
    }
    return false;
}

} // end namespace DAVA
