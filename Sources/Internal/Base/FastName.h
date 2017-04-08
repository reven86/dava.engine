#ifndef __DAVAENGINE_FAST_NAME__
#define __DAVAENGINE_FAST_NAME__

#include "Base/HashMap.h"
#include "Base/StaticSingleton.h"
#include "Base/Any.h"

#include "Concurrency/Mutex.h"

#include <cstdlib>
namespace DAVA
{
struct FastNameDB : public StaticSingleton<FastNameDB>
{
    FastNameDB()
        : namesHash(HashMap<const char*, int>(8192 * 2, -1))
    {
    }

    ~FastNameDB()
    {
        const size_t count = namesTable.size();
        for (size_t i = 0; i < count; ++i)
        {
            SafeDeleteArray(namesTable[i]);
        }
    }

    Vector<const char*> namesTable;
    Vector<int> namesRefCounts;
    Vector<int> namesEmptyIndexes;
    HashMap<const char*, int> namesHash;

    Mutex dbMutex;
};

class FastName
{
public:
    FastName();
    explicit FastName(const char* name);
    FastName(const FastName& _name);
    explicit FastName(const String& name);
    ~FastName();

    inline const char* c_str() const;

    inline FastName& operator=(const FastName& _name);

    inline bool operator==(const FastName& _name) const;

    inline bool operator!=(const FastName& _name) const;

    /**
        \brief This operator doesn't compare strings, it compares only FastName index.
     */
    inline bool operator<(const FastName& _name) const;

    inline size_t find(const char* s, size_t pos = 0) const;

    inline size_t find(const String& str, size_t pos = 0) const;

    inline size_t find(const FastName& fn, size_t pos = 0) const;

    inline const char* operator*() const;

    inline int Index() const;

    inline bool IsValid() const;

private:
    void Init(const char* name);
    int index;

#ifdef __DAVAENGINE_DEBUG__
    const char* debug_str_ptr;
#endif

    void AddRef(int32 i) const;
    void RemRef(int32 i) const;
};

FastName& FastName::operator=(const FastName& _name)
{
    if ((*this) == _name)
        return *this;

    RemRef(index);

    index = _name.index;
		
#ifdef __DAVAENGINE_DEBUG__
    debug_str_ptr = _name.debug_str_ptr;
#endif

    AddRef(index);
    return *this;
}

bool FastName::operator==(const FastName& _name) const
{
    return index == _name.index;
}

bool FastName::operator!=(const FastName& _name) const
{
    return index != _name.index;
}

bool FastName::operator<(const FastName& _name) const
{
    return index < _name.index;
}

size_t FastName::find(const char* s, size_t pos) const
{
    if (c_str() && s)
    {
        const char* q = strstr(c_str() + pos, s);
        return q ? q - c_str() : String::npos;
    }
    return String::npos;
}

size_t FastName::find(const String& str, size_t pos) const
{
    return find(str.c_str(), pos);
}

size_t FastName::find(const FastName& fn, size_t pos) const
{
    return find(fn.c_str(), pos);
}

const char* FastName::operator*() const
{
    return c_str();
}

int FastName::Index() const
{
    return index;
}

bool FastName::IsValid() const
{
    return (index >= 0);
}

const char* FastName::c_str() const
{
    DVASSERT(index >= -1 && index < static_cast<int>(FastNameDB::Instance()->namesTable.size()));
    if (index >= 0)
    {
        return FastNameDB::Instance()->namesTable[index];
    }

    return nullptr;
}

template <>
bool AnyCompare<FastName>::IsEqual(const Any& v1, const Any& v2);
extern template struct AnyCompare<FastName>;
};

namespace std
{
template <>
struct hash<DAVA::FastName>
{
    std::size_t operator()(const DAVA::FastName& k) const
    {
        return k.Index();
    }
};
}

#endif // __DAVAENGINE_FAST_NAME__
