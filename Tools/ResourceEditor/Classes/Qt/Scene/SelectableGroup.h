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
    ANY EXPRESS OR IMWARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __SELECTABLE_OBJECT_GROUP_H__
#define __SELECTABLE_OBJECT_GROUP_H__

#include "Scene/Selectable.h"

class SelectableGroup
{
public:
    using CollectionType = DAVA::Vector<Selectable>;

public:
    bool operator==(const SelectableGroup& other) const;
    bool operator!=(const SelectableGroup& other) const;

    bool ContainsObject(const Selectable::Object* object) const;

    const CollectionType& GetContent() const;
    CollectionType& GetMutableContent();

    bool IsEmpty() const;
    CollectionType::size_type GetSize() const;
    void Clear();

    void Add(Selectable::Object* object);
    void Add(Selectable::Object* object, const DAVA::AABBox3& box);
    void Remove(Selectable::Object* object);
    void Exclude(const SelectableGroup&);
    void Join(const SelectableGroup&);

    template <typename F>
    void RemoveIf(F func);

    template <typename T>
    bool ContainsObjectsOfType() const;

    /*
	 * TODO : hide this function, recalculate box when needed
	 */
    void RebuildIntegralBoundingBox();
    const DAVA::AABBox3& GetIntegralBoundingBox() const;

    bool SupportsTransformType(Selectable::TransformType transformType) const;
    DAVA::Vector3 GetFirstTranslationVector() const;
    DAVA::Vector3 GetCommonTranslationVector() const;

    const Selectable& GetFirst() const;

    void Lock();
    void Unlock();
    bool IsLocked() const;

public:
    template <typename T>
    class Enumerator
    {
    public:
        class Iterator
        {
        public:
            Iterator(SelectableGroup::CollectionType&);
            Iterator(SelectableGroup::CollectionType&, DAVA::uint32);
            void SetIndex(DAVA::uint32);

            Iterator& operator++();
            bool operator!=(const Iterator&) const;

            T* operator*();

        private:
            DAVA::uint32 index = 0;
            DAVA::uint32 endIndex = 0;
            SelectableGroup::CollectionType& collection;
        };

    public:
        Enumerator(SelectableGroup* group, SelectableGroup::CollectionType& collection);
        ~Enumerator();

        Iterator& begin();
        Iterator& end();

    private:
        SelectableGroup* group = nullptr;
        Iterator iBegin;
        Iterator iEnd;
    };

    template <typename T>
    class ConstEnumerator
    {
    public:
        class Iterator
        {
        public:
            Iterator(const SelectableGroup::CollectionType&);
            Iterator(const SelectableGroup::CollectionType&, DAVA::uint32);
            void SetIndex(DAVA::uint32);

            Iterator& operator++();
            bool operator!=(const Iterator&) const;
            T* operator*() const;

        private:
            DAVA::uint32 index = 0;
            DAVA::uint32 endIndex = 0;
            const SelectableGroup::CollectionType& collection;
        };

    public:
        ConstEnumerator(const SelectableGroup* group, const SelectableGroup::CollectionType& collection);
        ~ConstEnumerator();

        const Iterator& begin() const;
        const Iterator& end() const;

    private:
        const SelectableGroup* group = nullptr;
        Iterator iBegin;
        Iterator iEnd;
    };

    template <typename T>
    Enumerator<T> ObjectsOfType();

    template <typename T>
    ConstEnumerator<T> ObjectsOfType() const;

private:
    CollectionType objects;
    DAVA::AABBox3 integralBoundingBox;
    DAVA::uint32 lockCounter = 0;
};

template <typename F>
inline void SelectableGroup::RemoveIf(F func)
{
    DVASSERT(!IsLocked());
    objects.erase(std::remove_if(objects.begin(), objects.end(), func), objects.end());
}

inline const SelectableGroup::CollectionType& SelectableGroup::GetContent() const
{
    return objects;
}

inline SelectableGroup::CollectionType& SelectableGroup::GetMutableContent()
{
    return objects;
}

inline bool SelectableGroup::IsEmpty() const
{
    return objects.empty();
}

inline SelectableGroup::CollectionType::size_type SelectableGroup::GetSize() const
{
    return objects.size();
}

inline const DAVA::AABBox3& SelectableGroup::GetIntegralBoundingBox() const
{
    return integralBoundingBox;
}

template <typename T>
inline SelectableGroup::Enumerator<T> SelectableGroup::ObjectsOfType()
{
    return SelectableGroup::Enumerator<T>(this, objects);
}

template <typename T>
inline SelectableGroup::ConstEnumerator<T> SelectableGroup::ObjectsOfType() const
{
    return SelectableGroup::ConstEnumerator<T>(this, objects);
}

