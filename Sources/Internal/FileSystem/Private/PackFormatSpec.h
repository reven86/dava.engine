#pragma once

namespace DAVA
{
namespace PackFormat
{
const Array<char8, 4> FileMarker = { 'P', 'A', 'C', 'K' };

struct PackFile
{
    struct HeaderBlock
    {
        Array<char8, 4> marker;
        uint32 namesBlockSizeCompressedLZ4HC;
        uint32 namesBlockSizeOriginal;
        uint32 filesTableBlockSize;
        uint32 startNamesBlockPosition;
        uint32 startFilesDataBlockPosition;
        uint32 startPackedFilesBlockPosition;
        uint32 numFiles;
    } header;

    struct NamesBlock
    {
        Vector<uint8> sortedNamesLz4hc; // '\0' separated all file names in pack file compressed with lz4hc
    } names;

    struct FilesDataBlock
    {
        struct Data
        {
            uint32 startPositionInPackedFilesBlock;
            uint32 compressed;
            uint32 original;
            Compressor::Type packType;
            Array<uint8, 16> reserved; // do we really need it? leave for future
        };
        Vector<Data> files;
    } filesData;

    struct PackedFilesBlock
    {
        uint8* packedFiles;
    } notUsedReadDirectlyFromFile;
}; // end PackFile struct

using FileTableEntry = PackFile::FilesDataBlock::Data;

static_assert(sizeof(PackFile::HeaderBlock) == 32, "header block size changed, something bad happened!");
static_assert(sizeof(FileTableEntry) == 32, "file table entry size changed, something bad happened!");

} // end of PackFormat namespace

} // end of DAVA namespace
