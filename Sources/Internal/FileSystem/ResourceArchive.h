#pragma once

#include "Compression/Compressor.h"

namespace DAVA
{
class ResourceArchiveImpl;

class FilePath;

class ResourceArchive final
{
public:
    explicit ResourceArchive(const FilePath& filePath);
    ~ResourceArchive();

    struct FileInfo
    {
        FileInfo() = default;
        FileInfo(const char8* relativePath, uint32 originalSize, uint32 compressedSize, Compressor::Type compressionType);

        String relativeFilePath;
        uint32 originalSize = 0;
        uint32 compressedSize = 0;
        Compressor::Type compressionType = Compressor::Type::None;
    };

    const Vector<FileInfo>& GetFilesInfo() const;
    const FileInfo* GetFileInfo(const String& relativeFilePath) const;
    bool HasFile(const String& relativeFilePath) const;
    bool LoadFile(const String& relativeFilePath, Vector<uint8>& outputFileContent) const;

    bool UnpackToFolder(const FilePath& dir) const;

private:
    std::unique_ptr<ResourceArchiveImpl> impl;
};
} // end namespace DAVA
