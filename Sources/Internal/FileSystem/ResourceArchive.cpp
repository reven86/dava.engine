#include "FileSystem/ResourceArchive.h"
#include "FileSystem/Private/ZipArchive.h"
#include "FileSystem/Private/PackArchive.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"

#include <fstream>

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
    const String& fileName = archiveName.GetAbsolutePathname();

    ScopedPtr<File> f(File::Create(fileName, File::OPEN | File::READ));
    if (!f)
    {
        throw std::runtime_error("can't open resource archive: " + fileName);
    }
    Array<char8, 4> firstBytes;
    uint32 count = f->Read(firstBytes.data(), static_cast<uint32>(firstBytes.size()));
    if (count != firstBytes.size())
    {
        throw std::runtime_error("can't read from resource archive: " + fileName);
    }

    f.reset();

    if (PackFormat::FileMarker == firstBytes)
    {
        impl.reset(new PackArchive(fileName));
    }
    else
    {
        impl.reset(new ZipArchive(fileName));
    }
}

ResourceArchive::~ResourceArchive() = default;

ResourceArchive::FileInfo::FileInfo(const char8* relativePath_, uint32 originalSize_, uint32 compressedSize_, Compressor::Type compressionType_)
    : relativeFilePath(relativePath_)
    , originalSize(originalSize_)
    , compressedSize(compressedSize_)
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
