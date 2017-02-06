#include "pack_meta_data.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <tuple>

#include "lz4.h"

static bool lz4_compressor_decompress(const std::vector<uint8_t>& in,
                                      std::vector<uint8_t>& out)
{
    int32_t decompressResult =
    LZ4_decompress_fast(reinterpret_cast<const char*>(in.data()),
                        reinterpret_cast<char*>(out.data()),
                        static_cast<uint32_t>(out.size()));
    if (decompressResult < 0)
    {
        return false;
    }
    return true;
}

pack_meta_data::pack_meta_data(const void* ptr, std::size_t size)
{
    deserialize(ptr, size);
}

uint32_t pack_meta_data::get_num_files() const
{
    return table_files.size();
}

uint32_t pack_meta_data::get_num_packs() const
{
    return table_packs.size();
}

uint32_t pack_meta_data::get_pack_index_for_file(const uint32_t fileIndex) const
{
    return table_files.at(fileIndex);
}

const std::tuple<std::string, std::string>& pack_meta_data::get_pack_info(const uint32_t packIndex) const
{
    return table_packs.at(packIndex);
}

std::vector<uint8_t> pack_meta_data::serialize() const
{
    return std::vector<uint8_t>();
}

struct membuf : std::streambuf
{
    membuf(const void* ptr, size_t size)
    {
        char* begin = const_cast<char*>(static_cast<const char*>(ptr));
        char* end = const_cast<char*>(begin + size);
        setg(begin, begin, end);
    }
};

void pack_meta_data::deserialize(const void* ptr, size_t size)
{
    using namespace std;
    assert(ptr != nullptr);
    assert(size >= 16);

    membuf buf(ptr, size);

    istream file(&buf);

    // 4b header - "meta"
    // 4b num_files
    // num_files b
    // 4b - uncompressed_size
    // 4b - compressed_size
    // compressed_size b
    array<char, 4> header;
    file.read(&header[0], 4);
    if (header != array<char, 4>{ 'm', 'e', 't', 'a' })
    {
        throw runtime_error("read metadata error - not meta");
    }
    uint32_t numFiles = 0;
    file.read(reinterpret_cast<char*>(&numFiles), 4);
    if (!file)
    {
        throw runtime_error("read metadata error - no numFiles");
    }
    table_files.resize(numFiles);

    const uint32_t numFilesBytes = numFiles * 4;
    file.read(reinterpret_cast<char*>(&table_files[0]), numFilesBytes);
    if (!file)
    {
        throw runtime_error("read metadata error - no tableFiles");
    }

    uint32_t uncompressedSize = 0;
    file.read(reinterpret_cast<char*>(&uncompressedSize), 4);
    if (!file)
    {
        throw runtime_error("read metadata error - no uncompressedSize");
    }
    uint32_t compressedSize = 0;
    file.read(reinterpret_cast<char*>(&compressedSize), 4);
    if (!file)
    {
        throw runtime_error("read metadata error - no compressedSize");
    }

    assert(16 + numFilesBytes + compressedSize == size);

    vector<uint8_t> compressedBuf(compressedSize);

    file.read(reinterpret_cast<char*>(&compressedBuf[0]), compressedSize);
    if (!file)
    {
        throw runtime_error("read metadata error - no compressedBuf");
    }

    assert(uncompressedSize >= compressedSize);

    vector<uint8_t> uncompressedBuf(uncompressedSize);

    if (!lz4_compressor_decompress(compressedBuf, uncompressedBuf))
    {
        throw runtime_error("read metadata error - can't decompress");
    }

    const char* startBuf = reinterpret_cast<const char*>(&uncompressedBuf[0]);

    membuf outBuf(startBuf, uncompressedSize);
    istream ss(&outBuf);

    // now parse decompressed packs data line by line (%s %s\n) format
    for (string line, packName, packDependency; getline(ss, line);)
    {
        auto first_space = line.find(' ');
        if (first_space == string::npos)
        {
            throw runtime_error("can't parse packs and dependencies");
        }
        packName = line.substr(0, first_space);
        packDependency = line.substr(first_space + 1);
        table_packs.push_back({ packName, packDependency });
    }
}
