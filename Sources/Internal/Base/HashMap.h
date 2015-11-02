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


#ifndef __DAVAENGINE_HASH_MAP__
#define __DAVAENGINE_HASH_MAP__

#include "Base/BaseTypes.h"
#include "Base/Hash.h"
#include "Base/TemplateHelpers.h"
#include "Math/MathHelpers.h"
#include "Debug/DVAssert.h"
#include <map>

namespace DAVA
{

template <typename K, typename V>
class HashMap
{
public:
	struct HashMapIterator;
    using iterator = HashMapIterator;

    HashMap(size_t _hashSize = 128, V _defaultV = V());
    HashMap(std::initializer_list<std::pair<K, V>> init_list, size_t _hashSize = 128, V _defaultV = V());
    HashMap(const HashMap<K, V>& hm);
    ~HashMap();

    size_t size() const;
    size_t count(const K& key) const;

    bool empty() const;

	iterator insert(const K &key, const V &value);
	void erase(const K &key);
	void clear();
    
	inline V & at(const K &key);
	inline const V & at(const K &key) const;
    
	V & operator[](const K &key);
    const V & operator[] (const K & key) const;

    HashMap<K, V>& operator=(const HashMap<K, V>& hm);
    HashMap<K, V>& operator=(std::initializer_list<std::pair<K, V>> init_list);

    void resize(size_t newSize);

    inline iterator begin() const;
    inline iterator end() const;

    inline iterator find(const K& key) const;

    V& valueByIndex(size_t index);
    const V& valueByIndex(size_t index) const;

    const K& keyByIndex(size_t index);
    const K& keyByIndex(size_t index) const;

public:
	struct HashMapItem
	{
		friend class HashMap;

		const K first;
		V second;

		HashMapItem(const K & k, const V & v)
			: first(k), second(v), next(NULL)
		{ }

	protected:
		HashMapItem *next;
	};

	struct HashMapIterator
	{
		friend class HashMap<K, V>;
        
		HashMapIterator();
		HashMapIterator(const HashMapIterator &i);
		HashMapIterator(const HashMap *map);
        
		inline bool operator==(const HashMapIterator &i) const;
		inline bool operator!=(const HashMapIterator &i) const;

		inline HashMapIterator& operator++();
		inline HashMapIterator operator++(int count);

		HashMapItem& operator*();
		const HashMapItem& operator*() const;

		inline HashMapItem* operator->();
		inline const HashMapItem* operator->() const;
        
	protected:
		size_t szTable;
		size_t current_index;
        
		HashMapItem **table;
		HashMapItem *current_item;
        
		HashMapIterator& GoEnd();
	};

protected:
	size_t sz;
	size_t szTable;

	HashMapItem **table;
	Hash<K> hashFn;

	V defaultV;
	K defaultK;

