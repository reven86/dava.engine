#pragma once

#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>

#include "pack_format.h"

struct FileInfo
{
    FileInfo() = default;

    FileInfo(const char* relativePath, uint32_t originalSize,
             uint32_t compressedSize, uint32_t compressionType);

    std::string relativeFilePath;
    uint32_t originalSize = 0;
    uint32_t compressedSize = 0;
    uint32_t compressionType = 0;
    uint32_t hash = 0; // crc32
};

class PackArchive final
{
public:
    explicit PackArchive(const std::string& archiveName);

    const std::vector<FileInfo>& GetFilesInfo() const;

    const FileInfo* GetFileInfo(const std::string& relativeFilePath) const;

    bool HasFile(const std::string& relativeFilePath) const;

    bool
    LoadFile(const std::string& relativeFilePath, std::vector<uint8_t>& output);

    int32_t fileIndex = 0;
    std::string arcName;
    std::string lastFileName;

private:
    std::ifstream file;
    PackFormat::PackFile packFile;
    std::unordered_map<std::string, PackFormat::FileTableEntry*> mapFileData;
    std::vector<FileInfo> filesInfo;
};
