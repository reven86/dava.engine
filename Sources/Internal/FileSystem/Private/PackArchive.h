#pragma once

#include "FileSystem/Private/ResourceArchivePrivate.h"
#include "FileSystem/Private/PackFormatSpec.h"
#include "FileSystem/File.h"

namespace DAVA
{
class PackArchive final : public ResourceArchiveImpl
{
public:
    explicit PackArchive(const FilePath& archiveName);

    const Vector<ResourceArchive::FileInfo>& GetFilesInfo() const override;
    const ResourceArchive::FileInfo* GetFileInfo(const String& relativeFilePath) const override;
    bool HasFile(const String& relativeFilePath) const override;
    bool LoadFile(const String& relativeFilePath, Vector<uint8>& output) const override;

private:
    mutable RefPtr<File> file;
    PackFormat::PackFile packFile;
    UnorderedMap<String, PackFormat::FileTableEntry*> mapFileData;
    Vector<ResourceArchive::FileInfo> filesInfoSortedByName;
};

} // end namespace DAVA
