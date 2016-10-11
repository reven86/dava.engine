#pragma once

#include "Compression/Compressor.h"

namespace DAVA
{
namespace PackFormat
{
const Array<char8, 4> FileMarker = { { 'D', 'V', 'P', 'K' } };

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
                uint64 startPosition; // from begin of file
                uint32 compressedSize;
                uint32 originalSize;
                uint32 compressedCrc32;
                Compressor::Type type;
                uint32 originalCrc32;
                Array<char8, 4> reserved; // null bytes, leave for future
            };

            Vector<Data> files;
        } data;

        // 0 to N bytes (all file names concatenated and '\0' separeted and packed)
        // order of file names same as in FilesData
        struct Names
        {
            Vector<uint8> compressedNames; // lz4hc
            uint32 compressedCrc32;
        } names;
    } filesTable;

    struct FooterBlock
    {
        Array<char, 4> reserved; // null bytes (space for future)
        uint32 infoCrc32;
        struct Info
        {
            uint32 numFiles;
            uint32 namesSizeCompressed; // lz4hc
            uint32 namesSizeOriginal;
            uint32 filesTableSize;
            uint32 filesTableCrc32; // hash for both FilesData and FileNames if one file change -> hash will change, or if name of file change -> hash will change too
            Array<char8, 4> packArchiveMarker;
        } info;
    } footer;
}; // end PackFile struct

using FileTableEntry = PackFile::FilesTableBlock::FilesData::Data;

static_assert(sizeof(PackFile::FooterBlock) == 32, "header block size changed, something bad happened!");
static_assert(sizeof(FileTableEntry) == 32, "file table entry size changed, something bad happened!");

} // end of PackFormat namespace

} // end of DAVA namespace
