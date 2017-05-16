#ifndef __DAVAENGINE_FAST_NAME_MAP__
#define __DAVAENGINE_FAST_NAME_MAP__

#include "Base/FastName.h"
#include "Base/HashMap.h"

namespace DAVA
{
template <>
struct Hash<FastName>
{
    size_t operator()(const FastName& name) const
    {
        return name.Index();
    }

    bool Compare(const FastName& name1, const FastName& name2) const
    {
        return name1.Index() == name2.Index();
    }
};

class FastNameSet : public HashMap<FastName, int>
{
public:
    FastNameSet(int size = 128)
        : HashMap<FastName, int>(size)
    {
    }

    void Insert(const char* name)
    {
        HashMap<FastName, int>::insert(FastName(name), 0);
    }

    void Insert(const FastName& name)
    {
        HashMap<FastName, int>::insert(name, 0);
    }

    void Combine(const FastNameSet& nameSet)
    {
        FastNameSet::iterator iter = nameSet.begin();
        while (iter != nameSet.end())
        {
            Insert(iter->first);
            ++iter;
        }
    }

    inline bool operator==(const FastNameSet& _another) const;

    //VI: this is slow method! Use it for debug purposes only
    void ToString(String& targetStr) const
    {
        FastNameSet::iterator it = begin();
        const FastNameSet::iterator& endIt = end();
        for (; it != endIt; ++it)
        {
            const FastName& key = it->first;
            targetStr += key.c_str();
            targetStr += " ";
        }
    }

private:
    int Insert(const char* name, const int& value);
    int Value(const FastName& key) const;
    int operator()(const FastName& key) const;
};

template <>
struct Hash<FastNameSet>
{
    size_t operator()(const FastNameSet& set) const
    {
        size_t i = 0;
        Vector<int> indices;
        indices.resize(set.size());

        FastNameSet::iterator it = set.begin();
        const FastNameSet::iterator& endIt = set.end();
        for (; it != endIt; ++it)
        {
            const FastName& key = it->first;
            indices[i] = key.Index();
            i++;
        }

        std::stable_sort(indices.begin(), indices.end());

        size_t keyCount = indices.size();
        size_t hashVal = 2166136261u;
        for (i = 0; i < keyCount; ++i)
        {
            hashVal += (hashVal * 16777619) ^ indices[i];
        }

        return hashVal;
    }

    bool Compare(const FastNameSet& set1, const FastNameSet& set2) const
    {
        return (set1 == set2);
    }
};

template <typename V>
class FastNameMap : public HashMap<FastName, V>
{
public:
    FastNameMap(size_t _hashSize = 128, V _defaultV = V())
        : HashMap<FastName, V>(_hashSize, _defaultV){};
    FastNameMap(std::initializer_list<std::pair<FastName, V>> init_list, size_t _hashSize = 128, V _defaultV = V())
        : HashMap<FastName, V>(init_list, _hashSize, _defaultV){};

    void Insert(const char* name, const V& value)
    {
        HashMap<FastName, V>::insert(FastName(name), value);
    }
    void Insert(const FastName& name, const V& value)
    {
        HashMap<FastName, V>::insert(name, value);
    }
};

// Implementation
inline bool FastNameSet::operator==(const FastNameSet& _another) const
{
    if (size() != _another.size())
        return false;
    FastNameSet::iterator it = this->begin();
    const FastNameSet::iterator& endIt = this->end();
    for (; it != endIt; ++it)
    {
        const FastName& name = it->first;
        if (!_another.count(name))
            return false;
    }
    return true;
}
};
#endif // __DAVAENGINE_FAST_NAME_MAP__
