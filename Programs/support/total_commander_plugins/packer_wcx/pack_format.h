#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#pragma once

namespace PackFormat
{
const std::array<char, 4> FileMarker = { 'D', 'V', 'P', 'K' };

struct PackFile
{
    // 0 to N bytes of files - packed contents
    struct PackedFilesBlock
    {
    } rawBytesOfCompressedFiles;

    struct FilesTableBlock
    {
        // table with info per file in archive (0 to N_files * sizeof(Data))
        struct FilesData
        {
            struct Data
            {
                uint64_t startPosition; // from begin of file
                uint32_t compressedSize;
                uint32_t originalSize;
                uint32_t compressedCrc32;
                uint32_t type;
                uint32_t customUserData; // null bytes, leave for future
            };
            std::vector<Data> files;
        } data;

        // 0 to N bytes (all file names concatenated and '\0' separeted and packed)
        // order of file names same as in FilesData
        struct Names
        {
            std::vector<uint8_t> compressedNames; // lz4hc
            uint32_t compressedCrc32;
        } names;
    } filesTable;

    struct FooterBlock
    {
        std::array<uint8_t, 8> reserved;
        uint32_t metaDataCrc32; // 0 or hash
        uint32_t metaDataSize; // 0 or size of meta data block
        uint32_t infoCrc32;
        struct Info
        {
            uint32_t numFiles;
            uint32_t namesSizeCompressed; // lz4hc
            uint32_t namesSizeOriginal;
            uint32_t filesTableSize;
            uint32_t filesTableCrc32; // hash for both FilesData and FileNames if one file change -> hash will change, or if name of file change -> hash will change too
            std::array<char, 4> packArchiveMarker;
        } info;
    } footer;
}; // end PackFile struct

using FileTableEntry = PackFile::FilesTableBlock::FilesData::Data;

static_assert(sizeof(PackFile::FooterBlock) == 44,
              "header block size changed, something bad happened!");
static_assert(sizeof(FileTableEntry) == 32,
              "file table entry size changed, something bad happened!");

} // end of PackFormat namespace