	inline size_t GetIndex(const K &key) const;
	inline HashMapItem* GetItem(const K &key) const;
	inline size_t InsertItem(HashMapItem* item);
};

// 
// HashMap implementation
// begin -->

template <typename K, typename V>
HashMap<K, V>::HashMap(size_t _hashSize, V _defaultV)
	: sz(0)
	, szTable(_hashSize)
	, defaultV(_defaultV)
{
	// not 0 and power of 2
	DVASSERT(IsPowerOf2((int32)szTable));

	table = new HashMapItem*[szTable];
	for(size_t i = 0; i < szTable; ++i)
	{
		table[i] = NULL;
	}
}

template <typename K, typename V>
HashMap<K, V>::HashMap(std::initializer_list<std::pair<K, V>> init_list, size_t _hashSize, V _defaultV)
    : HashMap(_hashSize, _defaultV)
{
    operator=(init_list);
}

template <typename K, typename V>
HashMap<K, V>::HashMap(const HashMap<K, V>& hm)
    : sz(0)
    , szTable(0)
    , table(NULL)
{
	operator=(hm);
}

template <typename K, typename V>
HashMap<K, V>::~HashMap()
{
	clear();
	delete[] table;
}

template <typename K, typename V>
size_t HashMap<K, V>::size() const
{
	return sz;
}

template <typename K, typename V>
bool HashMap<K, V>::empty() const
{
	return (0 == sz);
}

template <typename K, typename V>
typename HashMap<K, V>::iterator HashMap<K, V>::insert(const K &key, const V &value)
{
	iterator it(this);

	HashMapItem* item = GetItem(key);
	if(item)
	{
		item->second = value;
		it.current_index = GetIndex(key);
	}
	else
	{
		item = new HashMapItem(key, value);
		it.current_index = InsertItem(item);
	}

	it.current_item = item;
	return it;
}

template <typename K, typename V>
void HashMap<K, V>::erase(const K &key)
{
	size_t index = GetIndex(key);
	HashMapItem* item = table[index];
	HashMapItem* prev = NULL;

	while(NULL != item)
	{
		if(hashFn.Compare(item->first, key))
		{
			if(NULL != prev)
			{
				prev->next = item->next;
			}
			else
			{
				table[index] = item->next;
			}

			sz--;
			delete item;

			break;
		}

		prev = item;
		item = item->next;
	}
}

template <typename K, typename V>
size_t HashMap<K, V>::count(const K &key) const
{
	if(GetItem(key))
	{
		return 1;
	}

	return 0;
}
    
template <typename K, typename V>
inline V & HashMap<K, V>::at(const K &key)
{
    HashMapItem* item = GetItem(key);
    if(NULL != item)
    {
        return item->second;
    }
    
    return defaultV;
}
    
template <typename K, typename V>
inline const V & HashMap<K, V>::at(const K &key) const
{
    const HashMapItem* item = GetItem(key);
    if(NULL != item)
    {
        return item->second;
    }
    
    return defaultV;
}

template <typename K, typename V>
V & HashMap<K, V>::operator[](const K &key)
{
    HashMapItem* item = GetItem(key);
    if(NULL != item)
    {
        return item->second;
    }else
    {
        HashMap<K, V>::iterator it = insert(key, defaultV);
        return it.current_item->second;
    }
}
    
template <typename K, typename V>
const V & HashMap<K, V>::operator[](const K &key) const
{
    return at(key);
}

template <typename K, typename V>
HashMap<K, V>& HashMap<K, V>::operator=(const HashMap<K, V> &hm)
{
	if(NULL != table)
	{
		clear();
		delete[] table;
	}

	szTable = hm.szTable;
	defaultV = hm.defaultV;

	table = new HashMapItem*[szTable];
	for(size_t i = 0; i < szTable; ++i)
	{
		table[i] = NULL;
	}

	HashMap<K, V>::iterator i = hm.begin();
	for(; i != hm.end(); ++i)
	{
		insert(i->first, i->second);
	}

	return *this;
}

template <typename K, typename V>
HashMap<K, V>& HashMap<K, V>::operator=(std::initializer_list<std::pair<K, V>> init_list)
{
    for (size_t i = 0; i < szTable; ++i)
    {
        table[i] = NULL;
    }

    for (auto& element : init_list)
    {
        insert(element.first, element.second);
    }

    return *this;
}

template <typename K, typename V>
void HashMap<K, V>::clear()
{
	const HashMapItem* item = NULL;
	const HashMapItem* next = NULL;

	for(size_t i = 0; i < szTable; ++i)
	{
		item = table[i];
		table[i] = NULL;
		while(NULL != item)
		{
			next = item->next;
			delete item;
			item = next;
		}
	}
	
	sz = 0;
}

template <typename K, typename V>
void HashMap<K, V>::resize(size_t newSize)
{
	HashMapItem* item;
	HashMapItem* next = NULL;

	// not 0 and power of 2
	DVASSERT(IsPowerOf2(newSize));

	HashMapItem **oldTable = table;
	size_t szOld = szTable;

	szTable = newSize;
	table = new HashMapItem*[szTable];

	for(size_t i = 0; i < szTable; ++i)
	{
		table[i] = NULL;
	}

	for(size_t i = 0; i < szOld; ++i)
	{
		item = oldTable[i];
		while(NULL != item)
		{
			next = item->next;

			item->next = NULL;
			InsertItem(item);

			item = next;
		}
	}

	delete[] oldTable;
}

template <typename K, typename V>
inline typename HashMap<K, V>::iterator HashMap<K, V>::begin() const
{
	return iterator(this);
}

template <typename K, typename V>
inline typename HashMap<K, V>::iterator HashMap<K, V>::end() const
{
	iterator i(this);
	return i.GoEnd();
}

template <typename K, typename V>
inline typename HashMap<K, V>::iterator HashMap<K, V>::find(const K& key) const
{
	HashMap<K, V>::iterator i(this);
	HashMapItem *item = GetItem(key);

	if(NULL != item)
	{
		i.current_item = item;
		i.current_index = GetIndex(key);
	}
	else
	{
		i.GoEnd();
	}

	return i;
}

template <typename K, typename V>
inline size_t HashMap<K, V>::GetIndex(const K &key) const
{
	// fast hashFn(key) % szTable
	return hashFn(key) & (szTable - 1);
}

template <typename K, typename V>
inline typename HashMap<K, V>::HashMapItem* HashMap<K, V>::GetItem(const K &key) const
{
	size_t index = GetIndex(key);

	HashMapItem* i = table[index];

	while(NULL != i)
	{
		if(hashFn.Compare(i->first, key))
		{
			break;
		}

		i = i->next;
	}

	return i;
}

template <typename K, typename V>
inline size_t HashMap<K, V>::InsertItem(typename HashMap<K, V>::HashMapItem* item)
{
	size_t index = GetIndex(item->first);

	item->next = table[index];
	table[index] = item;

	sz++;

	return index;
}
template <typename K, typename V>
V& HashMap<K, V>::valueByIndex(size_t index)
{
	DVASSERT(index >= 0 && index < size());
	
	size_t curIndex = 0;
	HashMap<K, V>::iterator stateIter;
	if(index < size())
	{
		stateIter = begin();
		//HashMap<K, V>::iterator itEnd = end();
		while(stateIter != end() &&
			  curIndex < index)
		{
			++curIndex;
			++stateIter;
		}
	}
	
	return (curIndex == index) ? stateIter->second : defaultV;
}
	
template <typename K, typename V>
const V& HashMap<K, V>::valueByIndex(size_t index) const
{
	DVASSERT(index >= 0 && index < size());
	
	size_t curIndex = 0;
	HashMap<K, V>::iterator stateIter;
	if(index < size())
	{
		stateIter = begin();
		HashMap<K, V>::iterator itEnd = end();
		while(stateIter != itEnd &&
			  curIndex < index)
		{
			++curIndex;
			++stateIter;
		}
	}
	if (curIndex == index)
	{
	    // fix clang warning on return local variable pointer
	    HashMap<K, V>::HashMapItem* item = stateIter.operator ->();
	    return item->second;
	}
	return defaultV;
}

template <typename K, typename V>
const K& HashMap<K, V>::keyByIndex(size_t index)
{
	DVASSERT(index >= 0 && index < size());
	
	size_t curIndex = 0;
	HashMap<K, V>::iterator stateIter;
	if(index < size())
	{
		stateIter = begin();
		HashMap<K, V>::iterator itEnd = end();
		while(stateIter != itEnd &&
			  curIndex < index)
		{
			++curIndex;
			++stateIter;
		}
	}
	
	return (curIndex == index) ? stateIter->first : defaultK;
}

template <typename K, typename V>
const K& HashMap<K, V>::keyByIndex(size_t index) const
{
	DVASSERT(index >= 0 && index < size());
	
	size_t curIndex = 0;
	HashMap<K, V>::iterator stateIter;
	if(index < size())
	{
		stateIter = begin();
		HashMap<K, V>::iterator itEnd = end();
		while(stateIter != itEnd &&
			  curIndex < index)
		{
			++curIndex;
			++stateIter;
		}
	}
	
	return (curIndex == index) ? stateIter->first : defaultK;
}


// 
// HashMap implementation
// end <--


// 
// HashMapIterator implementation
// begin -->

template <typename K, typename V>
HashMap<K, V>::HashMapIterator::HashMapIterator()
: szTable(0)
, current_index(0)
, table(NULL)
, current_item(NULL)
{ }

template <typename K, typename V>
HashMap<K, V>::HashMapIterator::HashMapIterator(const typename HashMap<K, V>::HashMapIterator &i)
	: szTable(i.szTable)
	, current_index(i.current_index)
	, table(i.table)
	, current_item(i.current_item)
{ }

template <typename K, typename V>
HashMap<K, V>::HashMapIterator::HashMapIterator(const HashMap<K, V> *map)
	: szTable(map->szTable)
	, current_index(0)
	, table(map->table)
	, current_item(NULL)
{
	if(NULL != table && szTable > 0)
	{
		for (uint32 k = 0; k < szTable; ++k)
		{
			if (table[k] != 0)
			{
				current_item = table[k];
				current_index = k;
				break;
			}
		}

		if(NULL == current_item)
		{
			GoEnd();
		}
	}
}

template <typename K, typename V>
bool HashMap<K, V>::HashMapIterator::operator==(const typename HashMap<K, V>::HashMapIterator &i) const
{
	return (szTable == i.szTable &&
		table == i.table &&
		current_index == i.current_index &&
		current_item == i.current_item);
}

template <typename K, typename V>
inline bool HashMap<K, V>::HashMapIterator::operator!=(const typename HashMap<K, V>::HashMapIterator &i) const
{
	return !operator==(i);
}

template <typename K, typename V>
inline typename HashMap<K, V>::HashMapIterator& HashMap<K, V>::HashMapIterator::operator++()
{
	// operator ++iterator

	if(NULL != current_item)
	{
		if(NULL != current_item->next)
		{
			current_item = current_item->next;
		}
		else
		{
			current_index++;

			current_item = 0;
			while(current_index < szTable && current_item == 0)
			{
				current_item = table[current_index];
                if(!current_item) 
				{
					current_index++;
				}
			}

			if (current_item == 0)
			{
				GoEnd();
			}
		}
	}

	return *this;
}

template <typename K, typename V>
inline typename HashMap<K, V>::HashMapIterator HashMap<K, V>::HashMapIterator::operator++(int count)
{
	// operator iterator++

	HashMapIterator orig = *this;
	++(*this);

	return orig;
}


template <typename K, typename V>
typename HashMap<K, V>::HashMapItem& HashMap<K, V>::HashMapIterator::operator*()
{
	return *current_item;
}

template <typename K, typename V>
const typename HashMap<K, V>::HashMapItem& HashMap<K, V>::HashMapIterator::operator*() const
{
	return *current_item;
}

template <typename K, typename V>
inline typename HashMap<K, V>::HashMapItem* HashMap<K, V>::HashMapIterator::operator->()
{
	return current_item;
}

template <typename K, typename V>
inline const typename HashMap<K, V>::HashMapItem* HashMap<K, V>::HashMapIterator::operator->() const
{
	return current_item;
}

template <typename K, typename V>
typename HashMap<K, V>::HashMapIterator& HashMap<K, V>::HashMapIterator::GoEnd()
{
	current_item = NULL;
	current_index = 0;

	return *this;
}

// 
// HashMapIterator implementation
// end <--

};

#endif
