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


#ifndef __SELECTABLE_OBJECT_GROUP_H__
#define __SELECTABLE_OBJECT_GROUP_H__

#include "Scene/SelectableObject.h"

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

private:
    template <typename T>
    void GetObjectsOfType(DAVA::Vector<T>& collection) const;

private:
    CollectionType objects;
    DAVA::AABBox3 integralBoundingBox;
};

template <typename T>
inline void SelectableObjectGroup::GetObjectsOfType(DAVA::Vector<T>& collection) const
{
    for (auto object : objects)
    {
        if (object->CanBeCastedTo<T>())
        {
            collection.push_back(object->Cast<T>());
        }
    }
}

template <typename F>
inline void SelectableObjectGroup::RemoveIf(F func)
{
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

#endif // __SELECTABLE_OBJECT_GROUP_H__
