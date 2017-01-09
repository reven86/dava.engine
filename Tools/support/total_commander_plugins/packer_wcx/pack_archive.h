#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "pack_format.h"
#include "pack_meta_data.h"

class PackArchive final
{
public:
    explicit PackArchive(const std::string& archiveName);

    const std::vector<FileInfo>& GetFilesInfo() const;

    const FileInfo* GetFileInfo(const std::string& relativeFilePath) const;

    bool HasFile(const std::string& relativeFilePath) const;

    bool
    LoadFile(const std::string& relativeFilePath, std::vector<uint8_t>& output);

    bool HasMeta() const;

    const PackMetaData& GetMeta() const;

    std::string PrintMeta() const;

    int32_t fileIndex = 0;
    std::string arcName;
    std::string lastFileName;

private:
    std::ifstream file;
    PackFormat::PackFile packFile;
    std::unique_ptr<PackMetaData> packMeta;
    std::unordered_map<std::string, PackFormat::FileTableEntry*> mapFileData;
    std::vector<FileInfo> filesInfo;
};