template <typename T>
bool SelectableGroup::ContainsObjectsOfType() const
{
    for (const auto& obj : objects)
    {
        if (obj.CanBeCastedTo<T>())
            return true;
    }

    return false;
}

/*
 * Enumerator
 */
template <typename T>
inline SelectableGroup::Enumerator<T>::Enumerator(SelectableGroup* g, SelectableGroup::CollectionType& c)
    : group(g)
    , iBegin(c)
    , iEnd(c, c.size())
{
    group->Lock();
}

template <typename T>
inline SelectableGroup::Enumerator<T>::~Enumerator()
{
    group->Unlock();
}

template <typename T>
inline typename SelectableGroup::Enumerator<T>::Iterator& SelectableGroup::Enumerator<T>::begin()
{
    return iBegin;
}

template <typename T>
inline typename SelectableGroup::Enumerator<T>::Iterator& SelectableGroup::Enumerator<T>::end()
{
    return iEnd;
}

/*
 * Iterator
 */
template <typename T>
inline SelectableGroup::Enumerator<T>::Iterator::Iterator(SelectableGroup::CollectionType& c)
    : collection(c)
    , endIndex(static_cast<DAVA::uint32>(c.size()))
{
    while ((index < endIndex) && (collection[index].template CanBeCastedTo<T>() == false))
    {
        ++index;
    }
}

template <typename T>
inline SelectableGroup::Enumerator<T>::Iterator::Iterator(SelectableGroup::CollectionType& c, DAVA::uint32 end)
    : collection(c)
    , index(end)
    , endIndex(static_cast<DAVA::uint32>(c.size()))
{
}

template <typename T>
inline typename SelectableGroup::Enumerator<T>::Iterator& SelectableGroup::Enumerator<T>::Iterator::operator++()
{
    for (++index; index < endIndex; ++index)
    {
        if (collection[index].template CanBeCastedTo<T>())
            break;
    }
    return *this;
}

template <typename T>
inline bool SelectableGroup::Enumerator<T>::Iterator::operator!=(const SelectableGroup::Enumerator<T>::Iterator& other) const
{
    return index != other.index;
}

template <typename T>
inline T* SelectableGroup::Enumerator<T>::Iterator::operator*()
{
    DVASSERT(collection[index].template CanBeCastedTo<T>());
    return collection[index].template Cast<T>();
}

/*
 * Const Enumerator
 */
template <typename T>
inline SelectableGroup::ConstEnumerator<T>::ConstEnumerator(const SelectableGroup* g, const SelectableGroup::CollectionType& c)
    : group(g)
    , iBegin(c)
    , iEnd(c, c.size())
{
}

template <typename T>
inline SelectableGroup::ConstEnumerator<T>::~ConstEnumerator()
{
}

template <typename T>
inline const typename SelectableGroup::ConstEnumerator<T>::Iterator& SelectableGroup::ConstEnumerator<T>::begin() const
{
    return iBegin;
}

template <typename T>
inline const typename SelectableGroup::ConstEnumerator<T>::Iterator& SelectableGroup::ConstEnumerator<T>::end() const
{
    return iEnd;
}

/*
 * Const Iterator
 */
template <typename T>
inline SelectableGroup::ConstEnumerator<T>::Iterator::Iterator(const SelectableGroup::CollectionType& c)
    : collection(c)
    , endIndex(static_cast<DAVA::uint32>(c.size()))
{
    while ((index < endIndex) && (collection[index].template CanBeCastedTo<T>() == false))
    {
        ++index;
    }
}

template <typename T>
inline SelectableGroup::ConstEnumerator<T>::Iterator::Iterator(const SelectableGroup::CollectionType& c, DAVA::uint32 end)
    : collection(c)
    , index(end)
    , endIndex(static_cast<DAVA::uint32>(c.size()))
{
}

template <typename T>
inline typename SelectableGroup::ConstEnumerator<T>::Iterator& SelectableGroup::ConstEnumerator<T>::Iterator::operator++()
{
    for (++index; index < endIndex; ++index)
    {
        if (collection[index].template CanBeCastedTo<T>())
            break;
    }
    return *this;
}

template <typename T>
inline bool SelectableGroup::ConstEnumerator<T>::Iterator::operator!=(const typename SelectableGroup::ConstEnumerator<T>::Iterator& other) const
{
    return index != other.index;
}

template <typename T>
inline T* SelectableGroup::ConstEnumerator<T>::Iterator::operator*() const
{
    DVASSERT(collection[index].template CanBeCastedTo<T>());
    return collection[index].template Cast<T>();
}

#endif // __SELECTABLE_OBJECT_GROUP_H__
