/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

// Next python 2.7 code generate test.db

//import sqlite3
//import os

//action = "create"
//if action == "create":
//os.remove('test.db')

//conn = sqlite3.connect('test.db')
//c = conn.cursor()
//c.execute(
//	'''CREATE TABLE packs (
//	name TEXT PRIMARY KEY NOT NULL,
//	hash text NOT NULL
//)''')
//pack_name = 'unit_test.pak'
//c.execute("INSERT INTO packs VALUES (?, '9bec4b20')", (pack_name, ))
//c.execute("INSERT INTO packs VALUES (?, '0')", ("virtual_test_pack.pak", ))  # empty hash for virtual pack
//conn.commit()
//c.execute(
//	'''CREATE TABLE dependency (
//	key INTEGER PRIMARY KEY, /* autoincrement if NULL inserted */
//	pack TEXT NOT NULL,
//	depends TEXT NOT NULL,
//	FOREIGN KEY(pack) REFERENCES packs(name),
//	FOREIGN KEY(depends) REFERENCES packs(name)
//)''')
//conn.commit()
//c.execute(
//	'''INSERT INTO dependency VALUES (NULL, "virtual_test_pack.pak", "unit_test.pak")'''
//)
//conn.commit()
//c.execute(
//	'''CREATE TABLE files (
//	path TEXT PRIMARY KEY,
//	pack TEXT NOT NULL,
//	FOREIGN KEY(pack) REFERENCES packs(name)
//)''')
//path_to_test_bed = "d:/Users/l_chayka/job/dava.framework/Projects/UnitTests/Data"
//#path_to_test_bed = "d:/Users/l_chayka/job/wot.blitz/Data"
//for path, subdirs, files in os.walk(path_to_test_bed) :
//	for name in files :
//print("name: " + name + " path: " + path + " subdirs: ")
//rel_path = os.path.relpath(os.path.join(path, name), path_to_test_bed)
//rel_path = rel_path.replace("\\", "/")
//c.execute("INSERT INTO files VALUES (?, ?)", (rel_path, pack_name))
//conn.commit()
//elif action == "read":
//conn = sqlite3.connect('test.db')
//c = conn.cursor()
//print("----packs-------------")
//for row in c.execute("SELECT * FROM packs ORDER BY name") :
//	print(row)
//	print("----files-------------")
//	for row in c.execute("SELECT * FROM files ORDER BY path") :
//		print(row)
//		conn.close()



#include "PackManager/Private/PacksDB.h"
#include <sqlite_modern_cpp.h>
#include "MemoryManager/MemoryManager.h"

namespace DAVA
{
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
    PacksDBData(const String& dbPath)
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
#endif // DAVA_MEMORY_PROFILING_ENABLE
        if (FileSystem::Instance()->IsFile(dbPath))
        {
            db.reset(new sqlite::database(dbPath));
        }
        else
        {
            throw std::runtime_error("can't find db file: " + dbPath);
        }
    }
    sqlite::database& GetDB()
    {
        return *db;
    }
    std::unique_ptr<sqlite::database> db;
    FilePath dbPath;
};

PacksDB::PacksDB(const FilePath& filePath)
{
    data.reset(new PacksDBData(filePath.GetAbsolutePathname()));
}

PacksDB::~PacksDB()
{
    data.reset();
}

const String& PacksDB::FindPack(const FilePath& relativeFilePath) const
{
    static String result; // I want return const String, reuse same memory

    result.clear(); // empty string if nothing found

    String relativePath = relativeFilePath.GetRelativePathname("~res:/");
    data->GetDB() << "SELECT pack FROM files WHERE path = ?"
                  << relativePath
    >> [&](std::string packName)
    {
        result = packName;
    };

    return result;
}

void PacksDB::InitializePacks(Vector<PackManager::Pack>& packs) const
{
    packs.clear();

    auto selectQuery = data->GetDB() << "SELECT name, hash FROM packs";

    selectQuery >> [&](String name, String hash)
    {
        PackManager::Pack pack;
        pack.name = name;
        pack.state = PackManager::Pack::NotRequested;

        uint32 crc32 = 0;
        if (!hash.empty())
        {
            StringStream ss;
            ss << std::hex << hash;
            ss >> crc32;
        }

        pack.crc32FromDB = crc32;

        packs.push_back(pack);
    };

    auto selectDependency = data->GetDB() << "SELECT depends FROM dependency WHERE pack = ?";

    for (PackManager::Pack& pack : packs)
    {
        selectDependency << pack.name;
        selectDependency >> [&](String depends_name)
        {
            pack.dependency.push_back(depends_name);
        };

        selectDependency->reset();
    }
}

} // end namespace DAVA