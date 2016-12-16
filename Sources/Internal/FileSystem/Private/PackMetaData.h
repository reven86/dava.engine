#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class FilePath;

class PackMetaData
{
public:
    /** Create meta from sqlite db file
		    Throw exception on error
		*/
    explicit PackMetaData(const FilePath& metaDb);
    /** Create meta from serialized bytes
		    Throw exception on error
		*/
    PackMetaData(const void* ptr, std::size_t size);

    uint32 GetPackIndexForFile(const String& relativeFilePath) const;
    void GetPackInfo(const uint32 packIndex, String& packName, String& dependencies) const;

    Vector<char> Serialize() const;

private:
    // TODO fileNames already in DVPK format optimize
    // TODO
    // table 1.
    // fileName -> fileIndex | packIndex
    Vector<std::tuple<uint32, uint32>> tableFiles;
    // table 2.
    // packIndex | packName | dependencies
    Vector<std::tuple<uint32, String, String>> tablePacks;
    // TODO optimization
    // how to pack fileNames?
    // 3d/gfx/tank/t-34/tex.pvr ->
};

} // end namespace DAVA