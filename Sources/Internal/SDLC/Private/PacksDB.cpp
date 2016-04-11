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

//import sqlite3
//import os
//
//action = "read"
//if action == "create":
//os.remove('test.db')
//
//conn = sqlite3.connect('test.db')
//c = conn.cursor()
//c.execute(
//    '''CREATE TABLE packs (
//    name TEXT PRIMARY KEY NOT NULL,
//    status INTEGER,
//    priority REAL,
//    download REAL,
//    hash text
//    )''')
//    pack_name = 'unit_test.pak'
//    c.execute("INSERT INTO packs VALUES (?, 0, 0.5, 0.0, '0x12345678')", (pack_name, ))
//    conn.commit()
//    c.execute(
//        '''CREATE TABLE files (
//        path TEXT PRIMARY KEY,
//        pack_name TEXT,
//        FOREIGN KEY(pack_name) REFERENCES packs(name)
//        )''')
//    #path_to_test_bed = "d:/Users/l_chayka/job/dava.framework/Projects/UnitTests/Data"
//    path_to_test_bed = "d:/Users/l_chayka/job/wot.blitz/Data"
//    for path, subdirs, files in os.walk(path_to_test_bed) :
//        for name in files :
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
//    print(row)
//    print("----files-------------")
//    for row in c.execute("SELECT * FROM files ORDER BY path") :
//        print(row)
//        conn.close()



#include "SDLC/Private/PacksDB.h"
#include <sqlite_modern_cpp.h>

namespace DAVA
{
static void* MyMalloc(int32 size)
{
    return ::malloc(size);
}
static void MyFree(void* ptr)
{
    ::free(ptr);
}
static void* MyRealloc(void* ptr, int32 size)
{
    return ::realloc(ptr, size);
}
static int32 MyMemSize(void* ptr)
{
    return ::_msize(ptr);
}
static int32 MyRoundUp(int32 size)
{
    return size;
}
static int32 MyMemInit(void* pAppData)
{
    return 0;
}
static void MyMemShutdown(void* pAppData)
{
    // do nothing
}

class PacksDBData
{
public:
    PacksDBData(const String& dbPath)
    {
        sqlite3_mem_methods mem = {
            &MyMalloc,
            &MyFree,
            &MyRealloc,
            &MyMemSize,
            &MyRoundUp,
            &MyMemInit,
            &MyMemShutdown
        };
        int32 result = sqlite3_config(SQLITE_CONFIG_MALLOC, &mem);
        db.reset(new sqlite::database(dbPath));
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

const String& PacksDB::FindPack(const FilePath& relativeFilePath) const
{
    static String result;
    String relativePath = relativeFilePath.GetRelativePathname("~res:/");
    data->GetDB() << "SELECT pack_name FROM files WHERE path=?"
                  << relativePath
    >> [&](std::string packName)
    {
        result = std::move(packName);
    };

    return result;
}

void PacksDB::GetAllPacksState(Vector<SmartDlc::PackState>& out) const
{
    out.clear();

    auto selectQuery = data->GetDB() << "SELECT name, status, priority, download FROM packs";

    selectQuery >> [&](String name, int32 status, float32 priority, float32 progress)
    {
        auto stat = static_cast<SmartDlc::PackState::Status>(status);
        out.push_back(SmartDlc::PackState(name, stat, priority, progress));
    };
}

void PacksDB::UpdatePackState(const SmartDlc::PackState& state)
{
    data->GetDB() << "begin";
    data->GetDB() << "UPDATE packs SET status = ?, priority = ?, progress = ? WHERE name = ?"
                  << static_cast<int32>(state.state)
                  << state.priority
                  << state.downloadProgress
                  << state.name;
    data->GetDB() << "commit";
}

} // end namespace DAVA