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
#include "Debug/DVAssert.h"

namespace DAVA
{

template <typename K, typename V>
class HashMap
{
protected:
    struct HashMapItem;

public:
	struct HashMapIterator;
	typedef HashMapIterator Iterator;

	HashMap(size_t hashSize = 128, V defaulV = V());
	~HashMap();

	size_t Size();

	bool IsEmpty();
	bool IsKey(const K &key);

	void Insert(const K &key, const V &value);
	void Remove(const K &key);
	void Clear();

	V GetValue(const K &key);
	V operator[](const K &key);

	void Resize(size_t newSize);

	Iterator Begin();
	Iterator End();
    
public:
	struct HashMapIterator
	{
		friend class HashMap<K, V>;
        
		HashMapIterator(const HashMapIterator &i);
		HashMapIterator(const HashMap *map);
        
		bool operator==(const HashMapIterator &i);
		bool operator!=(const HashMapIterator &i);
		HashMapIterator& operator++();
		HashMapIterator operator++(int count);
        
		K GetKey();
		V GetValue();
        
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

	V defaulV;

	inline size_t GetIndex(const K &key);
	inline const HashMapItem* GetItem(const K &key);
	inline void InsertItem(HashMapItem* item);

protected:
	struct HashMapItemBase
	{ };

	struct HashMapItem : public HashMapItemBase
	{
		K key;
		V value;
		HashMapItem *next;

		HashMapItem(K k, V v)
		{
			next = NULL;
			key = k;
			value = v;
		}
	};
};

// 
// HashMap implementation
// begin -->

template <typename K, typename V>
HashMap<K, V>::HashMap(size_t hashSize, V defaulV)
	: sz(0)
	, szTable(hashSize)
	, defaulV(defaulV)
{
	table = new HashMapItem*[szTable];
	for(size_t i = 0; i < szTable; ++i)
	{
		table[i] = NULL;
	}
}

template <typename K, typename V>
HashMap<K, V>::~HashMap()
{
	Clear();
	delete[] table;
}

template <typename K, typename V>
size_t HashMap<K, V>::Size()
{
	return sz;
}

template <typename K, typename V>
bool HashMap<K, V>::IsEmpty()
{
	return (0 == sz);
}

template <typename K, typename V>
void HashMap<K, V>::Insert(const K &key, const V &value)
{
	HashMapItem* item = new HashMapItem(key, value);
	InsertItem(item);
}

template <typename K, typename V>
void HashMap<K, V>::Remove(const K &key)
{
	size_t index = GetIndex(key);
	HashMapItem* item = table[index];
	HashMapItem* prev = NULL;

	while(NULL != item)
	{
		if(hashFn.Compare(item->key, key))
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
bool HashMap<K, V>::IsKey(const K &key)
{
	return (NULL != GetItem(key));
}

template <typename K, typename V>
V HashMap<K, V>::GetValue(const K &key)
{
	const HashMapItem* item = GetItem(key);
	if(NULL != item)
	{
		return item->value;
	}

	return defaulV;
}

template <typename K, typename V>
V HashMap<K, V>::operator[](const K &key)
{
	return GetValue(key);
}

template <typename K, typename V>
void HashMap<K, V>::Clear()
{
	const HashMapItem* item;
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
}

template <typename K, typename V>
void HashMap<K, V>::Resize(size_t newSize)
{
	HashMapItem* item;
	HashMapItem* next = NULL;

	// not 0 and power of 2
	DVASSERT(0 != newSize && 0 == (newSize & (newSize - 1)));

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
typename HashMap<K, V>::Iterator HashMap<K, V>::Begin()
{
	return Iterator(this);
}

template <typename K, typename V>
typename HashMap<K, V>::Iterator HashMap<K, V>::End()
{
	Iterator i(this);
	return i.GoEnd();
}

template <typename K, typename V>
inline size_t HashMap<K, V>::GetIndex(const K &key)
{
	// fast hashFn(key) % szTable
	return hashFn(key) & (szTable - 1);
}

template <typename K, typename V>
inline const typename HashMap<K, V>::HashMapItem* HashMap<K, V>::GetItem(const K &key)
{
	size_t index = GetIndex(key);

	HashMapItem* i = table[index];

	while(NULL != i)
	{
		if(hashFn.Compare(i->key, key))
		{
			break;
		}

		i = i->next;
	}

	return i;
}

template <typename K, typename V>
inline void HashMap<K, V>::InsertItem(typename HashMap<K, V>::HashMapItem* item)
{
	size_t index = GetIndex(item->key);

	item->next = table[index];
	table[index] = item;

	sz++;
}

// 
// HashMap implementation
// end <--


// 
// HashMapIterator implementation
// begin -->

template <typename K, typename V>
HashMap<K, V>::HashMapIterator::HashMapIterator(const typename HashMap<K, V>::HashMapIterator &i)
	: szTable(i.szTable)
	, table(i.table)
	, current_index(i.current_index)
	, current_item(i.current_item)
{ }

template <typename K, typename V>
HashMap<K, V>::HashMapIterator::HashMapIterator(const HashMap<K, V> *map)
	: szTable(map->szTable)
	, table(map->table)
	, current_index(0)
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
bool HashMap<K, V>::HashMapIterator::operator==(const typename HashMap<K, V>::HashMapIterator &i)
{
	return (szTable == i.szTable &&
		table == i.table &&
		current_index == i.current_index &&
		current_item == i.current_item);
}

template <typename K, typename V>
bool HashMap<K, V>::HashMapIterator::operator!=(const typename HashMap<K, V>::HashMapIterator &i)
{
	return !operator==(i);
}

template <typename K, typename V>
typename HashMap<K, V>::HashMapIterator& HashMap<K, V>::HashMapIterator::operator++()
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
			current_item = NULL;
			while(((current_index + 1) < szTable) && (current_item == NULL))
			{
				current_item = table[++current_index];
			}

			if (current_item == NULL)
			{
				GoEnd();
			}
		}
	}

	return *this;
}

template <typename K, typename V>
typename HashMap<K, V>::HashMapIterator HashMap<K, V>::HashMapIterator::operator++(int count)
{
	// operator iterator++

	HashMapIterator tmp = *this;

	while(0 < count--)
	{
		++tmp;
	}

	return tmp;
}

template <typename K, typename V>
K HashMap<K, V>::HashMapIterator::GetKey()
{
	return current_item->key;
}

template <typename K, typename V>
V HashMap<K, V>::HashMapIterator::GetValue()
{
	return current_item->value;
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
