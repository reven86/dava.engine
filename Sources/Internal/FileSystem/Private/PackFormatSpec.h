#pragma once

namespace DAVA
{
namespace PackFormat
{
const Array<char8, 4> FileMarker = { 'D', 'V', 'P', 'K' };

struct PackFile
{
    // 0 to N bytes of files - packed contents
    struct PackedFilesBlock
    {
    } notUsedReadDirectlyFromFile;

    // table with info per file in archive (0 to N_files * sizeof(Data))
    struct FilesDataBlock
    {
        struct Data
        {
            uint64 startPositionInPackedFilesBlock;
            uint32 compressed;
            uint32 original;
            uint32 hash;
            Compressor::Type packType;
            Array<uint8, 8> reserved; // null bytes, leave for future
        };
        Vector<Data> files;
    } filesData;

    // 0 to N bytes (all file names concatenated and '\0' separeted and packed)
    // order of file names same as in FilesDataBlock
    struct NamesBlock
    {
        Vector<uint8> sortedNamesLz4hc;
    } names;

    struct HeaderBlock
    {
        // uint64 startNamesBlock = end_of_file - (sizeof(HeaderBlock) + namesBlockSizeOriginal)
        // uint64 startFilesDataBlock = startNamesBlock - fileTableBlockSize
        // uint64 startPackedFilesBlockPosition = 0 // begin_of_the_file
        Array<uint8, 8> reserved; // null bytes (space for future)
        uint32 namesBlockSizeCompressedLZ4HC;
        uint32 namesBlockSizeOriginal;
        uint32 filesTableBlockSize;
        uint32 filesTableHash;
        uint32 numFiles;
        Array<char8, 4> packArchiveMarker;
    } header;
}; // end PackFile struct

using FileTableEntry = PackFile::FilesDataBlock::Data;

static_assert(sizeof(PackFile::HeaderBlock) == 32, "header block size changed, something bad happened!");
static_assert(sizeof(FileTableEntry) == 32, "file table entry size changed, something bad happened!");

} // end of PackFormat namespace

} // end of DAVA namespace
