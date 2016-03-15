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


#include "Scene/SelectableObjectGroup.h"

bool SelectableObjectGroup::operator==(const SelectableObjectGroup& other) const
{
    if (GetSize() != other.GetSize())
        return false;

    CollectionType s1 = objects;
    CollectionType s2 = other.objects;
    std::sort(s1.begin(), s1.end());
    std::sort(s2.begin(), s2.end());
    for (size_t i = 0, e = s1.size(); i < e; ++i)
    {
        if (s1[i] != s2[i])
            return false;
    }
    return true;
}

bool SelectableObjectGroup::operator!=(const SelectableObjectGroup& other) const
{
    if (GetSize() != other.GetSize())
        return true;

    CollectionType s1 = objects;
    CollectionType s2 = other.objects;
    std::sort(s1.begin(), s1.end());
    std::sort(s2.begin(), s2.end());
    for (size_t i = 0, e = s1.size(); i < e; ++i)
    {
        if (s1[i] != s2[i])
            return true;
    }
    return false;
}

bool SelectableObjectGroup::ContainsObject(const DAVA::BaseObject* object) const
{
    for (const auto& obj : objects)
    {
        if (obj.GetContainedObject() == object)
            return true;
    }

    return false;
}

void SelectableObjectGroup::Clear()
{
    DVASSERT(!IsLocked());
    objects.clear();
}

void SelectableObjectGroup::Add(DAVA::BaseObject* object)
{
    DVASSERT(!IsLocked());
    objects.emplace_back(object);
}

void SelectableObjectGroup::Add(DAVA::BaseObject* object, const DAVA::AABBox3& box)
{
    DVASSERT(!IsLocked());
    objects.emplace_back(object);
    objects.back().SetBoundingBox(box);
}

void SelectableObjectGroup::Remove(DAVA::BaseObject* object)
{
    RemoveIf([object](const SelectableObject& obj) { return obj.GetContainedObject() == object; });
}

void SelectableObjectGroup::RebuildIntegralBoundingBox()
{
    integralBoundingBox.Empty();
    for (const auto& item : objects)
    {
        integralBoundingBox.AddAABBox(item.GetBoundingBox());
    }
}

void SelectableObjectGroup::Exclude(const SelectableObjectGroup& other)
{
    DVASSERT(!IsLocked());
    RemoveIf([&other](const SelectableObject& object)
             {
                 return other.ContainsObject(object.GetContainedObject());
             });
}

void SelectableObjectGroup::Join(const SelectableObjectGroup& other)
{
    DVASSERT(!IsLocked());
    for (auto& obj : other.GetContent())
    {
        if (!ContainsObject(obj.GetContainedObject()))
        {
            Add(obj.GetContainedObject(), obj.GetBoundingBox());
        }
    }
}

const SelectableObject& SelectableObjectGroup::GetFirst() const
{
    DVASSERT(!objects.empty());
    return objects.front();
}

bool SelectableObjectGroup::SupportsTransformType(SelectableObject::TransformType transformType) const
{
    for (const auto& obj : objects)
    {
        if (!obj.SupportsTransformType(transformType))
            return false;
    }

    return true;
}

DAVA::Vector3 SelectableObjectGroup::GetFirstTranslationVector() const
{
    if (objects.empty())
        return DAVA::Vector3(0.0f, 0.0f, 0.0f);

    return objects.front().GetWorldTransform().GetTranslationVector();
}

DAVA::Vector3 SelectableObjectGroup::GetCommonTranslationVector() const
{
    DAVA::AABBox3 tmp;
    for (const auto& item : objects)
    {
        tmp.AddPoint(item.GetWorldTransform().GetTranslationVector());
    }
    return tmp.GetCenter();
}

void SelectableObjectGroup::Lock()
{
    ++lockCounter;
}

void SelectableObjectGroup::Unlock()
{
    --lockCounter;
}

bool SelectableObjectGroup::IsLocked() const
{
    return lockCounter > 0;
}