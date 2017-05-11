#include "FastName.h"
#include "Debug/DVAssert.h"

#include "Concurrency/LockGuard.h"

namespace DAVA
{
FastNameDB* FastName::db;

void FastName::Init(const char* name)
{
    static FastNameDB staticDB;
    static bool initialized = false;

    if (!initialized)
    {
        db = &staticDB;
        initialized = true;
    }

    DVASSERT(nullptr != name);

    LockGuard<FastNameDB::MutexT> guard(db->mutex);

    // search if that name is already in hash
    if (db->namesHash.find(name) != db->namesHash.end())
    {
        // already exist, so we just need to set the same index to this object
        index = db->namesHash[name];
    }
    else
    {
        // string isn't in hash and it isn't in names table, so we need to copy it
        // and find place for copied string in names table and set it
        size_t nameLen = strlen(name);
        FastNameDB::CharT* nameCopy = new FastNameDB::CharT[nameLen + 1];
        memcpy(nameCopy, name, nameLen + 1);

        db->sizeOfNames += (nameLen * sizeof(FastNameDB::CharT));

        // index will be a new row in names table
        index = static_cast<int32>(db->namesTable.size());

        db->namesTable.push_back(nameCopy);
        db->namesHash.emplace(nameCopy, index);
    }

    DVASSERT(index != -1);

#ifdef __DAVAENGINE_DEBUG__
    debug_str = c_str();
#endif
}

template <>
bool AnyCompare<FastName>::IsEqual(const Any& v1, const Any& v2)
{
    return v1.Get<FastName>() == v2.Get<FastName>();
}

} // namespace DAVA
