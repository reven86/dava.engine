#include "FastName.h"
#include "Debug/DVAssert.h"

#include "Concurrency/LockGuard.h"

namespace DAVA
{
FastName::FastName()
    : index(-1)
{
    // make sure FastNameDB exists
    FastNameDB::Instance();

#ifdef __DAVAENGINE_DEBUG__
    debug_str_ptr = nullptr;
#endif
}

FastName::FastName(const String& name)
    : index(-1)
{
    Init(name.c_str());
}

FastName::FastName(const char* name)
    : index(-1)
{
    Init(name);
}

FastName::FastName(const FastName& _name)
{
    index = _name.index;
    
#ifdef __DAVAENGINE_DEBUG__
    debug_str_ptr = _name.debug_str_ptr;
#endif

    if (-1 != index)
    {
        AddRef(index);
    }
}

FastName::~FastName()
{
    if (-1 != index)
    {
        RemRef(index);
    }
}

void FastName::Init(const char* name)
{
    DVASSERT(nullptr != name);

    FastNameDB* db = FastNameDB::Instance();
    LockGuard<Mutex> guard(FastNameDB::Instance()->dbMutex);

    // search if that name is already in hash
    if (db->namesHash.count(name))
    {
        // already exist, so we just need to set the same index to this object
        index = db->namesHash[name];
        db->namesRefCounts[index]++;
    }
    else
    {
        // string isn't in hash and it isn't in names table, so we need to copy it
        // and find place for copied string in names table and set it
        size_t nameLen = strlen(name);
        char* nameCopy = new char[nameLen + 1];
        memcpy(nameCopy, name, nameLen + 1);

        // search for empty indexes in names table
        if (db->namesEmptyIndexes.size() > 0)
        {
            // take last empty index from emptyIndexes table
            index = db->namesEmptyIndexes.back();
            db->namesEmptyIndexes.pop_back();
        }
        else
        {
            // index will be a new row in names table
            index = static_cast<int32>(db->namesTable.size());
            db->namesTable.resize(index + 1);
            db->namesRefCounts.resize(index + 1);
        }

        // set name to names table
        db->namesTable[index] = nameCopy;
        db->namesRefCounts[index] = 1;

        // add name and its index into hash
        db->namesHash.insert(nameCopy, index);
    }

    DVASSERT(index != -1);
#ifdef __DAVAENGINE_DEBUG__
    debug_str_ptr = c_str();
#endif
}

void FastName::AddRef(int32 i) const
{
    LockGuard<Mutex> guard(FastNameDB::Instance()->dbMutex);

    FastNameDB* db = FastNameDB::Instance();
    DVASSERT(i >= -1 && i < static_cast<int32>(db->namesTable.size()));
    if (i >= 0)
    {
        db->namesRefCounts[i]++;
    }
}

void FastName::RemRef(int32 i) const
{
    LockGuard<Mutex> guard(FastNameDB::Instance()->dbMutex);

    FastNameDB* db = FastNameDB::Instance();
    DVASSERT(i >= -1 && i < static_cast<int32>(db->namesTable.size()));
    if (i >= 0)
    {
        db->namesRefCounts[i]--;
        if (0 == db->namesRefCounts[i])
        {
            DVASSERT(db->namesTable[i] && "Need be not NULL");

            // remove name and index from hash
            db->namesHash.erase(db->namesTable[i]);

            // delete allocated memory for this string
            delete[] db->namesTable[i];

            // remove name from names table
            db->namesTable[i] = nullptr;

            // remember that this index is empty already
            db->namesEmptyIndexes.push_back(i);
        }
    }
}

template <>
bool AnyCompare<FastName>::IsEqual(const Any& v1, const Any& v2)
{
    return v1.Get<FastName>() == v2.Get<FastName>();
}
};
