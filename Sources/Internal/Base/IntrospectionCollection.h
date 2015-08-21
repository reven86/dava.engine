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


#ifndef __DAVAENGINE_INTROSPECTION_COLLECTION_H__
#define __DAVAENGINE_INTROSPECTION_COLLECTION_H__

#include "Base/IntrospectionBase.h"

namespace DAVA
{
	// Класс представляет расширение базового класса IntrospectionMember и описывает члена интроспекции, как коллекцию
	// Поддерживаемые коллекци - контейнеры с одним шаблонным параметром: Vector, List, Set
	template<template <typename, typename> class C, typename T, typename A>
	class InspCollImpl : public InspColl
	{
	public:
        using CollectionT = C<T, A>;

		InspCollImpl(const char *_name, const InspDesc &_desc, const size_t _offset, const MetaInfo *_type, int _flags = 0)
			: InspColl(_name, _desc, _offset, _type, _flags)
		{ }

		DAVA::MetaInfo* CollectionType() const
		{
			return DAVA::MetaInfo::Instance<CollectionT>();
		}

		DAVA::MetaInfo* ItemType() const
		{
			return DAVA::MetaInfo::Instance<T>();
		}

		int Size(void *object) const
		{
			int size = 0;

			if(nullptr != object)
			{
				size = static_cast<int>(((CollectionT *) object)->size());
			}

			return size;
		}

		Iterator Begin(void *object) const
		{
			Iterator i = nullptr;

			if(nullptr != object)
			{
                CollectionT *collection = (CollectionT *) object;

				typename CollectionT::iterator begin = collection->begin();
				typename CollectionT::iterator end = collection->end();

				if(begin != end)
				{
					CollectionPos *pos = new CollectionPos();
					pos->curPos = begin;
					pos->endPos = end;

					i = (Iterator) pos;
				}
			}

			return i;
		}

		Iterator Next(Iterator i) const
		{
			CollectionPos* pos = (CollectionPos *) i;

			if(nullptr != pos)
			{
				pos->curPos++;

				if(pos->curPos == pos->endPos)
				{
					delete pos;
					i = nullptr;
				}
			}

			return i;
		}

		void Finish(Iterator i) const
		{
			CollectionPos* pos = (CollectionPos *) i;
			if(nullptr != pos)
			{
				delete pos;
			}
		}

		void ItemValueGet(Iterator i, void *itemDst) const
		{
			CollectionPos* pos = (CollectionPos *) i;
			if(nullptr != pos)
			{
				T *dstT = (T*) itemDst;
				*dstT = *(pos->curPos);
			}
		}

		void ItemValueSet(Iterator i, void *itemSrc)
		{
			CollectionPos* pos = (CollectionPos *) i;
			if(nullptr != pos)
			{
				T *srcT = (T*) itemSrc;
				*(pos->curPos) = *srcT;
			}
		}

		void* ItemPointer(Iterator i) const 
		{
			void *p = nullptr;
			CollectionPos* pos = (CollectionPos *) i;

			if(nullptr != pos)
			{
				p = &(*(pos->curPos));
			}

			return p;
		}

		void* ItemData(Iterator i) const
		{
			if(ItemType()->IsPointer())
			{
				return *((void **) ItemPointer(i));
			}
			else
			{
				return ItemPointer(i);
			}
		}

		MetaInfo* ItemKeyType() const
		{
			return nullptr;
		}

		const void* ItemKeyPointer(Iterator i) const
		{
			return nullptr;
		}

		const void* ItemKeyData(Iterator i) const
		{
			return nullptr;
		}

		const InspColl* Collection() const
		{
			return this;
		}

	protected:
		struct CollectionPos
		{
			typename CollectionT::iterator curPos;
			typename CollectionT::iterator endPos;
		};
	};

	template<template <typename, typename, typename> class C, typename K, typename V, typename A>
	class InspKeyedCollImpl : public InspColl
	{
	public:
        using CollectionT = C<K, V, A>;

		InspKeyedCollImpl(const char *_name, const InspDesc &_desc, const size_t _offset, const MetaInfo *_type, int _flags = 0)
			: InspColl(_name, _desc, _offset, _type, _flags)
		{ }

