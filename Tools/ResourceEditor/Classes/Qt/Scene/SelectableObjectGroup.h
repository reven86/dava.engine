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

#include "Scene/SelectableObject.h"
#include <atomic>

class SelectableObjectGroup
{
public:
    using CollectionType = DAVA::Vector<SelectableObject>;

public:
    SelectableObjectGroup() = default;
    ~SelectableObjectGroup() = default;

    bool operator==(const SelectableObjectGroup& other) const;
    bool operator!=(const SelectableObjectGroup& other) const;

    bool ContainsObject(const DAVA::BaseObject* object) const;

    const CollectionType& GetContent() const;
    CollectionType& GetMutableContent();

    bool IsEmpty() const;
    CollectionType::size_type GetSize() const;
    void Clear();

    void Add(DAVA::BaseObject* object, const DAVA::AABBox3& box);
    void Remove(DAVA::BaseObject* object);
    void Exclude(const SelectableObjectGroup&);
    void Join(const SelectableObjectGroup&);

    template <typename F>
    void RemoveIf(F func);

    /*
	 * TODO : hide this function, recalculate box when needed
	 */
    void RebuildIntegralBoundingBox();
    const DAVA::AABBox3& GetIntegralBoundingBox() const;

    DAVA::Vector3 GetFirstTranslationVector() const;
    DAVA::Vector3 GetCommonTranslationVector() const;

    const SelectableObject& GetFirst() const;

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
            Iterator(SelectableObjectGroup::CollectionType&);
            Iterator(SelectableObjectGroup::CollectionType&, DAVA::uint32);
            void SetIndex(DAVA::uint32);

            Iterator& operator++();
            bool operator!=(const Iterator&) const;
            T* operator*();

        private:
            DAVA::uint32 index = 0;
            DAVA::uint32 endIndex = 0;
            SelectableObjectGroup::CollectionType& collection;
        };

    public:
        Enumerator(SelectableObjectGroup* group, SelectableObjectGroup::CollectionType& collection);
        ~Enumerator();

        Iterator& begin();
        Iterator& end();

    private:
        SelectableObjectGroup* group = nullptr;
        Iterator iBegin;
        Iterator iEnd;
    };

    template <typename T>
    Enumerator<T> ObjectsOfType();

private:
    CollectionType objects;
    DAVA::AABBox3 integralBoundingBox;
    DAVA::uint32 lockCounter = 0;
};

template <typename F>
inline void SelectableObjectGroup::RemoveIf(F func)
{
    DVASSERT(!IsLocked());
    objects.erase(std::remove_if(objects.begin(), objects.end(), func), objects.end());
}

inline const SelectableObjectGroup::CollectionType& SelectableObjectGroup::GetContent() const
{
    return objects;
}

inline SelectableObjectGroup::CollectionType& SelectableObjectGroup::GetMutableContent()
{
    return objects;
}

inline bool SelectableObjectGroup::IsEmpty() const
{
    return objects.empty();
}

inline SelectableObjectGroup::CollectionType::size_type SelectableObjectGroup::GetSize() const
{
    return objects.size();
}

inline const DAVA::AABBox3& SelectableObjectGroup::GetIntegralBoundingBox() const
{
    return integralBoundingBox;
}

template <typename T>
inline SelectableObjectGroup::Enumerator<T> SelectableObjectGroup::ObjectsOfType()
{
    return SelectableObjectGroup::Enumerator<T>(this, objects);
}

/*
 * Enumerator
 */
template <typename T>
inline SelectableObjectGroup::Enumerator<T>::Enumerator(SelectableObjectGroup* g, SelectableObjectGroup::CollectionType& c)
    : group(g)
    , iBegin(c)
    , iEnd(c, c.size())
{
    group->Lock();
}

template <typename T>
inline SelectableObjectGroup::Enumerator<T>::~Enumerator()
{
    group->Unlock();
}

template <typename T>
inline typename SelectableObjectGroup::Enumerator<T>::Iterator& SelectableObjectGroup::Enumerator<T>::begin()
{
    return iBegin;
}

template <typename T>
inline typename SelectableObjectGroup::Enumerator<T>::Iterator& SelectableObjectGroup::Enumerator<T>::end()
{
    return iEnd;
}

/*
 * Iterator
 */
template <typename T>
inline SelectableObjectGroup::Enumerator<T>::Iterator::Iterator(SelectableObjectGroup::CollectionType& c)
    : collection(c)
    , endIndex(static_cast<DAVA::uint32>(c.size()))
{
    while ((index < endIndex) && collection[index].CanBeCastedTo<T>())
    {
        ++index;
    }
}

template <typename T>
inline SelectableObjectGroup::Enumerator<T>::Iterator::Iterator(SelectableObjectGroup::CollectionType& c, DAVA::uint32 end)
    : collection(c)
    , index(end)
    , endIndex(static_cast<DAVA::uint32>(c.size()))
{
}

template <typename T>
inline typename SelectableObjectGroup::Enumerator<T>::Iterator& SelectableObjectGroup::Enumerator<T>::Iterator::operator++()
{
    for (++index; index < endIndex; ++index)
    {
        if (collection[index].CanBeCastedTo<T>())
            break;
    }
    return *this;
}

template <typename T>
inline bool SelectableObjectGroup::Enumerator<T>::Iterator::operator!=(typename const SelectableObjectGroup::Enumerator<T>::Iterator& other) const
{
    return index != other.index;
}

template <typename T>
inline T* SelectableObjectGroup::Enumerator<T>::Iterator::operator*()
{
    DVASSERT(collection[index].CanBeCastedTo<T>());
    return collection[index].Cast<T>();
}


#endif // __SELECTABLE_OBJECT_GROUP_H__
