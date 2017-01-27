#pragma once

#include <cstdint>
#include <string>
#include <vector>

class pack_meta_data
{
public:
    /** Create meta from serialized bytes
		    Throw exception on error
		*/
    pack_meta_data(const void* ptr, std::size_t size);

    uint32_t GetNumFiles() const;
    uint32_t GetNumPacks() const;
    uint32_t GetPackIndexForFile(const uint32_t fileIndex) const;
    const std::tuple<std::string, std::string>& GetPackInfo(const uint32_t packIndex) const;

    std::vector<uint8_t> Serialize() const;
    void Deserialize(const void* ptr, size_t size);

private:
    // fileNames already in DVPK format
    // table 1.
    // fileName -> fileIndex(0-NUM_FILES) -> packIndex(0-NUM_PACKS)
    std::vector<uint32_t> tableFiles;
    // table 2.
    // packIndex(0-NUM_PACKS) -> packName, dependencies
    std::vector<std::tuple<std::string, std::string>> tablePacks;
};
