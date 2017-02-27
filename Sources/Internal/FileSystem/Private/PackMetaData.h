#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class FilePath;

class PackMetaData
{
public:
    /** Create meta from sqlite db file
	    open DB and read meta data vector
		Throw DAVA::Exception exception on error
		*/
    explicit PackMetaData(const FilePath& metaDb);
    /** Create meta from serialized bytes
		    Throw exception on error
		*/
    PackMetaData(const void* ptr, std::size_t size);

    Vector<String> GetDependenciesNames(const String& requestedPackName) const;

    Vector<uint32> GetFileIndexes(const String& requestedPackName) const;

    uint32 GetPackIndexForFile(const uint32 fileIndex) const;

    size_t GetNumTotalFiles() const
    {
        return packIndexes.size();
    }

    size_t GetNumTotalPacks() const
    {
        return packDependencies.size();
    }

    struct PackInfo
    {
        String packName;
        String packDependencies;
    };
    /**
	    Return tuple (packName, packDependencies)
	*/
    const PackInfo& GetPackInfo(const uint32 packIndex) const;
    const PackInfo& GetPackInfo(const String& packName) const;

    Vector<uint8> Serialize() const;
    void Deserialize(const void* ptr, size_t size);

private:
    // fileNames already in DVPK format
    // table 1.
    // fileName -> fileIndex(0-NUM_FILES) -> packIndex(0-NUM_PACKS)
    Vector<uint32> packIndexes;
    // table 2.
    // packIndex(0-NUM_PACKS) -> packName, dependencies
    Vector<PackInfo> packDependencies;
};

} // end namespace DAVA