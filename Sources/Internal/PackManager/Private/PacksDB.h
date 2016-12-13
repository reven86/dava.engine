#pragma once

#include "Base/BaseTypes.h"
#include "PackManager/PackManager.h"

namespace DAVA
{
class FilePath;
class PacksDBData;

class PacksDB final
{
public:
    PacksDB(const FilePath& filePath, bool dbInMemory);
    ~PacksDB();

    void CollectDependentPackNames(const String& packName, Vector<String>& dependencies);
    void CollectFilenamesForPack(const String& packName, Vector<String>& pathNames);

private:
    std::unique_ptr<PacksDBData> data;
};
} // end namespace DAVA