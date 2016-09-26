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

    const String& FindPack(const FilePath& relativeFilePath) const;
    void ListFiles(const String& relativePathDir, const Function<void(const String&, const String&)>& fn);

    void InitializePacks(Vector<IPackManager::Pack>& out) const;

private:
    std::unique_ptr<PacksDBData> data;
};
} // end namespace DAVA