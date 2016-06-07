#pragma once

#include <array>
#include <vector>
#include <string>
#include <cstdint>

namespace PackFormat
{
const std::array<char, 4> FileMarker = { 'P', 'A', 'C', 'K' };

struct PackFile
{
    struct HeaderBlock
    {
        std::array<char, 4> marker;
        uint32_t namesBlockSizeCompressedLZ4HC;
        uint32_t namesBlockSizeOriginal;
        uint32_t filesTableBlockSize;
        uint32_t startNamesBlockPosition;
        uint32_t startFilesDataBlockPosition;
        uint32_t startPackedFilesBlockPosition;
        uint32_t numFiles;
    } header;

    struct NamesBlock
    {
        std::string sortedNamesLz4hc; // '\0' separated all file names in pack file compressed with lz4hc
    } names;

    struct FilesDataBlock
    {
        struct Data
        {
            uint32_t startPositionInPackedFilesBlock;
            uint32_t compressed;
            uint32_t original;
            uint32_t packType; //Compressor::Type packType;
            std::array<char, 16> reserved; // do we really need it? leave for future
        };
        std::vector<Data> files;
    } filesData;

    struct PackedFilesBlock
    {
        uint8_t* packedFiles;
    } notUsedReadDirectlyFromFile;
}; // end PackFile struct

using FileTableEntry = PackFile::FilesDataBlock::Data;

static_assert(sizeof(PackFile::HeaderBlock) == 32,
              "header block size changed, something bad happened!");
static_assert(sizeof(FileTableEntry) == 32,
              "file table entry size changed, something bad happened!");

} // end of PackFormat namespace
