#include "PackManager/Private/PacksDB.h"
#include <sqlite_modern_cpp.h>
#include "MemoryManager/MemoryManager.h"
#include "FileSystem/FileSystem.h"
#include "PackManager/Private/VirtualFileSystemSqliteWraper.h"

namespace DAVA
{
// TODO anable later ask max
#if defined(_WIN64) && defined(DAVA_MEMORY_PROFILING_ENABLE)
#undef DAVA_MEMORY_PROFILING_ENABLE
#endif

#ifdef DAVA_MEMORY_PROFILING_ENABLE
static void* SqliteMalloc(int32 size)
{
    return MemoryManager::Instance()->Allocate(static_cast<size_t>(size), ALLOC_POOL_SQLITE);
}
static void SqliteFree(void* ptr)
{
    MemoryManager::Instance()->Deallocate(ptr);
}
static void* SqliteRealloc(void* ptr, int32 size)
{
    return MemoryManager::Instance()->Reallocate(ptr, static_cast<size_t>(size));
}
static int32 SqliteMemSize(void* ptr)
{
    return static_cast<int32>(MemoryManager::Instance()->MemorySize(ptr));
}
static int32 SqliteRoundUp(int32 size)
{
    return size + (16 - (size & (16 - 1)));
}
static int32 SqliteMemInit(void* pAppData)
{
    return 0;
}
static void SqliteMemShutdown(void* pAppData)
{
    // do nothing
}
#endif // DAVA_MEMORY_PROFILING_ENABLE

class PacksDBData
{
public:
    PacksDBData(const String& dbPath, bool dbInMemory)
    {
#ifdef DAVA_MEMORY_PROFILING_ENABLE
        sqlite3_mem_methods mem = {
            &SqliteMalloc,
            &SqliteFree,
            &SqliteRealloc,
            &SqliteMemSize,
            &SqliteRoundUp,
            &SqliteMemInit,
            &SqliteMemShutdown
        };
        int32 result = sqlite3_config(SQLITE_CONFIG_MALLOC, &mem);
        DVVERIFY(result == SQLITE_OK);
#endif // DAVA_MEMORY_PROFILING_ENABLE

        RegisterDavaVFSForSqlite3(dbInMemory);

        if (FileSystem::Instance()->IsFile(dbPath))
        {
            db.reset(new sqlite::database(dbPath));
        }
        else
        {
            throw std::runtime_error("can't find db file: " + dbPath);
        }
    }
    ~PacksDBData()
    {
        UnregisterDavaVFSForSqlite3();
    }
    sqlite::database& GetDB()
    {
        return *db;
    }
    std::unique_ptr<sqlite::database> db;
    FilePath dbPath;
};

PacksDB::PacksDB(const FilePath& filePath, bool dbInMemory)
{
    data.reset(new PacksDBData(filePath.GetAbsolutePathname(), dbInMemory));
}

PacksDB::~PacksDB() = default;

const String& PacksDB::FindPack(const FilePath& relativeFilePath) const
{
    static String result; // I want return const String, reuse same memory

    result.clear(); // empty string if nothing found

    String relativePath;
    if (relativeFilePath.GetType() == FilePath::PATH_IN_FILESYSTEM)
    {
        relativePath = relativeFilePath.GetRelativePathname();
    }
    else if (relativeFilePath.GetType() == FilePath::PATH_IN_RESOURCES)
    {
        relativePath = relativeFilePath.GetRelativePathname("~res:/");
    }

    data->GetDB() << "SELECT pack FROM files WHERE path = ?"
                  << relativePath
    >> [&](std::string packName)
    {
        result = packName;
    };

    return result;
}

void PacksDB::ListFiles(const String& relativePathDir, const Function<void(const String&, const String&)>& fn)
{
    try
    {
        data->GetDB() << "SELECT path, pack FROM files WHERE path LIKE ?"
                      << relativePathDir + "%"
        >> [&](std::string path, std::string pack)
        {
            fn(path, pack);
        };
    }
    catch (sqlite::sqlite_exception& ex)
    {
        Logger::Error("error while executing query to DB ListFiles: %s", ex.what());
    }
}

void PacksDB::InitializePacks(Vector<IPackManager::Pack>& packs) const
{
    packs.clear();
    packs.reserve(911); // now we have 911 packs

    try
    {
        auto selectQuery = data->GetDB() << "SELECT name, hash, is_gpu, size, dependency FROM packs";

        selectQuery >> [&](String name, String hash, int32 isGpu, int32 size, String dependency)
        {
            IPackManager::Pack pack;
            pack.name = name;
            pack.state = IPackManager::Pack::Status::NotRequested;
            pack.isGPU = (isGpu == 1);
            pack.totalSizeFromDB = static_cast<uint32>(size);

            uint32 crc32 = 0;
            if (!hash.empty())
            {
                StringStream ss;
                ss << std::hex << hash;
                ss >> crc32;
            }

            pack.hashFromDB = crc32;

            if (!dependency.empty())
            {
                std::istringstream iss(dependency);
                String depPackName;
                while (getline(iss, depPackName, ' '))
                {
                    pack.dependency.push_back(depPackName);
                }
            }

            packs.push_back(pack);
        };
    }
    catch (std::exception& ex)
    {
        throw std::runtime_error("DB error, update local DB for pack manager: " + data->dbPath.GetStringValue() + " cause: " + ex.what());
    }
}

} // end namespace DAVA