		DAVA::MetaInfo* CollectionType() const
		{
			return DAVA::MetaInfo::Instance<CollectionT>();
		}

		DAVA::MetaInfo* ItemType() const
		{
			return DAVA::MetaInfo::Instance<V>();
		}

		int Size(void *object) const
		{
			int size = 0;

			if(nullptr != object)
			{
				size = ((CollectionT *)object)->size();
			}

			return size;
		}

		Iterator Begin(void *object) const
		{
			Iterator i = nullptr;

			if(nullptr != object)
			{
				CollectionT *collection = (CollectionT *)object;

				typename CollectionT::iterator begin = collection->begin();
				typename CollectionT::iterator end = collection->end();

				if(begin != end)
				{
					CollectionPos *pos = new CollectionPos();
					pos->curPos = begin;
					pos->endPos = end;

					i = (Iterator) pos;
				}
			}

			return i;
		}

		Iterator Next(Iterator i) const
		{
			CollectionPos* pos = (CollectionPos *)i;

			if(nullptr != pos)
			{
				pos->curPos++;

				if(pos->curPos == pos->endPos)
				{
					delete pos;
					i = nullptr;
				}
			}

			return i;
		}

		void Finish(Iterator i) const
		{
			CollectionPos* pos = (CollectionPos *)i;
			if(nullptr != pos)
			{
				delete pos;
			}
		}

		void ItemValueGet(Iterator i, void *itemDst) const
		{
			CollectionPos* pos = (CollectionPos *)i;
			if(nullptr != pos)
			{
				V *dstT = (V*) itemDst;
				*dstT = pos->curPos->second;
			}
		}

		void ItemValueSet(Iterator i, void *itemSrc)
		{
			CollectionPos* pos = (CollectionPos *)i;
			if(nullptr != pos)
			{
				V *srcT = (V*) itemSrc;
				pos->curPos->second = *srcT;
			}
		}

		void* ItemPointer(Iterator i) const
		{
			void *p = nullptr;
			CollectionPos* pos = (CollectionPos *)i;

			if(nullptr != pos)
			{
				p = &pos->curPos->second;
			}

			return p;
		}

		const char* ItemName(Iterator i) const
		{
			return nullptr;
		}

		void* ItemData(Iterator i) const
		{
			if(ItemType()->IsPointer())
			{
				return *((void **)ItemPointer(i));
			}
			else
			{
				return ItemPointer(i);
			}
		}

		MetaInfo* ItemKeyType() const
		{
			return DAVA::MetaInfo::Instance<K>();
		}

		const void* ItemKeyPointer(Iterator i) const
		{
			const void *p = nullptr;
			CollectionPos* pos = (CollectionPos *)i;

			if(nullptr != pos)
			{
				p = &(pos->curPos->first);
			}

			return p;
		}

		const void* ItemKeyData(Iterator i) const
		{
			if(ItemKeyType()->IsPointer())
			{
				return *((const void **)ItemKeyPointer(i));
			}
			else
			{
				return ItemKeyPointer(i);
			}
		}

		const InspColl* Collection() const
		{
			return this;
		}

	protected:
		struct CollectionPos
		{
			typename CollectionT::iterator curPos;
			typename CollectionT::iterator endPos;
		};
	};

	// Функция создает IntrospectionCollection, типы выводятся автоматически
	template<template <typename, typename> class Container, class T, class A>
	static InspColl* CreateInspColl(Container<T, A> *t, const char *_name, const InspDesc &_desc, const size_t _offset, const MetaInfo *_type, int _flags)
	{
		return new InspCollImpl<Container, T, A>(_name, _desc, _offset, _type, _flags);
	}

	template<template <typename, typename, typename> class Container, class K, class V, class A>
	static InspColl* CreateInspColl(Container<K, V, A> *t, const char *_name, const InspDesc &_desc, const size_t _offset, const MetaInfo *_type, int _flags)
	{
		return new InspKeyedCollImpl<Container, K, V, A>(_name, _desc, _offset, _type, _flags);
	}
};

#endif
