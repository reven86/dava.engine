#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class FilePath;

class PackMetaData
{
public:
    /** Create meta from sqlite db file
	    open DB and read and fill tableFiles vector
		and tablePacks vector
		    Throw exception on error
		*/
    explicit PackMetaData(const FilePath& metaDb);
    /** Create meta from serialized bytes
		    Throw exception on error
		*/
    PackMetaData(const void* ptr, std::size_t size);

    uint32 GetPackIndexForFile(const uint32 fileIndex) const;
    void GetPackInfo(const uint32 packIndex, String& packName, String& dependencies) const;

    Vector<uint8> Serialize() const;
    void Deserialize(const void* ptr, size_t size);

private:
    // fileNames already in DVPK format
    // table 1.
    // fileName -> fileIndex(0-NUM_FILES) -> packIndex(0-NUM_PACKS)
    Vector<uint32> tableFiles;
    // table 2.
    // packIndex(0-NUM_PACKS) -> packName, dependencies
    Vector<std::tuple<String, String>> tablePacks;
};

} // end namespace DAVA