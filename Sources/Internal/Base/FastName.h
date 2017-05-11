#ifndef __DAVAENGINE_FAST_NAME__
#define __DAVAENGINE_FAST_NAME__

#include "Base/Hash.h"
#include "Base/Any.h"
#include "Concurrency/Spinlock.h"

namespace DAVA
{
struct FastNameDB
{
    using MutexT = Spinlock;
    using CharT = char;

    FastNameDB()
        : namesHash(8192 * 2)
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

    struct FastNameDBHash
    {
        size_t operator()(const char* str) const
        {
            return DavaHashString(str);
        }
    };

    struct FastNameDBEqualTo
    {
        bool operator()(const char* left, const char* right) const
        {
            return (0 == strcmp(left, right));
        }
    };

    Vector<const CharT*> namesTable;
    UnorderedMap<const CharT*, int, FastNameDBHash, FastNameDBEqualTo> namesHash;

    MutexT mutex;
    size_t sizeOfNames = 0;
};

class FastName
{
    static FastNameDB* db;

public:
    FastName();
    explicit FastName(const char* name);
    explicit FastName(const String& name);

    const char* c_str() const;

    bool operator<(const FastName& _name) const;
    bool operator==(const FastName& _name) const;
    bool operator!=(const FastName& _name) const;

    bool empty() const;
    size_t find(const char* s, size_t pos = 0) const;
    size_t find(const String& str, size_t pos = 0) const;
    size_t find(const FastName& fn, size_t pos = 0) const;

    int Index() const;
    bool IsValid() const;

private:
    void Init(const char* name);

    int index = -1;
    
#ifdef __DAVAENGINE_DEBUG__
    const char* debug_str = nullptr;
#endif
};

inline FastName::FastName() = default;

inline FastName::FastName(const String& name)
{
    Init(name.c_str());
}

inline FastName::FastName(const char* name)
{
    Init(name);
}

inline bool FastName::operator==(const FastName& _name) const
{
    return index == _name.index;
}

inline bool FastName::operator!=(const FastName& _name) const
{
    return index != _name.index;
}

inline bool FastName::operator<(const FastName& _name) const
{
    return index < _name.index;
}

inline bool FastName::empty() const
{
    return (index < 0);
}

inline size_t FastName::find(const char* s, size_t pos) const
{
    if (c_str() && s)
    {
        const char* q = strstr(c_str() + pos, s);
        return q ? q - c_str() : String::npos;
    }
    return String::npos;
}

inline size_t FastName::find(const String& str, size_t pos) const
{
    return find(str.c_str(), pos);
}

inline size_t FastName::find(const FastName& fn, size_t pos) const
{
    return find(fn.c_str(), pos);
}

inline int FastName::Index() const
{
    return index;
}

inline bool FastName::IsValid() const
{
    return !empty();
}

inline const char* FastName::c_str() const
{
    DVASSERT(index >= -1 && index < static_cast<int>(db->namesTable.size()));

    if (index >= 0)
    {
        return db->namesTable[index];
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
