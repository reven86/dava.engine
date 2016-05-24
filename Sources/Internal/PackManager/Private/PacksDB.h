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
    PacksDB(const FilePath& filePath);
    ~PacksDB();

    const String& FindPack(const FilePath& relativeFilePath) const;

    void InitializePacks(Vector<PackManager::Pack>& out) const;

private:
    std::unique_ptr<PacksDBData> data;
};
} // end namespace DAVA