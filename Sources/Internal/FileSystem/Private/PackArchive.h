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

    static void ExtractFileTableData(const PackFormat::PackFile::FooterBlock& footerBlock,
                                     const Vector<char>& tmpBuffer,
                                     String& fileNames,
                                     PackFormat::PackFile::FilesTableBlock& fileTableBlock);

    static void FillFilesInfo(const PackFormat::PackFile& packFile,
                              const String& fileNames,
                              UnorderedMap<String, PackFormat::FileTableEntry*> mapFileData,
                              Vector<ResourceArchive::FileInfo>& filesInfo);

private:
    mutable RefPtr<File> file;
    PackFormat::PackFile packFile;
    UnorderedMap<String, PackFormat::FileTableEntry*> mapFileData;
    Vector<ResourceArchive::FileInfo> filesInfo;
};

} // end namespace DAVA
